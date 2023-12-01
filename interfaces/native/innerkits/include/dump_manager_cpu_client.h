/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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
#ifndef HIDUMPER_SERVICES_DUMP_MANAGER_CPU_CLIENT_H
#define HIDUMPER_SERVICES_DUMP_MANAGER_CPU_CLIENT_H
#include <string>
#include <singleton.h>
#include <iservice_registry.h>
#include "dump_common_utils.h"
#include "idump_cpu_broker.h"
#include "dump_cpu_data.h"
#include "common.h"
namespace OHOS {
namespace HiviewDFX {
class DumpManagerCpuClient final : public DelayedRefSingleton<DumpManagerCpuClient> {
    DECLARE_DELAYED_REF_SINGLETON(DumpManagerCpuClient)
public:
    DISALLOW_COPY_AND_MOVE(DumpManagerCpuClient);
public:
    int32_t Request(DumpCpuData &dumpCpuData);
    int32_t GetCpuUsageByPid(int32_t pid, int &cpuUsage);
    // Used for connect to hidump cpu SA.
    ErrCode Connect();
    // Used for reset connect.
    void Reset();
private:
    class DumpManagerCpuDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        DumpManagerCpuDeathRecipient() = default;
        ~DumpManagerCpuDeathRecipient() = default;
        void OnRemoteDied(const wptr<IRemoteObject>& remote);
    private:
        DISALLOW_COPY_AND_MOVE(DumpManagerCpuDeathRecipient);
    };
private:
    void ResetProxy(const wptr<IRemoteObject>& remote);
    ErrCode OnDemandStart(sptr<ISystemAbilityManager> sam, sptr<IRemoteObject> &remoteObject);
private:
    sptr<IDumpCpuBroker> proxy_ {nullptr};
    sptr<IRemoteObject::DeathRecipient> deathRecipient_ {nullptr};
    std::mutex mutex_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIDUMPER_SERVICES_DUMP_MANAGER_CLIENT_H
