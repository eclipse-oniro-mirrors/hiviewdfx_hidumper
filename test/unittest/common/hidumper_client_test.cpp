/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include <fcntl.h>
#include <vector>
#include <unistd.h>
#include <cstdlib>
#include <gtest/gtest.h>
#include <string_ex.h>
#include "inner/dump_service_id.h"
#include "dump_client_main.h"
#include "dump_controller.h"
#include "dump_manager_client.h"
#include "app_mgr_client.h"

using namespace testing::ext;
using namespace std;
using OHOS::HiviewDFX::DumpClientMain;
using OHOS::AppExecFwk::AppMgrClient;
using OHOS::AppExecFwk::RunningProcessInfo;
namespace OHOS {
namespace HiviewDFX {
const std::string TOOL_NAME = "hidumper";
const int BUFFER_SIZE = 1024;
/* Emulator builds macro-gate the --mem detail options out in ValidateDumpOptions; the
 * service-side parse status is not propagated through Request (StartRequest always answers
 * DUMP_OK), so the message written to the output fd is the discriminator. */
const std::string EMU_UNSUPPORTED_MSG = "The capability is not supported on the emulator at this time.";
const std::string SECTION_MEMORY_TAG = "[memory]";
const std::string ERR_MISS_SHOW_GPUMEM = "option pid missed. showgpumem";
const int PIPE_PAIR_SIZE = 2;
const int PIPE_READ = 0;
const int PIPE_WRITE = 1;

/* Drain a pipe fd until EOF. The service finishes writing before Request returns,
 * so the write end must be closed by the caller first. Pipe fds follow the same
 * transfer path as the CLI's redirected stdout (proven by the function tests). */
static std::string DrainFd(int fd)
{
    std::string content;
    char buf[BUFFER_SIZE] = {0};
    ssize_t n = 0;
    while ((n = read(fd, buf, sizeof(buf) - 1)) > 0) {
        buf[n] = '\0';
        content += buf;
    }
    return content;
}

class HidumperClientTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void HidumperClientTest::SetUpTestCase(void)
{
}

void HidumperClientTest::TearDownTestCase(void)
{
}

void HidumperClientTest::SetUp(void)
{
}

void HidumperClientTest::TearDown(void)
{
}

/**
 * @tc.name: ClientMainTest001
 * @tc.desc: Test too many arguments.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperClientTest, ClientMainTest001, TestSize.Level0)
{
    char *argv[] = {
        const_cast<char *>(TOOL_NAME.c_str()),
    };
    int argc = ARG_MAX_COUNT + 1;
    int ret = DumpClientMain::GetInstance().Main(argc, argv, STDOUT_FILENO);
    ASSERT_EQ(ret, DumpStatus::DUMP_INVALID_ARG);
}

/**
 * @tc.name: ClientMainTest002
 * @tc.desc: Test empty argument.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperClientTest, ClientMainTest002, TestSize.Level0)
{
    char *argv[] = {
        const_cast<char *>(TOOL_NAME.c_str()),
        const_cast<char *>("--mem"),
        const_cast<char *>("-1"),
        const_cast<char *>(""),
        const_cast<char *>("--mem"),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    int ret = DumpClientMain::GetInstance().Main(argc, argv, STDOUT_FILENO);
    ASSERT_EQ(ret, DumpStatus::DUMP_INVALID_ARG);
}

/**
 * @tc.name: ClientMainTest003
 * @tc.desc: Test too long argument .
 * @tc.type: FUNC
 */
HWTEST_F(HidumperClientTest, ClientMainTest003, TestSize.Level0)
{
    std::string longArg;
    longArg.assign(SINGLE_ARG_MAXLEN + 1, 'c');
    char *argv[] = {
        const_cast<char *>(TOOL_NAME.c_str()),
        const_cast<char *>("-h"),
        const_cast<char *>(longArg.c_str()),
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    int ret = DumpClientMain::GetInstance().Main(argc, argv, STDOUT_FILENO);
    ASSERT_EQ(ret, DumpStatus::DUMP_INVALID_ARG);
}

/**
 * @tc.name: ClientMainTest004
 * @tc.desc: Test null argument.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperClientTest, ClientMainTest004, TestSize.Level0)
{
    char *argv[] = {
        const_cast<char *>(TOOL_NAME.c_str()),
        nullptr,
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    int ret = DumpClientMain::GetInstance().Main(argc, argv, STDOUT_FILENO);
    ASSERT_EQ(ret, DumpStatus::DUMP_INVALID_ARG);
}

/**
 * @tc.name: ClientMainTest005
 * @tc.desc: Test null fd.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperClientTest, ClientMainTest005, TestSize.Level0)
{
    char *argv[] = {
        const_cast<char *>("hidumper"),
        const_cast<char *>("--mem"),
        const_cast<char *>("1"),
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    int fd = open("/dev/null", O_RDWR | O_CREAT | O_TRUNC, 0664);
    if (fd <= 0) {
        fd = STDERR_FILENO;
    }
    int ret = DumpClientMain::GetInstance().Main(argc, argv, fd);
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
}

/**
 * @tc.name: ClientMainTest006
 * @tc.desc: Test null argv.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperClientTest, ClientMainTest006, TestSize.Level0)
{
    int argc = ARG_MAX_COUNT;
    int ret = DumpClientMain::GetInstance().Main(argc, nullptr, STDOUT_FILENO);
    ASSERT_EQ(ret, DumpStatus::DUMP_INVALID_ARG);
}

/**
 * @tc.name: ClientMainTest007
 * @tc.desc: Test null fd from hiview.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperClientTest, ClientMainTest007, TestSize.Level0)
{
    char *argv[] = {
        const_cast<char *>("hidumper"),
        const_cast<char *>("-s"),
        const_cast<char *>("1213"),
        const_cast<char *>("-a"),
        const_cast<char *>("--Logbackup"),
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    int fd = open("/dev/null",  O_CREAT | O_TRUNC, 0664);
    if (fd <= 0) {
        fd = STDERR_FILENO;
    }
    setuid(1201); //hiview uid
    int ret = DumpClientMain::GetInstance().Main(argc, argv, fd);
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
}

/**
 * @tc.name: ClientMainTest008
 * @tc.desc: Test the file does not exist.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperClientTest, ClientMainTest008, TestSize.Level0)
{
    char *argv[] = {
        const_cast<char *>("hidumper"),
        const_cast<char *>("--mem"),
        const_cast<char *>("1"),
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    int fd = open("/no/such/file",  O_RDWR);
    int ret = DumpClientMain::GetInstance().Main(argc, argv, fd);
    ASSERT_EQ(ret, DumpStatus::DUMP_FAIL);
    DumpClientMain::GetInstance().~DumpClientMain();
}

/**
 * @tc.name: ManagerClientTest001
 * @tc.desc: Test emtpy argument list.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperClientTest, ManagerClientTest001, TestSize.Level0)
{
    vector<u16string> args{};
    int ret = DumpManagerClient::GetInstance().Request(args, STDOUT_FILENO);
    ASSERT_EQ(ret, DumpStatus::DUMP_FAIL);
}

/**
 * @tc.name: ManagerClientTest002
 * @tc.desc: Test emtpy argument.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperClientTest, ManagerClientTest002, TestSize.Level0)
{
    vector<u16string> args{
        std::u16string(),
    };
    int ret = DumpManagerClient::GetInstance().Request(args, STDOUT_FILENO);
    ASSERT_EQ(ret, DumpStatus::DUMP_FAIL);
}

/**
 * @tc.name: ManagerClientTest003
 * @tc.desc: Test mamanage client Request.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperClientTest, ManagerClientTest003, TestSize.Level0)
{
    vector<u16string> args{
        std::u16string(u"hidumper"),
        std::u16string(u"-s"),
        std::u16string(u"1212"),
    };
    int ret = DumpManagerClient::GetInstance().Request(args, STDOUT_FILENO);
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
}

/**
 * @tc.name: ManagerClientTest004
 * @tc.desc: Test mamanage client ScanPidOverLimit.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperClientTest, ManagerClientTest004, TestSize.Level0)
{
    sptr<IDumpBroker> proxy_ {nullptr};
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    ASSERT_TRUE(sam != nullptr) << "ManagerClientTest004 fail to get GetSystemAbilityManager";
    sptr<IRemoteObject> remoteObject = sam->CheckSystemAbility(DFX_HI_DUMPER_SERVICE_ABILITY_ID);
    ASSERT_TRUE(remoteObject != nullptr) << "Get SystemAbility failed.";
    proxy_ = iface_cast<IDumpBroker>(remoteObject);
    std::string requestType = "fd";
    std::vector<int32_t> pidList;
    int ret = proxy_->ScanPidOverLimit(requestType, LIMIT_SIZE, pidList);
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
}

/**
 * @tc.name: ManagerClientTest004_1
 * @tc.desc: Test mamanage client ScanOrphanVnodeOverLimit.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperClientTest, ManagerClientTest004_1, TestSize.Level0)
{
    sptr<IDumpBroker> proxy_ {nullptr};
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    ASSERT_TRUE(sam != nullptr) << "ManagerClientTest004_1 fail to get GetSystemAbilityManager";
    sptr<IRemoteObject> remoteObject = sam->CheckSystemAbility(DFX_HI_DUMPER_SERVICE_ABILITY_ID);
    ASSERT_TRUE(remoteObject != nullptr) << "Get SystemAbility failed.";
    proxy_ = iface_cast<IDumpBroker>(remoteObject);
    int32_t fdLeakThreshold = 100;
    int32_t orphanVnodeThreshold = 10;
    std::vector<std::string> topOrphanVnodeInfoList;
    int ret = proxy_->ScanOrphanVnodeOverLimit(fdLeakThreshold, orphanVnodeThreshold, topOrphanVnodeInfoList);
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
}

/**
 * @tc.name: ManagerClientTest005
 * @tc.desc: Test mamanage client CountFdNums.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperClientTest, ManagerClientTest005, TestSize.Level0)
{
    sptr<IDumpBroker> proxy_ {nullptr};
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    ASSERT_TRUE(sam != nullptr) << "ManagerClientTest005 fail to get GetSystemAbilityManager";
    sptr<IRemoteObject> remoteObject = sam->CheckSystemAbility(DFX_HI_DUMPER_SERVICE_ABILITY_ID);
    ASSERT_TRUE(remoteObject != nullptr) << "Get SystemAbility failed.";
    proxy_ = iface_cast<IDumpBroker>(remoteObject);
    int32_t pid = 1;
    uint32_t fdNums = 0;
    std::string detailFdInfo;
    std::vector<std::string> topLeakedTypeList;
    int ret = proxy_->CountFdNums(pid, fdNums, detailFdInfo, topLeakedTypeList);
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
}

/**
 * @tc.name: ManagerClientTest006
 * @tc.desc: Test ipc stat dump with pid.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperClientTest, ManagerClientTest006, TestSize.Level0)
{
    string pid;
    FILE* file = popen("pidof samgr", "r");
    if (file) {
        char buffer[BUFFER_SIZE];
        if (fgets(buffer, sizeof(buffer), file) != nullptr) {
            pid.assign(buffer);
        };
        pid = pid.substr(0, pid.length() - 1);
        pclose(file);
    } else {
        std::cerr << "Failed to execute command" << std::endl;
    }
    vector<u16string> args{
        std::u16string(u"hidumper"),
        std::u16string(u"--ipc"),
        Str8ToStr16(pid),
        std::u16string(u"--start-stat"),
    };
    int32_t ret = DumpManagerClient::GetInstance().Request(args, STDOUT_FILENO);
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
}

/**
 * @tc.name: ManagerClientTest007
 * @tc.desc: Test ipc stat dump all.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperClientTest, ManagerClientTest007, TestSize.Level0)
{
    vector<u16string> args{
        std::u16string(u"hidumper"),
        std::u16string(u"--ipc"),
        std::u16string(u"-a"),
        std::u16string(u"--start-stat"),
    };
    int ret = DumpManagerClient::GetInstance().Request(args, STDOUT_FILENO);
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
}

/**
 * @tc.name: ManagerClientTest008
 * @tc.desc: Test cpuusage of all processes.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperClientTest, ManagerClientTest008, TestSize.Level0)
{
    vector<u16string> args{
        std::u16string(u"hidumper"),
        std::u16string(u"--cpuusage"),
    };
    int ret = DumpManagerClient::GetInstance().Request(args, STDOUT_FILENO);
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
}

/**
 * @tc.name: ManagerClientTest009
 * @tc.desc: Test cpuusage with pid.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperClientTest, ManagerClientTest009, TestSize.Level0)
{
    vector<u16string> args{
        std::u16string(u"hidumper"),
        std::u16string(u"--cpuusage"),
        std::u16string(u"1"),
    };
    int ret = DumpManagerClient::GetInstance().Request(args, STDOUT_FILENO);
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
}

/**
 * @tc.name: ManagerClientTest010
 * @tc.desc: Test cpufreq.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperClientTest, ManagerClientTest010, TestSize.Level0)
{
    vector<u16string> args{
        std::u16string(u"hidumper"),
        std::u16string(u"--cpufreq"),
    };
    int ret = DumpManagerClient::GetInstance().Request(args, STDOUT_FILENO);
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
}

/**
 * @tc.name: ManagerClientTest011
 * @tc.desc: Test --mem-jsheap.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperClientTest, ManagerClientTest011, TestSize.Level0)
{
    vector<u16string> args{
        std::u16string(u"hidumper"),
        std::u16string(u"--mem-jsheap"),
    };
    int ret = DumpManagerClient::GetInstance().Request(args, STDOUT_FILENO);
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
}

/**
 * @tc.name: ManagerClientTest012
 * @tc.desc: Test --mem-jsheap.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperClientTest, ManagerClientTest012, TestSize.Level0)
{
    string pid;
    auto appMgrClient = std::make_unique<AppMgrClient>();
    std::vector<RunningProcessInfo> runningProcessInfos;
    appMgrClient->GetAllRunningProcesses(runningProcessInfos);
    ASSERT_TRUE(runningProcessInfos.size() > 0);

    pid = to_string(runningProcessInfos[0].pid_);
    vector<u16string> args{
        std::u16string(u"hidumper"),
        std::u16string(u"--mem-jsheap"),
        Str8ToStr16(pid)
    };
    int ret = DumpManagerClient::GetInstance().Request(args, STDOUT_FILENO);
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
}

/**
 * @tc.name: ManagerClientTest013
 * @tc.desc: Test --mem-cjheap.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperClientTest, ManagerClientTest013, TestSize.Level0)
{
    vector<u16string> args{
        std::u16string(u"hidumper"),
        std::u16string(u"--mem-cjheap"),
    };
    int ret = DumpManagerClient::GetInstance().Request(args, STDOUT_FILENO);
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
}

/**
 * @tc.name: ManagerClientTest014
 * @tc.desc: Test --mem-cjheap.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperClientTest, ManagerClientTest014, TestSize.Level0)
{
    string pid;
    auto appMgrClient = std::make_unique<AppMgrClient>();
    std::vector<RunningProcessInfo> runningProcessInfos;
    appMgrClient->GetAllRunningProcesses(runningProcessInfos);
    ASSERT_TRUE(runningProcessInfos.size() > 0);

    pid = to_string(runningProcessInfos[0].pid_);
    vector<u16string> args{
        std::u16string(u"hidumper"),
        std::u16string(u"--mem-cjheap"),
        Str8ToStr16(pid)
    };
    int ret = DumpManagerClient::GetInstance().Request(args, STDOUT_FILENO);
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
}

/**
 * @tc.name: ManagerClientTest015
 * @tc.desc: Test --mem 1 --show-dmabuf. On emulator builds the capability is macro-gated
 *           out in ValidateDumpOptions: the output fd only carries the unsupported message;
 *           on real devices the [memory] report with the DMA aggregate is produced.
 *           Note: the service pops the last argument as the caller ppid (RawParam::Init),
 *           mirroring DumpClientMain::SetCmdArgs, so a trailing ppid argument is appended.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperClientTest, ManagerClientTest015, TestSize.Level0)
{
    vector<u16string> args{
        std::u16string(u"hidumper"),
        std::u16string(u"--mem"),
        std::u16string(u"1"),
        std::u16string(u"--show-dmabuf"),
        Str8ToStr16(std::to_string(getppid())),
    };
    int pipeFd[PIPE_PAIR_SIZE] = {-1, -1};
    ASSERT_EQ(pipe(pipeFd), 0);
    int ret = DumpManagerClient::GetInstance().Request(args, pipeFd[PIPE_WRITE]);
    close(pipeFd[PIPE_WRITE]);
    std::string output = DrainFd(pipeFd[PIPE_READ]);
    close(pipeFd[PIPE_READ]);
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
#ifdef IS_EMULATOR
    ASSERT_NE(output.find(EMU_UNSUPPORTED_MSG), std::string::npos) << output;
#else
    ASSERT_NE(output.find(SECTION_MEMORY_TAG), std::string::npos) << output;
    ASSERT_NE(output.find("DMA:"), std::string::npos) << output;
#endif
}

/**
 * @tc.name: ManagerClientTest016
 * @tc.desc: Test --mem 1 --show-gpumem. On emulator builds (plugin enabled) the capability is
 *           macro-gated out in ValidateDumpOptions: the output fd only carries the unsupported
 *           message; without the plugin the option is not compiled in at all and the output
 *           carries the unknown-option rejection; on real devices with the plugin the [memory]
 *           report is produced. The trailing ppid argument mirrors DumpClientMain::SetCmdArgs.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperClientTest, ManagerClientTest016, TestSize.Level0)
{
    vector<u16string> args{
        std::u16string(u"hidumper"),
        std::u16string(u"--mem"),
        std::u16string(u"1"),
        std::u16string(u"--show-gpumem"),
        Str8ToStr16(std::to_string(getppid())),
    };
    int pipeFd[PIPE_PAIR_SIZE] = {-1, -1};
    ASSERT_EQ(pipe(pipeFd), 0);
    int ret = DumpManagerClient::GetInstance().Request(args, pipeFd[PIPE_WRITE]);
    close(pipeFd[PIPE_WRITE]);
    std::string output = DrainFd(pipeFd[PIPE_READ]);
    close(pipeFd[PIPE_READ]);
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
#if !defined(HIDUMPER_HIVIEWDFX_PLUGIN_ENABLE)
    ASSERT_NE(output.find(ERR_MISS_SHOW_GPUMEM), std::string::npos) << output;
#elif defined(IS_EMULATOR)
    ASSERT_NE(output.find(EMU_UNSUPPORTED_MSG), std::string::npos) << output;
#else
    ASSERT_NE(output.find(SECTION_MEMORY_TAG), std::string::npos) << output;
#endif
}

/**
 * @tc.name: ManagerClientTest017
 * @tc.desc: Test --mem-heap.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperClientTest, ManagerClientTest017, TestSize.Level0)
{
    vector<u16string> args{
        std::u16string(u"hidumper"),
        std::u16string(u"--mem-heap"),
    };
    int ret = DumpManagerClient::GetInstance().Request(args, STDOUT_FILENO);
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
}

/**
 * @tc.name: ManagerClientTest018
 * @tc.desc: Test --mem-heap pid --native.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperClientTest, ManagerClientTest018, TestSize.Level0)
{
    string pid;
    auto appMgrClient = std::make_unique<AppMgrClient>();
    std::vector<RunningProcessInfo> runningProcessInfos;
    appMgrClient->GetAllRunningProcesses(runningProcessInfos);
    ASSERT_TRUE(runningProcessInfos.size() > 0);

    pid = to_string(runningProcessInfos[0].pid_);
    vector<u16string> args{
        std::u16string(u"hidumper"),
        std::u16string(u"--mem-heap"),
        Str8ToStr16(pid),
        std::u16string(u"--native"),
    };
    int ret = DumpManagerClient::GetInstance().Request(args, STDOUT_FILENO);
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
}

/**
 * @tc.name: ManagerClientTest019
 * @tc.desc: Test --mem-heap pid --native --leakobj.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperClientTest, ManagerClientTest019, TestSize.Level0)
{
    string pid;
    auto appMgrClient = std::make_unique<AppMgrClient>();
    std::vector<RunningProcessInfo> runningProcessInfos;
    appMgrClient->GetAllRunningProcesses(runningProcessInfos);
    ASSERT_TRUE(runningProcessInfos.size() > 0);

    pid = to_string(runningProcessInfos[0].pid_);
    vector<u16string> args{
        std::u16string(u"hidumper"),
        std::u16string(u"--mem-heap"),
        Str8ToStr16(pid),
        std::u16string(u"--native"),
        std::u16string(u"--leakobj"),
    };
    int ret = DumpManagerClient::GetInstance().Request(args, STDOUT_FILENO);
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
}
} // namespace HiviewDFX
} // namespace OHOS