/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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
#ifndef GET_DMA_INFO_H
#define GET_DMA_INFO_H
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "executor/memory/parse/meminfo_data.h"

namespace OHOS {
namespace HiviewDFX {
class DmaInfo {
public:
    DmaInfo();
    ~DmaInfo();

    using ValueMap = std::map<uint32_t, uint64_t>;
    using DmaInfoMap = std::map<uint64_t, MemInfoData::DmaInfo>;

    bool ParseDmaInfo();
    uint64_t GetDmaByPid(const int32_t &pid) const;
    uint64_t GetTotalDma();

private:
    void CreateDmaInfo(const std::string &line);

    DmaInfoMap dmaInfos_;
    ValueMap dmaMap_;
    bool initialized_ = false;
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // GET_DMA_INFO_H