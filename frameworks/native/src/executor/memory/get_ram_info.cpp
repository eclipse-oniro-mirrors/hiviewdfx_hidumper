/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#include "executor/memory/get_ram_info.h"
#include "executor/memory/memory_util.h"
#include "executor/memory/memory_filter.h"
#include "util/string_utils.h"
using namespace std;
namespace OHOS {
namespace HiviewDFX {
GetRamInfo::GetRamInfo()
{
}
GetRamInfo::~GetRamInfo()
{
}

uint64_t GetRamInfo::GetPairMatrixGroupValue(const PairMatrixGroup &infos, const vector<string> keys)
{
    uint64_t totalValue = 0;
    for (auto &info : infos) {
        auto valueMap = info.second;
        for (auto str : keys) {
            map<string, uint64_t>::iterator it = valueMap.find(str);
            if (it != valueMap.end()) {
                totalValue += it->second;
            }
        }
    }
    return totalValue;
}

uint64_t GetRamInfo::GetPairMatrixValue(const PairMatrix &infos, const vector<string> strs)
{
    uint64_t totalValue = 0;
    for (auto str : strs) {
        map<string, uint64_t>::const_iterator it = infos.find(str);
        if (it != infos.end()) {
            if (str == "Mapped") {
                totalValue -= it->second;
            } else {
                totalValue += it->second;
            }
        }
    }
    return totalValue;
}

uint64_t GetRamInfo::GetTotalPss(const PairMatrixGroup &infos)
{
    uint64_t totalValue = GetPairMatrixGroupValue(infos, MemoryFilter::GetInstance().CALC_TOTAL_PSS_);
    return totalValue;
}

uint64_t GetRamInfo::GetTotalSwapPss(const PairMatrixGroup &infos)
{
    uint64_t totalValue = GetPairMatrixGroupValue(infos, MemoryFilter::GetInstance().CALC_TOTAL_SWAP_PSS_);
    return totalValue;
}

uint64_t GetRamInfo::GetFreeInfo(const PairMatrix &infos)
{
    uint64_t totalValue = GetPairMatrixValue(infos, MemoryFilter::GetInstance().CALC_FREE_);
    return totalValue;
}

uint64_t GetRamInfo::GetKernelUsedInfo(const PairMatrix &infos)
{
    uint64_t totalValue = GetPairMatrixValue(infos, MemoryFilter::GetInstance().CALC_KERNEL_USED_);
    return totalValue;
}

uint64_t GetRamInfo::GetCachedInfo(const PairMatrix &infos)
{
    uint64_t totalValue = GetPairMatrixValue(infos, MemoryFilter::GetInstance().CALC_CACHED_);
    return totalValue;
}

uint64_t GetRamInfo::GetTotalRam(const PairMatrix &infos)
{
    uint64_t totalValue = GetPairMatrixValue(infos, MemoryFilter::GetInstance().CALC_TOTAL_);
    return totalValue;
}

uint64_t GetRamInfo::GetZramTotalInfo(const PairMatrix &infos)
{
    uint64_t totalValue = GetPairMatrixValue(infos, MemoryFilter::GetInstance().CALC_ZARM_TOTAL_);
    return totalValue;
}

uint64_t GetRamInfo::GetUsedRam(const PairMatrixGroup &smapsInfo, const PairMatrix &meminfo, Ram &ram)
{
    ram.totalPss = GetTotalPss(smapsInfo);
    ram.kernelUsed = GetKernelUsedInfo(meminfo);
    uint64_t totalValue = ram.totalPss + ram.kernelUsed;
    return totalValue;
}

uint64_t GetRamInfo::GetFreeRam(const PairMatrix &meminfo, Ram &ram)
{
    ram.cachedInfo = GetCachedInfo(meminfo);
    ram.freeInfo = GetFreeInfo(meminfo);
    uint64_t totalValue = ram.cachedInfo + ram.freeInfo;
    return totalValue;
}

uint64_t GetRamInfo::GetLostRam(const PairMatrixGroup &smapsInfo, const PairMatrix &meminfo)
{
    uint64_t totalValue = GetTotalRam(meminfo) - (GetTotalPss(smapsInfo) - GetTotalSwapPss(smapsInfo))
                      - GetFreeInfo(meminfo) - GetCachedInfo(meminfo) - GetKernelUsedInfo(meminfo)
                      - GetZramTotalInfo(meminfo);
    return totalValue;
}

GetRamInfo::Ram GetRamInfo::GetRam(const PairMatrixGroup &smapsInfo, const PairMatrix &meminfo)
{
    Ram ram;

    ram.total = GetTotalRam(meminfo);
    ram.used = GetUsedRam(smapsInfo, meminfo, ram);
    ram.free = GetFreeRam(meminfo, ram);
    ram.lost = GetLostRam(smapsInfo, meminfo);

    return ram;
}
} // namespace HiviewDFX
} // namespace OHOS