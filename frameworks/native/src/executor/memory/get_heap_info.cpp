/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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

#include "executor/memory/get_heap_info.h"
#include "executor/memory/memory_util.h"
#include "executor/memory/memory_info.h"
#include "hilog_wrapper.h"
#include "util/string_utils.h"
using namespace std;
namespace OHOS {
namespace HiviewDFX {
GetHeapInfo::GetHeapInfo()
{
}

GetHeapInfo::~GetHeapInfo()
{
}
#ifdef HIDUMPER_ABILITY_RUNTIME_ENABLE
OHOS::sptr<OHOS::AppExecFwk::IAppMgr> GetHeapInfo::GetAppManagerInstance()
{
    OHOS::sptr<OHOS::ISystemAbilityManager> systemAbilityManager =
        OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    OHOS::sptr<OHOS::IRemoteObject> appObject = systemAbilityManager->GetSystemAbility(OHOS::APP_MGR_SERVICE_ID);
    return OHOS::iface_cast<OHOS::AppExecFwk::IAppMgr>(appObject);
}
#endif

void GetHeapInfo::GetMallocHeapInfo(const int& pid, std::unique_ptr<MallHeapInfo>& mallocHeapInfo)
{
    DUMPER_HILOGI(MODULE_SERVICE, "GetMallocHeapInfo pid:%{public}d begin.", pid);
    mallocHeapInfo->size = 0;
    mallocHeapInfo->alloc = 0;
    mallocHeapInfo->free = 0;
#ifdef HIDUMPER_ABILITY_RUNTIME_ENABLE
    OHOS::sptr<OHOS::AppExecFwk::IAppMgr> appManager = GetAppManagerInstance();
    if (appManager == nullptr) {
        DUMPER_HILOGE(MODULE_SERVICE, "GetHeapInfo: Get the appManager is nullptr.");
    }
    OHOS::AppExecFwk::MallocInfo mallocInfo;
    int ret = appManager->DumpHeapMemory(pid, mallocInfo);
    if (ret != ERR_OK) {
        DUMPER_HILOGE(MODULE_SERVICE, "DumpHeapMemory return failed, ret is:%{public}d", ret);
        return;
    } else {
        mallocHeapInfo->size = mallocInfo.hblkhd / numberSys;
        mallocHeapInfo->alloc = mallocInfo.uordblks / numberSys;
        mallocHeapInfo->free = mallocInfo.fordblks / numberSys;
    }
    DUMPER_HILOGD(MODULE_SERVICE, "DumpHeapMemory result: %{public}i, uordblks: %{public}llu, fordblks: %{public}llu,"
        "hblkhd: %{public}llu", ret, static_cast<unsigned long long>(mallocInfo.fordblks),
        static_cast<unsigned long long>(mallocInfo.uordblks), static_cast<unsigned long long>(mallocInfo.hblkhd));
#endif
    DUMPER_HILOGI(MODULE_SERVICE, "GetMallocHeapInfo pid:%{public}d end, success!", pid);
}
} // namespace HiviewDFX
} // namespace OHOS
