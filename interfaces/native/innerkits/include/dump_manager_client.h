/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#ifndef HIDUMPER_SERVICES_DUMP_MANAGER_CLIENT_H
#define HIDUMPER_SERVICES_DUMP_MANAGER_CLIENT_H
#include <string>
#include <singleton.h>
#include <iservice_registry.h>
#include "dump_common_utils.h"
#include "idump_broker.h"
namespace OHOS {
namespace HiviewDFX {
class DumpManagerClient final : public DelayedRefSingleton<DumpManagerClient> {
    DECLARE_DELAYED_REF_SINGLETON(DumpManagerClient)
public:
    DISALLOW_COPY_AND_MOVE(DumpManagerClient);
    // Used for dump request
    int32_t Request(std::vector<std::u16string> &args, int outfd);
public:
    // Used for connect to hidump SA.
    ErrCode Connect();
    bool IsConnected();
    // Used for reset connect.
    void Reset();
private:
    class DumpManagerDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        DumpManagerDeathRecipient() = default;
        ~DumpManagerDeathRecipient() = default;
        void OnRemoteDied(const wptr<IRemoteObject>& remote);
    private:
        DISALLOW_COPY_AND_MOVE(DumpManagerDeathRecipient);
    };
private:
    void ResetProxy(const wptr<IRemoteObject>& remote);
    ErrCode OnDemandStart(sptr<ISystemAbilityManager> sam, sptr<IRemoteObject> &remoteObject);
private:
    sptr<IDumpBroker> proxy_ {nullptr};
    sptr<IRemoteObject::DeathRecipient> deathRecipient_ {nullptr};
    std::mutex mutex_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIDUMPER_SERVICES_DUMP_MANAGER_CLIENT_H
