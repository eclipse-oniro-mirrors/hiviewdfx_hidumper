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
#include "dump_cpu_data.h"
#include "hilog_wrapper.h"
namespace OHOS {
namespace HiviewDFX {
DumpCpuData::DumpCpuData()
{
}

DumpCpuData::DumpCpuData(std::string& startTime, std::string& endTime, int cpuUsagePid, StringCpuMatrix dumpCPUDatas)
    : startTime_(startTime), endTime_(endTime), cpuUsagePid_(cpuUsagePid), dumpCPUDatas_(dumpCPUDatas)
{
}

DumpCpuData::~DumpCpuData()
{
}

bool DumpCpuData::Marshalling(Parcel& parcel) const
{
    RETURN_PARCEL_WRITE_HELPER_RET(parcel, String, startTime_, false);
    RETURN_PARCEL_WRITE_HELPER_RET(parcel, String, endTime_, false);
    RETURN_PARCEL_WRITE_HELPER_RET(parcel, Int32, cpuUsagePid_, false);
    if (!WriteStringMatrix(dumpCPUDatas_, parcel)) {
        DUMPER_HILOGE(MODULE_CPU_DATA, "failed to write dumpCPUDatas_");
        return false;
    }
    return true;
}

DumpCpuData *DumpCpuData::Unmarshalling(Parcel& parcel)
{
    DumpCpuData *info = new (std::nothrow) DumpCpuData();
    if (info == nullptr) {
        DUMPER_HILOGE(MODULE_CPU_DATA, "failed to create DumpCpuData");
        return nullptr;
    }
    if (!info->ReadFromParcel(parcel)) {
        DUMPER_HILOGE(MODULE_CPU_DATA, "failed to read from parcel");
        delete info;
        info = nullptr;
    }
    return info;
}

bool DumpCpuData::ReadFromParcel(Parcel &parcel)
{
    RETURN_PARCEL_READ_HELPER_RET(parcel, String, startTime_, false);
    RETURN_PARCEL_READ_HELPER_RET(parcel, String, endTime_, false);
    RETURN_PARCEL_READ_HELPER_RET(parcel, Int32, cpuUsagePid_, false);
    ReadStringMatrix(dumpCPUDatas_, parcel);
    return true;
}

bool DumpCpuData::WriteStringMatrix(const std::vector<std::vector<std::string>> &martrixVec, Parcel &data) const
{
    if (!data.WriteUint32(martrixVec.size())) {
        DUMPER_HILOGE(MODULE_CPU_DATA, "failed to WriteInt32 for martrixVec.size()");
        return false;
    }

    for (auto &parcelable : martrixVec) {
        if (!data.WriteStringVector(parcelable)) {
            DUMPER_HILOGE(MODULE_CPU_DATA, "failed to WriteParcelable for parcelable");
            return false;
        }
    }
    return true;
}

bool DumpCpuData::ReadStringMatrix(std::vector<std::vector<std::string>> &martrixVec, Parcel &data)
{
    int size = 0;
    if (!data.ReadInt32(size)) {
        return false;
    }
    if (size > INT_MAX) {
        return flase;
    }
    DUMPER_HILOGI(MODULE_CPU_DATA, "ReadStringMatrix size = %{public}d", size);
    martrixVec.clear();
    for (int i = 0; i < size; i++) {
        std::vector<std::string> vec;
        if (!data.ReadStringVector(&vec)) {
            DUMPER_HILOGE(MODULE_CPU_DATA, "failed to ReadStringMatrix for martrixVec");
            return false;
        }
        martrixVec.emplace_back(vec);
    }
    return true;
}
} // namespace HiviewDFX
} // namespace OHOS
