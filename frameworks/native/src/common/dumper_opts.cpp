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
#include "common/dumper_opts.h"

#include <algorithm>
#include <string>
#include <vector>

#include "dump_common_utils.h"
#include "dump_controller.h"
#include "dump_utils.h"
#include "hilog_wrapper.h"
#include "string_ex.h"
#include "util/config_data.h"
#include "util/config_utils.h"
namespace OHOS {
namespace HiviewDFX {
namespace {
static const std::string PATH_SEPARATOR = "/";
}

DumperOpts::DumperOpts()
{
    Reset();
}

void DumperOpts::Reset()
{
    ResetCpuOptions();
    ResetMemOptions();
    ResetStorageOptions();
    ResetNetOptions();
    ResetSystemAbilityOptions();
    ResetSystemOptions();
    ResetProcessOptions();
    ResetEventOptions();
    ResetIpcOptions();
    ResetJsHeapOptions();
    ResetMiscOptions();
}

void DumperOpts::ResetCpuOptions()
{
    isDumpCpuFreq_ = false;
    isDumpCpuUsage_ = false;
    cpuUsagePid_ = -1;
}

void DumperOpts::ResetMemOptions()
{
    isDumpMem_ = false;
    isReceivedSigInt_ = false;
    dumpMemPrune_ = false;
    showAshmem_ = false;
    showDmaBuf_ = false;
    timeInterval_ = 0;
    memPid_ = -1;
}

void DumperOpts::ResetStorageOptions()
{
    isDumpStorage_ = false;
    storagePid_ = -1;
}

void DumperOpts::ResetNetOptions()
{
    isDumpNet_ = false;
    netPid_ = -1;
}

void DumperOpts::ResetSystemAbilityOptions()
{
    isDumpSystemAbility_ = false;
    abilitieNames_.clear();
    abilitieArgs_.clear();
}

void DumperOpts::ResetSystemOptions()
{
    isDumpSystem_ = false;
    systemArgs_.clear();
}

void DumperOpts::ResetProcessOptions()
{
    isDumpProcesses_ = false;
    processPid_ = -1;
    processName_.clear();
}

void DumperOpts::ResetEventOptions()
{
    isEventList_ = false;
    isEventDetail_ = false;
    eventId_.clear();
    showEventCount_ = -1;
    startTime_ = -1;
    endTime_ = -1;
}

void DumperOpts::ResetIpcOptions()
{
    ipcStatPid_ = -1;
    isDumpAllIpc_ = false;
    isDumpIpc_ = false;
    isDumpIpcStartStat_ = false;
    isDumpIpcStopStat_ = false;
    isDumpIpcStat_ = false;
}

void DumperOpts::ResetJsHeapOptions()
{
    isDumpJsHeapMem_ = false;
    isDumpJsHeapMemGC_ = false;
    isDumpJsHeapLeakobj_ = false;
    isDumpCjHeapMem_ = false;
    isDumpCjHeapMemGC_ = false;
    dumpJsHeapMemPid_ = 0;
    dumpCjHeapMemPid_ = 0;
    dumpJsRawHeap_ = false;
}

void DumperOpts::ResetMiscOptions()
{
    isDumpLog_ = false;
    logArgs_.clear();
    isDumpList_ = false;
    isDumpService_ = false;
    isFaultLog_ = false;
    path_.clear(); // for zip
    isAppendix_ = false;
    isShowSmaps_ = false;
    isShowSmapsInfo_ = false;
    threadId_ = 0;
}

DumperOpts& DumperOpts::operator=(const DumperOpts& opts)
{
    Reset();

    AssignCpuOptions(opts);
    AssignMemOptions(opts);
    AssignStorageOptions(opts);
    AssignListServiceOptions(opts);
    AssignSystemAbilityOptions(opts);
    AssignSystemOptions(opts);
    AssignProcessOptions(opts);
    AssignEventOptions(opts);
    AssignJsHeapOptions(opts);
    AssignIpcOptions(opts);
    AssignMiscOptions(opts);

    return *this;
}

void DumperOpts::AssignCpuOptions(const DumperOpts& opts)
{
    isDumpCpuFreq_ = opts.isDumpCpuFreq_;
    isDumpCpuUsage_ = opts.isDumpCpuUsage_;
    cpuUsagePid_ = opts.cpuUsagePid_;
}

void DumperOpts::AssignMemOptions(const DumperOpts& opts)
{
    isDumpMem_ = opts.isDumpMem_;
    isReceivedSigInt_ = opts.isReceivedSigInt_;
    timeInterval_ = opts.timeInterval_;
    memPid_ = opts.memPid_;
    dumpMemPrune_ = opts.dumpMemPrune_;
    showAshmem_ = opts.showAshmem_;
    showDmaBuf_ = opts.showDmaBuf_;
    isShowSmaps_ = opts.isShowSmaps_;
    isShowSmapsInfo_ = opts.isShowSmapsInfo_;
}

void DumperOpts::AssignStorageOptions(const DumperOpts& opts)
{
    isDumpStorage_ = opts.isDumpStorage_;
    storagePid_ = opts.storagePid_;
}

void DumperOpts::AssignListServiceOptions(const DumperOpts& opts)
{
    isDumpList_ = opts.isDumpList_;
    isDumpService_ = opts.isDumpService_;
}

void DumperOpts::AssignSystemAbilityOptions(const DumperOpts& opts)
{
    isDumpSystemAbility_ = opts.isDumpSystemAbility_;
    abilitieNames_ = opts.abilitieNames_;
    abilitieArgs_ = opts.abilitieArgs_;
}

void DumperOpts::AssignSystemOptions(const DumperOpts& opts)
{
    isDumpSystem_ = opts.isDumpSystem_;
    systemArgs_ = opts.systemArgs_;
}

void DumperOpts::AssignProcessOptions(const DumperOpts& opts)
{
    isDumpProcesses_ = opts.isDumpProcesses_;
    processPid_ = opts.processPid_;
    processName_ = opts.processName_;
}

void DumperOpts::AssignEventOptions(const DumperOpts& opts)
{
    eventId_ = opts.eventId_;
    showEventCount_ = opts.showEventCount_;
    isEventList_ = opts.isEventList_;
    isEventDetail_ = opts.isEventDetail_;
    startTime_ = opts.startTime_;
    endTime_ = opts.endTime_;
}

void DumperOpts::AssignJsHeapOptions(const DumperOpts& opts)
{
    isDumpJsHeapMem_ = opts.isDumpJsHeapMem_;
    isDumpJsHeapMemGC_ = opts.isDumpJsHeapMemGC_;
    isDumpJsHeapLeakobj_ = opts.isDumpJsHeapLeakobj_;
    isDumpCjHeapMem_ = opts.isDumpCjHeapMem_;
    isDumpCjHeapMemGC_ = opts.isDumpCjHeapMemGC_;
    dumpJsHeapMemPid_ = opts.dumpJsHeapMemPid_;
    dumpCjHeapMemPid_ = opts.dumpCjHeapMemPid_;
    dumpJsRawHeap_ = opts.dumpJsRawHeap_;
}

void DumperOpts::AssignIpcOptions(const DumperOpts& opts)
{
    ipcStatPid_ = opts.ipcStatPid_;
    isDumpAllIpc_ = opts.isDumpAllIpc_;
    isDumpIpc_ = opts.isDumpIpc_;
    isDumpIpcStartStat_ = opts.isDumpIpcStartStat_;
    isDumpIpcStopStat_ = opts.isDumpIpcStopStat_;
    isDumpIpcStat_ = opts.isDumpIpcStat_;
}

void DumperOpts::AssignMiscOptions(const DumperOpts& opts)
{
    isDumpNet_ = opts.isDumpNet_;
    netPid_ = opts.netPid_;
    isDumpLog_ = opts.isDumpLog_;
    logArgs_ = opts.logArgs_;
    isDumpList_ = opts.isDumpList_;
    isDumpService_ = opts.isDumpService_;
    isFaultLog_ = opts.isFaultLog_;
    path_ = opts.path_;
    isAppendix_ = opts.isAppendix_;
    threadId_ = opts.threadId_;
}

void DumperOpts::AddSelectAll()
{
    isDumpCpuFreq_ = true;
    isDumpCpuUsage_ = true;
    isDumpLog_ = true;
    isDumpMem_ = true;
    isDumpStorage_ = true;
    isDumpNet_ = true;
    isDumpService_ = true;
    isDumpSystemAbility_ = true;
    isDumpSystem_ = true;
    isDumpProcesses_ = true;
    isFaultLog_ = true;
    isAppendix_ = true;
}

bool DumperOpts::IsDumpZip() const
{
    return DumpCommonUtils::StartWith(path_, PATH_SEPARATOR);
}

bool DumperOpts::IsSelectAny() const
{
    if (isDumpCpuFreq_ || isDumpCpuUsage_) {
        return true;
    }
    if (isDumpLog_ || isFaultLog_) {
        return true;
    }
    if (isDumpMem_) {
        return true;
    }
    if (isDumpStorage_) {
        return true;
    }
    if (isDumpNet_) {
        return true;
    }
    if (isDumpService_ || isDumpSystemAbility_ || isDumpSystem_) {
        return true;
    }
    if (isDumpProcesses_) {
        return true;
    }
    if (isShowSmaps_) {
        return true;
    }
    if (isDumpJsHeapMem_) {
        return true;
    }
    if (isDumpCjHeapMem_) {
        return true;
    }
    if (isDumpIpc_) {
        return true;
    }
    DUMPER_HILOGE(MODULE_COMMON, "select nothing.");
    return false;
}

bool DumperOpts::CheckRemainingOptions(std::string& errStr) const
{
    if (processPid_ < -1) {
        errStr = std::to_string(processPid_);
        return false;
    }
    if (storagePid_ < -1) {
        errStr = std::to_string(storagePid_);
        return false;
    }
    if (netPid_ < -1) {
        errStr = std::to_string(netPid_);
        return false;
    }
    if (dumpJsHeapMemPid_ < 0) {
        errStr = std::to_string(dumpJsHeapMemPid_);
        return false;
    }
    if (dumpCjHeapMemPid_ < 0) {
        errStr = std::to_string(dumpCjHeapMemPid_);
        return false;
    }
    if (threadId_ < 0) {
        errStr = std::to_string(threadId_);
        return false;
    }
    if (timeInterval_ < 0) {
        errStr = std::to_string(timeInterval_);
        return false;
    }
    if (ipcStatPid_ < -1) {
        errStr = std::to_string(ipcStatPid_);
        return false;
    }
    return true;
}

bool DumperOpts::CheckOptions(std::string& errStr) const
{
    if (cpuUsagePid_ < -1) {
        errStr = std::to_string(cpuUsagePid_);
        return false;
    }
    if (memPid_ < -1) {
        errStr = std::to_string(memPid_);
        return false;
    }
    if (isDumpList_ && ((!isDumpService_) && (!isDumpSystemAbility_) && (!isDumpSystem_))) {
        errStr = "-l";
        return false;
    }
    if (isEventDetail_ && isEventList_) {
        errStr = "--list and --detail cannot be used together";
        return false;
    }
    std::string path = TrimStr(path_);
    if ((!path.empty()) && (!DumpCommonUtils::StartWith(path, PATH_SEPARATOR))) {
        errStr = path_;
        return false;
    }
    for (size_t i = 0; i < abilitieNames_.size(); i++) {
        if (DumpUtils::StrToId(abilitieNames_[i]) == -1) {
            errStr = abilitieNames_[i];
            return false;
        }
    }
    std::vector<std::string> systemList;
    ConfigUtils::GetSectionNames(ConfigUtils::CONFIG_GROUP_SYSTEM_, systemList);
    for (size_t i = 0; i < systemArgs_.size(); i++) {
        if (std::find(systemList.begin(), systemList.end(), systemArgs_[i]) == systemList.end()) {
            errStr = systemArgs_[i];
            return false;
        }
    }
    return CheckRemainingOptions(errStr);
}
} // namespace HiviewDFX
} // namespace OHOS
