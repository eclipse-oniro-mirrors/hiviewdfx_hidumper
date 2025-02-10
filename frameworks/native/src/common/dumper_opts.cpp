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
    isDumpCpuFreq_ = false;
    isDumpCpuUsage_ = false;
    cpuUsagePid_ = -1;
    isDumpLog_ = false;
    logArgs_.clear();
    isDumpMem_ = false;
    memPid_ = -1;
    isDumpStorage_ = false;
    storagePid_ = -1;
    isDumpNet_ = false;
    netPid_ = -1;
    isDumpList_ = false;
    isDumpService_ = false;
    isDumpSystemAbility_ = false;
    abilitieNames_.clear();
    abilitieArgs_.clear();
    isDumpSystem_ = false;
    systemArgs_.clear();
    isDumpProcesses_ = false;
    processPid_ = -1;
    isFaultLog_ = false;
    path_.clear(); // for zip
    isAppendix_ = false;
    isShowSmaps_ = false;
    isShowSmapsInfo_ = false;
    isDumpJsHeapMem_ = false;
    isDumpJsHeapMemGC_ = false;
    isDumpJsHeapLeakobj_ = false;
    dumpJsHeapMemPid_ = 0;
    threadId_ = 0;
    ipcStatPid_ = -1;
    isDumpAllIpc_ = false;
    isDumpIpc_ = false;
    isDumpIpcStartStat_ = false;
    isDumpIpcStopStat_ = false;
    isDumpIpcStat_ = false;
    dumpJsRawHeap_ = false;
}

DumperOpts& DumperOpts::operator = (const DumperOpts& opts)
{
    Reset();
    isDumpCpuFreq_ = opts.isDumpCpuFreq_;
    isDumpCpuUsage_ = opts.isDumpCpuUsage_;
    cpuUsagePid_ = opts.cpuUsagePid_;
    isDumpLog_ = opts.isDumpLog_;
    logArgs_.assign((opts.logArgs_).begin(), (opts.logArgs_).end());
    isDumpMem_ = opts.isDumpMem_;
    memPid_ = opts.memPid_;
    isDumpStorage_ = opts.isDumpStorage_;
    storagePid_ = opts.storagePid_;
    isDumpNet_ = opts.isDumpNet_;
    netPid_ = opts.netPid_;
    isDumpList_ = opts.isDumpList_;
    isDumpService_ = opts.isDumpService_;
    isDumpSystemAbility_ = opts.isDumpSystemAbility_;
    abilitieNames_.assign((opts.abilitieNames_).begin(), (opts.abilitieNames_).end());
    abilitieArgs_.assign((opts.abilitieArgs_).begin(), (opts.abilitieArgs_).end());
    isDumpSystem_ = opts.isDumpSystem_;
    systemArgs_ = opts.systemArgs_;
    isDumpProcesses_ = opts.isDumpProcesses_;
    processPid_ = opts.processPid_;
    isFaultLog_ = opts.isFaultLog_;
    path_ = opts.path_;
    isAppendix_ = opts.isAppendix_;
    isShowSmaps_ = opts.isShowSmaps_;
    isShowSmapsInfo_ = opts.isShowSmapsInfo_;
    isDumpJsHeapMem_ = opts.isDumpJsHeapMem_;
    isDumpJsHeapMemGC_ = opts.isDumpJsHeapMemGC_;
    isDumpJsHeapLeakobj_ = opts.isDumpJsHeapLeakobj_;
    dumpJsHeapMemPid_ = opts.dumpJsHeapMemPid_;
    threadId_ = opts.threadId_;
    ipcStatPid_ = opts.ipcStatPid_;
    isDumpAllIpc_ = opts.isDumpAllIpc_;
    isDumpIpc_ = opts.isDumpIpc_;
    isDumpIpcStartStat_ = opts.isDumpIpcStartStat_;
    isDumpIpcStopStat_ = opts.isDumpIpcStopStat_;
    isDumpIpcStat_ = opts.isDumpIpcStat_;
    dumpJsRawHeap_ = opts.dumpJsRawHeap_;
    return *this;
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
    if (isDumpIpc_) {
        return true;
    }
    DUMPER_HILOGE(MODULE_COMMON, "select nothing.");
    return false;
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
        errStr = "-1";
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
    if (threadId_ < 0) {
        errStr = std::to_string(threadId_);
        return false;
    }
    if (ipcStatPid_ < -1) {
        errStr = std::to_string(ipcStatPid_);
        return false;
    }
    return true;
}
} // namespace HiviewDFX
} // namespace OHOS
