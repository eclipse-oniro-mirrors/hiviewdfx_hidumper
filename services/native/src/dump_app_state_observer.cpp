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
#ifdef HIDUMPER_ABILITY_BASE_ENABLE
#include "dump_app_state_observer.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "hilog_wrapper.h"
#include "dump_manager_cpu_service.h"

namespace OHOS {
namespace HiviewDFX {
DumpAppStateObserver::DumpAppStateObserver()
{}

DumpAppStateObserver::~DumpAppStateObserver()
{}

OHOS::sptr<OHOS::AppExecFwk::IAppMgr> DumpAppStateObserver::GetAppManagerInstance()
{
    DUMPER_HILOGI(MODULE_SERVICE, "GetAppManagerInstance");

    OHOS::sptr<OHOS::ISystemAbilityManager> systemAbilityManager =
        OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    OHOS::sptr<OHOS::IRemoteObject> appObject = systemAbilityManager->GetSystemAbility(OHOS::APP_MGR_SERVICE_ID);
    return OHOS::iface_cast<OHOS::AppExecFwk::IAppMgr>(appObject);
}

bool DumpAppStateObserver::SubscribeAppState()
{
    DUMPER_HILOGI(MODULE_SERVICE, "RegisterAppState");
    if (appProcessState_) {
        DUMPER_HILOGE(MODULE_SERVICE, "appStateObserver has subscribed");
        return true;
    }

    appProcessState_ = new (std::nothrow) AppProcessState(*this);
    if (appProcessState_ == nullptr) {
        DUMPER_HILOGE(MODULE_SERVICE, "create AppProcessState fail, not enough memory");
        return false;
    }

    sptr<AppExecFwk::IAppMgr> appObject = GetAppManagerInstance();
    if (appObject == nullptr) {
        DUMPER_HILOGE(MODULE_SERVICE, "get SystemAbilityManager fail");
        return false;
    }
    int ret = appObject->RegisterApplicationStateObserver(appProcessState_);
    if (ret != ERR_OK) {
        appProcessState_.clear();
        appProcessState_ = nullptr;
        DUMPER_HILOGE(MODULE_SERVICE, "register fail, ret = %{public}d", ret);
        return false;
    }
    DUMPER_HILOGD(MODULE_SERVICE, "register success");
    return true;
}

bool DumpAppStateObserver::UnsubscribeAppState()
{
    DUMPER_HILOGI(MODULE_SERVICE, "UnsubscribeAppState");

    if (appProcessState_ == nullptr) {
        DUMPER_HILOGE(MODULE_SERVICE, "appStateObserver has not subscribed");
        return true;
    }

    sptr<AppExecFwk::IAppMgr> appMgr = GetAppManagerInstance();
    if (appMgr == nullptr) {
        DUMPER_HILOGE(MODULE_SERVICE, "get app mgr fail!");
        return false;
    }
    if (appMgr->UnregisterApplicationStateObserver(appProcessState_) != ERR_OK) {
        DUMPER_HILOGE(MODULE_SERVICE, "UnregisterApplicationStateObserver fail!");
        return false;
    }
    appProcessState_.clear();
    appProcessState_ = nullptr;
    return true;
}

void DumpAppStateObserver::SendUpdateCpuInfoEvent()
{
    DUMPER_HILOGI(MODULE_SERVICE, "SendUpdateCpuInfoEvent|enter");
    auto dumpManagerService = DumpDelayedSpSingleton<DumpManagerCpuService>::GetInstance();
    if (dumpManagerService == nullptr) {
        DUMPER_HILOGE(MODULE_SERVICE, "get dump manager service null");
        return;
    }
    dumpManagerService->SendImmediateEvent();
}

void DumpAppStateObserver::AppProcessState::OnProcessCreated(const AppExecFwk::ProcessData &processData)
{
    DUMPER_HILOGI(MODULE_SERVICE, "OnProcessCreated pid=%{public}d, bundleName=%{public}s", processData.pid,
        processData.bundleName.c_str());
    DumpAppStateObserver::GetInstance().SendUpdateCpuInfoEvent();
}

void DumpAppStateObserver::AppProcessState::OnProcessDied(const AppExecFwk::ProcessData &processData)
{
    DUMPER_HILOGI(MODULE_SERVICE, "OnProcessDied pid=%{public}d, bundleName=%{public}s", processData.pid,
        processData.bundleName.c_str());
    DumpAppStateObserver::GetInstance().SendUpdateCpuInfoEvent();
}
} // namespace HiviewDFX
} // namespace OHOS
#endif
