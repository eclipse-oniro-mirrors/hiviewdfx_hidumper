/*
* Copyright (C) 2021 Huawei Device Co., Ltd.
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
#include <thread>
#include <future>
#include <fstream>
#include "executor/memory/get_cma_info.h"
#include "executor/memory/get_hardware_info.h"
#include "executor/memory/get_kernel_info.h"
#include "executor/memory/get_process_info.h"
#include "executor/memory/get_ram_info.h"
#include "executor/memory/memory_util.h"
#include "executor/memory/parse/parse_smaps_info.h"
#include "executor/memory/parse/parse_meminfo.h"
#include "dump_common_utils.h"
#include "hilog_wrapper.h"
#include "util/string_utils.h"
#include "executor/memory/parse/meminfo_data.h"
#include "executor/memory/parse/parse_smaps_rollup_info.h"
#include "securec_p.h"
using namespace std;
namespace OHOS {
namespace HiviewDFX {
MemoryInfo::MemoryInfo()
{
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

void MemoryInfo::BuildResult(const PairMatrixGroup &infos, StringMatrix result)
{
    insertMemoryTitle(result);
    for (auto info : infos) {
        vector<string> tempResult;
        string group = info.first;
        StringUtils::GetInstance().SetWidth(LINE_WIDTH_, BLANK_, false, group);
        tempResult.push_back(group);

        auto pairs = info.second;
        for (auto pair : pairs) {
            string value = to_string(pair.second);
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

void MemoryInfo::CalcGroup(const PairMatrixGroup &infos, StringMatrix result)
{
    string separator = "-";
    StringUtils::GetInstance().SetWidth(LINE_WIDTH_, SEPARATOR_, false, separator);

    MemInfoData::MemInfo meminfo;
    MemoryUtil::GetInstance().InitMemInfo(meminfo);
    for (auto info : infos) {
        auto pairs = info.second;
        for (auto pair : pairs) {
            string key = pair.first;
            uint64_t value = pair.second;
            if (key == "Pss") {
                meminfo.pss += value;
            } else if (key == "Shared_Clean") {
                meminfo.sharedClean += value;
            } else if (key == "Shared_Dirty") {
                meminfo.sharedDirty += value;
            } else if (key == "Private_Clean") {
                meminfo.privateClean += value;
            } else if (key == "Private_Dirty") {
                meminfo.privateDirty += value;
            } else if (key == "Swap") {
                meminfo.swap += value;
            } else if (key == "SwapPss") {
                meminfo.swapPss += value;
            }
        }
    }

    vector<string> lines;
    vector<string> values;

    SetValue("Total", lines, values);
    SetValue(to_string(meminfo.pss), lines, values);
    SetValue(to_string(meminfo.sharedClean), lines, values);
    SetValue(to_string(meminfo.sharedDirty), lines, values);
    SetValue(to_string(meminfo.privateClean), lines, values);
    SetValue(to_string(meminfo.privateDirty), lines, values);
    SetValue(to_string(meminfo.swap), lines, values);
    SetValue(to_string(meminfo.swapPss), lines, values);

    result->push_back(lines);
    result->push_back(values);
}

bool MemoryInfo::GetMemoryInfoByPid(const int &pid, StringMatrix result)
{
    DUMPER_HILOGD(MODULE_SERVICE, "GetMemoryInfoByPid (%d) begin\n", pid);
    PairMatrixGroup smapsInfo;
    unique_ptr<ParseSmapsInfo> parseSmapsInfo = make_unique<ParseSmapsInfo>();
    bool success = parseSmapsInfo->GetInfo(MemoryFilter::APPOINT_PID, pid, smapsInfo);
    if (success) {
        BuildResult(smapsInfo, result);
        CalcGroup(smapsInfo, result);
    }
    DUMPER_HILOGD(MODULE_SERVICE, "GetMemoryInfoByPid (%d) end,result:(%d)\n", pid, success);
    return success;
}

string MemoryInfo::AddKbUnit(const uint64_t &value)
{
    return to_string(value) + MemoryUtil::GetInstance().KB_UNIT_;
}

bool MemoryInfo::GetSmapsInfoNoPid(const int &pid, PairMatrixGroup &result)
{
    DUMPER_HILOGD(MODULE_SERVICE, "GetSmapsInfoNoPid (%d) begin\n", pid);
    unique_ptr<ParseSmapsInfo> parseSmapsInfo = make_unique<ParseSmapsInfo>();
    bool success = parseSmapsInfo->GetInfo(MemoryFilter::NOT_SPECIFIED_PID, pid, result);
    DUMPER_HILOGD(MODULE_SERVICE, "GetSmapsInfoNoPid (%d) end,result:(%d)\n", pid, success);
    return success;
}

bool MemoryInfo::GetMeminfo(PairMatrix &result)
{
    DUMPER_HILOGD(MODULE_SERVICE, "GetMeminfo begin\n");
    unique_ptr<ParseMeminfo> parseMeminfo = make_unique<ParseMeminfo>();
    bool success = parseMeminfo->GetMeminfo(result);
    DUMPER_HILOGD(MODULE_SERVICE, "GetMeminfo end\n");
    return success;
}

bool MemoryInfo::GetHardWareUsage(StringMatrix result)
{
    DUMPER_HILOGD(MODULE_SERVICE, "GetHardWareUsage begin\n");
    uint64_t value;
    unique_ptr<GetHardwareInfo> getHardwareInfo = make_unique<GetHardwareInfo>();
    bool success = getHardwareInfo->GetHardwareUsage(value);
    if (success) {
        vector<string> hardware;
        string title = "Hardware Usage:";
        StringUtils::GetInstance().SetWidth(RAM_WIDTH_, BLANK_, false, title);
        hardware.push_back(title);
        hardware.push_back(AddKbUnit(value));
        result->push_back(hardware);
    }
    DUMPER_HILOGD(MODULE_SERVICE, "GetHardWareUsage end,success:(%d)\n", success);
    return success;
}

bool MemoryInfo::GetCMAUsage(StringMatrix result)
{
    DUMPER_HILOGD(MODULE_SERVICE, "GetCMAUsage begin\n");
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
    DUMPER_HILOGD(MODULE_SERVICE, "GetCMAUsage end,success:(%d)\n", success);
    return success;
}

bool MemoryInfo::GetKernelUsage(const PairMatrix &infos, StringMatrix result)
{
    DUMPER_HILOGD(MODULE_SERVICE, "GetKernelUsage begin");
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
    DUMPER_HILOGD(MODULE_SERVICE, "GetKernelUsage end,success:(%d)\n", success);
    return success;
}

void MemoryInfo::GetProcesses(const PairMatrixGroup &infos, StringMatrix result)
{
    DUMPER_HILOGD(MODULE_SERVICE, "GetProcesses begin");
    uint64_t value = 0;

    unique_ptr<GetProcessInfo> getProcessInfo = make_unique<GetProcessInfo>();
    value = getProcessInfo->GetProcess(infos);

    vector<string> process;
    string title = "Processes Usage:";
    StringUtils::GetInstance().SetWidth(RAM_WIDTH_, BLANK_, false, title);
    process.push_back(title);
    process.push_back(AddKbUnit(value));
    result->push_back(process);
    DUMPER_HILOGD(MODULE_SERVICE, "GetProcesses end");
}

void MemoryInfo::GetPssTotal(const PairMatrixGroup &infos, StringMatrix result)
{
    DUMPER_HILOGD(MODULE_SERVICE, "GetPssTotal begin");
    vector<string> title;
    title.push_back("Total PSS by Category:");
    result->push_back(title);
    for (auto info : infos) {
        vector<string> pss;
        string group = info.first;
        auto pairs = info.second;
        uint64_t pssValue = 0;
        for (auto pair : pairs) {
            string type = pair.first;
            uint64_t value = pair.second;
            if (MemoryUtil::GetInstance().IsPss(type)) {
                pssValue += value;
            }
        }

        string pssStr = AddKbUnit(pssValue);
        StringUtils::GetInstance().SetWidth(RAM_WIDTH_, BLANK_, false, pssStr);

        pss.push_back(pssStr);
        pss.push_back(":");
        pss.push_back(group);

        result->push_back(pss);
        DUMPER_HILOGD(MODULE_SERVICE, "GetPssTotal end");
    }
}

void MemoryInfo::GetRamUsage(const PairMatrixGroup &smapsinfos, const PairMatrix &meminfo, StringMatrix result)
{
    DUMPER_HILOGD(MODULE_SERVICE, "GetRamUsage begin");
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
    result->push_back(free);

    vector<string> used;
    string usedTitle = "Used RAM:";
    StringUtils::GetInstance().SetWidth(RAM_WIDTH_, BLANK_, false, usedTitle);
    used.push_back(usedTitle);
    used.push_back(AddKbUnit(ram.used));
    result->push_back(used);

    vector<string> lost;
    string lostTitle = "Lost RAM:";
    StringUtils::GetInstance().SetWidth(RAM_WIDTH_, BLANK_, false, lostTitle);
    lost.push_back(lostTitle);
    lost.push_back(AddKbUnit(ram.lost));
    result->push_back(lost);
    DUMPER_HILOGD(MODULE_SERVICE, "GetRamUsage end");
}

void MemoryInfo::GetRamCategory(const PairMatrixGroup &smapsInfos, const PairMatrix &meminfos, StringMatrix result)
{
    DUMPER_HILOGD(MODULE_SERVICE, "GetRamCategory begin");
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
    DUMPER_HILOGD(MODULE_SERVICE, "GetRamCategory end");
}

void MemoryInfo::AddBlankLine(StringMatrix result)
{
    DUMPER_HILOGD(MODULE_SERVICE, "AddBlankLine begin");
    vector<string> blank;
    blank.push_back("\n");
    result->push_back(blank);
    DUMPER_HILOGD(MODULE_SERVICE, "AddBlankLine end");
}

bool MemoryInfo::GetProcName(const int &pid, string &name)
{
    DUMPER_HILOGD(MODULE_SERVICE, "GetProcName begin");
    string str = "grep \"Name\" /proc/" + to_string(pid) + "/status";
    vector<string> cmdResult;
    bool success = MemoryUtil::GetInstance().RunCMD(str, cmdResult);
    if (cmdResult.size() > 0) {
        vector<string> names;
        StringUtils::GetInstance().StringSplit(cmdResult.at(0), ":", names);
        if (names.size() == NAME_SIZE_) {
            name = names.at(1);
            StringUtils::GetInstance().ReplaceAll(name, " ", "");
        }
    }
    DUMPER_HILOGD(MODULE_SERVICE, "GetProcName end");
    return success;
}

bool MemoryInfo::GetPids()
{
    DUMPER_HILOGD(MODULE_SERVICE, "GetPids begin");
    pids_.clear();
    memUsages_.clear();
    bool success = DumpCommonUtils::GetUserPids(pids_);
    if (!success) {
        DUMPER_HILOGE(MODULE_SERVICE, "GetPids error\n");
    }
    DUMPER_HILOGD(MODULE_SERVICE, "GetPids end,success:(%d),pids_.size:(%zu)", success, pids_.size());
    return success;
}

bool MemoryInfo::GetVss(const int &pid, uint64_t &value)
{
    DUMPER_HILOGD(MODULE_SERVICE, "GetVss pid:(%d) begin", pid);
    string filename = "/proc/" + to_string(pid) + "/statm";
    ifstream in(filename);
    bool success = false;
    if (in) {
        string lineContent;
        getline(in, lineContent);
        if (!lineContent.empty()) {
            uint64_t tempValue;
            int ret = sscanf_s(lineContent.c_str(), "%lld^*", &tempValue);
            if (ret != -1) {
                value = tempValue * VSS_BIT;
                success = true;
            } else {
                success = false;
            }
        }
        in.close();
    } else {
        success = false;
        LOG_ERR("file %s not found.\n", filename.c_str());
    }

    DUMPER_HILOGD(MODULE_SERVICE, "GetVss pid:(%d) end,success:(%d),value:(%"PRIu64")", pid, success, value);
    return success;
}

bool MemoryInfo::GetMemByProcessPid(const int &pid, MemInfoData::MemUsage &usage)
{
    DUMPER_HILOGD(MODULE_SERVICE, "GetMemByProcessPid pid`:(%d) begin", pid);
    bool success = false;
    MemInfoData::MemInfo memInfo;
    MemoryUtil::GetInstance().InitMemInfo(memInfo);
    unique_ptr<ParseSmapsRollupInfo> getSmapsRollup = make_unique<ParseSmapsRollupInfo>();
    bool getRollupSuccess = getSmapsRollup->GetMemInfo(pid, memInfo);
    if (getRollupSuccess) {
        uint64_t vss = 0;
        bool getVssSuccess = GetVss(pid, vss);
        if (getVssSuccess) {
            usage.vss = vss;
            uint64_t uss = memInfo.privateClean + memInfo.privateDirty;
            usage.uss = uss;
            usage.rss = memInfo.rss;
            usage.pss = memInfo.pss;

            string name;
            bool getNameSuccess = GetProcName(pid, name);
            if (getNameSuccess) {
                usage.name = name;
            }
            usage.pid = pid;
            success = true;
        }
    }
    DUMPER_HILOGD(MODULE_SERVICE, "GetMemByProcessPid pid:(%d) end,success:(%d)", pid, success);
    return success;
}

MemoryInfo::MemProcessData MemoryInfo::GetMemProcess(const vector<int> &pids, const int &threadId)
{
    DUMPER_HILOGD(MODULE_SERVICE, "GetMemProcess begin\n");
    MemProcessData data;
    if (pids.size() > 0) {
        auto iter = pids.begin();
        int pid = *iter;
        data.pid = pid;
        PairMatrixGroup smapsInfo;
        bool getSmapsSuccess = GetSmapsInfoNoPid(pid, smapsInfo);
        if (getSmapsSuccess) {
            data.smapsInfo = smapsInfo;
            data.smapsSuccess = true;
            MemInfoData::MemUsage usage;
            MemoryUtil::GetInstance().InitMemUsage(usage);
            bool getProcessSuccess = GetMemByProcessPid(pid, usage);
            if (getProcessSuccess) {
                data.usage = usage;
                data.usageSuccess = true;
            } else {
                DUMPER_HILOGE(MODULE_SERVICE, "GetMemProcess process ussage (%d) ERROR\n", pid);
            }
        } else {
            DUMPER_HILOGE(MODULE_SERVICE, "GetMemProcess Smaps (%d) ERROR\n", pid);
        }
    }
    DUMPER_HILOGD(MODULE_SERVICE, "GetMemProcess end");
    return data;
}

void MemoryInfo::GetGroupOfPids(const int &index, const int &size, const std::vector<int> &pids,
                                std::vector<int> &groupPids)
{
    for (size_t i = index * size; i <= (index + 1) * size - 1; i++) {
        if (i <= pids.size() - 1) {
            groupPids.push_back(pids.at(i));
        }
    }
}

void MemoryInfo::DeletePid(vector<int> &pids, const int &pid)
{
    vector<int>::iterator iter;
    for (iter = pids.begin(); iter != pids.end();) {
        if (*iter == pid) {
            iter = pids.erase(iter);
            break;
        }
    }
}

void MemoryInfo::GetMemProcessGroup(const vector<int> &pids, PairMatrixGroup &result,
                                    vector<MemInfoData::MemUsage> &memInfos)
{
    DUMPER_HILOGD(MODULE_SERVICE, "GetMemProcessGroup begin");
    size_t threadNum = MemoryUtil::GetInstance().GetMaxThreadNum(MemoryFilter::GetInstance().SMAPS_THREAD_NUM_);
    if (threadNum == 0) {
        threadNum = 1;
    }

    if (threadNum > pids.size()) {
        threadNum = pids.size();
    }

    size_t size = pids.size() / threadNum;
    std::vector<future<MemoryInfo::MemProcessData>> futures;
    threadNum = 1;
    for (size_t i = 0; i < threadNum; i++) {
        vector<int> groupPids;
        GetGroupOfPids(i, size, pids, groupPids);
        auto funture = std::async(std::launch::async, GetMemProcess, groupPids, i);
        futures.emplace_back(std::move(funture));
    }

    for (auto &futrue : futures) {
        MemProcessData data = futrue.get();
        if (data.usageSuccess) {
            memInfos.push_back(data.usage);
            memUsages_.push_back(data.usage);
        }
        if (data.smapsSuccess) {
            MemoryUtil::GetInstance().ClacTotalByGroup(data.smapsInfo, result);
        }
        DeletePid(pids_, data.pid);
    }

    if (pids_.size() == 0) {
        memProcessDone_ = true;
    }
    DUMPER_HILOGD(MODULE_SERVICE, "GetMemProcessGroup end");
}

void MemoryInfo::MemUsageToMatrix(const vector<MemInfoData::MemUsage> &memInfos, StringMatrix result)
{
    DUMPER_HILOGD(MODULE_SERVICE, "MemUsageToMatrix begin");
    for (auto memUsage : memInfos) {
        vector<string> strs;
        string pid = to_string(memUsage.pid);
        StringUtils::GetInstance().SetWidth(PID_WIDTH_, BLANK_, true, pid);
        strs.push_back(pid);

        string name = memUsage.name;
        StringUtils::GetInstance().ReplaceAll(name, " ", "");
        StringUtils::GetInstance().SetWidth(NAME_WIDTH_, BLANK_, true, name);
        strs.push_back(name);

        uint64_t pss = memUsage.pss;
        string totalPss = AddKbUnit(pss);
        StringUtils::GetInstance().SetWidth(KB_WIDTH_, BLANK_, true, totalPss);
        strs.push_back(totalPss);

        uint64_t vss = memUsage.vss;
        string totalVss = AddKbUnit(vss);
        StringUtils::GetInstance().SetWidth(KB_WIDTH_, BLANK_, true, totalVss);
        strs.push_back(totalVss);

        uint64_t rss = memUsage.rss;
        string totalRss = AddKbUnit(rss);
        StringUtils::GetInstance().SetWidth(KB_WIDTH_, BLANK_, true, totalRss);
        strs.push_back(totalRss);

        uint64_t uss = memUsage.uss;
        string totalUss = AddKbUnit(uss);
        StringUtils::GetInstance().SetWidth(KB_WIDTH_, BLANK_, true, totalUss);
        strs.push_back(totalUss);

        result->emplace_back(strs);
    }
    DUMPER_HILOGD(MODULE_SERVICE, "MemUsageToMatrix end");
}

void MemoryInfo::AddMemByProcessTitle(StringMatrix result)
{
    DUMPER_HILOGD(MODULE_SERVICE, "AddMemByProcessTitle begin");
    vector<string> process;
    string processTitle = "Total Memory Usage by Process:";
    process.push_back(processTitle);
    result->push_back(process);

    vector<string> title;
    string pid = "PID";
    StringUtils::GetInstance().SetWidth(PID_WIDTH_, BLANK_, true, pid);
    title.push_back(pid);

    string name = "Name";
    StringUtils::GetInstance().SetWidth(NAME_WIDTH_, BLANK_, true, name);
    title.push_back(name);

    string totalPss = "Total PSS";
    StringUtils::GetInstance().SetWidth(KB_WIDTH_, BLANK_, true, totalPss);
    title.push_back(totalPss);

    string totalVss = "Total VSS";
    StringUtils::GetInstance().SetWidth(KB_WIDTH_, BLANK_, true, totalVss);
    title.push_back(totalVss);

    string totalRss = "Total RSS";
    StringUtils::GetInstance().SetWidth(KB_WIDTH_, BLANK_, true, totalRss);
    title.push_back(totalRss);

    string totalUss = "Total USS";
    StringUtils::GetInstance().SetWidth(KB_WIDTH_, BLANK_, true, totalUss);
    title.push_back(totalUss);

    result->push_back(title);
    DUMPER_HILOGD(MODULE_SERVICE, "AddMemByProcessTitle end");
}

DumpStatus MemoryInfo::GetMemoryInfoNoPid(StringMatrix result)
{
    DUMPER_HILOGD(MODULE_SERVICE, "GetMemoryInfoNoPid begin");
    if (!getPidDone_) {
        pidSuccess_ = GetPids();
        getPidDone_ = true;
    }

    if (!pidSuccess_) {
        return DUMP_FAIL;
    }

    if (!addMemProcessTitle_) {
        AddMemByProcessTitle(result);
        addMemProcessTitle_ = true;
        return DUMP_MORE_DATA;
    }

    PairMatrixGroup pairMatrixGroup;
    if (!memProcessDone_) {
        vector<MemInfoData::MemUsage> memUsages;
        GetMemProcessGroup(pids_, pairMatrixGroup, memUsages);
        if (memUsages.size() > 0) {
            MemUsageToMatrix(memUsages, result);
        }
        MemoryUtil::GetInstance().ClacTotalByGroup(pairMatrixGroup, smapsResult_);
        return DUMP_MORE_DATA;
    }

    PairMatrix meminfoResult;
    bool getMemSuccess = GetMeminfo(meminfoResult);
    if (!getMemSuccess) {
        DUMPER_HILOGE(MODULE_SERVICE, "Get MEM ERROR\n");
        return DUMP_FAIL;
    }

    GetSortedMemoryInfoNoPid(result);
    AddBlankLine(result);
    GetPssTotal(smapsResult_, result);
    AddBlankLine(result);

    GetRamUsage(smapsResult_, meminfoResult, result);
    AddBlankLine(result);

    GetRamCategory(smapsResult_, meminfoResult, result);
    DUMPER_HILOGD(MODULE_SERVICE, "GetMemoryInfoNoPid end");
    return DUMP_OK;
}

void MemoryInfo::GetSortedMemoryInfoNoPid(StringMatrix result)
{
    DUMPER_HILOGD(MODULE_SERVICE, "GetSortedMemoryInfoNoPid begin");
    AddBlankLine(result);
    AddMemByProcessTitle(result);

    std::sort(memUsages_.begin(), memUsages_.end(),
        [] (MemInfoData::MemUsage &left, MemInfoData::MemUsage &right) {
        if (right.pss != left.pss) {
            return right.pss < left.pss;
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

    MemUsageToMatrix(memUsages_, result);

    memUsages_.clear();
    DUMPER_HILOGD(MODULE_SERVICE, "GetSortedMemoryInfoNoPid end");
}
} // namespace HiviewDFX
} // namespace OHOS
