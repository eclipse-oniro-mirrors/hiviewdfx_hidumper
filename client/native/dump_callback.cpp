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
#include "dump_callback.h"
#include <unistd.h>
#include "dump_manager_client.h"
namespace OHOS {
namespace HiviewDFX {
const int DumpCallback::WAIT_START_MAX = 3; // wait 3 second for STATUS_INIT
const int DumpCallback::WATT_COMPLETE = 200;

DumpCallback::DumpCallback(int outFd) : outFd_(outFd), status_(IDumpCallbackBroker::STATUS_INIT)
{
}

DumpCallback::~DumpCallback()
{
}

DumpCallback::RemoteDeathRecipient::RemoteDeathRecipient(uint32_t& status) : status_(status)
{
}

void DumpCallback::RemoteDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    status_ = IDumpCallbackBroker::STATUS_DUMP_ERROR;
}

void DumpCallback::OnStatusChanged(uint32_t status)
{
    switch (status) {
        case IDumpCallbackBroker::STATUS_DUMP_STARTED: {
            status_ = IDumpCallbackBroker::STATUS_DUMP_STARTED;
            break;
        }
        case IDumpCallbackBroker::STATUS_DUMP_FINISHED: {
            status_ = IDumpCallbackBroker::STATUS_DUMP_FINISHED;
            break;
        }
        case IDumpCallbackBroker::STATUS_DUMP_ERROR: {
            status_ = IDumpCallbackBroker::STATUS_DUMP_ERROR;
            break;
        }
        default:
            break;
    }
}

bool DumpCallback::WaitCompleted()
{
    if (deathRecipient_ == nullptr) {
        deathRecipient_ = sptr<IRemoteObject::DeathRecipient>(new RemoteDeathRecipient(status_));
    }
    AddDeathRecipient(deathRecipient_);

    auto& dumpManagerClient = DumpManagerClient::GetInstance();
    for (int i = 0; dumpManagerClient.IsConnected(); i++) {
        if ((i >= WAIT_START_MAX) && (status_ == IDumpCallbackBroker::STATUS_INIT)) {
            (void)dprintf(outFd_, "start error\n");
            break;
        }
        if (status_ == IDumpCallbackBroker::STATUS_DUMP_FINISHED) {
            break;
        }
        if (status_ == IDumpCallbackBroker::STATUS_DUMP_ERROR) {
            (void)dprintf(outFd_, "dump error\n");
            break;
        }
        usleep(WATT_COMPLETE);
    }

    RemoveDeathRecipient(deathRecipient_);
    return true;
}

sptr<DumpCallback> DumpCallback::CreateCallback(int outFd)
{
    return sptr<DumpCallback>(new DumpCallback(outFd));
}
} // namespace HiviewDFX
} // namespace OHOS
