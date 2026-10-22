/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/*
 * hidumper_target.cpp - synthetic dump targets for hidumper function UT.
 *
 * Provides three self-driven targets inside this app process so that
 * `hidumper --mem-heap <pid> --native|--jsvm|--kotlin` has a deterministic
 * victim without depending on third-party apps:
 *   1. Native malloc worker   : keeps the native heap populated/churning.
 *   2. JSVM worker            : creates a JSVM isolate and periodically runs
 *                               a script, so RequestInterrupt-based heap
 *                               snapshot dumps always hit an active isolate.
 *   3. KMP mem-dump listener  : registers OH_HiDebug_RegisterMemDumpListener
 *                               ("KMP") via dlopen/dlsym (symbol is API 26+,
 *                               resolved at runtime; graceful when absent).
 *                               The callback writes a synthetic snapshot
 *                               (>1024 bytes) into the faultloggerd fd.
 */
#include <atomic>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <dlfcn.h>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <unistd.h>

#include <napi/native_api.h>
#include <hilog/log.h>

/* Types only; libjsvm.so is resolved at runtime (see JsvmApi), NOT linked: the 32-bit board
 * image ships no libjsvm.so and a DT_NEEDED dependency would abort the whole napi module. */
#include "ark_runtime/jsvm.h"

#undef LOG_DOMAIN
#undef LOG_TAG
#define LOG_DOMAIN 0xFDF0
#define LOG_TAG "HidumperFuncTarget"

namespace {
constexpr uint32_t MALLOC_BLOCK_SIZE = 64 * 1024;
constexpr uint32_t MALLOC_TOUCH_BYTES = 64; // first-page touch pattern size
constexpr size_t MALLOC_MAX_BLOCKS = 128; // pool cap ~8MB
constexpr uint32_t MALLOC_INTERVAL_US = 100 * 1000;
constexpr uint32_t JSVM_INTERVAL_US = 50 * 1000;
// Mirrors OH_HiDebug_MemListenerType (API 26, absent from older native SDK headers).
constexpr int32_t MEM_LISTENER_DUMP_SNAPSHOT = 2;
// Payload must exceed the UT artifact threshold MIN_SNAPSHOT_BYTES (1024).
constexpr int32_t KMP_SNAPSHOT_RECORDS = 96;
// g_jsvmState negative codes
constexpr int JSVM_STATE_API_FAILURE = -1;
constexpr int JSVM_STATE_LIB_MISSING = -3;
// RegisterKmpListener negative codes (also stored in g_kmpState)
constexpr int KMP_STATE_NO_LIB = -1;
constexpr int KMP_STATE_NO_SYMBOL = -2;

std::atomic<bool> g_started { false };
std::atomic<int> g_mallocState { 0 }; // 0=init 1=running -1=failed
std::atomic<int> g_jsvmState { 0 };   // 0=init 1=running <0=failed (see codes above)
std::atomic<int> g_kmpState { -1000 }; // >=0: rc; KMP_STATE_* on failure; -1000: not started

/*
 * Runtime-resolved JSVM native API. Signatures come from the native API header via decltype so
 * they can never drift; the library itself is dlopen'ed because not every product image ships
 * it (verified: the 32-bit board has no libjsvm.so, the phone ships one).
 */
struct JsvmApi {
    void *handle = nullptr;
    decltype(&OH_JSVM_Init) init = nullptr;
    decltype(&OH_JSVM_CreateVM) createVM = nullptr;
    decltype(&OH_JSVM_OpenVMScope) openVMScope = nullptr;
    decltype(&OH_JSVM_CreateEnv) createEnv = nullptr;
    decltype(&OH_JSVM_OpenEnvScope) openEnvScope = nullptr;
    decltype(&OH_JSVM_OpenHandleScope) openHandleScope = nullptr;
    decltype(&OH_JSVM_CreateStringUtf8) createStringUtf8 = nullptr;
    decltype(&OH_JSVM_CompileScript) compileScript = nullptr;
    decltype(&OH_JSVM_RunScript) runScript = nullptr;
    decltype(&OH_JSVM_PumpMessageLoop) pumpMessageLoop = nullptr;
};

bool LoadJsvmApi(JsvmApi &api)
{
    api.handle = dlopen("libjsvm.so", RTLD_NOW);
    if (api.handle == nullptr) {
        OH_LOG_WARN(LOG_APP, "libjsvm.so unavailable: %{public}s", dlerror());
        return false;
    }
    const std::pair<const char *, void **> symbols[] = {
        { "OH_JSVM_Init", reinterpret_cast<void **>(&api.init) },
        { "OH_JSVM_CreateVM", reinterpret_cast<void **>(&api.createVM) },
        { "OH_JSVM_OpenVMScope", reinterpret_cast<void **>(&api.openVMScope) },
        { "OH_JSVM_CreateEnv", reinterpret_cast<void **>(&api.createEnv) },
        { "OH_JSVM_OpenEnvScope", reinterpret_cast<void **>(&api.openEnvScope) },
        { "OH_JSVM_OpenHandleScope", reinterpret_cast<void **>(&api.openHandleScope) },
        { "OH_JSVM_CreateStringUtf8", reinterpret_cast<void **>(&api.createStringUtf8) },
        { "OH_JSVM_CompileScript", reinterpret_cast<void **>(&api.compileScript) },
        { "OH_JSVM_RunScript", reinterpret_cast<void **>(&api.runScript) },
        { "OH_JSVM_PumpMessageLoop", reinterpret_cast<void **>(&api.pumpMessageLoop) },
    };
    for (const auto &symbol : symbols) {
        *(symbol.second) = dlsym(api.handle, symbol.first);
        if (*(symbol.second) == nullptr) {
            OH_LOG_WARN(LOG_APP, "jsvm symbol %{public}s missing", symbol.first);
            dlclose(api.handle);
            api.handle = nullptr;
            return false;
        }
    }
    return true;
}

void MallocWorker()
{
    std::vector<void *> pool;
    g_mallocState = 1;
    OH_LOG_INFO(LOG_APP, "native malloc worker started");
    while (g_started.load()) {
        void *block = malloc(MALLOC_BLOCK_SIZE);
        if (block != nullptr) {
            volatile unsigned char *touched = static_cast<volatile unsigned char *>(block);
            for (uint32_t i = 0; i < MALLOC_TOUCH_BYTES; ++i) {
                touched[i] = 0xAB;
            }
            pool.push_back(block);
        }
        if (pool.size() > MALLOC_MAX_BLOCKS) {
            free(pool.front());
            pool.erase(pool.begin());
        }
        usleep(MALLOC_INTERVAL_US);
    }
}

/* Opens the VM, the env and the scopes the keep-alive script needs. */
bool CreateJsvmRuntime(JsvmApi &api, JSVM_VM &vm, JSVM_Env &env)
{
    JSVM_InitOptions initOptions {};
    JSVM_Status status = api.init(&initOptions);
    // JSVM_GENERIC_FAILURE means already initialized in this process: fine.
    if (status != JSVM_OK && status != JSVM_GENERIC_FAILURE) {
        OH_LOG_ERROR(LOG_APP, "OH_JSVM_Init failed: %{public}d", static_cast<int>(status));
        return false;
    }
    JSVM_CreateVMOptions vmOptions {};
    if (api.createVM(&vmOptions, &vm) != JSVM_OK || vm == nullptr) {
        OH_LOG_ERROR(LOG_APP, "OH_JSVM_CreateVM failed");
        return false;
    }
    JSVM_VMScope scope = nullptr;
    if (api.openVMScope(vm, &scope) != JSVM_OK) {
        OH_LOG_ERROR(LOG_APP, "OH_JSVM_OpenVMScope failed");
        return false;
    }
    if (api.createEnv(vm, 0, nullptr, &env) != JSVM_OK || env == nullptr) {
        OH_LOG_ERROR(LOG_APP, "OH_JSVM_CreateEnv failed");
        return false;
    }
    /* Value-creating APIs require an active env scope; without one the engine layer aborts. */
    JSVM_EnvScope envScope = nullptr;
    if (api.openEnvScope(env, &envScope) != JSVM_OK) {
        OH_LOG_ERROR(LOG_APP, "OH_JSVM_OpenEnvScope failed");
        return false;
    }
    /* The JSVM implementation on the phone product follows the node-api contract and aborts in
     * the engine (handle-scope extension) when a value is created without an explicit handle
     * scope, while newer open-source builds manage it internally. Opening one is valid on both;
     * verified with a standalone probe binary on SCA-AL00 7.1.0.42. */
    JSVM_HandleScope handleScope = nullptr;
    if (api.openHandleScope(env, &handleScope) != JSVM_OK) {
        OH_LOG_ERROR(LOG_APP, "OH_JSVM_OpenHandleScope failed");
        return false;
    }
    return true;
}

/* Compiles the script that keeps the isolate allocating and thus dumpable. */
bool CompileKeepAliveScript(const JsvmApi &api, JSVM_Env env, JSVM_Script &script)
{
    static const char *keepAliveScript =
        "(function() {"
        "  if (!globalThis.__hidumperKeep) { globalThis.__hidumperKeep = []; }"
        "  for (var i = 0; i < 64; i++) {"
        "    globalThis.__hidumperKeep.push({ idx: i, pad: 'jsvm-keepalive-payload' });"
        "  }"
        "  if (globalThis.__hidumperKeep.length > 8192) {"
        "    globalThis.__hidumperKeep.splice(0, 4096);"
        "  }"
        "  return globalThis.__hidumperKeep.length;"
        "})()";
    JSVM_Value scriptSource = nullptr;
    if (api.createStringUtf8(env, keepAliveScript, strlen(keepAliveScript), &scriptSource) != JSVM_OK) {
        OH_LOG_ERROR(LOG_APP, "OH_JSVM_CreateStringUtf8 failed");
        return false;
    }
    bool cacheRejected = false;
    if (api.compileScript(env, scriptSource, nullptr, 0, false, &cacheRejected, &script) != JSVM_OK) {
        OH_LOG_ERROR(LOG_APP, "OH_JSVM_CompileScript failed");
        return false;
    }
    return true;
}

void JsvmWorker()
{
    JsvmApi api;
    if (!LoadJsvmApi(api)) {
        g_jsvmState = JSVM_STATE_LIB_MISSING;
        return;
    }
    JSVM_VM vm = nullptr;
    JSVM_Env env = nullptr;
    if (!CreateJsvmRuntime(api, vm, env)) {
        g_jsvmState = JSVM_STATE_API_FAILURE;
        return;
    }
    JSVM_Script script = nullptr;
    if (!CompileKeepAliveScript(api, env, script)) {
        g_jsvmState = JSVM_STATE_API_FAILURE;
        return;
    }
    g_jsvmState = 1;
    OH_LOG_INFO(LOG_APP, "jsvm isolate keep-alive started");
    while (g_started.load()) {
        JSVM_Value result = nullptr;
        if (api.runScript(env, script, &result) != JSVM_OK) {
            OH_LOG_ERROR(LOG_APP, "OH_JSVM_RunScript failed");
            break;
        }
        bool pumped = false;
        (void)api.pumpMessageLoop(vm, &pumped);
        usleep(JSVM_INTERVAL_US);
    }
}

bool KmpMemDumpCallback(int32_t fd, int32_t tag, bool mayReportToOEM, const char *arg)
{
    OH_LOG_INFO(LOG_APP, "KMP mem-dump listener triggered: fd=%{public}d tag=%{public}d", fd, tag);
    if (fd < 0) {
        return false;
    }
    if (tag == MEM_LISTENER_DUMP_SNAPSHOT) {
        for (int i = 0; i < KMP_SNAPSHOT_RECORDS; i++) {
            if (dprintf(fd,
                "HidumperFuncTest synthetic KMP snapshot record %03d: "
                "objects=128 retained=4096 klass=com.example.hidumperfunctest.Target%03d\n", i, i) < 0) {
                OH_LOG_ERROR(LOG_APP, "KMP snapshot write failed at record %{public}d", i);
                return false;
            }
        }
    }
    return true;
}

int RegisterKmpListener()
{
    void *handle = dlopen("libohhidebug.so", RTLD_NOW);
    if (handle == nullptr) {
        OH_LOG_WARN(LOG_APP, "dlopen libohhidebug.so failed: %{public}s", dlerror());
        return KMP_STATE_NO_LIB;
    }
    using RegisterFunc = int (*)(const char *, bool (*)(int32_t, int32_t, bool, const char *));
    auto regFunc = reinterpret_cast<RegisterFunc>(dlsym(handle, "OH_HiDebug_RegisterMemDumpListener"));
    if (regFunc == nullptr) {
        OH_LOG_WARN(LOG_APP, "OH_HiDebug_RegisterMemDumpListener symbol absent (device < API 26?)");
        return KMP_STATE_NO_SYMBOL;
    }
    int rc = regFunc("KMP", KmpMemDumpCallback);
    OH_LOG_INFO(LOG_APP, "KMP listener registered, rc=%{public}d", rc);
    return rc;
}

napi_value Start(napi_env env, napi_callback_info info)
{
    bool expected = false;
    if (g_started.compare_exchange_strong(expected, true)) {
        std::thread(MallocWorker).detach();
        std::thread(JsvmWorker).detach();
        g_kmpState = RegisterKmpListener();
    }
    napi_value result = nullptr;
    napi_get_boolean(env, true, &result);
    return result;
}

napi_value Status(napi_env env, napi_callback_info info)
{
    std::string text = "pid=" + std::to_string(static_cast<int>(getpid())) +
        " malloc=" + std::to_string(g_mallocState.load()) +
        " jsvm=" + std::to_string(g_jsvmState.load()) +
        " kmp=" + std::to_string(g_kmpState.load());
    napi_value result = nullptr;
    napi_create_string_utf8(env, text.c_str(), NAPI_AUTO_LENGTH, &result);
    return result;
}

/*
 * Deliberate null-pointer write, invoked from the ohosTest CrashTest case so the suite can
 * provision its own cppcrash faultlog record (hidumper -e --list / --print consume those).
 * Pattern matches the faultlogger test HAP (hiviewdfx_hiview PR !4552). Does not return.
 */
napi_value TriggerSegv(napi_env env, napi_callback_info info)
{
    volatile char *crashPtr = nullptr;
    crashPtr[1] = 1;
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        { "start", nullptr, Start, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "status", nullptr, Status, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "TriggerSegv", nullptr, TriggerSegv, nullptr, nullptr, nullptr, napi_default, nullptr },
    };
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    return exports;
}
} // namespace

static napi_module g_hidumperTargetModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "hidumper_target",
    .nm_priv = reinterpret_cast<void *>(Init),
    .reserved = { nullptr },
};

extern "C" __attribute__((constructor)) void RegisterHidumperTargetModule(void)
{
    napi_module_register(&g_hidumperTargetModule);
}
