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
using namespace std;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::HiviewDFX;
namespace OHOS {
namespace HiviewDFX {
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
    sptr<IRemoteObject> remoteObject = sam->CheckSystemAbility(DFX_SYS_HIDUMPER_ABILITY_ID);
    ASSERT_TRUE(remoteObject != nullptr) << "GetSystemAbility failed.";
}
} // namespace HiviewDFX
} // namespace OHOS