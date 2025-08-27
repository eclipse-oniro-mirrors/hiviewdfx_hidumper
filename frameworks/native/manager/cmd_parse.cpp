/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "cmd_parse.h"

#include "common.h"
#include "dump_context.h"
#include "dump_utils.h"
#include "hilog_wrapper.h"
#include "file_ex.h"
#include "string_ex.h"
#include "securec.h"
#include "unistd.h"
#include "writer_utils.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr int ARG_MAX_COUNT = 100;
constexpr int SINGLE_ARG_MAXLEN = 1024;
constexpr int ARG_INDEX_OFFSET_LAST_OPTION = 2;
constexpr int IPC_STAT_ARG_NUMS = 4;
const std::string UNRECOGNIZED_ERROR_MSG = "hidumper: option pid missed. ";
const std::string ARGUMENT_ERROR_MSG = "hidumper: option requires an argument: ";
const std::string INVALID_ARG_MSG = "hidumper: invalid arg: ";
const std::string PID_ERROR_MSG = "hidumper: No such process: ";

static struct option LONG_OPTIONS[] = {
    {"cpufreq", no_argument, 0, 0},
#ifdef HIDUMPER_HIVIEWDFX_HIVIEW_ENABLE
    {"cpuusage", optional_argument, 0, 0},
#endif
    {"mem", optional_argument, 0, 0},
    {"net", no_argument, 0, 0},
    {"storage", no_argument, 0, 0},
    {"zip", no_argument, 0, 0},
    {"mem-smaps", required_argument, 0, 0},
    {"mem-jsheap", required_argument, 0, 0},
    {"gc", no_argument, 0, 0},
    {"leakobj", no_argument, 0, 0},
    {"raw", no_argument, 0, 0},
    {"ipc", optional_argument, 0, 0},
    {"start-stat", no_argument, 0, 0},
    {"stop-stat", no_argument, 0, 0},
    {"stat", no_argument, 0, 0},
    {0, 0, 0, 0}
};

void PrintHelpInfo(DumpContext& dumpContext)
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
        "  --net [pid]                 |dump network information; if pid is specified,"
        " dump traffic usage of specified pid\n"
        "  --storage [pid]             |dump storage information; if pid is specified, dump /proc/pid/io\n"
        "  -p                          |processes information, include list and infromation of processes"
        " and threads\n"
        "  -p [pid]                    |dump threads under pid, includes smap, block channel,"
        " execute time, mountinfo\n"
        "  --cpufreq                   |dump real CPU frequency of each core\n"
        "  --mem [pid] [--prune]       |dump memory usage of total; dump memory usage of specified"
        " pid if pid was specified; dump simplified memory infomation if prune is specified and not support"
        " dumped simplified memory infomation of specified pid\n"
        "  --mem [pid] [--show-ashmem]   |show ashmem info when dumping memory of specified pid\n"
        "  --mem [pid] -t [timeInterval]  |dump process memory change information, press Ctrl+C to stop the export."
        " detail information is stored in /data/log/hidumper/record_mem.txt.\n"
        "  --zip                       |compress output to /data/log/hidumper\n"
        "  --mem-smaps pid [-v]        |display statistic in /proc/pid/smaps, use -v specify more details\n"
        "  --mem-jsheap pid [-T tid] [--gc] [--leakobj] [--raw]  |triggerGC, dumpHeapSnapshot, dumpRawHeap"
        " and dumpLeakList under pid and tid\n"
        "  --mem-cjheap pid [--gc]     |the pid should belong to the Cangjie process; triggerGC and"
        " dumpHeapSnapshot under pid\n"
        "  --ipc pid ARG               |ipc load statistic; pid must be specified or set to -a dump all"
        " processes. ARG must be one of --start-stat | --stop-stat | --stat\n";

#ifdef HIDUMPER_HIVIEWDFX_HIVIEW_ENABLE
    const std::string extendedUsageStr =
        "  --cpuusage [pid]            |dump cpu usage by processes and category; if PID is specified,"
        " dump category usage of specified pid\n";

    std::string helpInfo = commonUsageStr + extendedUsageStr;
#else
    std::string helpInfo = commonUsageStr;
#endif
    WriteStringIntoFd(helpInfo, dumpContext.GetOutputFd());
}

void PrintErrorMessage(const DumpContext& dumpContext, const std::string& errorMsg)
{
    WriteStringIntoFd(errorMsg, dumpContext.GetOutputFd());
}

bool IsArgValid(char* arg)
{
    if (arg == nullptr) {
        DUMPER_HILOGE(MODULE_COMMON, "argument is null.");
        return false;
    }
    size_t len = strlen(arg);
    if (len == 0) {
        DUMPER_HILOGE(MODULE_COMMON, "argument is empty.");
        return false;
    }
    if (len > SINGLE_ARG_MAXLEN) {
        DUMPER_HILOGE(MODULE_COMMON, "too long args:%{public}zu, limit size:%{public}d.", len, SINGLE_ARG_MAXLEN);
        return false;
    }
    return true;
}

bool IsHiviewEnabled()
{
#ifdef HIDUMPER_HIVIEWDFX_HIVIEW_ENABLE
    return true;
#else
    return false;
#endif
}
}

CmdParse::CmdParse() {}
CmdParse::~CmdParse() {}

DumpStatus CmdParse::Parse(int argc, char *argv[], DumpContext& dumpContext)
{
    if (CheckArgs(argc, argv) != DumpStatus::DUMP_OK) {
        DUMPER_HILOGE(MODULE_COMMON, "Invalid input parameters");
        return DumpStatus::DUMP_FAIL;
    }
    if (CmdParseHelper(argc, argv, dumpContext) != DumpStatus::DUMP_OK) {
        DUMPER_HILOGE(MODULE_COMMON, "CmdParseHelper failed");
        return DumpStatus::DUMP_FAIL;
    }
    if (VerifyOptions(dumpContext) != DumpStatus::DUMP_OK) {
        DUMPER_HILOGE(MODULE_COMMON, "VerifyOptions failed");
        return DumpStatus::DUMP_FAIL;
    }
    return DumpStatus::DUMP_OK;
}

DumpStatus CmdParse::CheckArgs(int argc, char* argv[])
{
    if (argc < 1 || argv == nullptr) {
        DUMPER_HILOGE(MODULE_COMMON, "Invalid input parameters");
        return DumpStatus::DUMP_FAIL;
    }
    if (argc > ARG_MAX_COUNT) {
        DUMPER_HILOGE(MODULE_COMMON, "too many arguments(%{public}d), limit size %{public}d.", argc, ARG_MAX_COUNT);
        return DumpStatus::DUMP_FAIL;
    }
    std::string dumpCmdStr;
    for (int i = 0; i < argc; i++) {
        if (!IsArgValid(argv[i])) {
            return DumpStatus::DUMP_FAIL;
        }
        dumpCmdStr += std::string(argv[i]) + " ";
    }
    return DumpStatus::DUMP_OK;
}

DumpStatus CmdParse::CmdParseHelper(int argc, char *argv[], DumpContext& dumpContext)
{
    optind = 0; // reset getopt_long
    opterr = 0; // getopt not show error info
    const char optStr[] = "-hlcsa:epvT:";

    while (true) {
        int optionIndex = 0;
        int c = getopt_long(argc, argv, optStr, LONG_OPTIONS, &optionIndex);
        if (c == -1) {
            break;
        }

        DumpStatus status;
        switch (c) {
            case 0:
                status = ParseLongCmdOption(argc, dumpContext, LONG_OPTIONS, optionIndex, argv);
                break;
            case 'h':
                PrintHelpInfo(dumpContext);
                return DumpStatus::DUMP_HELP;
            case '?':
                CheckIncorrectCmdOption(optStr, argv, dumpContext);
                return DumpStatus::DUMP_INVALID_ARG;
            default:
                status = ParseShortCmdOption(c, dumpContext, argc, argv);
                break;
        }
        if (status != DumpStatus::DUMP_OK) {
            return status;
        }
    }
    if (CheckProcessAlive(dumpContext) != DumpStatus::DUMP_OK) {
        return DumpStatus::DUMP_FAIL;
    }
    return DumpStatus::DUMP_OK;
}

DumpStatus CmdParse::ParseLongCmdOption(int argc, DumpContext& dumpContext,
                                        const struct option longOptions[], const int &optionIndex, char *argv[])
{
    if (ParseSubLongCmdOption(argc, dumpContext, longOptions, optionIndex, argv)) {
        return DumpStatus::DUMP_OK;
    }
    
    const std::string optionName = longOptions[optionIndex].name;
    if (optionName == "mem-smaps") {
        if (ARG_INDEX_OFFSET_LAST_OPTION < 0 || argc >= ARG_INDEX_OFFSET_LAST_OPTION) {
            return DumpStatus::DUMP_FAIL;
        }
        dumpContext.GetDumperOpts()->isShowSmaps = true;
        return SetCmdIntegerParameter(argv[ARG_INDEX_OFFSET_LAST_OPTION],
                                      dumpContext.GetDumperOpts()->memPid, dumpContext);
    } else if (optionName == "mem-jsheap") {
        return SetMemJsheapParam(dumpContext);
    } else if (optionName == "raw") {
        return SetRawParam(dumpContext);
    } else if (optionName == "gc") {
        dumpContext.GetDumperOpts()->isDumpJsHeapMemGC = true;
    } else if (optionName == "leakobj") {
        dumpContext.GetDumperOpts()->isDumpJsHeapLeakobj = true;
    } else if (optionName == "ipc") {
        dumpContext.GetDumperOpts()->isDumpIpc = true;
        if (argc != IPC_STAT_ARG_NUMS) {
            DUMPER_HILOGE(MODULE_COMMON, "ipc stat cmd args invalid");
            PrintErrorMessage(dumpContext, "ipc stat cmd args invalid\n");
            PrintHelpInfo(dumpContext);
            return DumpStatus::DUMP_HELP;
        }
    } else if (SetIpcStatParam(dumpContext, optionName)) {
        if (!dumpContext.GetDumperOpts()->isDumpIpc) {
            DUMPER_HILOGE(MODULE_COMMON, "ipc stat param invalid");
            PrintErrorMessage(dumpContext, "ipc stat cmd args invalid\n");
            PrintHelpInfo(dumpContext);
            return DumpStatus::DUMP_HELP;
        }
    } else {
        DUMPER_HILOGE(MODULE_COMMON, "ParseLongCmdOption %{public}s", optionName.c_str());
    }
    
    return DumpStatus::DUMP_OK;
}

bool CmdParse::ParseSubLongCmdOption(int argc, DumpContext& dumpContext,
                                     const struct option longOptions[], const int &optionIndex, char *argv[])
{
    const bool hiviewEnable = IsHiviewEnabled();

    const std::string optionName = longOptions[optionIndex].name;
    if (optionName == "cpufreq") {
        dumpContext.GetDumperOpts()->isDumpCpuFreq = true;
    } else if (hiviewEnable && optionName == "cpuusage") {
        dumpContext.GetDumperOpts()->isDumpCpuUsage = true;
    } else if (optionName == "mem") {
        dumpContext.GetDumperOpts()->isDumpMem = true;
    } else if (optionName == "net") {
        dumpContext.GetDumperOpts()->isDumpNet = true;
    } else if (optionName == "storage") {
        dumpContext.GetDumperOpts()->isDumpStorage = true;
    } else if (optionName == "zip") {
    } else {
        return false;
    }
    
    return true;
}

void CmdParse::CheckIncorrectCmdOption(const char *optStr, char *argv[], DumpContext& dumpContext)
{
    if (optopt == 0) {
        std::string optionName = RemoveCharacterFromStr(argv[optind - 1], '-');
        PrintErrorMessage(dumpContext, UNRECOGNIZED_ERROR_MSG + optionName);
    } else if (!IsShortOptionReqArg(optStr, dumpContext)) {
        std::string errorStr = UNRECOGNIZED_ERROR_MSG;
        errorStr += optopt;
        PrintErrorMessage(dumpContext, errorStr);
    }
}

DumpStatus CmdParse::ParseShortCmdOption(int c, DumpContext& dumpContext, int argc, char *argv[])
{
    switch (c) {
        case 'a':
            return ParseCmdOptionForA(dumpContext, argv);
        case 'c':
            dumpContext.GetDumperOpts()->isDumpSystem = true;
            break;
        case 'e':
            dumpContext.GetDumperOpts()->isFaultLog = true;
            break;
        case 'l':
            dumpContext.GetDumperOpts()->isDumpList = true;
            break;
        case 's':
            dumpContext.GetDumperOpts()->isDumpSystemAbility = true;
            break;
        case 'p':
            dumpContext.GetDumperOpts()->isDumpProcesses = true;
            break;
        case 'v':
            dumpContext.GetDumperOpts()->isShowSmapsInfo = true;
            break;
        default:
            return SetCmdParameter(argc, argv, dumpContext);
    }
    return DumpStatus::DUMP_OK;
}

bool CmdParse::IsShortOptionReqArg(const char *optStr, DumpContext& dumpContext)
{
    for (int i = 0; optStr[i] != '\0'; ++i) {
        if (optStr[i] == optopt) {
            std::string errorMsg = ARGUMENT_ERROR_MSG + std::string(1, optStr[i]);
            PrintErrorMessage(dumpContext, errorMsg);
            return true;
        }
    }
    return false;
}

DumpStatus CmdParse::SetCmdParameter(int argc, char *argv[], DumpContext& dumpContext)
{
    DUMPER_HILOGD(MODULE_COMMON,
                  "debug|SetCmdParameter optind is %{public}d"
                  " argc is  %{public}d",
                  optind,
                  argc);
    if (optind > 1 && optind <= argc) {
    const std::string prevArg = argv[optind - ARG_INDEX_OFFSET_LAST_OPTION];
    const std::string currentArg = argv[optind - 1];
    
        if (IsHiviewEnabled() && prevArg == "--cpuusage") {
            return SetCmdIntegerParameter(currentArg, dumpContext.GetDumperOpts()->cpuUsagePid, dumpContext);
        } else if (prevArg == "--mem") {
            return SetCmdIntegerParameter(currentArg, dumpContext.GetDumperOpts()->memPid, dumpContext);
        } else if (prevArg == "--net") {
            return SetCmdIntegerParameter(currentArg, dumpContext.GetDumperOpts()->netPid, dumpContext);
        } else if (prevArg == "--storage") {
            return SetCmdIntegerParameter(currentArg, dumpContext.GetDumperOpts()->storagePid, dumpContext);
        } else if (prevArg == "-c") {
            dumpContext.GetDumperOpts()->systemArgs.insert(currentArg);
        } else if (prevArg == "-p") {
            return SetCmdIntegerParameter(currentArg, dumpContext.GetDumperOpts()->processPid, dumpContext);
        } else if (prevArg == "-T") {
            return SetCmdIntegerParameter(currentArg, dumpContext.GetDumperOpts()->threadId, dumpContext);
        } else if (IsSADumperOption(argv)) {
            dumpContext.GetDumperOpts()->abilityNames.emplace_back(currentArg);
        } else if (prevArg == "--ipc") {
            return SetCmdIntegerParameter(currentArg, dumpContext.GetDumperOpts()->ipcStatPid, dumpContext);
        } else {
            std::string optionName = RemoveCharacterFromStr(currentArg, '-');
            std::string errorStr = UNRECOGNIZED_ERROR_MSG + optionName;
            PrintErrorMessage(dumpContext, errorStr);
            return DumpStatus::DUMP_FAIL;
        }
    }
    return DumpStatus::DUMP_OK;
}

DumpStatus CmdParse::SetCmdIntegerParameter(const std::string &str, int &value, DumpContext& dumpContext)
{
    if (!IsNumericStr(str)) {
        DUMPER_HILOGE(MODULE_COMMON, "Invalid string arg %{public}s", str.c_str());
        PrintErrorMessage(dumpContext, INVALID_ARG_MSG + str);
        return DumpStatus::DUMP_INVALID_ARG;
    }
    if (!StrToInt(str, value)) {
        DUMPER_HILOGE(MODULE_COMMON, "StrToInt error, str=%{public}s", str.c_str());
        return DumpStatus::DUMP_FAIL;
    }
    return DumpStatus::DUMP_OK;
}

DumpStatus CmdParse::SetMemJsheapParam(DumpContext& dumpContext)
{
    if (optarg == nullptr) {
        DUMPER_HILOGE(MODULE_COMMON, "mem-jsheap nullptr");
        return DumpStatus::DUMP_FAIL;
    }
    dumpContext.GetDumperOpts()->isDumpJsHeapMem = true;
    return SetCmdIntegerParameter(optarg, dumpContext.GetDumperOpts()->dumpJsHeapMemPid, dumpContext);
}

DumpStatus CmdParse::SetRawParam(DumpContext& dumpContext)
{
    if (dumpContext.GetDumperOpts()->isDumpJsHeapMem) {
        dumpContext.GetDumperOpts()->dumpJsRawHeap = true;
        return DumpStatus::DUMP_OK;
    }
    return DumpStatus::DUMP_FAIL;
}

bool CmdParse::SetIpcStatParam(DumpContext& dumpContext, const std::string& param)
{
    if (param == "start-stat") {
        dumpContext.GetDumperOpts()->isDumpIpcStartStat = true;
    } else if (param == "stop-stat") {
        dumpContext.GetDumperOpts()->isDumpIpcStopStat = true;
    } else if (param == "stat") {
        dumpContext.GetDumperOpts()->isDumpIpcStat = true;
    } else {
        return false;
    }
    return true;
}

std::string CmdParse::RemoveCharacterFromStr(const std::string &str, const char character)
{
    std::string result = str;
    size_t pos;
    while ((pos = result.find(character)) != std::string::npos) {
        result.erase(pos, 1);
    }
    return result;
}

DumpStatus CmdParse::ParseCmdOptionForA(DumpContext& dumpContext, char *argv[])
{
    if (dumpContext.GetDumperOpts()->isDumpSystemAbility) {
        SplitStr(optarg, " ", dumpContext.GetDumperOpts()->abilityArgs);
    } else if (dumpContext.GetDumperOpts()->isDumpIpc) {
        dumpContext.GetDumperOpts()->isDumpAllIpc = true;
        if (optarg != nullptr) {
            std::vector<std::string> ipcStatParams;
            SplitStr(optarg, "--", ipcStatParams);
            if (ipcStatParams.empty()) {
                PrintErrorMessage(dumpContext, INVALID_ARG_MSG);
                return DumpStatus::DUMP_INVALID_ARG;
            }
            if (!SetIpcStatParam(dumpContext, ipcStatParams[0])) {
                PrintErrorMessage(dumpContext, INVALID_ARG_MSG + ":" + ipcStatParams[0]);
                return DumpStatus::DUMP_INVALID_ARG;
            }
        }
    } else {
        std::string optionName = RemoveCharacterFromStr(argv[optind - 1], '-');
        std::string errorStr = UNRECOGNIZED_ERROR_MSG + optionName;
        PrintErrorMessage(dumpContext, errorStr);
        return DumpStatus::DUMP_INVALID_ARG;
    }
    return DumpStatus::DUMP_OK;
}

DumpStatus CmdParse::CheckProcessAlive(DumpContext& dumpContext)
{
    auto opts = dumpContext.GetDumperOpts();
    std::vector<int> pids = {
        opts->cpuUsagePid,
        opts->memPid,
        opts->processPid,
        opts->storagePid,
        opts->netPid,
        opts->dumpJsHeapMemPid,
        opts->ipcStatPid
    };

    for (int pid : pids) {
        if (pid < -1) {
            PrintErrorMessage(dumpContext, INVALID_ARG_MSG + std::to_string(pid));
            return DumpStatus::DUMP_FAIL;
        }
        if (pid > -1 && !DumpUtils::CheckProcessAlive(pid)) {
            PrintErrorMessage(dumpContext, PID_ERROR_MSG + std::to_string(pid));
            return DumpStatus::DUMP_FAIL;
        }
    }
    return DumpStatus::DUMP_OK;
}

bool CmdParse::IsSADumperOption(char *argv[])
{
    for (int i = optind - 2; i > 0; i--) {
        if (IsSubStr(argv[i], "-")) {
            return std::string(argv[i]) == "-s" || std::string(argv[i]) == "-a";
        }
    }
    return false;
}

DumpStatus CmdParse::VerifyOptions(const DumpContext& dumpContext)
{
    auto opts = dumpContext.GetDumperOpts();
    if (opts->isDumpList && ((!opts->isDumpService) && (!opts->isDumpSystemAbility) && (!opts->isDumpSystem))) {
        PrintErrorMessage(dumpContext, INVALID_ARG_MSG + "-l");
        return DumpStatus::DUMP_FAIL;
    }
    for (const auto& name : opts->abilityNames) {
        if (DumpUtils::StrToId(name) == -1) {
            PrintErrorMessage(dumpContext, INVALID_ARG_MSG + name);
            return DumpStatus::DUMP_FAIL;
        }
    }
    if (!opts->systemArgs.empty()) {
        if (opts->systemArgs.find("base") == opts->systemArgs.end() &&
            opts->systemArgs.find("system") == opts->systemArgs.end()) {
            std::string errorMsg = INVALID_ARG_MSG;
            for (const auto& arg : opts->systemArgs) {
                errorMsg += arg + " ";
            }
            PrintErrorMessage(dumpContext, errorMsg);
            return DumpStatus::DUMP_FAIL;
        }
    }
    return DumpStatus::DUMP_OK;
}
} // namespace HiviewDFX
} // namespace OHOS
