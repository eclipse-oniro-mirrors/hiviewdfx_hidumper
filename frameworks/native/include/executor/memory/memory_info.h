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
#ifndef MEMORY_INFO_H
#define MEMORY_INFO_H
#include <memory>
#include <string>
#include <vector>
#include "executor/memory/parse/meminfo_data.h"
#include "common.h"
#include "time.h"
namespace OHOS {
namespace HiviewDFX {
class MemoryInfo {
public:
    MemoryInfo();
    ~MemoryInfo();

    using StringMatrix = std::shared_ptr<std::vector<std::vector<std::string>>>;
    using PairMatrix = std::vector<std::pair<std::string, uint64_t>>;
    using PairMatrixGroup = std::vector<std::pair<std::string, PairMatrix>>;

    bool GetMemoryInfoByPid(const int &pid, StringMatrix result);
    DumpStatus GetMemoryInfoNoPid(StringMatrix result);

private:
    enum Status {
        SUCCESS_MORE_DATA = 1,
        FAIL_MORE_DATA = 2,
        SUCCESS_NO_MORE_DATA = 3,
        FAIL_NO_MORE_DATA = 4,
    };
    struct MemProcessData {
        PairMatrixGroup smapsInfo;
        MemInfoData::MemUsage usage;
        int threadId;
        int pid;
        bool usageSuccess;
        bool smapsSuccess;
    };

    const int LINE_WIDTH_ = 14;
    const int RAM_WIDTH_ = 16;
    const size_t TYPE_SIZE = 2;
    const char SEPARATOR_ = '-';
    const char BLANK_ = ' ';
    const static int NAME_SIZE_ = 2;
    const int PID_WIDTH_ = 5;
    const int NAME_WIDTH_ = 20;
    const int KB_WIDTH_ = 12;
    const static int VSS_BIT = 4;
    bool getPidDone_ = false;
    bool pidSuccess_ = false;
    bool memProcessDone_ = false;
    bool addMemProcessTitle_ = false;
    std::vector<int> pids_;
    std::vector<MemInfoData::MemUsage> memUsages_;

    PairMatrixGroup smapsResult_;
    void insertMemoryTitle(StringMatrix result);
    void BuildResult(const PairMatrixGroup &infos, StringMatrix result);

    std::string AddKbUnit(const uint64_t &value);
    bool static GetMemByProcessPid(const int &pid, MemInfoData::MemUsage &usage);
    MemProcessData static GetMemProcess(const std::vector<int> &pids, const int &threadId);
    void GetMemProcessGroup(const std::vector<int> &pids, PairMatrixGroup &result,
                            std::vector<MemInfoData::MemUsage> &memInfos);
    bool static GetSmapsInfoNoPid(const int &pid, PairMatrixGroup &result);
    bool GetMeminfo(PairMatrix &result);
    bool GetHardWareUsage(StringMatrix result);
    bool GetCMAUsage(StringMatrix result);
    bool GetKernelUsage(const PairMatrix &infos, StringMatrix result);
    void GetProcesses(const PairMatrixGroup &infos, StringMatrix result);
    bool GetPids();
    void GetGroupOfPids(const int &index, const int &size, const std::vector<int> &pids, std::vector<int> &groupPids);
    void GetPssTotal(const PairMatrixGroup &infos, StringMatrix result);
    void GetRamUsage(const PairMatrixGroup &smapsinfos, const PairMatrix &meminfo, StringMatrix result);
    void GetRamCategory(const PairMatrixGroup &smapsinfos, const PairMatrix &meminfos, StringMatrix result);
    void AddBlankLine(StringMatrix result);
    void MemUsageToMatrix(const std::vector<MemInfoData::MemUsage> &memInfos, StringMatrix result);
    void DeletePid(std::vector<int> &pids, const int &pid);
    void AddMemByProcessTitle(StringMatrix result, std::string sortType);
    bool static GetVss(const int &pid, uint64_t &value);
    bool static GetProcName(const int &pid, std::string &name);
    void static InitMemInfo(MemInfoData::MemInfo &memInfo);
    void static InitMemUsage(MemInfoData::MemUsage &usage);
    void CalcGroup(const PairMatrixGroup &infos, StringMatrix result);
    void SetValue(const std::string &value, std::vector<std::string> &lines, std::vector<std::string> &values);
    void GetSortedMemoryInfoNoPid(StringMatrix result);
};
} // namespace HiviewDFX
} // namespace OHOS
#endif
