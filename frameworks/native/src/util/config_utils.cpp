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
#include "util/config_utils.h"
#include "directory_ex.h"
#include "hilog_wrapper.h"
#include "dump_common_utils.h"
#include "dump_utils.h"
#include "parameter.h"
#include "common/dumper_constant.h"
namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr int ROOT_UID = 0;
constexpr int BMS_UID = 1000;
constexpr int APP_FIRST_UID = 10000;
static const std::string SMAPS_PATH = "smaps/";
static const std::string SMAPS_PATH_START = "/proc/";
static const std::string SMAPS_PATH_END = "/smaps";
} // namespace

ConfigUtils::ConfigUtils(const std::shared_ptr<DumperParameter> &param) : dumperParam_(param)
{
}

ConfigUtils::~ConfigUtils()
{
    pidInfos_.clear();
    cpuInfos_.clear();
    currentPidInfos_.clear();
    currentPidInfo_.Reset();
}

DumpStatus ConfigUtils::GetDumperConfigs(const std::shared_ptr<DumperParameter> &param)
{
    DUMPER_HILOGD(MODULE_COMMON, "enter|");
    DumpStatus ret = DumpStatus::DUMP_FAIL;

    if (param != nullptr) {
        ConfigUtils configUtils(param);
        ret = configUtils.GetDumperConfigs();
    }

    if (ret == DumpStatus::DUMP_OK) {
        auto dumpCfgs = param->GetExecutorConfigList();
        for (size_t i = 0; i < dumpCfgs.size(); i++) {
            dumpCfgs[i]->Dump();
        }
    }

    DUMPER_HILOGD(MODULE_COMMON, "leave|ret=%{public}d", ret);
    return ret;
}

DumpStatus ConfigUtils::GetDumperConfigs()
{
    DUMPER_HILOGD(MODULE_COMMON, "enter|");

    DumpCommonUtils::GetPidInfos(pidInfos_);
    DumpCommonUtils::GetCpuInfos(cpuInfos_);
    DUMPER_HILOGD(MODULE_COMMON, "debug|pidInfos=%{public}zu, cpuInfos=%{public}zu",
        pidInfos_.size(), cpuInfos_.size());

    std::vector<std::shared_ptr<DumpCfg>> dumpCfgs;

    currentPidInfo_.Reset();
    currentPidInfos_.clear();

    HandleDumpSystem(dumpCfgs);
    HandleDumpCpuFreq(dumpCfgs);  // cpuid
    HandleDumpCpuUsage(dumpCfgs); // pid
    HandleDumpMem(dumpCfgs);
    HandleDumpLog(dumpCfgs);
    HandleDumpStorage(dumpCfgs);
    HandleDumpNet(dumpCfgs);
    HandleDumpList(dumpCfgs);
    HandleDumpAbility(dumpCfgs);
    HandleDumpService(dumpCfgs);
    HandleDumpProcesses(dumpCfgs);
    HandleDumpFaultLog(dumpCfgs);
    HandleDumpAppendix(dumpCfgs);
    HandleDumpTest(dumpCfgs);

    DUMPER_HILOGD(MODULE_COMMON, "debug|dumpCfgs=%{public}zu", dumpCfgs.size());

    dumperParam_->SetExecutorConfigList(dumpCfgs);

    DUMPER_HILOGD(MODULE_COMMON, "leave|");
    return DumpStatus::DUMP_OK;
}

DumpStatus ConfigUtils::GetSectionNames(const std::string &name, std::vector<std::string> &nameList)
{
    std::vector<std::string> tmpUse;
    GetGroupNames(name, tmpUse);
    std::transform(tmpUse.begin(), tmpUse.end(), std::back_inserter(nameList),
                   [](std::string &a) { return GetSectionName(a); });
    DumpUtils::RemoveDuplicateString(nameList); // remove duplicate log names
    return DumpStatus::DUMP_OK;
}

DumpStatus ConfigUtils::GetGroupNames(const std::string &name, std::vector<std::string> &nameList)
{
    bool check = !name.empty();
    for (int i = 0; i < groupSum_; i++) {
        if (groups_[i].name_.empty()) {
            continue;
        }
        if (check && (groups_[i].name_.find(name) != 0)) {
            continue;
        }
        nameList.push_back(groups_[i].name_);
    }
    return DumpStatus::DUMP_OK;
}

std::string ConfigUtils::GetSectionName(const std::string &name)
{
    std::string ret;
    std::size_t pos = name.find_last_of(CONFIG_NAME_SPLIT);
    if (pos != std::string::npos) {
        ret = name.substr(pos + 1);
    }
    return ret;
}

bool ConfigUtils::MergePidInfos(std::vector<DumpCommonUtils::PidInfo> &pidInfos, int pid)
{
    pidInfos.clear();
    if (pid < 0) {
        currentPidInfo_.pid_ = pid;
        currentPidInfo_.uid_ = -1;
        pidInfos.assign(pidInfos_.begin(), pidInfos_.end());
    } else {
        if (DumpCommonUtils::GetProcessInfo(pid, currentPidInfo_)) {
            pidInfos.push_back(currentPidInfo_);
        }
    }
    return true;
}

bool ConfigUtils::HandleDumpLog(std::vector<std::shared_ptr<DumpCfg>> &dumpCfgs)
{
    const DumperOpts &dumperOpts = dumperParam_->GetOpts();
    if (!dumperOpts.isDumpLog_) {
        return false;
    }

    DUMPER_HILOGD(MODULE_COMMON, "debug|log");
    currentPidInfo_.Reset();
    currentPidInfos_.clear();

    auto args = OptionArgs::Create();
    args->SetStrList(dumperOpts.logArgs_);
    for (size_t i = 0; i < dumperOpts.logArgs_.size(); i++) {
        std::string name = CONFIG_GROUP_LOG_ + dumperOpts.logArgs_[i];
        GetConfig(name, dumpCfgs, args);
    }

    currentPidInfos_.clear();
    currentPidInfo_.Reset();
    return true;
}

bool ConfigUtils::HandleDumpList(std::vector<std::shared_ptr<DumpCfg>> &dumpCfgs)
{
    const DumperOpts &dumperOpts = dumperParam_->GetOpts();
    if (!dumperOpts.isDumpList_) {
        return false;
    }

    DUMPER_HILOGD(MODULE_COMMON, "debug|list");
    currentPidInfo_.Reset();
    currentPidInfos_.clear();

    if (dumperOpts.isDumpSystemAbility_) {
        DUMPER_HILOGD(MODULE_COMMON, "debug|list ability");
        std::shared_ptr<OptionArgs> args;
        GetConfig(CONFIG_DUMPER_LIST_SYSTEM_ABILITY, dumpCfgs, args);
    }

    if (dumperOpts.isDumpService_) {
        DUMPER_HILOGD(MODULE_COMMON, "debug|list service");
        std::shared_ptr<OptionArgs> args;
        GetConfig(CONFIG_DUMPER_LIST_SERVICE, dumpCfgs, args);
    }

    if (dumperOpts.isDumpSystem_) {
        DUMPER_HILOGD(MODULE_COMMON, "debug|list system");
        std::shared_ptr<OptionArgs> args;
        GetConfig(CONFIG_DUMPER_LIST_SYSTEM, dumpCfgs, args);
    }

    currentPidInfos_.clear();
    currentPidInfo_.Reset();
    return true;
}

bool ConfigUtils::HandleDumpService(std::vector<std::shared_ptr<DumpCfg>> &dumpCfgs)
{
    const DumperOpts &dumperOpts = dumperParam_->GetOpts();
    if (dumperOpts.isDumpList_ || (!dumperOpts.isDumpService_)) {
        return false;
    }

    DUMPER_HILOGD(MODULE_COMMON, "debug|service");
    currentPidInfo_.Reset();
    currentPidInfos_.clear();

    std::shared_ptr<OptionArgs> args;
    GetConfig(CONFIG_GROUP_SERVICE, dumpCfgs, args);

    currentPidInfos_.clear();
    currentPidInfo_.Reset();
    return true;
}

bool ConfigUtils::HandleDumpAbility(std::vector<std::shared_ptr<DumpCfg>> &dumpCfgs)
{
    const DumperOpts &dumperOpts = dumperParam_->GetOpts();
    if (dumperOpts.isDumpList_ || (!dumperOpts.isDumpSystemAbility_)) {
        return false;
    }

    DUMPER_HILOGD(MODULE_COMMON, "debug|ability");
    currentPidInfo_.Reset();
    currentPidInfos_.clear();

    auto args = OptionArgs::Create();
    args->SetNamesAndArgs(dumperOpts.abilitieNames_, dumperOpts.abilitieArgs_);
    GetConfig(CONFIG_GROUP_ABILITY, dumpCfgs, args);

    currentPidInfos_.clear();
    currentPidInfo_.Reset();
    return true;
}

bool ConfigUtils::HandleDumpSystem(std::vector<std::shared_ptr<DumpCfg>> &dumpCfgs)
{
    const DumperOpts &dumperOpts = dumperParam_->GetOpts();
    if (dumperOpts.isDumpList_ || (!dumperOpts.isDumpSystem_)) {
        return false;
    }

    DUMPER_HILOGD(MODULE_COMMON, "debug|system");
    currentPidInfo_.Reset();
    currentPidInfos_.clear();

    if (dumperOpts.systemArgs_.empty()) {
        std::shared_ptr<OptionArgs> args;
        GetConfig(CONFIG_GROUP_SYSTEM_BASE, dumpCfgs, args);
        GetConfig(CONFIG_GROUP_SYSTEM_SYSTEM, dumpCfgs, args);
        isDumpSystemSystem = true;
        return true;
    }

    auto args = OptionArgs::Create();
    args->SetStrList(dumperOpts.systemArgs_);
    for (size_t i = 0; i < dumperOpts.systemArgs_.size(); i++) {
        std::string name = CONFIG_GROUP_SYSTEM_ + dumperOpts.systemArgs_[i];
        GetConfig(name, dumpCfgs, args);
        if (name == CONFIG_GROUP_SYSTEM_SYSTEM) {
            isDumpSystemSystem = true;
        }
    }

    currentPidInfos_.clear();
    currentPidInfo_.Reset();
    return true;
}

bool ConfigUtils::HandleDumpCpuFreq(std::vector<std::shared_ptr<DumpCfg>> &dumpCfgs)
{
    const DumperOpts &dumperOpts = dumperParam_->GetOpts();
    if (!dumperOpts.isDumpCpuFreq_) {
        return false;
    }

    DUMPER_HILOGD(MODULE_COMMON, "debug|isDumpSystem=%{public}d", isDumpSystemSystem);
    if (isDumpSystemSystem) {
        return false;
    }

    DUMPER_HILOGD(MODULE_COMMON, "debug|cpu freq");
    currentPidInfo_.Reset();
    currentPidInfos_.clear();

    std::shared_ptr<OptionArgs> args;
    GetConfig(CONFIG_GROUP_CPU_FREQ, dumpCfgs, args);

    currentPidInfos_.clear();
    currentPidInfo_.Reset();
    return true;
}

bool ConfigUtils::HandleDumpCpuUsage(std::vector<std::shared_ptr<DumpCfg>> &dumpCfgs)
{
    const DumperOpts &dumperOpts = dumperParam_->GetOpts();
    if (!dumperOpts.isDumpCpuUsage_) {
        return false;
    }

    DUMPER_HILOGD(MODULE_COMMON, "debug|isDumpSystem=%{public}d, cpuUsagePid=%{public}d",
        isDumpSystemSystem, dumperOpts.cpuUsagePid_);
    if (isDumpSystemSystem && (dumperOpts.cpuUsagePid_ < 0)) {
        return false;
    }

    DUMPER_HILOGD(MODULE_COMMON, "debug|cpu usage");
    currentPidInfo_.Reset();
    currentPidInfos_.clear();
    MergePidInfos(currentPidInfos_, dumperOpts.cpuUsagePid_);

    std::shared_ptr<OptionArgs> args;
    GetConfig(CONFIG_GROUP_CPU_USAGE, dumpCfgs, args);

    currentPidInfos_.clear();
    currentPidInfo_.Reset();
    return true;
}

bool ConfigUtils::HandleDumpMem(std::vector<std::shared_ptr<DumpCfg>> &dumpCfgs)
{
    const DumperOpts &dumperOpts = dumperParam_->GetOpts();
    if (!dumperOpts.isDumpMem_) {
        return false;
    }

    DUMPER_HILOGD(MODULE_COMMON, "debug|isDumpSystem=%{public}d, memPid=%{public}d",
        isDumpSystemSystem, dumperOpts.memPid_);
    if (isDumpSystemSystem && (dumperOpts.memPid_ < 0)) {
        return false;
    }

    DUMPER_HILOGD(MODULE_COMMON, "debug|mem");
    currentPidInfo_.Reset();
    currentPidInfos_.clear();
    MergePidInfos(currentPidInfos_, dumperOpts.memPid_);

    std::shared_ptr<OptionArgs> args;
    GetConfig(CONFIG_GROUP_MEMORY, dumpCfgs, args);

    currentPidInfos_.clear();
    currentPidInfo_.Reset();
    return true;
}

bool ConfigUtils::HandleDumpStorage(std::vector<std::shared_ptr<DumpCfg>> &dumpCfgs)
{
    const DumperOpts &dumperOpts = dumperParam_->GetOpts();
    if (!dumperOpts.isDumpStorage_) {
        return false;
    }

    DUMPER_HILOGD(MODULE_COMMON, "debug|storage");
    currentPidInfo_.Reset();
    currentPidInfos_.clear();

    std::shared_ptr<OptionArgs> args;
    GetConfig(CONFIG_GROUP_STORAGE, dumpCfgs, args);

    currentPidInfos_.clear();
    currentPidInfo_.Reset();
    return true;
}

bool ConfigUtils::HandleDumpNet(std::vector<std::shared_ptr<DumpCfg>> &dumpCfgs)
{
    const DumperOpts &dumperOpts = dumperParam_->GetOpts();
    if (!dumperOpts.isDumpNet_) {
        return false;
    }

    DUMPER_HILOGD(MODULE_COMMON, "debug|net");
    currentPidInfo_.Reset();
    currentPidInfos_.clear();

    std::shared_ptr<OptionArgs> args;
    GetConfig(CONFIG_GROUP_NET, dumpCfgs, args);

    currentPidInfos_.clear();
    currentPidInfo_.Reset();
    return true;
}

bool ConfigUtils::HandleDumpProcesses(std::vector<std::shared_ptr<DumpCfg>> &dumpCfgs)
{
    const DumperOpts &dumperOpts = dumperParam_->GetOpts();
    std::string path = dumperOpts.path_;
    if (!dumperOpts.isDumpProcesses_) {
        return false;
    }

    DUMPER_HILOGD(MODULE_COMMON, "debug|processes");
    currentPidInfo_.Reset();
    currentPidInfos_.clear();
    MergePidInfos(currentPidInfos_, dumperOpts.processPid_);

    std::string mode = GetBuildType();
    std::shared_ptr<OptionArgs> args;
    if (mode == RELEASE_MODE) { // release mode
        if (dumperOpts.processPid_ < 0) {
            GetConfig(CONFIG_GROUP_PROCESSES, dumpCfgs, args);
        } else {
            GetConfig(CONFIG_GROUP_PROCESSES_PID, dumpCfgs, args);
        }
    } else { // engine mode
        if (dumperOpts.processPid_ < 0) {
            GetConfig(CONFIG_GROUP_PROCESSES_ENG, dumpCfgs, args);
        } else {
            GetConfig(CONFIG_GROUP_PROCESSES_PID_ENG, dumpCfgs, args);
        }

        if (dumperOpts.IsDumpZip()) {
            CopySmaps();
        }
    }

    currentPidInfos_.clear();
    currentPidInfo_.Reset();
    return true;
}

bool ConfigUtils::HandleDumpFaultLog(std::vector<std::shared_ptr<DumpCfg>> &dumpCfgs)
{
    const DumperOpts &dumperOpts = dumperParam_->GetOpts();
    if (!dumperOpts.isFaultLog_) {
        return false;
    }

    DUMPER_HILOGD(MODULE_COMMON, "debug|fault log");
    currentPidInfo_.Reset();
    currentPidInfos_.clear();

    std::shared_ptr<OptionArgs> args;
    GetConfig(CONFIG_GROUP_FAULT_LOG, dumpCfgs, args);

    currentPidInfos_.clear();
    currentPidInfo_.Reset();
    return true;
}

bool ConfigUtils::HandleDumpAppendix(std::vector<std::shared_ptr<DumpCfg>> &dumpCfgs)
{
    const DumperOpts &dumperOpts = dumperParam_->GetOpts();
    if (!dumperOpts.isAppendix_) {
        return false;
    }

    DUMPER_HILOGD(MODULE_COMMON, "debug|appendix");
    currentPidInfo_.Reset();
    currentPidInfos_.clear();

    MergePidInfos(currentPidInfos_, -1);
    std::shared_ptr<OptionArgs> args;

    GetConfig(CONFIG_GROUP_LOG_KERNEL, dumpCfgs, args);
    GetConfig(CONFIG_GROUP_LOG_INIT, dumpCfgs, args);
    GetConfig(CONFIG_GROUP_LOG_HILOG, dumpCfgs, args);
    int callingUid = dumperParam_->GetUid();
    if (callingUid == ROOT_UID || callingUid == BMS_UID) {
        GetConfig(CONFIG_GROUP_STACK, dumpCfgs, args);
    } else {
        DUMPER_HILOGE(MODULE_COMMON, "No permission to perform dump stack operation, uid=%d", callingUid);
    }
    currentPidInfos_.clear();
    currentPidInfo_.Reset();
    return true;
}

bool ConfigUtils::HandleDumpTest(std::vector<std::shared_ptr<DumpCfg>> &dumpCfgs)
{
    const DumperOpts &dumperOpts = dumperParam_->GetOpts();
    if (!dumperOpts.isTest_) {
        return false;
    }

    DUMPER_HILOGD(MODULE_COMMON, "debug|test");
    currentPidInfo_.Reset();
    currentPidInfos_.clear();

    std::shared_ptr<OptionArgs> args;
    GetConfig(CONFIG_GROUP_TEST, dumpCfgs, args);

    currentPidInfos_.clear();
    currentPidInfo_.Reset();
    return true;
}

int ConfigUtils::GetDumpLevelByPid(int uid, const DumpCommonUtils::PidInfo &pidInfo)
{
    int ret = DumperConstant::LEVEL_NONE;
    if (uid == ROOT_UID) {
        ret = DumperConstant::LEVEL_HIGH;
    } else if (uid < APP_FIRST_UID) {
        ret = DumperConstant::LEVEL_MIDDLE;
    } else {
        if (uid == pidInfo.uid_) {
            ret = DumperConstant::LEVEL_MIDDLE;
        }
    }
    return ret;
}

DumpStatus ConfigUtils::GetConfig(const std::string &name, std::vector<std::shared_ptr<DumpCfg>> &result,
                                  std::shared_ptr<OptionArgs> args)
{
    DumpStatus ret = DumpStatus::DUMP_FAIL;
    if (name.find(CONFIG_DUMPER_) == 0) {
        DUMPER_HILOGD(MODULE_COMMON, "debug|dumper, name=%{public}s", name.c_str());
        ret = GetDumper(name, result, args);
    } else if (name.find(CONFIG_GROUP_) == 0) {
        DUMPER_HILOGD(MODULE_COMMON, "debug|group, name=%{public}s", name.c_str());
        ret = GetGroup(name, result, args);
    } else {
        DUMPER_HILOGE(MODULE_COMMON, "error|name=%{public}s", name.c_str());
    }
    return ret;
}

DumpStatus ConfigUtils::GetDumper(int index, std::vector<std::shared_ptr<DumpCfg>> &result,
                                  std::shared_ptr<OptionArgs> args, int level)
{
    if ((index < 0) || (index >= dumperSum_)) {
        return DumpStatus::DUMP_INVALID_ARG;
    }
    auto itemlist = dumpers_[index].list_;
    auto itemsize = dumpers_[index].size_;
    for (int i = 0; i < itemsize; i++) {
        if (DumpCfg::IsFilter(itemlist[i].class_) && DumpCfg::IsLevel(level)) {
            if ((itemlist[i].level_ != DumperConstant::LEVEL_ALL) && (itemlist[i].level_ != level)) {
                continue;
            }
        }
        auto dumpCfg = DumpCfg::Create();
        dumpCfg->name_ = itemlist[i].name_;
        dumpCfg->desc_ = itemlist[i].desc_;
        dumpCfg->target_ = itemlist[i].target_;
        dumpCfg->section_ = itemlist[i].section_;
        dumpCfg->class_ = itemlist[i].class_;
        dumpCfg->level_ = itemlist[i].level_;
        dumpCfg->loop_ = itemlist[i].loop_;
        dumpCfg->filterCfg_ = itemlist[i].filterCfg_;
        dumpCfg->args_ = dumpCfg->IsDumper() ? args : nullptr;
        result.push_back(dumpCfg);
    }
    return DumpStatus::DUMP_OK;
}

DumpStatus ConfigUtils::GetDumper(const std::string &name, std::vector<std::shared_ptr<DumpCfg>> &result,
                                  std::shared_ptr<OptionArgs> args, int level)
{
    DumpStatus ret = DumpStatus::DUMP_FAIL;
    int index = -1;
    for (int i = 0; i < dumperSum_; i++) {
        if (dumpers_[i].name_.empty()) {
            continue;
        }
        if (name != dumpers_[i].name_) {
            continue;
        }
        index = i;
        break;
    }
    if (index > -1) {
        ret = GetDumper(index, result, args, level);
    }
    return ret;
}

DumpStatus ConfigUtils::GetGroupSimple(const GroupCfg &groupCfg, std::vector<std::shared_ptr<DumpCfg>> &result,
                                       std::shared_ptr<OptionArgs> args, int level, int nest)
{
    if (nest > NEST_MAX) {
        return DumpStatus::DUMP_INVALID_ARG;
    }
    if ((groupCfg.list_ == nullptr) || (groupCfg.size_ < 1)) {
        return DumpStatus::DUMP_OK;
    }

    auto dumpGroup = DumpCfg::Create();
    if (groupCfg.expand_) {
        dumpGroup->class_ = DumperConstant::GROUP;
        dumpGroup->name_ = groupCfg.name_;
        dumpGroup->desc_ = groupCfg.desc_;
        dumpGroup->type_ = groupCfg.type_;
        dumpGroup->expand_ = groupCfg.expand_;
        result.push_back(dumpGroup);
    }
    auto &outlist = (groupCfg.expand_) ? dumpGroup->childs_ : result;

    for (int i = 0; i < groupCfg.size_; i++) {
        if (groupCfg.list_[i].empty()) {
            continue;
        }
        if (DumpCommonUtils::StartWith(groupCfg.list_[i], CONFIG_DUMPER_)) {
            GetDumper(groupCfg.list_[i], outlist, args, level);
        } else if (DumpCommonUtils::StartWith(groupCfg.list_[i], CONFIG_MINIGROUP_)) {
            GetGroup(groupCfg.list_[i], outlist, args, level, nest + 1);
        } else {
            DUMPER_HILOGE(MODULE_COMMON, "error|name=%{public}s", groupCfg.name_.c_str());
            return DumpStatus::DUMP_INVALID_ARG;
        }
    }

    return DumpStatus::DUMP_OK;
}

DumpStatus ConfigUtils::GetGroup(int index, std::vector<std::shared_ptr<DumpCfg>> &result,
                                 std::shared_ptr<OptionArgs> args, int level, int nest)
{
    if (nest > NEST_MAX) {
        return DumpStatus::DUMP_INVALID_ARG;
    }
    auto dumpGroup = DumpCfg::Create();
    dumpGroup->class_ = DumperConstant::GROUP;
    dumpGroup->name_ = groups_[index].name_;
    dumpGroup->desc_ = groups_[index].desc_;
    dumpGroup->type_ = groups_[index].type_;
    dumpGroup->expand_ = groups_[index].expand_;
    result.push_back(dumpGroup);
    if (dumpGroup->expand_ && (dumpGroup->type_ == DumperConstant::GROUPTYPE_PID)) {
        for (auto pidInfo : currentPidInfos_) {
            int newLevel = GetDumpLevelByPid(dumperParam_->GetUid(), pidInfo);
            if (newLevel == DumperConstant::LEVEL_NONE) {
                continue;
            }
            auto newArgs = OptionArgs::Clone(args);
            newArgs->SetPid(pidInfo.pid_, pidInfo.uid_);
            GetGroupSimple(groups_[index], dumpGroup->childs_, newArgs, newLevel, nest);
        }
    } else if (dumpGroup->expand_ && (dumpGroup->type_ == DumperConstant::GROUPTYPE_CPUID)) {
        for (auto cpuInfo : cpuInfos_) {
            auto newArgs = OptionArgs::Clone(args);
            newArgs->SetCpuId(cpuInfo.id_);
            GetGroupSimple(groups_[index], dumpGroup->childs_, newArgs, level, nest);
        }
    } else if (dumpGroup->type_ == DumperConstant::GROUPTYPE_PID) {
        int newLevel = GetDumpLevelByPid(dumperParam_->GetUid(), currentPidInfo_);
        if (newLevel != DumperConstant::LEVEL_NONE) {
            auto newArgs = OptionArgs::Clone(args);
            newArgs->SetPid(currentPidInfo_.pid_, currentPidInfo_.uid_);
            GetGroupSimple(groups_[index], dumpGroup->childs_, newArgs, level, nest);
        }
    } else if (dumpGroup->type_ == DumperConstant::GROUPTYPE_CPUID) {
        auto newArgs = OptionArgs::Clone(args);
        newArgs->SetCpuId(-1);
        GetGroupSimple(groups_[index], dumpGroup->childs_, newArgs, level, nest);
    } else if (dumpGroup->type_ == DumperConstant::NONE) {
        GetGroupSimple(groups_[index], dumpGroup->childs_, args, level, nest);
    } else {
        DUMPER_HILOGE(MODULE_COMMON, "error|type=%{public}d", dumpGroup->type_);
        return DumpStatus::DUMP_INVALID_ARG;
    }
    return DumpStatus::DUMP_OK;
}

DumpStatus ConfigUtils::GetGroup(const std::string &name, std::vector<std::shared_ptr<DumpCfg>> &result,
                                 std::shared_ptr<OptionArgs> args, int level, int nest)
{
    if (nest > NEST_MAX) {
        return DumpStatus::DUMP_INVALID_ARG;
    }
    DumpStatus ret = DumpStatus::DUMP_FAIL;
    int index = -1;
    // find group
    for (int i = 0; i < groupSum_; i++) {
        if (groups_[i].name_.empty()) {
            continue;
        }
        if (name != groups_[i].name_) {
            continue;
        }
        index = i;
        break;
    }

    // add dump config to tmpUse
    std::vector<std::shared_ptr<DumpCfg>> tmpUse;
    if (index > -1) {
        ret = GetGroup(index, tmpUse, args, level);
    }

    if (nest) {
        result.insert(result.end(), tmpUse.begin(), tmpUse.end());
    } else {
        // add section & add config to result
        SetSection(tmpUse, GetSectionName(name));
        ConvertTreeToList(tmpUse, result);
    }

    return ret;
}

void ConfigUtils::ConvertTreeToList(std::vector<std::shared_ptr<DumpCfg>> &tree,
                                    std::vector<std::shared_ptr<DumpCfg>> &list, int nest)
{
    if (nest > NEST_MAX) {
        return;
    }

    std::vector<std::shared_ptr<DumpCfg>> tmpUsed;
    for (auto item : tree) {
        if (item == nullptr) {
            continue;
        }
        tmpUsed.push_back(item);
        if (item->childs_.empty()) {
            continue;
        }
        for (auto child : item->childs_) {
            child->parent_ = item; // after point to parent, childs must be cleared.
        }
        ConvertTreeToList(item->childs_, tmpUsed, nest + 1);
        item->childs_.clear(); // must clear
    }

    list.insert(list.end(), tmpUsed.begin(), tmpUsed.end());
}

void ConfigUtils::SetSection(std::vector<std::shared_ptr<DumpCfg>> &dumpCfgs, const std::string &section, int nest)
{
    if (nest > NEST_MAX) {
        return;
    }

    for (auto dumpCfg : dumpCfgs) {
        if (dumpCfg == nullptr) {
            continue;
        }
        if (dumpCfg->IsDumper()) {
            dumpCfg->section_ = section;
        }
        if (dumpCfg->childs_.empty()) {
            continue;
        }
        SetSection(dumpCfg->childs_, section, nest + 1);
    }
}

bool ConfigUtils::CopySmaps()
{
    DUMPER_HILOGD(MODULE_COMMON, "CopySmaps enter|");

    std::shared_ptr<RawParam> callback = dumperParam_->getClientCallback();
    if (callback == nullptr) {
        DUMPER_HILOGD(MODULE_COMMON, "CopySmaps leave|callback");
        return false;
    }

    callback->SetProgressEnabled(true);
    std::string logFolder = callback->GetFolder();
    int uid = dumperParam_->GetUid();
    for (auto &pidInfo : currentPidInfos_) {
        int newLevel = GetDumpLevelByPid(uid, pidInfo);
        if (newLevel == DumperConstant::LEVEL_NONE) {
            continue;
        }
        if (callback->IsCanceled()) {
            DUMPER_HILOGD(MODULE_COMMON, "CopySmaps debug|Canceled");
            break;
        }
        callback->UpdateProgress(0);
        std::string pid = std::to_string(pidInfo.pid_);
        std::string desfolder = logFolder + SMAPS_PATH + pidInfo.name_ + "-" + pid;
        std::string src = SMAPS_PATH_START + pid + SMAPS_PATH_END;
        std::string des = desfolder + SMAPS_PATH_END;
        ForceCreateDirectory(IncludeTrailingPathDelimiter(desfolder));
        DumpUtils::CopyFile(src, des);
    }

    DUMPER_HILOGD(MODULE_COMMON, "CopySmaps leave|true");
    return true;
}
} // namespace HiviewDFX
} // namespace OHOS
