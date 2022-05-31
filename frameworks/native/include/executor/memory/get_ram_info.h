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
#ifndef GET_RAM_INFO_H
#define GET_RAM_INFO_H
#include <map>
#include <string>
#include <vector>
#include <memory>
namespace OHOS {
namespace HiviewDFX {
class GetRamInfo {
public:
    GetRamInfo();
    ~GetRamInfo();

    struct Ram {
        uint64_t total {0};
        uint64_t free {0};
        uint64_t used {0};
        uint64_t lost {0};
        uint64_t totalPss {0};
        uint64_t kernelUsed {0};
        uint64_t cachedInfo {0};
        uint64_t freeInfo {0};
    };

    using ValueMap = std::map<std::string, uint64_t>;
    using GroupMap = std::map<std::string, ValueMap>;

    Ram GetRam(const GroupMap &smapsInfo, const ValueMap &meminfo) const;

private:
    uint64_t GetGroupMapValue(const GroupMap &infos, const std::vector<std::string> keys) const;
    uint64_t GetValueMapValue(const ValueMap &infos, const std::vector<std::string> strs) const;
    uint64_t GetTotalPss(const GroupMap &infos) const;
    uint64_t GetTotalSwapPss(const GroupMap &infos) const;
    uint64_t GetFreeInfo(const ValueMap &infos) const;
    uint64_t GetKernelUsedInfo(const ValueMap &infos) const;
    uint64_t GetCachedInfo(const ValueMap &infos) const;
    uint64_t GetTotalRam(const ValueMap &infos) const;
    uint64_t GetZramTotalInfo(const ValueMap &infos) const;
    uint64_t GetUsedRam(const GroupMap &smapsInfo, const ValueMap &meminfo, Ram &ram) const;
    uint64_t GetFreeRam(const ValueMap &meminfo, Ram &ram) const;
    uint64_t GetLostRam(const GroupMap &smapsInfo, const ValueMap &meminfo) const;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif