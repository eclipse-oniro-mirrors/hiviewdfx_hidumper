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
#ifndef HIDUMPER_CLIENT_DUMP_CALLBACK_H
#define HIDUMPER_CLIENT_DUMP_CALLBACK_H
#include "dump_callback_broker_stub.h"
namespace OHOS {
namespace HiviewDFX {
class DumpCallback : public DumpCallbackBrokerStub {
public:
    explicit DumpCallback(int outFd);
    virtual ~DumpCallback();
public:
    void OnStatusChanged(uint32_t status) override;
    bool WaitCompleted();
    static sptr<DumpCallback> CreateCallback(int outFd);
private:
    class RemoteDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        explicit RemoteDeathRecipient(uint32_t& status);
        virtual ~RemoteDeathRecipient() = default;
    public:
        virtual void OnRemoteDied(const wptr<IRemoteObject>& remote);
    private:
        uint32_t& status_;
    };
    sptr<IRemoteObject::DeathRecipient> deathRecipient_;
    int outFd_;
    uint32_t status_;
    static const int WAIT_START_MAX; // wait 3 second for STATUS_INIT
    static const int WATT_COMPLETE;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIDUMPER_CLIENT_DUMP_CALLBACK_H
