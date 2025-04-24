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
#ifndef PARSE_SMAPS_INFO_H
#define PARSE_SMAPS_INFO_H
#include <map>
#include <string>
#include <vector>
#include "executor/memory/memory_filter.h"
#include "executor/memory/parse/meminfo_data.h"
#include "memory_collector.h"
namespace OHOS {
namespace HiviewDFX {
struct MemoryData {
    std::string name = "[anon]";
    std::string memoryClass;
    std::string startAddr;  // 566ab4e000
    std::string endAddr;    // 566ab7c000
    std::string permission; // r-xp
    int counts = 0; // number of occurrences of the same name
    uint64_t size = 0; // calculate the sum of sizes corresponding to the same name.
    uint64_t rss = 0;
    uint64_t pss = 0;
    uint64_t swapPss = 0;
    uint64_t swap = 0;
    uint64_t sharedDirty = 0;
    uint64_t privateDirty = 0;
    uint64_t sharedClean = 0;
    uint64_t privateClean = 0;
    uint64_t iNode = 0;
};
class ParseSmapsInfo {
public:
    ParseSmapsInfo();
    ~ParseSmapsInfo();

    using ValueMap = std::map<std::string, uint64_t>;
    using GroupMap = std::map<std::string, ValueMap>;

    bool GetInfo(const MemoryFilter::MemoryType &memType, const int &pid, GroupMap &nativeMap, GroupMap &result);
    // countSameNameMemMap---key: name, value: MemoryData
    bool GetSmapsData(const int &pid, bool isShowSmapsAddress,
        std::map<std::string, MemoryData>& countSameNameMemMap, std::vector<MemoryData>& showAddressMemInfoVec);

private:
    const std::vector<std::string> MEMORY_CLASS_VEC = {
        "graph", "ark ts heap", ".db", "dev", "dmabuf", "guard", ".hap",
        "native heap", ".so", "stack", ".ttf", "other"
    };
    std::string memGroup_ = "";
    std::string nativeMemGroup_ = "";

    bool GetValue(const MemoryFilter::MemoryType &memType, const std::string &str, std::string &type, uint64_t &value);
    bool GetHasPidValue(const std::string &str, std::string &type, uint64_t &value);
    bool GetNoPidValue(const std::string &str, std::string &type, uint64_t &value);
    void UpdateShowAddressMemInfoVec(const std::vector<MemoryItem>& memoryItems, const std::string& memoryClassStr,
        std::vector<MemoryData>& showAddressMemInfoVec);
    void UpdateCountSameNameMemMap(const std::vector<MemoryItem>& memoryItems, const std::string& memoryClassStr,
        std::map<std::string, MemoryData>& countSameNameMemMap);
    void SetMemoryData(MemoryData &memoryData, const MemoryItem &memoryItem, const std::string& memoryClassStr);
};
} // namespace HiviewDFX
} // namespace OHOS
#endif