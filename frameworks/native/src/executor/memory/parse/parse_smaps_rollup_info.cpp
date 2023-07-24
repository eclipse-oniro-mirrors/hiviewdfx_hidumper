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
#include "executor/memory/parse/parse_smaps_rollup_info.h"
#include <fstream>
#include "executor/memory/memory_util.h"
#include "hilog_wrapper.h"
#include "util/string_utils.h"

using namespace std;

namespace OHOS {
namespace HiviewDFX {
ParseSmapsRollupInfo::ParseSmapsRollupInfo()
{
}
ParseSmapsRollupInfo::~ParseSmapsRollupInfo()
{
}


void ParseSmapsRollupInfo::GetValue(const string &str, MemInfoData::MemInfo &memInfo)
{
    if (StringUtils::GetInstance().IsBegin(str, "R")) {
        string type;
        uint64_t value = 0;
        bool success = MemoryUtil::GetInstance().GetTypeAndValue(str, type, value);
        if (!success) {
            return;
        }
        if (type == "Rss") {
            memInfo.rss = value;
        }
    } else if (StringUtils::GetInstance().IsBegin(str, "P")) {
        string type;
        uint64_t value = 0;
        bool success = MemoryUtil::GetInstance().GetTypeAndValue(str, type, value);
        if (!success) {
            return;
        }
        if (type == "Pss") {
            memInfo.pss = value;
        } else if (type == "Private_Clean") {
            memInfo.privateClean = value;
        } else if (type == "Private_Dirty") {
            memInfo.privateDirty = value;
        }
    } else if (StringUtils::GetInstance().IsBegin(str, "S")) {
        string type;
        uint64_t value = 0;
        bool success = MemoryUtil::GetInstance().GetTypeAndValue(str, type, value);
        if (!success) {
            return;
        }
        if (type == "Shared_Clean") {
            memInfo.sharedClean = value;
        } else if (type == "Shared_Dirty") {
            memInfo.sharedDirty = value;
        } else if (type == "Swap") {
            memInfo.swap = value;
        } else if (type == "SwapPss") {
            memInfo.swapPss = value;
        }
    }
}

bool ParseSmapsRollupInfo::GetMemInfo(const int &pid, MemInfoData::MemInfo &memInfo)
{
    string filename = "/proc/" + to_string(pid) + "/smaps_rollup";
    auto fp = std::unique_ptr<FILE, decltype(&fclose)>{fopen(filename.c_str(), "re"), fclose};
    if (fp == nullptr) {
        return false;
    }
    MemoryUtil::GetInstance().InitMemInfo(memInfo);
    char *line = nullptr;
    ssize_t lineLen;
    size_t lineAlloc = 0;
    while ((lineLen = getline(&line, &lineAlloc, fp.get())) > 0) {
        line[lineLen] = '\0';
        string content = line;
        GetValue(content, memInfo);
    }
    if (line != nullptr) {
        free(line);
        line = nullptr;
    }
    return true;
}
} // namespace HiviewDFX
} // namespace OHOS
