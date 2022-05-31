/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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

uint64_t GetRamInfo::GetGroupMapValue(const GroupMap &infos, const vector<string> keys) const
{
    uint64_t totalValue = 0;
    for (const auto &info : infos) {
        auto &valueMap = info.second;
        for (const auto &str : keys) {
            auto it = valueMap.find(str);
            if (it != valueMap.end()) {
                totalValue += it->second;
            }
        }
    }
    return totalValue;
}

uint64_t GetRamInfo::GetValueMapValue(const ValueMap &infos, const vector<string> strs) const
{
    uint64_t totalValue = 0;
    for (const auto &str : strs) {
        auto it = infos.find(str);
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

uint64_t GetRamInfo::GetTotalPss(const GroupMap &infos) const
{
    uint64_t totalValue = GetGroupMapValue(infos, MemoryFilter::GetInstance().CALC_TOTAL_PSS_);
    return totalValue;
}

uint64_t GetRamInfo::GetTotalSwapPss(const GroupMap &infos) const
{
    uint64_t totalValue = GetGroupMapValue(infos, MemoryFilter::GetInstance().CALC_TOTAL_SWAP_PSS_);
    return totalValue;
}

uint64_t GetRamInfo::GetFreeInfo(const ValueMap &infos) const
{
    uint64_t totalValue = GetValueMapValue(infos, MemoryFilter::GetInstance().CALC_FREE_);
    return totalValue;
}

uint64_t GetRamInfo::GetKernelUsedInfo(const ValueMap &infos) const
{
    uint64_t totalValue = GetValueMapValue(infos, MemoryFilter::GetInstance().CALC_KERNEL_USED_);
    return totalValue;
}

uint64_t GetRamInfo::GetCachedInfo(const ValueMap &infos) const
{
    uint64_t totalValue = GetValueMapValue(infos, MemoryFilter::GetInstance().CALC_CACHED_);
    return totalValue;
}

uint64_t GetRamInfo::GetTotalRam(const ValueMap &infos) const
{
    uint64_t totalValue = GetValueMapValue(infos, MemoryFilter::GetInstance().CALC_TOTAL_);
    return totalValue;
}

uint64_t GetRamInfo::GetZramTotalInfo(const ValueMap &infos) const
{
    uint64_t totalValue = GetValueMapValue(infos, MemoryFilter::GetInstance().CALC_ZARM_TOTAL_);
    return totalValue;
}

uint64_t GetRamInfo::GetUsedRam(const GroupMap &smapsInfo, const ValueMap &meminfo, Ram &ram) const
{
    ram.totalPss = GetTotalPss(smapsInfo);
    ram.kernelUsed = GetKernelUsedInfo(meminfo);
    uint64_t totalValue = ram.totalPss + ram.kernelUsed;
    return totalValue;
}

uint64_t GetRamInfo::GetFreeRam(const ValueMap &meminfo, Ram &ram) const
{
    ram.cachedInfo = GetCachedInfo(meminfo);
    ram.freeInfo = GetFreeInfo(meminfo);
    uint64_t totalValue = ram.cachedInfo + ram.freeInfo;
    return totalValue;
}

uint64_t GetRamInfo::GetLostRam(const GroupMap &smapsInfo, const ValueMap &meminfo) const
{
    uint64_t totalValue = GetTotalRam(meminfo) - (GetTotalPss(smapsInfo) - GetTotalSwapPss(smapsInfo))
                      - GetFreeInfo(meminfo) - GetCachedInfo(meminfo) - GetKernelUsedInfo(meminfo)
                      - GetZramTotalInfo(meminfo);
    return totalValue;
}

GetRamInfo::Ram GetRamInfo::GetRam(const GroupMap &smapsInfo, const ValueMap &meminfo) const
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