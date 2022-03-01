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
#include "executor/cpu_dumper.h"
#include <unistd.h>
#include <dirent.h>
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
const std::string CPUDumper::PROC_STAT_FILE_PATH = "/proc/stat";
const useconds_t CPUDumper::DUMP_DELAY_MICROSECOND = 1 * 1000 * 1000;
const int CPUDumper::DEC_SYSTEM_VALUE = 10;
const int CPUDumper::PROC_CPU_LENGTH = 256;
const int CPUDumper::CPU_STAT_USER_TIME_INDEX = 1;
const int CPUDumper::CPU_STAT_NICE_TIME_INDEX = 2;
const int CPUDumper::CPU_STAT_SYS_TIME_INDEX = 3;
const int CPUDumper::CPU_STAT_IDLE_TIME_INDEX = 4;
const int CPUDumper::CPU_STAT_IOW_TIME_INDEX = 5;
const int CPUDumper::CPU_STAT_IRQ_TIME_INDEX = 6;
const int CPUDumper::CPU_STAT_SIRQ_TIME_INDEX = 7;
const long unsigned CPUDumper::HUNDRED_PERCENT_VALUE = 100;
const int CPUDumper::PROC_INFO_USER_TIME_INDEX = 13;
const int CPUDumper::PROC_INFO_SYS_TIME_INDEX = 14;
const int CPUDumper::PROC_INFO_MINOR_FAULT_INDEX = 9;
const int CPUDumper::PROC_INFO_MAJOR_FAULT_INDEX = 11;

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
    curSpecProc_.reset();
    oldSpecProc_.reset();
    return DumpStatus::DUMP_OK;
}

DumpStatus CPUDumper::DumpCpuUsageData()
{
    GetDateAndTime(startTime_);
    DumpStatus ret = ReadCPUInfo();
    if (ret != DumpStatus::DUMP_OK) {
        return DumpStatus::DUMP_FAIL;
    }
    ret = ReadProcInfo();
    if (ret != DumpStatus::DUMP_OK) {
        return DumpStatus::DUMP_FAIL;
    }

    usleep(DUMP_DELAY_MICROSECOND);
    oldCPUInfo_ = std::move(curCPUInfo_);
    oldProcs_.assign(curProcs_.begin(), curProcs_.end());
    curProcs_.clear();

    ret = ReadCPUInfo();
    if (ret != DumpStatus::DUMP_OK) {
        return DumpStatus::DUMP_FAIL;
    }
    ret = ReadProcInfo();
    if (ret != DumpStatus::DUMP_OK) {
        return DumpStatus::DUMP_FAIL;
    }

    std::string avgInfo;
    ret = ReadLoadAvgInfo(LOAD_AVG_FILE_PATH, avgInfo);
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
    return DumpStatus::DUMP_OK;
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

DumpStatus CPUDumper::ReadCPUInfo()
{
    std::string statRawData;
    if (!LoadStringFromFile(PROC_STAT_FILE_PATH, statRawData)) {
        return DumpStatus::DUMP_FAIL;
    }
    size_t pos = statRawData.find_first_of("\n");
    if (pos == std::string::npos) {
        return DumpStatus::DUMP_FAIL;
    }

    statRawData = statRawData.substr(0, pos);
    std::vector<std::string> cpuStates;
    SplitStr(statRawData, DumpUtils::SPACE, cpuStates);
    curCPUInfo_ = std::make_shared<CPUInfo>();
    SetCPUInfo(curCPUInfo_->uTime, cpuStates[CPU_STAT_USER_TIME_INDEX]);
    SetCPUInfo(curCPUInfo_->nTime, cpuStates[CPU_STAT_NICE_TIME_INDEX]);
    SetCPUInfo(curCPUInfo_->sTime, cpuStates[CPU_STAT_SYS_TIME_INDEX]);
    SetCPUInfo(curCPUInfo_->iTime, cpuStates[CPU_STAT_IDLE_TIME_INDEX]);
    SetCPUInfo(curCPUInfo_->iowTime, cpuStates[CPU_STAT_IOW_TIME_INDEX]);
    SetCPUInfo(curCPUInfo_->irqTime, cpuStates[CPU_STAT_IRQ_TIME_INDEX]);
    SetCPUInfo(curCPUInfo_->sirqTime, cpuStates[CPU_STAT_SIRQ_TIME_INDEX]);
    return DumpStatus::DUMP_OK;
}

void CPUDumper::SetCPUInfo(long unsigned &info, const std::string &strInfo)
{
    if (IsNumericStr(strInfo)) {
        info = atol(strInfo.c_str());
    } else {
        DUMPER_HILOGE(MODULE_COMMON, "info is %{public}s not Numeric.", strInfo.c_str());
        info = 0;
    }
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

    for (size_t i = 0; i < curProcs_.size(); i++) {
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
    str.append(std::to_string(totalUsage));
    str.append("%; ");
    str.append("User Space: ");
    str.append(std::to_string(userSpaceUsage));
    str.append("%; ");
    str.append("Kernel Space: ");
    str.append(std::to_string(sysSpaceUsage));
    str.append("%; ");
    str.append("iowait: ");
    str.append(std::to_string(iowUsage));
    str.append("%; ");
    str.append("irq: ");
    str.append(std::to_string(irqUsage));
    str.append("%; ");
    str.append("idle: ");
    str.append(std::to_string(idleUsage));
    str.append("%");
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

DumpStatus CPUDumper::ReadProcInfo()
{
    std::vector<std::string> procFiles;
    GetProcessDirFiles("/proc", "stat", procFiles);
    if (procFiles.size() < 1) {
        return DumpStatus::DUMP_FAIL;
    }
    for (size_t i = 0; i < procFiles.size(); i++) {
        std::string rawData;
        if (!LoadStringFromFile(procFiles[i], rawData)) {
            continue;
        }

        std::shared_ptr<ProcInfo> ptrProcInfo = std::make_shared<ProcInfo>();
        // Get comm name, (xxx) in stat file.
        std::vector<std::string> comms;
        GetSubStrBetween(rawData, "(", ")", comms);
        ptrProcInfo->comm = comms[0];

        /**
         * @brief (xxx xxx xx) contain ' ' will effect function SplitStr,
         * there will be wrong string for infos, so replace '()' instead of (xxx xxx xx)
         */
        rawData = ReplaceStr(rawData, comms[0], "()");
        std::vector<std::string> procInfos;
        SplitStr(rawData, DumpUtils::SPACE, procInfos);

        ptrProcInfo->pid = procInfos[0];
        SetCPUInfo(ptrProcInfo->uTime, procInfos[PROC_INFO_USER_TIME_INDEX]);
        SetCPUInfo(ptrProcInfo->sTime, procInfos[PROC_INFO_SYS_TIME_INDEX]);
        ptrProcInfo->minflt = procInfos[PROC_INFO_MINOR_FAULT_INDEX];
        ptrProcInfo->majflt = procInfos[PROC_INFO_MAJOR_FAULT_INDEX];
        curProcs_.push_back(ptrProcInfo);
    }
    if (curProcs_.size() < 1) {
        return DumpStatus::DUMP_FAIL;
    }
    return DumpStatus::DUMP_OK;
}

void CPUDumper::GetProcessDirFiles(const std::string &path, const std::string &file, std::vector<std::string> &files)
{
    DIR *dir = opendir(path.c_str());
    if (dir == nullptr) {
        return;
    }

    if (cpuUsagePid_ != -1) {
        /**
         * @brief Dump cpu info of specific pid.
         *
         */
        std::string filePath(path + "/" + std::to_string(cpuUsagePid_) + "/" + file);
        files.push_back(filePath);
        closedir(dir);
        return;
    }

    while (true) {
        struct dirent *pidDir = readdir(dir);
        if (pidDir == nullptr) {
            break;
        }

        if (!IsNumericStr(std::string(pidDir->d_name))) {
            continue;
        }

        std::string filePath(path + "/" + std::string(pidDir->d_name) + "/" + file);
        files.push_back(std::move(filePath));
    }
    closedir(dir);
}

void CPUDumper::DumpProcInfo()
{
    AddStrLineToDumpInfo("Details of Processes:");
    AddStrLineToDumpInfo("    PID   Total Usage	   User Space    Kernel Space    Page Fault Minor"
                         "    Page Fault Major    Name");
    for (size_t i = 0; i < curProcs_.size(); i++) {
        char format[PROC_CPU_LENGTH] = {0};
        int ret = sprintf_s(format, PROC_CPU_LENGTH,
                            "    %-5s    %3d%%             %3d%%"
                            "           %3d%%            %8s            %8s        %-15s",
                            (curProcs_[i]->pid).c_str(), curProcs_[i]->totalUsage, curProcs_[i]->userSpaceUsage,
                            curProcs_[i]->sysSpaceUsage, (curProcs_[i]->minflt).c_str(), (curProcs_[i]->majflt).c_str(),
                            (curProcs_[i]->comm).c_str());
        if (ret < 0) {
            continue;
        }
        AddStrLineToDumpInfo(std::string(format));
    }
}

float CPUDumper::GetCpuUsage(int pid)
{
    DumpStatus ret = ReadCPUInfo();
    if (ret != DumpStatus::DUMP_OK) {
        return 0;
    }
    ret = ReadSpecProcInfo(pid);
    if (ret != DumpStatus::DUMP_OK) {
        return 0;
    }

    usleep(DUMP_DELAY_MICROSECOND);
    oldCPUInfo_ = std::move(curCPUInfo_);
    oldSpecProc_ = std::move(curSpecProc_);

    ret = ReadCPUInfo();
    if (ret != DumpStatus::DUMP_OK) {
        return 0;
    }
    ret = ReadSpecProcInfo(pid);
    if (ret != DumpStatus::DUMP_OK) {
        return 0;
    }

    long unsigned totalDeltaTime = (curCPUInfo_->uTime + curCPUInfo_->nTime + curCPUInfo_->sTime + curCPUInfo_->iTime
                                    + curCPUInfo_->iowTime + curCPUInfo_->irqTime + curCPUInfo_->sirqTime)
                                   - (oldCPUInfo_->uTime + oldCPUInfo_->nTime + oldCPUInfo_->sTime + oldCPUInfo_->iTime
                                      + oldCPUInfo_->iowTime + oldCPUInfo_->irqTime + oldCPUInfo_->sirqTime);

    if (totalDeltaTime != 0) {
        return (curSpecProc_->uTime - oldSpecProc_->uTime)
               + (curSpecProc_->sTime - oldSpecProc_->sTime) / totalDeltaTime;
    } else {
        return -1;
    }
}

DumpStatus CPUDumper::ReadSpecProcInfo(int pid)
{
    std::string filePath = "/proc/" + std::to_string(pid) + "/stat";
    std::string rawData;
    if (!LoadStringFromFile(filePath, rawData)) {
        return DumpStatus::DUMP_FAIL;
    }

    curSpecProc_ = std::make_shared<ProcInfo>();
    // Get comm name, (xxx) in stat file.
    std::vector<std::string> comms;
    GetSubStrBetween(rawData, "(", ")", comms);
    curSpecProc_->comm = comms[0];

    /**
     * @brief (xxx xxx xx) contain ' ' will effect function SplitStr,
     * there will be wrong string for infos, so replace '()' instead of (xxx xxx xx)
     */
    rawData = ReplaceStr(rawData, comms[0], "()");
    std::vector<std::string> procInfos;
    SplitStr(rawData, DumpUtils::SPACE, procInfos);

    curSpecProc_->pid = procInfos[0];
    SetCPUInfo(curSpecProc_->uTime, procInfos[PROC_INFO_USER_TIME_INDEX]);
    SetCPUInfo(curSpecProc_->sTime, procInfos[PROC_INFO_SYS_TIME_INDEX]);
    curSpecProc_->minflt = procInfos[PROC_INFO_MINOR_FAULT_INDEX];
    curSpecProc_->majflt = procInfos[PROC_INFO_MAJOR_FAULT_INDEX];

    return DumpStatus::DUMP_OK;
}
} // namespace HiviewDFX
} // namespace OHOS
