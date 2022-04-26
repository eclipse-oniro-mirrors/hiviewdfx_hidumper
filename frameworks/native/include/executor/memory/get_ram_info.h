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
#ifndef GET_RAM_INFO_H
#define GET_RAM_INFO_H
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

    using PairMatrix = std::vector<std::pair<std::string, uint64_t>>;
    using PairMatrixGroup = std::vector<std::pair<std::string, PairMatrix>>;

    Ram GetRam(const PairMatrixGroup &smapsInfo, const PairMatrix &meminfo);

private:
    uint64_t GetPairMatrixGroupValue(const PairMatrixGroup &infos, const std::vector<std::string> keys);
    uint64_t GetPairMatrixValue(const PairMatrix &infos, const std::vector<std::string> strs);
    uint64_t GetTotalPss(const PairMatrixGroup &infos);
    uint64_t GetTotalSwapPss(const PairMatrixGroup &infos);
    uint64_t GetFreeInfo(const PairMatrix &infos);
    uint64_t GetKernelUsedInfo(const PairMatrix &infos);
    uint64_t GetCachedInfo(const PairMatrix &infos);
    uint64_t GetTotalRam(const PairMatrix &infos);
    uint64_t GetZramTotalInfo(const PairMatrix &infos);
    uint64_t GetUsedRam(const PairMatrixGroup &smapsInfo, const PairMatrix &meminfo, Ram &ram);
    uint64_t GetFreeRam(const PairMatrix &meminfo, Ram &ram);
    uint64_t GetLostRam(const PairMatrixGroup &smapsInfo, const PairMatrix &meminfo);
};
} // namespace HiviewDFX
} // namespace OHOS
#endif