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

#include "executor/memory/dump_arkts_heap_info.h"
#include "executor/memory/dump_heap_info.h"

#include <cstdlib>
#include <cerrno>
#include <climits>

using namespace std;
namespace OHOS {
namespace HiviewDFX {

static const int32_t ARKTS_HEAP_TIMEOUT_MS = 1000;

#ifdef HIDUMPER_ABILITY_RUNTIME_ENABLE
static OHOS::sptr<OHOS::AppExecFwk::IAppMgr> GetAppMgr()
{
    auto sam = OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        DUMPER_HILOGE(MODULE_SERVICE, "GetSystemAbilityManager failed");
        return nullptr;
    }
    return OHOS::iface_cast<OHOS::AppExecFwk::IAppMgr>(sam->GetSystemAbility(OHOS::APP_MGR_SERVICE_ID));
}
#endif

DumpArktsHeapInfo::DumpArktsHeapInfo() {}

DumpArktsHeapInfo::~DumpArktsHeapInfo() {}

bool DumpArktsHeapInfo::GetArktsHeapSize(int32_t pid, std::string &result)
{
#ifdef HIDUMPER_ABILITY_RUNTIME_ENABLE
    sptr<MemDumpCallbackImpl> callback = new MemDumpCallbackImpl();

    OHOS::AppExecFwk::MemDumpInfo info;
    info.pid = static_cast<uint32_t>(pid);
    info.dumpType = OHOS::AppExecFwk::MemDumpType::ARKTS_HEAP;
    info.isSync = true;
    info.needDump = false;

    auto appManager = GetAppMgr();
    if (appManager == nullptr) {
        DUMPER_HILOGE(MODULE_SERVICE, "Get appManager failed");
        return false;
    }

    int ret = appManager->DumpMem(info, callback);
    if (ret != ERR_OK) {
        DUMPER_HILOGE(MODULE_SERVICE, "DumpMem call failed, ret:%{public}d", ret);
        return false;
    }

    std::string dumpResult;
    if (!callback->WaitForResult(dumpResult, ARKTS_HEAP_TIMEOUT_MS)) {
        DUMPER_HILOGE(MODULE_SERVICE, "WaitForResult timeout (%{public}d ms)", ARKTS_HEAP_TIMEOUT_MS);
        return false;
    }

    if (dumpResult.empty()) {
        DUMPER_HILOGE(MODULE_SERVICE, "dumpResult is empty");
        return false;
    }

    result = dumpResult;
    DUMPER_HILOGI(MODULE_SERVICE, "GetArktsHeapSize success, pid:%{public}d, result:%{public}s", pid, result.c_str());
    return true;
#else
    return false;
#endif
}
} // namespace HiviewDFX
} // namespace OHOS
