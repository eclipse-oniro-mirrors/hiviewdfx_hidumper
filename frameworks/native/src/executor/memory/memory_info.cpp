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

#include <cinttypes>
#include <fstream>
#include <numeric>
#include <thread>
#include <v1_0/imemory_tracker_interface.h>

#include "dump_common_utils.h"
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
#include "mem_mgr_constant.h"
#include "securec.h"
#include "string_ex.h"
#include "util/string_utils.h"
using namespace std;
using namespace OHOS::HDI::Memorytracker::V1_0;
namespace OHOS {
namespace HiviewDFX {
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
}

MemoryInfo::~MemoryInfo()
{
}

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

void MemoryInfo::SetValue(const string &value, vector<string> &lines, vector<string> &values)
{
    string separator = "-";
    StringUtils::GetInstance().SetWidth(LINE_WIDTH_, SEPARATOR_, false, separator);
    lines.push_back(separator);
    string tempValue = value;
    StringUtils::GetInstance().SetWidth(LINE_WIDTH_, BLANK_, false, tempValue);
    values.push_back(tempValue);
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

    SetValue("Total", lines, values);
    SetValue(to_string(meminfo.pss + meminfo.swapPss), lines, values);
    SetValue(to_string(meminfo.sharedClean), lines, values);
    SetValue(to_string(meminfo.sharedDirty), lines, values);
    SetValue(to_string(meminfo.privateClean), lines, values);
    SetValue(to_string(meminfo.privateDirty), lines, values);
    SetValue(to_string(meminfo.swap), lines, values);
    SetValue(to_string(meminfo.swapPss), lines, values);

    result->push_back(lines);
    result->push_back(values);
}

bool MemoryInfo::GetGraphicsMemory(int32_t pid, MemInfoData::GraphicsMemory &graphicsMemory)
{
    bool ret = false;
    sptr<IMemoryTrackerInterface> memtrack = IMemoryTrackerInterface::Get(true);
    if (memtrack == nullptr) {
        DUMPER_HILOGE(MODULE_SERVICE, "memtrack service is null");
        return ret;
    }

    for (const auto &memTrackerType : MemoryFilter::GetInstance().MEMORY_TRACKER_TYPES) {
        std::vector<MemoryRecord> records;
        if (memtrack->GetDevMem(pid, memTrackerType.first, records) == HDF_SUCCESS) {
            uint64_t value = 0;
            for (const auto &record : records) {
                if ((static_cast<uint32_t>(record.flags) & FLAG_UNMAPPED) == FLAG_UNMAPPED) {
                    value = static_cast<uint64_t>(record.size / BYTE_PER_KB);
                    break;
                }
            }
            if (memTrackerType.first == MEMORY_TRACKER_TYPE_GL) {
                graphicsMemory.gl = value;
                ret = true;
            } else if (memTrackerType.first == MEMORY_TRACKER_TYPE_GRAPH) {
                graphicsMemory.graph = value;
                ret = true;
            }
        }
    }
    return ret;
}

bool MemoryInfo::GetMemoryInfoByPid(const int &pid, StringMatrix result)
{
    GroupMap groupMap;
    unique_ptr<ParseSmapsInfo> parseSmapsInfo = make_unique<ParseSmapsInfo>();
    if (!parseSmapsInfo->GetInfo(MemoryFilter::APPOINT_PID, pid, groupMap)) {
        DUMPER_HILOGE(MODULE_SERVICE, "parse smaps info fail");
        return false;
    }

    MemInfoData::GraphicsMemory graphicsMemory;
    MemoryUtil::GetInstance().InitGraphicsMemory(graphicsMemory);
    if (GetGraphicsMemory(pid, graphicsMemory)) {
        map<string, uint64_t> valueMap;
        valueMap.insert(pair<string, uint64_t>("Pss", graphicsMemory.gl));
        valueMap.insert(pair<string, uint64_t>("Private_Dirty", graphicsMemory.gl));
        groupMap.insert(pair<string, map<string, uint64_t>>("AnonPage # GL", valueMap));
        valueMap.clear();
        valueMap.insert(pair<string, uint64_t>("Pss", graphicsMemory.graph));
        valueMap.insert(pair<string, uint64_t>("Private_Dirty", graphicsMemory.graph));
        groupMap.insert(pair<string, map<string, uint64_t>>("AnonPage # Graph", valueMap));
    }
    BuildResult(groupMap, result);
    CalcGroup(groupMap, result);
    return true;
}

string MemoryInfo::AddKbUnit(const uint64_t &value) const
{
    return to_string(value) + MemoryUtil::GetInstance().KB_UNIT_;
}

bool MemoryInfo::GetSmapsInfoNoPid(const int &pid, GroupMap &result)
{
    unique_ptr<ParseSmapsInfo> parseSmapsInfo = make_unique<ParseSmapsInfo>();
    return parseSmapsInfo->GetInfo(MemoryFilter::NOT_SPECIFIED_PID, pid, result);
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

    vector<pair<string, uint64_t>> dmaValue;
    dmaValue.push_back(make_pair(MemoryFilter::GetInstance().GL_OUT_LABEL, totalGL_));
    dmaValue.push_back(make_pair(MemoryFilter::GetInstance().GRAPH_OUT_LABEL, totalGraph_));
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

void MemoryInfo::AddBlankLine(StringMatrix result)
{
    vector<string> blank;
    blank.push_back("\n");
    result->push_back(blank);
}

string MemoryInfo::GetProcName(const int &pid)
{
    string str = "grep \"Name\" /proc/" + to_string(pid) + "/status";
    string procName = "unknown";
    vector<string> cmdResult;
    if (MemoryUtil::GetInstance().RunCMD(str, cmdResult) && cmdResult.size() > 0) {
        vector<string> names;
        StringUtils::GetInstance().StringSplit(cmdResult.at(0), ":", names);
        if (names.size() == NAME_SIZE_) {
            procName = names.at(1);
            StringUtils::GetInstance().ReplaceAll(procName, " ", "");
            return procName;
        }
    }
    DUMPER_HILOGE(MODULE_SERVICE, "GetProcName fail! pid = %d", pid);
    return procName;
}

string MemoryInfo::GetProcessAdjLabel(const int pid)
{
    string cmd = "cat /proc/" + to_string(pid) + "/oom_score_adj";
    vector<string> cmdResult;
    string adjLabel = Memory::RECLAIM_PRIORITY_UNKNOWN_DESC;
    if (!MemoryUtil::GetInstance().RunCMD(cmd, cmdResult) || cmdResult.size() == 0) {
        DUMPER_HILOGE(MODULE_SERVICE, "GetProcessAdjLabel fail! pid = %{pubilic}d", pid);
        return adjLabel;
    }
    string oom_score = cmdResult.front();
    int value = 0;
    bool ret = StrToInt(oom_score, value);
    if (!ret) {
        return adjLabel;
    }
    auto it = Memory::ReclaimPriorityMapping.find(value);
    if (it != Memory::ReclaimPriorityMapping.end()) {
        adjLabel = it->second;
    }
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

uint64_t MemoryInfo::GetVss(const int &pid)
{
    string filename = "/proc/" + to_string(pid) + "/statm";
    ifstream in(filename);
    if (!in) {
        LOG_ERR("file %s not found.\n", filename.c_str());
        return false;
    }

    uint64_t res = 0;
    string lineContent;
    getline(in, lineContent);
    if (!lineContent.empty()) {
        uint64_t tempValue = 0;
        int ret = sscanf_s(lineContent.c_str(), "%lld^*", &tempValue);
        if (ret != -1) {
            res = tempValue * VSS_BIT;
        } else {
            DUMPER_HILOGE(MODULE_SERVICE, "GetVss error! pid = %d", pid);
        }
    }
    in.close();
    return res;
}

bool MemoryInfo::GetMemByProcessPid(const int &pid, MemInfoData::MemUsage &usage)
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
        usage.name = GetProcName(pid);
        usage.pid = pid;
        usage.adjLabel = GetProcessAdjLabel(pid);
        success = true;
    }

    MemInfoData::GraphicsMemory graphicsMemory;
    MemoryUtil::GetInstance().InitGraphicsMemory(graphicsMemory);
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

    string name = memUsage.name;
    StringUtils::GetInstance().ReplaceAll(name, " ", "");
    StringUtils::GetInstance().SetWidth(NAME_WIDTH_, BLANK_, true, name);
    strs.push_back(name);

    uint64_t pss = memUsage.pss + memUsage.swapPss;
    string totalPss = to_string(pss) + "(" + to_string(memUsage.swapPss) + " in SwapPss) kB";
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

    string name = "Name";
    StringUtils::GetInstance().SetWidth(NAME_WIDTH_, BLANK_, true, name);
    title.push_back(name);

    string totalPss = "Total Pss(xxx in SwapPss)";
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

    result->push_back(title);
}

DumpStatus MemoryInfo::GetMemoryInfoNoPid(StringMatrix result)
{
    if (!isReady_) {
        memUsages_.clear();
        pids_.clear();
        totalGL_ = 0;
        totalGraph_ = 0;
        AddMemByProcessTitle(result, "PID");
        if (!GetPids()) {
            return DUMP_FAIL;
        }
        isReady_ = true;
        return DUMP_MORE_DATA;
    }

    if (!dumpSmapsOnStart_) {
        dumpSmapsOnStart_ = true;
        fut_ = std::async(std::launch::async, [&]() {
            GroupMap groupMap;
            std::vector<int> pids(pids_);
            for (auto pid : pids) {
                GetSmapsInfoNoPid(pid, groupMap);
            }
            return groupMap;
        });
    }
    MemInfoData::MemUsage usage;
    MemoryUtil::GetInstance().InitMemUsage(usage);
    if (pids_.size() > 0) {
        if (GetMemByProcessPid(pids_.front(), usage)) {
            memUsages_.push_back(usage);
            adjMemResult_[usage.adjLabel].push_back(usage);
            totalGL_ += usage.gl;
            totalGraph_ += usage.graph;
            MemUsageToMatrix(usage, result);
        } else {
            DUMPER_HILOGE(MODULE_SERVICE, "Get smaps_rollup error! pid = %{public}d\n", pids_.front());
        }
        pids_.erase(pids_.begin());
        return DUMP_MORE_DATA;
    }

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

    for (const auto &adjLabel : MemoryFilter::GetInstance().RECLAIM_PRIORITY) {
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
            string name = memUsage.name + "(pid=" + to_string(memUsage.pid) + "): ";
            StringUtils::GetInstance().SetWidth(NAME_AND_PID_WIDTH, BLANK_, true, name);
            line.push_back(name + AddKbUnit(memUsage.pss + memUsage.swapPss));
            if (memUsage.swapPss > 0) {
                line.push_back(" (" + to_string(memUsage.swapPss) + " kB in SwapPss)");
            }
            result->push_back(line);
        }
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
} // namespace HiviewDFX
} // namespace OHOS
