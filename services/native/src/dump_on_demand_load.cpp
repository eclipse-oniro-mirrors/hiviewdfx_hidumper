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
#include "dump_on_demand_load.h"
#include "hilog_wrapper.h"
namespace OHOS {
namespace HiviewDFX {
static sptr<IRemoteObject> remoteObjectPtr = nullptr;

OnDemandLoadCallback::OnDemandLoadCallback()
{
}

OnDemandLoadCallback::~OnDemandLoadCallback()
{
}

void OnDemandLoadCallback::OnLoadSystemAbilitySuccess(int32_t systemAbilityId, const sptr<IRemoteObject> &remoteObject)
{
    DUMPER_HILOGD(MODULE_CLIENT,
        "debug|OnLoadSystemAbilitySuccess systemAbilityId:%{public}d IRemoteObject result:%{public}s", systemAbilityId,
        ((remoteObject != nullptr) ? "succeed" : "failed"));
    remoteObjectPtr = remoteObject;
}

void OnDemandLoadCallback::OnLoadSystemAbilityFail(int32_t systemAbilityId)
{
    DUMPER_HILOGD(MODULE_CLIENT, "debug|OnLoadSystemAbilityFail systemAbilityId:%{public}d", systemAbilityId);
    remoteObjectPtr = nullptr;
}

sptr<IRemoteObject> OnDemandLoadCallback::GetLoadSystemAbilityRemoteObj()
{
    return remoteObjectPtr;
}
} // namespace HiviewDFX
} // namespace OHOS
