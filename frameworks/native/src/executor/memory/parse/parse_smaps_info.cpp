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

#include "executor/memory/parse/parse_smaps_info.h"
#include <fstream>
#include "executor/memory/memory_util.h"
#include "hilog_wrapper.h"

using namespace std;
namespace OHOS {
namespace HiviewDFX {
ParseSmapsInfo::ParseSmapsInfo()
{
}

ParseSmapsInfo::~ParseSmapsInfo()
{
}

/**
 * @description: Data sorting
 * @param {PairMatrixGroup} &infos-Data to be sorted
 * @param {vector<std::string>} tags-Sort order
 * @return {PairMatrixGroup}-Sorted results
 */
ParseSmapsInfo::PairMatrixGroup ParseSmapsInfo::DataSort(const PairMatrixGroup &infos,
                                                         const std::vector<std::string> &tags)
{
    PairMatrixGroup result;

    for (auto info : infos) {
        string group = info.first;
        auto pairs = info.second;
        PairMatrix pairMatrix;
        for (auto pair : pairs) {
            string type = pair.first;
            bool findTag = false;
            for (auto tag : tags) {
                bool contain = StringUtils::GetInstance().IsContain(tag, "_Total");
                if (contain) {
                    StringUtils::GetInstance().ReplaceAll(tag, "_Total", "");
                }
                if (tag == type) {
                    findTag = true;
                    uint64_t value = pair.second;
                    pairMatrix.push_back(std::pair(type, value));
                    break;
                }
            }
        }
        result.push_back(std::pair(group, pairMatrix));
    }
    return result;
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
            if (type == "Shared_Clean" || type == "Shared_Dirty" || type == "Swap" || type == "SwapPss") {
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
    type = "";
    value = 0;
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
 * @param {PairMatrixGroup} &result-The result of parsing
 * @return bool-true:parse success,false-parse fail
 */
bool ParseSmapsInfo::GetInfo(const MemoryFilter::MemoryType &memType, const int &pid, PairMatrixGroup &result)
{
    DUMPER_HILOGD(MODULE_SERVICE, "ParseSmapsInfo: GetInfo pid:(%d) begin.\n", pid);
    result.clear();
    string lineContent;
    string filename = "/proc/" + to_string(pid) + "/smaps";
    ifstream in(filename);
    if (in) {
        PairMatrixGroup tempResult;

        vector<string> strs;
        while (getline(in, lineContent)) {
            strs.push_back(lineContent);
        }

        for (auto str : strs) {
            string content = str;
            string type = "";
            string group = "";
            string name = "";
            bool isNameLine = MemoryUtil::GetInstance().IsNameLine(content, name);
            if (isNameLine) {
                MemoryFilter::GetInstance().ParseMemoryGroup(content, name, group);
                if (!group.empty()) {
                    memGroup_ = group;
                } else {
                    memGroup_ = "other";
                }
            } else {
                uint64_t value = 0;
                bool getValueSuccess = GetValue(memType, content, type, value);
                if (getValueSuccess) {
                    MemoryUtil::GetInstance().CalcGroup(memGroup_, type, value, tempResult);
                }
            }
        }

        if (memType == MemoryFilter::GetInstance().MemoryType::APPOINT_PID) {
            result = DataSort(tempResult, MemoryFilter::GetInstance().TITLE_HAS_PID_);
        } else {
            result = DataSort(tempResult, MemoryFilter::GetInstance().TITLE_NO_PID_);
        }
        in.close();
        DUMPER_HILOGD(MODULE_SERVICE, "ParseSmapsInfo: GetInfo pid:(%d) end,success!\n", pid);
        return true;
    } else {
        DUMPER_HILOGE(MODULE_SERVICE, "File %s not found.\n", filename.c_str());
        return false;
    }
}
} // namespace HiviewDFX
} // namespace OHOS
