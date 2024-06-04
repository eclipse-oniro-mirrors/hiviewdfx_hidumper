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

bool ParseSmapsInfo::GetSmapsValue(const MemoryFilter::MemoryType &memType, const string &str, string &type,
    uint64_t &value)
{
    if (memType == MemoryFilter::MemoryType::APPOINT_PID) {
        return GetHasPidValue(str, type, value);
    }
    return false;
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

void ParseSmapsInfo::SetMapByNameLine(const string &group, const string &content)
{
    memMap_.insert(pair<string, string>("Name", group));
    vector<string> datas;
    StringUtils::GetInstance().StringSplit(content, " ", datas);
    vector<string> startAndEnd;
    StringUtils::GetInstance().StringSplit(datas.at(0), "-", startAndEnd);
    string startVal = startAndEnd.front();
    string endVal = startAndEnd.back();
    memMap_.insert(pair<string, string>("Start", startVal));
    memMap_.insert(pair<string, string>("End", endVal));
    memMap_.insert(pair<string, string>("Perm", datas.at(1)));
}

bool ParseSmapsInfo::ShowSmapsData(const MemoryFilter::MemoryType &memType, const int &pid, GroupMap &result,
    bool isShowSmapsInfo, vector<map<string, string>> &vectMap)
{
    string path = "/proc/" + to_string(pid) + "/smaps";
    bool ret = FileUtils::GetInstance().LoadStringFromProcCb(path, false, true, [&](const string& line) -> void {
        string name;
        uint64_t iNode = 0;
        if (StringUtils::GetInstance().IsEnd(line, "B")) {
            string type;
            uint64_t value = 0;
            if (GetSmapsValue(memType, line, type, value)) {
                MemoryUtil::GetInstance().CalcGroup(memGroup_, type, value, result);
                memMap_.insert(pair<string, string>(type, to_string(value)));
            }
        } else if (MemoryUtil::GetInstance().IsNameLine(line, name, iNode)) {
            memGroup_ = name;
            if (!memMap_.empty()) {
                vectMap.push_back(memMap_);
                memMap_.clear();
            }
            if (result.find(memGroup_) != result.end()) {
                result[memGroup_]["Counts"]++;
            } else {
                vector<string> datas;
                StringUtils::GetInstance().StringSplit(line, " ", datas);
                result[memGroup_].insert(pair<string, uint64_t>("Perm",
                                                                MemoryUtil::GetInstance().PermToInt(datas.at(1))));
                result[memGroup_].insert(pair<string, uint64_t>("Counts", 1));
                result[memGroup_].insert(pair<string, uint64_t>("Name", 0));
            }
            if (isShowSmapsInfo) {
                SetMapByNameLine(memGroup_, line);
            }
        }
    });
    if (!memMap_.empty()) {
        vectMap.push_back(memMap_);
        memMap_.clear();
    }
    return ret;
}
} // namespace HiviewDFX
} // namespace OHOS
