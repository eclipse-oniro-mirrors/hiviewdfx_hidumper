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
#include "dump_callback_broker_stub.h"
#include <message_parcel.h>
#include "dump_errors.h"
#include "hilog_wrapper.h"
namespace OHOS {
namespace HiviewDFX {
int DumpCallbackBrokerStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    std::u16string descripter = DumpCallbackBrokerStub::GetDescriptor();
    std::u16string remoteDescripter = data.ReadInterfaceToken();
    if (descripter != remoteDescripter) {
        return ERROR_GET_DUMPER_SERVICE;
    }
    int ret = ERR_OK;
    switch (code) {
        case static_cast<int>(IDumpCallbackBroker::ON_STATUS_CHANGED): {
            DUMPER_HILOGD(MODULE_ZIDL, "debug|OnStatusChangedStub");
            ret = OnStatusChangedStub(data);
            break;
        }
        default: {
            ret = IPCObjectStub::OnRemoteRequest(code, data, reply, option);
            break;
        }
    }
    return ret;
}

void DumpCallbackBrokerStub::OnStatusChanged(uint32_t status)
{
}

int32_t DumpCallbackBrokerStub::OnStatusChangedStub(MessageParcel& data)
{
    int32_t ret = ERR_OK;
    uint32_t status;
    if (!data.ReadUint32(status)) {
        return ERROR_READ_PARCEL;
    }
    OnStatusChanged(status);
    return ret;
}
} // namespace HiviewDFX
} // namespace OHOS
