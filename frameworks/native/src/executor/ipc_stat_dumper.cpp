/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#include "executor/ipc_stat_dumper.h"

#include <cstdio>
#include <thread>
#include <unistd.h>

#include "app_mgr_client.h"
#include "dump_common_utils.h"
#include "dump_utils.h"
#include "file_ex.h"
#include "running_process_info.h"
#include "securec.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace HiviewDFX {
const int SAMGR_SA_ID = 0;

using AppExecFwk::RunningProcessInfo;
using AppExecFwk::AppMgrClient;
using StringMatrix = std::shared_ptr<std::vector<std::vector<std::string>>>;

IPCStatDumper::IPCStatDumper()
{}

IPCStatDumper::~IPCStatDumper()
{}

void IPCStatDumper::SendErrorMessage(const std::string &errorStr)
{
    if (ptrReqCtl_ == nullptr) {
        DUMPER_HILOGE(MODULE_SERVICE, "ptrReqCtl_ is nullptr, return");
        return;
    }
    int rawParamFd = ptrReqCtl_->GetOutputFd();
    if (rawParamFd < 0) {
        DUMPER_HILOGE(MODULE_SERVICE, "rawParamFd(%{public}d) is invalid, return", rawParamFd);
        return;
    }
    SaveStringToFd(rawParamFd, errorStr + "\n");
}

DumpStatus IPCStatDumper::PreExecute(const std::shared_ptr<DumperParameter> &parameter, StringMatrix dumpDatas)
{
    ptrReqCtl_ = parameter->getClientCallback();
    pid_ = parameter->GetOpts().ipcStatPid_;
    isDumpAllIpc_ = parameter->GetOpts().isDumpAllIpc_;
    isDumpIpc_ = parameter->GetOpts().isDumpIpc_;
    isDumpIpcStartStat_ = parameter->GetOpts().isDumpIpcStartStat_;
    isDumpIpcStopStat_ = parameter->GetOpts().isDumpIpcStopStat_;
    isDumpIpcStat_ = parameter->GetOpts().isDumpIpcStat_;
    result_ = dumpDatas;
    return DumpStatus::DUMP_OK;
}

bool IPCStatDumper::CheckPidIsSa(const sptr<ISystemAbilityManager> &sam, std::vector<std::string> &args,
                                 sptr<IRemoteObject> &sa)
{
    if (sam == nullptr) {
        DUMPER_HILOGE(MODULE_SERVICE, "get samgr fail!");
        return false;
    }

    std::list<SystemProcessInfo> systemProcessInfos;
    int32_t ret = sam->GetRunningSystemProcess(systemProcessInfos);
    if (ret != ERR_OK) {
        DUMPER_HILOGE(MODULE_SERVICE, "get running system process fail! ret:%{public}d", ret);
        return false;
    }
    bool isFind = false;
    for (const auto& sysProcessInfo : systemProcessInfos) {
        if (pid_ == sysProcessInfo.pid) {
            args.push_back(sysProcessInfo.processName);
            isFind = true;
            break;
        }
    }
    if (!isFind) {
        DUMPER_HILOGE(MODULE_SERVICE, "pid:%{public}d is not in systemProcessInfos!", pid_);
        return false;
    }

    sa = sam->CheckSystemAbility(SAMGR_SA_ID);
    return true;
}

bool IPCStatDumper::CheckPidIsApp(const sptr<ISystemAbilityManager> &sam, std::vector<std::string> &args,
                                  sptr<IRemoteObject> &sa)
{
    if (sam == nullptr) {
        DUMPER_HILOGE(MODULE_SERVICE, "get samgr fail!");
        return false;
    }

    auto appMgrClient = std::make_unique<AppMgrClient>();
    if (appMgrClient == nullptr) {
        DUMPER_HILOGE(MODULE_SERVICE, "get AppMgrClient fail!");
        return false;
    }
    std::vector<RunningProcessInfo> runningProcessInfos;
    int32_t ret = appMgrClient->GetAllRunningProcesses(runningProcessInfos);
    if (ret != ERR_OK) {
        DUMPER_HILOGE(MODULE_SERVICE, "get running app process fail! ret:%{public}d", ret);
        return false;
    }
    bool isFind = false;
    for (const auto& runningProcessInfo : runningProcessInfos) {
        if (pid_ == runningProcessInfo.pid_) {
            args.push_back(std::to_string(pid_));
            isFind = true;
            break;
        }
    }
    if (!isFind) {
        DUMPER_HILOGE(MODULE_SERVICE, "pid:%{public}d is not in app runningProcessInfos!", pid_);
        return false;
    }

    sa = sam->CheckSystemAbility(APP_MGR_SERVICE_ID);
    return true;
}

DumpStatus IPCStatDumper::SetIpcStatCmd(const sptr<ISystemAbilityManager> &sam, sptr<IRemoteObject> &sa)
{
    if (sam == nullptr) {
        DUMPER_HILOGE(MODULE_SERVICE, "get samgr fail!");
        return DumpStatus::DUMP_FAIL;
    }

    std::vector<std::string> args;
    args.push_back("--ipc");
    if (isDumpAllIpc_) {
        args.push_back("all");
    } else {
        if (!CheckPidIsSa(sam, args, sa) && !CheckPidIsApp(sam, args, sa)) {
            DUMPER_HILOGE(MODULE_SERVICE, "args get process id fail");
            SendErrorMessage("pid is not support ipc statistic.");
            return DumpStatus::DUMP_FAIL;
        }
    }
    if (isDumpIpcStartStat_) {
        args.push_back("--start-stat");
    } else if (isDumpIpcStopStat_) {
        args.push_back("--stop-stat");
    } else if (isDumpIpcStat_) {
        args.push_back("--stat");
    } else {
        DUMPER_HILOGE(MODULE_SERVICE, "cmd is not valid.");
        SendErrorMessage("input ARG is not valid");
        return DumpStatus::DUMP_FAIL;
    }
    if (!args.empty()) {
        std::transform(args.begin(), args.end(), std::back_inserter(ipcDumpCmd_), Str8ToStr16);
    }

    return DumpStatus::DUMP_OK;
}

DumpStatus IPCStatDumper::DoDumpIpcStat(sptr<IRemoteObject> &sa)
{
    if (sa == nullptr) {
        DUMPER_HILOGE(MODULE_SERVICE, "no such system ability\n");
        return DumpStatus::DUMP_FAIL;
    }

    int rawParamFd = ptrReqCtl_->GetOutputFd();
    int ret = sa->Dump(rawParamFd, ipcDumpCmd_);
    if (ret != ERR_OK) {
        DUMPER_HILOGE(MODULE_SERVICE, "system ability dump fail! ret:%{public}d\n", ret);
    }
    if (rawParamFd < 0) {
        DUMPER_HILOGE(MODULE_SERVICE, "sa dump failed\n");
        return DumpStatus::DUMP_FAIL;
    }

    return DumpStatus::DUMP_OK;
}

DumpStatus IPCStatDumper::DumpIpcStat(const sptr<ISystemAbilityManager> &sam, sptr<IRemoteObject> &sa)
{
    if (sam == nullptr) {
        DUMPER_HILOGE(MODULE_SERVICE, "get samgr fail!");
        return DumpStatus::DUMP_FAIL;
    }

    std::vector<int32_t> saIds = { SAMGR_SA_ID, APP_MGR_SERVICE_ID };
    sptr<IRemoteObject> tmpSa;

    if (isDumpAllIpc_) {
        for (auto &id : saIds) {
            tmpSa = sam->CheckSystemAbility(id);
            if (DoDumpIpcStat(tmpSa) != DumpStatus::DUMP_OK) {
                DUMPER_HILOGE(MODULE_SERVICE, "sa dump failed %{public}d\n", id);
                return DumpStatus::DUMP_FAIL;
            }
        }
    } else {
        if (DoDumpIpcStat(sa) != DumpStatus::DUMP_OK) {
            DUMPER_HILOGE(MODULE_SERVICE, "sa dump failed \n");
            return DumpStatus::DUMP_FAIL;
        }
    }

    return DumpStatus::DUMP_OK;
}

DumpStatus IPCStatDumper::Execute()
{
    if (ptrReqCtl_ == nullptr) {
        DUMPER_HILOGE(MODULE_SERVICE, "ptrReqCtl_ is nullptr, return");
        return DumpStatus::DUMP_FAIL;
    }

    DUMPER_HILOGI(MODULE_COMMON, "info|IPCStatDumper Execute, %{public}d", pid_);
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        DUMPER_HILOGE(MODULE_SERVICE, "get samgr fail!");
        return DumpStatus::DUMP_FAIL;
    }
    sptr<IRemoteObject> sa;
    if (SetIpcStatCmd(sam, sa) != DumpStatus::DUMP_OK) {
        DUMPER_HILOGE(MODULE_SERVICE, "no such process\n");
        return DumpStatus::DUMP_FAIL;
    }

    if (DumpIpcStat(sam, sa) != DumpStatus::DUMP_OK) {
        DUMPER_HILOGE(MODULE_SERVICE, "dump ipc statistic fail\n");
        return DumpStatus::DUMP_FAIL;
    }
    DUMPER_HILOGI(MODULE_COMMON, "info|IPCStatDumper Execute end");
    return DumpStatus::DUMP_OK;
}

DumpStatus IPCStatDumper::AfterExecute()
{
    return DumpStatus::DUMP_OK;
}

}  // namespace HiviewDFX
}  // namespace OHOS
