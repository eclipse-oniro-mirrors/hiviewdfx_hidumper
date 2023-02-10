/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "util/dump_cpu_info_util.h"
#include <dirent.h>
#include <unistd.h>
#include "file_ex.h"
#include "string_ex.h"
#include "hilog_wrapper.h"
namespace OHOS {
namespace HiviewDFX {
const std::string DumpCpuInfoUtil::LOAD_AVG_FILE_PATH = "/proc/loadavg";
const std::string DumpCpuInfoUtil::PROC_STAT_FILE_PATH = "/proc/stat";
const std::string DumpCpuInfoUtil::SPACE = " ";
DumpCpuInfoUtil::DumpCpuInfoUtil()
{
    DUMPER_HILOGD(MODULE_COMMON, "create debug|");
    curCPUInfo_ = std::make_shared<CPUInfo>();
    oldCPUInfo_ = std::make_shared<CPUInfo>();
}

DumpCpuInfoUtil::~DumpCpuInfoUtil()
{
    DUMPER_HILOGD(MODULE_COMMON, "release debug|");
    curCPUInfo_.reset();
    oldCPUInfo_.reset();
    curProcs_.clear();
    oldProcs_.clear();
}

void DumpCpuInfoUtil::UpdateCpuInfo()
{
    DUMPER_HILOGD(MODULE_COMMON, "UpdateCpuInfo debug|");
    std::unique_lock<std::mutex> lock(mutex_);
    CopyCpuInfo(oldCPUInfo_, curCPUInfo_);
    GetCurCPUInfo(curCPUInfo_);
    oldProcs_.assign(curProcs_.begin(), curProcs_.end());
    GetCurProcInfo(curProcs_);
}

bool DumpCpuInfoUtil::GetCurCPUInfo(std::shared_ptr<CPUInfo> &cpuInfo)
{
    if (cpuInfo == nullptr) {
        return false;
    }
    std::string statRawData;
    if (!LoadStringFromFile(PROC_STAT_FILE_PATH, statRawData)) {
        return false;
    }
    size_t pos = statRawData.find_first_of("\n");
    if (pos == std::string::npos) {
        return false;
    }

    statRawData = statRawData.substr(0, pos);
    std::vector<std::string> cpuStates;
    SplitStr(statRawData, SPACE, cpuStates);
    if (cpuStates.size() <= static_cast<size_t>(CPU_STAT_SIRQ_TIME_INDEX)) {
        return false;
    }

    SetCPUInfo(cpuInfo->uTime, cpuStates[CPU_STAT_USER_TIME_INDEX]);
    SetCPUInfo(cpuInfo->nTime, cpuStates[CPU_STAT_NICE_TIME_INDEX]);
    SetCPUInfo(cpuInfo->sTime, cpuStates[CPU_STAT_SYS_TIME_INDEX]);
    SetCPUInfo(cpuInfo->iTime, cpuStates[CPU_STAT_IDLE_TIME_INDEX]);
    SetCPUInfo(cpuInfo->iowTime, cpuStates[CPU_STAT_IOW_TIME_INDEX]);
    SetCPUInfo(cpuInfo->irqTime, cpuStates[CPU_STAT_IRQ_TIME_INDEX]);
    SetCPUInfo(cpuInfo->sirqTime, cpuStates[CPU_STAT_SIRQ_TIME_INDEX]);
    return true;
}

void DumpCpuInfoUtil::SetCPUInfo(long unsigned &info, const std::string &strInfo)
{
    if (IsNumericStr(strInfo)) {
        const int base = CONSTANT_NUM_10;
        info = strtoul(strInfo.c_str(), nullptr, base);
    } else {
        info = 0;
    }
}

bool DumpCpuInfoUtil::GetCurProcInfo(std::vector<std::shared_ptr<ProcInfo>> &procInfos)
{
    std::vector<std::string> procFiles;
    GetProcessDirFiles("/proc", "stat", procFiles);
    if (procFiles.size() < 1) {
        return false;
    }
    // Set procInfos size 0
    procInfos.clear();
    for (size_t i = 0; i < procFiles.size(); i++) {
        std::string rawData;
        if (!LoadStringFromFile(procFiles[i], rawData)) {
            continue;
        }

        // Get comm name, (xxx) in stat file.
        std::vector<std::string> comms;
        GetSubStrBetween(rawData, "(", ")", comms);
        if (comms.empty()) {
            continue;
        }

        /**
         * @brief (xxx xxx xx) contain ' ' will effect function SplitStr,
         * there will be wrong string for infos, so replace '()' instead of (xxx xxx xx)
         */
        rawData = ReplaceStr(rawData, comms[0], "()");
        std::vector<std::string> procInfosStr;
        SplitStr(rawData, SPACE, procInfosStr);
        if (procInfosStr.size() <= static_cast<size_t>(PROC_INFO_SYS_TIME_INDEX)) {
            continue;
        }

        std::shared_ptr<ProcInfo> ptrProcInfo = std::make_shared<ProcInfo>();
        ptrProcInfo->comm = comms[0];
        ptrProcInfo->pid = procInfosStr[0];
        SetCPUInfo(ptrProcInfo->uTime, procInfosStr[PROC_INFO_USER_TIME_INDEX]);
        SetCPUInfo(ptrProcInfo->sTime, procInfosStr[PROC_INFO_SYS_TIME_INDEX]);
        ptrProcInfo->minflt = procInfosStr[PROC_INFO_MINOR_FAULT_INDEX];
        ptrProcInfo->majflt = procInfosStr[PROC_INFO_MAJOR_FAULT_INDEX];
        procInfos.push_back(ptrProcInfo);
    }
    return true;
}

void DumpCpuInfoUtil::GetProcessDirFiles(const std::string &path, const std::string &file,
    std::vector<std::string> &files)
{
    DIR *dir = opendir(path.c_str());
    if (dir == nullptr) {
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

bool DumpCpuInfoUtil::GetCurSpecProcInfo(int pid, std::shared_ptr<ProcInfo> &specProc)
{
    if (specProc == nullptr) {
        return false;
    }

    std::string filePath = "/proc/" + std::to_string(pid) + "/stat";
    std::string rawData;
    if (!LoadStringFromFile(filePath, rawData)) {
        return false;
    }

    // Get comm name, (xxx) in stat file.
    std::vector<std::string> comms;
    GetSubStrBetween(rawData, "(", ")", comms);
    if (comms.empty()) {
        return false;
    }

    /**
     * @brief (xxx xxx xx) contain ' ' will effect function SplitStr,
     * there will be wrong string for infos, so replace '()' instead of (xxx xxx xx)
     */
    rawData = ReplaceStr(rawData, comms[0], "()");
    std::vector<std::string> procInfosStr;
    SplitStr(rawData, SPACE, procInfosStr);
    if (procInfosStr.size() <= static_cast<size_t>(PROC_INFO_SYS_TIME_INDEX)) {
        return false;
    }

    specProc->comm = comms[0];
    specProc->pid = procInfosStr[0];
    SetCPUInfo(specProc->uTime, procInfosStr[PROC_INFO_USER_TIME_INDEX]);
    SetCPUInfo(specProc->sTime, procInfosStr[PROC_INFO_SYS_TIME_INDEX]);
    specProc->minflt = procInfosStr[PROC_INFO_MINOR_FAULT_INDEX];
    specProc->majflt = procInfosStr[PROC_INFO_MAJOR_FAULT_INDEX];
    return true;
}

bool DumpCpuInfoUtil::GetOldCPUInfo(std::shared_ptr<CPUInfo> &cpuInfo)
{
    if (!CheckFrequentDumpping()) {
        CopyCpuInfo(cpuInfo, curCPUInfo_);
    } else {
        CopyCpuInfo(cpuInfo, oldCPUInfo_);
    }
    return true;
}

bool DumpCpuInfoUtil::GetOldProcInfo(std::vector<std::shared_ptr<ProcInfo>> &procInfos)
{
    if (!CheckFrequentDumpping()) {
        procInfos.assign(curProcs_.begin(), curProcs_.end());
    } else {
        procInfos.assign(oldProcs_.begin(), oldProcs_.end());
    }
    return true;
}

bool DumpCpuInfoUtil::GetOldSpecProcInfo(int pid, std::shared_ptr<ProcInfo> &specProc)
{
    if (!CheckFrequentDumpping()) {
        for (size_t i = 0; i < curProcs_.size(); i++) {
            if (curProcs_[i] == nullptr) {
                continue;
            }
            if (!IsNumericStr(curProcs_[i]->pid)) {
                return false;
            }
            int specPid = 0;
            if (!StrToInt(curProcs_[i]->pid, specPid)) {
                return false;
            }
            if (pid == specPid) {
                CopyProcInfo(specProc, curProcs_[i]);
                return true;
            }
        }
    } else {
        for (size_t i = 0; i < oldProcs_.size(); i++) {
            if (oldProcs_[i] == nullptr) {
                continue;
            }
            if (!IsNumericStr(oldProcs_[i]->pid)) {
                return false;
            }
            int specPid = 0;
            if (!StrToInt(oldProcs_[i]->pid, specPid)) {
                return false;
            }
            if (pid == specPid) {
                CopyProcInfo(specProc, oldProcs_[i]);
                return true;
            }
        }
    }
    return false;
}

void DumpCpuInfoUtil::CopyCpuInfo(std::shared_ptr<CPUInfo> &tar, const std::shared_ptr<CPUInfo> &source)
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

void DumpCpuInfoUtil::CopyProcInfo(std::shared_ptr<ProcInfo> &tar, const std::shared_ptr<ProcInfo> &source)
{
    if ((tar == nullptr) || (source == nullptr)) {
        return;
    }

    tar->pid = source->pid;
    tar->comm = source->comm;
    tar->uTime = source->uTime;
    tar->sTime = source->sTime;
    tar->minflt = source->minflt;
    tar->majflt = source->majflt;
}

bool DumpCpuInfoUtil::CheckFrequentDumpping()
{
    time_t curTime;
    struct tm ptrCurtime = {0};
    (void)time(&curTime);
    gmtime_r(&curTime, &ptrCurtime);
    int interval = ptrCurtime.tm_sec - dumpTimeSec_;
    if (interval > 0 && interval < DUMP_TIME_INTERVAL) {
        return true;
    }
    dumpTimeSec_ = ptrCurtime.tm_sec;
    return false;
}

float DumpCpuInfoUtil::GetCpuUsage(const int pid)
{
    std::shared_ptr<CPUInfo> oldCPUInfo = std::make_shared<CPUInfo>();
    if (!GetCurCPUInfo(oldCPUInfo)) {
        return false;
    }
    std::shared_ptr<ProcInfo> oldSpecProc = std::make_shared<ProcInfo>();
    if (!GetCurSpecProcInfo(pid, oldSpecProc)) {
        return false;
    }

    sleep(1);

    std::shared_ptr<CPUInfo> curCPUInfo = std::make_shared<CPUInfo>();
    if (!GetCurCPUInfo(curCPUInfo)) {
        return false;
    }
    std::shared_ptr<ProcInfo> curSpecProc = std::make_shared<ProcInfo>();
    if (!GetCurSpecProcInfo(pid, curSpecProc)) {
        return false;
    }

    long unsigned totalDeltaTime = (curCPUInfo->uTime + curCPUInfo->nTime + curCPUInfo->sTime + curCPUInfo->iTime
                                    + curCPUInfo->iowTime + curCPUInfo->irqTime + curCPUInfo->sirqTime)
                                   - (oldCPUInfo->uTime + oldCPUInfo->nTime + oldCPUInfo->sTime + oldCPUInfo->iTime
                                      + oldCPUInfo->iowTime + oldCPUInfo->irqTime + oldCPUInfo->sirqTime);

    if (totalDeltaTime != 0) {
        float cpuUsage = static_cast<float>((curSpecProc->uTime - oldSpecProc->uTime)
               + (curSpecProc->sTime - oldSpecProc->sTime)) / static_cast<float>(totalDeltaTime);
        return cpuUsage;
    } else {
        return 0;
    }
}
} // namespace HiviewDFX
} // namespace OHOS
