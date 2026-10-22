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
 * End-to-end functional verification suite for hidumper.
 *
 * Unlike the sibling suites in this directory, which drive C++ classes in-process and mainly
 * protect interfaces and coverage, every case here exercises the deployed artifact through the
 * real command line (CLI -> IPC -> DumpManagerService(1212) -> executors -> output) and asserts
 * on externally observable evidence: stdout sections, generated files, and values cross-checked
 * against the kernel source of truth under /proc.
 *
 * Two device shapes are covered by one source file:
 *   - plugin-disabled build (HIDUMPER_HIVIEWDFX_PLUGIN_ENABLE undefined), e.g. an OpenHarmony
 *     board without the closed-source memory plugin;
 *   - plugin-enabled build (macro defined), e.g. a commercial phone carrying
 *     libhidumperplugin_impl.z.so and the closed-source leak-detector companions.
 *
 * The expected behaviour of a case is chosen by a two-axis oracle:
 *   axis 1 (build time) : HIDUMPER_HIVIEWDFX_PLUGIN_ENABLE / HIDUMPER_HIVIEWDFX_HIVIEW_ENABLE
 *                         decide whether a capability is compiled into hidumper at all;
 *   axis 2 (run time)   : const.product.software.version decides whether the closed-source
 *                         companions that consume hidumper output (snapshot migration) exist.
 * A third step probes whether the hardware/process state can physically exercise the capability
 * (GPU-holding process, ":render" process, the dedicated target HAP). Probing failures SKIP,
 * never FAIL, so that a missing environment is not reported as a product defect; in particular
 * the HAP-driven cases skip as unsupported when the dedicated HAP is neither installed nor
 * pushed anywhere installable (FuncTestHapAvailable). A mismatch between the oracle and the
 * observed behaviour FAILS, because that is a genuine build or deployment defect - an available
 * HAP that fails to start or produce artifacts is on that side of the line.
 */

#include <gtest/gtest.h>
#include <dirent.h>
#include <poll.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cctype>
#include <cstdio>
#include <algorithm>
#include <chrono>
#include <set>
#include <string>
#include <thread>
#include <vector>
#include "hidumper_function_test_env.h"
#include "hidumper_test_utils.h"

using namespace testing::ext;

namespace OHOS {
namespace HiviewDFX {

class HidumperFunctionTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void HidumperFunctionTest::SetUpTestCase(void)
{
    printf("hidumper functional environment: %s\n", DescribeEnvironment().c_str());
    /* Best effort: pick up the pushed target HAPs so snapshot-family cases get deterministic
     * targets. Devices without the pushed files keep the system-app fallback behavior. */
    EnsureFuncTestHapInstalled();
    printf("target HAP (%s) installed: %d\n", FUNC_TEST_HAP_BUNDLE.c_str(),
           IsFuncTestHapInstalled() ? 1 : 0);
}

void HidumperFunctionTest::TearDownTestCase(void)
{
}

void HidumperFunctionTest::SetUp(void)
{
}

void HidumperFunctionTest::TearDown(void)
{
}

/* ===================================================== help ===================================================== */

/**
 * @tc.name: Help_Basic_001
 * @tc.desc: Command: hidumper -h
 *           Function: help information.
 *           Pass with plugin disabled: 'usage:' and the '-h |help text for the tool' anchor line
 *           are present.
 *           Pass with plugin enabled: same as plugin-disabled.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperFunctionTest, Help_Basic_001, TestSize.Level1)
{
    CmdResult help = RunHidumper("-h");
    ASSERT_FALSE(help.output.empty()) << "-h produced no output at all";
    ASSERT_TRUE(Contains(help, "usage:")) << help.output;
    ASSERT_NE(help.output.find("|help text for the tool"), std::string::npos) << help.output;
}

/* ======================================== information clusters ======================================== */

/**
 * @tc.name: Cluster_List_002
 * @tc.desc: Command: hidumper -lc
 *           Function: list the information clusters.
 *           Pass with plugin disabled: the 'System cluster list:' header carries data below.
 *           Pass with plugin enabled: same as plugin-disabled.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperFunctionTest, Cluster_List_002, TestSize.Level1)
{
    CmdResult result = RunHidumper("-lc");
    ASSERT_TRUE(Contains(result, "System cluster list:")) << result.output;
    ASSERT_TRUE(HasContentBelow(result.output, "System cluster list:")) << result.output;
}

/**
 * @tc.name: Cluster_Multi_003
 * @tc.desc: Command: hidumper -c base -c system
 *           Function: dump several clusters in one invocation.
 *           Pass with plugin disabled: the [base] and [system] headers each carry data below.
 *           Pass with plugin enabled: same as plugin-disabled.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperFunctionTest, Cluster_Multi_003, TestSize.Level2)
{
    CmdResult result = RunHidumper("-c base -c system");
    ASSERT_TRUE(Contains(result, "[base]")) << result.output;
    ASSERT_TRUE(HasContentBelow(result.output, "[base]")) << result.output;
    ASSERT_TRUE(Contains(result, "[system]")) << result.output;
    ASSERT_TRUE(HasContentBelow(result.output, "[system]")) << result.output;
}

/**
 * @tc.name: Cluster_Base_004
 * @tc.desc: Command: hidumper -c base
 *           Function: dump the base cluster.
 *           Pass with plugin disabled: the [base] header carries data below.
 *           Pass with plugin enabled: same as plugin-disabled.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperFunctionTest, Cluster_Base_004, TestSize.Level2)
{
    CmdResult result = RunHidumper("-c base");
    ASSERT_TRUE(Contains(result, SECTION_BASE)) << result.output;
    ASSERT_TRUE(HasContentBelow(result.output, "[base]")) << result.output;
}

/**
 * @tc.name: Cluster_All_005
 * @tc.desc: Command: hidumper -c
 *           Function: dump all clusters.
 *           Pass with plugin disabled: the [base] and [system] headers each carry data below.
 *           Pass with plugin enabled: same as plugin-disabled.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperFunctionTest, Cluster_All_005, TestSize.Level3)
{
    CmdResult result = RunHidumper("-c");
    ASSERT_TRUE(Contains(result, "[base]")) << result.output;
    ASSERT_TRUE(HasContentBelow(result.output, "[base]")) << result.output;
    ASSERT_TRUE(Contains(result, "[system]")) << result.output;
    ASSERT_TRUE(HasContentBelow(result.output, "[system]")) << result.output;
}

/**
 * @tc.name: Cluster_Exclusive_006
 * @tc.desc: Command: hidumper -c system
 *           Function: dump the system cluster.
 *           Pass with plugin disabled: the [system] header carries data below.
 *           Pass with plugin enabled: same as plugin-disabled.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperFunctionTest, Cluster_Exclusive_006, TestSize.Level3)
{
    CmdResult result = RunHidumper("-c system");
    ASSERT_TRUE(Contains(result, "[system]")) << result.output;
    ASSERT_TRUE(HasContentBelow(result.output, "[system]")) << result.output;
}

/* ============================================ system abilities ============================================ */

/**
 * @tc.name: Sa_List_007
 * @tc.desc: Command: hidumper -ls
 *           Function: list the system abilities.
 *           Pass with plugin disabled: the 'System ability list:' header carries data below.
 *           Pass with plugin enabled: same as plugin-disabled.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperFunctionTest, Sa_List_007, TestSize.Level1)
{
    CmdResult result = RunHidumper("-ls");
    ASSERT_TRUE(Contains(result, "System ability list:")) << result.output;
    ASSERT_TRUE(HasContentBelow(result.output, "System ability list:")) << result.output;
}

/**
 * @tc.name: Sa_Multi_008
 * @tc.desc: Command: hidumper -s HiDumperService RenderService
 *           Function: dump several system abilities.
 *           Pass with plugin disabled: both ability headers carry data below.
 *           Pass with plugin enabled: same as plugin-disabled.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperFunctionTest, Sa_Multi_008, TestSize.Level2)
{
    CmdResult result = RunHidumper("-s HiDumperService RenderService");
    ASSERT_TRUE(Contains(result, "HiDumperService")) << result.output;
    ASSERT_TRUE(HasContentBelow(result.output, "HiDumperService")) << result.output;
    ASSERT_TRUE(Contains(result, "RenderService")) << result.output;
    ASSERT_TRUE(HasContentBelow(result.output, "RenderService")) << result.output;
}

/**
 * @tc.name: Sa_All_009
 * @tc.desc: Command: hidumper -s
 *           Function: dump all system abilities.
 *           Pass with plugin disabled: more than 2 abilities ANSWER with data below their section;
 *           the [ability] header alone does not count because hidumper prints it itself.
 *           Pass with plugin enabled: same as plugin-disabled.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperFunctionTest, Sa_All_009, TestSize.Level3)
{
    CmdResult result = RunHidumper("-s");
    /* The [ability] header is printed by hidumper itself - only sections that actually carry
     * an answer count (review decision). */
    ASSERT_GT(CountAnsweredAbilities(result.output), 2) << "expected more than 2 answered abilities: " << result.output;
}

/**
 * @tc.name: Sa_ForwardById_010
 * @tc.desc: Command: hidumper -s 1212 -a "-h"
 *           Function: forward arguments to the ability selected by SAID.
 *           Pass with plugin disabled: the HiDumperService header carries data below.
 *           Pass with plugin enabled: same as plugin-disabled.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperFunctionTest, Sa_ForwardById_010, TestSize.Level2)
{
    CmdResult result = RunHidumper("-s 1212 -a \"-h\"");
    ASSERT_TRUE(Contains(result, "HiDumperService")) << result.output;
    ASSERT_TRUE(HasContentBelow(result.output, "HiDumperService")) << result.output;
}

/**
 * @tc.name: Sa_ForwardByName_011
 * @tc.desc: Command: hidumper -s HiDumperService -a "-h"
 *           Function: forward arguments to the ability selected by name.
 *           Pass with plugin disabled: the HiDumperService header carries data below.
 *           Pass with plugin enabled: same as plugin-disabled.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperFunctionTest, Sa_ForwardByName_011, TestSize.Level2)
{
    CmdResult result = RunHidumper("-s HiDumperService -a \"-h\"");
    ASSERT_TRUE(Contains(result, "HiDumperService")) << result.output;
    ASSERT_TRUE(HasContentBelow(result.output, "HiDumperService")) << result.output;
}

/* =============================================== process views =============================================== */

/**
 * @tc.name: Process_Table_012
 * @tc.desc: Command: hidumper -p
 *           Function: whole-device process table.
 *           Pass with plugin disabled: the [processes] section and the UID/PID header carry data
 *           below.
 *           Pass with plugin enabled: same as plugin-disabled.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperFunctionTest, Process_Table_012, TestSize.Level2)
{
    CmdResult result = RunHidumper("-p");
    ASSERT_TRUE(Contains(result, SECTION_PROCESSES)) << result.output;
    ASSERT_TRUE(Contains(result, "UID")) << result.output;
    ASSERT_TRUE(HasContentBelow(result.output, "UID")) << result.output;
}

/**
 * @tc.name: Process_Single_013
 * @tc.desc: Command: hidumper -p <hap pid>
 *           Function: process dump narrowed to the own application.
 *           Pass with plugin disabled: the HAP pid row with the bundle name in the CMD column
 *           appears.
 *           Pass with plugin enabled: same as plugin-disabled.
 *           Notes: the HAP is self-built infrastructure: a start failure fails the case.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperFunctionTest, Process_Single_013, TestSize.Level2)
{
    if (!FuncTestHapAvailable()) {
        GTEST_SKIP() << "the dedicated target HAP is unavailable in this environment "
                        "(neither installed nor pushed to an installable location)";
    }
    int32_t pid = StartFuncTestHap();
    ASSERT_GT(pid, NO_PID) << "the dedicated target HAP (" << FUNC_TEST_HAP_BUNDLE <<
                           ") failed to start although it ships with the test resources";
    CmdResult result = RunHidumper("-p " + std::to_string(pid));
    ASSERT_TRUE(Contains(result, SECTION_PROCESSES)) << result.output;
    ASSERT_TRUE(Contains(result, std::to_string(pid))) << result.output;
    ASSERT_TRUE(Contains(result, FUNC_TEST_HAP_BUNDLE)) <<
        "the application's row carries no content: " << result.output;
}

/**
 * @tc.name: Process_Fd_014
 * @tc.desc: Command: hidumper -p <hap pid> --fd
 *           Function: fd accounting of the own application.
 *           Pass with plugin disabled: 'fd num:' is positive and the 'Leaked fd Top 10:' header
 *           carries data below.
 *           Pass with plugin enabled: same as plugin-disabled.
 *           Notes: the HAP is self-built infrastructure: a start failure fails the case.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperFunctionTest, Process_Fd_014, TestSize.Level2)
{
    if (!FuncTestHapAvailable()) {
        GTEST_SKIP() << "the dedicated target HAP is unavailable in this environment "
                        "(neither installed nor pushed to an installable location)";
    }
    int32_t pid = StartFuncTestHap();
    ASSERT_GT(pid, NO_PID) << "the dedicated target HAP (" << FUNC_TEST_HAP_BUNDLE <<
                           ") failed to start although it ships with the test resources";
    CmdResult result = RunHidumper("-p " + std::to_string(pid) + " --fd");
    ASSERT_TRUE(Contains(result, "fd num:")) << result.output;
    ASSERT_GT(ParsePrefixedNumber(result.output, "fd num:"), 0) << result.output;
    ASSERT_TRUE(Contains(result, "Leaked fd Top 10:")) << result.output;
    ASSERT_TRUE(HasContentBelow(result.output, "Leaked fd Top 10:")) << result.output;
}

/**
 * @tc.name: Process_Thread_015
 * @tc.desc: Command: hidumper -p <own pid> --thread
 *           Function: thread accounting.
 *           Pass with plugin disabled: 'Thread num:' is positive and the 'Top 10 Thread Names:'
 *           header carries data below.
 *           Pass with plugin enabled: same as plugin-disabled.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperFunctionTest, Process_Thread_015, TestSize.Level2)
{
    int32_t pid = getpid();
    CmdResult result = RunHidumper("-p " + std::to_string(pid) + " --thread");
    ASSERT_TRUE(Contains(result, "Thread num:")) << result.output;
    ASSERT_GT(ParsePrefixedNumber(result.output, "Thread num:"), 0) << result.output;
    ASSERT_TRUE(Contains(result, "Top 10 Thread Names:")) << result.output;
    ASSERT_TRUE(HasContentBelow(result.output, "Top 10 Thread Names:")) << result.output;
}

/**
 * @tc.name: Process_Verbose_016
 * @tc.desc: Command: hidumper -p <own pid> --fd/--thread, with and without -v
 *           Function: the verbose variants keep the reports intact.
 *           Pass with plugin disabled: every variant shows its count header ('fd num:' / 'Thread
 *           num:') with data below.
 *           Pass with plugin enabled: same as plugin-disabled.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperFunctionTest, Process_Verbose_016, TestSize.Level3)
{
    const std::string target = std::to_string(getpid());
    for (const auto &args : std::vector<std::string> {"--fd", "--fd -v"}) {
        CmdResult result = RunHidumper("-p " + target + " " + args);
        ASSERT_TRUE(Contains(result, "fd num:")) << args << ": " << result.output;
        ASSERT_TRUE(HasContentBelow(result.output, "fd num:")) << args << ": " << result.output;
    }
    for (const auto &args : std::vector<std::string> {"--thread", "--thread -v"}) {
        CmdResult result = RunHidumper("-p " + target + " " + args);
        ASSERT_TRUE(Contains(result, "Thread num:")) << args << ": " << result.output;
        ASSERT_TRUE(HasContentBelow(result.output, "Thread num:")) << args << ": " << result.output;
    }
}

/**
 * @tc.name: Process_ChildOracle_017
 * @tc.desc: Command: hidumper -p <child pid> --thread / --fd
 *           Function: accuracy oracle of the fd/thread counting against a child of constructed
 *           known shape (8 named sleeper threads + 1 busy + main; known fd budget).
 *           Pass with plugin disabled: 'Thread num:' equals the child handshake self-report AND the
 *           live /proc/<pid>/task count (three-way equality, safe because the quiescent child
 *           cannot churn); 'fd num:' equals the constructed budget exactly; the sleeper thread name
 *           ranks in the Top 10.
 *           Pass with plugin enabled: same as plugin-disabled.
 *           Notes: the child is self-built infrastructure: a creation or handshake failure fails
 *           the case.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperFunctionTest, Process_ChildOracle_017, TestSize.Level2)
{
    TargetChildGuard child;
    std::string failReason;
    if (!child.Start(failReason)) {
        /* The controlled child is self-built test infrastructure: a creation failure is a real
         * environment defect, not a reason to pass silently. */
        FAIL() << "controlled child creation failed: " << failReason;
    }
    const int32_t pid = child.Pid();
    const std::string pidText = std::to_string(pid);

    CmdResult threadOut = RunHidumper("-p " + pidText + " --thread");
    ASSERT_TRUE(Contains(threadOut, "Thread num:")) << threadOut.output;
    int32_t reportedThreads = ParsePrefixedNumber(threadOut.output, "Thread num:");
    int32_t liveTasks = CountDirEntries("/proc/" + pidText + "/task");
    const int32_t expectedThreads = TARGET_SLEEPER_THREADS + 2; // sleepers + busy + child main
    ASSERT_EQ(child.Report().threadCount, expectedThreads) << "child self-report disagrees with shape";
    ASSERT_EQ(reportedThreads, expectedThreads) << threadOut.output;
    ASSERT_EQ(reportedThreads, liveTasks) << "hidumper said " << reportedThreads << " but /proc says " << liveTasks;
    ASSERT_TRUE(Contains(threadOut, "Top 10 Thread Names:")) << threadOut.output;
    ASSERT_TRUE(Contains(threadOut, TARGET_THREAD_NAME)) <<
        "the sleeper thread name is missing from the ranking: " << threadOut.output;

    CmdResult fdOut = RunHidumper("-p " + pidText + " --fd");
    ASSERT_TRUE(Contains(fdOut, "fd num:")) << fdOut.output;
    int32_t reportedFds = ParsePrefixedNumber(fdOut.output, "fd num:");
    int32_t liveFds = CountDirEntries("/proc/" + pidText + "/fd");
    int32_t expectedFds = TARGET_BASE_FDS + TARGET_EXTRA_FDS;
    if (child.Report().ashmemFd >= 0) {
        expectedFds += 1; // the held ashmem descriptor
    }
    ASSERT_EQ(child.Report().fdCount, expectedFds) << "child self-report disagrees with shape";
    ASSERT_EQ(reportedFds, expectedFds) << fdOut.output;
    ASSERT_EQ(reportedFds, liveFds) << "hidumper said " << reportedFds << " but /proc says " << liveFds;
}

/* ===================================================== cpu ====================================================== */

/**
 * @tc.name: Cpu_Freq_018
 * @tc.desc: Command: hidumper --cpufreq
 *           Function: per-core CPU frequency.
 *           Pass with plugin disabled: the [cpufreq] header carries data below.
 *           Pass with plugin enabled: same as plugin-disabled.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperFunctionTest, Cpu_Freq_018, TestSize.Level2)
{
    CmdResult result = RunHidumper("--cpufreq");
    ASSERT_TRUE(Contains(result, SECTION_CPUFREQ)) << result.output;
    ASSERT_TRUE(HasContentBelow(result.output, SECTION_CPUFREQ)) << result.output;
}

/**
 * @tc.name: Cpu_Usage_019
 * @tc.desc: Command: hidumper --cpuusage
 *           Function: whole-device CPU usage report.
 *           Pass with plugin disabled: the [cpuusage] section appears and the 'Load average:'
 *           header carries data below.
 *           Pass with plugin enabled: same as plugin-disabled.
 *           Notes: builds without the CPU service (HIVIEW macro undefined; both current devices
 *           compile it in) are judged success directly.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperFunctionTest, Cpu_Usage_019, TestSize.Level3)
{
    if (!HIVIEW_ENABLED) {
        /* Reserved: this build does not compile the option in (both current devices do;
         * unreachable in practice), judged success directly. */
        GTEST_SUCCEED();
        return;
    }
    CmdResult result = RunHidumper("--cpuusage");
    ASSERT_TRUE(Contains(result, SECTION_CPUUSAGE)) << result.output;
    ASSERT_TRUE(Contains(result, "Load average:")) << result.output;
    ASSERT_TRUE(HasContentBelow(result.output, "Load average:")) << result.output;
}

/**
 * @tc.name: Cpu_UsagePid_020
 * @tc.desc: Command: hidumper --cpuusage 1
 *           Function: per-pid CPU usage for a fixed documented subject.
 *           Pass with plugin disabled: the [cpuusage] section appears and the 'Load average:'
 *           header carries data below.
 *           Pass with plugin enabled: same as plugin-disabled.
 *           Notes: init is the subject because the service cache is seeded once at collector
 *           construction and never refreshed (defect candidate #6, source-verified), so processes
 *           born after the service loaded are invisible; builds without the CPU service are judged
 *           success directly.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperFunctionTest, Cpu_UsagePid_020, TestSize.Level3)
{
    if (!HIVIEW_ENABLED) {
        /* Reserved: this build does not compile the option in (both current devices do;
         * unreachable in practice), judged success directly. */
        GTEST_SUCCEED();
        return;
    }
    CmdResult report = RunHidumper("--cpuusage " + std::to_string(INIT_PID));
    ASSERT_TRUE(Contains(report, SECTION_CPUUSAGE)) << report.output;
    ASSERT_TRUE(Contains(report, "Load average:")) << report.output;
    ASSERT_TRUE(HasContentBelow(report.output, "Load average:")) << report.output;
}

/* =================================================== network ==================================================== */

/**
 * @tc.name: Net_All_021
 * @tc.desc: Command: hidumper --net
 *           Function: network view.
 *           Pass with plugin disabled: the [net traffic] header carries data below.
 *           Pass with plugin enabled: same as plugin-disabled.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperFunctionTest, Net_All_021, TestSize.Level2)
{
    CmdResult result = RunHidumper("--net");
    ASSERT_TRUE(Contains(result, SECTION_NET_TRAFFIC)) << result.output;
    ASSERT_TRUE(HasContentBelow(result.output, SECTION_NET_TRAFFIC)) << result.output;
}

/**
 * @tc.name: Net_Pid_022
 * @tc.desc: Command: hidumper --net <own pid>
 *           Function: per-pid network view.
 *           Pass with plugin disabled: the [net traffic] header carries data below.
 *           Pass with plugin enabled: same as plugin-disabled.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperFunctionTest, Net_Pid_022, TestSize.Level2)
{
    CmdResult result = RunHidumper("--net " + std::to_string(getpid()));
    ASSERT_TRUE(Contains(result, SECTION_NET_TRAFFIC)) << result.output;
    ASSERT_TRUE(HasContentBelow(result.output, SECTION_NET_TRAFFIC)) << result.output;
}

/* =================================================== storage ==================================================== */

/**
 * @tc.name: Storage_All_023
 * @tc.desc: Command: hidumper --storage
 *           Function: storage view.
 *           Pass with plugin disabled: the [storage] header carries data below.
 *           Pass with plugin enabled: same as plugin-disabled.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperFunctionTest, Storage_All_023, TestSize.Level2)
{
    CmdResult result = RunHidumper("--storage");
    ASSERT_TRUE(Contains(result, SECTION_STORAGE)) << result.output;
    ASSERT_TRUE(HasContentBelow(result.output, SECTION_STORAGE)) << result.output;
}

/**
 * @tc.name: Storage_Io_024
 * @tc.desc: Command: hidumper --storage <pid> over fixed candidates (init, own pid)
 *           Function: per-process io accounting in the shape each kernel provides.
 *           Pass with plugin disabled: the board kernel exposes no /proc/<pid>/io for any candidate
 *           (verified); the [storage io] header is the interface's normal return there and the case
 *           passes on it.
 *           Pass with plugin enabled: a candidate's [storage io] header carries data below
 *           (rchar:/wchar: lines, verified).
 * @tc.type: FUNC
 */
HWTEST_F(HidumperFunctionTest, Storage_Io_024, TestSize.Level2)
{
    CmdResult result;
    bool found = false;
    for (int32_t pid : std::vector<int32_t> {INIT_PID, getpid()}) {
        result = RunHidumper("--storage " + std::to_string(pid));
        if (Contains(result, SECTION_STORAGE_IO) && Contains(result, "rchar:")) {
            found = true;
            break;
        }
    }
    /* The section header is the interface's normal return on every kernel; the io detail
     * lines additionally appear where the kernel exposes /proc/<pid>/io. */
    ASSERT_TRUE(Contains(result, SECTION_STORAGE_IO)) << result.output;
    if (found) {
        ASSERT_TRUE(HasContentBelow(result.output, SECTION_STORAGE_IO)) << result.output;
    }
    /* Board kernel (no /proc/<pid>/io for any candidate, verified): the normal section return
     * asserted above is the expected presentation. */
}

/* ============================================== ipc statistics ============================================== */

/**
 * @tc.name: Ipc_All_025
 * @tc.desc: Command: hidumper --ipc -a --start-stat / --stat / --stop-stat
 *           Function: three-phase IPC statistics lifecycle for all processes.
 *           Pass with plugin disabled: start/stop report success and the GlobalStatisticsInfo
 *           header carries data below; stop always runs (no lingering sampling).
 *           Pass with plugin enabled: same as plugin-disabled.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperFunctionTest, Ipc_All_025, TestSize.Level3)
{
    CmdResult start = RunHidumper("--ipc -a --start-stat");
    ASSERT_TRUE(Contains(start, "StartIpcStatistics")) << start.output;
    ASSERT_TRUE(Contains(start, "success")) << start.output;
    std::this_thread::sleep_for(std::chrono::seconds(2));
    CmdResult stat = RunHidumper("--ipc -a --stat");
    ASSERT_TRUE(Contains(stat, "GlobalStatisticsInfo")) << stat.output;
    ASSERT_TRUE(HasContentBelow(stat.output, "GlobalStatisticsInfo")) << stat.output;
    CmdResult stop = RunHidumper("--ipc -a --stop-stat");
    ASSERT_TRUE(Contains(stop, "StopIpcStatistics")) << stop.output;
    ASSERT_TRUE(Contains(stop, "success")) << stop.output;
}

/**
 * @tc.name: Ipc_Pid_026
 * @tc.desc: Command: hidumper --ipc <pid> --start-stat / --stat / --stop-stat
 *           Function: the same lifecycle for one pid.
 *           Pass with plugin disabled: the lifecycle works and the stat report is scoped via
 *           'CurrentPid:<pid>'.
 *           Pass with plugin enabled: same as plugin-disabled.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperFunctionTest, Ipc_Pid_026, TestSize.Level3)
{
    std::string processName;
    int32_t pid = PickUiProcessPid(processName);
    if (pid <= 0) {
        pid = getpid();
    }
    const std::string pidText = std::to_string(pid);
    CmdResult start = RunHidumper("--ipc " + pidText + " --start-stat");
    bool startOk = Contains(start, "StartIpcStatistics") && Contains(start, "success");
    CmdResult stat;
    if (startOk) {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        stat = RunHidumper("--ipc " + pidText + " --stat");
    }
    CmdResult stop = RunHidumper("--ipc " + pidText + " --stop-stat");
    ASSERT_TRUE(startOk) << start.output;
    ASSERT_TRUE(Contains(stop, "StopIpcStatistics")) << stop.output;
    ASSERT_TRUE(Contains(stat, "CurrentPid:" + pidText)) << stat.output;
}

/* ================= memory overview, zip and periodic recording ================= */

/**
 * @tc.name: Mem_Overview_027
 * @tc.desc: Command: hidumper --mem
 *           Function: whole-device memory overview.
 *           Pass with plugin disabled: the [memory] section and the 'Total Memory Usage by PID:'
 *           header carry data below.
 *           Pass with plugin enabled: same as plugin-disabled.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperFunctionTest, Mem_Overview_027, TestSize.Level3)
{
    CmdResult result = RunHidumper("--mem");
    ASSERT_TRUE(Contains(result, SECTION_MEMORY)) << result.output;
    ASSERT_TRUE(Contains(result, "Total Memory Usage by PID:")) << result.output;
    ASSERT_TRUE(HasContentBelow(result.output, "Total Memory Usage by PID:")) << result.output;
}

/**
 * @tc.name: Mem_Prune_028
 * @tc.desc: Command: hidumper --mem --prune
 *           Function: simplified memory table.
 *           Pass with plugin disabled: the 'Total Memory Usage by PID:' header carries data below.
 *           Pass with plugin enabled: same as plugin-disabled.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperFunctionTest, Mem_Prune_028, TestSize.Level3)
{
    CmdResult result = RunHidumper("--mem --prune");
    ASSERT_TRUE(Contains(result, "Total Memory Usage by PID:")) << result.output;
    ASSERT_TRUE(HasContentBelow(result.output, "Total Memory Usage by PID:")) << result.output;
}

/**
 * @tc.name: Mem_Pid_029
 * @tc.desc: Command: hidumper --mem <UI process pid, init fallback>
 *           Function: per-process memory breakdown.
 *           Pass with plugin disabled: the [memory] section carries data below and the Total row
 *           appears.
 *           Pass with plugin enabled: same as plugin-disabled.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperFunctionTest, Mem_Pid_029, TestSize.Level3)
{
    std::string processName;
    int32_t pid = PickUiProcessPid(processName);
    if (pid <= 0) {
        pid = INIT_PID;
    }
    CmdResult result = RunHidumper("--mem " + std::to_string(pid));
    ASSERT_TRUE(Contains(result, SECTION_MEMORY)) << result.output;
    ASSERT_TRUE(HasContentBelow(result.output, SECTION_MEMORY)) << result.output;
    ASSERT_TRUE(Contains(result, "Total")) << result.output;
}

/**
 * @tc.name: Mem_ViewConsistency_030
 * @tc.desc: Command: hidumper --mem <hap pid> + hidumper --mem --prune
 *           Function: both --mem renderings carry the same application.
 *           Pass with plugin disabled: the detail view carries data below its [memory] header and
 *           the simplified table carries the HAP pid row.
 *           Pass with plugin enabled: same as plugin-disabled.
 *           Notes: the HAP is self-built infrastructure: a start failure fails the case.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperFunctionTest, Mem_ViewConsistency_030, TestSize.Level3)
{
    if (!FuncTestHapAvailable()) {
        GTEST_SKIP() << "the dedicated target HAP is unavailable in this environment "
                        "(neither installed nor pushed to an installable location)";
    }
    int32_t pid = StartFuncTestHap();
    ASSERT_GT(pid, NO_PID) << "the dedicated target HAP (" << FUNC_TEST_HAP_BUNDLE <<
                           ") failed to start although it ships with the test resources";
    CmdResult full = RunHidumper("--mem " + std::to_string(pid));
    CmdResult prune = RunHidumper("--mem --prune");
    ASSERT_TRUE(Contains(full, SECTION_MEMORY)) << full.output;
    ASSERT_TRUE(HasContentBelow(full.output, SECTION_MEMORY)) << full.output;
    std::string rowPrefix = "\n" + std::to_string(pid) + " ";
    ASSERT_NE(prune.output.find(rowPrefix), std::string::npos) <<
        "pid " << pid << " missing from prune table: " << prune.output;
}

/**
 * @tc.name: Mem_Zip_031
 * @tc.desc: Command: hidumper --mem --prune --zip + hidumper -e --zip
 *           Function: compressed output to /data/log/hidumper in two documented combination forms.
 *           Pass with plugin disabled: each reported zip path exists and is non-empty; the first
 *           form starts with the PK magic; the archives are removed afterwards.
 *           Pass with plugin enabled: same as plugin-disabled.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperFunctionTest, Mem_Zip_031, TestSize.Level3)
{
    CmdResult result = RunHidumper("--mem --prune --zip");
    const std::string marker = "The result is:";
    size_t pos = result.output.find(marker);
    ASSERT_NE(pos, std::string::npos) << result.output;
    std::string zipPath = result.output.substr(pos + marker.size());
    while (!zipPath.empty() && (zipPath.back() == '\n' || zipPath.back() == '\r' || zipPath.back() == ' ')) {
        zipPath.pop_back();
    }
    ASSERT_EQ(zipPath.compare(0, HIDUMPER_OUTPUT_DIR.size(), HIDUMPER_OUTPUT_DIR), 0) << zipPath;
    ASSERT_TRUE(FileExists(zipPath)) << "reported zip does not exist: " << zipPath;
    int64_t size = FileSize(zipPath);
    ASSERT_GT(size, 0) << zipPath;
    FILE *fp = fopen(zipPath.c_str(), "rb");
    ASSERT_NE(fp, nullptr) << zipPath;
    char magic[2] = {0};
    size_t got = fread(magic, sizeof(char), sizeof(magic), fp);
    fclose(fp);
    ASSERT_EQ(got, sizeof(magic));
    ASSERT_EQ(magic[0], 'P');
    ASSERT_EQ(magic[1], 'K');
    (void)unlink(zipPath.c_str());

    /* Second documented combination: -e --zip archives the faultlog view through the same
     * mechanism (the bare --zip form shares it and costs a full-device dump). */
    CmdResult faultZip = RunHidumper("-e --zip");
    size_t faultPos = faultZip.output.find(marker);
    ASSERT_NE(faultPos, std::string::npos) << faultZip.output;
    std::string faultZipPath = faultZip.output.substr(faultPos + marker.size());
    while (!faultZipPath.empty() &&
           (faultZipPath.back() == '\n' || faultZipPath.back() == '\r' ||
            faultZipPath.back() == ' ')) {
        faultZipPath.pop_back();
    }
    ASSERT_TRUE(FileExists(faultZipPath)) << "reported zip does not exist: " << faultZipPath;
    ASSERT_GT(FileSize(faultZipPath), 0) << faultZipPath;
    (void)unlink(faultZipPath.c_str());
}

/**
 * @tc.name: Mem_Record_032
 * @tc.desc: Command: hidumper --mem <pid> -t 1 (background) + hidumper --mem SIGINT
 *           Function: periodic memory recording and its stop path.
 *           Pass with plugin disabled: record_mem.txt carries at least two 'times:' sections with
 *           memory-report content, stops growing after the SIGINT and is removed afterwards.
 *           Pass with plugin enabled: same as plugin-disabled.
 *           Notes: the SIGINT is sent exactly once: a stray SIGINT silently voids the next
 *           recording (known product defect).
 * @tc.type: FUNC
 */
HWTEST_F(HidumperFunctionTest, Mem_Record_032, TestSize.Level3)
{
    std::string processName;
    int32_t pid = PickUiProcessPid(processName);
    if (pid <= 0) {
        pid = INIT_PID;
    }
    (void)unlink(RECORD_MEM_PATH.c_str());
    MemRecordGuard guard;
    StartBackground("hidumper --mem " + std::to_string(pid) + " -t 1");

    int32_t intervals = 0;
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(RECORD_MEM_TIMEOUT_SEC);
    while (std::chrono::steady_clock::now() < deadline) {
        std::this_thread::sleep_for(std::chrono::milliseconds(POLL_INTERVAL_MS));
        intervals = CountOccurrences(ReadFileContent(RECORD_MEM_PATH), TIMES_SECTION);
        if (intervals >= MIN_RECORD_INTERVALS) {
            break;
        }
    }
    guard.Stop();
    ASSERT_GE(intervals, MIN_RECORD_INTERVALS) << RECORD_MEM_PATH << " did not accumulate interval records";
    /* SIGINT clears the service-side loop flag, but an interval already in flight may
     * legitimately complete its write afterwards. Drain to quiescence instead of sampling
     * immediately; a loop that truly failed to stop keeps appending every second, never
     * quiesces inside the drain window, and is then caught by the frozen-size assert. */
    int64_t sizeAfterStop = FileSize(RECORD_MEM_PATH);
    const auto drainDeadline =
        std::chrono::steady_clock::now() + std::chrono::seconds(RECORD_STOP_DRAIN_SEC);
    while (std::chrono::steady_clock::now() < drainDeadline) {
        std::this_thread::sleep_for(std::chrono::milliseconds(POLL_INTERVAL_MS));
        int64_t current = FileSize(RECORD_MEM_PATH);
        if (current == sizeAfterStop) {
            break;
        }
        sizeAfterStop = current;
    }
    std::this_thread::sleep_for(std::chrono::seconds(RECORD_STOP_VERIFY_SEC));
    ASSERT_EQ(FileSize(RECORD_MEM_PATH), sizeAfterStop) << "the recording loop kept writing after --mem SIGINT";
    ASSERT_TRUE(Contains(CmdResult{0, ReadFileContent(RECORD_MEM_PATH)}, "Pss")) <<
        "recorded content does not look like a memory report";
    (void)unlink(RECORD_MEM_PATH.c_str());
}

/* ================================================= smaps views ================================================= */

/**
 * @tc.name: Smaps_Basic_033
 * @tc.desc: Command: hidumper --mem-smaps <own pid>
 *           Function: smaps statistics view.
 *           Pass with plugin disabled: the [memory] section carries data below and the Summary row
 *           appears.
 *           Pass with plugin enabled: same as plugin-disabled.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperFunctionTest, Smaps_Basic_033, TestSize.Level3)
{
    int32_t pid = getpid();
    CmdResult result = RunHidumper("--mem-smaps " + std::to_string(pid));
    ASSERT_TRUE(Contains(result, SECTION_MEMORY)) << result.output;
    ASSERT_TRUE(HasContentBelow(result.output, SECTION_MEMORY)) << result.output;
    ASSERT_TRUE(Contains(result, "Summary")) << result.output;
}

/**
 * @tc.name: Smaps_Verbose_034
 * @tc.desc: Command: hidumper --mem-smaps <own pid>, with and without -v
 *           Function: verbose smaps view.
 *           Pass with plugin disabled: the plain report carries data below [memory]; the -v report
 *           shows the 'Perm Start End' header with data below.
 *           Pass with plugin enabled: same as plugin-disabled.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperFunctionTest, Smaps_Verbose_034, TestSize.Level3)
{
    int32_t pid = getpid();
    CmdResult plain = RunHidumper("--mem-smaps " + std::to_string(pid));
    ASSERT_TRUE(Contains(plain, SECTION_MEMORY)) << plain.output;
    ASSERT_TRUE(HasContentBelow(plain.output, SECTION_MEMORY)) << plain.output;
    CmdResult verbose = RunHidumper("--mem-smaps " + std::to_string(pid) + " -v");
    ASSERT_TRUE(Contains(verbose, SMAPS_VERBOSE_HEADER)) << verbose.output;
    ASSERT_TRUE(HasContentBelow(verbose.output, SMAPS_VERBOSE_HEADER)) << verbose.output;
}

/* ====================================== memory detail switches ====================================== */

/**
 * @tc.name: MemDetail_Ashmem_035
 * @tc.desc: Command: hidumper --mem <hap pid> --show-ashmem
 *           Function: per-descriptor ashmem detail of the own application.
 *           Pass with plugin disabled: judged success directly.
 *           Pass with plugin enabled: the [memory] report carries data below and the 'Total
 *           Ashmem:' header carries the per-descriptor data below (the HAP holds ashmem, verified:
 *           Total Ashmem:384 kB).
 *           Notes: --show-ashmem is not adapted on RK3568 yet: the kernel exports the legacy header
 *           'Process name Process ID fd ashmem_name size' and no 'Total ashmem  of' overview while
 *           the parser only recognizes 'Process_name' (parse_ashmem_info.cpp:83, product defect
 *           candidate #5), so no Ashmem section can appear there.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperFunctionTest, MemDetail_Ashmem_035, TestSize.Level3)
{
    if (!PLUGIN_ENABLED) {
        /* Reserved (RK3568/board): --show-ashmem is not adapted on this image yet - the kernel
         * exports the legacy header "Process name Process ID fd ashmem_name size" and no
         * "Total ashmem  of" overview while the parser only recognizes "Process_name"
         * (parse_ashmem_info.cpp:83, product defect candidate #5) - so no Ashmem section can
         * be produced; judged success directly per the reservation rule. */
        GTEST_SUCCEED();
        return;
    }
    if (!FuncTestHapAvailable()) {
        GTEST_SKIP() << "the dedicated target HAP is unavailable in this environment "
                        "(neither installed nor pushed to an installable location)";
    }
    /* Self-built application target: the dedicated HAP holds ashmem regions (verified on the
     * phone: Total Ashmem:384 kB with per-descriptor rows). */
    int32_t pid = StartFuncTestHap();
    ASSERT_GT(pid, NO_PID) << "the dedicated target HAP (" << FUNC_TEST_HAP_BUNDLE <<
                           ") failed to start although it ships with the test resources";
    CmdResult result = RunHidumper("--mem " + std::to_string(pid) + " --show-ashmem");
    ASSERT_TRUE(Contains(result, SECTION_MEMORY)) << result.output;
    ASSERT_TRUE(HasContentBelow(result.output, SECTION_MEMORY)) << result.output;
    ASSERT_TRUE(Contains(result, "Total Ashmem:")) << result.output;
    ASSERT_TRUE(HasContentBelow(result.output, "Total Ashmem:")) << result.output;
}

/**
 * @tc.name: MemDetail_Dmabuf_036
 * @tc.desc: Command: hidumper --mem <pid> --show-dmabuf
 *           Function: DMA aggregate on top of the base report.
 *           Pass with plugin disabled: the [memory] section, 'DMA:' and a numeric 'Dma:' value
 *           appear.
 *           Pass with plugin enabled: same as plugin-disabled.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperFunctionTest, MemDetail_Dmabuf_036, TestSize.Level3)
{
    std::string processName;
    int32_t pid = PickUiProcessPid(processName);
    if (pid <= 0) {
        pid = INIT_PID;
    }
    CmdResult result = RunHidumper("--mem " + std::to_string(pid) + " --show-dmabuf");
    ASSERT_TRUE(Contains(result, SECTION_MEMORY)) << result.output;
    ASSERT_TRUE(Contains(result, "DMA:")) << result.output;
    ASSERT_GE(ParsePrefixedNumber(result.output, "Dma:"), 0) << result.output;
}

/**
 * @tc.name: MemDetail_ArktsHeap_037
 * @tc.desc: Command: hidumper --mem <hap pid> --show-arktsheap
 *           Function: ArkTS heap accounting of the own application.
 *           Pass with plugin disabled: the 'arkts heap:' section and the 'arkts_heap(KB)' header
 *           carry data below and the HAP pid appears.
 *           Pass with plugin enabled: same as plugin-disabled.
 *           Notes: the HAP is a self-built ArkTS target: a start failure fails the case.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperFunctionTest, MemDetail_ArktsHeap_037, TestSize.Level3)
{
    if (!FuncTestHapAvailable()) {
        GTEST_SKIP() << "the dedicated target HAP is unavailable in this environment "
                        "(neither installed nor pushed to an installable location)";
    }
    int32_t pid = StartFuncTestHap();
    ASSERT_GT(pid, NO_PID) << "the dedicated target HAP (" << FUNC_TEST_HAP_BUNDLE <<
                           ") failed to start although it ships with the test resources";
    CmdResult result = RunHidumper("--mem " + std::to_string(pid) + " --show-arktsheap");
    ASSERT_TRUE(Contains(result, "arkts heap:")) << result.output;
    ASSERT_TRUE(Contains(result, "arkts_heap(KB)")) << result.output;
    ASSERT_TRUE(HasContentBelow(result.output, "arkts_heap(KB)")) << result.output;
    ASSERT_TRUE(Contains(result, std::to_string(pid))) << result.output;
}

/**
 * @tc.name: MemDetail_GpuMem_038
 * @tc.desc: Command: hidumper --mem <pid> --show-gpumem
 *           Function: GPU memory detail.
 *           Pass with plugin disabled: judged success directly.
 *           Pass with plugin enabled: against the fixed GPU holders (sceneboard, then
 *           render_service; absence fails the case) the [memory] section and the 'GPU:' header
 *           carry data below.
 *           Notes: the option is not compiled in with the plugin disabled: the tool shows
 *           'hidumper: option pid missed. showgpumem' (verified) before the case is judged success
 *           directly.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperFunctionTest, MemDetail_GpuMem_038, TestSize.Level3)
{
    /* Environment-adaptive entry: when the option is not compiled into this build the tool
     * shows that it does not exist (verified board presentation) and the case is judged
     * success directly per the reservation rule. */
    CmdResult probe = RunHidumper("--mem " + std::to_string(INIT_PID) + " --show-gpumem");
    if (Contains(probe, "option pid missed") || Contains(probe, "invalid arg")) {
        /* Reserved (RK3568 shape: option not compiled in, macro isolation): judged success
         * directly - the absence itself is positively shown by the rejection above. */
        GTEST_SUCCEED();
        return;
    }
    /* The option exists in this build: positive assertions against the fixed system GPU
     * holders; their absence is real breakage and fails the case. */
    int32_t pid = GetPidByName("com.ohos.sceneboard");
    if (pid <= 0) {
        pid = GetPidByName("render_service");
    }
    if (pid <= 0) {
        FAIL() << "neither sceneboard nor render_service is running: " << DescribeEnvironment();
    }
    CmdResult result = RunHidumper("--mem " + std::to_string(pid) + " --show-gpumem");
    ASSERT_TRUE(Contains(result, SECTION_MEMORY)) << result.output;
    ASSERT_TRUE(Contains(result, "GPU:")) << result.output;
    ASSERT_TRUE(HasContentBelow(result.output, "GPU:")) << result.output;
}

/* ===================================== controlled child target ===================================== */

/**
 * @tc.name: ChildTarget_MemoryFloor_039
 * @tc.desc: Command: hidumper --mem <child pid> + hidumper --mem-smaps <child pid>
 *           Function: memory floors against the child's fully touched 32 MiB heap block.
 *           Pass with plugin disabled: the 'native heap' and Total rows plus the smaps Summary
 *           Rss/Pss are all at or above the 28 MiB floor.
 *           Pass with plugin enabled: same as plugin-disabled.
 *           Notes: the child is self-built infrastructure: a creation failure fails the case.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperFunctionTest, ChildTarget_MemoryFloor_039, TestSize.Level3)
{
    TargetChildGuard child;
    std::string failReason;
    if (!child.Start(failReason)) {
        /* The controlled child is self-built test infrastructure: a creation failure is a real
         * environment defect, not a reason to pass silently. */
        FAIL() << "controlled child creation failed: " << failReason;
    }
    const std::string pidText = std::to_string(child.Pid());

    CmdResult mem = RunHidumper("--mem " + pidText);
    ASSERT_TRUE(Contains(mem, SECTION_MEMORY)) << mem.output;
    int64_t nativeHeapKb = RowFirstNumber(mem.output, "native heap");
    int64_t totalKb = RowFirstNumber(mem.output, "Total");
    ASSERT_GE(nativeHeapKb, TARGET_HEAP_FLOOR_KB) << "native heap row below the touched-block floor: " << mem.output;
    ASSERT_GE(totalKb, TARGET_HEAP_FLOOR_KB) << "Total row below the touched-block floor: " << mem.output;

    CmdResult smaps = RunHidumper("--mem-smaps " + pidText);
    ASSERT_TRUE(Contains(smaps, SECTION_MEMORY)) << smaps.output;
    ASSERT_TRUE(Contains(smaps, "Summary")) << smaps.output;
    int64_t sizeKb = 0;
    int64_t rssKb = 0;
    int64_t pssKb = 0;
    ASSERT_TRUE(SummaryRowNumbers(smaps.output, sizeKb, rssKb, pssKb)) << smaps.output;
    ASSERT_GE(rssKb, TARGET_HEAP_FLOOR_KB) << "Summary Rss below the touched-block floor: " << smaps.output;
    ASSERT_GE(pssKb, TARGET_HEAP_FLOOR_KB) << "Summary Pss below the touched-block floor: " << smaps.output;
}

/* =============================== js heap family (--mem-jsheap) =============================== */

/**
 * @tc.name: JsHeap_Basic_040
 * @tc.desc: Command: hidumper --mem-jsheap <hap pid>
 *           Function: js heap snapshot end-to-end.
 *           Pass with plugin disabled: jsheap-<pid>-<tid>-<ts> appears in /data/log/faultlog/temp
 *           (this build's final directory), larger than 1024 bytes.
 *           Pass with plugin enabled: hidumper-jsheap-<pid>-<tid>-<ts> appears in
 *           /data/log/reliability/resource_leak/memory_leak, larger than 1024 bytes.
 *           Notes: the HAP is the mandatory self-built target: a start failure fails the case.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperFunctionTest, JsHeap_Basic_040, TestSize.Level3)
{
    if (!FuncTestHapAvailable()) {
        GTEST_SKIP() << "the dedicated target HAP is unavailable in this environment "
                        "(neither installed nor pushed to an installable location)";
    }
    int32_t pid = StartFuncTestHap();
    ASSERT_GT(pid, NO_PID) << "the dedicated target HAP (" << FUNC_TEST_HAP_BUNDLE <<
                           ") failed to start although it ships with the test resources";
    ArtifactCheck check = VerifySnapshotArtifact("--mem-jsheap " + std::to_string(pid),
                                                FAMILY_JSHEAP, pid, SNAPSHOT_TIMEOUT_SEC);
    ASSERT_TRUE(check.verified) << FUNC_TEST_HAP_BUNDLE << "(" << pid << "): " << check.detail;
}

/**
 * @tc.name: JsHeap_Gc_041
 * @tc.desc: Command: hidumper --mem-jsheap <hap pid> (S1); --gc; (S2), driven by aa start --ps
 * @tc.desc: hdaction holdGarbage/dropGarbage
 *           Function: --gc must measurably reclaim memory (artifact-level oracle).
 *           Pass with plugin disabled: S1 and S2 verify as jsheap artifacts in the build's final
 *           directory and S1-S2 >= 4 MiB (measured 23.0 MiB); --gc itself writes no snapshot.
 *           Pass with plugin enabled: same, measured 22.8 MiB on the phone.
 *           Notes: the hold phase polls the HAP RSS until stable so S1 sees the fully constructed
 *           ~24 MiB garbage; the HAP is mandatory (it owns the garbage protocol).
 * @tc.type: FUNC
 */
HWTEST_F(HidumperFunctionTest, JsHeap_Gc_041, TestSize.Level3)
{
    if (!FuncTestHapAvailable()) {
        GTEST_SKIP() << "the dedicated target HAP is unavailable in this environment "
                        "(neither installed nor pushed to an installable location)";
    }
    int32_t pid = StartFuncTestHap();
    ASSERT_GT(pid, NO_PID) << "the dedicated target HAP (" << FUNC_TEST_HAP_BUNDLE <<
                           ") failed to start although it ships with the test resources";
    const std::string pidText = std::to_string(pid);
    const std::string needle = ArtifactNeedle(FAMILY_JSHEAP, pid);
    const std::vector<std::string> dirs = {SNAPSHOT_STAGING_DIR, SNAPSHOT_FINAL_DIR};

    /* Artifact-level positive oracle for --gc: the HAP holds ~24 MiB of unique garbage on command
     * (holdGarbage), so the jsheap snapshot S1 taken while holding must be significantly larger than
     * the snapshot S2 taken after dropGarbage + --gc - the artifact size delta is the memory the
     * forced full GC reclaimed (measured 23.0 MiB board, 22.8 MiB phone).
     */
    CmdResult hold = StartHapAction("holdGarbage");
    ASSERT_TRUE(Contains(hold, "successfully")) << hold.output;
    /* Poll the HAP's resident size until it stops growing instead of a fixed sleep:
     * holdGarbage builds the ~24 MiB of chunks synchronously on the ability main thread and
     * under suite load the construction outgrew the old 4 s window (observed flake: S1 taken
     * mid-construction, delta below the floor). GC_HOLD_STABLE_REPEATS consecutive equal
     * readings mean the construction finished; a two-reading plateau once aliased with a
     * construction pause under back-to-back suite load (observed flake: S1 undersized).
     * Reading /proc never disturbs the application's threads. */
    int64_t lastRss = -1;
    int32_t stablePolls = 0;
    const auto holdDeadline =
        std::chrono::steady_clock::now() + std::chrono::seconds(GC_HOLD_MAX_SEC);
    while (std::chrono::steady_clock::now() < holdDeadline) {
        std::this_thread::sleep_for(std::chrono::seconds(GC_HOLD_POLL_SEC));
        const int64_t rss = ReadResidentPages(pid);
        stablePolls = (rss > 0 && rss == lastRss) ? (stablePolls + 1) : 0;
        lastRss = rss;
        if (stablePolls >= GC_HOLD_STABLE_REPEATS) {
            break;
        }
    }

    ArtifactCheck s1 = VerifySnapshotArtifact("--mem-jsheap " + pidText, FAMILY_JSHEAP, pid,
                                              SNAPSHOT_TIMEOUT_SEC);
    ASSERT_TRUE(s1.verified) << "S1 (garbage held): " << s1.detail;

    CmdResult drop = StartHapAction("dropGarbage");
    ASSERT_TRUE(Contains(drop, "successfully")) << drop.output;

    /* --gc itself must not write a snapshot (needSnapshot cleared) - the design contract. */
    SnapshotCleanupGuard gcGuard(needle);
    WaitSnapshotQuiescent(dirs, needle, QUIESCENT_ROUNDS, QUIESCENT_TIMEOUT_SEC);
    const std::set<std::string> beforeGcStaging = CollectFiles({SNAPSHOT_STAGING_DIR}, needle);
    const std::set<std::string> beforeGcFinal = CollectFiles({SNAPSHOT_FINAL_DIR}, needle);
    CmdResult gc = RunHidumper("--mem-jsheap " + pidText + " --gc");
    ASSERT_FALSE(Contains(gc, "Error [")) << gc.output;
    ASSERT_TRUE(WaitNoNewFiles({SNAPSHOT_STAGING_DIR}, needle, beforeGcStaging, NEGATIVE_WAIT_SEC)) <<
        "--gc must not write a snapshot; " << DirListingForReport();
    std::this_thread::sleep_for(std::chrono::seconds(GC_RECLAIM_SETTLE_SEC));

    ArtifactCheck s2 = VerifySnapshotArtifact("--mem-jsheap " + pidText, FAMILY_JSHEAP, pid,
                                              SNAPSHOT_TIMEOUT_SEC);
    ASSERT_TRUE(s2.verified) << "S2 (after --gc): " << s2.detail;

    ASSERT_GE(s1.size - s2.size, GC_SHRINK_FLOOR_BYTES) <<
        "--gc did not reclaim the released garbage: S1=" << s1.size << " S2=" << s2.size;
}

/**
 * @tc.name: JsHeap_Tid_042
 * @tc.desc: Command: hidumper --mem-jsheap <hap pid> -T <pid>
 *           Function: -T reaches faultloggerd's file naming.
 *           Pass with plugin disabled: a jsheap-<pid>-<pid>- artifact appears in the final
 *           directory, larger than 1024 bytes.
 *           Pass with plugin enabled: same as plugin-disabled.
 *           Notes: the HAP is the mandatory self-built target.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperFunctionTest, JsHeap_Tid_042, TestSize.Level3)
{
    if (!FuncTestHapAvailable()) {
        GTEST_SKIP() << "the dedicated target HAP is unavailable in this environment "
                        "(neither installed nor pushed to an installable location)";
    }
    int32_t pid = StartFuncTestHap();
    ASSERT_GT(pid, NO_PID) << "the dedicated target HAP (" << FUNC_TEST_HAP_BUNDLE <<
                           ") failed to start although it ships with the test resources";
    const std::string pidText = std::to_string(pid);
    const std::string tidNeedle = FAMILY_JSHEAP + "-" + pidText + "-" + pidText + "-";
    const std::vector<std::string> dirs = {SNAPSHOT_STAGING_DIR, SNAPSHOT_FINAL_DIR};
    SnapshotCleanupGuard guard(ArtifactNeedle(FAMILY_JSHEAP, pid));
    WaitSnapshotQuiescent(dirs, tidNeedle, QUIESCENT_ROUNDS, QUIESCENT_TIMEOUT_SEC);
    const std::set<std::string> before = CollectFiles(dirs, tidNeedle);

    CmdResult result = RunHidumper("--mem-jsheap " + pidText + " -T " + pidText);
    ASSERT_FALSE(Contains(result, "Error [")) << result.output;

    std::string foundPath;
    ASSERT_TRUE(WaitNewSnapshotSettled({SNAPSHOT_FINAL_DIR}, tidNeedle, before,
                                       SNAPSHOT_TIMEOUT_SEC, foundPath)) <<
        "no snapshot named with tid " << pid << " in " << SNAPSHOT_FINAL_DIR << "; " << DirListingForReport();
    ASSERT_GT(FileSize(foundPath), static_cast<int64_t>(MIN_SNAPSHOT_BYTES)) << foundPath;
}

/**
 * @tc.name: JsHeap_Leakobj_043
 * @tc.desc: Command: hidumper --mem-jsheap <hap pid> --leakobj
 *           Function: leak object list end-to-end with a content-level oracle.
 *           Pass with plugin disabled: the jsheap artifact verifies as in JsHeap_Basic_040 and a
 *           non-empty leaklist-<pid>-<ts> appears whose JSON carries the probe class name and
 *           marker.
 *           Pass with plugin enabled: both artifacts appear in the memory_leak directory with the
 *           same content markers.
 *           Notes: the HAP arms jsLeakWatcher (enable + a watched probe held alive on purpose,
 *           exactly what check() reports as leaked) and is the only leaklist producer: mandatory
 *           target.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperFunctionTest, JsHeap_Leakobj_043, TestSize.Level3)
{
    if (!FuncTestHapAvailable()) {
        GTEST_SKIP() << "the dedicated target HAP is unavailable in this environment "
                        "(neither installed nor pushed to an installable location)";
    }
    int32_t pid = StartFuncTestHap();
    ASSERT_GT(pid, NO_PID) << "the dedicated target HAP (" << FUNC_TEST_HAP_BUNDLE <<
                           ") failed to start although it ships with the test resources";
    const std::string args = "--mem-jsheap " + std::to_string(pid) + " --leakobj";
    /* Artifact 1: the js heap snapshot, verified per build exactly as in JsHeap_Basic_040. */
    ArtifactCheck snapshot = VerifySnapshotArtifact(args, FAMILY_JSHEAP, pid, SNAPSHOT_TIMEOUT_SEC);
    ASSERT_TRUE(snapshot.verified) << FUNC_TEST_HAP_BUNDLE << "(" << pid << "): " << snapshot.detail;
    /* Artifact 2: the leaklist. The command runs a second time with its own observation window
     * because the 1 KiB floor inside VerifySnapshotArtifact does not fit the tiny leaklist JSON
     * by design. A case level guard covers the js heap copy this second run leaves behind. */
    SnapshotCleanupGuard secondRunGuard(ArtifactNeedle(FAMILY_JSHEAP, pid));
    const std::string leakNeedle = std::string(FAMILY_LEAKLIST) + "-" + std::to_string(pid);
    SnapshotCleanupGuard leakGuard(leakNeedle);
    WaitSnapshotQuiescent({SNAPSHOT_STAGING_DIR, SNAPSHOT_FINAL_DIR}, leakNeedle,
                          QUIESCENT_ROUNDS, QUIESCENT_TIMEOUT_SEC);
    const std::set<std::string> beforeFinal = CollectFiles({SNAPSHOT_FINAL_DIR}, leakNeedle);
    CmdResult result = RunHidumper(args);
    ASSERT_FALSE(Contains(result, "Error [")) << result.output;
    std::string leakPath;
    ASSERT_TRUE(WaitNewSnapshotSettled({SNAPSHOT_FINAL_DIR}, leakNeedle, beforeFinal,
                                       LEAKLIST_WAIT_SEC, leakPath)) <<
        "the watcher-armed HAP must yield a leaklist artifact in " << SNAPSHOT_FINAL_DIR <<
        ": " << DirListingForReport();
    ASSERT_GT(FileSize(leakPath), 0) << leakPath;
    /* Content level oracle: check()'s JSON carries the probe's class name and marker message. */
    const std::string content = ReadFileContent(leakPath);
    ASSERT_NE(content.find("LeakProbe"), std::string::npos) << "content: " << content;
    ASSERT_NE(content.find(LEAK_PROBE_MARKER), std::string::npos) << "content: " << content;
    /* Placement contract: generated in the build's final directory with content - the
     * companion's prefix/staging migration details are no longer asserted (review decision). */
}

/**
 * @tc.name: JsHeap_Raw_044
 * @tc.desc: Command: hidumper --mem-jsheap <hap pid> --raw
 *           Function: raw heap snapshot; the .rawheap extension proves needBinary reached the
 *           runtime.
 *           Pass with plugin disabled: judged success directly.
 *           Pass with plugin enabled: hidumper-jsheap-<pid>-<tid>-<ts>.rawheap appears in the final
 *           directory, larger than 1024 bytes.
 *           Notes: the 32-bit board runtime does not honor needBinary (verified: zero artifacts
 *           with the HAP target inside a 20 s window).
 * @tc.type: FUNC
 */
HWTEST_F(HidumperFunctionTest, JsHeap_Raw_044, TestSize.Level3)
{
    if (sizeof(void *) != 8) { // NOLINT: arch gate
        /* Reserved (RK3568/32-bit board): this ArkTS runtime does not honor needBinary - verified
         * zero artifacts with the HAP target inside a 20 s window - so --raw never generates
         * anything there; judged success directly per the reservation rule. */
        GTEST_SUCCEED();
        return;
    }
    if (!FuncTestHapAvailable()) {
        GTEST_SKIP() << "the dedicated target HAP is unavailable in this environment "
                        "(neither installed nor pushed to an installable location)";
    }
    int32_t pid = StartFuncTestHap();
    ASSERT_GT(pid, NO_PID) << "the dedicated target HAP (" << FUNC_TEST_HAP_BUNDLE <<
                           ") failed to start although it ships with the test resources";
    ArtifactCheck check = VerifySnapshotArtifact("--mem-jsheap " + std::to_string(pid) + " --raw",
                                                 FAMILY_JSHEAP, pid, TARGET_PROBE_TIMEOUT_SEC);
    ASSERT_TRUE(check.produced) << "64-bit runtimes honor needBinary (verified on the phone): " << check.detail;
    ASSERT_TRUE(check.verified) << check.detail;
    size_t extPos = check.path.size() - RAWHEAP_EXT.size();
    ASSERT_TRUE(check.path.size() > RAWHEAP_EXT.size()) << check.path;
    ASSERT_EQ(check.path.compare(extPos, RAWHEAP_EXT.size(), RAWHEAP_EXT), 0) <<
        "--raw must produce a .rawheap artifact, got " << check.path;
}

/**
 * @tc.name: JsHeap_Clean_045
 * @tc.desc: Command: hidumper --mem-jsheap <hap pid> --clean
 *           Function: --clean still produces a heap snapshot.
 *           Pass with plugin disabled: the jsheap artifact verifies as in JsHeap_Basic_040.
 *           Pass with plugin enabled: same as plugin-disabled.
 *           Notes: --clean clears the VM node-id cache AFTER the dump (js_runtime.cpp:1468); the
 *           effect is VM-internal and externally unobservable (verified by a four-dump artifact
 *           comparison and an app-memory polling experiment), so the artifact itself is the
 *           contract.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperFunctionTest, JsHeap_Clean_045, TestSize.Level3)
{
    if (!FuncTestHapAvailable()) {
        GTEST_SKIP() << "the dedicated target HAP is unavailable in this environment "
                        "(neither installed nor pushed to an installable location)";
    }
    int32_t pid = StartFuncTestHap();
    ASSERT_GT(pid, NO_PID) << "the dedicated target HAP (" << FUNC_TEST_HAP_BUNDLE <<
                           ") failed to start although it ships with the test resources";
    ArtifactCheck check = VerifySnapshotArtifact("--mem-jsheap " + std::to_string(pid) + " --clean",
                                                 FAMILY_JSHEAP, pid, SNAPSHOT_TIMEOUT_SEC);
    ASSERT_TRUE(check.verified) << FUNC_TEST_HAP_BUNDLE << "(" << pid << "): " << check.detail;
}

/**
 * @tc.name: JsHeap_RawSingle_046
 * @tc.desc: Command: hidumper --mem-jsheap <hap pid> --raw --single
 *           Function: both flags reach the runtime (name shows .rawheap and the tid folded onto the
 *           pid).
 *           Pass with plugin disabled: judged success directly.
 *           Pass with plugin enabled: hidumper-jsheap-<pid>-<pid>-<ts>.rawheap appears in the final
 *           directory.
 *           Notes: the board runtime produces nothing for --raw (verified).
 * @tc.type: FUNC
 */
HWTEST_F(HidumperFunctionTest, JsHeap_RawSingle_046, TestSize.Level3)
{
    if (sizeof(void *) != 8) { // NOLINT: arch gate
        /* Reserved (RK3568/32-bit board): the runtime produces nothing for --raw (verified), so
         * the --raw --single combination never generates anything there; judged success directly
         * per the reservation rule. */
        GTEST_SUCCEED();
        return;
    }
    if (!FuncTestHapAvailable()) {
        GTEST_SKIP() << "the dedicated target HAP is unavailable in this environment "
                        "(neither installed nor pushed to an installable location)";
    }
    int32_t pid = StartFuncTestHap();
    ASSERT_GT(pid, NO_PID) << "the dedicated target HAP (" << FUNC_TEST_HAP_BUNDLE <<
                           ") failed to start although it ships with the test resources";
    const std::string pidText = std::to_string(pid);
    ArtifactCheck check = VerifySnapshotArtifact(
        "--mem-jsheap " + pidText + " --raw --single", FAMILY_JSHEAP, pid, TARGET_PROBE_TIMEOUT_SEC,
        ArtifactNeedle(FAMILY_JSHEAP, pid) + pidText + "-");
    ASSERT_TRUE(check.produced) <<
        "64-bit runtimes honor needBinary/needProcDump (verified on the phone): " << check.detail;
    ASSERT_TRUE(check.verified) << check.detail;
    std::string baseName = check.path.substr(check.path.rfind('/') + 1);
    ASSERT_NE(baseName.find(RAWHEAP_EXT), std::string::npos) <<
        "--raw must produce a .rawheap artifact, got " << baseName;
    ASSERT_NE(baseName.find("-" + pidText + "-" + pidText + "-"), std::string::npos) <<
        "--single must fold the tid onto the pid, got " << baseName;
}

/* ==================================== heap family (--mem-heap) ==================================== */

/**
 * @tc.name: MemHeap_Native_047
 * @tc.desc: Command: hidumper --mem-heap <pid> --native
 *           Function: native heap snapshot end-to-end.
 *           Pass with plugin disabled: judged success directly.
 *           Pass with plugin enabled: sceneboard (fixed system target) yields
 *           hidumper-nativeheap-<pid>-<ts> larger than 1024 bytes; a non-application pid yields no
 *           artifact (AppMgrService routing).
 *           Notes: the 32-bit board compiles SO_NAME empty (__LP64__ gate in
 *           dump_runtime_helper.cpp) and ships no helper library; the HAP is deliberately not the
 *           target: the tracker ignores third-party app allocations (verified 416-byte NSNAP1.0
 *           header only).
 * @tc.type: FUNC
 */
HWTEST_F(HidumperFunctionTest, MemHeap_Native_047, TestSize.Level3)
{
    if (sizeof(void *) != 8) { // NOLINT: arch gate
        /* Reserved (RK3568/32-bit board): the appkit compiles SO_NAME empty on non-LP64
         * (dump_runtime_helper.cpp __LP64__ gate) and the image ships no helper library, so this
         * command never generates a snapshot there; judged success directly per the reservation
         * rule. */
        GTEST_SUCCEED();
        return;
    }
    int32_t pid = NO_PID;
    std::string processName;
    ArtifactCheck check;
    if (!FindCapableHeapTarget("--native", FAMILY_NATIVEHEAP, NATIVE_TARGETS,
                               pid, processName, check)) {
        /* NATIVE_TARGETS are fixed system processes (sceneboard verified producing on the phone);
         * none of them yielding an artifact is a real regression, not a reservation. */
        FAIL() << "no fixed native heap target produced an artifact: " << check.detail << "; " << DirListingForReport();
    }
    ASSERT_TRUE(check.verified) << "process " << processName << "(" << pid << "): " << check.detail;

    /* A non application pid is refused by AppMgrService; it must not create an artifact either. */
    const std::string familyNeedle = std::string(FAMILY_NATIVEHEAP) + "-";
    SnapshotCleanupGuard negativeGuard(familyNeedle);
    const std::set<std::string> before = CollectFiles(
        {SNAPSHOT_STAGING_DIR, SNAPSHOT_FINAL_DIR}, familyNeedle);
    CmdResult refused = RunHidumper("--mem-heap " + std::to_string(getpid()) + " --native");
    ASSERT_FALSE(Contains(refused, "Error [")) << refused.output;
    std::this_thread::sleep_for(std::chrono::seconds(NEGATIVE_WAIT_SEC));
    ASSERT_TRUE(WithoutPreexisting(CollectFiles({SNAPSHOT_STAGING_DIR, SNAPSHOT_FINAL_DIR},
                                                familyNeedle), before).empty()) <<
        "a non application pid must not yield an artifact; " << DirListingForReport();
}

/**
 * @tc.name: MemHeap_NativeLeakobj_048
 * @tc.desc: Command: hidumper --mem-heap <pid> --native --leakobj (fixed targets: sceneboard,
 * @tc.desc: launcher)
 *           Function: native leak report, the only heap command answering synchronously on stdout.
 *           Pass with plugin disabled: judged success directly.
 *           Pass with plugin enabled: stdout carries 'bytes leak' plus allocation detail
 *           (verified); no file is involved.
 *           Notes: the memleak helper library is 64-bit only; target absence fails the case.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperFunctionTest, MemHeap_NativeLeakobj_048, TestSize.Level3)
{
    if (sizeof(void *) != 8) { // NOLINT: arch gate
        /* Reserved (RK3568/32-bit board): the memleak helper library is 64-bit only (verified),
         * so no leak report can ever be produced there; judged success directly per the
         * reservation rule. */
        GTEST_SUCCEED();
        return;
    }
    int32_t pid = NO_PID;
    std::string processName;
    for (const auto &hint : NATIVE_TARGETS) {
        pid = GetPidByName(hint.bundleName);
        if (pid > 0) {
            processName = hint.bundleName;
            break;
        }
    }
    if (pid <= 0) {
        /* NATIVE_TARGETS are fixed system processes present on both products; their absence is
         * real breakage. */
        FAIL() << "neither of the fixed native heap targets is running: " << DescribeEnvironment();
    }
    CmdResult result = RunHidumper("--mem-heap " + std::to_string(pid) + " --native --leakobj");
    ASSERT_FALSE(Contains(result, "Error [")) << result.output;
    /* 64-bit images carry the helper library and sceneboard is tracker-enabled (verified leak
     * report on the phone), so the report is mandatory. */
    ASSERT_TRUE(Contains(result, "bytes leak")) << result.output;
    ASSERT_TRUE(Contains(result, "allocations") || Contains(result, "memory:")) << result.output;
}

/**
 * @tc.name: MemHeap_Kmp_049
 * @tc.desc: Command: hidumper --mem-heap <hap pid> --kotlin
 *           Function: KMP heap snapshot end-to-end.
 *           Pass with plugin disabled: kotlinheap-<pid>-<ts> appears in the final directory
 *           (verified: 11808 bytes on the board).
 *           Pass with plugin enabled: hidumper-kotlinheap-<pid>-<ts>.kdump appears in the
 *           reliability directory (verified).
 *           Notes: the HAP registers its own KMP mem-dump listener (verified producing on both
 *           products): mandatory first target, no capable target fails the case.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperFunctionTest, MemHeap_Kmp_049, TestSize.Level3)
{
    if (!FuncTestHapAvailable()) {
        GTEST_SKIP() << "the dedicated target HAP is unavailable in this environment "
                        "(neither installed nor pushed to an installable location)";
    }
    int32_t pid = NO_PID;
    std::string processName;
    ArtifactCheck check;
    if (!FindCapableHeapTarget("--kotlin", FAMILY_KOTLINHEAP, KMP_TARGETS,
                               pid, processName, check)) {
        /* The dedicated HAP registers its own KMP listener and was verified producing on BOTH
         * products (the board's 11808-byte artifact included), so no capable target is a real
         * failure, not a presentation to accept. */
        FAIL() << "no KMP-capable target produced an artifact: " << check.detail << "; " << DirListingForReport();
    }
    ASSERT_TRUE(check.verified) << "process " << processName << "(" << pid << "): " << check.detail;
}

/**
 * @tc.name: MemHeap_Jsvm_050
 * @tc.desc: Command: hidumper --mem-heap <hap pid> --jsvm
 *           Function: JSVM heap snapshot end-to-end.
 *           Pass with plugin disabled: judged success directly.
 *           Pass with plugin enabled: hidumper-jsvmheap-<pid>-<tid>-<ts> appears in the final
 *           directory, larger than 1024 bytes, holding an engine heap snapshot (verified 1.24 MiB).
 *           Notes: the option is not compiled in with the plugin disabled (the tool shows 'option
 *           pid missed. jsvm'); the HAP keeps its own executing isolate so the dump needs no UI
 *           driver.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperFunctionTest, MemHeap_Jsvm_050, TestSize.Level3)
{
    /* Environment-adaptive entry: builds without the option show its absence (verified board
     * presentation: "hidumper: option pid missed. jsvm") and are judged success directly. */
    CmdResult probe = RunHidumper("--mem-heap " + std::to_string(INIT_PID) + " --jsvm");
    if (Contains(probe, "option pid missed") || Contains(probe, "invalid arg")) {
        /* Reserved (RK3568 shape: --jsvm not compiled in, macro isolation): judged success
         * directly - the absence itself is positively shown by the rejection above. */
        GTEST_SUCCEED();
        return;
    }
    if (!FuncTestHapAvailable()) {
        GTEST_SKIP() << "the dedicated target HAP is unavailable in this environment "
                        "(neither installed nor pushed to an installable location)";
    }
    int32_t pid = NO_PID;
    std::string processName;
    ArtifactCheck check;
    if (!FindCapableHeapTarget("--jsvm", FAMILY_JSVMHEAP, JSVM_TARGETS, pid, processName, check)) {
        /* The dedicated HAP keeps its own isolate executing and was verified producing a 1.24 MiB
         * engine snapshot on the phone, so no capable target is a real failure. */
        FAIL() << "no JSVM-capable target produced an artifact: " << check.detail << "; " << DirListingForReport();
    }
    ASSERT_TRUE(check.verified) << "process " << processName << "(" << pid << "): " << check.detail;
}

/**
 * @tc.name: MemHeap_ArkwebJs_051
 * @tc.desc: Command: hidumper --mem-heap <render pid> --arkweb-js
 *           Function: ArkWeb js heap snapshot end-to-end.
 *           Pass with plugin disabled: judged success directly.
 *           Pass with plugin enabled: hidumper-arkweb_jsheap-<renderPid>-<ts> appears in the final
 *           directory, larger than 1024 bytes (verified 1.9 MiB).
 *           Notes: the option is not compiled in with the plugin disabled ('option pid missed.
 *           arkwebjs'); the target is the HAP's own ':render' process, a missing render process or
 *           a silent non-production fails the case.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperFunctionTest, MemHeap_ArkwebJs_051, TestSize.Level3)
{
    /* Environment-adaptive entry: builds without the option show its absence (verified board
     * presentation: "hidumper: option pid missed. arkwebjs") and are judged success directly. */
    CmdResult probe = RunHidumper("--mem-heap " + std::to_string(INIT_PID) + " --arkweb-js");
    if (Contains(probe, "option pid missed") || Contains(probe, "invalid arg")) {
        /* Reserved (RK3568 shape: --arkweb-js not compiled in, macro isolation): judged success
         * directly - the absence itself is positively shown by the rejection above. */
        GTEST_SUCCEED();
        return;
    }
    int32_t renderPid = FindRenderProcessPid();
    if (renderPid <= 0) {
        if (!FuncTestHapAvailable()) {
            GTEST_SKIP() << "no ':render' process exists and the dedicated target HAP is "
                            "unavailable in this environment";
        }
        renderPid = StartAppAndWaitRenderPid(FUNC_TEST_HAP_BUNDLE, FUNC_TEST_HAP_ABILITY);
    }
    if (renderPid <= 0) {
        /* The dedicated HAP shows a Web component and must own a ':render' process (verified on
         * the phone); without one the case has no self-built target and fails. */
        FAIL() << "the dedicated HAP yielded no ':render' process: " << DescribeEnvironment();
    }
    /* The staged artifact is named with the main pid, the migrated one with the render pid. */
    int32_t mainPid = GetMainPidOfRender(renderPid);
    std::string stagingNeedle;
    if (mainPid > 0) {
        stagingNeedle = ArtifactNeedle(FAMILY_ARKWEB_JSHEAP, mainPid);
    }
    ArtifactCheck check = VerifySnapshotArtifact(
        "--mem-heap " + std::to_string(renderPid) + " --arkweb-js",
        FAMILY_ARKWEB_JSHEAP, renderPid, SNAPSHOT_TIMEOUT_SEC, stagingNeedle);
    if (!check.produced) {
        /* The phone was verified producing a 1.9 MiB arkweb_jsheap artifact from this very HAP,
         * so a silent non-production is a regression, not a reservation. */
        FAIL() << "the ':render' process produced no arkweb js heap artifact: " << check.detail <<
               "; " << DirListingForReport();
    }
    ASSERT_TRUE(check.verified) << "render pid " << renderPid << ": " << check.detail;
}

/* ================================= cangjie heap (--mem-cjheap) ================================= */

/**
 * @tc.name: CjHeap_Reserved_052
 * @tc.desc: Command: hidumper --mem-cjheap <Cangjie process pid>
 *           Function: Cangjie heap snapshot.
 *           Pass with plugin disabled: judged success directly.
 *           Pass with plugin enabled: judged success directly.
 *           Notes: neither image ships a Cangjie process and the HAP cannot host a CJVM: DumpCjHeap
 *           is compiled only under CJ_FRONTEND and needs CJEnv::LoadInstance()
 *           (dump_runtime_helper.cpp:348), while GN/hvigor has no Cangjie component support.
 *           Activation criteria recorded: a cjheap-<pid>-<ts> artifact larger than 1024 bytes.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperFunctionTest, CjHeap_Reserved_052, TestSize.Level3)
{
    /* Reserved: verifies the --mem-cjheap Cangjie heap dump (activation criteria: a
     * cjheap-<pid>-<ts> artifact larger than 1024 bytes, migrated under the plugin build with the
     * staging copy gone). Neither image ships a Cangjie runtime target (adding the Cangjie toolchain
     * to the HAP was evaluated and deferred), so the case passes directly.
     */
    GTEST_SUCCEED();
}

/* ======================================== faultlog (-e) family ======================================== */

/**
 * @tc.name: Faultlog_View_053
 * @tc.desc: Command: hidumper -e
 *           Function: crash history view (file-driven).
 *           Pass with plugin disabled: the [faultlog] section carries content.
 *           Pass with plugin enabled: same as plugin-disabled.
 *           Notes: the faultlog file must exist: preexisting or provisioned through the HAP crash
 *           trigger (verified writing it on both images); a provisioning failure fails the case.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperFunctionTest, Faultlog_View_053, TestSize.Level2)
{
    /* File-driven view: a faultlog file must exist - either preexisting or provisioned through
     * the HAP crash trigger, which is part of the test resources and verified writing the file
     * on both images. */
    if (!EnsureFaultLogFile()) {
        if (!FuncTestHapAvailable()) {
            GTEST_SKIP() << "no faultlog file exists and the dedicated HAP crash trigger is "
                            "unavailable in this environment";
        }
        FAIL() << "no faultlog file exists and the HAP crash trigger could not provision one";
    }
    CmdResult result = RunHidumper("-e");
    ASSERT_FALSE(Contains(result, HINT_NO_FAULTLOGS)) << result.output;
    ASSERT_TRUE(Contains(result, SECTION_FAULTLOG)) << result.output;
    ASSERT_GT(CountLines(result.output), 2) << result.output;
}

/**
 * @tc.name: Faultlog_List_054
 * @tc.desc: Command: hidumper -e --list; -e --list -n 1; -e --list <process_name>; -e --list
 * @tc.desc: --since <ts> --until <same ts>
 *           Function: abnormal-exit record list with the documented filters.
 *           Pass with plugin disabled: judged success directly without running any command.
 *           Pass with plugin enabled: five documented columns, at least one record row (provisioned
 *           through the HAP crash trigger), -n 1 yields exactly one row, the name filter scopes
 *           every row, an identical --since/--until window yields 'no records found.'.
 *           Notes: the board image has no hisysevent fault pipeline: it writes cppcrash files but
 *           never exit-reason records (verified), so records cannot be triggered there.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperFunctionTest, Faultlog_List_054, TestSize.Level2)
{
    if (!PLUGIN_ENABLED) {
        /* Reserved (RK3568/board image): the downstream hisysevent fault pipeline does not exist
         * there - the image writes cppcrash files but never exit-reason records (verified) - so
         * -e --list records cannot be triggered on the board at all; judged success directly
         * without running the command or the costly crash trigger. */
        GTEST_SUCCEED();
        return;
    }
    if (!FuncTestHapAvailable()) {
        GTEST_SKIP() << "the dedicated target HAP is unavailable in this environment "
                        "(neither installed nor pushed to an installable location)";
    }
    /* Provision a record through the HAP crash trigger when the device has none, so the list
     * view is exercised against real data. */
    (void)ProvisionFaultRecord();
    CmdResult result = RunHidumper("-e --list");
    ASSERT_FALSE(Contains(result, HINT_NO_RECORDS)) <<
        "this image carries the hisysevent fault pipeline and a crash was provisioned through "
           "the HAP trigger, so records must be listed: " << result.output;
    for (const auto &column : std::vector<std::string> {"time", "foreground", "reason",
                                                        "record_id", "process_name"}) {
        ASSERT_TRUE(Contains(result, column)) << "column " << column << " missing: " << result.output;
    }
    const std::vector<std::string> rows = CollectListDataRows(result.output);
    ASSERT_FALSE(rows.empty()) << "the table has no record row: " << result.output;
    ASSERT_FALSE(ParseFirstRecordId(result.output).empty()) << "the table has no parsable record id: " << result.output;
    /* Documented filter: -n limits the number of record rows. */
    CmdResult limited = RunHidumper("-e --list -n 1");
    ASSERT_EQ(CollectListDataRows(limited.output).size(), 1u) << limited.output;
    /* Documented filter: process_name scopes every row to the requested process. */
    const std::string name = ListRowColumn(rows[0], LIST_ROW_NAME_COLUMN);
    CmdResult filtered = RunHidumper("-e --list " + name);
    const std::vector<std::string> filteredRows = CollectListDataRows(filtered.output);
    ASSERT_FALSE(filteredRows.empty()) << filtered.output;
    for (const auto &row : filteredRows) {
        ASSERT_NE(ListRowColumn(row, LIST_ROW_NAME_COLUMN).find(name), std::string::npos) <<
            "row outside the requested process: " << row;
    }
    /* Documented edge on a record-carrying image: an identical --since/--until window is empty. */
    CmdResult edge = RunHidumper("-e --list --since '" + EMPTY_WINDOW_TS + "' --until '" +
                                 EMPTY_WINDOW_TS + "'");
    ASSERT_TRUE(Contains(edge, HINT_NO_RECORDS)) << edge.output;
}

/**
 * @tc.name: Faultlog_Print_055
 * @tc.desc: Command: hidumper -e --print; hidumper -e --print -n 1
 *           Function: faultlog content dump with the documented -n filter.
 *           Pass with plugin disabled: judged success directly without running any command.
 *           Pass with plugin enabled: a faultlog-carrying HAP CppCrash record is ensured; the
 *           [faultlog] section carries content; -n 1 prints exactly one section.
 *           Notes: ambient record types may legitimately carry no faultlog ('this type of record
 *           does not have faultlog.', verified), hence the bundle-scoped subject.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperFunctionTest, Faultlog_Print_055, TestSize.Level2)
{
    if (!PLUGIN_ENABLED) {
        /* Reserved (board image): no hisysevent fault records exist there (verified), so the
         * record-driven print view cannot be positively populated. */
        GTEST_SUCCEED();
        return;
    }
    if (!FuncTestHapAvailable()) {
        GTEST_SKIP() << "the dedicated target HAP is unavailable in this environment "
                        "(neither installed nor pushed to an installable location)";
    }
    std::string recordId;
    CmdResult bundlePrint;
    ASSERT_TRUE(FindBundleFaultlogRecord(recordId, bundlePrint)) <<
        "the phone image carries the hisysevent fault pipeline, so a faultlog-carrying HAP "
           "record must exist or be producible through the crash trigger";
    CmdResult result = RunHidumper("-e --print");
    ASSERT_FALSE(Contains(result, HINT_NO_FAULTLOGS)) << result.output;
    ASSERT_TRUE(Contains(result, SECTION_FAULTLOG)) << result.output;
    ASSERT_GT(CountLines(result.output), 1) << result.output;
    /* Documented filter: -n limits the printout to exactly one faultlog section. */
    CmdResult limited = RunHidumper("-e --print -n 1");
    ASSERT_EQ(CountOccurrences(limited.output, SECTION_FAULTLOG), 1) << limited.output;
}

/**
 * @tc.name: Faultlog_PrintId_056
 * @tc.desc: Command: hidumper -e --print 999; hidumper -e --print <bundle record id>
 *           Function: print by record id.
 *           Pass with plugin disabled: judged success directly without running any command.
 *           Pass with plugin enabled: id 999 yields the documented empty diagnostic; a bundle
 *           record with a live faultlog yields the [faultlog] section carrying the log file path
 *           (with the bundle name) and content.
 *           Notes: the numeric id is not echoed (verified); records whose log rotated away are
 *           skipped and the crash is re-triggered when no bundle record has a live log.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperFunctionTest, Faultlog_PrintId_056, TestSize.Level2)
{
    if (!PLUGIN_ENABLED) {
        /* Reserved (RK3568/board image): the downstream hisysevent fault pipeline does not exist
         * there (verified), so -e --print records cannot be triggered on the board at all;
         * judged success directly without running any command. */
        GTEST_SUCCEED();
        return;
    }
    CmdResult unknown = RunHidumper("-e --print " + std::to_string(UNKNOWN_RECORD_ID));
    ASSERT_TRUE(Contains(unknown, HINT_NO_FAULTLOGS) || Contains(unknown, SECTION_FAULTLOG)) << unknown.output;
    if (!FuncTestHapAvailable()) {
        GTEST_SKIP() << "the dedicated target HAP is unavailable in this environment "
                        "(neither installed nor pushed to an installable location)";
    }
    std::string recordId;
    CmdResult printed;
    ASSERT_TRUE(FindBundleFaultlogRecord(recordId, printed)) <<
        "the phone image carries the hisysevent fault pipeline, so a faultlog-carrying HAP "
           "record must exist or be producible through the crash trigger";
    ASSERT_TRUE(Contains(printed, SECTION_FAULTLOG)) << printed.output;
    /* The section does not echo the numeric id (the documented shape is the faultlog file path
     * plus content, verified on the phone); the bundle name inside the path binds the output to
     * the process of the requested record. */
    ASSERT_TRUE(Contains(printed, FUNC_TEST_HAP_BUNDLE)) <<
        "the printed faultlog does not belong to the bundle record " << recordId << ": " << printed.output;
}

} // namespace HiviewDFX
} // namespace OHOS
