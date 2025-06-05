/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "executor/memory/dump_cjheap_info.h"
#include "executor/memory/memory_util.h"
#include "executor/memory/memory_info.h"
#include "hilog_wrapper.h"
#include "util/string_utils.h"
using namespace std;
namespace OHOS {
namespace HiviewDFX {
DumpCjHeapInfo::DumpCjHeapInfo()
{
}

DumpCjHeapInfo::~DumpCjHeapInfo()
{
}

bool DumpCjHeapInfo::DumpCjHeapMemory(OHOS::AppExecFwk::CjHeapDumpInfo &info)
{
    DUMPER_HILOGI(MODULE_SERVICE, "DumpCjHeapMemory pid:%{public}d, needGc:%{public}d, needSnapshot:%{public}d",
        info.pid, info.needGc, info.needSnapshot);
#ifdef HIDUMPER_ABILITY_RUNTIME_ENABLE
    OHOS::sptr<OHOS::ISystemAbilityManager> systemAbilityManager =
        OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    OHOS::sptr<OHOS::AppExecFwk::IAppMgr> appManager =
        OHOS::iface_cast<OHOS::AppExecFwk::IAppMgr>(systemAbilityManager->GetSystemAbility(OHOS::APP_MGR_SERVICE_ID));
    if (appManager == nullptr) {
        DUMPER_HILOGE(MODULE_SERVICE, "DumpCjHeapMemory: Get the appManager is nullptr.");
        return false;
    }
    int ret = appManager->DumpCjHeapMemory(info);
    if (ret != ERR_OK) {
        DUMPER_HILOGE(MODULE_SERVICE, "DumpCjHeapMemory return failed, ret is:%{public}d", ret);
        return false;
    }
#endif
    return true;
}
} // namespace HiviewDFX
} // namespace OHOS
