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
#ifndef HIDUMPER_UTILS_DUMP_CPU_INFO_H
#define HIDUMPER_UTILS_DUMP_CPU_INFO_H
#include <string>
#include <vector>
#include "singleton.h"
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
    long unsigned uTime;
    long unsigned sTime;
    long unsigned userSpaceUsage;
    long unsigned sysSpaceUsage;
    long unsigned totalUsage;
    std::string pid;
    std::string comm;
    std::string minflt;
    std::string majflt;
};

class DumpCpuInfoUtil : public Singleton<DumpCpuInfoUtil> {
public:
    DumpCpuInfoUtil();
    ~DumpCpuInfoUtil();
    void UpdateCpuInfo();
    float GetCpuUsage(const int pid);
    bool GetCurCPUInfo(std::shared_ptr<CPUInfo> &cpuInfo);
    bool GetCurProcInfo(std::vector<std::shared_ptr<ProcInfo>> &procInfos);
    bool GetCurSpecProcInfo(int pid, std::shared_ptr<ProcInfo> &specProc);
    bool GetOldCPUInfo(std::shared_ptr<CPUInfo> &cpuInfo);
    bool GetOldProcInfo(std::vector<std::shared_ptr<ProcInfo>> &procInfos);
    bool GetOldSpecProcInfo(int pid, std::shared_ptr<ProcInfo> &specProc);

private:
    void SetCPUInfo(long unsigned& info, const std::string& strInfo);
    void GetProcessDirFiles(const std::string& path, const std::string& file,
        std::vector<std::string>& files);
    void CopyCpuInfo(std::shared_ptr<CPUInfo> &tar, const std::shared_ptr<CPUInfo> &source);
    void CopyProcInfo(std::shared_ptr<ProcInfo> &tar, const std::shared_ptr<ProcInfo> &source);
    bool CheckFrequentDumpping();

private:
    static const std::string LOAD_AVG_FILE_PATH;
    static const size_t LOAD_AVG_INFO_COUNT = 3;
    static const std::string PROC_STAT_FILE_PATH;
    static const std::string SPACE;
    static const int CPU_STAT_USER_TIME_INDEX = 1;
    static const int CPU_STAT_NICE_TIME_INDEX = 2;
    static const int CPU_STAT_SYS_TIME_INDEX = 3;
    static const int CPU_STAT_IDLE_TIME_INDEX = 4;
    static const int CPU_STAT_IOW_TIME_INDEX = 5;
    static const int CPU_STAT_IRQ_TIME_INDEX = 6;
    static const int CPU_STAT_SIRQ_TIME_INDEX = 7;
    static const int PROC_INFO_USER_TIME_INDEX = 13;
    static const int PROC_INFO_SYS_TIME_INDEX = 14;
    static const int PROC_INFO_MINOR_FAULT_INDEX = 9;
    static const int PROC_INFO_MAJOR_FAULT_INDEX = 11;
    static const int CONSTANT_NUM_10 = 10;
    static const int DUMP_TIME_INTERVAL = 5;

    std::shared_ptr<CPUInfo> curCPUInfo_;
    std::shared_ptr<CPUInfo> oldCPUInfo_;
    std::vector<std::shared_ptr<ProcInfo>> curProcs_;
    std::vector<std::shared_ptr<ProcInfo>> oldProcs_;
    int dumpTimeSec_ = 0;
    std::mutex mutex_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIDUMPER_UTILS_DUMP_CPU_INFO_H
