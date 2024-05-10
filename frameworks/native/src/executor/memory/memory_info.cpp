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
#include "executor/memory/get_heap_info.h"
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
#include "util/file_utils.h"
#ifdef HIDUMPER_GRAPHIC_ENABLE
#include "transaction/rs_interfaces.h"
using namespace OHOS::Rosen;
#endif

using namespace std;
using namespace OHOS::HDI::Memorytracker::V1_0;

namespace OHOS {
namespace HiviewDFX {
#if defined(__LP64__)
static const std::string LIB_PATH = "/vendor/lib64/libai_infra.so";
#else
static const std::string LIB_PATH = "/vendor/lib/libai_infra.so";
#endif

static string g_initProcessNSPid; //init process namespace pid
static const std::string UNKNOWN_PROCESS = "unknown";
static const std::string PRE_BLANK = "   ";
constexpr int HIAI_MAX_QUERIED_USER_MEMINFO_LIMIT = 256;
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

    if (g_initProcessNSPid.empty()) {
        GetNSPidByPid(1, g_initProcessNSPid);
    }
}

MemoryInfo::~MemoryInfo()
{
}

static uint64_t g_sumPidsMemGL = 0;
#ifdef HIDUMPER_GRAPHIC_ENABLE
std::vector<MemoryGraphic> memGraphicVec_;
#endif

void MemoryInfo::insertMemoryTitle(StringMatrix result)
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
    line1.push_back(space);
    line2.push_back(space);
    line3.push_back(space);
    line4.push_back(space);

    for (string str : MemoryFilter::GetInstance().TITLE_HAS_PID_) {
        vector<string> types;
        StringUtils::GetInstance().StringSplit(str, "_", types);
        if (types.size() == TYPE_SIZE) {
            string title1 = types.at(0);
            StringUtils::GetInstance().SetWidth(LINE_WIDTH_, BLANK_, false, title1);
            line1.push_back(title1);

            string title2 = types.at(1);
            StringUtils::GetInstance().SetWidth(LINE_WIDTH_, BLANK_, false, title2);
            line2.push_back(title2);
            line3.push_back(unit);
            line4.push_back(separator);
        }
    }
    result->push_back(line1);
    result->push_back(line2);
    result->push_back(line3);
    result->push_back(line4);
}

void MemoryInfo::BuildResult(const GroupMap &infos, StringMatrix result)
{
    insertMemoryTitle(result);
    for (const auto &info : infos) {
        vector<string> tempResult;
        vector<string> pageTag;
        StringUtils::GetInstance().StringSplit(info.first, "#", pageTag);
        string group;
        if (pageTag[1] == "other") {
            group = pageTag[0] == MemoryFilter::GetInstance().FILE_PAGE_TAG ? "FilePage other" : "AnonPage other";
        } else {
            group = pageTag[1];
        }
        StringUtils::GetInstance().SetWidth(LINE_WIDTH_, BLANK_, false, group);
        tempResult.push_back(group);

        auto &valueMap = info.second;
        for (const auto &tag : MemoryFilter::GetInstance().VALUE_WITH_PID) {
            auto it = valueMap.find(tag);
            string value = "0";
            if (it != valueMap.end()) {
                value = to_string(it->second);
            }
            StringUtils::GetInstance().SetWidth(LINE_WIDTH_, BLANK_, false, value);
            tempResult.push_back(value);
        }
        result->push_back(tempResult);
    }
}


void MemoryInfo::CalcGroup(const GroupMap &infos, StringMatrix result)
{
    MemInfoData::MemInfo meminfo;
    MemoryUtil::GetInstance().InitMemInfo(meminfo);
    for (const auto &info : infos) {
        auto &valueMap = info.second;
        for (const auto &method : methodVec_) {
            auto it = valueMap.find(method.first);
            if (it != valueMap.end()) {
                method.second(meminfo, it->second);
            }
        }
    }

    vector<string> lines;
    vector<string> values;

    MemoryUtil::GetInstance().SetMemTotalValue("Total", lines, values);
    MemoryUtil::GetInstance().SetMemTotalValue(to_string(meminfo.pss + meminfo.swapPss), lines, values);
    MemoryUtil::GetInstance().SetMemTotalValue(to_string(meminfo.sharedClean), lines, values);
    MemoryUtil::GetInstance().SetMemTotalValue(to_string(meminfo.sharedDirty), lines, values);
    MemoryUtil::GetInstance().SetMemTotalValue(to_string(meminfo.privateClean), lines, values);
    MemoryUtil::GetInstance().SetMemTotalValue(to_string(meminfo.privateDirty), lines, values);
    MemoryUtil::GetInstance().SetMemTotalValue(to_string(meminfo.swap), lines, values);
    MemoryUtil::GetInstance().SetMemTotalValue(to_string(meminfo.swapPss), lines, values);
    MemoryUtil::GetInstance().SetMemTotalValue(to_string(meminfo.heapSize), lines, values);
    MemoryUtil::GetInstance().SetMemTotalValue(to_string(meminfo.heapAlloc), lines, values);
    MemoryUtil::GetInstance().SetMemTotalValue(to_string(meminfo.heapFree), lines, values);

    result->push_back(lines);
    result->push_back(values);
}

bool MemoryInfo::GetRenderServiceGraphics(int32_t pid, MemInfoData::GraphicsMemory &graphicsMemory)
{
    bool ret = false;
    sptr<IMemoryTrackerInterface> memtrack = IMemoryTrackerInterface::Get(true);
    if (memtrack == nullptr) {
        DUMPER_HILOGE(MODULE_SERVICE, "memtrack service is null");
        return ret;
    }

    std::vector<MemoryRecord> records;
    if (memtrack->GetDevMem(pid, MEMORY_TRACKER_TYPE_GL, records) == HDF_SUCCESS) {
        uint64_t value = 0;
        for (const auto& record : records) {
            if ((static_cast<uint32_t>(record.flags) & FLAG_UNMAPPED) == FLAG_UNMAPPED) {
                value = static_cast<uint64_t>(record.size / BYTE_PER_KB);
                break;
            }
        }
        graphicsMemory.gl = value;
        ret = true;
    }
    return ret;
}

bool MemoryInfo::IsRenderService(int32_t pid)
{
    std::string rsName = "render_service";
    std::string processName = GetProcName(pid);
    const char whitespace[] = " \n\t\v\r\f";
    processName.erase(0, processName.find_first_not_of(whitespace));
    processName.erase(processName.find_last_not_of(whitespace) + 1U);
    if (processName == rsName) {
        return true;
    }
    return false;
}

bool MemoryInfo::IsOHService(const int32_t &pid)
{
    string namespacePid;
    GetNSPidByPid(pid, namespacePid);
    if (namespacePid.empty()) {
        DUMPER_HILOGE(MODULE_SERVICE, "get namespace pid error\n");
        return false;
    }
    return namespacePid == g_initProcessNSPid;
}

bool MemoryInfo::GetMemoryInfoByPid(const int32_t &pid, StringMatrix result)
{
    if (!dmaInfo_.ParseDmaInfo()) {
        DUMPER_HILOGE(MODULE_SERVICE, "Parse dma info error\n");
    }
    GroupMap groupMap;
    GroupMap nativeGroupMap;
    unique_ptr<ParseSmapsInfo> parseSmapsInfo = make_unique<ParseSmapsInfo>();
    if (!parseSmapsInfo->GetInfo(MemoryFilter::APPOINT_PID, pid, nativeGroupMap, groupMap)) {
        DUMPER_HILOGE(MODULE_SERVICE, "parse smaps info fail");
        return false;
    }

    unique_ptr<GetHeapInfo> getHeapInfo = make_unique<GetHeapInfo>();
    if (!getHeapInfo->GetInfo(MemoryFilter::APPOINT_PID, pid, groupMap)) {
        DUMPER_HILOGE(MODULE_SERVICE, "get heap info fail");
        return false;
    }

    MemInfoData::GraphicsMemory graphicsMemory;
    MemoryUtil::GetInstance().InitGraphicsMemory(graphicsMemory);
    graphicsMemory.graph = dmaInfo_.GetDmaByPid(pid);
    GetGraphicsMemory(pid, graphicsMemory);
    SetGraphGroupMap(groupMap, graphicsMemory);
    BuildResult(groupMap, result);
    CalcGroup(groupMap, result);
    GetNativeHeap(nativeGroupMap, result);
    GetPurgByPid(pid, result);
    GetDmaByPid(pid, result);
    GetHiaiServerIon(pid, result);
    return true;
}

void MemoryInfo::SetGraphGroupMap(GroupMap& groupMap, MemInfoData::GraphicsMemory &graphicsMemory)
{
    map<string, uint64_t> valueMap;
    valueMap.insert(pair<string, uint64_t>("Pss", graphicsMemory.gl));
    valueMap.insert(pair<string, uint64_t>("Private_Dirty", graphicsMemory.gl));
    groupMap.insert(pair<string, map<string, uint64_t>>("AnonPage # GL", valueMap));
    valueMap.clear();
    valueMap.insert(pair<string, uint64_t>("Pss", graphicsMemory.graph));
    valueMap.insert(pair<string, uint64_t>("Private_Dirty", graphicsMemory.graph));
    groupMap.insert(pair<string, map<string, uint64_t>>("AnonPage # Graph", valueMap));
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
        vector<string> hardware;
        string title = "Hardware Usage:";
        StringUtils::GetInstance().SetWidth(RAM_WIDTH_, BLANK_, false, title);
        hardware.push_back(title);
        hardware.push_back(AddKbUnit(value));
        result->push_back(hardware);
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
        vector<string> cma;
        string title = "CMA Usage:";
        StringUtils::GetInstance().SetWidth(RAM_WIDTH_, BLANK_, false, title);
        cma.push_back(title);
        cma.push_back(AddKbUnit(value));
        result->push_back(cma);
    }
    return success;
}

bool MemoryInfo::GetKernelUsage(const ValueMap &infos, StringMatrix result)
{
    uint64_t value = 0;
    unique_ptr<GetKernelInfo> getGetKernelInfo = make_unique<GetKernelInfo>();
    bool success = getGetKernelInfo->GetKernel(infos, value);
    if (success) {
        vector<string> kernel;
        string title = "Kernel Usage:";
        StringUtils::GetInstance().SetWidth(RAM_WIDTH_, BLANK_, false, title);
        kernel.push_back(title);
        kernel.push_back(AddKbUnit(value));
        result->push_back(kernel);
    }
    return success;
}

void MemoryInfo::GetProcesses(const GroupMap &infos, StringMatrix result)
{
    uint64_t value = 0;

    unique_ptr<GetProcessInfo> getProcessInfo = make_unique<GetProcessInfo>();
    value = getProcessInfo->GetProcess(infos);

    vector<string> process;
    string title = "Processes Usage:";
    StringUtils::GetInstance().SetWidth(RAM_WIDTH_, BLANK_, false, title);
    process.push_back(title);
    process.push_back(AddKbUnit(value));
    result->push_back(process);
}

void MemoryInfo::GetPssTotal(const GroupMap &infos, StringMatrix result)
{
    vector<string> title;
    title.push_back("Total Pss by Category:");
    result->push_back(title);

    vector<pair<string, uint64_t>> filePage;
    vector<pair<string, uint64_t>> anonPage;
    for (const auto &info : infos) {
        vector<string> pageTag;
        StringUtils::GetInstance().StringSplit(info.first, "#", pageTag);
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
    dmaValue.push_back(make_pair(MemoryFilter::GetInstance().DMA_OUT_LABEL, dmaInfo_.GetTotalDma()));
    PairToStringMatrix(MemoryFilter::GetInstance().DMA_TAG, dmaValue, result);
}

void MemoryInfo::PairToStringMatrix(const string &titleStr, vector<pair<string, uint64_t>> &vec, StringMatrix result)
{
    uint64_t totalPss = accumulate(vec.begin(), vec.end(), (uint64_t)0, [] (uint64_t a, pair<string, uint64_t> &b) {
        return a + b.second;
    });
    vector<string> title;
    title.push_back(titleStr + "(" + AddKbUnit(totalPss) + "):");
    result->push_back(title);

    std::sort(vec.begin(), vec.end(),
        [] (pair<string, uint64_t> &left, pair<string, uint64_t> &right) {
        return right.second < left.second;
    });
    for (const auto &pair : vec) {
        vector<string> line;
        string pssStr = AddKbUnit(pair.second);
        StringUtils::GetInstance().SetWidth(RAM_WIDTH_, BLANK_, false, pssStr);
        line.push_back(pssStr + " : " + pair.first);
        result->push_back(line);
    }
}

void MemoryInfo::GetRamUsage(const GroupMap &smapsinfos, const ValueMap &meminfo, StringMatrix result)
{
    unique_ptr<GetRamInfo> getRamInfo = make_unique<GetRamInfo>();
    GetRamInfo::Ram ram = getRamInfo->GetRam(smapsinfos, meminfo);

    vector<string> total;
    string totalTitle = "Total RAM:";
    StringUtils::GetInstance().SetWidth(RAM_WIDTH_, BLANK_, false, totalTitle);
    total.push_back(totalTitle);
    total.push_back(AddKbUnit(ram.total));
    result->push_back(total);

    vector<string> free;
    string freeTitle = "Free RAM:";
    StringUtils::GetInstance().SetWidth(RAM_WIDTH_, BLANK_, false, freeTitle);
    free.push_back(freeTitle);
    free.push_back(AddKbUnit(ram.free));
    free.push_back(" (" + to_string(ram.cachedInfo) + " cached + " + to_string(ram.freeInfo) + " free)");
    result->push_back(free);

    vector<string> used;
    string usedTitle = "Used RAM:";
    StringUtils::GetInstance().SetWidth(RAM_WIDTH_, BLANK_, false, usedTitle);
    used.push_back(usedTitle);
    used.push_back(AddKbUnit(ram.used));
    used.push_back(" (" + to_string(ram.totalPss) + " total pss + " + to_string(ram.kernelUsed) + " kernel)");
    result->push_back(used);

    vector<string> lost;
    string lostTitle = "Lost RAM:";
    StringUtils::GetInstance().SetWidth(RAM_WIDTH_, BLANK_, false, lostTitle);
    lost.push_back(lostTitle);
    lost.push_back(AddKbUnit(ram.lost));
    result->push_back(lost);
}

void MemoryInfo::GetPurgTotal(const ValueMap &meminfo, StringMatrix result)
{
    vector<string> title;
    title.push_back("Total Purgeable:");
    result->push_back(title);

    uint64_t purgSumTotal = meminfo.find(MemoryFilter::GetInstance().PURG_SUM[0])->second +
                            meminfo.find(MemoryFilter::GetInstance().PURG_SUM[1])->second;
    uint64_t purgPinTotal = meminfo.find(MemoryFilter::GetInstance().PURG_PIN[0])->second;

    vector<string> purgSum;
    string totalPurgSumTitle = "Total PurgSum:";
    StringUtils::GetInstance().SetWidth(RAM_WIDTH_, BLANK_, false, totalPurgSumTitle);
    purgSum.push_back(totalPurgSumTitle);
    purgSum.push_back(AddKbUnit(purgSumTotal));
    result->push_back(purgSum);

    vector<string> purgPin;
    string totalPurgPinTitle = "Total PurgPin:";
    StringUtils::GetInstance().SetWidth(RAM_WIDTH_, BLANK_, false, totalPurgPinTitle);
    purgPin.push_back(totalPurgPinTitle);
    purgPin.push_back(AddKbUnit(purgPinTotal));
    result->push_back(purgPin);
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

void MemoryInfo::GetNativeValue(const string& tag, const GroupMap& nativeGroupMap, StringMatrix result)
{
    vector<string> heap;
    string heapTitle = tag + ":";
    StringUtils::GetInstance().SetWidth(RAM_WIDTH_, BLANK_, false, heapTitle);
    heap.push_back(heapTitle);

    auto info = nativeGroupMap.find(tag);
    if (info == nativeGroupMap.end()) {
        DUMPER_HILOGE(MODULE_SERVICE, "GetNativeValue fail! tag = %{public}s", tag.c_str());
        return;
    }
    string nativeValue;
    auto &valueMap = info->second;
    for (const auto &key : MemoryFilter::GetInstance().VALUE_WITH_PID) {
        auto it = valueMap.find(key);
        string value = "0";
        if (it != valueMap.end()) {
            value = to_string(it->second);
        }
        StringUtils::GetInstance().SetWidth(LINE_WIDTH_, BLANK_, false, value);
        nativeValue += value;
    }

    heap.push_back(nativeValue);
    result->push_back(heap);
}

void MemoryInfo::GetNativeHeap(const GroupMap& nativeGroupMap, StringMatrix result)
{
    AddBlankLine(result);
    vector<string> title;
    title.push_back(MemoryFilter::GetInstance().NATIVE_HEAP_LABEL + ":");
    result->push_back(title);
    for (const auto &it: NATIVE_HEAP_TAG_) {
        GetNativeValue(it, nativeGroupMap, result);
    }
}

void MemoryInfo::GetDmaByPid(const int32_t &pid, StringMatrix result)
{
    AddBlankLine(result);
    vector<string> title;
    title.push_back(MemoryFilter::GetInstance().DMA_TAG + ":");
    result->push_back(title);

    vector<string> dma;
    string dmaTitle = MemoryFilter::GetInstance().DMA_OUT_LABEL + ":";
    StringUtils::GetInstance().SetWidth(RAM_WIDTH_, BLANK_, false, dmaTitle);
    dma.push_back(dmaTitle);
    dma.push_back(AddKbUnit(dmaInfo_.GetDmaByPid(pid)));
    result->push_back(dma);
}

void MemoryInfo::GetHiaiServerIon(const int32_t &pid, StringMatrix result)
{
    if (GetProcName(pid) != "hiaiserver") {
        return;
    }
    void *handle = dlopen(LIB_PATH.c_str(), RTLD_LAZY);
    if (handle == nullptr) {
        DUMPER_HILOGE(MODULE_SERVICE, "fail to open %{public}s.", LIB_PATH.c_str());
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

void MemoryInfo::GetRamCategory(const GroupMap &smapsInfos, const ValueMap &meminfos, StringMatrix result)
{
    vector<string> title;
    title.push_back("Total RAM by Category:");
    result->push_back(title);

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

void MemoryInfo::GetNSPidByPid(const int32_t &pid, std::string &nsPid)
{
    /*
        ls -al /proc/1/ns/pid
        lrwxrwxrwx 1 root root 0 2023-11-01 16:03 /proc/1/ns/pid -> pid:[4026531836]
    */
    string cmd = "ls -al /proc/" + to_string(pid) + "/ns/pid";
    vector<string> cmdResult;
    if (!MemoryUtil::GetInstance().RunCMD(cmd, cmdResult) || cmdResult.size() == 0) {
        DUMPER_HILOGE(MODULE_SERVICE, "GetNSPidByPid fail! pid = %{public}d", pid);
        return;
    }
    string pidStr = cmdResult.front();
    StringUtils::GetInstance().StringRegex(pidStr, "\\[([0-9]+)\\]", 1, nsPid);
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
        procName = GetProcStatusName(pid);
    }
    return procName;
}

std::string MemoryInfo::GetProcStatusName(const int32_t &pid)
{
    string str = "grep \"Name\" /proc/" + to_string(pid) + "/status";
    string procName = UNKNOWN_PROCESS;
    vector<string> cmdResult;
    if (!MemoryUtil::GetInstance().RunCMD(str, cmdResult) || cmdResult.size() == 0) {
        DUMPER_HILOGE(MODULE_SERVICE, "GetProcName fail! pid = %{public}d", pid);
        return procName;
    }
    vector<string> names;
    StringUtils::GetInstance().StringSplit(cmdResult.at(0), ":", names);
    if (names.empty()) {
        return procName;
    }
    procName = cmdResult.at(0).substr(names[0].length() + 1);
    return procName;
}

uint64_t MemoryInfo::GetProcValue(const int32_t &pid, const string& key)
{
    string str = "grep \"" + key + "\" /proc/" + to_string(pid) + "/status";
    vector<string> cmdResult;
    if (!MemoryUtil::GetInstance().RunCMD(str, cmdResult) || cmdResult.size() == 0) {
        DUMPER_HILOGE(MODULE_SERVICE, "GetProcValue RunCMD failed");
        return 0;
    }
    vector<string> names;
    StringUtils::GetInstance().StringSplit(cmdResult.at(0), ":", names);
    if (names.empty()) {
        DUMPER_HILOGE(MODULE_SERVICE, "GetProcValue names is empty");
        return 0;
    }
    return stoi(names[1].substr(0, names[1].size() - 3)); // 3: ' kB'
}

string MemoryInfo::GetProcessAdjLabel(const int32_t pid)
{
    string adjLabel = RECLAIM_PRIORITY_UNKNOWN_DESC;
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
    int value = 0;
    std::string priority(buf);
    StrToInt(priority.substr(0, priority.size() - 1), value);
    adjLabel = GetReclaimPriorityString(value);
    return adjLabel;
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

bool MemoryInfo::GetGraphicsMemory(int32_t pid, MemInfoData::GraphicsMemory &graphicsMemory)
{
    if ((IsRenderService(pid))) {
#ifdef HIDUMPER_GRAPHIC_ENABLE
        GetMemGraphics();
#endif
        GetRenderServiceGraphics(pid, graphicsMemory);
        if (graphicsMemory.gl > g_sumPidsMemGL) {
            graphicsMemory.gl -= g_sumPidsMemGL;
        } else {
            DUMPER_HILOGE(MODULE_SERVICE, "GL: %{public}d, sum pid GL: %{public}d",
                static_cast<int>(graphicsMemory.gl), static_cast<int>(g_sumPidsMemGL));
        }
    } else {
        GetRenderServiceGraphics(pid, graphicsMemory);
#ifdef HIDUMPER_GRAPHIC_ENABLE
        auto& rsClient = Rosen::RSInterfaces::GetInstance();
        unique_ptr<MemoryGraphic> memGraphic = make_unique<MemoryGraphic>(rsClient.GetMemoryGraphic(pid));
        graphicsMemory.gl += memGraphic-> GetGpuMemorySize() / BYTE_PER_KB;
#endif
    }
    return true;
}

bool MemoryInfo::GetMemByProcessPid(const int32_t &pid, const DmaInfo &dmaInfo, MemInfoData::MemUsage &usage)
{
    bool success = false;
    MemInfoData::MemInfo memInfo;
    unique_ptr<ParseSmapsRollupInfo> getSmapsRollup = make_unique<ParseSmapsRollupInfo>();
    if (getSmapsRollup->GetMemInfo(pid, memInfo)) {
        usage.vss = GetVss(pid);
        usage.uss = memInfo.privateClean + memInfo.privateDirty;
        usage.rss = memInfo.rss;
        usage.pss = memInfo.pss;
        usage.swapPss = memInfo.swapPss;
        usage.dma = dmaInfo.GetDmaByPid(pid);
        usage.name = GetProcName(pid);
        usage.pid = pid;
        usage.adjLabel = GetProcessAdjLabel(pid);
        success = true;
    }

    MemInfoData::GraphicsMemory graphicsMemory;
    MemoryUtil::GetInstance().InitGraphicsMemory(graphicsMemory);
    graphicsMemory.graph = dmaInfo_.GetDmaByPid(pid);
    if (GetGraphicsMemory(pid, graphicsMemory)) {
        usage.gl = graphicsMemory.gl;
        usage.graph = graphicsMemory.graph;
        usage.uss = usage.uss + graphicsMemory.gl + graphicsMemory.graph;
        usage.pss = usage.pss + graphicsMemory.gl + graphicsMemory.graph;
        usage.rss = usage.rss + graphicsMemory.gl + graphicsMemory.graph;
    }
    return success;
}

void MemoryInfo::MemUsageToMatrix(const MemInfoData::MemUsage &memUsage, StringMatrix result)
{
    vector<string> strs;
    string pid = to_string(memUsage.pid);
    StringUtils::GetInstance().SetWidth(PID_WIDTH_, BLANK_, true, pid);
    strs.push_back(pid);

    uint64_t pss = memUsage.pss + memUsage.swapPss;
    string totalPss = to_string(pss) + "(" + to_string(memUsage.swapPss) + " in SwapPss) kB";
    StringUtils::GetInstance().SetWidth(PSS_WIDTH_, BLANK_, false, totalPss);
    strs.push_back(totalPss);

    uint64_t vss = memUsage.vss;
    string totalVss = AddKbUnit(vss);
    StringUtils::GetInstance().SetWidth(KB_WIDTH_, BLANK_, false, totalVss);
    strs.push_back(totalVss);

    uint64_t rss = memUsage.rss;
    string totalRss = AddKbUnit(rss);
    StringUtils::GetInstance().SetWidth(KB_WIDTH_, BLANK_, false, totalRss);
    strs.push_back(totalRss);

    uint64_t uss = memUsage.uss;
    string totalUss = AddKbUnit(uss);
    StringUtils::GetInstance().SetWidth(KB_WIDTH_, BLANK_, false, totalUss);
    strs.push_back(totalUss);

    uint64_t gl = memUsage.gl;
    string unMappedGL = AddKbUnit(gl);
    StringUtils::GetInstance().SetWidth(KB_WIDTH_, BLANK_, false, unMappedGL);
    strs.push_back(unMappedGL);

    uint64_t graph = memUsage.graph;
    string unMappedGraph = AddKbUnit(graph);
    StringUtils::GetInstance().SetWidth(KB_WIDTH_, BLANK_, false, unMappedGraph);
    strs.push_back(unMappedGraph);

    uint64_t dma = memUsage.dma;
    string unMappedDma = AddKbUnit(dma);
    StringUtils::GetInstance().SetWidth(KB_WIDTH_, BLANK_, false, unMappedDma);
    strs.push_back(unMappedDma);

    uint64_t purgSum = memUsage.purgSum;
    string unMappedPurgSum = AddKbUnit(purgSum);
    StringUtils::GetInstance().SetWidth(KB_WIDTH_, BLANK_, false, unMappedPurgSum);
    strs.push_back(unMappedPurgSum);

    uint64_t purgPin = memUsage.purgPin;
    string unMappedPurgPin = AddKbUnit(purgPin);
    StringUtils::GetInstance().SetWidth(KB_WIDTH_, BLANK_, false, unMappedPurgPin);
    strs.push_back(unMappedPurgPin);

    string name = memUsage.name;
    string preBlank = "   ";
    StringUtils::GetInstance().SetWidth(NAME_WIDTH_, BLANK_, true, name);
    strs.push_back(preBlank.append(name));

    result->emplace_back(strs);
}

void MemoryInfo::AddMemByProcessTitle(StringMatrix result, string sortType)
{
    vector<string> process;
    string processTitle = "Total Memory Usage by " + sortType + ":";
    process.push_back(processTitle);
    result->push_back(process);

    vector<string> title;
    string pid = "PID";
    StringUtils::GetInstance().SetWidth(PID_WIDTH_, BLANK_, true, pid);
    title.push_back(pid);

    string totalPss = "Total Pss(xxx in SwapPss)";
    StringUtils::GetInstance().SetWidth(PSS_WIDTH_, BLANK_, false, totalPss);
    title.push_back(totalPss);

    string totalVss = "Total Vss";
    StringUtils::GetInstance().SetWidth(KB_WIDTH_, BLANK_, false, totalVss);
    title.push_back(totalVss);

    string totalRss = "Total Rss";
    StringUtils::GetInstance().SetWidth(KB_WIDTH_, BLANK_, false, totalRss);
    title.push_back(totalRss);

    string totalUss = "Total Uss";
    StringUtils::GetInstance().SetWidth(KB_WIDTH_, BLANK_, false, totalUss);
    title.push_back(totalUss);

    string unMappedGL = MemoryFilter::GetInstance().GL_OUT_LABEL;
    StringUtils::GetInstance().SetWidth(KB_WIDTH_, BLANK_, false, unMappedGL);
    title.push_back(unMappedGL);

    string unMappedGraph = MemoryFilter::GetInstance().GRAPH_OUT_LABEL;
    StringUtils::GetInstance().SetWidth(KB_WIDTH_, BLANK_, false, unMappedGraph);
    title.push_back(unMappedGraph);

    string unMappedDma = MemoryFilter::GetInstance().DMA_OUT_LABEL;
    StringUtils::GetInstance().SetWidth(KB_WIDTH_, BLANK_, false, unMappedDma);
    title.push_back(unMappedDma);

    string unMappedPurgSum = MemoryFilter::GetInstance().PURGSUM_OUT_LABEL;
    StringUtils::GetInstance().SetWidth(KB_WIDTH_, BLANK_, false, unMappedPurgSum);
    title.push_back(unMappedPurgSum);

    string unMappedPurgPin = MemoryFilter::GetInstance().PURGPIN_OUT_LABEL;
    StringUtils::GetInstance().SetWidth(KB_WIDTH_, BLANK_, false, unMappedPurgPin);
    title.push_back(unMappedPurgPin);

    string name = "Name";
    string preBlank = "   ";
    StringUtils::GetInstance().SetWidth(NAME_WIDTH_, BLANK_, true, name);
    title.push_back(preBlank.append(name));

    result->push_back(title);
}

#ifdef HIDUMPER_GRAPHIC_ENABLE
void MemoryInfo::GetMemGraphics()
{
    memGraphicVec_.clear();
    auto& rsClient = Rosen::RSInterfaces::GetInstance();
    memGraphicVec_ = rsClient.GetMemoryGraphics();
    auto sumPidsMemGL = 0;
    for (auto it = memGraphicVec_.begin(); it != memGraphicVec_.end(); it++) {
        sumPidsMemGL += it->GetGpuMemorySize();
    }
    g_sumPidsMemGL = static_cast<uint64_t>(sumPidsMemGL / BYTE_PER_KB);
}
#endif

DumpStatus MemoryInfo::GetMemoryInfoNoPid(StringMatrix result)
{
    if (!dmaInfo_.ParseDmaInfo()) {
        DUMPER_HILOGE(MODULE_SERVICE, "Parse dma info error\n");
    }
    if (!isReady_) {
        memUsages_.clear();
        pids_.clear();
        totalGL_ = 0;
        totalGraph_ = 0;
        AddMemByProcessTitle(result, "PID");
        if (!GetPids()) {
            return DUMP_FAIL;
        }
#ifdef HIDUMPER_GRAPHIC_ENABLE
        GetMemGraphics();
#endif
        isReady_ = true;
        return DUMP_MORE_DATA;
    }

    if (!dumpSmapsOnStart_) {
        dumpSmapsOnStart_ = true;
        fut_ = std::async(std::launch::async, [&]() {
            GroupMap groupMap;
            std::vector<int32_t> pids(pids_);
            for (auto pid : pids) {
                GetSmapsInfoNoPid(pid, groupMap);
            }
            return groupMap;
        });
    }
    MemInfoData::MemUsage usage;
    MemoryUtil::GetInstance().InitMemUsage(usage);
    if (pids_.size() > 0) {
        if (GetMemByProcessPid(pids_.front(), dmaInfo_, usage)) {
            memUsages_.push_back(usage);
            adjMemResult_[usage.adjLabel].push_back(usage);
            totalGL_ += usage.gl;
            totalGraph_ += usage.graph;
            MemUsageToMatrix(usage, result);
        } else {
            DUMPER_HILOGE(MODULE_SERVICE, "Get smaps_rollup error! pid = %{public}d\n",
                          static_cast<int>(pids_.front()));
        }
        pids_.erase(pids_.begin());
        return DUMP_MORE_DATA;
    }
    return DealResult(result);
}

DumpStatus MemoryInfo::DealResult(StringMatrix result)
{
    ValueMap meminfoResult;
    if (!GetMeminfo(meminfoResult)) {
        DUMPER_HILOGE(MODULE_SERVICE, "Get meminfo error\n");
        return DUMP_FAIL;
    }

    GetSortedMemoryInfoNoPid(result);
    AddBlankLine(result);
    GetMemoryByAdj(result);
    AddBlankLine(result);

    GroupMap smapsResult = fut_.get();

    GetPssTotal(smapsResult, result);
    AddBlankLine(result);

    GetRamUsage(smapsResult, meminfoResult, result);
    AddBlankLine(result);

    GetRamCategory(smapsResult, meminfoResult, result);
    AddBlankLine(result);

    GetPurgTotal(meminfoResult, result);

    isReady_ = false;
    dumpSmapsOnStart_ = false;
    memUsages_.clear();
    smapsResult.clear();
    return DUMP_OK;
}

void MemoryInfo::GetSortedMemoryInfoNoPid(StringMatrix result)
{
    AddBlankLine(result);
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

void MemoryInfo::GetMemoryByAdj(StringMatrix result)
{
    vector<string> title;
    title.push_back("Total Pss by OOM adjustment:");
    result->push_back(title);

    vector<string> reclaimPriority_;
    for (auto reclaim : ReclaimPriorityMapping) {
        reclaimPriority_.push_back(reclaim.second);
    }
    reclaimPriority_.push_back(RECLAIM_PRIORITY_UNKNOWN_DESC);

    for (const auto &adjLabel : reclaimPriority_) {
        vector<MemInfoData::MemUsage> memUsages = adjMemResult_[adjLabel];
        vector<string> label;
        if (memUsages.size() == 0) {
            label.push_back(adjLabel + ": " + AddKbUnit(0));
            result->push_back(label);
            continue;
        }
        uint64_t totalPss = accumulate(memUsages.begin(), memUsages.end(), (uint64_t)0,
        [] (uint64_t a, MemInfoData::MemUsage &b) {
            return a + b.pss + b.swapPss;
        });
        label.push_back(adjLabel + ": " + AddKbUnit(totalPss));
        result->push_back(label);

        std::sort(memUsages.begin(), memUsages.end(),
            [] (MemInfoData::MemUsage &left, MemInfoData::MemUsage &right) {
            return right.pss + right.swapPss < left.pss + left.swapPss;
        });
        for (const auto &memUsage : memUsages) {
            vector<string> line;
            string name = PRE_BLANK + memUsage.name + "(pid=" + to_string(memUsage.pid) + "): ";
            StringUtils::GetInstance().SetWidth(NAME_AND_PID_WIDTH, BLANK_, true, name);
            line.push_back(name + AddKbUnit(memUsage.pss + memUsage.swapPss));
            if (memUsage.swapPss > 0) {
                line.push_back(" (" + to_string(memUsage.swapPss) + " kB in SwapPss)");
            }
            result->push_back(line);
        }
    }
}

std::string MemoryInfo::GetReclaimPriorityString(int32_t priority)
{
    if (priority < RECLAIM_PRIORITY_SYSTEM || priority > RECLAIM_PRIORITY_UNKNOWN) {
        return RECLAIM_PRIORITY_UNKNOWN_DESC;
    } else if (priority < RECLAIM_ONDEMAND_SYSTEM) {
        return ReclaimPriorityMapping.at(RECLAIM_PRIORITY_SYSTEM);
    } else if (priority < RECLAIM_PRIORITY_KILLABLE_SYSTEM) {
        return ReclaimPriorityMapping.at(RECLAIM_ONDEMAND_SYSTEM);
    } else if (priority < RECLAIM_PRIORITY_FOREGROUND) {
        return ReclaimPriorityMapping.at(RECLAIM_PRIORITY_KILLABLE_SYSTEM);
    } else if (priority < RECLAIM_PRIORITY_VISIBLE) {
        return ReclaimPriorityMapping.at(RECLAIM_PRIORITY_FOREGROUND);
    } else if (priority < RECLAIM_PRIORITY_BG_SUSPEND_DELAY) {
        return ReclaimPriorityMapping.at(RECLAIM_PRIORITY_VISIBLE);
    } else if (priority < RECLAIM_PRIORITY_BG_PERCEIVED) {
        return ReclaimPriorityMapping.at(RECLAIM_PRIORITY_BG_SUSPEND_DELAY);
    } else if (priority < RECLAIM_PRIORITY_BG_DIST_DEVICE) {
        return ReclaimPriorityMapping.at(RECLAIM_PRIORITY_BG_PERCEIVED);
    } else if (priority < RECLAIM_PRIORITY_BACKGROUND) {
        return ReclaimPriorityMapping.at(RECLAIM_PRIORITY_BG_DIST_DEVICE);
    } else {
        return ReclaimPriorityMapping.at(RECLAIM_PRIORITY_BACKGROUND);
    }
}

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
