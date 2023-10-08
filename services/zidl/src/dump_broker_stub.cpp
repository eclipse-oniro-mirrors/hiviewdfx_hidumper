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
#include "hidumper_service_ipc_interface_code.h"
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
        case static_cast<int>(HidumperServiceInterfaceCode::DUMP_REQUEST_FILEFD): {
            DUMPER_HILOGD(MODULE_ZIDL, "debug|RequestFileFdStub");
            ret = RequestFileFdStub(data, reply);
            break;
        }
        case static_cast<int>(HidumperServiceInterfaceCode::SCAN_PID_OVER_LIMIT): {
            DUMPER_HILOGD(MODULE_ZIDL, "debug|ScanPidOverLimitStub");
            ret = ScanPidOverLimitStub(data, reply);
            break;
        }
        case static_cast<int>(HidumperServiceInterfaceCode::COUNT_FD_NUMS): {
            DUMPER_HILOGD(MODULE_ZIDL, "debug|CountFdNumsStub");
            ret = CountFdNumsStub(data, reply);
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
    if (outfd < 0) {
    {
        return ERROR_READ_PARCEL;
    }
    int32_t res = Request(args, outfd);
    close(outfd);
    if (!reply.WriteInt32(res)) {
        return ERROR_WRITE_PARCEL;
    }
    return ret;
}

int32_t DumpBrokerStub::ScanPidOverLimitStub(MessageParcel& data, MessageParcel& reply)
{
    int32_t ret = ERR_OK;
    std::vector<int32_t> pidList;
    std::string requestType = data.ReadString();
    int32_t limitSize = data.ReadInt32();
    ret = ScanPidOverLimit(requestType, limitSize, pidList);
    if (!reply.WriteInt32Vector(pidList)) {
        return ERROR_WRITE_PARCEL;
    }
    if (!reply.WriteInt32(ret)) {
        return ERROR_WRITE_PARCEL;
    }
    return ret;
}

int32_t DumpBrokerStub::CountFdNumsStub(MessageParcel& data, MessageParcel& reply)
{
    int32_t ret = ERR_OK;
    uint32_t fdNums = 0;
    std::string detailFdInfo;
    std::string topLeakedType;

    int32_t pid = data.ReadInt32();
    ret = CountFdNums(pid, fdNums, detailFdInfo, topLeakedType);
    if (!reply.WriteInt32(fdNums)) {
        return ERROR_WRITE_PARCEL;
    }
    if (!reply.WriteString(detailFdInfo)) {
        return ERROR_WRITE_PARCEL;
    }
    if (!reply.WriteString(topLeakedType)) {
        return ERROR_WRITE_PARCEL;
    }
    if (!reply.WriteInt32(ret)) {
        return ERROR_WRITE_PARCEL;
    }
    return ret;
}
} // namespace HiviewDFX
} // namespace OHOS
