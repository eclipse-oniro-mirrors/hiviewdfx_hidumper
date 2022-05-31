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
#include "executor/memory/get_process_info.h"
#include "executor/memory/memory_filter.h"
#include "executor/memory/memory_util.h"
using namespace std;
namespace OHOS {
namespace HiviewDFX {
GetProcessInfo::GetProcessInfo()
{
}
GetProcessInfo::~GetProcessInfo()
{
}

/**
 * @description: get the value of process usage
 * @param {GroupMap} &infos-Smaps information
 * @return {uint64_t}-the value of
 */
uint64_t GetProcessInfo::GetProcess(const GroupMap &infos) const
{
    uint64_t totalValue = 0;
    for (const auto &info : infos) {
        auto &valueMap = info.second;
        for (const auto &str : MemoryFilter::GetInstance().CALC_PROCESS_TOTAL_) {
            auto it = valueMap.find(str);
            if (it != valueMap.end()) {
                totalValue += it->second;
            }
        }
    }
    return totalValue;
}
} // namespace HiviewDFX
} // namespace OHOS
