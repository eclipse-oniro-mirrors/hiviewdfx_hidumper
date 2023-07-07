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
#ifndef DUMP_CPU_DATA_H
#define DUMP_CPU_DATA_H
#include <string>
#include <vector>
#include "parcel.h"
#include "util/dump_cpu_info_util.h"
#include "delayed_sp_singleton.h"
#include "dump_errors.h"
namespace OHOS {
namespace HiviewDFX {
#define RETURN_PARCEL_READ_HELPER_RET(parcel, type, out, failRet) \
    do { \
        bool ret = (parcel).Read##type((out)); \
        if (!ret) { \
            return failRet; \
        } \
    } while (0)

#define RETURN_PARCEL_WRITE_HELPER_RET(parcel, type, value, failRet) \
    do { \
        bool ret = (parcel).Write##type((value)); \
        if (!ret) { \
            return failRet; \
        } \
    } while (0)

class DumpCpuData : public Parcelable {
public:
    using StringCpuMatrix = std::vector<std::vector<std::string>>;
    DumpCpuData();
    DumpCpuData(std::string& startTime, std::string& endTime, int cpuUsagePid, StringCpuMatrix dumpCPUDatas);
    ~DumpCpuData();
    static DumpCpuData *Unmarshalling(Parcel& parcel);
    bool Marshalling(Parcel& parcel) const override;
    bool ReadFromParcel(Parcel &parcel);
    bool WriteStringMatrix(const std::vector<std::vector<std::string>> &martrixVec, Parcel &data) const;
    bool ReadStringMatrix(std::vector<std::vector<std::string>> &martrixVec, Parcel &data);
public:
    std::string startTime_;
    std::string endTime_;
    std::shared_ptr<CPUInfo> curCPUInfo_{nullptr};
    std::shared_ptr<CPUInfo> oldCPUInfo_{nullptr};
    std::shared_ptr<ProcInfo> curSpecProc_{nullptr};
    std::shared_ptr<ProcInfo> oldSpecProc_{nullptr};
    std::vector<std::shared_ptr<ProcInfo>> curProcs_;
    std::vector<std::shared_ptr<ProcInfo>> oldProcs_;
    int cpuUsagePid_ = -1;
    StringCpuMatrix dumpCPUDatas_{nullptr};
    DumperError errorCode;
private:
    friend DumpDelayedSpSingleton<DumpCpuData>;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif
