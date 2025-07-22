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
#include "executor/memory/smaps_memory_info.h"

#include <cinttypes>
#include <fstream>
#include <numeric>
#include <thread>
#include <v1_0/imemory_tracker_interface.h>
#include <cstring>

#include "dump_common_utils.h"
#include "dump_utils.h"
#include "executor/memory/get_cma_info.h"
#include "executor/memory/get_hardware_info.h"
#include "executor/memory/get_kernel_info.h"
#include "executor/memory/get_process_info.h"
#include "executor/memory/get_ram_info.h"
#include "executor/memory/memory_util.h"
#include "executor/memory/parse/meminfo_data.h"
#include "executor/memory/parse/parse_meminfo.h"
#include "executor/memory/parse/parse_smaps_rollup_info.h"
#include "executor/memory/parse/parse_smaps_info.h"
#include "hdf_base.h"
#include "hilog_wrapper.h"
#include "securec.h"
#include "string_ex.h"
#include "util/string_utils.h"

using namespace std;
using namespace OHOS::HDI::Memorytracker::V1_0;
namespace OHOS {
namespace HiviewDFX {
static constexpr int LINE_WIDTH = 12;
static constexpr int LINE_NAME_VAL_WIDTH = 60;
static constexpr int LINE_START_VAL_WIDTH = 18;
static constexpr int LINE_NAME_V_WIDTH = 16;
static constexpr int LINE_MEMORY_CLASS_WIDTH = 15;
static constexpr size_t TYPE_SIZE = 2;
static constexpr size_t TYPE_MIN_SIZE = 1;
static constexpr char BLANK = ' ';
SmapsMemoryInfo::SmapsMemoryInfo()
{
}

SmapsMemoryInfo::~SmapsMemoryInfo()
{
}

void SmapsMemoryInfo::InsertSmapsTitle(StringMatrix result, bool isShowSmapsInfo)
{
    vector<string> line1;
    vector<string> line2;
    vector<string> titleVec = isShowSmapsInfo ? MemoryFilter::GetInstance().TITLE_V_SMAPS_HAS_PID_ :
        MemoryFilter::GetInstance().TITLE_SMAPS_HAS_PID_;
    if (DumpUtils::IsUserMode()) {
        titleVec.erase(std::remove(titleVec.begin(), titleVec.end(), "Perm"), titleVec.end());
        titleVec.erase(std::remove(titleVec.begin(), titleVec.end(), "Start"), titleVec.end());
        titleVec.erase(std::remove(titleVec.begin(), titleVec.end(), "End"), titleVec.end());
    }
    for (string str : titleVec) {
        vector<string> types;
        StringUtils::GetInstance().StringSplit(str, "_", types);
        if (types.size() == TYPE_SIZE) {
            string title1 = types.at(0);
            StringUtils::GetInstance().SetWidth(LINE_WIDTH, BLANK, true, title1);
            line1.push_back(title1);
            string title2 = types.at(1);
            StringUtils::GetInstance().SetWidth(LINE_WIDTH, BLANK, true, title2);
            line2.push_back(title2);
        } else if (types.size() == TYPE_MIN_SIZE) {
            string title = types.at(0);
            string space = " ";
            StringUtils::GetInstance().SetWidth(LINE_WIDTH, BLANK, true, space);
            line1.push_back(space);
            constexpr int LINE_NAME_KEY_WIDTH = 22;
            if (StringUtils::GetInstance().IsSameStr(title, "Name")) {
                StringUtils::GetInstance().SetWidth(isShowSmapsInfo ? LINE_NAME_V_WIDTH : LINE_NAME_KEY_WIDTH,
                    BLANK, false, title);
            } else if (StringUtils::GetInstance().IsSameStr(title, "Category")) {
                StringUtils::GetInstance().SetWidth(LINE_MEMORY_CLASS_WIDTH, BLANK, true, title);
            } else {
                StringUtils::GetInstance().SetWidth(StringUtils::GetInstance().IsSameStr(title, "Start") ?
                    LINE_START_VAL_WIDTH : LINE_WIDTH, BLANK, true, title);
            }
            line2.push_back(title);
        }
    }
    result->push_back(line1);
    result->push_back(line2);
}

void SmapsMemoryInfo::SetValueForRet(const std::string& value, const int &width, std::vector<std::string>& tempResult)
{
    std::string tempStr = value;
    StringUtils::GetInstance().SetWidth(width, BLANK, true, tempStr);
    tempResult.push_back(tempStr);
}

void SmapsMemoryInfo::SetOneRowMemInfo(const MemoryData &memInfo, bool isShowSmapsInfo,
    bool isSummary, StringMatrix result)
{
    vector<string> tempResult;
    SetValueForRet(to_string(memInfo.size), LINE_WIDTH, tempResult);
    SetValueForRet(to_string(memInfo.rss), LINE_WIDTH, tempResult);
    SetValueForRet(to_string(memInfo.pss), LINE_WIDTH, tempResult);
    SetValueForRet(to_string(memInfo.sharedClean), LINE_WIDTH, tempResult);
    SetValueForRet(to_string(memInfo.sharedDirty), LINE_WIDTH, tempResult);
    SetValueForRet(to_string(memInfo.privateClean), LINE_WIDTH, tempResult);
    SetValueForRet(to_string(memInfo.privateDirty), LINE_WIDTH, tempResult);
    SetValueForRet(to_string(memInfo.swap), LINE_WIDTH, tempResult);
    SetValueForRet(to_string(memInfo.swapPss), LINE_WIDTH, tempResult);
    if (isShowSmapsInfo) {
        if (!DumpUtils::IsUserMode()) {
            SetValueForRet(memInfo.permission, LINE_WIDTH, tempResult);
            SetValueForRet(memInfo.startAddr, LINE_START_VAL_WIDTH, tempResult);
            SetValueForRet(memInfo.endAddr, LINE_WIDTH, tempResult);
        }
    } else {
        SetValueForRet(to_string(memInfo.counts), LINE_WIDTH, tempResult);
    }
    // set memory class
    string memoryClass = memInfo.memoryClass;
    if (memoryClass == "other") {
        if (memInfo.iNode == 0) {
            memoryClass = "AnonPage other";
        } else {
            memoryClass = "FilePage other";
        }
    }
    SetValueForRet(memoryClass, LINE_MEMORY_CLASS_WIDTH, tempResult);
    // set name
    string space = " ";
    StringUtils::GetInstance().SetWidth(isShowSmapsInfo ? LINE_WIDTH : LINE_START_VAL_WIDTH, BLANK, false, space);
    string value = isSummary ? "Summary" : memInfo.name;
    value = space + value;
    StringUtils::GetInstance().SetWidth(LINE_NAME_VAL_WIDTH, BLANK, true, value);
    tempResult.push_back(value);

    // set one row data
    result->push_back(tempResult);
}

void SmapsMemoryInfo::UpdateShowAddressMemInfoResult(const std::vector<MemoryData>& showAddressMemInfoVec,
    StringMatrix result)
{
    MemoryData summary;
    for (const auto &memInfo : showAddressMemInfoVec) {
        summary.size += memInfo.size;
        summary.rss += memInfo.rss;
        summary.pss += memInfo.pss;
        summary.sharedClean += memInfo.sharedClean;
        summary.sharedDirty += memInfo.sharedDirty;
        summary.privateClean += memInfo.privateClean;
        summary.privateDirty += memInfo.privateDirty;
        summary.swap += memInfo.swap;
        summary.swapPss += memInfo.swapPss;
        SetOneRowMemInfo(memInfo, true, false, result);
    }
    summary.pss += summary.swapPss;
    SetOneRowMemInfo(summary, true, true, result);
}

void SmapsMemoryInfo::UpdateCountSameNameMemResult(std::map<std::string, MemoryData>& countSameNameMemMap,
    StringMatrix result)
{
    MemoryData summary;
    for (const auto &memInfo : countSameNameMemMap) {
        summary.size += memInfo.second.size;
        summary.rss += memInfo.second.rss;
        summary.pss += memInfo.second.pss;
        summary.sharedClean += memInfo.second.sharedClean;
        summary.sharedDirty += memInfo.second.sharedDirty;
        summary.privateClean += memInfo.second.privateClean;
        summary.privateDirty += memInfo.second.privateDirty;
        summary.swap += memInfo.second.swap;
        summary.swapPss += memInfo.second.swapPss;
        summary.counts += memInfo.second.counts;
        SetOneRowMemInfo(memInfo.second, false, false, result);
    }
    summary.pss += summary.swapPss;
    SetOneRowMemInfo(summary, false, true, result);
}

bool SmapsMemoryInfo::ShowMemorySmapsByPid(const int &pid, StringMatrix result, bool isShowSmapsAddress)
{
    DUMPER_HILOGI(MODULE_SERVICE, "get smaps data begin, pid:%{public}d", pid);
    std::map<std::string, MemoryData> countSameNameMemMap;
    std::vector<MemoryData> showAddressMemInfoVec;
    unique_ptr<ParseSmapsInfo> parseSmapsInfo = make_unique<ParseSmapsInfo>();
    bool ret = parseSmapsInfo->GetSmapsData(pid, isShowSmapsAddress, countSameNameMemMap, showAddressMemInfoVec);
    if (!ret) {
        return false;
    }
    InsertSmapsTitle(result, isShowSmapsAddress);
    if (isShowSmapsAddress) {
        UpdateShowAddressMemInfoResult(showAddressMemInfoVec, result);
    } else {
        UpdateCountSameNameMemResult(countSameNameMemMap, result);
    }
    DUMPER_HILOGI(MODULE_SERVICE, "get smaps data end, pid:%{public}d", pid);
    return true;
}
} // namespace HiviewDFX
} // namespace OHOS