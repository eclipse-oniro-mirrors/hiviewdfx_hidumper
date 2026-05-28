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
#include <ipc_skeleton.h>
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
#include "factory/jsheap_memory_dumper_factory.h"
#include "factory/cjheap_memory_dumper_factory.h"
#include "factory/heap_memory_dumper_factory.h"
#include "factory/event_list_dumper_factory.h"
#include "factory/event_detail_dumper_factory.h"
#include "factory/fd_thread_dumper_factory.h"
#include "factory/traffic_dumper_factory.h"
#include "factory/ipc_stat_dumper_factory.h"
#include "dump_utils.h"
#include "string_ex.h"
#include "file_ex.h"
#include "util/string_utils.h"
#include "common/dumper_constant.h"
#include "securec.h"
#include "parameters.h"
#include "parameter.h"
#ifdef HIDUMPER_HIVIEWDFX_HISYSEVENT_ENABLE
#include "hisysevent.h"
#endif
#include "manager/dump_manager.h"

#include <unordered_set>
namespace OHOS {
namespace HiviewDFX {
static struct option LONG_OPTIONS[] = {{"cpufreq", no_argument, 0, 0},
#ifdef HIDUMPER_HIVIEWDFX_HIVIEW_ENABLE
    {"cpuusage", optional_argument, 0, 0},
#endif
    {"mem", optional_argument, 0, 0},
    {"net", no_argument, 0, 0},
    {"storage", no_argument, 0, 0},
    {"zip", no_argument, 0, 0},
    {"mem-smaps", required_argument, 0, 0},
    {"mem-jsheap", required_argument, 0, 0},
    {"mem-cjheap", required_argument, 0, 0},
    {"mem-heap", optional_argument, 0, 0},
    {"native", optional_argument, 0, 0},
    {"kotlin", optional_argument, 0, 0},
    #ifdef HIDUMPER_HIVIEWDFX_PLUGIN_ENABLE
    {"jsvm", optional_argument, 0, 0},
    {"arkweb-js", optional_argument, 0, 0},
    #endif
    {"gc", no_argument, 0, 0},
    {"leakobj", no_argument, 0, 0},
    {"clean", no_argument, 0, 0},
    {"raw", no_argument, 0, 0},
    {"single", no_argument, 0, 0},
    {"prune", no_argument, 0, 0},
    {"show-ashmem", no_argument, 0, 0},
    {"show-dmabuf", no_argument, 0, 0},
    #ifdef HIDUMPER_HIVIEWDFX_PLUGIN_ENABLE
    {"show-gpumem", no_argument, 0, 0},
    #endif
    {"ipc", optional_argument, 0, 0},
    {"start-stat", no_argument, 0, 0},
    {"stop-stat", no_argument, 0, 0},
    {"stat", no_argument, 0, 0},
    {"list", no_argument, 0, 0},
    {"print", no_argument, 0, 0},
    {"since", required_argument, 0, 0},
    {"until", required_argument, 0, 0},
    {"fd", no_argument, 0, 0},
    {"thread", no_argument, 0, 0},
    {0, 0, 0, 0}};

thread_local std::unique_ptr<DumperSysEventParams> DumpImplement::dumperSysEventParams_{nullptr};

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
#ifdef HIDUMPER_HIVIEWDFX_HIVIEW_ENABLE
    ptrExecutorFactoryMap_->insert(std::make_pair(DumperConstant::CPU_DUMPER, std::make_shared<CPUDumperFactory>()));
#endif
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
    ptrExecutorFactoryMap_->insert(
        std::make_pair(DumperConstant::JSHEAP_MEMORY_DUMPER, std::make_shared<JsHeapMemoryDumperFactory>()));
    ptrExecutorFactoryMap_->insert(
        std::make_pair(DumperConstant::CJHEAP_MEMORY_DUMPER, std::make_shared<CjHeapMemoryDumperFactory>()));
    ptrExecutorFactoryMap_->insert(
        std::make_pair(DumperConstant::HEAP_MEMORY_DUMPER, std::make_shared<HeapMemoryDumperFactory>()));
    ptrExecutorFactoryMap_->insert(
        std::make_pair(DumperConstant::EVENT_LIST_DUMPER, std::make_shared<EventListDumperFactory>()));
    ptrExecutorFactoryMap_->insert(
        std::make_pair(DumperConstant::EVENT_DETAIL_DUMPER, std::make_shared<EventDetailDumperFactory>()));
    ptrExecutorFactoryMap_->insert(
        std::make_pair(DumperConstant::FD_THREAD_DUMPER, std::make_shared<FdThreadDumperFactory>()));
    ptrExecutorFactoryMap_->insert(
        std::make_pair(DumperConstant::TRAFFIC_DUMPER, std::make_shared<TrafficDumperFactory>()));
    ptrExecutorFactoryMap_->insert(
        std::make_pair(DumperConstant::IPC_STAT_DUMPER, std::make_shared<IPCStatDumperFactory>()));
}

DumpStatus DumpImplement::Main(int argc, char *argv[], const std::shared_ptr<RawParam> &reqCtl)
{
    std::shared_ptr<DumperParameter> ptrDumperParameter = std::make_shared<DumperParameter>();
    DumpStatus ret = InitHandle(argc, argv, reqCtl, ptrDumperParameter);
    if (ret != DumpStatus::DUMP_OK) {
        return ret;
    }
    if (IsNewStructSupport(ptrDumperParameter)) {
        DumpContext context = DumpContext(reqCtl->GetUid(), reqCtl->GetPid(), reqCtl->GetOutputFd());
        return DumpManager::GetInstance().StartDump(argc, argv, context);
    }
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
    isZip ? reqCtl->SetTitle(",The result is:" + path_) : reqCtl->SetTitle("");
    HidumperExecutor::StringMatrix dumpDatas = std::make_shared<std::vector<std::vector<std::string>>>();
    ret = DumpDatas(hidumperExecutors, ptrDumperParameter, dumpDatas);
    std::lock_guard<std::mutex> lock(mutexCmdLock_); // lock for dumperSysEventParams_
    if (ret != DumpStatus::DUMP_OK) {
        DUMPER_HILOGE(MODULE_COMMON, "DUMP FAIL!!!");
        dumperSysEventParams_->errorCode = static_cast<int32_t>(ret);
        dumperSysEventParams_->errorMsg = "dump fail";
        DumpCommonUtils::ReportCmdUsage(dumperSysEventParams_);
        return ret;
    }
    DumpCommonUtils::ReportCmdUsage(dumperSysEventParams_);
    return DumpStatus::DUMP_OK;
}

bool DumpImplement::IsNewStructSupport(std::shared_ptr<DumperParameter> ptrDumperParameter)
{
    return false;
}

DumpStatus DumpImplement::InitHandle(int argc, char *argv[], const std::shared_ptr<RawParam> &reqCtl,
    std::shared_ptr<DumperParameter>& ptrDumperParameter)
{
    ptrDumperParameter->setClientCallback(reqCtl);
    ptrDumperParameter->SetPid(reqCtl->GetPid());
    ptrDumperParameter->SetUid(reqCtl->GetUid());
    std::lock_guard<std::mutex> lock(mutexCmdLock_); // lock for optind value safe
    dumperSysEventParams_ = std::make_unique<DumperSysEventParams>();
    dumperSysEventParams_->errorCode = 0;
    dumperSysEventParams_->callerPpid = -1;
    DumpStatus ret = CmdParse(argc, argv, ptrDumperParameter);
    if (ret != DumpStatus::DUMP_OK) {
        DUMPER_HILOGE(MODULE_COMMON, "Parse cmd FAIL!!!");
        dumperSysEventParams_->errorCode = static_cast<int32_t>(ret);
        dumperSysEventParams_->errorMsg = "parse cmd fail";
        DumpCommonUtils::ReportCmdUsage(dumperSysEventParams_);
        return ret;
    }
    ConfigUtils::GetDumperConfigs(ptrDumperParameter);
    return DumpStatus::DUMP_OK;
}

void DumpImplement::ProcessDumpOptions(int clientPid, std::shared_ptr<DumperParameter> &dumpParameter, DumperOpts &opts)
{
    if (IsHidumperClientProcess(clientPid)) {
        opts.AddSelectAll();
        opts.isAppendix_ = true;
    } else {
        opts.isDumpCpuFreq_ = true;
#ifdef HIDUMPER_HIVIEWDFX_HIVIEW_ENABLE
        opts.isDumpCpuUsage_ = true;
        opts.cpuUsagePid_ = clientPid;
#endif
        opts.isDumpMem_ = true;
        opts.memPid_ = clientPid;
    }
    dumpParameter->SetPid(clientPid);
}

DumpStatus DumpImplement::CheckArgs(int argc, char* argv[])
{
    std::stringstream dumpCmdSs;
    if (argc > ARG_MAX_COUNT) {
        DUMPER_HILOGE(MODULE_COMMON, "too many arguments(%{public}d), limit size %{public}d.", argc, ARG_MAX_COUNT);
        return DumpStatus::DUMP_FAIL;
    }
    for (int i = 0; i < argc; i++) {
        if (argv[i] == nullptr) {
            DUMPER_HILOGE(MODULE_COMMON, "argument(%{public}d) is null.", i);
            return DumpStatus::DUMP_FAIL;
        }
        size_t len = strlen(argv[i]);
        if (len == 0) {
            DUMPER_HILOGE(MODULE_COMMON, "argument(%{public}d) is empty.", i);
            return DumpStatus::DUMP_FAIL;
        }
        if (len > SINGLE_ARG_MAXLEN) {
            DUMPER_HILOGE(MODULE_COMMON, "too long args:%{public}zu, limit size:%{public}d.", len, SINGLE_ARG_MAXLEN);
            return DumpStatus::DUMP_FAIL;
        }
        dumpCmdSs << argv[i] << " ";
    }
    if (dumpCmdSs.str().length() > 0) {
        dumperSysEventParams_->arguments = dumpCmdSs.str().substr(0, dumpCmdSs.str().length() - 1);
    }
    return DumpStatus::DUMP_OK;
}

DumpStatus DumpImplement::CmdParse(int argc, char *argv[], std::shared_ptr<DumperParameter> &dumpParameter)
{
    if (CheckArgs(argc, argv) != DumpStatus::DUMP_OK)
        return DumpStatus::DUMP_FAIL;
    DumperOpts opts;
    DumpStatus status = CmdParseWithParameter(dumpParameter, argc, argv, opts);
    if (status != DumpStatus::DUMP_OK)
        return status;
    if (!opts.IsSelectAny()) { // 注：hidumper不添加任何参数时，dump全部内容；IPC方式dump时，仅dump 当前进程的CPU和memory情况
        int clientPid = dumpParameter->GetPid(); // to be set value
        ProcessDumpOptions(clientPid, dumpParameter, opts);
    }
    ReportJsheap(opts);
    ReportCjheap(opts);
    ReportMemheap(opts);
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

DumpStatus DumpImplement::CmdParseWithParameter(int argc, char *argv[], DumperOpts &opts)
{
    optind = 0; // reset getopt_long
    opterr = 0; // getopt not show error info
    const char optStr[] = "-hlcsa:epvT:t:n:";
    bool loop = true;
    while (loop) {
        int optionIndex = 0;
        int c = getopt_long(argc, argv, optStr, LONG_OPTIONS, &optionIndex);
        DUMPER_HILOGD(MODULE_COMMON, "debug|getopt_long(), c=%{public}d, optind=%{public}d", c, optind);
        if (c == -1) {
            break;
        } else if (c == 0) {
            DumpStatus status = ParseLongCmdOption(argc, opts, LONG_OPTIONS, optionIndex, argv);
            if (status != DumpStatus::DUMP_OK) {
                return status;
            }
        } else if (c == 'h') {
            CmdHelp();
            return DumpStatus::DUMP_HELP;
        } else if (c == '?') {
            CheckIncorrectCmdOption(optStr, argv);
            return DumpStatus::DUMP_INVALID_ARG;
        } else {
            DumpStatus status = ParseShortCmdOption(c, opts, argc, argv);
            if (status != DumpStatus::DUMP_OK) {
                return status;
            }
        }
    }
    DumpStatus status = CheckProcessAlive(opts);
    if (status != DumpStatus::DUMP_OK) {
        return status;
    }
    if (!CheckDumpPermission(opts)) {
        if (!opts.isShowSmaps_ || !opts.isDumpMem_) {
            CmdHelp();
        }
        return DumpStatus::DUMP_HELP;
    }
    if (opts.isDumpJsHeapMem_ && !CheckJsHeapSingleParam(opts)) {
        return DumpStatus::DUMP_FAIL;
    }
    if (CheckUnableToDumpAll(argc, opts) || (opts.isDumpHeapMem_ && !CheckDumpHeapMemParameter(argc, opts))) {
        CmdHelp();
        return DumpStatus::DUMP_HELP;
    }
    RemoveDuplicateString(opts);
    return DumpStatus::DUMP_OK;
}

DumpStatus DumpImplement::CmdParseWithParameter(std::shared_ptr<DumperParameter> &dumpParameter, int argc, char *argv[],
                                                DumperOpts &opts)
{
    DUMPER_HILOGD(MODULE_COMMON, "enter|");
    ptrReqCtl_ = dumpParameter->getClientCallback();
    dumperSysEventParams_->callerPpid = ptrReqCtl_->GetCallerPpid();
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

DumpStatus DumpImplement::SetCmdParameter(int argc, char *argv[], DumperOpts &opts)
{
    DumpStatus status = DumpStatus::DUMP_OK;
    DUMPER_HILOGD(MODULE_COMMON,
                  "debug|SetCmdParameter optind is %{public}d"
                  " argc is  %{public}d",
                  optind,
                  argc);
    if (optind > 1 && optind <= argc) {
        std::string optionName = argv[optind - ARG_INDEX_OFFSET_LAST_OPTION];
        std::string optionValue = argv[optind - 1];
        status = HandleOptionParameter(optionName, optionValue, opts, argv);
        if (status == DumpStatus::DUMP_FAIL) {
            return status;
        }
    }
    return status;
}

DumpStatus DumpImplement::HandleOptionParameter(const std::string &optionName,
                                                const std::string &optionValue, DumperOpts &opts, char *argv[])
{
    DumpStatus status = DumpStatus::DUMP_OK;
    bool hiviewEnable = false;
#ifdef HIDUMPER_HIVIEWDFX_HIVIEW_ENABLE
    hiviewEnable = true;
#endif

    if (hiviewEnable && optionName == "--cpuusage") {
        status = SetCmdIntegerParameter(optionValue, opts.cpuUsagePid_);
    } else if (optionName == "--mem") {
        if (optionValue == "SIGINT") {
            opts.isReceivedSigInt_ = true;
        } else {
            status = SetCmdIntegerParameter(optionValue, opts.memPid_);
        }
    } else if (optionName == "--net") {
        status = SetCmdIntegerParameter(optionValue, opts.netPid_);
    } else if (optionName == "--storage") {
        status = SetCmdIntegerParameter(optionValue, opts.storagePid_);
    } else if (optionName == "-c") {
        opts.systemArgs_.push_back(optionValue);
        dumperSysEventParams_->subOpt = optionValue;
    } else if (optionName == "-p") {
        status = SetCmdIntegerParameter(optionValue, opts.processPid_);
    } else if (optionName == "-T") {
        status = SetCmdIntegerParameter(optionValue, opts.threadId_);
    } else if (optionName == "-t") {
        status = SetCmdIntegerParameter(optionValue, opts.timeInterval_);
    } else if (optionName == "-n") {
        status = SetShowCount(optionValue, opts.showEventCount_);
    } else if (optionName == "--list") {
        opts.processName_ = optionValue;
    } else if (optionName == "--print") {
        if (IsNumericStr(optionValue)) {
            opts.eventId_ = optionValue;
        } else {
            opts.processName_ = optionValue;
        }
    } else if (IsSADumperOption(argv)) {
        opts.abilitieNames_.push_back(optionValue);
        dumperSysEventParams_->target += optionValue;
    } else if (optionName == "--ipc") {
        status = SetCmdIntegerParameter(optionValue, opts.ipcStatPid_);
    } else if (optionName == "--mem-heap") {
        status = SetCmdIntegerParameter(optionValue, opts.dumpHeapMemPid_);
    } else if (optionName == "--native") {
        status = SetCmdIntegerParameter(optionValue, opts.dumpHeapArgPid_);
    } else if (optionName == "--kotlin") {
        status = SetCmdIntegerParameter(optionValue, opts.dumpHeapArgPid_);
    }
    #ifdef HIDUMPER_HIVIEWDFX_PLUGIN_ENABLE
    else if (optionName == "--jsvm") {
        status = SetCmdIntegerParameter(optionValue, opts.dumpHeapArgPid_);
    } else if (optionName == "--arkweb-js") {
        status = SetCmdIntegerParameter(optionValue, opts.dumpHeapArgPid_);
    }
    #endif
    else {
        SendErrorMessageIf(opts, optionValue);
        return DumpStatus::DUMP_FAIL;
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

bool DumpImplement::ParseEventCmdOption(DumperOpts &opts, const std::string& param)
{
    if (!opts.isFaultLog_) {
        return false;
    }
    static const std::unordered_set<std::string> validParams = {
        "list", "print", "since", "until"
    };
    return validParams.find(param) != validParams.end();
}

DumpStatus DumpImplement::SetEventParam(DumperOpts &opts, const std::string& param)
{
    if (StringUtils::GetInstance().IsSameStr(param, "list")) {
        opts.isEventList_ = true;
        dumperSysEventParams_->subOpt = "list";
        return DumpStatus::DUMP_OK;
    } else if (StringUtils::GetInstance().IsSameStr(param, "print")) {
        opts.isEventDetail_ = true;
        dumperSysEventParams_->subOpt = "print";
        return DumpStatus::DUMP_OK;
    } else if (StringUtils::GetInstance().IsSameStr(param, "since")) {
        if (optarg == nullptr) {
            DUMPER_HILOGE(MODULE_COMMON, "since option miss argument");
            return DumpStatus::DUMP_FAIL;
        }
        opts.startTime_ = StringUtils::GetInstance().StringToUnixMs(optarg);
        if (opts.startTime_ < 0) {
            DUMPER_HILOGE(MODULE_COMMON, "Invalid string arg %{public}s", optarg);
            std::string errorStr = invalidError_ + optarg;
            SendErrorMessage(errorStr);
            return DumpStatus::DUMP_INVALID_ARG;
        }
        return DumpStatus::DUMP_OK;
    } else if (StringUtils::GetInstance().IsSameStr(param, "until")) {
        if (optarg == nullptr) {
            DUMPER_HILOGE(MODULE_COMMON, "until option miss argument");
            return DumpStatus::DUMP_FAIL;
        }
        opts.endTime_ = StringUtils::GetInstance().StringToUnixMs(optarg);
        if (opts.endTime_ < 0) {
            DUMPER_HILOGE(MODULE_COMMON, "Invalid string arg %{public}s", optarg);
            std::string errorStr = invalidError_ + optarg;
            SendErrorMessage(errorStr);
            return DumpStatus::DUMP_INVALID_ARG;
        }
        return DumpStatus::DUMP_OK;
    }
    return DumpStatus::DUMP_FAIL;
}

bool DumpImplement::ParseSubLongCmdOption(int argc, DumperOpts &opts, const struct option longOptions[],
                                          const int &optionIndex, char *argv[])
{
    bool hiviewEnable = false;
#ifdef HIDUMPER_HIVIEWDFX_HIVIEW_ENABLE
    hiviewEnable = true;
#endif
    if (StringUtils::GetInstance().IsSameStr(longOptions[optionIndex].name, "cpufreq")) {
        opts.isDumpCpuFreq_ = true;
        dumperSysEventParams_->opt = "cpufreq";
    } else if (hiviewEnable && StringUtils::GetInstance().IsSameStr(longOptions[optionIndex].name, "cpuusage")) {
        opts.isDumpCpuUsage_ = true;
        dumperSysEventParams_->opt = "cpuusage";
    } else if (StringUtils::GetInstance().IsSameStr(longOptions[optionIndex].name, "mem")) {
        opts.isDumpMem_ = true;
        dumperSysEventParams_->opt = "mem";
    } else if (StringUtils::GetInstance().IsSameStr(longOptions[optionIndex].name, "net")) {
        opts.isDumpNet_ = true;
        dumperSysEventParams_->opt = "net";
    } else if (StringUtils::GetInstance().IsSameStr(longOptions[optionIndex].name, "storage")) {
        opts.isDumpStorage_ = true;
        dumperSysEventParams_->opt = "storage";
    } else if (StringUtils::GetInstance().IsSameStr(longOptions[optionIndex].name, "zip")) {
        path_ = ZIP_FOLDER + GetTime() + ".zip";
        opts.path_ = path_;
    } else {
        return false;
    }

    return true;
}

DumpStatus DumpImplement::ParseLongCmdOption(int argc, DumperOpts &opts, const struct option longOptions[],
                                             const int &optionIndex, char *argv[])
{
    if (ParseSubLongCmdOption(argc, opts, longOptions, optionIndex, argv)) {
        return DumpStatus::DUMP_OK;
    } else if (StringUtils::GetInstance().IsSameStr(longOptions[optionIndex].name, "mem-smaps")) {
        DumpStatus status = SetMemSmapsParam(opts, argc, argv);
        if (status != DumpStatus::DUMP_OK) {
            return status;
        }
    } else if (StringUtils::GetInstance().IsSameStr(longOptions[optionIndex].name, "show-ashmem")) {
        opts.showAshmem_ = true;
    } else if (StringUtils::GetInstance().IsSameStr(longOptions[optionIndex].name, "show-dmabuf")) {
        opts.showDmaBuf_ = true;
    } 
    #ifdef HIDUMPER_HIVIEWDFX_PLUGIN_ENABLE
    else if (StringUtils::GetInstance().IsSameStr(longOptions[optionIndex].name, "show-gpumem")) {
        opts.showGpumem_ = true;
    } 
    #endif
    else if (StringUtils::GetInstance().IsSameStr(longOptions[optionIndex].name, "fd")) {
        opts.isDumpFd_ = true;
    } else if (StringUtils::GetInstance().IsSameStr(longOptions[optionIndex].name, "thread")) {
        opts.isDumpThread_ = true;
    } else if (StringUtils::GetInstance().IsSameStr(longOptions[optionIndex].name, "mem-jsheap")) {
        return SetMemJsheapParam(opts);
    } else if (StringUtils::GetInstance().IsSameStr(longOptions[optionIndex].name, "mem-cjheap")) {
        return SetMemCjheapParam(opts);
    } else if (StringUtils::GetInstance().IsSameStr(longOptions[optionIndex].name, "mem-heap")) {
        return SetMemHeapParam(opts);
    } else if (StringUtils::GetInstance().IsSameStr(longOptions[optionIndex].name, "native")) {
        return SetNativeParam(opts);
    } else if (StringUtils::GetInstance().IsSameStr(longOptions[optionIndex].name, "kotlin")) {
        return SetKotlinParam(opts);
    }
    #ifdef HIDUMPER_HIVIEWDFX_PLUGIN_ENABLE
    else if (StringUtils::GetInstance().IsSameStr(longOptions[optionIndex].name, "jsvm")) {
        return SetJsvmParam(opts);
    } else if (StringUtils::GetInstance().IsSameStr(longOptions[optionIndex].name, "arkweb-js")) {
        return SetArkwebJsParam(opts);
    }
    #endif
    else if (StringUtils::GetInstance().IsSameStr(longOptions[optionIndex].name, "raw")) {
        return SetRawParam(opts);
    } else if (StringUtils::GetInstance().IsSameStr(longOptions[optionIndex].name, "prune")) {
        return SetMemPruneParam(opts);
    } else if (StringUtils::GetInstance().IsSameStr(longOptions[optionIndex].name, "gc")) {
        return SetGCParam(opts);
    } else if (StringUtils::GetInstance().IsSameStr(longOptions[optionIndex].name, "leakobj")) {
        return SetLeakobjParam(opts);
    } else if (StringUtils::GetInstance().IsSameStr(longOptions[optionIndex].name, "single")) {
        return SetHeapCombineParam(opts);
    } else if (StringUtils::GetInstance().IsSameStr(longOptions[optionIndex].name, "clean")) {
        return SetCleanParam(opts);
    } else if (StringUtils::GetInstance().IsSameStr(longOptions[optionIndex].name, "ipc")) {
        opts.isDumpIpc_ = true;
        dumperSysEventParams_->opt = "ipc";
        if (IPC_STAT_ARG_NUMS != argc) {
            DUMPER_HILOGE(MODULE_COMMON, "ipc stat cmd args invalid");
            SendErrorMessage("ipc stat cmd args invalid\n");
            CmdHelp();
            return DumpStatus::DUMP_HELP;
        }
    } else if (SetIpcStatParam(opts, longOptions[optionIndex].name)) {
        if (!opts.isDumpIpc_) {
            DUMPER_HILOGE(MODULE_COMMON, "ipc stat param invalid");
            SendErrorMessage("ipc stat cmd args invalid\n");
            CmdHelp();
            return DumpStatus::DUMP_HELP;
        }
    } else if (ParseEventCmdOption(opts, longOptions[optionIndex].name)) {
        return SetEventParam(opts, longOptions[optionIndex].name);
    } else {
        DUMPER_HILOGE(MODULE_COMMON, "ParseLongCmdOption %{public}s", longOptions[optionIndex].name);
    }
    return DumpStatus::DUMP_OK;
}

DumpStatus DumpImplement::SetMemSmapsParam(DumperOpts &opt, int argc, char *argv[])
{
    opt.isShowSmaps_ = true;
    dumperSysEventParams_->opt = "mem-smaps";
    DumpStatus status;
    if (ARG_INDEX_OFFSET_LAST_OPTION < 0 || ARG_INDEX_OFFSET_LAST_OPTION >= argc) {
        status = DumpStatus::DUMP_FAIL;
    } else {
        status = SetCmdIntegerParameter(argv[ARG_INDEX_OFFSET_LAST_OPTION], opt.memPid_);
    }
    return status;
}

DumpStatus DumpImplement::SetMemJsheapParam(DumperOpts &opt)
{
    opt.isDumpJsHeapMem_ = true;
    dumperSysEventParams_->opt = "mem-jsheap";
    if (optarg == nullptr) {
        DUMPER_HILOGE(MODULE_COMMON, "mem-jsheap nullptr");
        return DumpStatus::DUMP_FAIL;
    }
    return SetCmdIntegerParameter(optarg, opt.dumpJsHeapMemPid_);
}

DumpStatus DumpImplement::SetMemCjheapParam(DumperOpts &opt)
{
    opt.isDumpCjHeapMem_ = true;
    dumperSysEventParams_->opt = "mem-cjheap";
    if (optarg == nullptr) {
        DUMPER_HILOGE(MODULE_COMMON, "mem-cjheap nullptr");
        return DumpStatus::DUMP_FAIL;
    }
    return SetCmdIntegerParameter(optarg, opt.dumpCjHeapMemPid_);
}

DumpStatus DumpImplement::SetMemHeapParam(DumperOpts &opt)
{
    opt.isDumpHeapMem_ = true;
    dumperSysEventParams_->opt = "mem-heap";
    return DumpStatus::DUMP_OK;
}

DumpStatus DumpImplement::SetNativeParam(DumperOpts &opt)
{
    DumpStatus status = DumpStatus::DUMP_FAIL;
    if (opt.isDumpHeapMem_) {
        DUMPER_HILOGI(MODULE_COMMON, "SetNativeParam success");
        opt.isDumpHeapNative_ = true;
        status = DumpStatus::DUMP_OK;
    } else {
        DUMPER_HILOGE(MODULE_COMMON, "native param invalid");
        SendErrorMessage("native param invalid\n");
        CmdHelp();
        status = DumpStatus::DUMP_HELP;
    }
    return status;
}

DumpStatus DumpImplement::SetKotlinParam(DumperOpts &opt)
{
    DumpStatus status = DumpStatus::DUMP_FAIL;
    if (opt.isDumpHeapMem_) {
        DUMPER_HILOGI(MODULE_COMMON, "SetKotlinParam success");
        opt.isDumpHeapKotlin_ = true;
        status = DumpStatus::DUMP_OK;
    } else {
        DUMPER_HILOGE(MODULE_COMMON, "kotlin param invalid");
        SendErrorMessage("kotlin param invalid\n");
        CmdHelp();
        status =  DumpStatus::DUMP_HELP;
    }
    return status;
}

DumpStatus DumpImplement::SetJsvmParam(DumperOpts &opt)
{
    DumpStatus status = DumpStatus::DUMP_FAIL;
    if (opt.isDumpHeapMem_) {
        DUMPER_HILOGI(MODULE_COMMON, "SetJsvmParam success");
        opt.isDumpHeapJsvm_ = true;
        status = DumpStatus::DUMP_OK;
    } else {
        DUMPER_HILOGE(MODULE_COMMON, "jsvm param invalid");
        SendErrorMessage("jsvm param invalid\n");
        CmdHelp();
        status =  DumpStatus::DUMP_HELP;
    }
    return status;
}

DumpStatus DumpImplement::SetArkwebJsParam(DumperOpts &opt)
{
    DumpStatus status = DumpStatus::DUMP_FAIL;
    if (opt.isDumpHeapMem_) {
        DUMPER_HILOGI(MODULE_COMMON, "SetArkwebJsParam success");
        opt.isDumpHeapArkwebJs_ = true;
        status = DumpStatus::DUMP_OK;
    } else {
        DUMPER_HILOGE(MODULE_COMMON, "arkweb-js param invalid");
        SendErrorMessage("arkweb-js param invalid\n");
        CmdHelp();
        status = DumpStatus::DUMP_HELP;
    }
    return status;
}

DumpStatus DumpImplement::SetRawParam(DumperOpts &opt)
{
    DumpStatus status = DumpStatus::DUMP_FAIL;
    if (opt.isDumpJsHeapMem_) {
        opt.dumpJsRawHeap_ = true;
        dumperSysEventParams_->opt = "mem-jsrawheap";
        status = DumpStatus::DUMP_OK;
    } else if (opt.isDumpHeapMem_) {
        opt.dumpRawHeap_ = true;
        dumperSysEventParams_->opt = "mem-rawheap";
        status = DumpStatus::DUMP_OK;
    } else {
        DUMPER_HILOGE(MODULE_COMMON, "raw param invalid");
        SendErrorMessage("raw param invalid\n");
        CmdHelp();
        status =  DumpStatus::DUMP_HELP;
    }
    return status;
}

DumpStatus DumpImplement::SetMemPruneParam(DumperOpts &opt)
{
    DumpStatus status = DumpStatus::DUMP_FAIL;
    if (opt.isDumpMem_) {
        opt.dumpMemPrune_ = true;
        dumperSysEventParams_->opt = "mem";
        status = DumpStatus::DUMP_OK;
    }
    return status;
}

DumpStatus DumpImplement::SetGCParam(DumperOpts &opt)
{
    DumpStatus status = DumpStatus::DUMP_FAIL;
    if (opt.isDumpJsHeapMem_) {
        opt.isDumpJsHeapMemGC_ = true;
        status = DumpStatus::DUMP_OK;
    } else if (opt.isDumpCjHeapMem_) {
        opt.isDumpCjHeapMemGC_ = true;
        status = DumpStatus::DUMP_OK;
    } else if (opt.isDumpHeapMem_) {
        opt.isDumpHeapMemGc_ = true;
        status = DumpStatus::DUMP_OK;
    }
    return status;
}

DumpStatus DumpImplement::SetLeakobjParam(DumperOpts &opt)
{
    DumpStatus status = DumpStatus::DUMP_FAIL;
    if (opt.isDumpJsHeapMem_) {
        opt.isDumpJsHeapLeakobj_ = true;
        status = DumpStatus::DUMP_OK;
    } else if (opt.isDumpHeapMem_ && opt.isDumpHeapNative_) {
        DUMPER_HILOGI(MODULE_COMMON, "SetLeakobjParam success");
        opt.isDumpHeapLeakobj_ = true;
        status = DumpStatus::DUMP_OK;
    } else {
        DUMPER_HILOGE(MODULE_COMMON, "leakobj param invalid");
        SendErrorMessage("leakobj param invalid\n");
        CmdHelp();
        status = DumpStatus::DUMP_HELP;
    }
    return status;
}

DumpStatus DumpImplement::SetCleanParam(DumperOpts &opt)
{
    DumpStatus status = DumpStatus::DUMP_FAIL;
    if (opt.isDumpJsHeapMem_) {
        opt.isDumpJsHeapClean_ = true;
        status = DumpStatus::DUMP_OK;
    }
    return status;
}

bool DumpImplement::SetIpcStatParam(DumperOpts &opts, const std::string& param)
{
    if (StringUtils::GetInstance().IsSameStr(param, "start-stat")) {
        opts.isDumpIpcStartStat_ = true;
        dumperSysEventParams_->subOpt = "start-stat";
    } else if (StringUtils::GetInstance().IsSameStr(param, "stop-stat")) {
        opts.isDumpIpcStopStat_ = true;
        dumperSysEventParams_->subOpt = "stop-stat";
    } else if (StringUtils::GetInstance().IsSameStr(param, "stat")) {
        opts.isDumpIpcStat_ = true;
        dumperSysEventParams_->subOpt = "stat";
    } else {
        return false;
    }
    return true;
}

DumpStatus DumpImplement::ParseCmdOptionForA(DumperOpts &opts, char *argv[])
{
    if (opts.isDumpSystemAbility_) {
        SplitStr(optarg, " ", opts.abilitieArgs_);
        dumperSysEventParams_->subOpt = "a";
    } else if (opts.isDumpIpc_) {
        opts.isDumpAllIpc_ = true;
        dumperSysEventParams_->target = "allPid";
        if (optarg != nullptr) {
            std::vector<std::string> ipcStatParams;
            SplitStr(optarg, "--", ipcStatParams);
            if (ipcStatParams.empty()) {
                SendErrorMessage(invalidError_);
                return DumpStatus::DUMP_INVALID_ARG;
            }
            if (!SetIpcStatParam(opts, ipcStatParams[0])) {
                SendErrorMessage(invalidError_ + ":" + ipcStatParams[0]);
                return DumpStatus::DUMP_INVALID_ARG;
            }
        }
    } else {
        std::string optionName = RemoveCharacterFromStr(argv[optind - 1], '-');
        std::string errorStr = unrecognizedError_ + optionName;
        SendErrorMessage(errorStr);
        return DumpStatus::DUMP_INVALID_ARG;
    }
    return DumpStatus::DUMP_OK;
}

DumpStatus DumpImplement::ParseShortCmdOption(int c, DumperOpts &opts, int argc, char *argv[])
{
    switch (c) {
        case 'a': {
            DumpStatus status = ParseCmdOptionForA(opts, argv);
            if (status != DumpStatus::DUMP_OK) {
                return status;
            }
            break;
        }
        case 'c':
            opts.isDumpSystem_ = true;
            dumperSysEventParams_->opt += "c";
            break;
        case 'e':
            opts.isFaultLog_ = true;
            dumperSysEventParams_->opt = "e";
            break;
        case 'l':
            opts.isDumpList_ = true;
            dumperSysEventParams_->opt += "l";
            break;
        case 's':
            opts.isDumpSystemAbility_ = true;
            dumperSysEventParams_->opt += "s";
            break;
        case 'p':
            opts.isDumpProcesses_ = true;
            dumperSysEventParams_->opt = "p";
            break;
        case 'v':
            opts.isShowSmapsInfo_ = true;
            dumperSysEventParams_->subOpt = "v";
            break;
        default: {
            DumpStatus status = SetCmdParameter(argc, argv, opts);
            if (status != DumpStatus::DUMP_OK) {
                return status;
            }
            break;
        }
    }
    return DumpStatus::DUMP_OK;
}

DumpStatus DumpImplement::SetShowCount(const std::string &str, int &value)
{
    DumpStatus status = SetCmdIntegerParameter(str, value);
    if (status != DumpStatus::DUMP_OK) {
        return status;
    }
    if (value <= 0) {
        DUMPER_HILOGE(MODULE_COMMON, "Invalid string arg %{public}s", str.c_str());
        std::string errorStr = invalidError_ + str;
        SendErrorMessage(errorStr);
        dumperSysEventParams_->target.clear();
        return DumpStatus::DUMP_INVALID_ARG;
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
    if (!StrToInt(str, value)) {
        DUMPER_HILOGE(MODULE_COMMON, "StrToInt error, str=%{public}s", str.c_str());
        return DumpStatus::DUMP_FAIL;
    }
    dumperSysEventParams_->target = str;
    return DumpStatus::DUMP_OK;
}

void DumpImplement::PrintCommonUsage(std::string& str)
{
    if (DumpUtils::IsUserMode()) {
        std::string substr = "  --mem [pid] -t [timeInterval]  |dump process memory change information,"
        " press Ctrl+C to stop the export. detail information is stored in /data/log/hidumper/record_mem.txt.\n";

        size_t pos = str.find(substr);
        if (pos != std::string::npos) {
            str.erase(pos, substr.length());
        }
    }

    if (ptrReqCtl_ == nullptr) {
        return;
    }
    int rawParamFd = ptrReqCtl_->GetOutputFd();
    if (rawParamFd < 0) {
        return;
    }
    SaveStringToFd(rawParamFd, str.c_str());
}

void DumpImplement::CmdHelp()
{
    const std::string commonUsageStr =
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
        "  -e --list [process_name] [-n num] [--since timestamp] [--until timestamp]  |dump list of abnormal exits\n"
        "  -e --print [process_name] [-n num] [--since timestamp] [--until timestamp]  |dump faultlogs of abnormal"
        " exits\n"
        "  -e --print record_id        |dump faultlogs of the specified record\n"
        "  --net [pid]                 |dump network information; if pid is specified,"
        " dump traffic usage of specified pid\n"
        "  --storage [pid]             |dump storage information; if pid is specified, dump /proc/pid/io\n"
        "  -p                          |processes information, include list and infromation of processes"
        " and threads\n"
        "  -p [pid]                    |dump threads under pid, includes smap, block channel,"
        " execute time, mountinfo\n"
        "  -p pid ARG [-v]             |dump thread or fd descriptor information;"
        " ARG must be one of --thread | --fd; use -v to dump all thread or fd descriptor information\n"
        "  --cpufreq                   |dump real CPU frequency of each core\n"
        "  --mem [pid] [--prune]       |dump memory usage of total; dump memory usage of specified"
        " pid if pid was specified; dump simplified memory information if prune is specified and not support"
        " dumped simplified memory information of specified pid\n"
        "  --mem [pid] [--show-ashmem]   |show ashmem info when dumping memory of specified pid\n"
        "  --mem [pid] [--show-dmabuf]   |show dmabuf info when dumping memory of specified pid\n"
        #ifdef HIDUMPER_HIVIEWDFX_PLUGIN_ENABLE
        "  --mem [pid] [--show-gpumem]   |show gpumem info when dumping memory of specified pid\n"
        #endif
        "  --mem [pid] -t [timeInterval]  |dump process memory change information, press Ctrl+C to stop the export."
        " detail information is stored in /data/log/hidumper/record_mem.txt.\n"
        "  --zip                       |compress output to /data/log/hidumper\n"
        "  --mem-smaps pid [-v]        |display statistic in /proc/pid/smaps, use -v specify more details\n"
        "  --mem-jsheap pid [-T tid] [--gc] [--leakobj] [--raw] [--single] [--clean]  |triggerGC, dumpHeapSnapshot,"
        " dumpRawHeap and dumpLeakList under pid and tid\n"
        "  --mem-cjheap pid [--gc]     |the pid should belong to the Cangjie process; triggerGC and"
        " dumpHeapSnapshot under pid\n"
        #ifdef HIDUMPER_HIVIEWDFX_PLUGIN_ENABLE
        "  --mem-heap pid ARG [--leakobj] [--raw] [-T tid] [--gc] |ARG must be one of --native | --kotlin | --jsvm | "
        "--arkweb-js.\n"
        #else
        "  --mem-heap pid ARG [--leakobj] |ARG must be one of --native | --kotlin.\n"
        #endif
        "  --ipc pid ARG               |ipc load statistic; pid must be specified or set to -a dump all"
        " processes. ARG must be one of --start-stat | --stop-stat | --stat\n";

#ifdef HIDUMPER_HIVIEWDFX_HIVIEW_ENABLE
    const std::string extendedUsageStr =
        "  --cpuusage [pid]            |dump cpu usage by processes and category; if PID is specified,"
        " dump category usage of specified pid\n";
    std::string str = commonUsageStr + extendedUsageStr;
#else
    std::string str = commonUsageStr;
#endif
    PrintCommonUsage(str);
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
            AddGroupTitle(groupName, dumpDatas, dumpParameter);
        }

        DumpStatus ret = DumpStatus::DUMP_FAIL;
        ret = executors[index]->DoPreExecute(dumpParameter, dumpDatas);
        if (ret != DumpStatus::DUMP_OK) {
            SetZipTitle(executors[index], dumpParameter);
            continue;
        }

        ret = executors[index]->DoExecute();
        if ((ret != DumpStatus::DUMP_OK) && (ret != DumpStatus::DUMP_MORE_DATA)) {
            SetZipTitle(executors[index], dumpParameter);
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

void DumpImplement::SetZipTitle(const std::shared_ptr<HidumperExecutor> &executor,
                                const std::shared_ptr<DumperParameter> &dumpParameter)
{
    if (executor->GetDumpConfig()->class_ == DumperConstant::FD_OUTPUT &&
        dumpParameter->GetOpts().IsDumpZip()) {
        dumpParameter->getClientCallback()->SetTitle(",Failed to create the file.");
        DUMPER_HILOGE(MODULE_COMMON, "Zip output failed.");
    }
}

void DumpImplement::AddGroupTitle(const std::string &groupName, HidumperExecutor::StringMatrix dumpDatas,
    const std::shared_ptr<DumperParameter>& dumpParameter)
{
    /**
     * @brief The group title is followed
     * '
     * -------------------------------[groupName]-------------------------------
     * '
     */
    if (StringUtils::GetInstance().IsSameStr(groupName, "ipc")) {
        DUMPER_HILOGI(MODULE_COMMON, "ipc statistic cmd, do not need title.");
        return;
    }
    if (StringUtils::GetInstance().IsSameStr(groupName, "memory") && dumpParameter->GetOpts().memPid_ <= 0) {
        DUMPER_HILOGI(MODULE_COMMON, "hidumper --mem cmd, do not need title.");
        return;
    }
    if (StringUtils::GetInstance().IsSameStr(groupName, "memory") && dumpParameter->GetOpts().timeInterval_ > 0) {
        DUMPER_HILOGI(MODULE_COMMON, "hidumper --mem pid -t timeinterval cmd, do not need title.");
        return;
    }
    if (StringUtils::GetInstance().IsSameStr(groupName, "ability")) {
        return;
    }
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
        std::string optLongStr = RemoveCharacterFromStr(argv[optind - 1], '-');
        if (optLongStr == "since" || optLongStr == "until") {
            SendErrorMessage(requireError_ + optLongStr);
            return;
        }
        SendErrorMessage(unrecognizedError_ + optLongStr);
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

void DumpImplement::SendErrorMessageIf(DumperOpts &opts, const std::string &optionValue)
{
    std::string errorStr = unrecognizedError_ + RemoveCharacterFromStr(optionValue, '-');
    if (opts.isFaultLog_ && (opts.isEventList_ || opts.isEventDetail_)) {
        errorStr = invalidError_ + RemoveCharacterFromStr(optionValue, '-');
    }
    SendErrorMessage(errorStr);
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

void DumpImplement::SendReleaseAppErrorMessage(const std::string& opt)
{
    if (ptrReqCtl_ == nullptr) {
        DUMPER_HILOGE(MODULE_COMMON, "ptrReqCtl_ == nullptr");
        return;
    }
    int rawParamFd = ptrReqCtl_->GetOutputFd();
    if (rawParamFd < 0) {
        DUMPER_HILOGE(MODULE_COMMON, "rawParamFd < 0");
        return;
    }
    std::string onlySupportDebugSignedAppMessage_;
    onlySupportDebugSignedAppMessage_ += "[ERROR]: The %s option is only supported for debug-signed application [ ";
    onlySupportDebugSignedAppMessage_ += "\"appProvisionType\": \"debug\"].\n";
    dprintf(rawParamFd, onlySupportDebugSignedAppMessage_.c_str(), opt.c_str());
}

void DumpImplement::SendReleaseVersionErrorMessage(const std::string& opt)
{
    if (ptrReqCtl_ == nullptr) {
        DUMPER_HILOGE(MODULE_COMMON, "ptrReqCtl_ == nullptr");
        return;
    }
    int rawParamFd = ptrReqCtl_->GetOutputFd();
    if (rawParamFd < 0) {
        DUMPER_HILOGE(MODULE_COMMON, "rawParamFd < 0");
        return;
    }
    std::string onlySupportDebugVersionAppMessage;
    onlySupportDebugVersionAppMessage += "[ERROR]: The %s option is not supported in user privilege.\n";
    dprintf(rawParamFd, onlySupportDebugVersionAppMessage.c_str(), opt.c_str());
}

void DumpImplement::SendPermissionCheckErrorMessage(const std::string& opt, bool isProfileable,
    bool isInhouse, bool isDeveloperMode)
{
    if (ptrReqCtl_ == nullptr) {
        DUMPER_HILOGE(MODULE_COMMON, "ptrReqCtl_ == nullptr");
        return;
    }
    int rawParamFd = ptrReqCtl_->GetOutputFd();
    if (rawParamFd < 0) {
        DUMPER_HILOGE(MODULE_COMMON, "rawParamFd < 0");
        return;
    }
    std::string supportReleaseSignedAppMessage_ = "[Permission Check Failed]\n";
    supportReleaseSignedAppMessage_ += "  The %s option needs a debug-signed application.\n";
    supportReleaseSignedAppMessage_ +=
        "  If release-signed application needs to meet all of the following conditions:\n";
    supportReleaseSignedAppMessage_ += "  - profilerable-label: ";
    supportReleaseSignedAppMessage_ += (isProfileable ? "satisfied" : "not satisfied");
    supportReleaseSignedAppMessage_ += "\n";
    supportReleaseSignedAppMessage_ += "  - enterprise-app-type: ";
    supportReleaseSignedAppMessage_ += (isInhouse ? "satisfied" : "not satisfied");
    supportReleaseSignedAppMessage_ += "\n";
    supportReleaseSignedAppMessage_ += "  - developer mode: ";
    supportReleaseSignedAppMessage_ += (isDeveloperMode ? "satisfied" : "not satisfied");
    supportReleaseSignedAppMessage_ += "\n";
    dprintf(rawParamFd, supportReleaseSignedAppMessage_.c_str(), opt.c_str());
}

std::string DumpImplement::RemoveCharacterFromStr(const std::string &str, const char character)
{
    std::string strTmp = str;
    while (strTmp.find(character) != std::string::npos) {
        strTmp.erase(strTmp.find(character), 1);
    }
    return strTmp;
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

DumpStatus DumpImplement::CheckProcessAlive(const DumperOpts &opts)
{
    if ((opts.cpuUsagePid_ > -1) && !DumpUtils::CheckProcessAlive(opts.cpuUsagePid_)) {
        SendPidErrorMessage(opts.cpuUsagePid_);
        return DumpStatus::DUMP_FAIL;
    }
    if ((opts.memPid_ > -1) && !DumpUtils::CheckProcessAlive(opts.memPid_)) {
        SendPidErrorMessage(opts.memPid_);
        return DumpStatus::DUMP_FAIL;
    }
    if ((opts.processPid_ > -1) && !DumpUtils::CheckProcessAlive(opts.processPid_)) {
        SendPidErrorMessage(opts.processPid_);
        return DumpStatus::DUMP_FAIL;
    }
    if ((opts.storagePid_ > -1) && !DumpUtils::CheckProcessAlive(opts.storagePid_)) {
        SendPidErrorMessage(opts.storagePid_);
        return DumpStatus::DUMP_FAIL;
    }
    if ((opts.netPid_ > -1) && !DumpUtils::CheckProcessAlive(opts.netPid_)) {
        SendPidErrorMessage(opts.netPid_);
        return DumpStatus::DUMP_FAIL;
    }
    if ((opts.dumpJsHeapMemPid_ > 0) && !DumpUtils::CheckProcessAlive(opts.dumpJsHeapMemPid_)) {
        SendPidErrorMessage(opts.dumpJsHeapMemPid_);
        return DumpStatus::DUMP_FAIL;
    }
    if ((opts.dumpCjHeapMemPid_ > 0) && !DumpUtils::CheckProcessAlive(opts.dumpCjHeapMemPid_)) {
        SendPidErrorMessage(opts.dumpCjHeapMemPid_);
        return DumpStatus::DUMP_FAIL;
    }
    if ((opts.dumpHeapMemPid_ > 0) && !DumpUtils::CheckProcessAlive(opts.dumpHeapMemPid_)) {
        SendPidErrorMessage(opts.dumpHeapMemPid_);
        return DumpStatus::DUMP_FAIL;
    }
    if ((opts.dumpHeapArgPid_ > 0) && !DumpUtils::CheckProcessAlive(opts.dumpHeapArgPid_)) {
        SendPidErrorMessage(opts.dumpHeapArgPid_);
        return DumpStatus::DUMP_FAIL;
    }
    if ((opts.ipcStatPid_ > 0) && !DumpUtils::CheckProcessAlive(opts.ipcStatPid_)) {
        SendPidErrorMessage(opts.ipcStatPid_);
        return DumpStatus::DUMP_FAIL;
    }
    return DumpStatus::DUMP_OK;
}

void DumpImplement::RemoveDuplicateString(DumperOpts &opts)
{
    DumpUtils::RemoveDuplicateString(opts.logArgs_);       // remove duplicate log names
    DumpUtils::RemoveDuplicateString(opts.systemArgs_);    // remove duplicate system names
    DumpUtils::RemoveDuplicateString(opts.abilitieNames_); // remove duplicate ability names
}

#ifdef HIDUMPER_HIVIEWDFX_HISYSEVENT_ENABLE
void DumpImplement::ReportJsheap(const DumperOpts &opts)
{
    if (!opts.isDumpJsHeapMem_ || opts.isDumpJsHeapMemGC_) {
        return;
    }
    std::string strType = "hidumper";
    if (opts.dumpJsRawHeap_) {
        strType = "hidumperRawHeap";
    }
    int memJsheapRet = HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::FRAMEWORK, "ARK_STATS_DUMP",
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT,
        "PID", std::to_string(opts.dumpJsHeapMemPid_),
        "TYPE", strType);
    if (memJsheapRet != 0) {
        DUMPER_HILOGE(MODULE_COMMON, "hisysevent report mem jsheap failed! ret %{public}d.", memJsheapRet);
    }
}

void DumpImplement::ReportCjheap(const DumperOpts &opts)
{
    if (!opts.isDumpCjHeapMem_ || opts.isDumpCjHeapMemGC_) {
        return;
    }
    std::string strType = "hidumper";
    int memCjheapRet = HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::FRAMEWORK, "ARK_STATS_DUMP",
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT,
        "PID", std::to_string(opts.dumpCjHeapMemPid_),
        "TYPE", strType);
    if (memCjheapRet != 0) {
        DUMPER_HILOGE(MODULE_COMMON, "hisysevent report mem cjheap failed! ret %{public}d.", memCjheapRet);
    }
}

void DumpImplement::ReportMemheap(const DumperOpts &opts)
{
    if (!opts.isDumpHeapMem_) {
        return;
    }
    std::string strType;
    if (opts.isDumpHeapKotlin_) {
        strType = "hidumperKmpHeap";
    } else if (opts.isDumpHeapNative_) {
        strType = "hidumperNativeHeap";
    } else if (opts.isDumpHeapJsvm_) {
        strType = "hidumperJsvmHeap";
        if (opts.dumpRawHeap_) {
            strType = "hidumperJsvmRawHeap";
        }
    } else if (opts.isDumpHeapArkwebJs_) {
        if (opts.isDumpHeapMemGc_) {
            return; // gc only, no dump
        }
        strType = "hidumperArkwebJsHeap";
    } else {
        return;
    }
    int pid = opts.dumpHeapArgPid_ > 0 ? opts.dumpHeapArgPid_ : opts.dumpHeapMemPid_;
    int memHeapRet = HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::FRAMEWORK, "ARK_STATS_DUMP",
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT,
        "PID", std::to_string(pid),
        "TYPE", strType);
    if (memHeapRet != 0) {
        DUMPER_HILOGE(MODULE_COMMON, "hisysevent report mem heap failed! ret %{public}d.", memHeapRet);
    }
}
#endif

bool DumpImplement::CheckJsHeapPermission(int pid)
{
    bool isDebugVersion = DumpUtils::CheckAppDebugVersion(pid);
    if (isDebugVersion) {
        DUMPER_HILOGD(MODULE_COMMON, "JsHeapPermission success, is Debug Version");
        return true;
    }

    bool isProfileable = DumpUtils::CheckAppProfileable(pid);
    bool isInhouse = DumpUtils::CheckAppInhouse(pid);
    bool isDeveloperMode = DumpUtils::IsDeveloperModeEnabled();
    if (!isProfileable || !isInhouse || !isDeveloperMode) {
        SendPermissionCheckErrorMessage("--mem-jsheap", isProfileable, isInhouse, isDeveloperMode);
        DUMPER_HILOGE(MODULE_COMMON,
            "JsHeapPermission failed, isProfileable=%{public}d isInhouse=%{public}d isDeveloperMode=%{public}d",
            isProfileable, isInhouse, isDeveloperMode);
        return false;
    }
    
    DUMPER_HILOGD(MODULE_COMMON, "JsHeapPermission success, release version with profileable+inhouse+developerMode");
    return true;
}

bool DumpImplement::CheckDumpPermission(DumperOpts& opt)
{
    bool isUserMode = DumpUtils::IsUserMode();
    DUMPER_HILOGD(MODULE_COMMON, "debug|isUserMode %{public}d", isUserMode);
    if ((opt.isDumpFd_ || opt.isDumpThread_) && opt.processPid_ <= 0) {
        SendErrorMessage(invalidError_ + "--fd/--thread requires -p PID");
        DUMPER_HILOGE(MODULE_COMMON, "Fd/Thread dump requires pid, isUserMode:%{public}d, processPid_:%{public}d",
            isUserMode, opt.processPid_);
        return false;
    }
    if (!isUserMode) {
        return true;
    }
    int uid = ptrReqCtl_->GetUid();
    if (opt.isShowSmaps_ && uid != HIVIEW_UID && !DumpUtils::CheckAppDebugVersion(opt.memPid_)) {
        SendReleaseAppErrorMessage("--mem-smaps");
        DUMPER_HILOGE(MODULE_COMMON, "ShowSmaps false, isUserMode:%{public}d, pid:%{public}d", isUserMode, opt.memPid_);
        return false;
    }
    if (opt.isDumpJsHeapMem_ && !CheckJsHeapPermission(opt.dumpJsHeapMemPid_)) {
        return false;
    }
    if (opt.isDumpCjHeapMem_ && !DumpUtils::CheckAppDebugVersion(opt.dumpCjHeapMemPid_)) {
        SendReleaseAppErrorMessage("--mem-cjheap");
        DUMPER_HILOGE(MODULE_COMMON, "DumpCjHeapMem false isUserMode %{public}d", isUserMode);
        return false;
    }
    // mem-heap + releaseApp
    int heapPid = opt.dumpHeapArgPid_ > 0 ? opt.dumpHeapArgPid_ : opt.dumpHeapMemPid_;
    if (opt.isDumpHeapMem_ && !DumpUtils::CheckAppDebugVersion(heapPid)) {
        SendReleaseAppErrorMessage("--mem-heap");
        DUMPER_HILOGE(MODULE_COMMON, "DumpHeapMem false isUserMode %{public}d", isUserMode);
        return false;
    }
    if (opt.isDumpMem_ && opt.timeInterval_ > 0) {
        SendReleaseVersionErrorMessage("-t");
        DUMPER_HILOGE(MODULE_COMMON, "Show mem false, isUserMode:%{public}d, pid:%{public}d, timeInterval_:%{public}d",
            isUserMode, opt.memPid_, opt.timeInterval_);
        return false;
    }
    return true;
}

bool DumpImplement::CheckUnableToDumpAll(int argc, DumperOpts& opt)
{
    bool isZip = opt.IsDumpZip();
    bool noSelect = !opt.IsSelectAny();
    bool validArgc = (argc == ARG_COUNT_NO_PARAM) || (argc == ARG_COUNT_WITH_ZIP && isZip);
    return noSelect && !validArgc;
}

bool DumpImplement::CheckDumpHeapMemParameter(int argc, DumperOpts& opt)
{
    if (argc < ARG_COUNT_HEAP_MEM) {
        SendErrorMessage("mem-heap param error: insufficient arguments, need at least --mem-heap pid ARG");
        return false;
    }
    int argCount = opt.isDumpHeapNative_ + opt.isDumpHeapKotlin_ + opt.isDumpHeapJsvm_ + opt.isDumpHeapArkwebJs_;
    if (argCount != 1) {
        SendErrorMessage("mem-heap param error: ARG must be one of --native | --kotlin | --jsvm | --arkweb-js");
        return false;
    }
    bool validPid = (opt.dumpHeapMemPid_ > 0 && opt.dumpHeapArgPid_ == 0) ||
        (opt.dumpHeapMemPid_ == 0 && opt.dumpHeapArgPid_ > 0);
    if (!validPid) {
        SendErrorMessage("mem-heap param error: pid must be specified and specified only once");
        return false;
    }
    DUMPER_HILOGI(MODULE_COMMON, "mem-heap basic check passed, entering ARG-specific check");
    if (opt.isDumpHeapArkwebJs_) {
        return CheckArkwebJsParameter(opt);
    }
    return true;
}

bool DumpImplement::CheckArkwebJsParameter(DumperOpts& opt)
{
    if (opt.isDumpHeapMemGc_ && opt.dumpRawHeap_) {
        DUMPER_HILOGE(MODULE_COMMON, "raw and gc cannot be used together for arkweb-js");
        SendErrorMessage("mem-heap param error: --raw and --gc cannot be used together");
        return false;
    }
    int renderPid = opt.dumpHeapArgPid_ > 0 ? opt.dumpHeapArgPid_ : opt.dumpHeapMemPid_;
    opt.dumpHeapRenderPid_ = renderPid;
    std::string renderProcName;
    if (!DumpCommonUtils::GetProcessNameByPid(renderPid, renderProcName)) {
        DUMPER_HILOGE(MODULE_COMMON, "GetProcessNameByPid failed for renderPid %{public}d", renderPid);
        SendErrorMessage("mem-heap param error: cannot get process name for pid " + std::to_string(renderPid));
        return false;
    }
    std::string mainProcName = renderProcName;
    const std::string suffix = ":render";
    bool isRenderValid = mainProcName.length() > suffix.length() &&
        mainProcName.substr(mainProcName.length() - suffix.length()) == suffix;
    if (!isRenderValid) {
        DUMPER_HILOGE(MODULE_COMMON, "Failed to get mian process name for renderPid %{public}d", renderPid);
        SendErrorMessage("mem-heap param error: pid " + std::to_string(renderPid) + " is not a valid render process");
        return false;
    }
    mainProcName = mainProcName.substr(0, mainProcName.length() - suffix.length());
    DUMPER_HILOGI(MODULE_COMMON, "renderPid=%{public}d, renderProcName=%{public}s, mainProcName=%{public}s",
        renderPid, renderProcName.c_str(), mainProcName.c_str());
    std::vector<DumpCommonUtils::PidInfo> pidInfos;
    DumpCommonUtils::GetPidInfos(pidInfos, true);
    int mainPid = -1;
    for (const auto& info : pidInfos) {
        if (info.cmdline_ == mainProcName) {
            mainPid = info.pid_;
            break;
        }
    }
    if (mainPid <= 0) {
        DUMPER_HILOGE(MODULE_COMMON, "Failed to find main process for %{public}s", mainProcName.c_str());
        SendErrorMessage("mem-heap param error: cannot find main process for " + mainProcName);
        return false;
    }
    opt.dumpHeapArgPid_ = opt.dumpHeapArgPid_ > 0 ? mainPid : 0;
    opt.dumpHeapMemPid_ = opt.dumpHeapMemPid_ > 0 ? mainPid : 0;
    DUMPER_HILOGI(MODULE_COMMON, "Found mainPid=%{public}d for renderPid=%{public}d", mainPid, renderPid);
    return true;
}

DumpStatus DumpImplement::SetHeapCombineParam(DumperOpts &opt)
{
    opt.isDumpJsHeapCombine_ = true;
    return DumpStatus::DUMP_OK;
}

bool DumpImplement::CheckJsHeapSingleParam(const DumperOpts &opt)
{
    if (!opt.isDumpJsHeapCombine_) {
        return true;
    }
    if (!opt.dumpJsRawHeap_) {
        SendErrorMessage("mem-jsheap paramerr, '--single' only used with '--raw'.");
        return false;
    }
    if (opt.threadId_ != 0) {
        SendErrorMessage("mem-jsheap paramerr, '--single' not need 'tid'.");
        return false;
    }
    return true;
}
} // namespace HiviewDFX
} // namespace OHOS
