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
#include "dump_manager_client.h"
#include "dump_manager_service.h"
#include "inner/dump_service_id.h"
#include "dump_on_demand_load.h"
#include "executor/memory/memory_util.h"
#include "string_ex.h"

using namespace std;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::HiviewDFX;
namespace OHOS {
namespace HiviewDFX {
const int32_t HidumperServiceTest::LIMIT_SIZE = 5000;
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
HWTEST_F(HidumperServiceTest, HidumperServiceTest001, TestSize.Level3)
{
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    ASSERT_TRUE(sam != nullptr) << "HidumperServiceTest001 fail to get GetSystemAbilityManager";
    sptr<OnDemandLoadCallback> loadCallback = new OnDemandLoadCallback();
    int32_t result = sam->LoadSystemAbility(DFX_SYS_HIDUMPER_ABILITY_ID, loadCallback);
    ASSERT_TRUE(result == ERR_OK) << "GetSystemAbility failed.";
}

/**
 * @tc.name: DumpManagerService002
 * @tc.desc: Test DumpManagerService Request.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperServiceTest, DumpManagerService002, TestSize.Level3)
{
    auto dumpManagerService = std::make_shared<DumpManagerService>();
    dumpManagerService->OnStart();
    dumpManagerService->started_ = true;
    std::vector<std::u16string> args;
    int32_t ret = ERR_OK;
    ret =  dumpManagerService->Dump(-1, args);
    ASSERT_TRUE(ret == ERR_OK);
    ret = dumpManagerService->Request(args, -1);
    ASSERT_TRUE(ret == ERR_OK);

    dumpManagerService->OnStop();
}

/**
 * @tc.name: DumpManagerService003
 * @tc.desc: Test DumpManagerService CountFdNums.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperServiceTest, DumpManagerService003, TestSize.Level3)
{
    auto dumpManagerService = std::make_shared<DumpManagerService>();
    int32_t pid = 1;
    uint32_t fdNums = 0;
    std::string detailFdInfo;
    std::string topLeakedType;
    int32_t ret = dumpManagerService->CountFdNums(pid, fdNums, detailFdInfo, topLeakedType);
    ASSERT_TRUE(ret == 0);
    ASSERT_FALSE(detailFdInfo.empty());
    ASSERT_FALSE(topLeakedType.empty());
}

/**
 * @tc.name: DumpManagerService004
 * @tc.desc: Test DumpManagerService ScanPidOverLimit.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperServiceTest, DumpManagerService004, TestSize.Level3)
{
    auto dumpManagerService = std::make_shared<DumpManagerService>();
    std::string requestType = "fd";
    std::vector<int32_t> pidList;
    int32_t ret = dumpManagerService->ScanPidOverLimit(requestType, LIMIT_SIZE, pidList);
    ASSERT_TRUE(ret == 0);
    ret = dumpManagerService->ScanPidOverLimit(requestType, 0, pidList);
    ASSERT_TRUE(ret == 0);

    ret = dumpManagerService->ScanPidOverLimit(requestType, -1, pidList);
    ASSERT_TRUE(ret != 0);

    requestType = "..";
    ret = dumpManagerService->ScanPidOverLimit(requestType, 1, pidList);
    ASSERT_TRUE(ret == 0);
}

/**
 * @tc.name: DumpManagerService005
 * @tc.desc: Test DumpManagerService Request.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperServiceTest, DumpManagerService005, TestSize.Level3)
{
    auto dumpManagerService = std::make_shared<DumpManagerService>();
    dumpManagerService->OnStart();
    dumpManagerService->started_ = true;
    dumpManagerService->OnStart();

    std::vector<std::u16string> args;
    dumpManagerService->blockRequest_ = true;
    int32_t ret = dumpManagerService->Request(args, -1);
    ASSERT_TRUE(ret != 0);
    dumpManagerService->blockRequest_ = false;
    dumpManagerService->started_ = false;
    ret = dumpManagerService->Request(args, -1);
    ASSERT_TRUE(ret != 0);
    dumpManagerService->OnStop();
    dumpManagerService->started_ = false;
    dumpManagerService->OnStop();
}

/**
 * @tc.name: DumpManagerService006
 * @tc.desc: Test DumpManagerService error request.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperServiceTest, DumpManagerService006, TestSize.Level3)
{
    auto dumpManagerService = std::make_shared<DumpManagerService>();
    std::vector<std::u16string> args;
    int outfd = -1;
    dumpManagerService->HandleRequestError(args, outfd, -1, "test");
    ASSERT_TRUE(args.size() == 0);
    args.push_back(Str8ToStr16("test"));
    dumpManagerService->HandleRequestError(args, outfd, -1, "test");
    ASSERT_TRUE(args.size() == 0);
}

/**
 * @tc.name: DumpManagerService007
 * @tc.desc: Test DumpManagerService OnIdle.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperServiceTest, DumpManagerService007, TestSize.Level3)
{
    auto dumpManagerService = std::make_shared<DumpManagerService>();
    SystemAbilityOnDemandReason idleReason;
    int32_t ret = dumpManagerService->OnIdle(idleReason);
    ASSERT_TRUE(ret == 0);
    idleReason.SetId(OnDemandReasonId::INTERFACE_CALL);
    ret = dumpManagerService->OnIdle(idleReason);
    ASSERT_TRUE(ret == 0);
    std::vector<std::u16string> args;
    args.push_back(Str8ToStr16("test"));
    dumpManagerService->AddRequestRawParam(args, -1);
    ret = dumpManagerService->OnIdle(idleReason);
    ASSERT_TRUE(ret == 120000);
    dumpManagerService->SetCpuSchedAffinity();
}
} // namespace HiviewDFX
} // namespace OHOS