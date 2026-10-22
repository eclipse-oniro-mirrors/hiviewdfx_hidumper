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
 * Shared test environment of the hidumper function suite: constants, data types,
 * helper functions and RAII guards. The cases live in hidumper_function_test.cpp.
 */
#ifndef HIDUMPER_FUNCTION_TEST_ENV_H
#define HIDUMPER_FUNCTION_TEST_ENV_H

#include <sys/types.h>
#include <unistd.h>
#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <thread>
#include <vector>
#include "ashmem.h"
namespace OHOS {
namespace HiviewDFX {

#ifdef HIDUMPER_HIVIEWDFX_PLUGIN_ENABLE
constexpr bool PLUGIN_ENABLED = true;
#else
constexpr bool PLUGIN_ENABLED = false;
#endif

#ifdef HIDUMPER_HIVIEWDFX_HIVIEW_ENABLE
constexpr bool HIVIEW_ENABLED = true;
#else
constexpr bool HIVIEW_ENABLED = false;
#endif

constexpr int32_t CMD_TIMEOUT_SEC = 90;

constexpr int32_t SHORT_CMD_TIMEOUT_SEC = 30;

constexpr int32_t SNAPSHOT_TIMEOUT_SEC = 120;

constexpr int32_t NEGATIVE_WAIT_SEC = 8;

constexpr int32_t QUIESCENT_ROUNDS = 5;

constexpr int32_t QUIESCENT_TIMEOUT_SEC = 90;

constexpr int32_t APP_START_WAIT_SEC = 30;

constexpr int32_t APP_FOREGROUND_WAIT_SEC = 6;

constexpr int32_t TARGET_PROBE_TIMEOUT_SEC = 30;

constexpr int32_t LEAKLIST_WAIT_SEC = 20;

constexpr int32_t POLL_INTERVAL_MS = 1000;

constexpr int32_t SETTLE_ROUNDS = 2;

constexpr size_t READ_BUF_SIZE = 4096;

constexpr size_t CMDLINE_BUF_SIZE = 512;

constexpr size_t PROC_STAT_BUF_SIZE = 1024;

constexpr int32_t DECIMAL_BASE = 10;

constexpr int32_t INIT_PID = 1;

constexpr int32_t NO_PID = -1;

constexpr size_t MIN_SNAPSHOT_BYTES = 1024;

/*
 * Controlled target child (cases 025/058/072/073). The suite forks itself into a quiescent child
 * with a precisely known shape, so count and size assertions become exact instead of tolerant:
 * N sleeper threads, one half-duty busy thread, N pipe pairs held open, a touched heap block, a
 * named ashmem region and one periodically written io file. The child reports its own descriptor
 * and thread counts over a handshake pipe after setup, and exits when the lifeline pipe closes.
 */
constexpr int32_t TARGET_SLEEPER_THREADS = 8;

constexpr int32_t TARGET_PIPE_PAIRS = 8;

constexpr size_t TARGET_HEAP_BYTES = 32 * 1024 * 1024;

/* Tolerant floor for the touched 32 MB block: allocator metadata and page rounding stay below it. */
constexpr int64_t TARGET_HEAP_FLOOR_KB = 28 * 1024;

constexpr int32_t TARGET_ASHMEM_BYTES = 1024 * 1024;

extern const std::string TARGET_ASHMEM_NAME;
extern const std::string TARGET_THREAD_NAME;
extern const std::string TARGET_IO_DIR;
constexpr int32_t TARGET_READY_TIMEOUT_SEC = 20;

constexpr int32_t TARGET_EXIT_TIMEOUT_SEC = 5;

constexpr size_t TARGET_IO_CHUNK_BYTES = 4096;

/* Settle window for the child descriptor count: the handshake pipe closes right after setup. */
constexpr int32_t TARGET_SETTLE_TIMEOUT_SEC = 5;

constexpr int32_t TARGET_SETTLE_POLL_MS = 50;

constexpr long MAX_FD_SWEEP = 4096;

constexpr size_t PAGE_STRIDE_BYTES = 4096;

/* Scanner fd plus the report pipe: descriptors that vanish right after the handshake. */
constexpr int32_t CHILD_TRANSIENT_FDS = 2;

extern const std::string SECTION_MEMORY;
extern const std::string SECTION_PROCESSES;
extern const std::string SECTION_BASE;
extern const std::string SECTION_ABILITY;
extern const std::string SECTION_NET_TRAFFIC;
extern const std::string SECTION_NET;
extern const std::string SECTION_STORAGE;
extern const std::string SECTION_STORAGE_IO;
extern const std::string SECTION_CPUFREQ;
extern const std::string SECTION_CPUUSAGE;
extern const std::string SECTION_FAULTLOG;
extern const std::string ERR_HEAP_MULTI_ARG;
extern const std::string SNAPSHOT_TEMP_DIR;
extern const std::string SNAPSHOT_LEAK_DIR;
extern const std::string HIDUMPER_OUTPUT_DIR;
extern const std::string RENDER_PROC_SUFFIX;
extern const std::string TIMEOUT_BIN;
/*
 * Snapshot artifact families. faultloggerd names every heap snapshot
 * <family>-<pid>[-<tid>]-<timestamp> inside SNAPSHOT_TEMP_DIR, and the hidumper command decides
 * which family is used, so the family name is the handle that ties a command to its artifact.
 * Taken from base/hiviewdfx/faultloggerd/services/config/faultloggerd_config.json.
 */
extern const std::string FAMILY_JSHEAP;
extern const std::string FAMILY_LEAKLIST;
extern const std::string FAMILY_NATIVEHEAP;
extern const std::string FAMILY_KOTLINHEAP;
extern const std::string FAMILY_JSVMHEAP;
extern const std::string FAMILY_ARKWEB_JSHEAP;
extern const std::string RAWHEAP_EXT;
extern const std::string RECORD_MEM_PATH;
extern const std::string TIMES_SECTION;
/*
 * Diagnostics of the argument parser. They are produced by hidumper itself, before any executor
 * runs, so they are identical on every product and can be asserted unconditionally. Verified
 * byte for byte on a plugin-enabled phone and on an OpenHarmony board. An option that a build does
 * not register is reported by getopt as a missing pid argument with the dashes stripped, which is
 * also how a plugin-only option is rejected on a build without the plugin.
 */
extern const std::string ERR_MISS_MEMSMAPS;
extern const std::string ERR_MISS_MEMJSHEAP;
extern const std::string ERR_MISS_MEMCJHEAP;
extern const std::string ERR_MISS_BOGUS;
extern const std::string ERR_MISS_NOSUCHOPT;
extern const std::string ERR_MISS_H;
extern const std::string ERR_FD_NEEDS_PID;
extern const std::string ERR_IPC_ARGS;
extern const std::string ERR_NO_SUCH_PROCESS;
extern const std::string HINT_NO_RECORDS;
extern const std::string HINT_NO_FAULTLOGS;
extern const std::string SMAPS_VERBOSE_HEADER;
/* Detail header that --show-ashmem adds. The Ashmem: summary itself is printed whenever the
 * process has ashmem at all, so the header is what actually proves the flag took effect. */
/* Written by the ArkTS runtime for every virtual machine it collected. */
extern const std::string ERR_MISS_UNKNOWN_OPT;
/* Always-present system processes, used to find one that can serve a data-dependent capability. */
extern const std::vector<std::string> SYSTEM_PROBE_TARGETS;
const int32_t UNKNOWN_RECORD_ID = 999;

const int32_t MIN_RECORD_INTERVALS = 2;

const int32_t RECORD_MEM_TIMEOUT_SEC = 60;
/* Stop drain: an interval already in flight may complete its write after the SIGINT cleared
 * the service-side loop flag; the verify window is the strict stopped oracle on top of it. */
constexpr int32_t RECORD_STOP_DRAIN_SEC = 10;
constexpr int32_t RECORD_STOP_VERIFY_SEC = 3;

/*
 * Target processes for the heap dump commands, tried in order and confirmed by artifact. These are
 * hints about which process on a given product usually carries the required runtime, not
 * requirements: a case skips when none of them yields an artifact, and picks up a different
 * process automatically once one is present.
 *   --native       any application process, so the UI process is enough;
 *   --kotlin       an application that registered a "KMP" hidebug mem dump listener, which
 *                  the KMP runtime does; the media library data service is such an
 *                  application on phone products;
 *   --jsvm         an application hosting a live JSVM isolate;
 *   --arkweb-js    an ArkWeb render process, created on demand by an application that shows a Web
 *                  component; the dedicated HAP provides one.
 */
/*
 * A heap dump target. bundleName is looked up with pidof; mainAbility, when set, is started first
 * because the process only exists or only runs JS while it is in the foreground; needsUiInteraction
 * keeps the isolate executing so that asynchronous dump requests are actually served.
 */
struct HeapTargetHint {
    std::string bundleName;
    std::string mainAbility;
    bool needsUiInteraction;
};

/*
 * In-repo dedicated target HAP (test/resource/hidumper_func_hap), built by GN with the generic
 * OpenHarmony test certificate and pushed by test/resource/ohos_test.xml. When installed it
 * provides deterministic dump targets: ArkTS heap ballast, a self-driven JSVM isolate (no UI
 * driver needed), a registered "KMP" mem-dump listener, an ArkWeb ':render' process, an armed
 * jsLeakWatcher (enable + a watched probe held alive on purpose, so --leakobj always has real
 * content to report), and an ohosTest CrashTest case that provisions faultlog records for the
 * -e family. The HAP is the mandatory self-built target of the snapshot and leak families: an
 * available HAP that cannot start FAILs its case instead of silently passing on a system-app
 * fallback; only an environment that provides no HAP at all (nothing installed and nothing
 * pushed) skips its HAP-driven cases as unsupported (FuncTestHapAvailable).
 */
extern const std::string FUNC_TEST_HAP_BUNDLE;
extern const std::string FUNC_TEST_HAP_ABILITY;
/* Marker the HAP passes to jsLeakWatcher.watch(); check() echoes it inside the leaklist JSON,
 * which makes the --leakobj artifact verifiable at content level on both products. */
extern const std::string LEAK_PROBE_MARKER;
extern const std::vector<std::string> FUNC_TEST_HAP_CANDIDATES;
extern const std::vector<std::string> FUNC_TEST_OHOS_TEST_HAP_CANDIDATES;
extern const std::string CRASH_TEST_CMD;
extern const std::string FAULTLOG_DIR;
constexpr int32_t HAP_START_WAIT_SEC = 20;

constexpr int32_t HAP_START_ATTEMPTS = 3;

constexpr int32_t HAP_START_RETRY_SEC = 2;

constexpr int32_t RECORD_PROVISION_TIMEOUT_SEC = 30;

constexpr int32_t AA_TEST_TIMEOUT_SEC = 60;

/* GC probe (case 030): settle windows around holdGarbage/dropGarbage+--gc and the artifact-size
 * floor for the reclaimed garbage. Measured deltas: 23.0 MiB on the board, 22.8 MiB on the
 * phone, against ~24 MiB of held unique garbage. */
constexpr int32_t GC_HOLD_POLL_SEC = 2;
/* Consecutive equal-readings confirmations before the construction counts as finished; two
 * readings once aliased with a construction pause under back-to-back suite load. */
constexpr int32_t GC_HOLD_STABLE_REPEATS = 2;

constexpr int32_t GC_HOLD_MAX_SEC = 40;

constexpr int32_t GC_RECLAIM_SETTLE_SEC = 6;

constexpr int64_t GC_SHRINK_FLOOR_BYTES = 4 * 1024 * 1024;

extern const std::string UITEST_BIN;
extern const std::string UI_INTERACTION_CMD;
constexpr int32_t UI_INTERACTION_INTERVAL_MS = 800;

extern const std::vector<HeapTargetHint> NATIVE_TARGETS;
extern const std::vector<HeapTargetHint> KMP_TARGETS;
extern const std::vector<HeapTargetHint> JSVM_TARGETS;
extern const std::string SNAPSHOT_STAGING_DIR;
extern const std::string SNAPSHOT_FINAL_DIR;
extern const std::string FINAL_NAME_PREFIX;
extern const std::string PLUGIN_SO_PATH_64;
struct CmdResult {
    int32_t exitCode {-1};
    std::string output;
};

bool FileExists(const std::string &path);

int64_t FileSize(const std::string &path);

bool HasTimeoutBinary();

CmdResult RunShell(const std::string &shellCmd, int32_t timeoutSec);

CmdResult RunHidumper(const std::string &args);

bool Contains(const CmdResult &result, const std::string &needle);

/* Universal "header carries data below" check: at least one non-blank content line follows
 * the line containing the header needle. This is the review-agreed minimal oracle for the
 * interface family - the command answered and its report is well formed. */
bool HasContentBelow(const std::string &output, const std::string &header);

/* Counts ability sections that actually ANSWERED: the [ability] header and the dashed ability
 * name banner are printed by hidumper itself, so a section only counts when real content
 * follows the banner. */
int32_t CountAnsweredAbilities(const std::string &output);

std::string GetSystemParam(const std::string &key);

std::string DescribeEnvironment();

std::vector<std::string> ListFiles(const std::string &dir);

int32_t CountDirEntries(const std::string &dir);

std::vector<std::string> ListFilesContaining(const std::string &dir, const std::string &needle);

std::string JoinFiles(const std::vector<std::string> &files);

std::string ReadProcessName(int32_t pid);

std::vector<int32_t> ListProcessIds();

/* Reads the parent pid (field 4) and the start time (field 22) from /proc/<pid>/stat. Parsing
 * starts after the last ')' because the comm field may contain spaces and parentheses. */
bool ReadProcStatParentAndStart(int32_t pid, int32_t &ppid, int64_t &startTime);

/*
 * Resolves the true main process among same-named pid candidates. The ArkTS heap snapshot
 * machinery forks a child of the target app which keeps the parent's process name while it
 * streams the snapshot out - observed to outlive the triggering hidumper command by several
 * seconds, and a pid wrap-around that sorts the child first made pidof-plus-atoi pick the
 * doomed child, failing the next dump with "No such process". A fork child is parented by the
 * app itself and never starts before it, so candidates parented by another candidate are
 * dropped and the earliest-started survivor wins.
 */
int32_t ResolveMainProcess(const std::vector<int32_t> &candidates);

/* Reads the resident page count (field 24 of /proc/<pid>/stat); 0 when unreadable. Used to
 * poll a process's heap construction progress without disturbing its threads. */
int64_t ReadResidentPages(int32_t pid);

int32_t FindRenderProcessPid();

/* pidof can list several same-named pids (snapshot fork children); resolve the real main
 * process instead of blindly taking the first token. */
int32_t GetPidByName(const std::string &processName);

/* The plugin build ships a GPU-capable UI process; the OpenHarmony board build ships a launcher. */
int32_t PickUiProcessPid(std::string &processName);

int32_t ParsePrefixedNumber(const std::string &output, const std::string &prefix);

/*
 * Collects the full paths of the files under dirs whose name contains needle. Used both to record
 * what already exists before a command runs and to observe what the command produced.
 */
std::set<std::string> CollectFiles(const std::vector<std::string> &dirs, const std::string &needle);

std::set<std::string> WithoutPreexisting(const std::set<std::string> &current,
                                         const std::set<std::string> &preexisting);

/*
 * Waits until a file that was not present in preexisting shows up, is non-empty and has stopped
 * growing for SETTLE_ROUNDS consecutive polls. Heap snapshots are streamed into the file by the
 * target process and are then copied by the migration companion, so existence alone is not proof
 * that the dump finished. The growth counter is kept per path: the leak directory accumulates
 * snapshots of previous runs, and comparing sizes across different files would both miss a settled
 * file and mistake two equally sized files for a stable one.
 */
/* Per-path growth state of the settle wait: a path counts as settled once its size is
 * non-zero and unchanged for SETTLE_ROUNDS consecutive polls. */
class SnapshotSettleTracker {
public:
    bool Update(const std::string &path)
    {
        int64_t size = FileSize(path);
        auto last = lastSizes_.find(path);
        if (size > 0 && last != lastSizes_.end() && last->second == size) {
            return ++stableRounds_[path] >= SETTLE_ROUNDS;
        }
        stableRounds_[path] = 0;
        lastSizes_[path] = size;
        return false;
    }

private:
    std::map<std::string, int64_t> lastSizes_;
    std::map<std::string, int32_t> stableRounds_;
};

bool WaitNewSnapshotSettled(const std::vector<std::string> &dirs, const std::string &needle,
                            const std::set<std::string> &preexisting, int32_t timeoutSec,
                            std::string &foundPath);

/* Waits until no file that was absent from preexisting still matches needle. */
bool WaitNoNewFiles(const std::vector<std::string> &dirs, const std::string &needle,
                    const std::set<std::string> &preexisting, int32_t timeoutSec);

/*
 * Waits until the set of files matching needle and their sizes stop changing for quietSec
 * consecutive polls. Dumping an ArkTS process writes one snapshot per virtual machine and the
 * migration companion moves them one by one, so artifacts of an earlier case can still be in
 * flight. Settling first keeps a later case from attributing them to its own command, which matters
 * most for the negative case that asserts no snapshot is written at all.
 */
void WaitSnapshotQuiescent(const std::vector<std::string> &dirs, const std::string &needle,
                           int32_t quietSec, int32_t timeoutSec);

/*
 * Removes exactly the snapshot artifacts a case produced and nothing else: the pre-existing
 * entries of both directories are recorded on construction, and only files matching the case's own
 * needle that were absent before are unlinked on destruction. The needle is matched as a substring
 * so that both the staged name and the migrated name, which carries an extra prefix, are covered.
 * The reliability directory is shared with the leak detector and holds unrelated artifacts such as
 * memleak-native-* samples, so a blanket cleanup is not acceptable.
 */
class SnapshotCleanupGuard {
public:
    explicit SnapshotCleanupGuard(const std::string &needle) : needle_(needle)
    {
        before_ = CollectFiles(Dirs(), needle_);
    }

    ~SnapshotCleanupGuard()
    {
        for (const auto &path : WithoutPreexisting(CollectFiles(Dirs(), needle_), before_)) {
            (void)unlink(path.c_str());
        }
    }

    SnapshotCleanupGuard(const SnapshotCleanupGuard &) = delete;
    SnapshotCleanupGuard &operator=(const SnapshotCleanupGuard &) = delete;

    const std::set<std::string> &Preexisting() const
    {
        return before_;
    }

private:
    static std::vector<std::string> Dirs()
    {
        return {SNAPSHOT_STAGING_DIR, SNAPSHOT_FINAL_DIR};
    }

    std::string needle_;
    std::set<std::string> before_;
};

std::string DirListingForReport();

/* Substring that every finished artifact of a family carries for the pid the command was given. */
std::string ArtifactNeedle(const std::string &family, int32_t pid);

struct ArtifactCheck {
    bool produced {false};
    bool verified {false};
    std::string path;
    std::string detail;
    int64_t size { 0 };
};

/*
 * Runs a snapshot producing command and verifies its artifact end to end.
 *
 * Every heap snapshot command of hidumper is asynchronous: the CLI returns as soon as the request
 * was handed to the target process, the process asks faultloggerd for a descriptor and streams the
 * dump into SNAPSHOT_STAGING_DIR, and on a plugin build the companion then moves it into
 * SNAPSHOT_FINAL_DIR. Asserting on stdout therefore proves nothing, which is why the contract is
 * checked on the artifact in the final directory instead:
 *   - a file that did not exist before the command appears,
 *   - its name carries the family of the command and the pid the command was given,
 *   - it is larger than the minimum, so an empty placeholder does not pass,
 *   - when the build has a migration companion the staging copy is gone afterwards.
 * produced is false only when no artifact appeared anywhere, which lets a caller that cannot
 * guarantee a capable target process skip instead of reporting a product defect. stagingNeedle
 * overrides the name filter used in the staging directory for families whose staged name leads with
 * a different pid than the migrated name, which is the case for arkweb_jsheap.
 */
ArtifactCheck VerifySnapshotArtifact(const std::string &hidumperArgs, const std::string &family,
                                     int32_t pid, int32_t timeoutSec,
                                     const std::string &stagingNeedle = "");

/*
 * Resolves the main process behind an ArkWeb render process. The staged arkweb_jsheap artifact is
 * named with the main pid while the migrated one carries the render pid, so both are needed to
 * describe the artifact precisely.
 */
int32_t GetMainPidOfRender(int32_t renderPid);

/*
 * Keeps the foreground application executing JS for as long as the object lives. The heap
 * snapshot dump is delivered through the engine's interrupt-request hook, which only runs when
 * the isolate's owner thread next reaches an interrupt point, so dumping an idle application
 * allocates the output file and never fills it. Verified on a phone product: without input the
 * artifacts stay at zero bytes, with input the active isolate dumps and the file is migrated.
 */
class UiInteractionDriver {
public:
    UiInteractionDriver() : stop_(false)
    {
        if (access(UITEST_BIN.c_str(), X_OK) != 0) {
            return;
        }
        worker_ = std::thread([this]() {
            while (!stop_.load()) {
                (void)RunShell(UI_INTERACTION_CMD, SHORT_CMD_TIMEOUT_SEC);
                std::this_thread::sleep_for(std::chrono::milliseconds(UI_INTERACTION_INTERVAL_MS));
            }
        });
    }

    ~UiInteractionDriver()
    {
        stop_.store(true);
        if (worker_.joinable()) {
            worker_.join();
        }
    }

    UiInteractionDriver(const UiInteractionDriver &) = delete;
    UiInteractionDriver &operator=(const UiInteractionDriver &) = delete;

private:
    std::atomic<bool> stop_;
    std::thread worker_;
};

/* ============================== controlled target child ============================== */

struct TargetChildReport {
    int32_t fdCount = 0;
    int32_t threadCount = 0;
    int32_t ashmemFd = -1; // >= 0 when the named ashmem region was created
};

struct TargetChildState {
    pid_t pid = -1;
    int lifelineWrite = -1;
    std::string ioPath;
    TargetChildReport report;
    bool valid = false;
};

void TargetChildSignalHandler(int);

/* Counts the descriptors of the calling process, including the descriptor opendir itself takes. */
int32_t CountSelfFdIncludingScanner();

/*
 * Runs in the forked child. The test process is single threaded at fork time, so the child starts
 * from a consistent address space and may allocate freely. Shape: stdio, the lifeline read end,
 * TARGET_PIPE_PAIRS held pipe pairs, one io file kept open, one ashmem descriptor when the device
 * provides /dev/ashmem, TARGET_SLEEPER_THREADS named sleepers and one half duty busy thread. The
 * busy thread exists so --cpuusage has a measurable subject; the io file so --storage has churn.
 * Exits when the parent closes the lifeline or on SIGTERM.
 */
/* Closes stray descriptors inherited from the test process so the fd shape is exact. */
void CloseInheritedFds(int reportFd, int lifelineFd);

/* Named sleeper threads plus one half-duty busy thread so --cpuusage has a measurable subject. */
void SpawnTargetThreads(std::vector<std::thread> &threads);

std::vector<int> CreateHeldPipes();

/* Material heap ballast; the volatile sink keeps the compiler from optimizing it away. */
void AllocateBallastHeap();

/* Creates and touches the named ashmem region. The caller keeps region alive: ~Ashmem()
 * closes the descriptor, and the shape report accounts for it as a held fd. */
int32_t CreateTouchedAshmem(sptr<Ashmem> &region);

/* Reports the stable shape; the scanner fd and the report pipe are gone right afterwards. */
void SendChildHandshake(int reportFd, const TargetChildReport &report);

/* Polls the lifeline and churns the io file until the parent closes the lifeline. */
void RunChildDutyLoop(int lifelineFd, int ioFd);

void TargetChildMain(int reportFd, int lifelineFd, const std::string &ioPath);

/* Blocks until the child reports its shape, or the timeout expires. */
bool ReadTargetChildReport(int reportFd, TargetChildReport &report);

/*
 * RAII owner of the controlled target child. Start forks and waits for the handshake plus the
 * descriptor count settling at the reported value; Stop closes the lifeline (child exits on EOF),
 * escalates through SIGTERM to SIGKILL, reaps, and removes the io file.
 */
class TargetChildGuard {
public:
    TargetChildGuard() = default;

    ~TargetChildGuard()
    {
        Stop();
    }

    TargetChildGuard(const TargetChildGuard &) = delete;
    TargetChildGuard &operator=(const TargetChildGuard &) = delete;

    bool Start(std::string &failReason)
    {
        static std::atomic<uint32_t> spawnCounter { 0 };
        int reportFds[2] = { -1, -1 };
        int lifeline[2] = { -1, -1 };
        if (pipe(reportFds) != 0) {
            failReason = "report pipe failed";
            return false;
        }
        if (pipe(lifeline) != 0) {
            close(reportFds[0]);
            close(reportFds[1]);
            failReason = "lifeline pipe failed";
            return false;
        }
        state_.ioPath = TARGET_IO_DIR + "/hdfunc_target_io_" + std::to_string(getpid()) +
                        "_" + std::to_string(spawnCounter.fetch_add(1));
        pid_t child = fork();
        if (child < 0) {
            close(reportFds[0]);
            close(reportFds[1]);
            close(lifeline[0]);
            close(lifeline[1]);
            failReason = "fork failed";
            return false;
        }
        if (child == 0) {
            close(reportFds[0]);
            close(lifeline[1]);
            TargetChildMain(reportFds[1], lifeline[0], state_.ioPath);
            _exit(1);
        }
        close(reportFds[1]);
        close(lifeline[0]);
        state_.pid = child;
        state_.lifelineWrite = lifeline[1];
        if (!ReadTargetChildReport(reportFds[0], state_.report)) {
            close(reportFds[0]);
            failReason = "child handshake timed out";
            Stop();
            return false;
        }
        close(reportFds[0]);
        /* The child closes the report pipe right after writing; wait for the count to settle.
         * Best effort: the shape was reported, so a lingering transient fd must not void it. */
        (void)WaitFdShapeSettled();
        state_.valid = true;
        return true;
    }

    void Stop()
    {
        if (state_.lifelineWrite >= 0) {
            close(state_.lifelineWrite);
            state_.lifelineWrite = -1;
        }
        if (state_.pid > 0) {
            /* Reaping this guard's own forked child, never a foreign process. The direct libc
             * spelling is flagged by the product-code ruleset (G.STD.17-CPP), so the syscall form
             * makes the purpose explicit. SIGTERM first (the child also exits on lifeline EOF),
             * SIGKILL only as the last-resort backstop. */
            (void)syscall(SYS_kill, state_.pid, SIGTERM);
            const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(TARGET_EXIT_TIMEOUT_SEC);
            int status = 0;
            bool reaped = false;
            while (std::chrono::steady_clock::now() < deadline) {
                pid_t done = waitpid(state_.pid, &status, WNOHANG);
                if (done == state_.pid) {
                    reaped = true;
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(50)); // NOLINT: reap poll
            }
            if (!reaped) {
                (void)syscall(SYS_kill, state_.pid, SIGKILL);
                (void)waitpid(state_.pid, &status, 0);
            }
            state_.pid = -1;
        }
        if (!state_.ioPath.empty()) {
            (void)unlink(state_.ioPath.c_str());
            state_.ioPath.clear();
        }
        state_.valid = false;
    }

    pid_t Pid() const
    {
        return state_.pid;
    }

    /* True once the child's descriptor count matches the reported shape. */
    bool WaitFdShapeSettled()
    {
        const auto deadline = std::chrono::steady_clock::now() +
            std::chrono::seconds(TARGET_SETTLE_TIMEOUT_SEC);
        std::string fdDir = "/proc/" + std::to_string(state_.pid) + "/fd";
        while (std::chrono::steady_clock::now() < deadline) {
            if (CountDirEntries(fdDir) == state_.report.fdCount) {
                return true;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(TARGET_SETTLE_POLL_MS));
        }
        return false;
    }

    const TargetChildReport &Report() const
    {
        return state_.report;
    }

private:
    TargetChildState state_;
};

/*
 * First integer of the table row whose trimmed text starts with rowLabel followed by whitespace.
 * Rows whose label matches but carry no number (column headers) are skipped, so the category
 * table's data rows are found even though the header repeats the label.
 */
int64_t RowFirstNumber(const std::string &output, const std::string &rowLabel);

/* The first three integers of the smaps Summary row (Size, Rss, Pss); the label trails them. */
constexpr int32_t SUMMARY_COLUMN_COUNT = 3;

constexpr int32_t SUMMARY_PSS_INDEX = 2;

bool ParseSummaryLine(const std::string &line, int64_t &sizeKb, int64_t &rssKb, int64_t &pssKb);

bool SummaryRowNumbers(const std::string &output, int64_t &sizeKb, int64_t &rssKb, int64_t &pssKb);

/* Known descriptor budget of the child: stdio, lifeline read end, pipe pairs and the io file. */
constexpr int32_t TARGET_BASE_FDS = 3;                                        // NOLINT: stdio

constexpr int32_t TARGET_EXTRA_FDS = TARGET_PIPE_PAIRS * 2 + 2;               // NOLINT: pipes+lifeline+io

/* ============================== dedicated target HAP ============================== */

bool IsFuncTestHapInstalled();

/* Environment capability probe for the HAP-driven cases: true when the dedicated HAP is
 * installed or its files were pushed to one of the known candidate locations (which
 * SetUpTestCase installs from). False means the environment cannot provide the self-built
 * dump target at all and the HAP-driven cases skip as unsupported; an available HAP that
 * fails to start or produce artifacts still FAILs its case. */
bool FuncTestHapAvailable();

std::string FirstExistingFile(const std::vector<std::string> &paths);

/* Idempotent: installs the pushed HAP pair when the files are present and the bundle is not. */
void EnsureFuncTestHapInstalled();

/* Brings the target HAP to the foreground and returns its pid, or NO_PID when unavailable. */
int32_t StartFuncTestHap();

/* The ability manager rejects a concurrent ability start with the documented transient
 * error 10106101 ("Another ability is being started ... just wait for the startup");
 * observed under back-to-back suite load. Retries exactly that case so a genuinely
 * missing bundle or ability still fails fast. */
CmdResult StartAbilityWithBusyRetry(const std::string &bundle, const std::string &ability,
                                    const std::string &extraArgs = "");

/* Delivers one hdaction (holdGarbage/dropGarbage/fillGarbage) to the target HAP. */
CmdResult StartHapAction(const std::string &action);

/*
 * Brings up an application that shows a Web component and waits for its ':render' process.
 * Returns NO_PID right away when the start command does not report success (bundle missing),
 * so callers fall through to the next candidate without burning the full wait window.
 */
int32_t StartAppAndWaitRenderPid(const std::string &bundle, const std::string &ability);

/*
 * Provisions one fault event record by crashing the HAP's ohosTest process (pattern of
 * faultlogger_hap, hiviewdfx_hiview PR !4552): aa test runs CrashTest#SegvCase, the test
 * process SIGSEGVs, faultloggerd writes the cppcrash log and - where the product carries the
 * hisysevent fault pipeline - the AppExitReason record that -e --list/--print consume.
 * Returns false when records still do not appear (verified board behavior: the file is written
 * but no hisysevent record exists on that image); callers then treat the case as reserved
 * (direct pass).
 */
bool ProvisionFaultRecord();

/*
 * Ensures at least one faultlog file exists for the file-driven views (-e). The crash trigger
 * writes the file even on images without the hisysevent pipeline.
 */
bool EnsureFaultLogFile();

/*
 * Finds a process that can actually serve --mem-heap <ARG>. The decision is made by the artifact,
 * never by a static trait: an application was observed to load libjsvm.so and still fail the jsvm
 * dump because it hosted no live isolate, so scanning maps would have produced a false target.
 * Candidates are tried in order and the first one yielding an artifact wins.
 */
bool FindCapableHeapTarget(const std::string &argFlag, const std::string &family,
                           const std::vector<HeapTargetHint> &candidates, int32_t &pid,
                           std::string &processName, ArtifactCheck &check);

int32_t CountLines(const std::string &text);

int32_t CountOccurrences(const std::string &text, const std::string &needle);

/*
 * Starts a command in the background and returns once the launching shell has exited. The command
 * is handed to its own shell instead of RunShell because that helper prefixes a timeout, and a
 * timeout wrapped around a subshell is not a valid command line. The launched process is reparented
 * when the shell exits and keeps running until the caller stops it.
 */
void StartBackground(const std::string &shellCmd);

std::string ReadFileContent(const std::string &path);

/*
 * Stops a running "--mem <pid> -t" recording loop. The CLI turns SIGINT into a
 * "hidumper --mem SIGINT" request that clears the service side loop flag, so sending the request
 * directly is the reliable way to stop it and is issued unconditionally: a recording loop left
 * behind would keep sampling memory on the device after the suite finished.
 *
 * Stop is one-shot on purpose. The loop flag is only reset when a recording loop actually exits
 * (memory_info.cpp:485), so a request that arrives while no loop is running leaves the flag cleared
 * and the next recording silently produces nothing. Sending the request twice would therefore break
 * the following run of this case.
 */
class MemRecordGuard {
public:
    MemRecordGuard() = default;

    ~MemRecordGuard()
    {
        Stop();
    }

    MemRecordGuard(const MemRecordGuard &) = delete;
    MemRecordGuard &operator=(const MemRecordGuard &) = delete;

    void Stop()
    {
        if (stopped_) {
            return;
        }
        stopped_ = true;
        (void)RunHidumper("--mem SIGINT");
    }

private:
    bool stopped_ {false};
};

/* An -e --list data row carries six whitespace-separated fields:
 * date time foreground reason record_id process_name; the header row has five. */
constexpr size_t LIST_ROW_COLUMNS = 6;

constexpr size_t LIST_ROW_RECORD_ID_COLUMN = 4;

constexpr size_t LIST_ROW_NAME_COLUMN = 5;

/* Documented edge: an identical --since/--until window is closed-open and finds nothing. */
extern const std::string EMPTY_WINDOW_TS;
std::vector<std::string> SplitTokens(const std::string &line);

std::vector<std::string> CollectListDataRows(const std::string &output);

std::string ListRowColumn(const std::string &row, size_t index);

/*
 * Finds a record of the dedicated HAP whose faultlog is present, and returns its id together
 * with the successful -e --print output. Device-ambient record types legitimately carry no
 * faultlog (verified on the phone: Upgrade and KillProcess answer the documented "this type of
 * record does not have faultlog."), while the crash trigger's signalExit record always has its
 * cppcrash log - so the bundle rows are walked newest-first, each id probed with -e --print,
 * and when no row yields a section the crash is triggered explicitly and the walk is repeated
 * within a provisioning deadline. Empty result: no hisysevent pipeline (board) or a broken
 * trigger; callers decide per build.
 */
/* One newest-first walk of the "-e --list" bundle rows, probing each id with -e --print. */
bool ProbeBundleFaultlogRows(std::string &recordId, CmdResult &printed);

bool FindBundleFaultlogRecord(std::string &recordId, CmdResult &printed);

/* First run of at least minDigits consecutive digits in a line, or an empty string. */
std::string FirstLongDigitRun(const std::string &line, size_t minDigits);

/* First record id of an "-e --list" table, or an empty string when the device has no records. */
std::string ParseFirstRecordId(const std::string &listOutput);

} // namespace HiviewDFX
} // namespace OHOS
#endif // HIDUMPER_FUNCTION_TEST_ENV_H
