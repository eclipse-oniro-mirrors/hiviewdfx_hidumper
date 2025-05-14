/*
* Copyright (C) 2021-2022 Huawei Device Co., Ltd.
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#include "executor/memory/memory_info.h"

#include <dlfcn.h>
#include <cinttypes>
#include <fstream>
#include <numeric>
#include <thread>
#include <v1_0/imemory_tracker_interface.h>

#include "dump_common_utils.h"
#include "dump_utils.h"
#include "executor/memory/get_cma_info.h"
#include "executor/memory/get_hardware_info.h"
#include "executor/memory/get_kernel_info.h"
#include "executor/memory/get_process_info.h"
#include "executor/memory/get_ram_info.h"
#include "executor/memory/memory_util.h"
#include "executor/memory/parse/parse_ashmem_info.h"
#include "executor/memory/parse/parse_meminfo.h"
#include "executor/memory/parse/parse_smaps_rollup_info.h"
#include "executor/memory/parse/parse_smaps_info.h"
#include "file_ex.h"
#include "hdf_base.h"
#include "hilog_wrapper.h"
#ifdef HIDUMPER_MEMMGR_ENABLE
#include "mem_mgr_constant.h"
#endif
#include "securec.h"
#include "string_ex.h"
#include "util/string_utils.h"
#include "util/file_utils.h"

using namespace std;
using namespace OHOS::HDI::Memorytracker::V1_0;

namespace OHOS {
namespace HiviewDFX {
static const int PAGETAG_MIN_LEN = 2; // page tag min length
static const std::string LIB = "libai_mnt_client.so";

static const std::string UNKNOWN_PROCESS = "unknown";
static const std::string PRE_BLANK = "   ";
static const std::string MEMORY_LINE = "-------------------------------[memory]-------------------------------";
constexpr char HIAI_MEM_INFO_FN[] = "HIAI_Memory_QueryAllUserAllocatedMemInfo";
using HiaiFunc = int (*)(MemInfoData::HiaiUserAllocatedMemInfo*, int, int*);

MemoryInfo::MemoryInfo()
{
    methodVec_.clear();
    methodVec_.push_back(make_pair(MEMINFO_PSS,
        bind(&MemoryInfo::SetPss, this, placeholders::_1, placeholders::_2)));
    methodVec_.push_back(make_pair(MEMINFO_SHARED_CLEAN,
        bind(&MemoryInfo::SetSharedClean, this, placeholders::_1, placeholders::_2)));
    methodVec_.push_back(make_pair(MEMINFO_SHARED_DIRTY,
        bind(&MemoryInfo::SetSharedDirty, this, placeholders::_1, placeholders::_2)));
    methodVec_.push_back(make_pair(MEMINFO_PRIVATE_CLEAN,
        bind(&MemoryInfo::SetPrivateClean, this, placeholders::_1, placeholders::_2)));
    methodVec_.push_back(make_pair(MEMINFO_PRIVATE_DIRTY,
        bind(&MemoryInfo::SetPrivateDirty, this, placeholders::_1, placeholders::_2)));
    methodVec_.push_back(make_pair(MEMINFO_SWAP,
        bind(&MemoryInfo::SetSwap, this, placeholders::_1, placeholders::_2)));
    methodVec_.push_back(make_pair(MEMINFO_SWAP_PSS,
        bind(&MemoryInfo::SetSwapPss, this, placeholders::_1, placeholders::_2)));
    methodVec_.push_back(make_pair(MEMINFO_HEAP_SIZE,
        bind(&MemoryInfo::SetHeapSize, this, placeholders::_1, placeholders::_2)));
    methodVec_.push_back(make_pair(MEMINFO_HEAP_ALLOC,
        bind(&MemoryInfo::SetHeapAlloc, this, placeholders::_1, placeholders::_2)));
    methodVec_.push_back(make_pair(MEMINFO_HEAP_FREE,
        bind(&MemoryInfo::SetHeapFree, this, placeholders::_1, placeholders::_2)));
}

MemoryInfo::~MemoryInfo()
{
}

void MemoryInfo::InsertMemoryTitle(StringMatrix result)
{
    // Pss        Shared   ---- this line is line1
    // Total      Clean    ---- this line is line2
    // (KB)         (KB)    ---- this line is line3
    // -----      ------   ---- this line is line4

    vector<string> line1;
    vector<string> line2;
    vector<string> line3;
    vector<string> line4;

    string space = " ";
    StringUtils::GetInstance().SetWidth(LINE_WIDTH_, BLANK_, false, space);

    string separator = "-";
    StringUtils::GetInstance().SetWidth(LINE_WIDTH_, SEPARATOR_, false, separator);

    string unit = "(" + MemoryUtil::GetInstance().KB_UNIT_ + " )";
    StringUtils::GetInstance().SetWidth(LINE_WIDTH_, BLANK_, false, unit);

    // Add  spaces at the beginning of the line
    line1.push_back(space + BLANK_);
    line2.push_back(space + BLANK_);
    line3.push_back(space + BLANK_);
    line4.push_back(space);

    for (string str : MemoryFilter::GetInstance().TITLE_HAS_PID_) {
        vector<string> types;
        StringUtils::GetInstance().StringSplit(str, "_", types);
        if (types.size() == TYPE_SIZE) {
            string title1 = types.at(0);
            StringUtils::GetInstance().SetWidth(LINE_WIDTH_, BLANK_, false, title1);
            line1.push_back(title1 + BLANK_);

            string title2 = types.at(1);
            StringUtils::GetInstance().SetWidth(LINE_WIDTH_, BLANK_, false, title2);
            line2.push_back(title2 + BLANK_);
            line3.push_back(unit + BLANK_);
            line4.push_back(separator + SEPARATOR_);
        }
    }
    result->push_back(line1);
    result->push_back(line2);
    result->push_back(line3);
    result->push_back(line4);
}

void MemoryInfo::UpdateResult(const int32_t& pid, const unique_ptr<ProcessMemoryDetail>& processMemoryDetail,
    unique_ptr<MemoryDetail>& nativeHeapDetail, StringMatrix result)
{
    unique_ptr<MallHeapInfo> mallocHeapInfo = make_unique<MallHeapInfo>();
    unique_ptr<GetHeapInfo> getHeapInfo = make_unique<GetHeapInfo>();
    getHeapInfo->GetMallocHeapInfo(pid, mallocHeapInfo);
    unique_ptr<MemoryDetail> anonPageDetail = make_unique<MemoryDetail>();
    unique_ptr<MemoryDetail> filePageDetail = make_unique<MemoryDetail>();
    vector<MemoryDetail> memoryDetails = processMemoryDetail->details;
    map<string, unique_ptr<MemoryDetail>> allDetailMap;
    for (const auto& memoryDetail : memoryDetails) {
        MemoryClass memoryClass = memoryDetail.memoryClass;
        if (static_cast<int>(memoryClass) < 0 || static_cast<size_t>(memoryClass) > MEMORY_CLASS_VEC.size()) {
            DUMPER_HILOGE(MODULE_SERVICE, "memoryClass:%{public}d is not exist", memoryClass);
            continue;
        }
        string memoryClassStr = MEMORY_CLASS_VEC[static_cast<int>(memoryClass)];
        unique_ptr<MemoryDetail> tempDetail = make_unique<MemoryDetail>(memoryDetail);
        if (memoryClassStr == "graph") {
            GetGraphicsMemoryByDetail(tempDetail, result);
        } else if (memoryClassStr == MemoryFilter::GetInstance().NATIVE_HEAP_LABEL) {
            nativeHeapDetail = make_unique<MemoryDetail>(memoryDetail);
        } else if (memoryClassStr == MEMINFO_OTHER) {
            UpdatePageDetail(anonPageDetail, filePageDetail, tempDetail);
            tempDetail = std::move(anonPageDetail);
            memoryClassStr = MEMINFO_ANONPAGE_OTHER;
        }
        auto it = std::find(MEMORY_PRINT_ORDER_VEC.begin(), MEMORY_PRINT_ORDER_VEC.end(), memoryClassStr);
        if (it!= MEMORY_PRINT_ORDER_VEC.end()) {
            allDetailMap[memoryClassStr] = std::move(tempDetail);
        }
    }
    for (const auto &classStr : MEMORY_PRINT_ORDER_VEC) {
        auto it = allDetailMap.find(classStr);
        if (it != allDetailMap.end()) {
            SetDetailRet(classStr, it->second, mallocHeapInfo, result);
        }
    }
    SetDetailRet(MEMINFO_FILEPAGE_OTHER, filePageDetail, mallocHeapInfo, result);
    UpdateTotalDetail(processMemoryDetail, mallocHeapInfo, result);
}

void MemoryInfo::UpdatePageDetail(std::unique_ptr<MemoryDetail>& anonPageDetail,
                                  std::unique_ptr<MemoryDetail>& filePageDetail,
                                  const std::unique_ptr<MemoryDetail>& tempDetail)
{
    if (!tempDetail) {
        DUMPER_HILOGE(MODULE_SERVICE, "detail is nullptr");
        return;
    }
    vector<MemoryItem> memoryItems = tempDetail->items;
    for (const auto& memoryItem : memoryItems) {
        int32_t iNode = memoryItem.iNode;
        if (iNode == 0) {
            anonPageDetail->totalPss += memoryItem.pss;
            anonPageDetail->totalSharedClean += memoryItem.sharedClean;
            anonPageDetail->totalSharedDirty += memoryItem.sharedDirty;
            anonPageDetail->totalPrivateClean += memoryItem.privateClean;
            anonPageDetail->totalPrivateDirty += memoryItem.privateDirty;
            anonPageDetail->totalSwap += memoryItem.swap;
            anonPageDetail->totalSwapPss += memoryItem.swapPss;
        } else {
            filePageDetail->totalPss += memoryItem.pss;
            filePageDetail->totalSharedClean += memoryItem.sharedClean;
            filePageDetail->totalSharedDirty += memoryItem.sharedDirty;
            filePageDetail->totalPrivateClean += memoryItem.privateClean;
            filePageDetail->totalPrivateDirty += memoryItem.privateDirty;
            filePageDetail->totalSwap += memoryItem.swap;
            filePageDetail->totalSwapPss += memoryItem.swapPss;
        }
    }
}

void MemoryInfo::SetDetailRet(const std::string& memoryClassStr, const std::unique_ptr<MemoryDetail>& detail,
                              const std::unique_ptr<MallHeapInfo>& heapInfo, StringMatrix result)
{
    if (!detail) {
        DUMPER_HILOGE(MODULE_SERVICE, "detail is nullptr");
        return;
    }
    vector<string> tempResult;
    SetValueForRet(memoryClassStr, tempResult);
    SetValueForRet(to_string(detail->totalPss), tempResult);
    SetValueForRet(to_string(detail->totalSharedClean), tempResult);
    SetValueForRet(to_string(detail->totalSharedDirty), tempResult);
    SetValueForRet(to_string(detail->totalPrivateClean), tempResult);
    SetValueForRet(to_string(detail->totalPrivateDirty), tempResult);
    SetValueForRet(to_string(detail->totalSwap), tempResult);
    SetValueForRet(to_string(detail->totalSwapPss), tempResult);
    if (memoryClassStr == MemoryFilter::GetInstance().NATIVE_HEAP_LABEL) {
        SetValueForRet(to_string(heapInfo->size), tempResult);
        SetValueForRet(to_string(heapInfo->alloc), tempResult);
        SetValueForRet(to_string(heapInfo->free), tempResult);
    } else {
        for (int i = 0; i < MALLOC_HEAP_TYPES; i++) {
            SetValueForRet(ZERO, tempResult);
        }
    }
    result->push_back(tempResult);
}

void MemoryInfo::SetValueForRet(const std::string& value, std::vector<std::string>& tempResult)
{
    std::string tempStr = value;
    StringUtils::GetInstance().SetWidth(LINE_WIDTH_, BLANK_, false, tempStr);
    tempResult.push_back(tempStr + BLANK_);
}

void MemoryInfo::UpdateTotalDetail(const std::unique_ptr<ProcessMemoryDetail>& detail,
                                   const std::unique_ptr<MallHeapInfo>& heapInfo, StringMatrix result)
{
    if (!detail) {
        DUMPER_HILOGE(MODULE_SERVICE, "detail is nullptr");
        return;
    }
    vector<string> lines;
    vector<string> values;
    uint64_t totalPrivateDirty = static_cast<uint64_t>(detail->totalPrivateDirty) + graphicsMemory_.gl +
        graphicsMemory_.graph;
    MemoryUtil::GetInstance().SetMemTotalValue(MEMINFO_TOTAL, lines, values);
    MemoryUtil::GetInstance().SetMemTotalValue(to_string(detail->totalAllPss), lines, values);
    MemoryUtil::GetInstance().SetMemTotalValue(to_string(detail->totalSharedClean), lines, values);
    MemoryUtil::GetInstance().SetMemTotalValue(to_string(detail->totalSharedDirty), lines, values);
    MemoryUtil::GetInstance().SetMemTotalValue(to_string(detail->totalPrivateClean), lines, values);
    MemoryUtil::GetInstance().SetMemTotalValue(to_string(totalPrivateDirty), lines, values);
    MemoryUtil::GetInstance().SetMemTotalValue(to_string(detail->totalSwap), lines, values);
    MemoryUtil::GetInstance().SetMemTotalValue(to_string(detail->totalSwapPss), lines, values);
    MemoryUtil::GetInstance().SetMemTotalValue(to_string(heapInfo->size), lines, values);
    MemoryUtil::GetInstance().SetMemTotalValue(to_string(heapInfo->alloc), lines, values);
    MemoryUtil::GetInstance().SetMemTotalValue(to_string(heapInfo->free), lines, values);

    // delete the last separator
    if (!lines.empty()) {
        lines.pop_back();
        string separator = "-";
        StringUtils::GetInstance().SetWidth(LINE_WIDTH_, SEPARATOR_, false, separator);
        lines.push_back(separator);
    }
    result->push_back(lines);
    result->push_back(values);
}

bool MemoryInfo::GetMemoryInfoByPid(const int32_t &pid, StringMatrix result)
{
    std::lock_guard<std::mutex> lock(mutex_);
    InsertMemoryTitle(result);
    GetResult(pid, result);
    GetPurgByPid(pid, result);
    GetDma(graphicsMemory_.graph, result);
    GetHiaiServerIon(pid, result);
    GetAshmem(pid, result);
    return true;
}

void MemoryInfo::GetResult(const int32_t& pid, StringMatrix result)
{
    DUMPER_HILOGD(MODULE_SERVICE, "CollectProcessMemoryDetail pid:%{public}d start", pid);
    std::shared_ptr<UCollectUtil::MemoryCollector> collector = UCollectUtil::MemoryCollector::Create();
    CollectResult<ProcessMemoryDetail> collectRet;
    collectRet = collector->CollectProcessMemoryDetail(pid, GraphicMemOption::LOW_LATENCY);
    if (collectRet.retCode != UCollect::UcError::SUCCESS) {
        DUMPER_HILOGE(MODULE_SERVICE, "collect process memory error, ret:%{public}d", collectRet.retCode);
        return;
    }
    unique_ptr<ProcessMemoryDetail> processMemoryDetail = make_unique<ProcessMemoryDetail>(collectRet.data);
    if (!processMemoryDetail) {
        DUMPER_HILOGE(MODULE_SERVICE, "processMemoryDetail is nullptr");
        return;
    }
    DUMPER_HILOGD(MODULE_SERVICE, "CollectProcessMemoryDetail pid:%{public}d end", pid);
    unique_ptr<MemoryDetail> nativeHeapDetail = {nullptr};
    UpdateResult(pid, processMemoryDetail, nativeHeapDetail, result);
    GetNativeHeap(nativeHeapDetail, result);
}

void MemoryInfo::GetGraphicsMemoryByDetail(const std::unique_ptr<MemoryDetail>& detail, StringMatrix result)
{
    if (!detail) {
        DUMPER_HILOGE(MODULE_SERVICE, "detail is nullptr");
        return;
    }
    vector<MemoryItem> memoryItems = detail->items;
    for (const auto& memoryItem : memoryItems) {
        MemoryItemType memoryItemType = memoryItem.type;
        auto info = GRAPHIC_MAP.find(memoryItemType);
        if (info == GRAPHIC_MAP.end()) {
            DUMPER_HILOGE(MODULE_SERVICE, "memoryItemType = %{public}d is not exist", memoryItemType);
            continue;
        }
        string type = info->second;
        if (type == MemoryFilter::GetInstance().GL_OUT_LABEL) {
            graphicsMemory_.gl = memoryItem.pss;
        } else if (type == MemoryFilter::GetInstance().GRAPH_OUT_LABEL) {
            graphicsMemory_.graph = memoryItem.pss;
        }
    }
    UpdateGraphicsMemoryRet(MemoryFilter::GetInstance().GL_OUT_LABEL, graphicsMemory_.gl, result);
    UpdateGraphicsMemoryRet(MemoryFilter::GetInstance().GRAPH_OUT_LABEL, graphicsMemory_.graph, result);
}

void MemoryInfo::UpdateGraphicsMemoryRet(const string& title, const uint64_t& value, StringMatrix result)
{
    map<string, uint64_t> valueMap;
    valueMap.insert(pair<string, uint64_t>(MEMINFO_PSS, value));
    valueMap.insert(pair<string, uint64_t>(MEMINFO_PRIVATE_DIRTY, value));
    vector<string> tempResult;
    string tempTitle = title;
    StringUtils::GetInstance().SetWidth(LINE_WIDTH_, BLANK_, false, tempTitle);
    tempResult.push_back(tempTitle + BLANK_);
    for (const auto &tag : MemoryFilter::GetInstance().VALUE_WITH_PID) {
        auto it = valueMap.find(tag);
        string value = ZERO;
        if (it != valueMap.end()) {
            value = to_string(it->second);
        }
        StringUtils::GetInstance().SetWidth(LINE_WIDTH_, BLANK_, false, value);
        tempResult.push_back(value + BLANK_);
    }
    result->push_back(tempResult);
}

string MemoryInfo::AddKbUnit(const uint64_t &value) const
{
    return to_string(value) + MemoryUtil::GetInstance().KB_UNIT_;
}

bool MemoryInfo::GetSmapsInfoNoPid(const int32_t &pid, GroupMap &result)
{
    GroupMap NativeMap;
    unique_ptr<ParseSmapsInfo> parseSmapsInfo = make_unique<ParseSmapsInfo>();
    return parseSmapsInfo->GetInfo(MemoryFilter::NOT_SPECIFIED_PID, pid, NativeMap, result);
}

bool MemoryInfo::GetMeminfo(ValueMap &result)
{
    unique_ptr<ParseMeminfo> parseMeminfo = make_unique<ParseMeminfo>();
    return parseMeminfo->GetMeminfo(result);
}

bool MemoryInfo::GetHardWareUsage(StringMatrix result)
{
    uint64_t value;
    unique_ptr<GetHardwareInfo> getHardwareInfo = make_unique<GetHardwareInfo>();
    if (getHardwareInfo->GetHardwareUsage(value)) {
        string title = "Hardware Usage:";
        StringUtils::GetInstance().SetWidth(RAM_WIDTH_, BLANK_, false, title);
        SaveStringToFd(rawParamFd_, title + AddKbUnit(value) + "\n");
        return true;
    }
    return false;
}

bool MemoryInfo::GetCMAUsage(StringMatrix result)
{
    uint64_t value = 0;
    unique_ptr<GetCMAInfo> getCMAInfo = make_unique<GetCMAInfo>();
    bool success = getCMAInfo->GetUsed(value);
    if (success) {
        string title = "CMA Usage:";
        StringUtils::GetInstance().SetWidth(RAM_WIDTH_, BLANK_, false, title);
        SaveStringToFd(rawParamFd_, title + AddKbUnit(value) + "\n");
    }
    return success;
}

bool MemoryInfo::GetKernelUsage(const ValueMap &infos, StringMatrix result)
{
    uint64_t value = 0;
    unique_ptr<GetKernelInfo> getGetKernelInfo = make_unique<GetKernelInfo>();
    bool success = getGetKernelInfo->GetKernel(infos, value);
    if (success) {
        string title = "Kernel Usage:";
        StringUtils::GetInstance().SetWidth(RAM_WIDTH_, BLANK_, false, title);
        SaveStringToFd(rawParamFd_, title + AddKbUnit(value) + "\n");
    }
    return success;
}

void MemoryInfo::GetProcesses(const GroupMap &infos, StringMatrix result)
{
    uint64_t value = 0;

    unique_ptr<GetProcessInfo> getProcessInfo = make_unique<GetProcessInfo>();
    value = getProcessInfo->GetProcess(infos);

    string title = "Processes Usage:";
    StringUtils::GetInstance().SetWidth(RAM_WIDTH_, BLANK_, false, title);
    SaveStringToFd(rawParamFd_, title + AddKbUnit(value) + "\n");
}

void MemoryInfo::GetPssTotal(const GroupMap &infos, StringMatrix result)
{
    SaveStringToFd(rawParamFd_, "Total Pss by Category:\n");
    vector<pair<string, uint64_t>> filePage;
    vector<pair<string, uint64_t>> anonPage;
    for (const auto &info : infos) {
        vector<string> pageTag;
        StringUtils::GetInstance().StringSplit(info.first, "#", pageTag);
        if (pageTag.size() < PAGETAG_MIN_LEN) {
            DUMPER_HILOGE(MODULE_COMMON, "Infos are invalid, info.first: %{public}s", info.first.c_str());
            continue;
        }
        string group = pageTag[1];
        auto &valueMap = info.second;
        uint64_t pssValue = 0;
        for (const auto &str : MemoryFilter::GetInstance().CALC_PSS_TOTAL_) {
            auto it = valueMap.find(str);
            if (it != valueMap.end()) {
                pssValue += it->second;
            }
        }

        if (pageTag[0] == MemoryFilter::GetInstance().FILE_PAGE_TAG) {
            filePage.push_back(make_pair(group, pssValue));
        } else {
            anonPage.push_back(make_pair(group, pssValue));
        }
    }
    PairToStringMatrix(MemoryFilter::GetInstance().FILE_PAGE_TAG, filePage, result);
    PairToStringMatrix(MemoryFilter::GetInstance().ANON_PAGE_TAG, anonPage, result);

    vector<pair<string, uint64_t>> gpuValue;
    gpuValue.push_back(make_pair(MemoryFilter::GetInstance().GL_OUT_LABEL, totalGL_));
    PairToStringMatrix(MemoryFilter::GetInstance().GPU_TAG, gpuValue, result);

    vector<pair<string, uint64_t>> graphValue;
    graphValue.push_back(make_pair(MemoryFilter::GetInstance().GRAPH_OUT_LABEL, totalGraph_));
    PairToStringMatrix(MemoryFilter::GetInstance().GRAPH_OUT_LABEL, graphValue, result);

    vector<pair<string, uint64_t>> dmaValue;
    dmaValue.push_back(make_pair(MemoryFilter::GetInstance().DMA_OUT_LABEL, totalDma_));
    PairToStringMatrix(MemoryFilter::GetInstance().DMA_TAG, dmaValue, result);
}

void MemoryInfo::PairToStringMatrix(const string &titleStr, vector<pair<string, uint64_t>> &vec, StringMatrix result)
{
    uint64_t totalPss = accumulate(vec.begin(), vec.end(), (uint64_t)0, [] (uint64_t a, pair<string, uint64_t> &b) {
        return a + b.second;
    });
    SaveStringToFd(rawParamFd_, titleStr + "(" + AddKbUnit(totalPss) + "):\n");

    std::sort(vec.begin(), vec.end(),
        [] (pair<string, uint64_t> &left, pair<string, uint64_t> &right) {
        return right.second < left.second;
    });
    for (const auto &pair : vec) {
        string pssStr = AddKbUnit(pair.second);
        StringUtils::GetInstance().SetWidth(RAM_WIDTH_, BLANK_, false, pssStr);
        SaveStringToFd(rawParamFd_, pssStr + " : " + pair.first + "\n");
    }
}

void MemoryInfo::GetRamUsage(const GroupMap &smapsinfos, const ValueMap &meminfo, StringMatrix result)
{
    unique_ptr<GetRamInfo> getRamInfo = make_unique<GetRamInfo>();
    GetRamInfo::Ram ram = getRamInfo->GetRam(smapsinfos, meminfo);

    string totalTitle = "Total RAM:";
    StringUtils::GetInstance().SetWidth(RAM_WIDTH_, BLANK_, false, totalTitle);
    SaveStringToFd(rawParamFd_, totalTitle + AddKbUnit(ram.total) + "\n");

    string freeTitle = "Free RAM:";
    StringUtils::GetInstance().SetWidth(RAM_WIDTH_, BLANK_, false, freeTitle);
    SaveStringToFd(rawParamFd_, freeTitle + AddKbUnit(ram.free) +
        " (" + to_string(ram.cachedInfo) + " cached + " + to_string(ram.freeInfo) + " free)\n");

    string usedTitle = "Used RAM:";
    StringUtils::GetInstance().SetWidth(RAM_WIDTH_, BLANK_, false, usedTitle);
    SaveStringToFd(rawParamFd_, usedTitle + AddKbUnit(ram.used) +
        " (" + to_string(ram.totalPss) + " total pss + " + to_string(ram.kernelUsed) + " kernel)\n");

    string lostTitle = "Lost RAM:";
    StringUtils::GetInstance().SetWidth(RAM_WIDTH_, BLANK_, false, lostTitle);
    SaveStringToFd(rawParamFd_, lostTitle + to_string(ram.lost) + MemoryUtil::GetInstance().KB_UNIT_ + "\n");
}

void MemoryInfo::GetPurgTotal(const ValueMap &meminfo, StringMatrix result)
{
    SaveStringToFd(rawParamFd_, "Total Purgeable:\n");

    uint64_t purgSumTotal = 0;
    uint64_t purgPinTotal = 0;
    auto purgSumActive = meminfo.find(MemoryFilter::GetInstance().PURG_SUM[0]);
    auto purgSumInactive = meminfo.find(MemoryFilter::GetInstance().PURG_SUM[1]);
    auto purgPinPined = meminfo.find(MemoryFilter::GetInstance().PURG_PIN[0]);
    if (purgSumActive == meminfo.end() || purgSumInactive == meminfo.end() || purgPinPined == meminfo.end()) {
        DUMPER_HILOGE(MODULE_SERVICE, "fail to get purg info \n");
    } else {
        purgSumTotal = purgSumActive->second + purgSumInactive->second;
        purgPinTotal = purgPinPined->second;
    }

    string totalPurgSumTitle = "Total PurgSum:";
    StringUtils::GetInstance().SetWidth(RAM_WIDTH_, BLANK_, false, totalPurgSumTitle);
    SaveStringToFd(rawParamFd_, totalPurgSumTitle + AddKbUnit(purgSumTotal) + "\n");

    string totalPurgPinTitle = "Total PurgPin:";
    StringUtils::GetInstance().SetWidth(RAM_WIDTH_, BLANK_, false, totalPurgPinTitle);
    SaveStringToFd(rawParamFd_, totalPurgPinTitle + AddKbUnit(purgPinTotal) + "\n");
}

void MemoryInfo::GetPurgByPid(const int32_t &pid, StringMatrix result)
{
    AddBlankLine(result);
    vector<string> title;
    title.push_back("Purgeable:");
    result->push_back(title);

    vector<string> purgSum;
    string purgSumTitle = MemoryFilter::GetInstance().PURGSUM_OUT_LABEL + ":";
    StringUtils::GetInstance().SetWidth(RAM_WIDTH_, BLANK_, false, purgSumTitle);
    purgSum.push_back(purgSumTitle);
    purgSum.push_back(AddKbUnit(GetProcValue(pid, MemoryFilter::GetInstance().PURGSUM_OUT_LABEL)));
    result->push_back(purgSum);

    vector<string> purgPin;
    string purgPinTitle = MemoryFilter::GetInstance().PURGPIN_OUT_LABEL + ":";
    StringUtils::GetInstance().SetWidth(RAM_WIDTH_, BLANK_, false, purgPinTitle);
    purgPin.push_back(purgPinTitle);
    purgPin.push_back(AddKbUnit(GetProcValue(pid, MemoryFilter::GetInstance().PURGPIN_OUT_LABEL)));
    result->push_back(purgPin);
}

void MemoryInfo::GetNativeHeap(const std::unique_ptr<MemoryDetail>& detail, StringMatrix result)
{
    if (!detail) {
        DUMPER_HILOGE(MODULE_SERVICE, "detail is nullptr");
        return;
    }
    AddBlankLine(result);
    vector<string> title;
    title.push_back(MemoryFilter::GetInstance().NATIVE_HEAP_LABEL + ":");
    result->push_back(title);

    vector<MemoryItem> memoryItems = detail->items;
    for (const auto& memoryItem : memoryItems) {
        MemoryItemType memoryItemType = memoryItem.type;
        auto info = NATIVE_HEAP_MAP.find(memoryItemType);
        if (info == NATIVE_HEAP_MAP.end()) {
            DUMPER_HILOGE(MODULE_SERVICE, "memoryItemType = %{public}d is not exist", memoryItemType);
            continue;
        }
        string type = info->second;
        auto it = memoryItemMap_.find(type);
        if (it == memoryItemMap_.end()) {
            memoryItemMap_.insert(pair<string, MemoryItem>(type, memoryItem));
        } else {
            it->second.pss += memoryItem.pss;
            it->second.sharedClean += memoryItem.sharedClean;
            it->second.sharedDirty += memoryItem.sharedDirty;
            it->second.privateClean += memoryItem.privateClean;
            it->second.privateDirty += memoryItem.privateDirty;
            it->second.swap += memoryItem.swap;
            it->second.swapPss += memoryItem.swapPss;
        }
    }
    for (const auto& type: NATIVE_HEAP_TAG) {
        auto it = memoryItemMap_.find(type);
        if (it == memoryItemMap_.end()) {
            continue;
        }
        string heapTitle = it->first;
        unique_ptr<MemoryItem> item = make_unique<MemoryItem>(it->second);
        SetNativeDetailRet(type, item, result);
    }
}

void MemoryInfo::SetNativeDetailRet(const std::string& nativeClassStr, const std::unique_ptr<MemoryItem>& item,
                                    StringMatrix result)
{
    vector<string> heap;
    string heapTitle = nativeClassStr + ":";
    StringUtils::GetInstance().SetWidth(RAM_WIDTH_, BLANK_, false, heapTitle);
    heap.push_back(heapTitle);
    SetValueForRet(to_string(item->pss), heap);
    SetValueForRet(to_string(item->sharedClean), heap);
    SetValueForRet(to_string(item->sharedDirty), heap);
    SetValueForRet(to_string(item->privateClean), heap);
    SetValueForRet(to_string(item->privateDirty), heap);
    SetValueForRet(to_string(item->swap), heap);
    SetValueForRet(to_string(item->swapPss), heap);
    for (int i = 0; i < MALLOC_HEAP_TYPES; i++) {
        SetValueForRet(ZERO, heap);
    }
    result->push_back(heap);
}

void MemoryInfo::GetDma(const uint64_t& dmaValue, StringMatrix result)
{
    AddBlankLine(result);
    vector<string> title;
    title.push_back(MemoryFilter::GetInstance().DMA_TAG + ":");
    result->push_back(title);

    vector<string> dma;
    string dmaTitle = MemoryFilter::GetInstance().DMA_OUT_LABEL + ":";
    StringUtils::GetInstance().SetWidth(RAM_WIDTH_, BLANK_, false, dmaTitle);
    dma.push_back(dmaTitle);
    dma.push_back(to_string(dmaValue) + MemoryUtil::GetInstance().KB_UNIT_);
    result->push_back(dma);
}

void MemoryInfo::GetHiaiServerIon(const int32_t &pid, StringMatrix result)
{
    if (GetProcName(pid) != "hiaiserver") {
        return;
    }
    void *handle = dlopen(LIB.c_str(), RTLD_LAZY);
    if (handle == nullptr) {
        DUMPER_HILOGE(MODULE_SERVICE, "fail to open %{public}s.", LIB.c_str());
        return;
    }
    HiaiFunc pfn = reinterpret_cast<HiaiFunc>(dlsym(handle, HIAI_MEM_INFO_FN));
    if (pfn == nullptr) {
        DUMPER_HILOGE(MODULE_SERVICE, "fail to dlsym %{public}s.", HIAI_MEM_INFO_FN);
        dlclose(handle);
        return;
    }
    MemInfoData::HiaiUserAllocatedMemInfo memInfos[HIAI_MAX_QUERIED_USER_MEMINFO_LIMIT] = {};

    int realSize = 0;
    pfn(memInfos, HIAI_MAX_QUERIED_USER_MEMINFO_LIMIT, &realSize);
    if (realSize > 0) {
        AddBlankLine(result);
        vector<string> vecIon;
        vecIon.push_back("HIAIServer ION:\n");
        for (int i = 0; i < realSize; i++) {
            vecIon.push_back(GetProcName(memInfos[i].pid) + "(" + to_string(memInfos[i].pid) + "):" +
                             to_string(memInfos[i].size / BYTE_PER_KB) + " kB\n");
        }
        result->push_back(vecIon);
    }
    dlclose(handle);
}

void MemoryInfo::GetAshmem(const int32_t &pid, StringMatrix result)
{
    std::pair<int, std::vector<std::string>> ashmemInfo;
    unique_ptr<ParseAshmemInfo> parseAshmeminfo = make_unique<ParseAshmemInfo>();
    if (!parseAshmeminfo->GetAshmemInfo(pid, ashmemInfo)) {
        DUMPER_HILOGE(MODULE_SERVICE, "GetAshmemInfo error");
        return;
    }
    if (ashmemInfo.second.size() == 0) {
        DUMPER_HILOGE(MODULE_SERVICE, "no detail ashmem info.");
        return;
    }
    AddBlankLine(result);
    vector<string> title;
    title.push_back("Ashmem:");
    result->push_back(title);
    vector<string> totalAshmemVec;
    totalAshmemVec.push_back("Total Ashmem:" + to_string(ashmemInfo.first) + MemoryUtil::GetInstance().KB_UNIT_);
    result->push_back(totalAshmemVec);
    for (auto detailInfo : ashmemInfo.second) {
        vector<string> tempResult;
        tempResult.push_back(detailInfo);
        result->push_back(tempResult);
    }
}

void MemoryInfo::GetRamCategory(const GroupMap &smapsInfos, const ValueMap &meminfos, StringMatrix result)
{
    SaveStringToFd(rawParamFd_, "Total RAM by Category:\n");

    bool hardWareSuccess = GetHardWareUsage(result);
    if (!hardWareSuccess) {
        DUMPER_HILOGE(MODULE_SERVICE, "Get hardWare usage fail.\n");
    }

    bool cmaSuccess = GetCMAUsage(result);
    if (!cmaSuccess) {
        DUMPER_HILOGE(MODULE_SERVICE, "Get CMA fail.\n");
    }

    bool kernelSuccess = GetKernelUsage(meminfos, result);
    if (!kernelSuccess) {
        DUMPER_HILOGE(MODULE_SERVICE, "Get kernel usage fail.\n");
    }

    GetProcesses(smapsInfos, result);
}

void MemoryInfo::AddBlankLine(StringMatrix result)
{
    vector<string> blank;
    blank.push_back("\n");
    result->push_back(blank);
}

string MemoryInfo::GetProcName(const int32_t &pid)
{
    string procName = UNKNOWN_PROCESS;
    DumpCommonUtils::GetProcessNameByPid(pid, procName);
    if (procName == UNKNOWN_PROCESS) {
        string path = "/proc/" + to_string(pid) + "/status";
        procName = FileUtils::GetInstance().GetProcValue(pid, path, "Name");
    }
    return procName;
}

uint64_t MemoryInfo::GetProcValue(const int32_t &pid, const string& key)
{
    string path = "/proc/" + to_string(pid) + "/status";
    std::string value = FileUtils::GetInstance().GetProcValue(pid, path, key);
    if (value == UNKNOWN_PROCESS) {
        DUMPER_HILOGE(MODULE_SERVICE, "GetProcStatusValue failed");
        return 0;
    }
    int number = 0;
    value = value.substr(0, value.size() - 3); // 3: ' kB'
    if (value.find_last_of(' ') != std::string::npos) {
        value = value.substr(value.find_last_of(' ') + 1);
    }
    if (!StrToInt(value, number)) {
        DUMPER_HILOGE(MODULE_COMMON, "StrToInt failed, value: %{public}s", value.c_str());
        return 0;
    }
    return static_cast<uint64_t>(number);
}

#ifdef HIDUMPER_MEMMGR_ENABLE
string MemoryInfo::GetProcessAdjLabel(const int32_t pid)
{
    string adjLabel = Memory::RECLAIM_PRIORITY_UNKNOWN_DESC;
    string fillPath = "/proc/" + to_string(pid) + "/oom_score_adj";
    if (!DumpUtils::PathIsValid(fillPath)) {
        DUMPER_HILOGE(MODULE_COMMON, "GetProcessAdjLabel leave|false, PathIsValid");
        return adjLabel;
    }
    auto fp = fopen(fillPath.c_str(), "rb");
    if (fp == nullptr) {
        DUMPER_HILOGE(MODULE_COMMON, "Open oom_score_adj failed.");
        return adjLabel;
    }
    constexpr int bufSize = 128; // 128: buf size
    char buf[bufSize] = {0};
    size_t readSum = fread(buf, 1, bufSize, fp);
    (void)fclose(fp);
    fp = nullptr;
    if (readSum < 1) {
        DUMPER_HILOGE(MODULE_COMMON, "Read oom_score_adj failed.");
        return adjLabel;
    }
    int value = Memory::RECLAIM_PRIORITY_UNKNOWN;
    std::string label(buf);
    if (label.empty()) {
        DUMPER_HILOGE(MODULE_COMMON, "label is empty.");
        return adjLabel;
    }
    if (!StrToInt(label.substr(0, label.size() - 1), value)) {
        DUMPER_HILOGE(MODULE_COMMON, "StrToInt failed.");
        return adjLabel;
    }
    adjLabel = Memory::GetReclaimPriorityString(value);
    return adjLabel;
}
#endif

int MemoryInfo::GetScoreAdj(const int32_t pid)
{
    string filePath = "/proc/" + to_string(pid) + "/oom_score_adj";
    if(!DumpUtils::PathIsValid(filePath)) {
        DUMPER_HILOGE(MODULE_COMMON, "GetScoreAdj leave|false, PathIsValid");
        return -1;
    }
    std::string content = "-1";
    OHOS::LoadStringFromFile(filePath, content);
    int value = Memory::RECLAIM_PRIORITY_UNKNOWN;
    if (!StrToInt(content.substr(0, content.size() - 1), value)) {
        DUMPER_HILOGE(MODULE_COMMON, "GetScoreAdj|StrToInt failed.");
        return -1;
    }
    return value;
}

bool MemoryInfo::GetPids()
{
    bool success = DumpCommonUtils::GetUserPids(pids_);
    if (!success) {
        DUMPER_HILOGE(MODULE_SERVICE, "GetPids error");
    }
    return success;
}

uint64_t MemoryInfo::GetVss(const int32_t &pid)
{
    string path = "/proc/" + to_string(pid) + "/statm";
    uint64_t res = 0;
    bool ret = FileUtils::GetInstance().LoadStringFromProcCb(path, true, true, [&](const string& line) -> void {
        if (!line.empty()) {
            uint64_t tempValue = 0;
            int retScanf = sscanf_s(line.c_str(), "%llu^*", &tempValue);
            if (retScanf != -1) {
                res = tempValue * VSS_BIT;
            } else {
                DUMPER_HILOGE(MODULE_SERVICE, "GetVss error! pid = %{public}d", pid);
            }
        }
    });
    if (!ret) {
        return 0;
    }
    return res;
}

bool MemoryInfo::GetGraphicsMemory(int32_t pid, MemInfoData::GraphicsMemory &graphicsMemory, GraphicType graphicType)
{
    DUMPER_HILOGD(MODULE_SERVICE, "GetGraphicUsage start, pid:%{public}d", pid);
    std::shared_ptr<UCollectUtil::GraphicMemoryCollector> collector = UCollectUtil::GraphicMemoryCollector::Create();
    CollectResult<int32_t> data;
    data = collector->GetGraphicUsage(pid, graphicType, true);
    if (data.retCode != UCollect::UcError::SUCCESS) {
        DUMPER_HILOGE(MODULE_SERVICE, "collect progress GL or Graph error, ret:%{public}d", data.retCode);
        return false;
    }
    DUMPER_HILOGD(MODULE_SERVICE, "GetGraphicUsage end, pid:%{public}d", pid);
    if (graphicType == GraphicType::GL) {
        graphicsMemory.gl = static_cast<uint64_t>(data.data);
    } else if (graphicType == GraphicType::GRAPH) {
        graphicsMemory.graph = static_cast<uint64_t>(data.data);
    } else {
        DUMPER_HILOGE(MODULE_SERVICE, "graphic type is not support.");
        return false;
    }
    return true;
}

bool MemoryInfo::GetMemByProcessPid(const int32_t &pid, MemInfoData::MemUsage &usage)
{
    bool success = false;
    MemInfoData::MemInfo memInfo;
    unique_ptr<ParseSmapsRollupInfo> getSmapsRollup = make_unique<ParseSmapsRollupInfo>();
    if (getSmapsRollup->GetMemInfo(pid, memInfo)) {
        if (!dumpPrune_) {
            usage.vss = GetVss(pid);
            usage.uss = memInfo.privateClean + memInfo.privateDirty;
            usage.rss = memInfo.rss;
        }
        usage.pss = memInfo.pss;
        usage.swapPss = memInfo.swapPss;
        usage.name = GetProcName(pid);
        usage.pid = pid;
#ifdef HIDUMPER_MEMMGR_ENABLE
        usage.adjLabel = GetProcessAdjLabel(pid);
#endif
        success = true;
    }

    MemInfoData::GraphicsMemory graphicsMemory;
    MemoryUtil::GetInstance().InitGraphicsMemory(graphicsMemory);
    if (GetGraphicsMemory(pid, graphicsMemory, GraphicType::GL)) {
        usage.gl = graphicsMemory.gl;
        if (!dumpPrune_) {
            usage.uss = usage.uss + graphicsMemory.gl;
            usage.rss = usage.rss + graphicsMemory.gl;
        }
        usage.pss = usage.pss + graphicsMemory.gl;
    }
    if (!dumpPrune_) {
        if (GetGraphicsMemory(pid, graphicsMemory, GraphicType::GRAPH)) {
            usage.graph = graphicsMemory.graph;
            usage.dma = graphicsMemory.graph;
            usage.uss = usage.uss + graphicsMemory.graph;
            usage.pss = usage.pss + graphicsMemory.graph;
            usage.rss = usage.rss + graphicsMemory.graph;
            DUMPER_HILOGD(MODULE_SERVICE, "uss:%{public}d pss:%{public}d rss:%{public}d gl:%{public}d graph:%{public}d",
                        static_cast<int>(usage.uss), static_cast<int>(usage.pss), static_cast<int>(usage.rss),
                        static_cast<int>(graphicsMemory.gl), static_cast<int>(graphicsMemory.graph));
        }
    }
    return success;
}

void MemoryInfo::MemUsageToMatrix(const MemInfoData::MemUsage &memUsage, StringMatrix result)
{
    string pid = to_string(memUsage.pid);
    StringUtils::GetInstance().SetWidth(PID_WIDTH_, BLANK_, true, pid);

    uint64_t pss = memUsage.pss + memUsage.swapPss;
    string totalPss = to_string(pss) + "(" + to_string(memUsage.swapPss) + " in SwapPss) kB";
    StringUtils::GetInstance().SetWidth(PSS_WIDTH_, BLANK_, false, totalPss);

    uint64_t gl = memUsage.gl;
    string unMappedGL = AddKbUnit(gl);
    StringUtils::GetInstance().SetWidth(KB_WIDTH_, BLANK_, false, unMappedGL);

    string name = "    " + memUsage.name;
    StringUtils::GetInstance().SetWidth(NAME_WIDTH_, BLANK_, true, name);

    if (!dumpPrune_) {
        uint64_t vss = memUsage.vss;
        string totalVss = AddKbUnit(vss);
        StringUtils::GetInstance().SetWidth(KB_WIDTH_, BLANK_, false, totalVss);

        uint64_t rss = memUsage.rss;
        string totalRss = AddKbUnit(rss);
        StringUtils::GetInstance().SetWidth(KB_WIDTH_, BLANK_, false, totalRss);

        uint64_t uss = memUsage.uss;
        string totalUss = AddKbUnit(uss);
        StringUtils::GetInstance().SetWidth(KB_WIDTH_, BLANK_, false, totalUss);

        uint64_t graph = memUsage.graph;
        string unMappedGraph = AddKbUnit(graph);
        StringUtils::GetInstance().SetWidth(KB_WIDTH_, BLANK_, false, unMappedGraph);

        uint64_t dma = memUsage.dma;
        string unMappedDma = AddKbUnit(dma);
        StringUtils::GetInstance().SetWidth(KB_WIDTH_, BLANK_, false, unMappedDma);

        uint64_t purgSum = memUsage.purgSum;
        string unMappedPurgSum = AddKbUnit(purgSum);
        StringUtils::GetInstance().SetWidth(KB_WIDTH_, BLANK_, false, unMappedPurgSum);

        uint64_t purgPin = memUsage.purgPin;
        string unMappedPurgPin = AddKbUnit(purgPin);
        StringUtils::GetInstance().SetWidth(KB_WIDTH_, BLANK_, false, unMappedPurgPin);

        (void)dprintf(rawParamFd_, "%s %s %s %s %s %s %s %s %s %s %s\n", pid.c_str(),
            totalPss.c_str(), totalVss.c_str(), totalRss.c_str(), totalUss.c_str(),
            unMappedGL.c_str(), unMappedGraph.c_str(), unMappedDma.c_str(), unMappedPurgSum.c_str(),
            unMappedPurgPin.c_str(), name.c_str());
    } else {
        int scoreAdj = GetScoreAdj(memUsage.pid);
        string scoreAdjStr = to_string(scoreAdj);
        StringUtils::GetInstance().SetWidth(KB_WIDTH_, BLANK_, false, scoreAdjStr);

        (void)dprintf(rawParamFd_, "%s %s %s %s %s\n", pid.c_str(),
            totalPss.c_str(), unMappedGL.c_str(), scoreAdjStr.c_str(), name.c_str());
    }
}

void MemoryInfo::AddMemByProcessTitle(StringMatrix result, string sortType)
{
    string processTitle = "Total Memory Usage by " + sortType + ":";
    (void)dprintf(rawParamFd_, "%s\n", processTitle.c_str());

    string pid = "PID";
    StringUtils::GetInstance().SetWidth(PID_WIDTH_, BLANK_, true, pid);

    string totalPss = "Total Pss(xxx in SwapPss)";
    StringUtils::GetInstance().SetWidth(PSS_WIDTH_, BLANK_, false, totalPss);

    string unMappedGL = MemoryFilter::GetInstance().GL_OUT_LABEL;
    StringUtils::GetInstance().SetWidth(KB_WIDTH_, BLANK_, false, unMappedGL);

    string name = "    Name";
    StringUtils::GetInstance().SetWidth(NAME_WIDTH_, BLANK_, true, name);

    if (!dumpPrune_) {
        string totalVss = "Total Vss";
        StringUtils::GetInstance().SetWidth(KB_WIDTH_, BLANK_, false, totalVss);

        string totalRss = "Total Rss";
        StringUtils::GetInstance().SetWidth(KB_WIDTH_, BLANK_, false, totalRss);

        string totalUss = "Total Uss";
        StringUtils::GetInstance().SetWidth(KB_WIDTH_, BLANK_, false, totalUss);

        string unMappedGraph = MemoryFilter::GetInstance().GRAPH_OUT_LABEL;
        StringUtils::GetInstance().SetWidth(KB_WIDTH_, BLANK_, false, unMappedGraph);

        string unMappedDma = MemoryFilter::GetInstance().DMA_OUT_LABEL;
        StringUtils::GetInstance().SetWidth(KB_WIDTH_, BLANK_, false, unMappedDma);

        string unMappedPurgSum = MemoryFilter::GetInstance().PURGSUM_OUT_LABEL;
        StringUtils::GetInstance().SetWidth(KB_WIDTH_, BLANK_, false, unMappedPurgSum);

        string unMappedPurgPin = MemoryFilter::GetInstance().PURGPIN_OUT_LABEL;
        StringUtils::GetInstance().SetWidth(KB_WIDTH_, BLANK_, false, unMappedPurgPin);

        (void)dprintf(rawParamFd_, "%s %s %s %s %s %s %s %s %s %s %s\n", pid.c_str(),
            totalPss.c_str(), totalVss.c_str(), totalRss.c_str(), totalUss.c_str(),
            unMappedGL.c_str(), unMappedGraph.c_str(), unMappedDma.c_str(), unMappedPurgSum.c_str(),
            unMappedPurgPin.c_str(), name.c_str());
    } else {
        string adjLabel = "AdjLabel";
        StringUtils::GetInstance().SetWidth(KB_WIDTH_, BLANK_, false, adjLabel);

        (void)dprintf(rawParamFd_, "%s %s %s %s %s\n", pid.c_str(),
            totalPss.c_str(), unMappedGL.c_str(), adjLabel.c_str(), name.c_str());
    } 
}

bool MemoryInfo::GetMemoryInfoInit(StringMatrix result)
{
    if (!isReady_) {
        memUsages_.clear();
        pids_.clear();
        totalGL_ = 0;
        totalGraph_ = 0;
        AddMemByProcessTitle(result, "PID");
        if (!GetPids()) {
            DUMPER_HILOGE(MODULE_SERVICE, "GetPids error!\n");
            return false;
        }
        isReady_ = true;
    }
    return true;
}

void MemoryInfo::GetMemoryUsageInfo(StringMatrix result)
{
    MemInfoData::MemUsage usage;
    MemoryUtil::GetInstance().InitMemUsage(usage);
    for (auto pid : pids_) {
        if (GetMemByProcessPid(pid, usage)) {
            memUsages_.push_back(usage);
            adjMemResult_[usage.adjLabel].push_back(usage);
            totalGL_ += usage.gl;
            totalGraph_ += usage.graph;
            totalDma_ += usage.dma;
            MemUsageToMatrix(usage, result);
        } else {
            DUMPER_HILOGE(MODULE_SERVICE, "Get smaps_rollup error! pid = %{public}d\n", static_cast<int>(pid));
        }
    }
}

DumpStatus MemoryInfo::GetMemoryInfoNoPid(int fd, StringMatrix result)
{
    std::lock_guard<std::mutex> lock(mutex_);
    rawParamFd_ = fd;
    (void)dprintf(rawParamFd_, "%s\n", MEMORY_LINE.c_str());
	if (!GetMemoryInfoInit(result)) {
		return DUMP_FAIL;
	}

    if (!dumpSmapsOnStart_) {
        dumpSmapsOnStart_ = true;
        std::promise<GroupMap> promise;
        fut_ = promise.get_future();
        std::thread([promise = std::move(promise), this]() mutable {
            GroupMap groupMap;
            std::vector<int32_t> pids(this->pids_);
            for (auto pid : pids) {
                GetSmapsInfoNoPid(pid, groupMap);
            }
            promise.set_value(groupMap);
            }).detach();
    }
    GetMemoryUsageInfo(result);
    return DealResult(result);
}

DumpStatus MemoryInfo::GetMemoryInfoPrune(int fd, StringMatrix result)
{
    std::lock_guard<std::mutex> lock(mutex_);
    rawParamFd_ = fd;
    dumpPrune_ = true;
    (void)dprintf(rawParamFd_, "%s\n", MEMORY_LINE.c_str());
	if (!GetMemoryInfoInit(result)) {
		return DUMP_FAIL;
	}
    GetMemoryUsageInfo(result);
    return DUMP_OK;
}

DumpStatus MemoryInfo::DealResult(StringMatrix result)
{
    ValueMap meminfoResult;
    if (!GetMeminfo(meminfoResult)) {
        DUMPER_HILOGE(MODULE_SERVICE, "Get meminfo error\n");
        return DUMP_FAIL;
    }

    GetSortedMemoryInfoNoPid(result);
    SaveStringToFd(rawParamFd_, "\n");
#ifdef HIDUMPER_MEMMGR_ENABLE
    GetMemoryByAdj(result);
#endif
    SaveStringToFd(rawParamFd_, "\n");

    GroupMap smapsResult = fut_.get();

    GetPssTotal(smapsResult, result);
    SaveStringToFd(rawParamFd_, "\n");

    GetRamUsage(smapsResult, meminfoResult, result);
    SaveStringToFd(rawParamFd_, "\n");

    GetRamCategory(smapsResult, meminfoResult, result);
    SaveStringToFd(rawParamFd_, "\n");

    GetPurgTotal(meminfoResult, result);

    isReady_ = false;
    dumpSmapsOnStart_ = false;
    memUsages_.clear();
    smapsResult.clear();
    return DUMP_OK;
}

void MemoryInfo::GetSortedMemoryInfoNoPid(StringMatrix result)
{
    SaveStringToFd(rawParamFd_, "\n");
    AddMemByProcessTitle(result, "Size");

    std::sort(memUsages_.begin(), memUsages_.end(),
        [] (MemInfoData::MemUsage &left, MemInfoData::MemUsage &right) {
        if (right.pss + right.swapPss != left.pss + left.swapPss) {
            return right.pss + right.swapPss < left.pss + left.swapPss;
        }
        if (right.vss != left.vss) {
            return right.vss < left.vss;
        }
        if (right.rss != left.rss) {
            return right.rss < left.rss;
        }
        if (right.uss != left.uss) {
            return right.uss < left.uss;
        }
        return right.pid < left.pid;
    });

    for (auto &memUsage : memUsages_) {
        MemUsageToMatrix(memUsage, result);
    }
}

#ifdef HIDUMPER_MEMMGR_ENABLE
void MemoryInfo::GetMemoryByAdj(StringMatrix result)
{
    SaveStringToFd(rawParamFd_, "Total Pss by OOM adjustment:\n");

    vector<string> reclaimPriority_;
    for (auto reclaim : Memory::ReclaimPriorityMapping) {
        reclaimPriority_.push_back(reclaim.second);
    }
    reclaimPriority_.push_back(Memory::RECLAIM_PRIORITY_UNKNOWN_DESC);

    for (const auto &adjLabel : reclaimPriority_) {
        vector<MemInfoData::MemUsage> memUsages = adjMemResult_[adjLabel];
        vector<string> label;
        if (memUsages.size() == 0) {
            SaveStringToFd(rawParamFd_, adjLabel + ": " + AddKbUnit(0) + "\n");
            continue;
        }
        uint64_t totalPss = accumulate(memUsages.begin(), memUsages.end(), (uint64_t)0,
        [] (uint64_t a, MemInfoData::MemUsage &b) {
            return a + b.pss + b.swapPss;
        });
        SaveStringToFd(rawParamFd_, adjLabel + ": " + AddKbUnit(totalPss) + "\n");

        std::sort(memUsages.begin(), memUsages.end(),
            [] (MemInfoData::MemUsage &left, MemInfoData::MemUsage &right) {
            return right.pss + right.swapPss < left.pss + left.swapPss;
        });
        for (const auto &memUsage : memUsages) {
            string name = PRE_BLANK + memUsage.name + "(pid=" + to_string(memUsage.pid) + "): ";
            StringUtils::GetInstance().SetWidth(NAME_AND_PID_WIDTH, BLANK_, true, name);
            name += AddKbUnit(memUsage.pss + memUsage.swapPss);
            if (memUsage.swapPss > 0) {
                name += " (" + to_string(memUsage.swapPss) + " kB in SwapPss)";
            }
            SaveStringToFd(rawParamFd_, name + "\n");
        }
    }
}
#endif

void MemoryInfo::SetPss(MemInfoData::MemInfo &meminfo, uint64_t value)
{
    meminfo.pss += value;
}

void MemoryInfo::SetSharedClean(MemInfoData::MemInfo &meminfo, uint64_t value)
{
    meminfo.sharedClean += value;
}

void MemoryInfo::SetSharedDirty(MemInfoData::MemInfo &meminfo, uint64_t value)
{
    meminfo.sharedDirty += value;
}

void MemoryInfo::SetPrivateClean(MemInfoData::MemInfo &meminfo, uint64_t value)
{
    meminfo.privateClean += value;
}

void MemoryInfo::SetPrivateDirty(MemInfoData::MemInfo &meminfo, uint64_t value)
{
    meminfo.privateDirty += value;
}

void MemoryInfo::SetSwap(MemInfoData::MemInfo &meminfo, uint64_t value)
{
    meminfo.swap += value;
}

void MemoryInfo::SetSwapPss(MemInfoData::MemInfo &meminfo, uint64_t value)
{
    meminfo.swapPss += value;
}
void MemoryInfo::SetHeapSize(MemInfoData::MemInfo &meminfo, uint64_t value)
{
    meminfo.heapSize += value;
}

void MemoryInfo::SetHeapAlloc(MemInfoData::MemInfo &meminfo, uint64_t value)
{
    meminfo.heapAlloc += value;
}

void MemoryInfo::SetHeapFree(MemInfoData::MemInfo &meminfo, uint64_t value)
{
    meminfo.heapFree += value;
}
} // namespace HiviewDFX
} // namespace OHOS