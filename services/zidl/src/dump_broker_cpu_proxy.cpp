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
#include "dump_broker_cpu_proxy.h"
#include <message_parcel.h>
#include "dump_errors.h"
#include "hidumper_cpu_service_ipc_interface_code.h"
#include "hilog_wrapper.h"
namespace OHOS {
namespace HiviewDFX {
int32_t DumpBrokerCpuProxy::Request(DumpCpuData &dumpCpuData)
{
    int32_t ret = -1;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        return ret;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(DumpBrokerCpuProxy::GetDescriptor())) {
        return ret;
    }
    if (!data.WriteParcelable(&dumpCpuData)) {
        return ERROR_WRITE_PARCEL;
    }
    int res = remote->SendRequest(static_cast<int>(HidumperCpuServiceInterfaceCode::DUMP_REQUEST_CPUINFO),
        data, reply, option);
    if (res != ERR_OK) {
        DUMPER_HILOGE(MODULE_CPU_ZIDL, "error|SendCpuRequest error code: %{public}d", res);
        return ret;
    }
    std::shared_ptr<DumpCpuData> dumpCpuDataPtr(reply.ReadParcelable<DumpCpuData>());
    if (dumpCpuDataPtr == nullptr) {
        DUMPER_HILOGE(MODULE_CPU_ZIDL, "error|ReadParcelable error");
        return ret;
    }
    if (!reply.ReadInt32(ret)) {
        return ERROR_READ_PARCEL;
    }
    dumpCpuData = *dumpCpuDataPtr;
    return ret;
}

int32_t DumpBrokerCpuProxy::GetCpuUsageByPid(int32_t pid, double &cpuUsage)
{
    int32_t ret = -1;
    sptr<IRemoteObject> remoteObject = Remote();
    if (remoteObject == nullptr) {
        return ret;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(DumpBrokerCpuProxy::GetDescriptor())) {
        return ret;
    }
    if (!data.WriteInt32(pid)) {
        return ERROR_WRITE_PARCEL;
    }
    int res = remoteObject->SendRequest(static_cast<int>(HidumperCpuServiceInterfaceCode::DUMP_USAGE_ONLY),
        data, reply, option);
    if (res != ERR_OK) {
        DUMPER_HILOGE(MODULE_CPU_SERVICE, "error|SendCpuRequest error code: %{public}d", res);
        return ret;
    }
    if (!reply.ReadDouble(cpuUsage)) {
        return ERROR_READ_PARCEL;
    }
    if (cpuUsage < 0) {
        return ret;
    }
    return ERR_OK;
}
} // namespace HiviewDFX
} // namespace OHOS
