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
#ifdef HIDUMPER_ABILITY_BASE_ENABLE
#include "dump_app_state_observer.h"
#endif
#ifdef HIDUMPER_BATTERY_ENABLE
#include "dump_battery_stats_subscriber.h"
#endif
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

    dumpManagerCpuService->started_ = true;
    dumpManagerCpuService->OnStart();

    dumpManagerCpuService->EventHandlerInit();
    auto handler_ = dumpManagerCpuService->GetHandler();
    dumpManagerCpuService->SendImmediateEvent();

    auto dumpCpuData = std::make_shared<DumpCpuData>();
    int ret = dumpManagerCpuService->Request(*dumpCpuData);
    ASSERT_TRUE(ret == 0) << "dump cpuusage failed.";
}

/**
 * @tc.name: DumpAppStateObserver001
 * @tc.desc: Test of dump appState observer.
 * @tc.type: FUNC
 */
#ifdef HIDUMPER_ABILITY_BASE_ENABLE
HWTEST_F(HidumperCpuServiceTest, DumpAppStateObserver001, TestSize.Level3)
{
    auto dumpAppStateObserver = std::make_shared<DumpAppStateObserver>();
    ASSERT_FALSE(dumpAppStateObserver->SubscribeAppState());
    ASSERT_TRUE(dumpAppStateObserver->UnsubscribeAppState());
}
#endif

/**
 * @tc.name: DumpBatteryStatsSubscriber001
 * @tc.desc: Test the function callback dump battery stats subscriber.
 * @tc.type: FUNC
 */
#ifdef HIDUMPER_BATTERY_ENABLE
HWTEST_F(HidumperCpuServiceTest, DumpBatteryStatsSubscriber001, TestSize.Level3)
{
    const std::string COMMON_EVENT_BATTERY_CHANGED = "usual.event.BATTERY_CHANGED";
    OHOS::EventFwk::CommonEventData data;
    AAFwk::Want want = data.GetWant();
    want.SetAction(COMMON_EVENT_BATTERY_CHANGED);
    data.SetWant(want);

    const EventFwk::CommonEventSubscribeInfo subscribeInfo;
    DumpBatteryStatsSubscriber* subscriber = new DumpBatteryStatsSubscriber(subscribeInfo);

    subscriber->lastCapacity_ = -1;
    subscriber->OnReceiveEvent(data);
}
#endif
} // namespace HiviewDFX
} // namespace OHOS