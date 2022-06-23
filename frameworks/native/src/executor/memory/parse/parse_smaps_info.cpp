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
bool ParseSmapsInfo::GetInfo(const MemoryFilter::MemoryType &memType, const int &pid, GroupMap &result)
{
    DUMPER_HILOGD(MODULE_SERVICE, "ParseSmapsInfo: GetInfo pid:(%d) begin.\n", pid);
    string filename = "/proc/" + to_string(pid) + "/smaps";
    ifstream in(filename);
    if (!in) {
        DUMPER_HILOGE(MODULE_SERVICE, "File %s not found.\n", filename.c_str());
        return false;
    }

    string content;
    while (getline(in, content)) {
        string name;
        uint64_t iNode = 0;
        if (StringUtils::GetInstance().IsEnd(content, "B")) {
            string type;
            uint64_t value = 0;
            if (GetValue(memType, content, type, value)) {
                MemoryUtil::GetInstance().CalcGroup(memGroup_, type, value, result);
            }
        } else if (MemoryUtil::GetInstance().IsNameLine(content, name, iNode)) {
            MemoryFilter::GetInstance().ParseMemoryGroup(name, memGroup_, iNode);
        }
    }
    in.close();
    DUMPER_HILOGD(MODULE_SERVICE, "ParseSmapsInfo: GetInfo pid:(%d) end,success!\n", pid);
    return true;
}
} // namespace HiviewDFX
} // namespace OHOS
