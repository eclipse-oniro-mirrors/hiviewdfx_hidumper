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
#ifndef CPU_DUMPER_H
#define CPU_DUMPER_H

#include "hidumper_executor.h"

namespace OHOS {
namespace HiviewDFX {
struct CPUInfo {
    long unsigned uTime; // user space cpu time
    long unsigned nTime; // adjust process priority cpu time
    long unsigned sTime; // kernel space cpu time
    long unsigned iTime; // idle cpu time
    long unsigned iowTime; // io wait cpu time
    long unsigned irqTime; // hard interrupt cpu time
    long unsigned sirqTime; // soft interrupt cpu time
};

struct ProcInfo {
    std::shared_ptr<ProcInfo> next;
    std::string pid;
    std::string comm;
    long unsigned uTime;
    long unsigned sTime;
    long unsigned userSpaceUsage;
    long unsigned sysSpaceUsage;
    long unsigned totalUsage;
    std::string minflt;
    std::string majflt;
};

class CPUDumper : public HidumperExecutor {
public:
    CPUDumper();
    ~CPUDumper();
    DumpStatus PreExecute(const std::shared_ptr<DumperParameter>& parameter,
        StringMatrix dumpDatas) override;
    DumpStatus Execute() override;
    DumpStatus AfterExecute() override;
    float GetCpuUsage(int pid);

private:
    DumpStatus DumpCpuUsageData();
    DumpStatus ReadCPUInfo();
    DumpStatus ReadProcInfo();
    DumpStatus ReadSpecProcInfo(int pid);
    DumpStatus ReadLoadAvgInfo(const std::string& filePath, std::string& info);
    bool GetDateAndTime(std::string& dateTime);
    void CreateDumpTimeString(const std::string& startTime, const std::string& endTime,
        std::string& timeStr);
    void SetCPUInfo(long unsigned& info, const std::string& strInfo);
    void AddStrLineToDumpInfo(const std::string& strLine);
    void CreateCPUStatString(std::string& str);
    std::shared_ptr<ProcInfo> GetOldProc(const std::string& pid);
    void GetProcessDirFiles(const std::string& path, const std::string& file,
        std::vector<std::string>& files);
    void DumpProcInfo();
    static bool SortProcInfo(std::shared_ptr<ProcInfo> &left, std::shared_ptr<ProcInfo> &right);

private:
    static const std::string LOAD_AVG_FILE_PATH;
    static const size_t LOAD_AVG_INFO_COUNT;
    static const int TM_START_YEAR;
    static const std::string PROC_STAT_FILE_PATH;
    static const useconds_t DUMP_DELAY_MICROSECOND;
    static const int DEC_SYSTEM_VALUE;
    static const int PROC_CPU_LENGTH;
    static const int CPU_STAT_USER_TIME_INDEX;
    static const int CPU_STAT_NICE_TIME_INDEX;
    static const int CPU_STAT_SYS_TIME_INDEX;
    static const int CPU_STAT_IDLE_TIME_INDEX;
    static const int CPU_STAT_IOW_TIME_INDEX;
    static const int CPU_STAT_IRQ_TIME_INDEX;
    static const int CPU_STAT_SIRQ_TIME_INDEX;
    static const long unsigned HUNDRED_PERCENT_VALUE;
    static const int PROC_INFO_USER_TIME_INDEX;
    static const int PROC_INFO_SYS_TIME_INDEX;
    static const int PROC_INFO_MINOR_FAULT_INDEX;
    static const int PROC_INFO_MAJOR_FAULT_INDEX;

    StringMatrix dumpCPUDatas_;
    bool isDumpCpuUsage_ = false;
    int cpuUsagePid_ = -1;
    std::shared_ptr<CPUInfo> curCPUInfo_;
    std::shared_ptr<CPUInfo> oldCPUInfo_;
    std::vector<std::shared_ptr<ProcInfo>> curProcs_;
    std::vector<std::shared_ptr<ProcInfo>> oldProcs_;
    std::shared_ptr<ProcInfo> curSpecProc_;
    std::shared_ptr<ProcInfo> oldSpecProc_;
    std::string startTime_;
    std::string endTime_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // CPU_DUMPER_H
