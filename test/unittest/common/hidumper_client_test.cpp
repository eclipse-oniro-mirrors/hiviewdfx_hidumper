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
    std::string topLeakedType;
    int ret = proxy_->CountFdNums(pid, fdNums, detailFdInfo, topLeakedType);
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
} // namespace HiviewDFX
} // namespace OHOS