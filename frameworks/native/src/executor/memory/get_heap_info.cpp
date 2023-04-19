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

bool GetHeapInfo::GetInfo(const MemoryFilter::MemoryType &memType, const int &pid, GroupMap &infos)
{
    DUMPER_HILOGI(MODULE_SERVICE, "GetHeapInfo: GetInfo memType:%{public}d pid:%{public}d begin.", memType, pid);
    struct MallHeapInfo heapInfo = {0};
#ifdef HIDUMPER_ABILITY_RUNTIME_ENABLE
    OHOS::sptr<OHOS::AppExecFwk::IAppMgr> appManager = GetAppManagerInstance();
    if (appManager == nullptr) {
        DUMPER_HILOGE(MODULE_SERVICE, "GetHeapInfo: Get the appManager is nullptr.");
        return false;
    }
    OHOS::AppExecFwk::MallocInfo mallocInfo;
    int ret = appManager->DumpHeapMemory(pid, mallocInfo);
    if (ret != ERR_OK) {
        DUMPER_HILOGE(MODULE_SERVICE, "DumpHeapMemory return failed, ret is:%{public}d", ret);
    } else {
        heapInfo.size = mallocInfo.usmblks / numberSys;
        heapInfo.alloc = mallocInfo.uordblks / numberSys;
        heapInfo.free = mallocInfo.fordblks / numberSys;
    }
    DUMPER_HILOGD(MODULE_SERVICE, "Dumper GetInfo DumpHeapMemory result: %{public}i, usmblks: %{public}i, uordblks: \
        %{public}i, fordblks: %{public}i", ret, mallocInfo.usmblks, mallocInfo.uordblks, mallocInfo.fordblks);
#endif
    for (const auto &info : infos) {
        vector<string> pageTag;
        StringUtils::GetInstance().StringSplit(info.first, "#", pageTag);
        if (pageTag.size() <= 1) {
            continue;
        }

        string group;
        if (pageTag[1] == "other") {
            group = pageTag[0] == MemoryFilter::GetInstance().FILE_PAGE_TAG ? "FilePage other" : "AnonPage other";
        } else {
            group = pageTag[1];
        }

        if (groupNative == group) {
            infos[info.first].insert(pair<string, uint64_t>(MEMINFO_HEAP_SIZE, heapInfo.size));
            infos[info.first].insert(pair<string, uint64_t>(MEMINFO_HEAP_ALLOC, heapInfo.alloc));
            infos[info.first].insert(pair<string, uint64_t>(MEMINFO_HEAP_FREE, heapInfo.free));
            break;
        }
    }

    DUMPER_HILOGI(MODULE_SERVICE, "GetHeapInfo: GetInfo memType:%{public}d pid:%{public}d end, success!", memType, pid);
    return true;
}
} // namespace HiviewDFX
} // namespace OHOS
