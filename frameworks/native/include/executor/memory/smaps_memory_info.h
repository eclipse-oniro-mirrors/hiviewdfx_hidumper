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
#ifndef SMAPS_MEMORY_INFO_H
#define SMAPS_MEMORY_INFO_H
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "common.h"
#include "executor/memory/parse/parse_smaps_info.h"

namespace OHOS {
namespace HiviewDFX {
class SmapsMemoryInfo {
public:
    SmapsMemoryInfo();
    ~SmapsMemoryInfo();

    using StringMatrix = std::shared_ptr<std::vector<std::vector<std::string>>>;
    bool ShowMemorySmapsByPid(const int &pid, StringMatrix result, bool isShowSmapsInfo);

private:
    void InsertSmapsTitle(StringMatrix result, bool isShowSmapsInfo);
    void SetValueForRet(const std::string& value, const int &width, std::vector<std::string>& tempResult);
    void SetOneRowMemInfo(const MemoryData &memoryData, bool isShowSmapsInfo, bool isSummary, StringMatrix result);
    void UpdateShowAddressMemInfoResult(const std::vector<MemoryData>& showAddressMemInfoVec, StringMatrix result);
    void UpdateCountSameNameMemResult(std::map<std::string, MemoryData>& countSameNameMemMap, StringMatrix result);
};
} // namespace HiviewDFX
} // namespace OHOS
#endif
