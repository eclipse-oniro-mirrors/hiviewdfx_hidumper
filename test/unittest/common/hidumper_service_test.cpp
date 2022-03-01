/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include "hidumper_service_test.h"
#include <iservice_registry.h>
#include "common.h"
#include "hilog_wrapper.h"
#include "dump_callback_broker_stub.h"
#include "dump_manager_client.h"
#include "raw_param.h"
#include "dump_manager_service.h"
#include "inner/dump_service_id.h"
#include "hidumper_test_default_callback.h"
using namespace std;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::HiviewDFX;
namespace OHOS {
namespace HiviewDFX {
namespace {
static const int TEST_LOOP = 10000;
static const int TASK_WAITTIME_MAX = 10000;
static const int TASK_REQUEST_MAX = 1000;
static const int TASK_WAIT_ONETIME = 100;
static const std::string TEST_ARGV_0 = "hidumper";
static const std::string TEST_ARGV_1 = "-h";
static int g_TaskOutfd = -1;
static std::vector<std::string> g_TaskArgs;
static IDumpCallbackBroker* g_TaskCallback;
}
void HidumperServiceTest::SetUpTestCase(void)
{
}

void HidumperServiceTest::TearDownTestCase(void)
{
}

void HidumperServiceTest::SetUp(void)
{
}

void HidumperServiceTest::TearDown(void)
{
}

/**
 * @tc.name: HidumperServiceTest001
 * @tc.desc: Test DumpManagerService service ready.
 * @tc.type: FUNC
 */
HWTEST_F (HidumperServiceTest, HidumperServiceTest001, TestSize.Level3)
{
    auto dmsTest = DelayedSpSingleton<DumpManagerService>::GetInstance();
    dmsTest->OnStart();
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    ASSERT_TRUE(sam != nullptr) << "HidumperServiceTest001 fail to get GetSystemAbilityManager";
    sptr<IRemoteObject> remoteObject = sam->CheckSystemAbility(DFX_SYS_HIDUMPER_ABILITY_ID);
    ASSERT_TRUE(remoteObject != nullptr) << "GetSystemAbility failed.";
    dmsTest->OnStop();
}

/**
 * @tc.name: HidumperServiceTest002
 * @tc.desc: Test DumpManagerService Start and stop.
 * @tc.type: FUNC
 */
HWTEST_F (HidumperServiceTest, HidumperServiceTest002, TestSize.Level3)
{
    auto dmsTest = DelayedSpSingleton<DumpManagerService>::GetInstance();
    ASSERT_TRUE(dmsTest != nullptr) << "HidumperServiceTest002 fail to get DumpManagerService";
    ASSERT_TRUE(!dmsTest->IsServiceStarted()) << "SetUpTestCase dmsTest is ready";
    dmsTest->OnStart();
    ASSERT_TRUE(dmsTest->IsServiceStarted()) << "SetUpTestCase dmsTest is not ready";
    dmsTest->OnStop();
    ASSERT_TRUE(!dmsTest->IsServiceStarted()) << "SetUpTestCase dmsTest stop fail";
    dmsTest->OnStart();
    ASSERT_TRUE(dmsTest->IsServiceStarted()) << "SetUpTestCase dmsTest is not ready";
    dmsTest->OnStop();
    ASSERT_TRUE(!dmsTest->IsServiceStarted()) << "SetUpTestCase dmsTest stop fail";
}

/**
 * @tc.name: HidumperServiceTest003
 * @tc.desc: Test DumpManagerService Request.
 * @tc.type: FUNC
 */
HWTEST_F (HidumperServiceTest, HidumperServiceTest003, TestSize.Level3)
{
    // prepare request data
    std::vector<std::u16string> args = {
        Str8ToStr16(TEST_ARGV_0), Str8ToStr16(TEST_ARGV_1),
    };
    int outfd = -1;
    uint32_t statusStart = IDumpCallbackBroker::STATUS_INIT;
    uint32_t statusFinish = IDumpCallbackBroker::STATUS_INIT;
    uint32_t statusError = IDumpCallbackBroker::STATUS_INIT;
    sptr<IDumpCallbackBroker> callback = sptr<IDumpCallbackBroker>(new HidumperTestDefaultCallback(statusStart,
        statusFinish, statusError));
    // start service
    auto dmsTest = DelayedSpSingleton<DumpManagerService>::GetInstance();
    ASSERT_TRUE(dmsTest != nullptr) << "Fail to get DumpManagerService";
    dmsTest->OnStart();
    ASSERT_TRUE(dmsTest->IsServiceStarted()) << "DumpManagerService isn't ready";
    // set dummy function
    dmsTest->SetTestMainFunc([=] (int argc, char *argv[], const std::shared_ptr<RawParam>& rawParam) {
        for (int i = 0; ((!rawParam->IsCanceled()) && (i < TASK_WAITTIME_MAX)); i++) {
            usleep(TASK_WAIT_ONETIME);
        }
    });
    // request
    int res = dmsTest->Request(args, outfd, callback);
    EXPECT_EQ(res, DumpStatus::DUMP_OK);
    // stop service
    dmsTest->OnStop();
    ASSERT_TRUE(!dmsTest->IsServiceStarted()) << "DumpManagerService stop fail";
    // check result
    EXPECT_EQ(statusStart, IDumpCallbackBroker::STATUS_DUMP_STARTED);
    EXPECT_EQ(statusFinish, IDumpCallbackBroker::STATUS_DUMP_FINISHED);
    ASSERT_EQ(statusError, IDumpCallbackBroker::STATUS_INIT);
}

/**
 * @tc.name: HidumperServiceTest004
 * @tc.desc: Test DumpManagerService Request argument.
 * @tc.type: FUNC
 */
HWTEST_F (HidumperServiceTest, HidumperServiceTest004, TestSize.Level3)
{
    // prepare request data
    std::vector<std::u16string> args = {
        Str8ToStr16(TEST_ARGV_0), Str8ToStr16(TEST_ARGV_1),
    };
    int outfd = -100;
    uint32_t statusStart = IDumpCallbackBroker::STATUS_INIT;
    uint32_t statusFinish = IDumpCallbackBroker::STATUS_INIT;
    uint32_t statusError = IDumpCallbackBroker::STATUS_INIT;
    sptr<IDumpCallbackBroker> callback = sptr<IDumpCallbackBroker>(new HidumperTestDefaultCallback(statusStart,
        statusFinish, statusError));
    // start service
    auto dmsTest = DelayedSpSingleton<DumpManagerService>::GetInstance();
    ASSERT_TRUE(dmsTest != nullptr) << "Fail to get DumpManagerService";
    dmsTest->OnStart();
    ASSERT_TRUE(dmsTest->IsServiceStarted()) << "DumpManagerService isn't ready";
    // set dummy function to get data
    dmsTest->SetTestMainFunc([=] (int argc, char *argv[], const std::shared_ptr<RawParam>& rawParam) {
        g_TaskOutfd = rawParam->GetOutputFd();
        const sptr<IDumpCallbackBroker> cb = rawParam->GetCallback();
        if (cb != nullptr) {
            g_TaskCallback = cb.GetRefPtr();
        }
        for (int i = 0; ((argv != nullptr) && (i < argc)); i++) {
            g_TaskArgs.push_back(std::string(argv[i]));
        }
        for (int i = 0; ((!rawParam->IsCanceled()) && (i < TASK_WAITTIME_MAX)); i++) {
            usleep(TASK_WAIT_ONETIME);
        }
    });
    g_TaskOutfd = -1;
    g_TaskArgs.clear();
    g_TaskCallback = nullptr;
    // request
    int res = dmsTest->Request(args, outfd, callback);
    EXPECT_EQ(res, DumpStatus::DUMP_OK);
    // stop service
    sleep(1);
    dmsTest->OnStop();
    ASSERT_TRUE(!dmsTest->IsServiceStarted()) << "DumpManagerService stop fail";
    // check result
    EXPECT_EQ(args.size(), g_TaskArgs.size());
    for (size_t i = 0; i < args.size(); i++) {
        EXPECT_EQ(Str16ToStr8(args[i]), g_TaskArgs[i]);
    }
    EXPECT_EQ(outfd, g_TaskOutfd);
    ASSERT_EQ(g_TaskCallback, callback);
}

/**
 * @tc.name: HidumperServiceTest005
 * @tc.desc: Test DumpManagerService Request Max.
 * @tc.type: FUNC
 */
HWTEST_F (HidumperServiceTest, HidumperServiceTest005, TestSize.Level3)
{
    // prepare request data
    uint32_t statusStart = IDumpCallbackBroker::STATUS_INIT;
    uint32_t statusFinish = IDumpCallbackBroker::STATUS_INIT;
    uint32_t statusError = IDumpCallbackBroker::STATUS_INIT;
    std::vector<std::u16string> args = {
        Str8ToStr16(TEST_ARGV_0), Str8ToStr16(TEST_ARGV_1),
    };
    int outfd = -1;
    sptr<IDumpCallbackBroker> callback = sptr<IDumpCallbackBroker>(new HidumperTestDefaultCallback(statusStart,
        statusFinish, statusError));
    // start service
    auto dmsTest = DelayedSpSingleton<DumpManagerService>::GetInstance();
    ASSERT_TRUE(dmsTest != nullptr) << "Fail to get DumpManagerService";
    dmsTest->OnStart();
    ASSERT_TRUE(dmsTest->IsServiceStarted()) << "DumpManagerService isn't ready";
    // set dummy function
    dmsTest->SetTestMainFunc([=] (int argc, char *argv[], const std::shared_ptr<RawParam>& rawParam) {
        for (int i = 0; ((!rawParam->IsCanceled()) && (i < TASK_WAITTIME_MAX)); i++) {
            usleep(TASK_WAIT_ONETIME);
        }
    });
    bool requestOk = false;
    bool requestMax = false;
    bool requestError = false;
    // request
    for (int i = 0; i < TASK_REQUEST_MAX; i++) {
        int res = dmsTest->Request(args, outfd, callback);
        if (res == DumpStatus::DUMP_OK) {
            requestOk = true;
            continue;
        } else if (res == DumpStatus::DUMP_REQUEST_MAX) {
            requestMax = true;
            break;
        }
        requestError = true;
        break;
    }
    // stop service
    dmsTest->OnStop();
    ASSERT_TRUE(!dmsTest->IsServiceStarted()) << "DumpManagerService stop fail";
    // check result
    ASSERT_TRUE(requestOk);
    ASSERT_TRUE(requestMax);
    ASSERT_TRUE(!requestError);
}

/**
 * @tc.name: HidumperServiceTest006
 * @tc.desc: Loop to Test DumpManagerService start & stop.
 * @tc.type: FUNC
 */
HWTEST_F (HidumperServiceTest, HidumperServiceTest006, TestSize.Level3)
{
    // loop to start & stop
    for (int i = 0; i < TEST_LOOP; i++) {
        auto dmsTest = DelayedSpSingleton<DumpManagerService>::GetInstance();
        ASSERT_TRUE(dmsTest != nullptr) << "Fail to get DumpManagerService";
        ASSERT_TRUE(!dmsTest->IsServiceStarted()) << "dmsTest isn't ready";
        dmsTest->OnStart();
        ASSERT_TRUE(dmsTest->IsServiceStarted()) << "dmsTest is ready";
        sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        ASSERT_TRUE(sam != nullptr) << "Fail to get GetSystemAbilityManager";
        sptr<IRemoteObject> remoteObject = sam->CheckSystemAbility(DFX_SYS_HIDUMPER_ABILITY_ID);
        ASSERT_TRUE(remoteObject != nullptr) << "GetSystemAbility failed.";
        dmsTest->OnStop();
        ASSERT_TRUE(!dmsTest->IsServiceStarted()) << "dmsTest isn't ready";
    }
}

/**
 * @tc.name: HidumperServiceTest007
 * @tc.desc: Loop to Test DumpManagerService Request.
 * @tc.type: FUNC
 */
HWTEST_F (HidumperServiceTest, HidumperServiceTest007, TestSize.Level3)
{
    // prepare request data
    uint32_t statusStart = IDumpCallbackBroker::STATUS_INIT;
    uint32_t statusFinish = IDumpCallbackBroker::STATUS_INIT;
    uint32_t statusError = IDumpCallbackBroker::STATUS_INIT;
    std::vector<std::u16string> args = {
        Str8ToStr16(TEST_ARGV_0), Str8ToStr16(TEST_ARGV_1),
    };
    int outfd = -1;
    sptr<IDumpCallbackBroker> callback = sptr<IDumpCallbackBroker>(new HidumperTestDefaultCallback(statusStart,
        statusFinish, statusError));
    // start service
    auto dmsTest = DelayedSpSingleton<DumpManagerService>::GetInstance();
    ASSERT_TRUE(dmsTest != nullptr) << "Fail to get DumpManagerService";
    dmsTest->OnStart();
    ASSERT_TRUE(dmsTest->IsServiceStarted()) << "DumpManagerService isn't ready";
    // set dummy function
    dmsTest->SetTestMainFunc([=] (int argc, char *argv[], const std::shared_ptr<RawParam>& rawParam) {
        usleep(TASK_WAIT_ONETIME);
    });
    // loop to request
    bool hasError = false;
    int hasErrorCode = 0;
    int hasErrorIndex = 0;
    for (int i = 0; i < TEST_LOOP; i++) {
        int res = dmsTest->Request(args, outfd, callback);
        if (res == DumpStatus::DUMP_OK) {
            usleep(TASK_WAIT_ONETIME);
            continue;
        } else if (res == DumpStatus::DUMP_REQUEST_MAX) {
            std::cout << "request max, times = " << i << std::endl;
            usleep(TASK_WAIT_ONETIME);
            continue;
        }
        hasError = true;
        hasErrorCode = res;
        hasErrorIndex = i;
        break;
    }
    // stop service
    dmsTest->OnStop();
    ASSERT_TRUE(!dmsTest->IsServiceStarted()) << "DumpManagerService stop fail";
    // check result
    ASSERT_TRUE(!hasError) << "request error, times = " << hasErrorIndex << ", res = " << hasErrorCode;
}
} // namespace HiviewDFX
} // namespace OHOS