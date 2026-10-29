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
 * Implementation of the shared helpers declared in hidumper_function_test_env.h.
 */

#include "hidumper_function_test_env.h"

#include <dirent.h>
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cctype>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include <securec.h>
#include "ashmem.h"
namespace OHOS {
namespace HiviewDFX {

const std::string TARGET_ASHMEM_NAME = "HidumperFuncTestAshmem";

const std::string TARGET_THREAD_NAME = "hd_tgt_sleep";

const std::string TARGET_IO_DIR = "/data/local/tmp";

const std::string SECTION_MEMORY = "-------------------------------[memory]-------------------------------";

const std::string SECTION_PROCESSES = "-------------------------------[processes]-------------------------------";

const std::string SECTION_BASE = "-------------------------------[base]-------------------------------";

const std::string SECTION_ABILITY = "-------------------------------[ability]-------------------------------";

const std::string SECTION_NET_TRAFFIC = "-------------------------------[net traffic]-------------------------------";

const std::string SECTION_NET = "-------------------------------[net]-------------------------------";

const std::string SECTION_STORAGE = "-------------------------------[storage]-------------------------------";

const std::string SECTION_STORAGE_IO = "-------------------------------[storage io]-------------------------------";

const std::string SECTION_CPUFREQ = "-------------------------------[cpufreq]-------------------------------";

const std::string SECTION_CPUUSAGE = "-------------------------------[cpuusage]-------------------------------";

const std::string SECTION_FAULTLOG = "-------------------------------[faultlog]-------------------------------";

const std::string ERR_HEAP_MULTI_ARG =
    "Error [ARGUMENT]: Invalid ARG for '--mem-heap' (must specify exactly one ARG).";

const std::string SNAPSHOT_TEMP_DIR = "/data/log/faultlog/temp";

const std::string SNAPSHOT_LEAK_DIR = "/data/log/reliability/resource_leak/memory_leak";

const std::string HIDUMPER_OUTPUT_DIR = "/data/log/hidumper";

const std::string RENDER_PROC_SUFFIX = ":render";

const std::string TIMEOUT_BIN = "/bin/timeout";

const std::string FAMILY_JSHEAP = "jsheap";

const std::string FAMILY_LEAKLIST = "leaklist";

const std::string FAMILY_NATIVEHEAP = "nativeheap";

const std::string FAMILY_KOTLINHEAP = "kotlinheap";

const std::string FAMILY_JSVMHEAP = "jsvmheap";

const std::string FAMILY_ARKWEB_JSHEAP = "arkweb_jsheap";

const std::string RAWHEAP_EXT = ".rawheap";

const std::string RECORD_MEM_PATH = "/data/log/hidumper/record_mem.txt";

const std::string TIMES_SECTION = "times:";

const std::string ERR_MISS_MEMSMAPS = "hidumper: option pid missed. memsmaps";

const std::string ERR_MISS_MEMJSHEAP = "hidumper: option pid missed. memjsheap";

const std::string ERR_MISS_MEMCJHEAP = "hidumper: option pid missed. memcjheap";

const std::string ERR_MISS_BOGUS = "hidumper: option pid missed. bogus";

const std::string ERR_MISS_NOSUCHOPT = "hidumper: option pid missed. nosuchopt";

const std::string ERR_MISS_H = "hidumper: option pid missed. h";

const std::string ERR_FD_NEEDS_PID = "hidumper: invalid arg: --fd/--thread requires -p PID";

const std::string ERR_IPC_ARGS = "ipc stat cmd args invalid";

const std::string ERR_NO_SUCH_PROCESS = "hidumper: No such process: ";

const std::string HINT_NO_RECORDS = "no records found.";

const std::string HINT_NO_FAULTLOGS = "no faultlogs found.";

const std::string SMAPS_VERBOSE_HEADER = "Perm        Start             End";

const std::string ERR_MISS_UNKNOWN_OPT = "hidumper: option pid missed. definitelynotanoption";

const std::vector<std::string> SYSTEM_PROBE_TARGETS = {"render_service", "foundation", "hiview",
    "hilogd", "com.ohos.sceneboard", "com.ohos.launcher"};

const std::string FUNC_TEST_HAP_BUNDLE = "ohos.samples.hidumperfunc";

const std::string FUNC_TEST_HAP_ABILITY = "EntryAbility";

const std::string LEAK_PROBE_MARKER = "HidumperFuncLeakProbe";

const std::vector<std::string> FUNC_TEST_HAP_CANDIDATES = {
    "/data/test/resource/hidumper/HidumperFuncHap.hap",
    "/data/local/tmp/HidumperFuncHap.hap"
};

const std::vector<std::string> FUNC_TEST_OHOS_TEST_HAP_CANDIDATES = {
    "/data/test/resource/hidumper/HidumperFuncHapOhosTest.hap",
    "/data/local/tmp/HidumperFuncHapOhosTest.hap"
};

const std::string CRASH_TEST_CMD =
    "aa test -b ohos.samples.hidumperfunc -m entry_test "
    "-s unittest /ets/testrunner/OpenHarmonyTestRunner -s class CrashTest#SegvCase -s timeout 15000";

const std::string FAULTLOG_DIR = "/data/log/faultlog/faultlogger";

const std::string UITEST_BIN = "/system/bin/uitest";

const std::string UI_INTERACTION_CMD = "uitest uiInput swipe 600 1800 600 700 500";

const std::vector<HeapTargetHint> NATIVE_TARGETS = {
    {"com.ohos.sceneboard", "", false},
    {"com.ohos.launcher", "", false}
};

const std::vector<HeapTargetHint> KMP_TARGETS = {
    {FUNC_TEST_HAP_BUNDLE, FUNC_TEST_HAP_ABILITY, false},
    {"com.ohos.medialibrary.medialibrarydata", "", false}
};

const std::vector<HeapTargetHint> JSVM_TARGETS = {
    /* The HAP keeps its own isolate executing, so no UI interaction driver is needed for it. */
    {FUNC_TEST_HAP_BUNDLE, FUNC_TEST_HAP_ABILITY, false}
};

/*
 * Where a finished snapshot has to end up, and how it is named there. The plugin build ships the
 * companion that moves snapshots out of the staging directory into the reliability directory and
 * re-prefixes them with "hidumper-"; without the plugin nothing moves them, so the staging
 * directory is the final one. This is axis 1 only: hidumper_hiviewdfx_plugin_enable is derived from
 * the presence of the closed-source part in hidumper.gni, so the build flag already encodes the
 * product shape and no run-time product detection is needed.
 */
#ifdef HIDUMPER_HIVIEWDFX_PLUGIN_ENABLE
const std::string SNAPSHOT_STAGING_DIR = SNAPSHOT_TEMP_DIR;
const std::string SNAPSHOT_FINAL_DIR = SNAPSHOT_LEAK_DIR;
const std::string FINAL_NAME_PREFIX = "hidumper-";
#else
const std::string SNAPSHOT_STAGING_DIR = SNAPSHOT_TEMP_DIR;
const std::string SNAPSHOT_FINAL_DIR = SNAPSHOT_TEMP_DIR;
const std::string FINAL_NAME_PREFIX = "";
#endif

#if defined(__aarch64__) || defined(__x86_64__)
const std::string PLUGIN_SO_PATH_64 = "/system/lib64/libhidumperplugin_impl.z.so";
#else
const std::string PLUGIN_SO_PATH_64 = "/system/lib/libhidumperplugin_impl.z.so";
#endif

bool FileExists(const std::string &path)
{
    struct stat st = {};
    return stat(path.c_str(), &st) == 0;
}

int64_t FileSize(const std::string &path)
{
    struct stat st = {};
    if (stat(path.c_str(), &st) != 0) {
        return -1;
    }
    return static_cast<int64_t>(st.st_size);
}

bool HasTimeoutBinary()
{
    static const bool available = (access(TIMEOUT_BIN.c_str(), X_OK) == 0);
    return available;
}

CmdResult RunShell(const std::string &shellCmd, int32_t timeoutSec)
{
    CmdResult result;
    std::string wrapped = shellCmd + " 2>&1";
    if (HasTimeoutBinary() && timeoutSec > 0) {
        wrapped = std::string(TIMEOUT_BIN) + " " + std::to_string(timeoutSec) + " " + wrapped;
    }
    FILE *fp = popen(wrapped.c_str(), "r");
    if (fp == nullptr) {
        result.output = "<popen failed>";
        return result;
    }
    char buf[READ_BUF_SIZE] = {0};
    size_t got = 0;
    while ((got = fread(buf, sizeof(char), READ_BUF_SIZE - 1, fp)) > 0) {
        buf[got] = '\0';
        result.output.append(buf);
    }
    int32_t status = pclose(fp);
    result.exitCode = (status == -1) ? -1 : WEXITSTATUS(status);
    return result;
}

CmdResult RunHidumper(const std::string &args)
{
    return RunShell("hidumper " + args, CMD_TIMEOUT_SEC);
}

bool Contains(const CmdResult &result, const std::string &needle)
{
    return result.output.find(needle) != std::string::npos;
}

bool HasContentBelow(const std::string &output, const std::string &header)
{
    size_t pos = output.find(header);
    if (pos == std::string::npos) {
        return false;
    }
    size_t lineEnd = output.find('\n', pos);
    if (lineEnd == std::string::npos) {
        return false;
    }
    std::istringstream rest(output.substr(lineEnd + 1));
    std::string line;
    while (std::getline(rest, line)) {
        if (line.find_first_not_of(" \t\r") != std::string::npos) {
            return true;
        }
    }
    return false;
}

int32_t CountAnsweredAbilities(const std::string &output)
{
    int32_t answered = 0;
    size_t headerLen = std::string(SECTION_ABILITY).size();
    size_t pos = output.find(SECTION_ABILITY);
    while (pos != std::string::npos) {
        size_t next = output.find(SECTION_ABILITY, pos + headerLen);
        size_t end = (next == std::string::npos) ? output.size() : next;
        std::istringstream chunk(output.substr(pos + headerLen, end - pos - headerLen));
        std::string line;
        while (std::getline(chunk, line)) {
            size_t first = line.find_first_not_of(" \t\r");
            if (first == std::string::npos || line[first] == '-') {
                continue; /* blank or hidumper-printed banner */
            }
            ++answered;
            break;
        }
        pos = next;
    }
    return answered;
}

std::string GetSystemParam(const std::string &key)
{
    CmdResult result = RunShell("param get " + key, SHORT_CMD_TIMEOUT_SEC);
    std::string value = result.output;
    const std::string blanks = " \t\n\r";
    size_t end = value.find_last_not_of(blanks);
    if (end == std::string::npos) {
        return "";
    }
    size_t begin = value.find_first_not_of(blanks);
    return value.substr(begin, end - begin + 1);
}

std::string DescribeEnvironment()
{
    std::string desc = "pluginEnabled=";
    desc += PLUGIN_ENABLED ? "true" : "false";
    desc += ", hiviewEnabled=";
    desc += HIVIEW_ENABLED ? "true" : "false";
    desc += ", snapshotFinalDir=" + SNAPSHOT_FINAL_DIR;
    desc += ", finalNamePrefix=" + (FINAL_NAME_PREFIX.empty() ? std::string("<none>") : FINAL_NAME_PREFIX);
    desc += ", version=" + GetSystemParam("const.product.software.version");
    return desc;
}

std::vector<std::string> ListFiles(const std::string &dir)
{
    std::vector<std::string> files;
    DIR *dp = opendir(dir.c_str());
    if (dp == nullptr) {
        return files;
    }
    struct dirent *entry = nullptr;
    while ((entry = readdir(dp)) != nullptr) {
        std::string name = entry->d_name;
        if (name == "." || name == "..") {
            continue;
        }
        files.push_back(name);
    }
    closedir(dp);
    std::sort(files.begin(), files.end());
    return files;
}

int32_t CountDirEntries(const std::string &dir)
{
    return static_cast<int32_t>(ListFiles(dir).size());
}

std::vector<std::string> ListFilesContaining(const std::string &dir, const std::string &needle)
{
    std::vector<std::string> matched;
    for (const auto &name : ListFiles(dir)) {
        if (name.find(needle) != std::string::npos) {
            matched.push_back(name);
        }
    }
    return matched;
}

std::string JoinFiles(const std::vector<std::string> &files)
{
    std::string joined;
    for (const auto &name : files) {
        joined += name + " ";
    }
    return joined.empty() ? std::string("<empty>") : joined;
}

std::string ReadProcessName(int32_t pid)
{
    std::string path = "/proc/" + std::to_string(pid) + "/cmdline";
    FILE *fp = fopen(path.c_str(), "r");
    if (fp == nullptr) {
        return "";
    }
    char buf[CMDLINE_BUF_SIZE] = {0};
    size_t got = fread(buf, sizeof(char), CMDLINE_BUF_SIZE - 1, fp);
    if (fclose(fp) != 0 || got == 0) {
        return "";
    }
    buf[got] = '\0';
    return std::string(buf);
}

std::vector<int32_t> ListProcessIds()
{
    std::vector<int32_t> pids;
    DIR *dp = opendir("/proc");
    if (dp == nullptr) {
        return pids;
    }
    struct dirent *entry = nullptr;
    while ((entry = readdir(dp)) != nullptr) {
        std::string name = entry->d_name;
        if (name.empty() ||
            std::find_if(name.begin(), name.end(), [](char c) { return !isdigit(c); }) != name.end()) {
            continue;
        }
        char *end = nullptr;
        long pid = strtol(name.c_str(), &end, DECIMAL_BASE);
        if (end != name.c_str() && pid > 0) {
            pids.push_back(static_cast<int32_t>(pid));
        }
    }
    closedir(dp);
    return pids;
}

bool ReadProcStatParentAndStart(int32_t pid, int32_t &ppid, int64_t &startTime)
{
    std::string path = "/proc/" + std::to_string(pid) + "/stat";
    FILE *fp = fopen(path.c_str(), "r");
    if (fp == nullptr) {
        return false;
    }
    char buf[PROC_STAT_BUF_SIZE] = {0};
    size_t got = fread(buf, sizeof(char), PROC_STAT_BUF_SIZE - 1, fp);
    if (fclose(fp) != 0 || got == 0) {
        return false;
    }
    buf[got] = '\0';
    std::string content(buf);
    size_t commEnd = content.rfind(')');
    if (commEnd == std::string::npos) {
        return false;
    }
    std::istringstream tail(content.substr(commEnd + 1));
    std::string state;
    if (!(tail >> state >> ppid)) {
        return false;
    }
    /* Fields 5..21 sit between ppid and starttime. */
    constexpr int32_t fieldsBeforeStartTime = 17;
    std::string skip;
    for (int32_t i = 0; i < fieldsBeforeStartTime; ++i) {
        if (!(tail >> skip)) {
            return false;
        }
    }
    return static_cast<bool>(tail >> startTime);
}

int32_t ResolveMainProcess(const std::vector<int32_t> &candidates)
{
    if (candidates.empty()) {
        return NO_PID;
    }
    if (candidates.size() == 1) {
        return candidates[0];
    }
    const std::set<int32_t> candidateSet(candidates.begin(), candidates.end());
    int32_t best = NO_PID;
    int64_t bestStart = 0;
    for (int32_t pid : candidates) {
        int32_t ppid = 0;
        int64_t startTime = 0;
        if (!ReadProcStatParentAndStart(pid, ppid, startTime)) {
            continue; /* already gone: a transient snapshot child mid-exit */
        }
        if (candidateSet.count(ppid) > 0) {
            continue; /* fork child of another candidate */
        }
        if (best == NO_PID || startTime < bestStart) {
            best = pid;
            bestStart = startTime;
        }
    }
    return best;
}

int64_t ReadResidentPages(int32_t pid)
{
    std::string path = "/proc/" + std::to_string(pid) + "/stat";
    FILE *fp = fopen(path.c_str(), "r");
    if (fp == nullptr) {
        return 0;
    }
    char buf[PROC_STAT_BUF_SIZE] = {0};
    size_t got = fread(buf, sizeof(char), PROC_STAT_BUF_SIZE - 1, fp);
    if (fclose(fp) != 0 || got == 0) {
        return 0;
    }
    buf[got] = '\0';
    std::string stat(buf);
    size_t commEnd = stat.rfind(')');
    if (commEnd == std::string::npos) {
        return 0;
    }
    std::istringstream tail(stat.substr(commEnd + 1));
    std::string token;
    int64_t value = 0;
    constexpr int32_t firstFieldAfterComm = 3;
    constexpr int32_t rssField = 24;
    for (int32_t field = firstFieldAfterComm; field <= rssField; ++field) {
        if (!(tail >> token)) {
            return 0;
        }
        if (field == rssField) {
            value = strtoll(token.c_str(), nullptr, DECIMAL_BASE);
        }
    }
    return value;
}

int32_t FindRenderProcessPid()
{
    std::vector<int32_t> matches;
    for (int32_t pid : ListProcessIds()) {
        std::string name = ReadProcessName(pid);
        if (name.size() > RENDER_PROC_SUFFIX.size() &&
            name.compare(name.size() - RENDER_PROC_SUFFIX.size(), RENDER_PROC_SUFFIX.size(), RENDER_PROC_SUFFIX) == 0) {
            matches.push_back(pid);
        }
    }
    return ResolveMainProcess(matches);
}

int32_t GetPidByName(const std::string &processName)
{
    CmdResult result = RunShell("pidof " + processName, SHORT_CMD_TIMEOUT_SEC);
    std::vector<int32_t> candidates;
    std::istringstream iss(result.output);
    std::string token;
    while (iss >> token) {
        int32_t pid = static_cast<int32_t>(strtol(token.c_str(), nullptr, DECIMAL_BASE));
        if (pid > 0) {
            candidates.push_back(pid);
        }
    }
    return ResolveMainProcess(candidates);
}

int32_t PickUiProcessPid(std::string &processName)
{
    const std::vector<std::string> candidates = PLUGIN_ENABLED
        ? std::vector<std::string> {"com.ohos.sceneboard", "com.ohos.launcher"}
        : std::vector<std::string> {"com.ohos.launcher", "com.ohos.sceneboard"};
    for (const auto &name : candidates) {
        int32_t pid = GetPidByName(name);
        if (pid > 0) {
            processName = name;
            return pid;
        }
    }
    processName = "";
    return NO_PID;
}

int32_t ParsePrefixedNumber(const std::string &output, const std::string &prefix)
{
    size_t pos = output.find(prefix);
    if (pos == std::string::npos) {
        return NO_PID;
    }
    return static_cast<int32_t>(strtol(output.c_str() + pos + prefix.size(), nullptr, DECIMAL_BASE));
}

std::set<std::string> CollectFiles(const std::vector<std::string> &dirs, const std::string &needle)
{
    std::set<std::string> paths;
    for (const auto &dir : dirs) {
        for (const auto &name : ListFilesContaining(dir, needle)) {
            paths.insert(dir + "/" + name);
        }
    }
    return paths;
}

std::set<std::string> WithoutPreexisting(const std::set<std::string> &current,
                                         const std::set<std::string> &preexisting)
{
    std::set<std::string> fresh;
    for (const auto &path : current) {
        if (preexisting.find(path) == preexisting.end()) {
            fresh.insert(path);
        }
    }
    return fresh;
}

bool WaitNewSnapshotSettled(const std::vector<std::string> &dirs, const std::string &needle,
                            const std::set<std::string> &preexisting, int32_t timeoutSec,
                            std::string &foundPath)
{
    SnapshotSettleTracker tracker;
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(timeoutSec);
    while (std::chrono::steady_clock::now() < deadline) {
        for (const auto &path : WithoutPreexisting(CollectFiles(dirs, needle), preexisting)) {
            if (tracker.Update(path)) {
                foundPath = path;
                return true;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(POLL_INTERVAL_MS));
    }
    return false;
}

bool WaitNoNewFiles(const std::vector<std::string> &dirs, const std::string &needle,
                    const std::set<std::string> &preexisting, int32_t timeoutSec)
{
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(timeoutSec);
    while (std::chrono::steady_clock::now() < deadline) {
        if (WithoutPreexisting(CollectFiles(dirs, needle), preexisting).empty()) {
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(POLL_INTERVAL_MS));
    }
    return WithoutPreexisting(CollectFiles(dirs, needle), preexisting).empty();
}

void WaitSnapshotQuiescent(const std::vector<std::string> &dirs, const std::string &needle,
                           int32_t quietSec, int32_t timeoutSec)
{
    std::map<std::string, int64_t> previous;
    int32_t quietRounds = 0;
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(timeoutSec);
    while (std::chrono::steady_clock::now() < deadline && quietRounds < quietSec) {
        std::map<std::string, int64_t> current;
        for (const auto &path : CollectFiles(dirs, needle)) {
            current[path] = FileSize(path);
        }
        quietRounds = (current == previous) ? (quietRounds + 1) : 0;
        previous = current;
        std::this_thread::sleep_for(std::chrono::milliseconds(POLL_INTERVAL_MS));
    }
}

std::string DirListingForReport()
{
    return "staging(" + SNAPSHOT_STAGING_DIR + ")=[" + JoinFiles(ListFiles(SNAPSHOT_STAGING_DIR)) +
           "] final(" + SNAPSHOT_FINAL_DIR + ")=[" + JoinFiles(ListFiles(SNAPSHOT_FINAL_DIR)) + "]";
}

std::string ArtifactNeedle(const std::string &family, int32_t pid)
{
    return family + "-" + std::to_string(pid) + "-";
}

ArtifactCheck VerifySnapshotArtifact(const std::string &hidumperArgs, const std::string &family,
                                     int32_t pid, int32_t timeoutSec,
                                     const std::string &stagingNeedle)
{
    ArtifactCheck check;
    const std::string needle = ArtifactNeedle(family, pid);
    const std::string stageNeedle = stagingNeedle.empty() ? needle : stagingNeedle;
    SnapshotCleanupGuard guard(needle);
    std::unique_ptr<SnapshotCleanupGuard> stagingGuard;
    if (stageNeedle != needle) {
        stagingGuard = std::make_unique<SnapshotCleanupGuard>(stageNeedle);
    }
    WaitSnapshotQuiescent({SNAPSHOT_STAGING_DIR, SNAPSHOT_FINAL_DIR}, stageNeedle,
                          QUIESCENT_ROUNDS, QUIESCENT_TIMEOUT_SEC);
    const std::set<std::string> beforeFinal = CollectFiles({SNAPSHOT_FINAL_DIR}, needle);
    const std::set<std::string> beforeStaging = CollectFiles({SNAPSHOT_STAGING_DIR}, stageNeedle);

    CmdResult result = RunHidumper(hidumperArgs);
    if (Contains(result, "Error [") || Contains(result, "No such process") ||
        Contains(result, "option pid missed")) {
        check.detail = "command reported a problem: " + result.output;
        return check;
    }

    if (WaitNewSnapshotSettled({SNAPSHOT_FINAL_DIR}, needle, beforeFinal, timeoutSec, check.path)) {
        check.produced = true;
        int64_t size = FileSize(check.path);
        if (size <= static_cast<int64_t>(MIN_SNAPSHOT_BYTES)) {
            check.detail = "artifact " + check.path + " is only " + std::to_string(size) + " bytes";
            return check;
        }
        check.size = size;
        /* Per the review decision the artifact contract stops at "the final file is generated
         * in the build's final directory with a sane size": the companion's migration details
         * (hidumper- prefix, staging copy removal) are closed-source behavior, environment
         * dependent, and no longer asserted. */
        check.verified = true;
        check.detail = check.path + " size=" + std::to_string(size);
        return check;
    }

    std::string stagedPath;
    if (SNAPSHOT_STAGING_DIR != SNAPSHOT_FINAL_DIR &&
        WaitNewSnapshotSettled({SNAPSHOT_STAGING_DIR}, stageNeedle, beforeStaging,
                               NEGATIVE_WAIT_SEC, stagedPath)) {
        check.produced = true;
        check.detail = "artifact " + stagedPath + " stayed in the staging directory and was never "
                       "moved to " + SNAPSHOT_FINAL_DIR + "; " + DescribeEnvironment();
        return check;
    }
    check.detail = "no " + family + " artifact for pid " + std::to_string(pid) + " within " +
                   std::to_string(timeoutSec) + "s; " + DirListingForReport();
    return check;
}

int32_t GetMainPidOfRender(int32_t renderPid)
{
    std::string name = ReadProcessName(renderPid);
    if (name.size() <= RENDER_PROC_SUFFIX.size() ||
        name.compare(name.size() - RENDER_PROC_SUFFIX.size(), RENDER_PROC_SUFFIX.size(),
                     RENDER_PROC_SUFFIX) != 0) {
        return NO_PID;
    }
    return GetPidByName(name.substr(0, name.size() - RENDER_PROC_SUFFIX.size()));
}

std::atomic<bool> g_targetChildStop { false };

/*
 * Escape hatch for the child's heap block: without an observable owner the compiler treats the
 * malloc+touch sequence as dead stores and removes it entirely (verified: the child showed a 1.5 MB
 * Total Pss instead of 33 MB). The volatile sink plus per-page volatile writes keep it material.
 */
volatile void *g_targetChildHeap = nullptr;

void TargetChildSignalHandler(int)
{
    _exit(0);
}

int32_t CountSelfFdIncludingScanner()
{
    int32_t count = 0;
    DIR *dir = opendir("/proc/self/fd");
    if (dir == nullptr) {
        return -1;
    }
    struct dirent *entry = nullptr;
    while ((entry = readdir(dir)) != nullptr) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        ++count;
    }
    closedir(dir);
    return count;
}

void CloseInheritedFds(int reportFd, int lifelineFd)
{
    long maxFd = sysconf(_SC_OPEN_MAX);
    if (maxFd <= 0 || maxFd > MAX_FD_SWEEP) {
        maxFd = MAX_FD_SWEEP;
    }
    for (long fd = 3; fd < maxFd; ++fd) {
        if (static_cast<int>(fd) != reportFd && static_cast<int>(fd) != lifelineFd) {
            (void)close(static_cast<int>(fd));
        }
    }
}

void SpawnTargetThreads(std::vector<std::thread> &threads)
{
    for (int32_t i = 0; i < TARGET_SLEEPER_THREADS; ++i) {
        threads.emplace_back([]() {
            (void)pthread_setname_np(pthread_self(), TARGET_THREAD_NAME.c_str());
            while (!g_targetChildStop) {
                usleep(200 * 1000); // NOLINT: poll interval
            }
        });
    }
    threads.emplace_back([]() {
        volatile uint64_t accumulator = 0;
        while (!g_targetChildStop) {
            for (int32_t i = 0; i < 300000; ++i) { // NOLINT: work quantum
                accumulator += static_cast<uint64_t>(i);
            }
            usleep(100 * 1000); // NOLINT: half duty
        }
    });
}

std::vector<int> CreateHeldPipes()
{
    std::vector<int> heldFds;
    for (int32_t i = 0; i < TARGET_PIPE_PAIRS; ++i) {
        int pipeFds[2] = { -1, -1 };
        if (pipe(pipeFds) == 0) {
            heldFds.push_back(pipeFds[0]);
            heldFds.push_back(pipeFds[1]);
        }
    }
    return heldFds;
}

void AllocateBallastHeap()
{
    void *heap = malloc(TARGET_HEAP_BYTES);
    if (heap == nullptr) {
        return;
    }
    g_targetChildHeap = heap;
    volatile char *touched = static_cast<volatile char *>(heap);
    for (size_t offset = 0; offset < TARGET_HEAP_BYTES; offset += PAGE_STRIDE_BYTES) {
        touched[offset] = 0x5A;
    }
}

int32_t CreateTouchedAshmem(sptr<Ashmem> &region)
{
    region = Ashmem::CreateAshmem(TARGET_ASHMEM_NAME.c_str(), TARGET_ASHMEM_BYTES);
    if (region == nullptr) {
        return -1;
    }
    int32_t ashmemFd = region->GetAshmemFd();
    void *mapped = mmap(nullptr, TARGET_ASHMEM_BYTES, PROT_READ | PROT_WRITE, MAP_SHARED, ashmemFd, 0);
    if (mapped != MAP_FAILED) {
        volatile char *ashmemPage = static_cast<volatile char *>(mapped);
        for (size_t offset = 0; offset < static_cast<size_t>(TARGET_ASHMEM_BYTES);
             offset += PAGE_STRIDE_BYTES) {
            ashmemPage[offset] = 0x3C;
        }
    }
    return ashmemFd;
}

void SendChildHandshake(int reportFd, const TargetChildReport &report)
{
    char handshake[96] = { 0 };
    int written = snprintf_s(handshake, sizeof(handshake), sizeof(handshake) - 1,
                             "fd=%d thread=%d ashmem=%d\n",
                             report.fdCount, report.threadCount, report.ashmemFd);
    if (written > 0) {
        ssize_t ignored = write(reportFd, handshake, static_cast<size_t>(written));
        (void)ignored;
    }
    (void)close(reportFd);
}

void RunChildDutyLoop(int lifelineFd, int ioFd)
{
    const std::vector<char> chunk(TARGET_IO_CHUNK_BYTES, 'x');
    while (!g_targetChildStop) {
        struct pollfd lifelinePoll = { lifelineFd, POLLIN, 0 };
        int ready = poll(&lifelinePoll, 1, 500); // NOLINT: io churn interval
        if (ready > 0) {
            char drain[16] = { 0 };
            ssize_t received = read(lifelineFd, drain, sizeof(drain));
            if (received <= 0) {
                break; // lifeline closed by the parent
            }
        }
        if (ioFd >= 0) {
            (void)lseek(ioFd, 0, SEEK_SET);
            ssize_t ignored = write(ioFd, chunk.data(), chunk.size());
            (void)ignored;
        }
    }
}

void TargetChildMain(int reportFd, int lifelineFd, const std::string &ioPath)
{
    (void)signal(SIGTERM, TargetChildSignalHandler);
    (void)signal(SIGINT, TargetChildSignalHandler);
    g_targetChildStop = false;
    CloseInheritedFds(reportFd, lifelineFd);

    std::vector<std::thread> threads;
    SpawnTargetThreads(threads);
    std::vector<int> heldFds = CreateHeldPipes();
    AllocateBallastHeap();
    sptr<Ashmem> region;
    int32_t ashmemFd = CreateTouchedAshmem(region);
    int ioFd = open(ioPath.c_str(), O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

    int32_t scannedFds = CountSelfFdIncludingScanner();
    TargetChildReport report;
    report.fdCount = (scannedFds > CHILD_TRANSIENT_FDS) ? (scannedFds - CHILD_TRANSIENT_FDS) : -1;
    report.threadCount = CountDirEntries("/proc/self/task");
    report.ashmemFd = ashmemFd;
    SendChildHandshake(reportFd, report);

    RunChildDutyLoop(lifelineFd, ioFd);
    g_targetChildStop = true;
    for (auto &worker : threads) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    _exit(0);
}

bool ReadTargetChildReport(int reportFd, TargetChildReport &report)
{
    struct pollfd reportPoll = { reportFd, POLLIN, 0 };
    int ready = poll(&reportPoll, 1, TARGET_READY_TIMEOUT_SEC * 1000);
    if (ready <= 0) {
        return false;
    }
    char buf[128] = { 0 };
    ssize_t received = read(reportFd, buf, sizeof(buf) - 1);
    if (received <= 0) {
        return false;
    }
    buf[received] = '\0';
    const char *fdField = strstr(buf, "fd=");
    const char *threadField = strstr(buf, "thread=");
    const char *ashmemField = strstr(buf, "ashmem=");
    if (fdField == nullptr || threadField == nullptr || ashmemField == nullptr) {
        return false;
    }
    report.fdCount = static_cast<int32_t>(strtol(fdField + 3, nullptr, DECIMAL_BASE));     // NOLINT: "fd="
    report.threadCount = static_cast<int32_t>(strtol(threadField + 7, nullptr, DECIMAL_BASE)); // NOLINT: "thread="
    report.ashmemFd = static_cast<int32_t>(strtol(ashmemField + 7, nullptr, DECIMAL_BASE));    // NOLINT: "ashmem="
    return report.fdCount > 0 && report.threadCount > 0;
}

int64_t RowFirstNumber(const std::string &output, const std::string &rowLabel)
{
    size_t lineStart = 0;
    while (lineStart <= output.size()) {
        size_t lineEnd = output.find('\n', lineStart);
        std::string line = output.substr(lineStart, lineEnd == std::string::npos ? std::string::npos
                                                                                 : lineEnd - lineStart);
        size_t first = line.find_first_not_of(" \t");
        if (first != std::string::npos && line.compare(first, rowLabel.size(), rowLabel) == 0 &&
            (line.size() == first + rowLabel.size() ||
             isspace(static_cast<unsigned char>(line[first + rowLabel.size()])) != 0)) {
            char *end = nullptr;
            long long value = strtoll(line.c_str() + first + rowLabel.size(), &end, DECIMAL_BASE);
            if (end != line.c_str() + first + rowLabel.size()) {
                return static_cast<int64_t>(value);
            }
        }
        if (lineEnd == std::string::npos) {
            break;
        }
        lineStart = lineEnd + 1;
    }
    return -1;
}

bool ParseSummaryLine(const std::string &line, int64_t &sizeKb, int64_t &rssKb, int64_t &pssKb)
{
    int64_t values[SUMMARY_COLUMN_COUNT] = { -1, -1, -1 };
    std::istringstream stream(line);
    for (int32_t index = 0; index < SUMMARY_COLUMN_COUNT; ++index) {
        if (!(stream >> values[index])) {
            return false;
        }
    }
    sizeKb = values[0];
    rssKb = values[1];
    pssKb = values[SUMMARY_PSS_INDEX];
    return true;
}

bool SummaryRowNumbers(const std::string &output, int64_t &sizeKb, int64_t &rssKb, int64_t &pssKb)
{
    std::istringstream stream(output);
    std::string line;
    while (std::getline(stream, line)) {
        if (line.find("Summary") != std::string::npos) {
            return ParseSummaryLine(line, sizeKb, rssKb, pssKb);
        }
    }
    return false;
}

bool IsFuncTestHapInstalled()
{
    CmdResult dump = RunShell("bm dump -n " + FUNC_TEST_HAP_BUNDLE, SHORT_CMD_TIMEOUT_SEC);
    return Contains(dump, FUNC_TEST_HAP_BUNDLE) && !Contains(dump, "error:");
}

std::string FirstExistingFile(const std::vector<std::string> &paths)
{
    for (const auto &path : paths) {
        if (access(path.c_str(), R_OK) == 0) {
            return path;
        }
    }
    return "";
}

void EnsureFuncTestHapInstalled()
{
    if (IsFuncTestHapInstalled()) {
        return;
    }
    std::string mainHap = FirstExistingFile(FUNC_TEST_HAP_CANDIDATES);
    if (mainHap.empty()) {
        return;
    }
    (void)RunShell("bm install -p " + mainHap, SHORT_CMD_TIMEOUT_SEC);
    std::string testHap = FirstExistingFile(FUNC_TEST_OHOS_TEST_HAP_CANDIDATES);
    if (!testHap.empty()) {
        (void)RunShell("bm install -p " + testHap, SHORT_CMD_TIMEOUT_SEC);
    }
}

bool FuncTestHapAvailable()
{
    return IsFuncTestHapInstalled() || !FirstExistingFile(FUNC_TEST_HAP_CANDIDATES).empty();
}

int32_t StartFuncTestHap()
{
    /* aa start occasionally competes with heavy system workload (an in-flight heap dump
     * migration was observed to fail it transiently). Retry a few times and also probe the
     * pid after a failed attempt - the app may already be up - so that only a genuinely
     * broken environment ever returns NO_PID. */
    for (int32_t attempt = 1; attempt <= HAP_START_ATTEMPTS; ++attempt) {
        CmdResult start =
            RunShell("aa start -b " + FUNC_TEST_HAP_BUNDLE + " -a " + FUNC_TEST_HAP_ABILITY,
                     SHORT_CMD_TIMEOUT_SEC);
        if (!Contains(start, "successfully")) {
            int32_t running = GetPidByName(FUNC_TEST_HAP_BUNDLE);
            if (running > 0) {
                return running;
            }
            if (attempt < HAP_START_ATTEMPTS) {
                std::this_thread::sleep_for(std::chrono::seconds(HAP_START_RETRY_SEC));
            }
            continue;
        }
        const auto deadline =
            std::chrono::steady_clock::now() + std::chrono::seconds(HAP_START_WAIT_SEC);
        int32_t pid = GetPidByName(FUNC_TEST_HAP_BUNDLE);
        while (pid <= 0 && std::chrono::steady_clock::now() < deadline) {
            std::this_thread::sleep_for(std::chrono::milliseconds(POLL_INTERVAL_MS));
            pid = GetPidByName(FUNC_TEST_HAP_BUNDLE);
        }
        return pid;
    }
    return NO_PID;
}

/* Ability-manager transient-start rejection hint (error 10106101); it advises to wait. */
const std::string ABILITY_BUSY_HINT = "Another ability is being started";

CmdResult StartAbilityWithBusyRetry(const std::string &bundle, const std::string &ability,
                                    const std::string &extraArgs)
{
    CmdResult result;
    for (int32_t attempt = 1; attempt <= HAP_START_ATTEMPTS; ++attempt) {
        result = RunShell("aa start -b " + bundle + " -a " + ability + extraArgs,
                          SHORT_CMD_TIMEOUT_SEC);
        if (Contains(result, "successfully") || !Contains(result, ABILITY_BUSY_HINT)) {
            return result;
        }
        std::this_thread::sleep_for(std::chrono::seconds(HAP_START_RETRY_SEC));
    }
    return result;
}

CmdResult StartHapAction(const std::string &action)
{
    return StartAbilityWithBusyRetry(FUNC_TEST_HAP_BUNDLE, FUNC_TEST_HAP_ABILITY,
                                     " --ps hdaction " + action);
}

int32_t StartAppAndWaitRenderPid(const std::string &bundle, const std::string &ability)
{
    CmdResult start = StartAbilityWithBusyRetry(bundle, ability);
    if (!Contains(start, "successfully")) {
        return NO_PID;
    }
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(APP_START_WAIT_SEC);
    int32_t renderPid = NO_PID;
    while (renderPid <= 0 && std::chrono::steady_clock::now() < deadline) {
        std::this_thread::sleep_for(std::chrono::milliseconds(POLL_INTERVAL_MS));
        renderPid = FindRenderProcessPid();
    }
    return renderPid;
}

bool ProvisionFaultRecord()
{
    CmdResult list = RunHidumper("-e --list");
    if (!Contains(list, "no records found")) {
        return true;
    }
    /* One full provisioning attempt per suite run: images without the hisysevent fault
     * pipeline never gain records, and repeating the crash plus the wait in every -e case
     * would triple the cost for the same verified answer. */
    static bool pipelineProbedAbsent = false;
    if (pipelineProbedAbsent) {
        return false;
    }
    if (!IsFuncTestHapInstalled()) {
        return false;
    }
    (void)RunShell(CRASH_TEST_CMD, AA_TEST_TIMEOUT_SEC);
    const auto deadline =
        std::chrono::steady_clock::now() + std::chrono::seconds(RECORD_PROVISION_TIMEOUT_SEC);
    while (std::chrono::steady_clock::now() < deadline) {
        list = RunHidumper("-e --list");
        if (!Contains(list, "no records found")) {
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(POLL_INTERVAL_MS));
    }
    pipelineProbedAbsent = true;
    return false;
}

bool EnsureFaultLogFile()
{
    if (!ListFilesContaining(FAULTLOG_DIR, "cppcrash").empty() ||
        !ListFilesContaining(FAULTLOG_DIR, "jscrash").empty() ||
        !ListFilesContaining(FAULTLOG_DIR, "appfreeze").empty()) {
        return true;
    }
    if (!IsFuncTestHapInstalled()) {
        return false;
    }
    (void)RunShell(CRASH_TEST_CMD, AA_TEST_TIMEOUT_SEC);
    const auto deadline =
        std::chrono::steady_clock::now() + std::chrono::seconds(RECORD_PROVISION_TIMEOUT_SEC);
    while (std::chrono::steady_clock::now() < deadline) {
        if (!ListFilesContaining(FAULTLOG_DIR, "cppcrash").empty()) {
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(POLL_INTERVAL_MS));
    }
    return false;
}

bool FindCapableHeapTarget(const std::string &argFlag, const std::string &family,
                           const std::vector<HeapTargetHint> &candidates, int32_t &pid,
                           std::string &processName, ArtifactCheck &check)
{
    for (const auto &hint : candidates) {
        if (!hint.mainAbility.empty()) {
            (void)StartAbilityWithBusyRetry(hint.bundleName, hint.mainAbility);
            std::this_thread::sleep_for(std::chrono::seconds(APP_FOREGROUND_WAIT_SEC));
        }
        int32_t candidate = GetPidByName(hint.bundleName);
        if (candidate <= 0) {
            continue;
        }
        std::unique_ptr<UiInteractionDriver> driver;
        if (hint.needsUiInteraction) {
            driver = std::make_unique<UiInteractionDriver>();
        }
        check = VerifySnapshotArtifact("--mem-heap " + std::to_string(candidate) + " " + argFlag,
                                       family, candidate, TARGET_PROBE_TIMEOUT_SEC);
        if (check.produced) {
            pid = candidate;
            processName = hint.bundleName;
            return true;
        }
    }
    return false;
}

int32_t CountLines(const std::string &text)
{
    int32_t lines = 0;
    for (char c : text) {
        if (c == '\n') {
            ++lines;
        }
    }
    return lines;
}

int32_t CountOccurrences(const std::string &text, const std::string &needle)
{
    if (needle.empty()) {
        return 0;
    }
    int32_t count = 0;
    size_t pos = text.find(needle);
    while (pos != std::string::npos) {
        ++count;
        pos = text.find(needle, pos + needle.size());
    }
    return count;
}

void StartBackground(const std::string &shellCmd)
{
    std::string wrapped = "sh -c '" + shellCmd + " >/dev/null 2>&1 &'";
    FILE *fp = popen(wrapped.c_str(), "r");
    if (fp != nullptr) {
        (void)pclose(fp);
    }
}

std::string ReadFileContent(const std::string &path)
{
    std::string content;
    FILE *fp = fopen(path.c_str(), "r");
    if (fp == nullptr) {
        return content;
    }
    char buf[READ_BUF_SIZE] = {0};
    size_t got = 0;
    while ((got = fread(buf, sizeof(char), READ_BUF_SIZE - 1, fp)) > 0) {
        buf[got] = '\0';
        content.append(buf);
    }
    if (fclose(fp) != 0) {
        content.clear();
    }
    return content;
}

const std::string EMPTY_WINDOW_TS = "2026-01-01 00:00:00";

std::vector<std::string> SplitTokens(const std::string &line)
{
    std::vector<std::string> tokens;
    std::istringstream iss(line);
    std::string token;
    while (iss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

std::vector<std::string> CollectListDataRows(const std::string &output)
{
    std::vector<std::string> rows;
    std::istringstream iss(output);
    std::string line;
    while (std::getline(iss, line)) {
        if (SplitTokens(line).size() >= LIST_ROW_COLUMNS) {
            rows.push_back(line);
        }
    }
    return rows;
}

std::string ListRowColumn(const std::string &row, size_t index)
{
    const std::vector<std::string> tokens = SplitTokens(row);
    return index < tokens.size() ? tokens[index] : std::string();
}

bool ProbeBundleFaultlogRows(std::string &recordId, CmdResult &printed)
{
    const std::vector<std::string> rows = CollectListDataRows(RunHidumper("-e --list").output);
    for (const auto &row : rows) {
        if (ListRowColumn(row, LIST_ROW_NAME_COLUMN).find(FUNC_TEST_HAP_BUNDLE) ==
            std::string::npos) {
            continue;
        }
        CmdResult result = RunHidumper("-e --print " + ListRowColumn(row, LIST_ROW_RECORD_ID_COLUMN));
        if (Contains(result, SECTION_FAULTLOG)) {
            recordId = ListRowColumn(row, LIST_ROW_RECORD_ID_COLUMN);
            printed = result;
            return true;
        }
    }
    return false;
}

bool FindBundleFaultlogRecord(std::string &recordId, CmdResult &printed)
{
    if (ProbeBundleFaultlogRows(recordId, printed)) {
        return true;
    }
    /* No live record: trigger the crash explicitly and keep probing within the deadline. */
    if (!IsFuncTestHapInstalled()) {
        return false;
    }
    (void)RunShell(CRASH_TEST_CMD, AA_TEST_TIMEOUT_SEC);
    const auto deadline = std::chrono::steady_clock::now() +
        std::chrono::seconds(RECORD_PROVISION_TIMEOUT_SEC);
    while (std::chrono::steady_clock::now() < deadline) {
        if (ProbeBundleFaultlogRows(recordId, printed)) {
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(POLL_INTERVAL_MS));
    }
    return false;
}

std::string FirstLongDigitRun(const std::string &line, size_t minDigits)
{
    std::string digits;
    for (char c : line) {
        if (isdigit(c) != 0) {
            digits += c;
        } else if (digits.size() >= minDigits) {
            return digits;
        } else {
            digits.clear();
        }
    }
    return digits.size() >= minDigits ? digits : std::string();
}

std::string ParseFirstRecordId(const std::string &listOutput)
{
    constexpr size_t minRecordIdDigits = 15;
    std::istringstream stream(listOutput);
    std::string line;
    bool firstLine = true;
    while (std::getline(stream, line)) {
        if (!firstLine) {
            std::string digits = FirstLongDigitRun(line, minRecordIdDigits);
            if (!digits.empty()) {
                return digits;
            }
        }
        firstLine = false;
    }
    return "";
}

} // namespace HiviewDFX
} // namespace OHOS
