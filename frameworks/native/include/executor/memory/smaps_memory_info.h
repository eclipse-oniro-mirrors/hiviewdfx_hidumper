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
#include <future>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "common.h"
#include "time.h"
#include "executor/memory/parse/meminfo_data.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
static const std::string SMAPS_MEMINFO_RSS = "Rss";
static const std::string SMAPS_MEMINFO_PSS = "Pss";
static const std::string SMAPS_MEMINFO_SHARED_CLEAN = "Shared_Clean";
static const std::string SMAPS_MEMINFO_SHARED_DIRTY = "Shared_Dirty";
static const std::string SMAPS_MEMINFO_PRIVATE_CLEAN = "Private_Clean";
static const std::string SMAPS_MEMINFO_PRIVATE_DIRTY = "Private_Dirty";
static const std::string SMAPS_MEMINFO_SWAP = "Swap";
static const std::string SMAPS_MEMINFO_SWAP_PSS = "SwapPss";
static const std::string SMAPS_MEMINFO_SIZE = "Size";
static const std::string SMAPS_MEMINFO_NAME = "Name";
static const std::string SMAPS_MEMINFO_COUNTS = "Counts";
}
class SmapsMemoryInfo {
public:
    SmapsMemoryInfo();
    ~SmapsMemoryInfo();

    using StringMatrix = std::shared_ptr<std::vector<std::vector<std::string>>>;
    using ValueMap = std::map<std::string, uint64_t>;
    using GroupMap = std::map<std::string, ValueMap>;
    using MemSmapsFun = std::function<void(MemInfoData::MemSmapsInfo&, uint64_t)>;
    bool ShowMemorySmapsByPid(const int &pid, StringMatrix result, bool isShowSmapsInfo);

private:
    enum Status {
        SUCCESS_MORE_DATA = 1,
        FAIL_MORE_DATA = 2,
        SUCCESS_NO_MORE_DATA = 3,
        FAIL_NO_MORE_DATA = 4,
    };

    void InsertSmapsTitle(StringMatrix result, bool isShowSmapsInfo);
    void BuildSmapsResult(const GroupMap &infos, StringMatrix result, bool isShowSmapsInfo,
        std::vector<std::map<std::string, std::string>> vectMap);
    void BuildSmapsInfo(StringMatrix result, std::vector<std::map<std::string, std::string>> vectMap);
    bool CalcSmapsStatData(MemInfoData::MemSmapsInfo &memSmapsInfo, const GroupMap &infos);
    bool CalcSmapsInfo(MemInfoData::MemSmapsInfo &memSmapsInfo,
        std::vector<std::map<std::string, std::string>> vectMap);
    void CalcSmapsGroup(const GroupMap &infos, StringMatrix result, MemInfoData::MemSmapsInfo &memSmapsInfo,
        bool isShowSmapsInfo, std::vector<std::map<std::string, std::string>> vectMap);
    void SetRss(MemInfoData::MemSmapsInfo &meminfo, uint64_t value);
    void SetPss(MemInfoData::MemSmapsInfo &meminfo, uint64_t value);
    void SetSharedClean(MemInfoData::MemSmapsInfo &meminfo, uint64_t value);
    void SetSharedDirty(MemInfoData::MemSmapsInfo &meminfo, uint64_t value);
    void SetPrivateClean(MemInfoData::MemSmapsInfo &meminfo, uint64_t value);
    void SetPrivateDirty(MemInfoData::MemSmapsInfo &meminfo, uint64_t value);
    void SetSwap(MemInfoData::MemSmapsInfo &meminfo, uint64_t value);
    void SetSwapPss(MemInfoData::MemSmapsInfo &meminfo, uint64_t value);
    void SetSize(MemInfoData::MemSmapsInfo &meminfo, uint64_t value);
    void SetName(MemInfoData::MemSmapsInfo &meminfo, const std::string &value);
    void SetCounts(MemInfoData::MemSmapsInfo &meminfo, uint64_t value);
    std::vector<std::pair<std::string, MemSmapsFun>> sMapsMethodVec_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif
