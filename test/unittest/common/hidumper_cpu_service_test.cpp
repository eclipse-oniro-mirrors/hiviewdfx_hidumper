/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include <gtest/gtest.h>
#include <iservice_registry.h>
#include "dump_manager_cpu_client.h"
#include "dump_manager_cpu_service.h"
#include "dump_common_utils.h"
#include "inner/dump_service_id.h"
using namespace std;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::HiviewDFX;
namespace OHOS {
namespace HiviewDFX {
class HidumperCpuServiceTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
protected:
    static const uint32_t TEST_PID = 1;
};

void HidumperCpuServiceTest::SetUpTestCase(void)
{
}

void HidumperCpuServiceTest::TearDownTestCase(void)
{
}

void HidumperCpuServiceTest::SetUp(void)
{
}

void HidumperCpuServiceTest::TearDown(void)
{
}

/**
 * @tc.name: HidumperCpuServiceTest001
 * @tc.desc: Test DumpManagerCpuService service ready.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperCpuServiceTest, HidumperCpuServiceTest001, TestSize.Level3)
{
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    ASSERT_TRUE(sam != nullptr) << "HidumperCpuServiceTest001 fail to get GetSystemAbilityManager";
    sptr<IRemoteObject> remoteObject = sam->CheckSystemAbility(DFX_SYS_HIDUMPER_CPU_ABILITY_ID);
    ASSERT_TRUE(remoteObject != nullptr) << "GetCpuSystemAbility failed.";
}

/**
 * @tc.name: HidumperCpuServiceTest002
 * @tc.desc: Test of send request to dump spuusage.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperCpuServiceTest, HidumperCpuServiceTest002, TestSize.Level3)
{
    auto dumpManagerCpuService = std::make_shared<DumpManagerCpuService>();
    dumpManagerCpuService->StartService();
    dumpManagerCpuService->OnStart();
    auto dumpCpuData = std::make_shared<DumpCpuData>();
    int ret = dumpManagerCpuService->Request(*dumpCpuData);
    ASSERT_TRUE(ret == 0) << "dump cpuusage failed.";
}

/**
 * @tc.name: HidumperCpuServiceTest003
 * @tc.desc: Test of send request to dump spuusage.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperCpuServiceTest, HidumperCpuServiceTest003, TestSize.Level3)
{
    auto dumpManagerCpuService = std::make_shared<DumpManagerCpuService>();
    dumpManagerCpuService->StartService();
    double cpuUsage;
    int ret = dumpManagerCpuService->GetCpuUsageByPid(TEST_PID, cpuUsage);
    ASSERT_TRUE(ret == 0) << "dump cpuusage failed.";
}

/**
 * @tc.name: HidumperCpuServiceTest004
 * @tc.desc: Test error cpuservice.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperCpuServiceTest, HidumperCpuServiceTest004, TestSize.Level3)
{
    auto dumpManagerCpuService = std::make_shared<DumpManagerCpuService>();
    dumpManagerCpuService->OnStart();
    dumpManagerCpuService->OnStart();
    dumpManagerCpuService->curCPUInfo_ = nullptr;
    int32_t ret = dumpManagerCpuService->DumpCpuUsageData();
    ASSERT_TRUE(ret != 0) << "dump cpuusage failed.";
}

/**
 * @tc.name: HidumperCpuServiceTest005
 * @tc.desc: Test error cpuservice.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperCpuServiceTest, HidumperCpuServiceTest005, TestSize.Level3)
{
    auto dumpManagerCpuService = std::make_shared<DumpManagerCpuService>();
    auto dumpCpuData = std::make_shared<DumpCpuData>();
    dumpCpuData->cpuUsagePid_ = TEST_PID;
    dumpManagerCpuService->InitParam(*dumpCpuData);
    dumpManagerCpuService->DumpProcInfo();
    ASSERT_TRUE(dumpManagerCpuService->curSpecProc_ != nullptr) << "init param failed.";
    dumpManagerCpuService->ResetParam();
    ASSERT_TRUE(dumpManagerCpuService->curSpecProc_ == nullptr) << "reset param failed.";
}

/**
 * @tc.name: HidumperCpuServiceTest006
 * @tc.desc: Test error cpuservice.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperCpuServiceTest, HidumperCpuServiceTest006, TestSize.Level3)
{
    auto dumpManagerCpuService = std::make_shared<DumpManagerCpuService>();
    auto dumpCpuData = std::make_shared<DumpCpuData>();
    dumpManagerCpuService->StartService();
    dumpManagerCpuService->OnStart();
    dumpCpuData->cpuUsagePid_ = TEST_PID;
    dumpManagerCpuService->InitParam(*dumpCpuData);
    dumpManagerCpuService->ResetParam();
    int32_t ret = dumpManagerCpuService->DumpCpuUsageData();
    ASSERT_TRUE(ret != 0) << "dump cpuusage failed.";
}

/**
 * @tc.name: HidumperCpuServiceTest007
 * @tc.desc: Test error cpuservice.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperCpuServiceTest, HidumperCpuServiceTest007, TestSize.Level3)
{
    auto dumpManagerCpuService = std::make_shared<DumpManagerCpuService>();
    struct tm timeInfo = {0};
    timeInfo.tm_year = 101;
    timeInfo.tm_mon = 0;
    timeInfo.tm_mday = 1;
    timeInfo.tm_hour = 1;
    timeInfo.tm_min = 1;
    timeInfo.tm_sec = 1;
    time_t timestamp = mktime(&timeInfo);

    std::string dateTime;
    bool ret = dumpManagerCpuService->GetDateAndTime(static_cast<uint64_t>(timestamp), dateTime);

    EXPECT_TRUE(ret);
    EXPECT_EQ(dateTime, " 2001-01-01 01:01:01");
}

/**
 * @tc.name: HidumperCpuServiceTest008
 * @tc.desc: Test error cpuservice.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperCpuServiceTest, HidumperCpuServiceTest008, TestSize.Level3)
{
    auto dumpManagerCpuService = std::make_shared<DumpManagerCpuService>();
    struct tm timeInfo = {0};
    timeInfo.tm_year = 123;
    timeInfo.tm_mon = 10;
    timeInfo.tm_mday = 15;
    timeInfo.tm_hour = 12;
    timeInfo.tm_min = 30;
    timeInfo.tm_sec = 45;
    time_t timestamp = mktime(&timeInfo);

    std::string dateTime;
    bool ret = dumpManagerCpuService->GetDateAndTime(static_cast<uint64_t>(timestamp), dateTime);

    EXPECT_TRUE(ret);
    EXPECT_EQ(dateTime, " 2023-11-15 12:30:45");
}

/**
 * @tc.name: HidumperCpuServiceTest009
 * @tc.desc: Test error cpuservice.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperCpuServiceTest, HidumperCpuServiceTest009, TestSize.Level3) {
    auto dumpManagerCpuService = std::make_shared<DumpManagerCpuService>();
    auto p1 = std::make_shared<ProcInfo>();
    p1->totalUsage = 50.0;
    p1->userSpaceUsage = 10.0;
    p1->sysSpaceUsage = 10.0;
    p1->pid = "100";

    auto p2 = std::make_shared<ProcInfo>();
    p2->totalUsage = 30.0;
    p2->userSpaceUsage = 20.0;
    p2->sysSpaceUsage = 5.0;
    p2->pid = "2000";

    EXPECT_FALSE(dumpManagerCpuService->SortProcInfo(p2, p1));
    EXPECT_TRUE(dumpManagerCpuService->SortProcInfo(p1, p2));

    p2->totalUsage = 50.0;
    EXPECT_FALSE(dumpManagerCpuService->SortProcInfo(p1, p2));
    EXPECT_TRUE(dumpManagerCpuService->SortProcInfo(p2, p1));

    p1->userSpaceUsage = 20.0;
    EXPECT_FALSE(dumpManagerCpuService->SortProcInfo(p2, p1));
    EXPECT_TRUE(dumpManagerCpuService->SortProcInfo(p1, p2));

    p1->sysSpaceUsage = 5.0;
    EXPECT_FALSE(dumpManagerCpuService->SortProcInfo(p1, p2));
    EXPECT_TRUE(dumpManagerCpuService->SortProcInfo(p2, p1));
}
} // namespace HiviewDFX
} // namespace OHOS