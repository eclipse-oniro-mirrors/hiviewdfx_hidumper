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
#include "dump_broker_stub.h"
#include <message_parcel.h>
#include "dump_errors.h"
#include "hilog_wrapper.h"
namespace OHOS {
namespace HiviewDFX {
int DumpBrokerStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::u16string descripter = DumpBrokerStub::GetDescriptor();
    std::u16string remoteDescripter = data.ReadInterfaceToken();
    if (descripter != remoteDescripter) {
        return ERROR_GET_DUMPER_SERVICE;
    }
    int ret = ERR_OK;
    switch (code) {
        case static_cast<int>(IDumpBroker::DUMP_REQUEST_FILEFD): {
            DUMPER_HILOGD(MODULE_ZIDL, "debug|RequestFileFdStub");
            ret = RequestFileFdStub(data, reply);
            break;
        }
        default: {
            ret = IPCObjectStub::OnRemoteRequest(code, data, reply, option);
            break;
        }
    }
    return ret;
}

int32_t DumpBrokerStub::RequestFileFdStub(MessageParcel& data, MessageParcel& reply)
{
    int32_t ret = ERR_OK;
    std::vector<std::u16string> args;
    if (!data.ReadString16Vector(&args)) {
        return ERROR_READ_PARCEL;
    }
    int outfd = data.ReadFileDescriptor();
    int32_t res = Request(args, outfd);
    if (!reply.WriteInt32(res)) {
        return ERROR_WRITE_PARCEL;
    }
    return ret;
}
} // namespace HiviewDFX
} // namespace OHOS
