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

#include "executor/memory/parse/parse_smaps_info.h"
#include <fstream>
#include "executor/memory/memory_util.h"
#include "hilog_wrapper.h"
#include "util/string_utils.h"
#include "util/file_utils.h"

using namespace std;
namespace OHOS {
namespace HiviewDFX {
ParseSmapsInfo::ParseSmapsInfo()
{
}

ParseSmapsInfo::~ParseSmapsInfo()
{
}

bool ParseSmapsInfo::GetHasPidValue(const string &str, string &type, uint64_t &value)
{
    bool success = false;
    if (StringUtils::GetInstance().IsBegin(str, "R")) {
        success = MemoryUtil::GetInstance().GetTypeAndValue(str, type, value);
        if (success) {
            if (type == "Rss") {
                return true;
            }
        }
        return false;
    } else if (StringUtils::GetInstance().IsBegin(str, "P")) {
        success = MemoryUtil::GetInstance().GetTypeAndValue(str, type, value);
        if (success) {
            if (type == "Pss" || type == "Private_Clean" || type == "Private_Dirty") {
                return true;
            }
        }
        return false;
    } else if (StringUtils::GetInstance().IsBegin(str, "S")) {
        success = MemoryUtil::GetInstance().GetTypeAndValue(str, type, value);
        if (success) {
            if (type == "Shared_Clean" || type == "Shared_Dirty" || type == "Swap" || type == "SwapPss" ||
                type == "Size") {
                return true;
            }
        }
        return false;
    }
    return false;
}

bool ParseSmapsInfo::GetNoPidValue(const string &str, string &type, uint64_t &value)
{
    if (StringUtils::GetInstance().IsBegin(str, "Pss") || StringUtils::GetInstance().IsBegin(str, "SwapPss")) {
        return MemoryUtil::GetInstance().GetTypeAndValue(str, type, value);
    }
    return false;
}

bool ParseSmapsInfo::GetValue(const MemoryFilter::MemoryType &memType, const string &str, string &type, uint64_t &value)
{
    if (memType == MemoryFilter::MemoryType::APPOINT_PID) {
        return GetHasPidValue(str, type, value);
    } else {
        return GetNoPidValue(str, type, value);
    }
}

/**
 * @description: Parse smaps file
 * @param {MemoryType} &memType-APPOINT_PID-Specify the PID,NOT_SPECIFIED_PID-No PID is specified
 * @param {int} &pid-Pid
 * @param {GroupMap} &result-The result of parsing
 * @return bool-true:parse success,false-parse fail
 */
bool ParseSmapsInfo::GetInfo(const MemoryFilter::MemoryType &memType, const int &pid,
                             GroupMap &nativeMap, GroupMap &result)
{
    DUMPER_HILOGD(MODULE_SERVICE, "ParseSmapsInfo: GetInfo pid:(%{public}d) begin.\n", pid);
    string path = "/proc/" + to_string(pid) + "/smaps";
    bool ret = FileUtils::GetInstance().LoadStringFromProcCb(path, false, true, [&](const string& line) -> void {
        string name;
        uint64_t iNode = 0;
        if (StringUtils::GetInstance().IsEnd(line, "B")) {
            string type;
            uint64_t value = 0;
            if (GetValue(memType, line, type, value)) {
                MemoryUtil::GetInstance().CalcGroup(memGroup_, type, value, result);
                MemoryUtil::GetInstance().CalcGroup(nativeMemGroup_, type, value, nativeMap);
            }
        } else if (MemoryUtil::GetInstance().IsNameLine(line, name, iNode)) {
            MemoryFilter::GetInstance().ParseMemoryGroup(name, memGroup_, iNode);
            MemoryFilter::GetInstance().ParseNativeHeapMemoryGroup(name, nativeMemGroup_, iNode);
        }
    });
    DUMPER_HILOGD(MODULE_SERVICE, "ParseSmapsInfo: GetInfo pid:(%{public}d) end,success!\n", pid);
    return ret;
}

void ParseSmapsInfo::SetMemoryData(MemoryData &memoryData, const MemoryItem &memoryItem,
    const std::string& memoryClassStr)
{
    memoryData.name = memoryItem.name == "" ? "[anon]" : memoryItem.name;
    memoryData.memoryClass = memoryClassStr;
    memoryData.counts = 1;
    memoryData.size = memoryItem.size;
    memoryData.rss = memoryItem.rss;
    memoryData.pss = memoryItem.pss;
    memoryData.swapPss = memoryItem.swapPss;
    memoryData.swap = memoryItem.swap;
    memoryData.sharedDirty = memoryItem.sharedDirty;
    memoryData.privateDirty = memoryItem.privateDirty;
    memoryData.sharedClean = memoryItem.sharedClean;
    memoryData.privateClean = memoryItem.privateClean;
    memoryData.iNode = memoryItem.iNode;
}

void ParseSmapsInfo::UpdateCountSameNameMemMap(const std::vector<MemoryItem>& memoryItems,
    const std::string& memoryClassStr, std::map<std::string, MemoryData>& countSameNameMemMap)
{
    for (const auto& memoryItem : memoryItems) {
        if (countSameNameMemMap.find(memoryItem.name) == countSameNameMemMap.end()) {
            MemoryData memoryData;
            SetMemoryData(memoryData, memoryItem, memoryClassStr);
            countSameNameMemMap[memoryItem.name] = memoryData;
        } else {
            MemoryData& data = countSameNameMemMap[memoryItem.name];
            data.size += memoryItem.size;
            data.rss += memoryItem.rss;
            data.pss += memoryItem.pss;
            data.swapPss += memoryItem.swapPss;
            data.swap += memoryItem.swap;
            data.sharedDirty += memoryItem.sharedDirty;
            data.privateDirty += memoryItem.privateDirty;
            data.sharedClean += memoryItem.sharedClean;
            data.privateClean += memoryItem.privateClean;
            data.counts++;
        }
    }
}

void ParseSmapsInfo::UpdateShowAddressMemInfoVec(const std::vector<MemoryItem>& memoryItems,
    const std::string& memoryClassStr, std::vector<MemoryData>& showAddressMemInfoVec)
{
    for (const auto& memoryItem : memoryItems) {
        MemoryData memoryData;
        memoryData.permission = memoryItem.permission;
        memoryData.startAddr = memoryItem.startAddr;
        memoryData.endAddr = memoryItem.endAddr;
        SetMemoryData(memoryData, memoryItem, memoryClassStr);
        showAddressMemInfoVec.push_back(memoryData);
    }
}

bool ParseSmapsInfo::GetSmapsData(const int &pid, bool isShowSmapsAddress,
    std::map<std::string, MemoryData>& countSameNameMemMap, std::vector<MemoryData>& showAddressMemInfoVec)
{
    DUMPER_HILOGI(MODULE_SERVICE, "collect memory begin, pid:%{public}d", pid);
    std::shared_ptr<UCollectUtil::MemoryCollector> collector = UCollectUtil::MemoryCollector::Create();
    auto collectRet = collector->CollectProcessMemoryDetail(pid, GraphicMemOption::NONE);
    if (collectRet.retCode != UCollect::UcError::SUCCESS) {
        DUMPER_HILOGE(MODULE_SERVICE, "collect process memory error, ret:%{public}d", collectRet.retCode);
        return false;
    }
    DUMPER_HILOGI(MODULE_SERVICE, "collect memory end, pid:%{public}d", pid);
    vector<MemoryDetail> memoryDetails = collectRet.data.details;
    for (const auto& memoryDetail : memoryDetails) {
        MemoryClass memoryClass = memoryDetail.memoryClass;
        if (static_cast<int>(memoryClass) < 0 ||
            static_cast<size_t>(memoryClass) > MEMORY_CLASS_VEC.size()) {
            DUMPER_HILOGE(MODULE_SERVICE, "memoryClass:%{public}d is not exist", memoryClass);
            continue;
        }
        string memoryClassStr = MEMORY_CLASS_VEC[static_cast<int>(memoryClass)];
        if (memoryClassStr == "graph") {
            continue;
        }
        vector<MemoryItem> memoryItems = memoryDetail.items;
        if (isShowSmapsAddress) {
            UpdateShowAddressMemInfoVec(memoryItems, memoryClassStr, showAddressMemInfoVec);
        } else {
            UpdateCountSameNameMemMap(memoryItems, memoryClassStr, countSameNameMemMap);
        }
    }
    return true;
}
} // namespace HiviewDFX
} // namespace OHOS
