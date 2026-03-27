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

#include "dumper_plugin.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace HiviewDFX {
using OnCreatePluginFuncPtr = Plugin* (*)();
static void *g_handle = nullptr;
static OnCreatePluginFuncPtr g_createPluginFn = nullptr;

int32_t LoadPlugin()
{
    g_handle = dlopen(PLUGIN_SO_PATH, RTLD_LAZY);
    if (g_handle == nullptr) {
        DUMPER_HILOGE(MODULE_SERVICE, "load plugin failed.");
        return PLUGIN_DISABLE;
    } else {
        dlerror();
        g_createPluginFn = reinterpret_cast<OnCreatePluginFuncPtr>(dlsym(g_handle, CREATE_PLUGIN));
        const char* error = dlerror();
        if (error != nullptr) {
            DUMPER_HILOGE(MODULE_SERVICE, "dlsym failed: %s", error);
            dlclose(g_handle);
            g_handle = nullptr;
            return PLUGIN_DISABLE;
        }
        DUMPER_HILOGI(MODULE_SERVICE, "load plugin success.");
        return PLUGIN_ENABLE;
    }
}

void UnloadPlugin()
{
    if (g_handle != nullptr) {
        dlclose(g_handle);
        g_handle = nullptr;
        g_createPluginFn = nullptr;
        DUMPER_HILOGI(MODULE_SERVICE, "unload plugin success.");
    }
}

bool QueryMemInfo(DumpMeminfo infos[], int& realSize)
{
    Plugin* plugin = nullptr;
    if (g_createPluginFn != nullptr) {
        DUMPER_HILOGI(MODULE_SERVICE, "QueryMemInfo get plugin.");
        plugin = g_createPluginFn();
    }
    if (plugin != nullptr && plugin->queryMemInfo != nullptr) {
        plugin->queryMemInfo(infos, realSize);
        return true;
    } else {
        DUMPER_HILOGE(MODULE_SERVICE, "QueryMemInfo default do nothing.");
        realSize = 0;
        return false;
    }
}
} // namespace HiviewDFX
} // namespace OHOS