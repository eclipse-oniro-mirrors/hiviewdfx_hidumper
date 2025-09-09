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
#include "dump_manager_cpu_service.h"
#include <file_ex.h>
#include <if_system_ability_manager.h>
#include <ipc_skeleton.h>
#include <iservice_registry.h>
#include <string_ex.h>
#include <system_ability_definition.h>
#include <thread>
#include <unistd.h>
#include "securec.h"

#include "accesstoken_kit.h"
#include "common/dumper_constant.h"
#include "cpu_collector.h"
#include "datetime_ex.h"
#include "dump_log_manager.h"
#include "dump_utils.h"
#include "hilog_wrapper.h"
#include "inner/dump_service_id.h"
#include "token_setproc.h"
#include "util/string_utils.h"
#include "util/file_utils.h"

using namespace std;
namespace OHOS {
namespace HiviewDFX {
namespace {
const int APP_UID = 10000;
const std::string DUMPMGR_CPU_SERVICE_NAME = "HiDumperCpuService";
static constexpr size_t LOAD_AVG_INFO_COUNT = 3;
static constexpr int PROC_CPU_LENGTH = 256;
static constexpr double HUNDRED_PERCENT_VALUE = 100.00;
static constexpr long unsigned THOUSAND_PERCENT_VALUE = 1000;
static constexpr int INVALID_PID = -1;
static const int TM_START_YEAR = 1900;
static const int DEC_SYSTEM_VALUE = 10;
static const int AVG_INFO_SUBSTR_LENGTH = 4;
}
DumpManagerCpuService::DumpManagerCpuService() : SystemAbility(DFX_SYS_HIDUMPER_CPU_ABILITY_ID, true)
{
}

DumpManagerCpuService::~DumpManagerCpuService()
{
}

void DumpManagerCpuService::OnStart()
{
    DUMPER_HILOGI(MODULE_CPU_SERVICE, "on start");
    if (started_) {
        DUMPER_HILOGE(MODULE_CPU_SERVICE, "it's ready, nothing to do.");
        return;
    }
    started_ = true;
}

int32_t DumpManagerCpuService::Request(DumpCpuData &dumpCpuData)
{
    DUMPER_HILOGI(MODULE_CPU_SERVICE, "enter");
    std::lock_guard<std::mutex> lock(mutex_);
    InitParam(dumpCpuData);
    if (!HasDumpPermission()) {
        DUMPER_HILOGE(MODULE_SERVICE,
                      "No ohos.permission.DUMP permission to acccess hidumper cpuservice, please check!");
        return DumpStatus::DUMP_NOPERMISSION;
    }
    int32_t ret = DumpCpuUsageData();
    dumpCpuData.dumpCPUDatas_ = *dumpCPUDatas_;
    ResetParam();
    return ret;
}

void DumpManagerCpuService::InitParam(DumpCpuData &dumpCpuData)
{
    cpuUsagePid_ = dumpCpuData.cpuUsagePid_;
    if (cpuUsagePid_ != INVALID_PID) {
        curSpecProc_ = std::make_shared<ProcInfo>();
    }
    curCPUInfo_ = std::make_shared<CPUInfo>();
    dumpCPUDatas_ = std::make_shared<std::vector<std::vector<std::string>>>(dumpCpuData.dumpCPUDatas_);
}

void DumpManagerCpuService::ResetParam()
{
    curCPUInfo_.reset();
    curProcs_.clear();
    if (cpuUsagePid_ != INVALID_PID) {
        curSpecProc_.reset();
    }
}

int DumpManagerCpuService::DumpCpuUsageData()
{
    if (!GetSysCPUInfo(curCPUInfo_)) {
        return DumpStatus::DUMP_FAIL;
    }

    if (cpuUsagePid_ != INVALID_PID) {
        if (!GetSingleProcInfo(cpuUsagePid_, curSpecProc_)) {
            return DumpStatus::DUMP_FAIL;
        }
    } else {
        if (!GetAllProcInfo(curProcs_)) {
            return DumpStatus::DUMP_FAIL;
        }
    }
    std::string avgInfo;
    if (ReadLoadAvgInfo(avgInfo) != DumpStatus::DUMP_OK) {
        return DumpStatus::DUMP_FAIL;
    }
    AddStrLineToDumpInfo(avgInfo);
    
    std::string startTime;
    std::string endTime;
    GetDateAndTime(startTime_ / THOUSAND_PERCENT_VALUE, startTime);
    GetDateAndTime(endTime_ / THOUSAND_PERCENT_VALUE, endTime);
    std::string dumpTimeStr;
    CreateDumpTimeString(startTime, endTime, dumpTimeStr);
    AddStrLineToDumpInfo(dumpTimeStr);

    std::string cpuStatStr;
    CreateCPUStatString(cpuStatStr);
    AddStrLineToDumpInfo(cpuStatStr);

    DumpProcInfo();
    return DumpStatus::DUMP_OK;
}

int DumpManagerCpuService::GetCpuUsageByPid(int32_t pid, double &cpuUsage)
{
    int32_t calllingUid = IPCSkeleton::GetCallingUid();
    int32_t calllingPid = IPCSkeleton::GetCallingPid();
    if (calllingUid >= APP_UID && pid != calllingPid) {
        DUMPER_HILOGE(MODULE_SERVICE, "No permission, pid:%{public}d, calllingPid:%{public}d, calllingUid:%{public}d",
            pid, calllingPid, calllingUid);
        return DumpStatus::DUMP_NOPERMISSION;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    std::shared_ptr<OHOS::HiviewDFX::UCollectUtil::CpuCollector> singleCollector =
        OHOS::HiviewDFX::UCollectUtil::CpuCollector::Create();
    if (singleCollector == nullptr) {
        DUMPER_HILOGE(MODULE_CPU_SERVICE, "get cpu usage by pid create failed");
        return DumpStatus::DUMP_FAIL;
    }
    if (pid != INVALID_PID) {
        std::shared_ptr<ProcInfo> singleProcInfo = std::make_shared<ProcInfo>();
        if (!GetSingleProcInfo(pid, singleProcInfo)) {
            return DumpStatus::DUMP_FAIL;
        }
        cpuUsage = singleProcInfo->totalUsage / HUNDRED_PERCENT_VALUE;
    }
    DUMPER_HILOGD(MODULE_CPU_SERVICE, "GetCpuUsageByPid end, pid = %{public}d, cpuUsage = %{public}f", pid, cpuUsage);
    return DumpStatus::DUMP_OK;
}

DumpStatus DumpManagerCpuService::ReadLoadAvgInfo(std::string &info)
{
    std::shared_ptr<OHOS::HiviewDFX::UCollectUtil::CpuCollector> singleCollector =
        OHOS::HiviewDFX::UCollectUtil::CpuCollector::Create();
    if (singleCollector == nullptr) {
        DUMPER_HILOGE(MODULE_CPU_SERVICE, "read load avg info create failed");
        return DumpStatus::DUMP_FAIL;
    }
    CollectResult<OHOS::HiviewDFX::SysCpuLoad> collectResult = singleCollector->CollectSysCpuLoad();
    if (collectResult.retCode != OHOS::HiviewDFX::UCollect::UcError::SUCCESS) {
        DUMPER_HILOGE(MODULE_CPU_SERVICE, "collect system cpu load error, ret:%{public}d", collectResult.retCode);
        return DumpStatus::DUMP_FAIL;
    }
    std::vector<std::string> avgLoadInfo;
    avgLoadInfo.push_back(std::string(std::to_string(collectResult.data.avgLoad1)).substr(0, AVG_INFO_SUBSTR_LENGTH));
    avgLoadInfo.push_back(std::string(std::to_string(collectResult.data.avgLoad5)).substr(0, AVG_INFO_SUBSTR_LENGTH));
    avgLoadInfo.push_back(std::string(std::to_string(collectResult.data.avgLoad15)).substr(0, AVG_INFO_SUBSTR_LENGTH));

    info = "Load average:";
    for (size_t i = 0; i < LOAD_AVG_INFO_COUNT; i++) {
        info.append(" ");
        info.append(avgLoadInfo[i]);
        if (i == LOAD_AVG_INFO_COUNT - 1) {
            info.append(";");
        } else {
            info.append(" /");
        }
    }
    info.append(" the cpu load average in 1 min, 5 min and 15 min");
    DUMPER_HILOGD(MODULE_CPU_SERVICE, "info is %{public}s", info.c_str());
    return DumpStatus::DUMP_OK;
}

bool DumpManagerCpuService::GetSysCPUInfo(std::shared_ptr<CPUInfo> &cpuInfo)
{
    if (cpuInfo == nullptr) {
        return false;
    }
    std::shared_ptr<OHOS::HiviewDFX::UCollectUtil::CpuCollector> singleCollector =
        OHOS::HiviewDFX::UCollectUtil::CpuCollector::Create();
    if (singleCollector == nullptr) {
        DUMPER_HILOGE(MODULE_CPU_SERVICE, "get sys cpu info create failed");
        return false;
    }
    auto collectResult = singleCollector->CollectProcessCpuStatInfos(false);
    if (collectResult.retCode != OHOS::HiviewDFX::UCollect::UcError::SUCCESS || collectResult.data.empty()) {
        DUMPER_HILOGE(MODULE_CPU_SERVICE, "collect process cpu stat info error");
        return false;
    }

    CollectResult<OHOS::HiviewDFX::SysCpuUsage> result = singleCollector->CollectSysCpuUsage(false);
    if (result.retCode != OHOS::HiviewDFX::UCollect::UcError::SUCCESS) {
        DUMPER_HILOGE(MODULE_CPU_SERVICE, "collect system cpu usage error,retCode is %{public}d", result.retCode);
        return false;
    }
    const OHOS::HiviewDFX::SysCpuUsage& sysCpuUsage = result.data;
    startTime_ = sysCpuUsage.startTime;
    endTime_ = sysCpuUsage.endTime;

    for (const auto& oneCpuInfo : sysCpuUsage.cpuInfos) {
        cpuInfo->userUsage = oneCpuInfo.userUsage;
        cpuInfo->niceUsage = oneCpuInfo.niceUsage;
        cpuInfo->systemUsage = oneCpuInfo.systemUsage;
        cpuInfo->idleUsage = oneCpuInfo.idleUsage;
        cpuInfo->ioWaitUsage = oneCpuInfo.ioWaitUsage;
        cpuInfo->irqUsage = oneCpuInfo.irqUsage;
        cpuInfo->softIrqUsage = oneCpuInfo.softIrqUsage;
        break;
    }

    return true;
}

bool DumpManagerCpuService::GetAllProcInfo(std::vector<std::shared_ptr<ProcInfo>> &procInfos)
{
    std::shared_ptr<OHOS::HiviewDFX::UCollectUtil::CpuCollector> singleCollector =
        OHOS::HiviewDFX::UCollectUtil::CpuCollector::Create();
    if (singleCollector == nullptr) {
        DUMPER_HILOGE(MODULE_CPU_SERVICE, "get all pro info create failed");
        return false;
    }
    auto collectResult = singleCollector->CollectProcessCpuStatInfos(false);
    if (collectResult.retCode != OHOS::HiviewDFX::UCollect::UcError::SUCCESS || collectResult.data.empty()) {
        DUMPER_HILOGE(MODULE_CPU_SERVICE, "collect process cpu stat info error");
        return false;
    }
    for (const auto& cpuInfo : collectResult.data) {
        std::shared_ptr<ProcInfo> ptrProcInfo = std::make_shared<ProcInfo>();;
        ptrProcInfo->pid = std::to_string(cpuInfo.pid);
        ptrProcInfo->comm = cpuInfo.procName;
        ptrProcInfo->minflt = std::to_string(cpuInfo.minFlt);
        ptrProcInfo->majflt = std::to_string(cpuInfo.majFlt);
        ptrProcInfo->userSpaceUsage = cpuInfo.uCpuUsage * HUNDRED_PERCENT_VALUE;
        ptrProcInfo->sysSpaceUsage = cpuInfo.sCpuUsage * HUNDRED_PERCENT_VALUE;
        ptrProcInfo->totalUsage = cpuInfo.cpuUsage * HUNDRED_PERCENT_VALUE;
        procInfos.push_back(ptrProcInfo);
    }
    return true;
}

bool DumpManagerCpuService::GetSingleProcInfo(int pid, std::shared_ptr<ProcInfo> &specProc)
{
    if (specProc == nullptr) {
        return false;
    }

    std::shared_ptr<OHOS::HiviewDFX::UCollectUtil::CpuCollector> singleCollector =
        OHOS::HiviewDFX::UCollectUtil::CpuCollector::Create();
    if (singleCollector == nullptr) {
        DUMPER_HILOGE(MODULE_CPU_SERVICE, "get single proc info create failed");
        return false;
    }

    CollectResult<OHOS::HiviewDFX::ProcessCpuStatInfo> collectResult = singleCollector->CollectProcessCpuStatInfo(pid);
    if (collectResult.retCode != OHOS::HiviewDFX::UCollect::UcError::SUCCESS) {
        DUMPER_HILOGE(MODULE_CPU_SERVICE, "collect system cpu usage error,ret:%{public}d", collectResult.retCode);
        return false;
    }
    specProc->comm = collectResult.data.procName;
    specProc->pid = std::to_string(collectResult.data.pid);
    specProc->minflt = std::to_string(collectResult.data.minFlt);
    specProc->majflt = std::to_string(collectResult.data.majFlt);
    specProc->userSpaceUsage = collectResult.data.uCpuUsage * HUNDRED_PERCENT_VALUE;
    specProc->sysSpaceUsage = collectResult.data.sCpuUsage * HUNDRED_PERCENT_VALUE;
    specProc->totalUsage = collectResult.data.cpuUsage * HUNDRED_PERCENT_VALUE;
    return true;
}

bool DumpManagerCpuService::GetDateAndTime(uint64_t timeStamp, std::string &dateTime)
{
    time_t time = static_cast<time_t>(timeStamp);
    struct tm timeData = {0};
    localtime_r(&time, &timeData);

    dateTime = " ";
    dateTime.append(std::to_string(TM_START_YEAR + timeData.tm_year));
    dateTime.append("-");
    if (1 + timeData.tm_mon < DEC_SYSTEM_VALUE) {
        dateTime.append(std::to_string(0));
    }
    dateTime.append(std::to_string(1 + timeData.tm_mon));
    dateTime.append("-");
    if (timeData.tm_mday < DEC_SYSTEM_VALUE) {
        dateTime.append(std::to_string(0));
    }
    dateTime.append(std::to_string(timeData.tm_mday));
    dateTime.append(" ");
    if (timeData.tm_hour < DEC_SYSTEM_VALUE) {
        dateTime.append(std::to_string(0));
    }
    dateTime.append(std::to_string(timeData.tm_hour));
    dateTime.append(":");
    if (timeData.tm_min < DEC_SYSTEM_VALUE) {
        dateTime.append(std::to_string(0));
    }
    dateTime.append(std::to_string(timeData.tm_min));
    dateTime.append(":");
    if (timeData.tm_sec < DEC_SYSTEM_VALUE) {
        dateTime.append(std::to_string(0));
    }
    dateTime.append(std::to_string(timeData.tm_sec));
    return true;
}


void DumpManagerCpuService::CreateDumpTimeString(const std::string &startTime,
    const std::string &endTime, std::string &timeStr)
{
    DUMPER_HILOGD(MODULE_CPU_SERVICE, "start:%{public}s, end:%{public}s", startTime.c_str(), endTime.c_str());
    timeStr = "CPU usage from";
    timeStr.append(startTime);
    timeStr.append(" to");
    timeStr.append(endTime);
}

void DumpManagerCpuService::AddStrLineToDumpInfo(const std::string &strLine)
{
    std::vector<std::string> vec;
    vec.push_back(strLine);
    dumpCPUDatas_->push_back(vec);
}

void DumpManagerCpuService::CreateCPUStatString(std::string &str)
{
    double userSpaceUsage = (curCPUInfo_->userUsage + curCPUInfo_->niceUsage) * HUNDRED_PERCENT_VALUE;
    double sysSpaceUsage = curCPUInfo_->systemUsage * HUNDRED_PERCENT_VALUE;
    double iowUsage = curCPUInfo_->ioWaitUsage * HUNDRED_PERCENT_VALUE;
    double irqUsage = (curCPUInfo_->irqUsage + curCPUInfo_->softIrqUsage) * HUNDRED_PERCENT_VALUE;
    double idleUsage = curCPUInfo_->idleUsage * HUNDRED_PERCENT_VALUE;
    double totalUsage = userSpaceUsage + sysSpaceUsage;

    char format[PROC_CPU_LENGTH] = {0};
    int ret = sprintf_s(format, PROC_CPU_LENGTH,
                        "Total: %.2f%%; User Space: %.2f%%; Kernel Space: %.2f%%; "
                        "iowait: %.2f%%; irq: %.2f%%; idle: %.2f%%",
                        totalUsage, userSpaceUsage, sysSpaceUsage, iowUsage, irqUsage, idleUsage);
    if (ret < 0) {
        DUMPER_HILOGE(MODULE_CPU_SERVICE, "create process cpu info failed!.");
        return;
    }
    str = std::string(format);
}

void DumpManagerCpuService::DumpProcInfo()
{
    std::vector<std::shared_ptr<ProcInfo>> sortedInfos;
    sortedInfos.assign(curProcs_.begin(), curProcs_.end());
    std::sort(sortedInfos.begin(), sortedInfos.end(), SortProcInfo);

    AddStrLineToDumpInfo("Details of Processes:");
    AddStrLineToDumpInfo("    PID   Total Usage	   User Space    Kernel Space    Page Fault Minor"
                         "    Page Fault Major    Name");
    if (cpuUsagePid_ != INVALID_PID) {
        char format[PROC_CPU_LENGTH] = {0};
        int ret = sprintf_s(format, PROC_CPU_LENGTH,
                            "    %-5s    %6.2f%%         %6.2f%%"
                            "        %6.2f%%        %8s            %8s            %-15s",
                            (curSpecProc_->pid).c_str(), curSpecProc_->totalUsage,
                            curSpecProc_->userSpaceUsage, curSpecProc_->sysSpaceUsage,
                            (curSpecProc_->minflt).c_str(), (curSpecProc_->majflt).c_str(),
                            (curSpecProc_->comm).c_str());
        AddStrLineToDumpInfo(std::string(format));
        if (ret < 0) {
            DUMPER_HILOGE(MODULE_CPU_SERVICE, "Dump process %{public}d cpu info failed!.", cpuUsagePid_);
        }
        return;
    }
    for (size_t i = 0; i < sortedInfos.size(); i++) {
        char format[PROC_CPU_LENGTH] = {0};
        int ret = sprintf_s(format, PROC_CPU_LENGTH,
                            "    %-5s    %6.2f%%         %6.2f%%"
                            "        %6.2f%%        %8s            %8s            %-15s",
                            (sortedInfos[i]->pid).c_str(), sortedInfos[i]->totalUsage,
                            sortedInfos[i]->userSpaceUsage, sortedInfos[i]->sysSpaceUsage,
                            (sortedInfos[i]->minflt).c_str(), (sortedInfos[i]->majflt).c_str(),
                            (sortedInfos[i]->comm).c_str());
        if (ret < 0) {
            continue;
        }
        AddStrLineToDumpInfo(std::string(format));
    }
}

bool DumpManagerCpuService::SortProcInfo(std::shared_ptr<ProcInfo> &left, std::shared_ptr<ProcInfo> &right)
{
    if (right->totalUsage != left->totalUsage) {
        return right->totalUsage < left->totalUsage;
    }
    if (right->userSpaceUsage != left->userSpaceUsage) {
        return right->userSpaceUsage < left->userSpaceUsage;
    }
    if (right->sysSpaceUsage != left->sysSpaceUsage) {
        return right->sysSpaceUsage < left->sysSpaceUsage;
    }
    if (right->pid.length() != left->pid.length()) {
        return right->pid.length() < left->pid.length();
    }
    return (right->pid.compare(left->pid) < 0);
}

void DumpManagerCpuService::StartService()
{
    sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        DUMPER_HILOGE(MODULE_CPU_SERVICE, "failed to find SystemAbilityManager.");
        return;
    }
    auto dumpManagerCpuService = DumpDelayedSpSingleton<DumpManagerCpuService>::GetInstance();
    int ret = samgr->AddSystemAbility(DFX_SYS_HIDUMPER_CPU_ABILITY_ID, dumpManagerCpuService);
    if (ret != 0) {
        DUMPER_HILOGE(MODULE_CPU_SERVICE, "failed to add sys dump cpu service ability.");
        return;
    }
    OnStart();
}

// Authenticate dump permissions
bool DumpManagerCpuService::HasDumpPermission() const
{
    uint32_t callingTokenID = IPCSkeleton::GetCallingTokenID();
    int res = Security::AccessToken::AccessTokenKit::VerifyAccessToken(callingTokenID, "ohos.permission.DUMP");
    if (res != Security::AccessToken::PermissionState::PERMISSION_GRANTED) {
        DUMPER_HILOGI(MODULE_SERVICE, "No dump permission, please check!");
        return false;
    }
    return true;
}
} // namespace HiviewDFX
} // namespace OHOS
