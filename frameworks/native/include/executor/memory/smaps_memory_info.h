/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include <future>
#include <string>
#include <vector>
#include "executor/memory/parse/meminfo_data.h"
#include "common.h"
#include "time.h"
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
    bool ShowMemorySmapsByPid(const int &pid, StringMatrix result);

private:
    enum Status {
        SUCCESS_MORE_DATA = 1,
        FAIL_MORE_DATA = 2,
        SUCCESS_NO_MORE_DATA = 3,
        FAIL_NO_MORE_DATA = 4,
    };

    const int LINE_WIDTH_ = 14;
    const int LINE_NAME_VAL_WIDTH_ = 60;
    const int LINE_NAME_KEY_WIDTH_ = 28;
    const size_t TYPE_SIZE = 2;
    const char SEPARATOR_ = '-';
    const char BLANK_ = ' ';
    const static int NAME_SIZE_ = 2;
    const static int VSS_BIT = 4;
    const static int BYTE_PER_KB = 1024;
    std::future<GroupMap> fut_;
    std::vector<int> pids_;
    std::vector<std::pair<std::string, MemSmapsFun>> sMapsMethodVec_;
    void insertSmapsTitle(StringMatrix result);
    void BuildSmapsResult(const GroupMap &infos, StringMatrix result);
    void CalcSmapsGroup(const GroupMap &infos, StringMatrix result, MemInfoData::MemSmapsInfo &memSmapsInfo);
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
};
} // namespace HiviewDFX
} // namespace OHOS
#endif
