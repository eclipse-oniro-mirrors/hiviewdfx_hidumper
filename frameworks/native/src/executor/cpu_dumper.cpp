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
#include <unistd.h>
#include "executor/cpu_dumper.h"
#include "file_ex.h"
#include "datetime_ex.h"
#include "dump_utils.h"
#include "securec.h"
#include "util/string_utils.h"
namespace OHOS {
namespace HiviewDFX {
const std::string CPUDumper::LOAD_AVG_FILE_PATH = "/proc/loadavg";
const size_t CPUDumper::LOAD_AVG_INFO_COUNT = 3;
const int CPUDumper::TM_START_YEAR = 1900;
const int CPUDumper::DEC_SYSTEM_VALUE = 10;
const int CPUDumper::PROC_CPU_LENGTH = 256;
const long unsigned CPUDumper::HUNDRED_PERCENT_VALUE = 100;

CPUDumper::CPUDumper()
{
}

CPUDumper::~CPUDumper()
{
}

DumpStatus CPUDumper::PreExecute(const std::shared_ptr<DumperParameter> &parameter, StringMatrix dumpDatas)
{
    DUMPER_HILOGD(MODULE_COMMON, "debug|CPUDumper PreExecute");
    dumpCPUDatas_ = dumpDatas;
    isDumpCpuUsage_ = (parameter->GetOpts()).isDumpCpuUsage_;
    cpuUsagePid_ = (parameter->GetOpts()).cpuUsagePid_;
    if (cpuUsagePid_ != -1) {
        curSpecProc_ = std::make_shared<ProcInfo>();
        oldSpecProc_ = std::make_shared<ProcInfo>();
    }
    curCPUInfo_ = std::make_shared<CPUInfo>();
    oldCPUInfo_ = std::make_shared<CPUInfo>();
    return DumpStatus::DUMP_OK;
}

DumpStatus CPUDumper::Execute()
{
    DUMPER_HILOGD(MODULE_COMMON, "debug|CPUDumper Execute");
    DumpStatus ret = DumpStatus::DUMP_FAIL;
    if (isDumpCpuUsage_) {
        ret = DumpCpuUsageData();
    } else {
        return DumpStatus::DUMP_FAIL;
    }
    return ret;
}

DumpStatus CPUDumper::AfterExecute()
{
    curCPUInfo_.reset();
    oldCPUInfo_.reset();
    curProcs_.clear();
    oldProcs_.clear();
    if (cpuUsagePid_ != -1) {
        curSpecProc_.reset();
        oldSpecProc_.reset();
    }
    return DumpStatus::DUMP_OK;
}

DumpStatus CPUDumper::DumpCpuUsageData()
{
    GetDateAndTime(startTime_);
    if (!DumpCpuInfoUtil::GetInstance().GetCurCPUInfo(curCPUInfo_)) {
        DUMPER_HILOGE(MODULE_COMMON, "Get current cpu info failed!.");
        return DumpStatus::DUMP_FAIL;
    }
    if (!DumpCpuInfoUtil::GetInstance().GetOldCPUInfo(oldCPUInfo_)) {
        DUMPER_HILOGE(MODULE_COMMON, "Get old cpu info failed!.");
        return DumpStatus::DUMP_FAIL;
    }

    if (cpuUsagePid_ != -1) {
        if (!GetProcCPUInfo()) {
            return DumpStatus::DUMP_FAIL;
        }
    } else {
        if (!DumpCpuInfoUtil::GetInstance().GetCurProcInfo(curProcs_)) {
            DUMPER_HILOGE(MODULE_COMMON, "Get current process info failed!.");
            return DumpStatus::DUMP_FAIL;
        }
        if (!DumpCpuInfoUtil::GetInstance().GetOldProcInfo(oldProcs_)) {
            DUMPER_HILOGE(MODULE_COMMON, "Get old process info failed!.");
            return DumpStatus::DUMP_FAIL;
        }
    }
    std::string avgInfo;
    DumpStatus ret = ReadLoadAvgInfo(LOAD_AVG_FILE_PATH, avgInfo);
    if (ret != DumpStatus::DUMP_OK) {
        DUMPER_HILOGD(MODULE_COMMON, "Get LoadAvgInfo failed!");
        return DumpStatus::DUMP_FAIL;
    }
    AddStrLineToDumpInfo(avgInfo);

    GetDateAndTime(endTime_);
    std::string dumpTimeStr;
    CreateDumpTimeString(startTime_, endTime_, dumpTimeStr);
    AddStrLineToDumpInfo(dumpTimeStr);

    std::string cpuStatStr;
    CreateCPUStatString(cpuStatStr);
    AddStrLineToDumpInfo(cpuStatStr);

    DumpProcInfo();
    DumpCpuInfoUtil::GetInstance().UpdateCpuInfo();
    return DumpStatus::DUMP_OK;
}

bool CPUDumper::GetProcCPUInfo()
{
    bool ret = false;
    if (!DumpCpuInfoUtil::GetInstance().GetOldSpecProcInfo(cpuUsagePid_, oldSpecProc_)) {
        DumpCpuInfoUtil::GetInstance().UpdateCpuInfo();
        if (!DumpCpuInfoUtil::GetInstance().GetOldSpecProcInfo(cpuUsagePid_, oldSpecProc_)) {
            DUMPER_HILOGE(MODULE_COMMON, "Get old process %{public}d info failed!.", cpuUsagePid_);
            return ret;
        }

        GetInitOldCPUInfo(oldCPUInfo_, curCPUInfo_);
        usleep(DELAY_VALUE);
        if (!DumpCpuInfoUtil::GetInstance().GetCurCPUInfo(curCPUInfo_)) {
            DUMPER_HILOGE(MODULE_COMMON, "Get current cpu info failed!.");
            return ret;
        }
    }

    if (!DumpCpuInfoUtil::GetInstance().GetCurSpecProcInfo(cpuUsagePid_, curSpecProc_)) {
        DUMPER_HILOGE(MODULE_COMMON, "Get current process %{public}d info failed!.", cpuUsagePid_);
        return ret;
    }
    ret = true;
    return ret;
}

void CPUDumper::GetInitOldCPUInfo(std::shared_ptr<CPUInfo> tar, const std::shared_ptr<CPUInfo> source)
{
    if ((tar == nullptr) || (source == nullptr)) {
        return;
    }

    tar->uTime = source->uTime;
    tar->nTime = source->nTime;
    tar->sTime = source->sTime;
    tar->iTime = source->iTime;
    tar->iowTime = source->iowTime;
    tar->irqTime = source->irqTime;
    tar->sirqTime = source->sirqTime;
}

DumpStatus CPUDumper::ReadLoadAvgInfo(const std::string &filePath, std::string &info)
{
    if (!FileExists(filePath)) {
        return DumpStatus::DUMP_FAIL;
    }

    std::string rawData;
    if (!LoadStringFromFile(filePath, rawData)) {
        return DumpStatus::DUMP_FAIL;
    }
    DUMPER_HILOGD(MODULE_COMMON, "debug|rawData is %{public}s", rawData.c_str());
    std::vector<std::string> vec;
    SplitStr(rawData, DumpUtils::SPACE, vec);
    info = "Load average:";
    for (size_t i = 0; i < LOAD_AVG_INFO_COUNT; i++) {
        info.append(" ");
        info.append(vec[i]);
        if (i == LOAD_AVG_INFO_COUNT - 1) {
            info.append(";");
        } else {
            info.append(" /");
        }
    }
    info.append(" the cpu load average in 1 min, 5 min and 15 min");
    DUMPER_HILOGD(MODULE_COMMON, "debug|info is %{public}s", info.c_str());
    return DumpStatus::DUMP_OK;
}

bool CPUDumper::GetDateAndTime(std::string &dateTime)
{
    struct tm timeData = {0};
    if (!GetSystemCurrentTime(&timeData)) {
        return false;
    }

    dateTime = " ";
    dateTime.append(std::to_string(TM_START_YEAR + timeData.tm_year));
    dateTime.append("-");
    if (1 + timeData.tm_mon < DEC_SYSTEM_VALUE) {
        dateTime.append(std::to_string(0));
    }
    dateTime.append(std::to_string(1 + timeData.tm_mon));
    dateTime.append("-");
    if (timeData.tm_mday < DEC_SYSTEM_VALUE) {
        dateTime.append(std::to_string(0));
    }
    dateTime.append(std::to_string(timeData.tm_mday));
    dateTime.append(" ");
    if (timeData.tm_hour < DEC_SYSTEM_VALUE) {
        dateTime.append(std::to_string(0));
    }
    dateTime.append(std::to_string(timeData.tm_hour));
    dateTime.append(":");
    if (timeData.tm_min < DEC_SYSTEM_VALUE) {
        dateTime.append(std::to_string(0));
    }
    dateTime.append(std::to_string(timeData.tm_min));
    dateTime.append(":");
    if (timeData.tm_sec < DEC_SYSTEM_VALUE) {
        dateTime.append(std::to_string(0));
    }
    dateTime.append(std::to_string(timeData.tm_sec));
    return true;
}

void CPUDumper::CreateDumpTimeString(const std::string &startTime, const std::string &endTime, std::string &timeStr)
{
    timeStr = "CPU usage from";
    timeStr.append(startTime);
    timeStr.append(" to");
    timeStr.append(endTime);
}

void CPUDumper::AddStrLineToDumpInfo(const std::string &strLine)
{
    std::vector<std::string> vec;
    vec.push_back(strLine);
    dumpCPUDatas_->push_back(vec);
}

void CPUDumper::CreateCPUStatString(std::string &str)
{
    long unsigned totalDeltaTime = (curCPUInfo_->uTime + curCPUInfo_->nTime + curCPUInfo_->sTime + curCPUInfo_->iTime
                                    + curCPUInfo_->iowTime + curCPUInfo_->irqTime + curCPUInfo_->sirqTime)
                                   - (oldCPUInfo_->uTime + oldCPUInfo_->nTime + oldCPUInfo_->sTime + oldCPUInfo_->iTime
                                      + oldCPUInfo_->iowTime + oldCPUInfo_->irqTime + oldCPUInfo_->sirqTime);
    if (cpuUsagePid_ != -1) {
        curSpecProc_->userSpaceUsage =
            (curSpecProc_->uTime - oldSpecProc_->uTime) * HUNDRED_PERCENT_VALUE / totalDeltaTime;
        curSpecProc_->sysSpaceUsage =
            (curSpecProc_->sTime - oldSpecProc_->sTime) * HUNDRED_PERCENT_VALUE / totalDeltaTime;
        curSpecProc_->totalUsage = curSpecProc_->userSpaceUsage + curSpecProc_->sysSpaceUsage;
    } else {
        for (size_t i = 0; i < curProcs_.size(); i++) {
            if (curProcs_[i] == nullptr) {
                continue;
            }
            std::shared_ptr<ProcInfo> oldProc = GetOldProc(curProcs_[i]->pid);
            if (oldProc) {
                curProcs_[i]->userSpaceUsage =
                    (curProcs_[i]->uTime - oldProc->uTime) * HUNDRED_PERCENT_VALUE / totalDeltaTime;
                curProcs_[i]->sysSpaceUsage =
                    (curProcs_[i]->sTime - oldProc->sTime) * HUNDRED_PERCENT_VALUE / totalDeltaTime;
                curProcs_[i]->totalUsage = curProcs_[i]->userSpaceUsage + curProcs_[i]->sysSpaceUsage;
            } else {
                curProcs_[i]->userSpaceUsage = 0;
                curProcs_[i]->sysSpaceUsage = 0;
                curProcs_[i]->totalUsage = 0;
            }
        }
    }

    long unsigned userSpaceUsage =
        ((curCPUInfo_->uTime + curCPUInfo_->nTime) - (oldCPUInfo_->uTime + oldCPUInfo_->nTime)) * HUNDRED_PERCENT_VALUE
        / totalDeltaTime;
    long unsigned sysSpaceUsage = (curCPUInfo_->sTime - oldCPUInfo_->sTime) * HUNDRED_PERCENT_VALUE / totalDeltaTime;
    long unsigned iowUsage = (curCPUInfo_->iowTime - oldCPUInfo_->iowTime) * HUNDRED_PERCENT_VALUE / totalDeltaTime;
    long unsigned irqUsage =
        ((curCPUInfo_->irqTime + curCPUInfo_->sirqTime) - (oldCPUInfo_->irqTime + oldCPUInfo_->sirqTime))
        * HUNDRED_PERCENT_VALUE / totalDeltaTime;
    long unsigned idleUsage = (curCPUInfo_->iTime - oldCPUInfo_->iTime) * HUNDRED_PERCENT_VALUE / totalDeltaTime;
    long unsigned totalUsage = userSpaceUsage + sysSpaceUsage;

    str = "Total: ";
    str.append(std::to_string(totalUsage)).append("%; ");
    str.append("User Space: ").append(std::to_string(userSpaceUsage)).append("%; ");
    str.append("Kernel Space: ").append(std::to_string(sysSpaceUsage)).append("%; ");
    str.append("iowait: ").append(std::to_string(iowUsage)).append("%; ");
    str.append("irq: ").append(std::to_string(irqUsage)).append("%; ");
    str.append("idle: ").append(std::to_string(idleUsage)).append("%");
}

std::shared_ptr<ProcInfo> CPUDumper::GetOldProc(const std::string &pid)
{
    for (size_t i = 0; i < oldProcs_.size(); i++) {
        if (StringUtils::GetInstance().IsSameStr(oldProcs_[i]->pid, pid)) {
            return oldProcs_[i];
        }
    }
    return nullptr;
}

void CPUDumper::DumpProcInfo()
{
    std::vector<std::shared_ptr<ProcInfo>> sortedInfos;
    sortedInfos.assign(curProcs_.begin(), curProcs_.end());
    std::sort(sortedInfos.begin(), sortedInfos.end(), SortProcInfo);

    AddStrLineToDumpInfo("Details of Processes:");
    AddStrLineToDumpInfo("    PID   Total Usage	   User Space    Kernel Space    Page Fault Minor"
                         "    Page Fault Major    Name");
    if (cpuUsagePid_ != -1) {
        char format[PROC_CPU_LENGTH] = {0};
        int ret = sprintf_s(format, PROC_CPU_LENGTH,
                            "    %-5s    %3lu%%             %3lu%%"
                            "           %3lu%%            %8s            %8s        %-15s",
                            (curSpecProc_->pid).c_str(), curSpecProc_->totalUsage,
                            curSpecProc_->userSpaceUsage, curSpecProc_->sysSpaceUsage,
                            (curSpecProc_->minflt).c_str(), (curSpecProc_->majflt).c_str(),
                            (curSpecProc_->comm).c_str());
        AddStrLineToDumpInfo(std::string(format));
        if (ret < 0) {
            DUMPER_HILOGE(MODULE_COMMON, "Dump process %{public}d cpu info failed!.", cpuUsagePid_);
        }
        return;
    }
    for (size_t i = 0; i < sortedInfos.size(); i++) {
        char format[PROC_CPU_LENGTH] = {0};
        int ret = sprintf_s(format, PROC_CPU_LENGTH,
                            "    %-5s    %3lu%%             %3lu%%"
                            "           %3lu%%            %8s            %8s        %-15s",
                            (sortedInfos[i]->pid).c_str(), sortedInfos[i]->totalUsage,
                            sortedInfos[i]->userSpaceUsage, sortedInfos[i]->sysSpaceUsage,
                            (sortedInfos[i]->minflt).c_str(), (sortedInfos[i]->majflt).c_str(),
                            (sortedInfos[i]->comm).c_str());
        if (ret < 0) {
            continue;
        }
        AddStrLineToDumpInfo(std::string(format));
    }
}

bool CPUDumper::SortProcInfo(std::shared_ptr<ProcInfo> &left, std::shared_ptr<ProcInfo> &right)
{
    if (right->totalUsage != left->totalUsage) {
        return right->totalUsage < left->totalUsage;
    }
    if (right->userSpaceUsage != left->userSpaceUsage) {
        return right->userSpaceUsage < left->userSpaceUsage;
    }
    if (right->sysSpaceUsage != left->sysSpaceUsage) {
        return right->sysSpaceUsage < left->sysSpaceUsage;
    }
    if (right->pid.length() != left->pid.length()) {
        return right->pid.length() < left->pid.length();
    }
    return (right->pid.compare(left->pid) < 0);
}
} // namespace HiviewDFX
} // namespace OHOS
