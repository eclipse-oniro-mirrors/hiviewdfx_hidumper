/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#ifndef HIDUMPER_SERVICES_CPU_MANAGER_SERVICE_H
#define HIDUMPER_SERVICES_CPU_MANAGER_SERVICE_H
#include <map>
#include <vector>
#include <system_ability.h>
#include "system_ability_status_change_stub.h"
#include "delayed_sp_singleton.h"
#include "dump_common_utils.h"
#include "dump_broker_cpu_stub.h"
#include "dump_cpu_data.h"
#include "common.h"

namespace OHOS {
namespace HiviewDFX {
struct CPUInfo {
    double userUsage; // user space usage
    double niceUsage; // adjust process priority cpu usage
    double systemUsage; // kernel space cpu usage
    double idleUsage; // idle cpu usage
    double ioWaitUsage; // io wait cpu usage
    double irqUsage; // hard interrupt cpu usage
    double softIrqUsage; // soft interrupt cpu usage
};

struct ProcInfo {
    double userSpaceUsage;
    double sysSpaceUsage;
    double totalUsage;
    std::string pid;
    std::string comm;
    std::string minflt;
    std::string majflt;
};
class DumpCpuData;
class DumpManagerCpuService final : public SystemAbility, public DumpBrokerCpuStub {
    DECLARE_SYSTEM_ABILITY(DumpManagerCpuService)
public:
    DumpManagerCpuService();
    ~DumpManagerCpuService();
public:
    virtual void OnStart() override;
public:
    // Used for dump request
    int32_t Request(DumpCpuData &dumpCpuData) override;
    int32_t GetCpuUsageByPid(int32_t pid, double &cpuUsage) override;
    int32_t DumpCpuUsageData();
    void InitParam(DumpCpuData &dumpCpuData);
    void ResetParam();
    bool GetSingleProcInfo(int pid, std::shared_ptr<ProcInfo> &specProc);
    void StartService();
private:
    friend DumpDelayedSpSingleton<DumpManagerCpuService>;
private:
    DumpStatus ReadLoadAvgInfo(std::string& info);
    void CreateDumpTimeString(const std::string& startTime, const std::string& endTime,
        std::string& timeStr);
    void AddStrLineToDumpInfo(const std::string& strLine);
    void CreateCPUStatString(std::string& str);
    void DumpProcInfo();
    static bool SortProcInfo(std::shared_ptr<ProcInfo> &left, std::shared_ptr<ProcInfo> &right);
    bool SubscribeAppStateEvent();
    bool SubscribeCommonEvent();

    bool GetSysCPUInfo(std::shared_ptr<CPUInfo> &cpuInfo);
    bool GetAllProcInfo(std::vector<std::shared_ptr<ProcInfo>> &procInfos);
    bool GetDateAndTime(uint64_t timeStamp, std::string& dateTime);
    bool HasDumpPermission() const;
private:
    using StringMatrix = std::shared_ptr<std::vector<std::vector<std::string>>>;
    bool started_{false};
    bool registered_{false};
    uint64_t startTime_{0};
    uint64_t endTime_{0};
    std::shared_ptr<CPUInfo> curCPUInfo_{nullptr};
    std::shared_ptr<ProcInfo> curSpecProc_{nullptr};
    std::vector<std::shared_ptr<ProcInfo>> curProcs_;
    int cpuUsagePid_{-1};
    StringMatrix dumpCPUDatas_{nullptr};
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIDUMPER_SERVICES_CPU_MANAGER_SERVICE_H
