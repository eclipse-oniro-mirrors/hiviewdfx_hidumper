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
#ifndef HIDUMPER_ZIDL_DUMP_BROKER_CPU_PROXY_H
#define HIDUMPER_ZIDL_DUMP_BROKER_CPU_PROXY_H
#include <iremote_proxy.h>
#include <nocopyable.h>
#include "idump_cpu_broker.h"

namespace OHOS {
namespace HiviewDFX {
class DumpBrokerCpuProxy : public IRemoteProxy<IDumpCpuBroker> {
public:
    explicit DumpBrokerCpuProxy(const sptr<IRemoteObject>& impl)
        : IRemoteProxy<IDumpCpuBroker>(impl) {}
    ~DumpBrokerCpuProxy() = default;
    DISALLOW_COPY_AND_MOVE(DumpBrokerCpuProxy);
public:
    // Used for dump request
    int32_t Request(DumpCpuData &dumpCpuData) override;
    int32_t GetCpuUsageByPid(int32_t pid, double &cpuUsage) override;
private:
    static inline BrokerDelegator<DumpBrokerCpuProxy> delegator_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIDUMPER_ZIDL_DUMP_BROKER_PROXY_H
