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
#ifndef HIDUMPER_SERVICES_DUMPER_OPTS_H
#define HIDUMPER_SERVICES_DUMPER_OPTS_H
#include <string>
#include <vector>
namespace OHOS {
namespace HiviewDFX {
struct DumperOpts {
    bool isDumpCpuFreq_;
    bool isDumpCpuUsage_;
    int cpuUsagePid_;
    bool isDumpLog_;
    std::vector<std::string> logArgs_;
    bool isDumpMem_;
    int memPid_;
    bool isDumpStorage_;
    int storagePid_;
    bool isDumpNet_;
    int netPid_;
    bool isDumpList_;
    bool isDumpService_;
    bool isDumpSystemAbility_;
    std::vector<std::string> abilitieNames_;
    std::vector<std::string> abilitieArgs_;
    bool isDumpSystem_;
    std::vector<std::string> systemArgs_;
    bool isDumpProcesses_;
    int processPid_;
    std::string eventId_;
    int showEventCount_;
    std::string processName_;
    long long startTime_;
    long long endTime_;
    bool isFaultLog_;
    std::string path_; // for zip
    bool isAppendix_;
    bool isShowSmaps_;
    bool isShowSmapsInfo_;
    bool isDumpJsHeapMem_;
    bool isDumpJsHeapMemGC_;
    bool isDumpJsHeapLeakobj_;
    int dumpJsHeapMemPid_;
    int threadId_;
    int ipcStatPid_;
    bool isDumpAllIpc_;
    bool isDumpIpc_;
    bool isDumpIpcStartStat_;
    bool isDumpIpcStopStat_;
    bool isDumpIpcStat_;
    bool dumpJsRawHeap_;
    bool dumpMemPrune_;
    bool isDumpCjHeapMem_;
    bool isDumpCjHeapMemGC_;
    int dumpCjHeapMemPid_;
    int timeInterval_;
    bool isReceivedSigInt_;
    bool showAshmem_;
    bool showDmaBuf_;
    bool isEventList_;
    bool isEventDetail_;

public:
    DumperOpts();
    void Reset();
    DumperOpts& operator = (const DumperOpts& opts);
    void AddSelectAll();
    bool IsSelectAny() const;
    bool IsDumpZip() const;
    bool CheckRemainingOptions(std::string& errStr) const;
    bool CheckOptions(std::string& errStr) const;
private:
    void ResetCpuOptions();
    void ResetMemOptions();
    void ResetStorageOptions();
    void ResetNetOptions();
    void ResetSystemAbilityOptions();
    void ResetSystemOptions();
    void ResetProcessOptions();
    void ResetEventOptions();
    void ResetIpcOptions();
    void ResetJsHeapOptions();
    void ResetMiscOptions();
    void AssignCpuOptions(const DumperOpts& opts);
    void AssignMemOptions(const DumperOpts& opts);
    void AssignStorageOptions(const DumperOpts& opts);
    void AssignListServiceOptions(const DumperOpts& opts);
    void AssignSystemAbilityOptions(const DumperOpts& opts);
    void AssignSystemOptions(const DumperOpts& opts);
    void AssignProcessOptions(const DumperOpts& opts);
    void AssignEventOptions(const DumperOpts& opts);
    void AssignJsHeapOptions(const DumperOpts& opts);
    void AssignIpcOptions(const DumperOpts& opts);
    void AssignMiscOptions(const DumperOpts& opts);
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIDUMPER_SERVICES_DUMPER_OPTS_H
