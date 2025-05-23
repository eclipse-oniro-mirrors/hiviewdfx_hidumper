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
#ifndef HIDUMPER_ZIDL_DUMP_BROKER_PROXY_H
#define HIDUMPER_ZIDL_DUMP_BROKER_PROXY_H
#include <iremote_proxy.h>
#include <nocopyable.h>
#include "idump_broker.h"
namespace OHOS {
namespace HiviewDFX {
class DumpBrokerProxy : public IRemoteProxy<IDumpBroker> {
public:
    explicit DumpBrokerProxy(const sptr<IRemoteObject>& impl)
        : IRemoteProxy<IDumpBroker>(impl) {}
    ~DumpBrokerProxy() = default;
    DISALLOW_COPY_AND_MOVE(DumpBrokerProxy);
public:
    // Used for dump request
    int32_t Request(std::vector<std::u16string> &args, int outfd) override;
    // Used for scan pids list over limit
    int32_t ScanPidOverLimit(std::string requestType, int32_t limitSize, std::vector<int32_t> &pidList) override;
    // Used for count fd nums
    int32_t CountFdNums(int32_t pid, uint32_t &fdNums, std::string &detailFdInfo, std::string &topLeakedType) override;
private:
    static inline BrokerDelegator<DumpBrokerProxy> delegator_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIDUMPER_ZIDL_DUMP_BROKER_PROXY_H
