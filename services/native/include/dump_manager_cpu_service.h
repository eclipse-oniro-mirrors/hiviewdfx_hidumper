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
#include "util/dump_cpu_info_util.h"
#include "delayed_sp_singleton.h"
#include "dump_common_utils.h"
#include "dump_broker_cpu_stub.h"
#include "dump_event_handler.h"
#include "dump_cpu_data.h"
#include "common.h"
#ifdef HIDUMPER_BATTERY_ENABLE
#include "common_event_subscriber.h"
#endif

namespace OHOS {
namespace HiviewDFX {
class DumpCpuData;
class DumpManagerCpuService final : public SystemAbility, public DumpBrokerCpuStub {
    DECLARE_SYSTEM_ABILITY(DumpManagerCpuService)
public:
    DumpManagerCpuService();
    ~DumpManagerCpuService();
public:
    virtual void OnStart() override;
    virtual void OnStop() override;
public:
    // Used for dump request
    int32_t Request(DumpCpuData &dumpCpuData) override;
    int32_t DumpCpuUsageData();
    void InitParam(DumpCpuData &dumpCpuData);
    void ResetParam();
    bool SendImmediateEvent();
    void StartService();
public:
    std::shared_ptr<DumpEventHandler> GetHandler();
private:
    friend DumpDelayedSpSingleton<DumpManagerCpuService>;
private:
    bool Init();
    void EventHandlerInit();
    DumpStatus ReadLoadAvgInfo(const std::string& filePath, std::string& info);
    void CreateDumpTimeString(const std::string& startTime, const std::string& endTime,
        std::string& timeStr);
    void AddStrLineToDumpInfo(const std::string& strLine);
    void CreateCPUStatString(std::string& str);
    std::shared_ptr<ProcInfo> GetOldProc(const std::string& pid);
    void DumpProcInfo();
    static bool SortProcInfo(std::shared_ptr<ProcInfo> &left, std::shared_ptr<ProcInfo> &right);
    bool SubscribeAppStateEvent();
    bool SubscribeCommonEvent();
    bool GetProcCPUInfo();
private:
    using StringMatrix = std::shared_ptr<std::vector<std::vector<std::string>>>;
    std::shared_ptr<AppExecFwk::EventRunner> eventRunner_{nullptr};
    std::shared_ptr<DumpEventHandler> handler_{nullptr};
    bool started_{false};
    bool registered_{false};
    std::string startTime_;
    std::string endTime_;
    std::shared_ptr<CPUInfo> curCPUInfo_{nullptr};
    std::shared_ptr<CPUInfo> oldCPUInfo_{nullptr};
    std::shared_ptr<ProcInfo> curSpecProc_{nullptr};
    std::shared_ptr<ProcInfo> oldSpecProc_{nullptr};
    std::vector<std::shared_ptr<ProcInfo>> curProcs_;
    std::vector<std::shared_ptr<ProcInfo>> oldProcs_;
    int cpuUsagePid_{-1};
    StringMatrix dumpCPUDatas_{nullptr};
#ifdef HIDUMPER_BATTERY_ENABLE
    std::shared_ptr<EventFwk::CommonEventSubscriber> subscriberPtr_{nullptr};
#endif

class SystemAbilityStatusChangeListener : public OHOS::SystemAbilityStatusChangeStub {
public:
    explicit SystemAbilityStatusChangeListener() {};
    ~SystemAbilityStatusChangeListener() override = default;

    virtual void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
    virtual void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
};
    sptr<SystemAbilityStatusChangeListener> sysAbilityListener_{nullptr};
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIDUMPER_SERVICES_CPU_MANAGER_SERVICE_H
