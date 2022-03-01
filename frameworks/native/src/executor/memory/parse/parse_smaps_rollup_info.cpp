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
#include "executor/memory/parse/parse_smaps_rollup_info.h"
#include <fstream>
#include "executor/memory/memory_util.h"
#include "hilog_wrapper.h"
#include "securec_p.h"

using namespace std;

namespace OHOS {
namespace HiviewDFX {
ParseSmapsRollupInfo::ParseSmapsRollupInfo()
{
}
ParseSmapsRollupInfo::~ParseSmapsRollupInfo()
{
}


void ParseSmapsRollupInfo::GetValue(const vector<string> strs, MemInfoData::MemInfo &memInfo)
{
    MemoryUtil::GetInstance().InitMemInfo(memInfo);
    for (auto str : strs) {
        string type = "";
        uint64_t value = 0;
        bool success = false;
        if (StringUtils::GetInstance().IsBegin(str, "R")) {
            success = MemoryUtil::GetInstance().GetTypeAndValue(str, type, value);
            if (success) {
                if (type == "Rss") {
                    memInfo.rss = value;
                    continue;
                }
            }
            continue;
        } else if (StringUtils::GetInstance().IsBegin(str, "P")) {
            success = MemoryUtil::GetInstance().GetTypeAndValue(str, type, value);
            if (success) {
                if (type == "Pss") {
                    memInfo.pss = value;
                    continue;
                } else if (type == "Private_Clean") {
                    memInfo.privateDirty = value;
                    continue;
                } else if (type == "Private_Dirty") {
                    memInfo.privateDirty = value;
                    continue;
                }
            }
            continue;
        } else if (StringUtils::GetInstance().IsBegin(str, "S")) {
            success = MemoryUtil::GetInstance().GetTypeAndValue(str, type, value);
            if (success) {
                if (type == "Shared_Clean") {
                    memInfo.sharedClean = value;
                    continue;
                } else if (type == "Shared_Dirty") {
                    memInfo.sharedDirty = value;
                    continue;
                } else if (type == "Swap") {
                    memInfo.swap = value;
                    continue;
                } else if (type == "SwapPss") {
                    memInfo.swapPss = value;
                    continue;
                }
            }
            continue;
        }
    }
}

bool ParseSmapsRollupInfo::GetMemInfo(const int &pid, MemInfoData::MemInfo &memInfo)
{
    string lineContent;
    string filename = "/proc/" + to_string(pid) + "/smaps_rollup";
    bool success = false;
    ifstream in(filename);
    vector<string> strs;
    if (in) {
        while (getline(in, lineContent)) {
            strs.push_back(lineContent);
        }
        GetValue(strs, memInfo);
        success = true;
        in.close();
    } else {
        success = false;
        DUMPER_HILOGE(MODULE_SERVICE, "File %s not found.\n", filename.c_str());
    }
    return success;
}
} // namespace HiviewDFX
} // namespace OHOS
