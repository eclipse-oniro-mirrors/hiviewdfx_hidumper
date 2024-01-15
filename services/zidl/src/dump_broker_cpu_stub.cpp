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
#include "dump_broker_cpu_stub.h"
#include <message_parcel.h>
#include "dump_cpu_data.h"
#include "dump_errors.h"
#include "hidumper_cpu_service_ipc_interface_code.h"
#include "hilog_wrapper.h"
#include <ipc_skeleton.h>
namespace OHOS {
namespace HiviewDFX {
const int APP_UID = 10000;
int DumpBrokerCpuStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::u16string descripter = DumpBrokerCpuStub::GetDescriptor();
    std::u16string remoteDescripter = data.ReadInterfaceToken();
    if (descripter != remoteDescripter) {
        return ERROR_GET_DUMPER_SERVICE;
    }
    int ret = ERR_OK;
    switch (code) {
        case static_cast<int>(HidumperCpuServiceInterfaceCode::DUMP_REQUEST_CPUINFO): {
            std::shared_ptr<DumpCpuData> dumpCpuData(data.ReadParcelable<DumpCpuData>());
            if (dumpCpuData == nullptr) {
                DUMPER_HILOGE(MODULE_CPU_ZIDL, "error|DumpBrokerCpuStub error");
                return ERROR_READ_PARCEL;
            }
            int32_t res = Request(*dumpCpuData);
            DumpCpuData dump = *dumpCpuData;
            if (!reply.WriteParcelable(&dump)) {
                return ERROR_WRITE_PARCEL;
            }
            if (!reply.WriteInt32(res)) {
                return ERROR_WRITE_PARCEL;
            }
            break;
        }
        case static_cast<int>(HidumperCpuServiceInterfaceCode::DUMP_USAGE_ONLY): {
            DUMPER_HILOGD(MODULE_CPU_ZIDL, "debug|DumpCpuUsageOnly");
            ret = DumpCpuUsageOnly(data, reply);
            break;
        }
        default: {
            ret = IPCObjectStub::OnRemoteRequest(code, data, reply, option);
            break;
        }
    }
    return ret;
}

int DumpBrokerCpuStub::DumpCpuUsageOnly(MessageParcel& data, MessageParcel& reply)
{
    int32_t pid = data.ReadInt32();
    int32_t calllingUid = IPCSkeleton::GetCallingUid();
    int32_t calllingPid = IPCSkeleton::GetCallingPid();
    if (calllingUid >= APP_UID && pid != calllingPid) {
        return ERROR_GET_DUMPER_SERVICE;
    }
    if (pid < 0) {
        return ERROR_READ_PARCEL;
    }
    int32_t usage = 0;
    int32_t res = GetCpuUsageByPid(pid, usage);
    if (!reply.WriteInt32(usage)) {
        return ERROR_WRITE_PARCEL;
    }
    if (res != 0) {
        return ERROR_WRITE_PARCEL;
    }
    return ERR_OK;
}
} // namespace HiviewDFX
} // namespace OHOS
