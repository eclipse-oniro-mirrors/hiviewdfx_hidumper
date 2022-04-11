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
#include "dump_manager_client.h"
#include <iservice_registry.h>
#include <string_ex.h>
#include "common.h"
#include "hilog_wrapper.h"
#include "dump_errors.h"
#include "inner/dump_service_id.h"
namespace OHOS {
namespace HiviewDFX {
DumpManagerClient::DumpManagerClient()
{
}

DumpManagerClient::~DumpManagerClient()
{
    if (proxy_ != nullptr) {
        auto remoteObject = proxy_->AsObject();
        if (remoteObject != nullptr) {
            remoteObject->RemoveDeathRecipient(deathRecipient_);
        }
    }
}

int32_t DumpManagerClient::Request(std::vector<std::u16string> &args, int outfd,
    const sptr<IDumpCallbackBroker>& callback)
{
    if ((args.size() < 1) || (outfd < 0) || (callback == nullptr)) {
        return DumpStatus::DUMP_FAIL;
    }
    for (size_t i = 0; i < args.size(); i++) {
        std::string trimArg = TrimStr(Str16ToStr8(args[i]));
        if (strlen(trimArg.c_str()) < 1) {
            return DumpStatus::DUMP_FAIL;
        }
    }
    if (Connect() != ERR_OK) {
        return DumpStatus::DUMP_FAIL;
    }
    int32_t ret = proxy_->Request(args, outfd, callback);
    DUMPER_HILOGD(MODULE_CLIENT, "debug|ret=%{public}d", ret);
    return ret;
}

ErrCode DumpManagerClient::Connect()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (proxy_ != nullptr) {
        return ERR_OK;
    }
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        return ERROR_GET_SYSTEM_ABILITY_MANAGER;
    }
    sptr<IRemoteObject> remoteObject = sam->CheckSystemAbility(DFX_SYS_HIDUMPER_ABILITY_ID);
    if (remoteObject == nullptr) {
        return ERROR_GET_DUMPER_SERVICE;
    }
    deathRecipient_ = sptr<IRemoteObject::DeathRecipient>(new DumpManagerDeathRecipient());
    if (deathRecipient_ == nullptr) {
        return ERR_NO_MEMORY;
    }
    if ((remoteObject->IsProxyObject()) && (!remoteObject->AddDeathRecipient(deathRecipient_))) {
        return ERROR_ADD_DEATH_RECIPIENT;
    }
    proxy_ = iface_cast<IDumpBroker>(remoteObject);
    DUMPER_HILOGD(MODULE_CLIENT, "debug|connected");
    return ERR_OK;
}

bool DumpManagerClient::IsConnected()
{
    return (proxy_ != nullptr);
}

void DumpManagerClient::Reset()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (proxy_ == nullptr) {
        return;
    }
    auto serviceRemote = proxy_->AsObject();
    if (serviceRemote != nullptr) {
        serviceRemote->RemoveDeathRecipient(deathRecipient_);
        proxy_ = nullptr;
        DUMPER_HILOGD(MODULE_CLIENT, "debug|disconnected");
    }
}

void DumpManagerClient::ResetProxy(const wptr<IRemoteObject>& remote)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (proxy_ == nullptr) {
        return;
    }
    auto serviceRemote = proxy_->AsObject();
    if ((serviceRemote != nullptr) && (serviceRemote == remote.promote())) {
        serviceRemote->RemoveDeathRecipient(deathRecipient_);
        proxy_ = nullptr;
        DUMPER_HILOGD(MODULE_CLIENT, "debug|disconnected");
    }
}

void DumpManagerClient::DumpManagerDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    if (remote == nullptr) {
        return;
    }
    DumpManagerClient::GetInstance().ResetProxy(remote);
}
} // namespace HiviewDFX
} // namespace OHOS
