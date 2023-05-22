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
#ifndef CPU_DUMPER_H
#define CPU_DUMPER_H
#include "util/dump_cpu_info_util.h"
#include "hidumper_executor.h"

namespace OHOS {
namespace HiviewDFX {
class CPUDumper : public HidumperExecutor {
public:
    CPUDumper();
    ~CPUDumper();
    DumpStatus PreExecute(const std::shared_ptr<DumperParameter>& parameter,
        StringMatrix dumpDatas) override;
    DumpStatus Execute() override;
    DumpStatus AfterExecute() override;

private:
    DumpStatus DumpCpuUsageData();
    DumpStatus ReadLoadAvgInfo(const std::string& filePath, std::string& info);
    bool GetDateAndTime(std::string& dateTime);
    void CreateDumpTimeString(const std::string& startTime, const std::string& endTime,
        std::string& timeStr);
    void AddStrLineToDumpInfo(const std::string& strLine);
    void CreateCPUStatString(std::string& str);
    std::shared_ptr<ProcInfo> GetOldProc(const std::string& pid);
    void DumpProcInfo();
    static bool SortProcInfo(std::shared_ptr<ProcInfo> &left, std::shared_ptr<ProcInfo> &right);
    void GetInitOldCPUInfo(std::shared_ptr<CPUInfo> tar, const std::shared_ptr<CPUInfo> source);
    bool GetProcCPUInfo();

private:
    static const std::string LOAD_AVG_FILE_PATH;
    static const size_t LOAD_AVG_INFO_COUNT;
    static const int TM_START_YEAR;
    static const int DEC_SYSTEM_VALUE;
    static const int PROC_CPU_LENGTH;
    static const long unsigned HUNDRED_PERCENT_VALUE;
    static const long unsigned DELAY_VALUE = 500000;

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
