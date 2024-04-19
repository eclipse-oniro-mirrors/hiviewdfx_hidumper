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
#include "dump_manager_cpu_client.h"
#include <iservice_registry.h>
#include <string_ex.h>
#include <unistd.h>
#include "hilog_wrapper.h"
#include "dump_errors.h"
#include "inner/dump_service_id.h"
#include "dump_on_demand_load.h"
#include "dump_broker_cpu_proxy.h"
namespace OHOS {
namespace HiviewDFX {
DumpManagerCpuClient::DumpManagerCpuClient()
{
}

DumpManagerCpuClient::~DumpManagerCpuClient()
{
    Reset();
}

int32_t DumpManagerCpuClient::Request(DumpCpuData &dumpCpuData)
{
    if (Connect() != ERR_OK) {
        DUMPER_HILOGE(MODULE_CPU_CLIENT, "debug|cpu connect error");
        return DumpStatus::DUMP_FAIL;
    }
    int32_t ret = proxy_->Request(dumpCpuData);
    return ret;
}

int32_t DumpManagerCpuClient::GetCpuUsageByPid(int32_t pid, double &cpuUsage)
{
    if (Connect() != ERR_OK) {
        DUMPER_HILOGE(MODULE_CPU_CLIENT, "cpu connect error");
        return DumpStatus::DUMP_FAIL;
    }
    int32_t ret = proxy_->GetCpuUsageByPid(pid, cpuUsage);
    return ret;
}

ErrCode DumpManagerCpuClient::Connect()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (proxy_ != nullptr) {
        return ERR_OK;
    }
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        DUMPER_HILOGE(MODULE_CPU_CLIENT, "sam is null");
        return ERROR_GET_SYSTEM_ABILITY_MANAGER;
    }
    sptr<IRemoteObject> remoteObject = sam->CheckSystemAbility(DFX_SYS_HIDUMPER_CPU_ABILITY_ID);
    if (remoteObject == nullptr) {
        DUMPER_HILOGE(MODULE_CPU_CLIENT, "cpu remoteobject is null");
        return ERROR_GET_DUMPER_SERVICE;
    }
    deathRecipient_ = sptr<IRemoteObject::DeathRecipient>(new DumpManagerCpuDeathRecipient());
    if (deathRecipient_ == nullptr) {
        DUMPER_HILOGE(MODULE_CPU_CLIENT, "cpu deathRecipient_ is null");
        return ERR_NO_MEMORY;
    }
    if ((remoteObject->IsProxyObject()) && (!remoteObject->AddDeathRecipient(deathRecipient_))) {
        DUMPER_HILOGE(MODULE_CPU_CLIENT, "cpu IsProxyObject is null");
        return ERROR_ADD_DEATH_RECIPIENT;
    }
    proxy_ = iface_cast<IDumpCpuBroker>(remoteObject);
    return ERR_OK;
}

void DumpManagerCpuClient::Reset()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (proxy_ == nullptr) {
        return;
    }
    auto serviceRemote = proxy_->AsObject();
    if (serviceRemote != nullptr) {
        serviceRemote->RemoveDeathRecipient(deathRecipient_);
        proxy_ = nullptr;
        DUMPER_HILOGD(MODULE_CPU_CLIENT, "debug|disconnected");
    }
}

void DumpManagerCpuClient::ResetProxy(const wptr<IRemoteObject>& remote)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (proxy_ == nullptr) {
        return;
    }
    auto serviceRemote = proxy_->AsObject();
    if ((serviceRemote != nullptr) && (serviceRemote == remote.promote())) {
        serviceRemote->RemoveDeathRecipient(deathRecipient_);
        proxy_ = nullptr;
        DUMPER_HILOGD(MODULE_CPU_CLIENT, "debug|disconnected");
    }
}

void DumpManagerCpuClient::DumpManagerCpuDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    if (remote == nullptr) {
        return;
    }
    DumpManagerCpuClient::GetInstance().ResetProxy(remote);
}
} // namespace HiviewDFX
} // namespace OHOS
