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
#include "dump_broker_proxy.h"
#include <message_parcel.h>
#include "dump_errors.h"
#include "hidumper_service_ipc_interface_code.h"
#include "hilog_wrapper.h"
namespace OHOS {
namespace HiviewDFX {
int32_t DumpBrokerProxy::Request(std::vector<std::u16string> &args, int outfd)
{
    int32_t ret = -1;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        return ret;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(DumpBrokerProxy::GetDescriptor())) {
        return ret;
    }
    if (!data.WriteString16Vector(args)) {
        return ERROR_WRITE_PARCEL;
    }
    if (!data.WriteFileDescriptor(outfd)) {
        return ERROR_WRITE_PARCEL;
    }
    int res = remote->SendRequest(static_cast<int>(HidumperServiceInterfaceCode::DUMP_REQUEST_FILEFD),
        data, reply, option);
    if (res != ERR_OK) {
        DUMPER_HILOGE(MODULE_ZIDL, "error|SendRequest error code: %{public}d", res);
        return ret;
    }
    if (!reply.ReadInt32(ret)) {
        return ERROR_READ_PARCEL;
    }
    return ret;
}

int32_t DumpBrokerProxy::ScanPidOverLimit(std::string requestType, int32_t limitSize, std::vector<int32_t> &pidList)
{
    int32_t ret = -1;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        return ret;
    }
    MessageParcel data;
    MessageOption option;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(DumpBrokerProxy::GetDescriptor())) {
        return ret;
    }
    data.WriteString(requestType);
    data.WriteInt32(limitSize);
    int res = remote->SendRequest(static_cast<int>(HidumperServiceInterfaceCode::SCAN_PID_OVER_LIMIT),
        data, reply, option);
    if (res != ERR_OK) {
        DUMPER_HILOGE(MODULE_ZIDL, "send ScanPidOverLimit error code: %{public}d.", res);
        return ret;
    }
    if (!reply.ReadInt32Vector(&pidList)) {
        return ERROR_READ_PARCEL;
    }
    if (!reply.ReadInt32(ret)) {
        return ERROR_READ_PARCEL;
    }
    return ret;
}

int32_t DumpBrokerProxy::CountFdNums(int32_t pid, uint32_t &fdNums,
    std::string &detailFdInfo, std::string &topLeakedType)
{
    int32_t ret = -1;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        return ret;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(DumpBrokerProxy::GetDescriptor())) {
        return ret;
    }
    data.WriteInt32(pid);
    MessageParcel reply;
    MessageOption option;
    int res = remote->SendRequest(static_cast<int>(HidumperServiceInterfaceCode::COUNT_FD_NUMS),
        data, reply, option);
    if (res != ERR_OK) {
        DUMPER_HILOGE(MODULE_ZIDL, "send CountFdNums error code: %{public}d.", res);
        return ret;
    }
    if (!reply.ReadUint32(fdNums)) {
        return ERROR_READ_PARCEL;
    }
    if (!reply.ReadString(detailFdInfo)) {
        return ERROR_READ_PARCEL;
    }
    if (!reply.ReadString(topLeakedType)) {
        return ERROR_READ_PARCEL;
    }
    if (!reply.ReadInt32(ret)) {
        return ERROR_READ_PARCEL;
    }
    DUMPER_HILOGI(MODULE_ZIDL, "sucess to count fd nums, pid is %{public}d", pid);
    return ret;
}
} // namespace HiviewDFX
} // namespace OHOS
