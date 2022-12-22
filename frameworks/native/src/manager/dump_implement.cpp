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
#include "manager/dump_implement.h"
#include "iservice_registry.h"
#include "hilog_wrapper.h"
#include "util/config_utils.h"
#include "factory/cpu_dumper_factory.h"
#include "factory/file_dumper_factory.h"
#include "factory/env_param_dumper_factory.h"
#include "factory/cmd_dumper_factory.h"
#include "factory/api_dumper_factory.h"
#include "factory/properties_dumper_factory.h"
#include "factory/sa_dumper_factory.h"
#include "factory/list_dumper_factory.h"
#include "factory/version_dumper_factory.h"
#include "factory/column_rows_filter_factory.h"
#include "factory/file_format_dump_filter_factory.h"
#include "factory/fd_output_factory.h"
#include "factory/zip_output_factory.h"
#include "factory/dumper_group_factory.h"
#include "factory/memory_dumper_factory.h"
#include "dump_utils.h"
#include "string_ex.h"
#include "file_ex.h"
#include "util/string_utils.h"
#include "common/dumper_constant.h"
#include "securec.h"
namespace OHOS {
namespace HiviewDFX {
DumpImplement::DumpImplement()
{
    AddExecutorFactoryToMap();
}

DumpImplement::~DumpImplement()
{
}

void DumpImplement::AddExecutorFactoryToMap()
{
    ptrExecutorFactoryMap_ = std::make_shared<ExecutorFactoryMap>();
    ptrExecutorFactoryMap_->insert(std::make_pair(DumperConstant::CPU_DUMPER, std::make_shared<CPUDumperFactory>()));
    ptrExecutorFactoryMap_->insert(std::make_pair(DumperConstant::FILE_DUMPER, std::make_shared<FileDumperFactory>()));
    ptrExecutorFactoryMap_->insert(
        std::make_pair(DumperConstant::ENV_PARAM_DUMPER, std::make_shared<EnvParamDumperFactory>()));
    ptrExecutorFactoryMap_->insert(std::make_pair(DumperConstant::CMD_DUMPER, std::make_shared<CMDDumperFactory>()));
    ptrExecutorFactoryMap_->insert(std::make_pair(DumperConstant::API_DUMPER, std::make_shared<APIDumperFactory>()));
    ptrExecutorFactoryMap_->insert(
        std::make_pair(DumperConstant::PROPERTIES_DUMPER, std::make_shared<PropertiesDumperFactory>()));
    ptrExecutorFactoryMap_->insert(std::make_pair(DumperConstant::LIST_DUMPER, std::make_shared<ListDumperFactory>()));
    ptrExecutorFactoryMap_->insert(
        std::make_pair(DumperConstant::VERSION_DUMPER, std::make_shared<VersionDumperFactory>()));
    ptrExecutorFactoryMap_->insert(std::make_pair(DumperConstant::SA_DUMPER, std::make_shared<SADumperFactory>()));
    ptrExecutorFactoryMap_->insert(
        std::make_pair(DumperConstant::COLUMN_ROWS_FILTER, std::make_shared<ColumnRowsFilterFactory>()));
    ptrExecutorFactoryMap_->insert(
        std::make_pair(DumperConstant::FILE_FORMAT_DUMP_FILTER, std::make_shared<FileFormatDumpFilterFactory>()));
    ptrExecutorFactoryMap_->insert(std::make_pair(DumperConstant::GROUP, std::make_shared<DumperGroupFactory>()));
    ptrExecutorFactoryMap_->insert(
        std::make_pair(DumperConstant::MEMORY_DUMPER, std::make_shared<MemoryDumperFactory>()));
}

DumpStatus DumpImplement::Main(int argc, char *argv[], const std::shared_ptr<RawParam> &reqCtl)
{
    std::shared_ptr<DumperParameter> ptrDumperParameter = std::make_shared<DumperParameter>();
    ptrDumperParameter->setClientCallback(reqCtl);
    ptrDumperParameter->SetPid(reqCtl->GetPid());
    ptrDumperParameter->SetUid(reqCtl->GetUid());
    DumpStatus ret = CmdParse(argc, argv, ptrDumperParameter);
    if (ret != DumpStatus::DUMP_OK) {
        DUMPER_HILOGE(MODULE_COMMON, "Parse cmd FAIL!!!");
        return ret;
    }

    ConfigUtils::GetDumperConfigs(ptrDumperParameter);
    std::vector<std::shared_ptr<DumpCfg>> &configs = ptrDumperParameter->GetExecutorConfigList();
    DUMPER_HILOGD(MODULE_COMMON, "debug|Main configs size is %{public}zu", configs.size());
    if (configs.size() == 0) {
        DUMPER_HILOGE(MODULE_COMMON, "Executor config list is empty, so can not dump.");
        return DumpStatus::DUMP_FAIL;
    }
    bool isZip = ptrDumperParameter->GetOpts().IsDumpZip();
    std::vector<std::shared_ptr<HidumperExecutor>> hidumperExecutors;
    setExecutorList(hidumperExecutors, configs, isZip);

    if (hidumperExecutors.empty()) {
        DUMPER_HILOGE(MODULE_COMMON, "Executor list is empty, so dump fail.");
        return DumpStatus::DUMP_FAIL;
    }

    reqCtl->SetProgressEnabled(isZip);
    if (isZip) {
        reqCtl->SetTitle(",The result is:" + path_);
    } else {
        reqCtl->SetTitle("");
    }
    HidumperExecutor::StringMatrix dumpDatas = std::make_shared<std::vector<std::vector<std::string>>>();
    ret = DumpDatas(hidumperExecutors, ptrDumperParameter, dumpDatas);
    if (ret != DumpStatus::DUMP_OK) {
        DUMPER_HILOGE(MODULE_COMMON, "DUMP FAIL!!!");
        return ret;
    }
    return DumpStatus::DUMP_OK;
}

DumpStatus DumpImplement::CmdParse(int argc, char *argv[], std::shared_ptr<DumperParameter> &dumpParameter)
{
    if (argc > ARG_MAX_COUNT) {
        LOG_ERR("too many arguments(%d), limit size %d.\n", argc, ARG_MAX_COUNT);
        return DumpStatus::DUMP_FAIL;
    }
    for (int i = 0; i < argc; i++) {
        if (argv[i] == nullptr) {
            LOG_ERR("argument(%d) is null.\n", i);
            return DumpStatus::DUMP_FAIL;
        }
        size_t len = strlen(argv[i]);
        if (len == 0) {
            LOG_ERR("argument(%d) is empty.\n", i);
            return DumpStatus::DUMP_FAIL;
        }
        if (len > SINGLE_ARG_MAXLEN) {
            LOG_ERR("too long argument(%d), limit size %d.\n", i, SINGLE_ARG_MAXLEN);
            return DumpStatus::DUMP_FAIL;
        }
    }
    DumperOpts opts;
    DumpStatus status = CmdParseWithParameter(dumpParameter, argc, argv, opts);
    if (status != DumpStatus::DUMP_OK) {
        return status;
    }
    if (!opts.IsSelectAny()) {
        // 注：hidumper不添加任何参数时，dump全部内容；IPC方式dump时，仅dump 当前进程的CPU和memory情况
        int clientPid = dumpParameter->GetPid(); // to be set value
        if (IsHidumperClientProcess(clientPid)) {
            opts.AddSelectAll();
            opts.isAppendix_ = true;
        } else {
            opts.isDumpCpuFreq_ = true;
            opts.isDumpCpuUsage_ = true;
            opts.cpuUsagePid_ = clientPid;
            opts.isDumpMem_ = true;
            opts.memPid_ = clientPid;
        }
        dumpParameter->SetPid(clientPid);
    }
    dumpParameter->SetOpts(opts);
    return DumpStatus::DUMP_OK;
}

bool DumpImplement::IsHidumperClientProcess(int pid)
{
    bool ret = false;
    std::string procName;
    if (DumpCommonUtils::GetProcessNameByPid(pid, procName)) {
        ret = (procName.find("hidumper") != std::string::npos);
    }
    DUMPER_HILOGD(
        MODULE_COMMON, "debug|ret=%{public}d, pid=%{public}d, procName=%{public}s", ret, pid, procName.c_str());
    return ret;
}

DumpStatus DumpImplement::CmdParseWithParameter(int argc, char *argv[], DumperOpts &opts_)
{
    optind = 0; // reset getopt_long
    opterr = 0; // getopt not show error info
    const char optStr[] = "-ht:lcsa:ep";
    bool loop = true;
    while (loop) {
        int optionIndex = 0;
        static struct option longOptions[] = {{"cpufreq", no_argument, 0, 0},
                                              {"cpuusage", optional_argument, 0, 0},
                                              {"mem", optional_argument, 0, 0},
                                              {"net", no_argument, 0, 0},
                                              {"storage", no_argument, 0, 0},
                                              {"zip", no_argument, 0, 0},
                                              {"test", no_argument, 0, 0},
                                              {0, 0, 0, 0}};
        int c = getopt_long(argc, argv, optStr, longOptions, &optionIndex);
        if (c == -1) {
            break;
        } else if (c == 0) {
            ParseLongCmdOption(opts_, longOptions, optionIndex, argv);
        } else if (c == 'h') {
            CmdHelp();
            return DumpStatus::DUMP_HELP;
        } else if (c == '?') {
            CheckIncorrectCmdOption(optStr, argv);
            return DumpStatus::DUMP_INVALID_ARG;
        } else {
            DumpStatus status = ParseShortCmdOption(c, opts_, argc, argv);
            if (status != DumpStatus::DUMP_OK) {
                return status;
            }
        }
    }
    DumpStatus status = CheckProcessAlive(opts_);
    if (status != DumpStatus::DUMP_OK) {
        return status;
    }
    RemoveDuplicateString(opts_);
    return DumpStatus::DUMP_OK;
}

DumpStatus DumpImplement::CmdParseWithParameter(std::shared_ptr<DumperParameter> &dumpParameter, int argc, char *argv[],
                                                DumperOpts &opts)
{
    DUMPER_HILOGD(MODULE_COMMON, "enter|");
    std::lock_guard<std::mutex> lock(mutexCmdLock_); // lock for optind value safe
    ptrReqCtl_ = dumpParameter->getClientCallback();
    DumpStatus ret = CmdParseWithParameter(argc, argv, opts);
    if (ret == DumpStatus::DUMP_OK) {
        std::string errorStr;
        if (!opts.CheckOptions(errorStr)) {
            SendErrorMessage(invalidError_ + errorStr);
            ret = DumpStatus::DUMP_INVALID_ARG;
        }
    }
    ptrReqCtl_ = nullptr;
    DUMPER_HILOGD(MODULE_COMMON, "leave|ret=%{public}d", ret);
    return ret;
}

DumpStatus DumpImplement::SetCmdParameter(int argc, char *argv[], DumperOpts &opts_)
{
    DumpStatus status = DumpStatus::DUMP_OK;
    DUMPER_HILOGD(MODULE_COMMON,
                  "debug|SetCmdParameter optind is %{public}d"
                  " argc is  %{public}d",
                  optind,
                  argc);
    if (optind > 1 && optind <= argc) {
        if (StringUtils::GetInstance().IsSameStr(argv[optind - ARG_INDEX_OFFSET_LAST_OPTION], "--cpuusage")) {
            status = SetCmdIntegerParameter(argv[optind - 1], opts_.cpuUsagePid_);
        } else if (StringUtils::GetInstance().IsSameStr(argv[optind - ARG_INDEX_OFFSET_LAST_OPTION], "--log")) {
            opts_.logArgs_.push_back(argv[optind - 1]);
        } else if (StringUtils::GetInstance().IsSameStr(argv[optind - ARG_INDEX_OFFSET_LAST_OPTION], "--mem")) {
            status = SetCmdIntegerParameter(argv[optind - 1], opts_.memPid_);
        } else if (StringUtils::GetInstance().IsSameStr(argv[optind - ARG_INDEX_OFFSET_LAST_OPTION], "-c")) {
            opts_.systemArgs_.push_back(argv[optind - 1]);
        } else if (StringUtils::GetInstance().IsSameStr(argv[optind - ARG_INDEX_OFFSET_LAST_OPTION], "-p")) {
            status = SetCmdIntegerParameter(argv[optind - 1], opts_.processPid_);
        } else if (IsSADumperOption(argv)) {
            opts_.abilitieNames_.push_back(argv[optind - 1]);
        } else {
            std::string optionName = RemoveCharacterFromStr(argv[optind - 1], '-');
            std::string errorStr = unrecognizedError_ + optionName;
            SendErrorMessage(errorStr);
            return DumpStatus::DUMP_FAIL;
        }
    }
    return status;
}

std::string DumpImplement::GetTime()
{
    struct timeval curTime;
    gettimeofday(&curTime, nullptr);
    int milli = curTime.tv_usec / 1000;

    char buffer[80] = {0};
    struct tm nowTime;
    localtime_r(&curTime.tv_sec, &nowTime);
    (void)strftime(buffer, sizeof(buffer), "%Y%m%d-%H%M%S", &nowTime);

    char currentTime[84] = {0};
    if (sprintf_s(currentTime, sizeof(currentTime), "%s-%03d", buffer, milli) < 0) {
        return "";
    };

    return currentTime;
}

DumpStatus DumpImplement::ParseLongCmdOption(DumperOpts &opts_, const struct option longOptions[],
                                             const int &optionIndex, char *argv[])
{
    path_ = "";
    if (StringUtils::GetInstance().IsSameStr(longOptions[optionIndex].name, "cpufreq")) {
        opts_.isDumpCpuFreq_ = true;
    } else if (StringUtils::GetInstance().IsSameStr(longOptions[optionIndex].name, "cpuusage")) {
        opts_.isDumpCpuUsage_ = true;
    } else if (StringUtils::GetInstance().IsSameStr(longOptions[optionIndex].name, "log")) {
        opts_.isDumpLog_ = true;
    } else if (StringUtils::GetInstance().IsSameStr(longOptions[optionIndex].name, "mem")) {
        opts_.isDumpMem_ = true;
    } else if (StringUtils::GetInstance().IsSameStr(longOptions[optionIndex].name, "net")) {
        opts_.isDumpNet_ = true;
    } else if (StringUtils::GetInstance().IsSameStr(longOptions[optionIndex].name, "storage")) {
        opts_.isDumpStorage_ = true;
    } else if (StringUtils::GetInstance().IsSameStr(longOptions[optionIndex].name, "zip")) {
        path_ = ZIP_FOLDER + GetTime() + ".zip";
        opts_.path_ = path_;
    } else if (StringUtils::GetInstance().IsSameStr(longOptions[optionIndex].name, "test")) {
        opts_.isTest_ = true;
    }
    return DumpStatus::DUMP_OK;
}

DumpStatus DumpImplement::ParseShortCmdOption(int c, DumperOpts &opts_, int argc, char *argv[])
{
    switch (c) {
        case 'a': {
            if (opts_.isDumpSystemAbility_) {
                SplitStr(optarg, " ", opts_.abilitieArgs_);
            } else {
                std::string optionName = RemoveCharacterFromStr(argv[optind - 1], '-');
                std::string errorStr = unrecognizedError_ + optionName;
                SendErrorMessage(errorStr);
                return DumpStatus::DUMP_INVALID_ARG;
            }
            break;
        }
        case 'c':
            opts_.isDumpSystem_ = true;
            break;
        case 'e':
            opts_.isFaultLog_ = true;
            break;
        case 'l':
            opts_.isDumpList_ = true;
            break;
        case 's':
            opts_.isDumpSystemAbility_ = true;
            break;
        case 'p':
            opts_.isDumpProcesses_ = true;
            break;
        case 't': {
            DumpStatus timeOutStatus = SetCmdIntegerParameter(optarg, opts_.timeout_);
            if (timeOutStatus != DumpStatus::DUMP_OK) {
                return timeOutStatus;
            }
            opts_.timeout_ = (opts_.timeout_ == 0) ? INT32_MAX : opts_.timeout_;
            break;
        }
        default: {
            DumpStatus status = SetCmdParameter(argc, argv, opts_);
            if (status != DumpStatus::DUMP_OK) {
                return status;
            }
            break;
        }
    }
    return DumpStatus::DUMP_OK;
}

DumpStatus DumpImplement::SetCmdIntegerParameter(const std::string &str, int &value)
{
    if (!IsNumericStr(str)) {
        DUMPER_HILOGE(MODULE_COMMON, "Invalid string arg %{public}s", str.c_str());
        std::string errorStr = invalidError_ + str;
        SendErrorMessage(errorStr);
        return DumpStatus::DUMP_INVALID_ARG;
    }
    return StrToInt(str, value) ? DumpStatus::DUMP_OK : DumpStatus::DUMP_FAIL;
}

void DumpImplement::CmdHelp()
{
    const char *str =
        "usage:\n"
        "  -h                          |help text for the tool\n"
        "  -lc                         |a list of system information clusters\n"
        "  -ls                         |a list of system abilities\n"
        "  -c                          |all system information clusters\n"
        "  -c [base system]            |system information clusters labeled \"base\" and \"system\"\n"
        "  -s                          |all system abilities\n"
        "  -s [SA0 SA1]                |system abilities labeled \"SA0\" and \"SA1\"\n"
        "  -s [SA] -a ['-h']           |system ability labeled \"SA\" with arguments \"-h\" specified\n"
        "  -e                          |faultlogs of crash history\n"
        "  --net                       |dump network information\n"
        "  --storage                   |dump storage information\n"
        "  -p                          |processes information, include list and infromation of processes"
        " and threads\n"
        "  -p [pid]                    |dump threads under pid, includes smap, block channel,"
        " execute time, mountinfo\n"
        "  --cpuusage [pid]            |dump cpu usage by processes and category; if PID is specified,"
        " dump category usage of specified pid\n"
        "  --cpufreq                   |dump real CPU frequency of each core\n"
        "  --mem [pid]                 |dump memory usage of total; dump memory usage of specified"
        " pid if pid was specified\n"
        "  --zip                       |compress output to /data/log/hidumper\n";
    if (ptrReqCtl_ == nullptr) {
        return;
    }
    int rawParamFd = ptrReqCtl_->GetOutputFd();
    if (rawParamFd < 0) {
        return;
    }
    SaveStringToFd(rawParamFd, str);
}

void DumpImplement::setExecutorList(std::vector<std::shared_ptr<HidumperExecutor>> &executors,
                                    const std::vector<std::shared_ptr<DumpCfg>> &configs, bool isZip)
{
    std::shared_ptr<HidumperExecutor> ptrOutput;

    for (size_t i = 0; i < configs.size(); i++) {
        std::shared_ptr<ExecutorFactory> ptrExecutorFactory;
        if ((configs[i]->class_) == DumperConstant::FD_OUTPUT) {
            if (isZip) {
                ptrExecutorFactory = std::make_shared<ZipOutputFactory>();
            } else {
                ptrExecutorFactory = std::make_shared<FDOutputFactory>();
            }

            if (ptrOutput.get() == nullptr) {
                ptrOutput = ptrExecutorFactory->CreateExecutor();
            }
            ptrOutput->SetDumpConfig(configs[i]);
            executors.push_back(ptrOutput);
            continue;
        } else {
            ExecutorFactoryMap::iterator it = ptrExecutorFactoryMap_->find(configs[i]->class_);
            if (it != ptrExecutorFactoryMap_->end()) {
                ptrExecutorFactory = it->second;
            }
        }

        if (ptrExecutorFactory.get() == nullptr) {
            DUMPER_HILOGE(MODULE_COMMON, "configs[%{public}zu].class_ is %{public}d", i, configs[i]->class_);
            continue;
        }
        std::shared_ptr<HidumperExecutor> ptrExecutor = ptrExecutorFactory->CreateExecutor();
        if (ptrExecutor != nullptr) {
            configs[i]->executor_ = ptrExecutor;
            ptrExecutor->SetDumpConfig(configs[i]);
        }
        executors.push_back(ptrExecutor);
    }

    // must clear.
    for (auto cfg : configs) {
        cfg->executor_ = nullptr;
    }
}

DumpStatus DumpImplement::DumpDatas(const std::vector<std::shared_ptr<HidumperExecutor>> &executors,
                                    const std::shared_ptr<DumperParameter> &dumpParameter,
                                    HidumperExecutor::StringMatrix dumpDatas)
{
    auto callback = dumpParameter->getClientCallback();

    std::string groupName = "";
    std::vector<size_t> loopStack;
    const size_t executorSum = executors.size();
    for (size_t index = 0; index < executorSum; index++) {
        callback->UpdateProgress(executors.size(), index);
        if (callback->IsCanceled()) {
            break;
        }

        auto dumpCfg = executors[index]->GetDumpConfig();
        if (dumpCfg->IsDumper() && CheckGroupName(groupName, dumpCfg->section_)) {
            AddGroupTitle(groupName, dumpDatas);
        }

        DumpStatus ret = DumpStatus::DUMP_FAIL;
        ret = executors[index]->DoPreExecute(dumpParameter, dumpDatas);
        if (ret != DumpStatus::DUMP_OK) {
            continue;
        }

        ret = executors[index]->DoExecute();
        if ((ret != DumpStatus::DUMP_OK) && (ret != DumpStatus::DUMP_MORE_DATA)) {
            continue;
        }

        ret = executors[index]->DoAfterExecute();
        if (dumpCfg->IsDumper() && dumpCfg->CanLoop() && (ret == DumpStatus::DUMP_MORE_DATA)) {
            loopStack.push_back(index);
        }

        if (dumpCfg->IsOutput() || dumpCfg->IsGroup()) {
            if (!loopStack.empty()) {
                index = loopStack.back() - 1; // the 1 will add back by end for.
            }
            loopStack.clear(); // clear now.
        }
    }
    for (auto executor : executors) {
        executor->Reset();
    }
    callback->UpdateProgress(executors.size(), executors.size());
    return DumpStatus::DUMP_OK;
}

void DumpImplement::AddGroupTitle(const std::string &groupName, HidumperExecutor::StringMatrix dumpDatas)
{
    /**
     * @brief The group title is followed
     * '
     * -------------------------------[groupName]-------------------------------
     * '
     */
    std::vector<std::string> lineData;
    lineData.push_back("");
    dumpDatas->push_back(lineData);
    std::vector<std::string>().swap(lineData);
    lineData.push_back("-------------------------------[");
    lineData.push_back(groupName);
    lineData.push_back("]-------------------------------");
    dumpDatas->push_back(lineData);
    std::vector<std::string>().swap(lineData);
    lineData.push_back("");
    dumpDatas->push_back(lineData);
    std::vector<std::string>().swap(lineData);
}

bool DumpImplement::CheckGroupName(std::string &lastName, const std::string &curName)
{
    if (curName.compare("") == 0) {
        return false;
    }

    if (lastName.compare(curName) == 0) {
        return false;
    }

    lastName.assign(curName);
    return true;
}

const sptr<ISystemAbilityManager> DumpImplement::GetSystemAbilityManager()
{
    sam_ = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam_ == nullptr) {
        DUMPER_HILOGE(MODULE_COMMON, "SystemAbilityManager not found.");
    }
    return sam_;
}

void DumpImplement::CheckIncorrectCmdOption(const char *optStr, char *argv[])
{
    if (optopt == 0) {
        SendErrorMessage(unrecognizedError_ + RemoveCharacterFromStr(argv[optind - 1], '-'));
    } else if (!IsShortOptionReqArg(optStr)) {
        std::string errorStr = unrecognizedError_;
        errorStr += optopt;
        SendErrorMessage(errorStr);
    }
}

bool DumpImplement::IsShortOptionReqArg(const char *optStr)
{
    int len = strlen(optStr);
    for (int i = 0; i < len; i++) {
        if (optStr[i] == optopt) {
            SendErrorMessage(requireError_ + optStr[i]);
            return true;
        }
    }
    return false;
}

void DumpImplement::SendErrorMessage(const std::string &errorStr)
{
    if (ptrReqCtl_ == nullptr) {
        return;
    }
    int rawParamFd = ptrReqCtl_->GetOutputFd();
    if (rawParamFd < 0) {
        return;
    }
    SaveStringToFd(rawParamFd, errorStr + "\n");
}

void DumpImplement::SendPidErrorMessage(int pid)
{
    if (ptrReqCtl_ == nullptr) {
        return;
    }
    int rawParamFd = ptrReqCtl_->GetOutputFd();
    if (rawParamFd < 0) {
        return;
    }
    dprintf(rawParamFd, pidError_.c_str(), pid);
}

std::string DumpImplement::RemoveCharacterFromStr(const std::string &str, const char character)
{
    std::string strTmp = str;
    while (strTmp.find(character) != std::string::npos) {
        strTmp.erase(strTmp.find(character), 1);
    }
    return strTmp;
}

bool DumpImplement::IsLongOption(const std::string &str, const struct option longOptions[], int size)
{
    for (int i = 0; i < (size - 1); i++) {
        if (StringUtils::GetInstance().IsSameStr(str, longOptions[i].name)) {
            return true;
        }
    }
    return false;
}

bool DumpImplement::IsSADumperOption(char *argv[])
{
    for (int i = optind - 2; i > 0; i--) {
        if (IsSubStr(argv[i], "-")) {
            return StringUtils::GetInstance().IsSameStr(argv[i], "-s")
                   || StringUtils::GetInstance().IsSameStr(argv[i], "-a");
        }
    }
    return false;
}

DumpStatus DumpImplement::CheckProcessAlive(const DumperOpts &opts_)
{
    if ((opts_.cpuUsagePid_ > -1) && !DumpUtils::CheckProcessAlive(opts_.cpuUsagePid_)) {
        SendPidErrorMessage(opts_.cpuUsagePid_);
        return DumpStatus::DUMP_FAIL;
    }
    if ((opts_.memPid_ > -1) && !DumpUtils::CheckProcessAlive(opts_.memPid_)) {
        SendPidErrorMessage(opts_.memPid_);
        return DumpStatus::DUMP_FAIL;
    }
    if ((opts_.processPid_ > -1) && !DumpUtils::CheckProcessAlive(opts_.processPid_)) {
        SendPidErrorMessage(opts_.processPid_);
        return DumpStatus::DUMP_FAIL;
    }
    return DumpStatus::DUMP_OK;
}

void DumpImplement::RemoveDuplicateString(DumperOpts &opts_)
{
    DumpUtils::RemoveDuplicateString(opts_.logArgs_);       // remove duplicate log names
    DumpUtils::RemoveDuplicateString(opts_.systemArgs_);    // remove duplicate system names
    DumpUtils::RemoveDuplicateString(opts_.abilitieNames_); // remove duplicate ability names
}
} // namespace HiviewDFX
} // namespace OHOS
