/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#include "executor/memory/dump_heap_info.h"
#include "executor/memory/memory_util.h"
#include "executor/memory/memory_info.h"
#include "hilog_wrapper.h"
#include "util/string_utils.h"

using namespace std;
namespace OHOS {
namespace HiviewDFX {
DumpHeapInfo::DumpHeapInfo()
{
}

DumpHeapInfo::~DumpHeapInfo()
{
}

bool DumpHeapInfo::DumpHeapMemory(OHOS::AppExecFwk::MemDumpInfo &info, std::string &dumpResult)
{
    DUMPER_HILOGI(MODULE_SERVICE, "DumpHeapMemory pid:%{public}d, "
        "dumpType:%{public}d, needLeakobj:%{public}d, isSync:%{public}d",
        info.pid, info.dumpType, info.needLeakobj, info.isSync);
#ifdef HIDUMPER_ABILITY_RUNTIME_ENABLE
    OHOS::sptr<OHOS::ISystemAbilityManager> systemAbilityManager =
        OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    OHOS::sptr<OHOS::AppExecFwk::IAppMgr> appManager =
        OHOS::iface_cast<OHOS::AppExecFwk::IAppMgr>(systemAbilityManager->GetSystemAbility(OHOS::APP_MGR_SERVICE_ID));
    if (appManager == nullptr) {
        DUMPER_HILOGE(MODULE_SERVICE, "DumpHeapMemory: Get the appManager is nullptr.");
        return false;
    }

    if (info.isSync) {
        sptr<MemDumpCallbackImpl> callback = new MemDumpCallbackImpl();
        if (callback == nullptr) {
            DUMPER_HILOGE(MODULE_SERVICE, "DumpHeapMemory: Create callback failed.");
            return false;
        }

        int ret = appManager->DumpMem(info, callback);
        if (ret != ERR_OK) {
            DUMPER_HILOGE(MODULE_SERVICE, "DumpHeapMemory call failed, ret is:%{public}d", ret);
            return false;
        }

        if (!callback->WaitForResult(dumpResult)) {
            DUMPER_HILOGE(MODULE_SERVICE, "DumpHeapMemory wait result timeout");
            return false;
        }
        if (dumpResult.empty()) {
            DUMPER_HILOGE(MODULE_SERVICE, "DumpHeapMemory dumpResult is empty");
            return false;
        }
        DUMPER_HILOGI(MODULE_SERVICE, "DumpHeapMemory: dumpResult:%{public}s", dumpResult.c_str());
    } else {
        int ret = appManager->DumpMem(info, nullptr);
        if (ret != ERR_OK) {
            DUMPER_HILOGE(MODULE_SERVICE, "DumpHeapMemory async call failed, ret is:%{public}d", ret);
            return false;
        }
        DUMPER_HILOGI(MODULE_SERVICE, "DumpHeapMemory: async mode, no result returned");
    }
#endif
    return true;
}
} // namespace HiviewDFX
} // namespace OHOS
