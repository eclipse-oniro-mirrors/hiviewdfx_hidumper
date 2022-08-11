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
#include "util/config_data.h"
#include <pubdef.h>
#include "common/dumper_constant.h"
namespace OHOS {
namespace HiviewDFX {
const std::string ConfigData::CONFIG_NAME_SPLIT = "_";
const std::string ConfigData::CONFIG_GROUP = "group";
const std::string ConfigData::CONFIG_GROUP_ = ConfigData::CONFIG_GROUP + ConfigData::CONFIG_NAME_SPLIT;
const std::string ConfigData::CONFIG_MINIGROUP = "groupmini";
const std::string ConfigData::CONFIG_MINIGROUP_ = ConfigData::CONFIG_MINIGROUP + ConfigData::CONFIG_NAME_SPLIT;
const std::string ConfigData::CONFIG_DUMPER = "dumper";
const std::string ConfigData::CONFIG_DUMPER_ = ConfigData::CONFIG_DUMPER + ConfigData::CONFIG_NAME_SPLIT;
const std::string ConfigData::CONFIG_DUMPER_LIST = ConfigData::CONFIG_DUMPER_ + "list";
const std::string ConfigData::CONFIG_DUMPER_LIST_ = ConfigData::CONFIG_DUMPER_LIST + ConfigData::CONFIG_NAME_SPLIT;
const std::string ConfigData::CONFIG_DUMPER_LIST_SERVICE = ConfigData::CONFIG_DUMPER_LIST_ + "service";
const std::string ConfigData::CONFIG_DUMPER_LIST_SYSTEM_ABILITY = ConfigData::CONFIG_DUMPER_LIST_ + "system_ability";
const std::string ConfigData::CONFIG_DUMPER_LIST_SYSTEM = ConfigData::CONFIG_DUMPER_LIST_ + "system";
const std::string ConfigData::CONFIG_GROUP_CPU_FREQ = ConfigData::CONFIG_GROUP_ + "cpufreq";
const std::string ConfigData::CONFIG_GROUP_CPU_USAGE = ConfigData::CONFIG_GROUP_ + "cpuusage";
const std::string ConfigData::CONFIG_GROUP_LOG = ConfigData::CONFIG_GROUP_ + "log";
const std::string ConfigData::CONFIG_GROUP_LOG_ = ConfigData::CONFIG_GROUP_LOG + ConfigData::CONFIG_NAME_SPLIT;
const std::string ConfigData::CONFIG_GROUP_LOG_KERNEL = ConfigData::CONFIG_GROUP_LOG_ + "kernel";
const std::string ConfigData::CONFIG_GROUP_LOG_HILOG = ConfigData::CONFIG_GROUP_LOG_ + "hilog";
const std::string ConfigData::CONFIG_GROUP_LOG_INIT = ConfigData::CONFIG_GROUP_LOG_ + "init";
const std::string ConfigData::CONFIG_GROUP_MEMORY = ConfigData::CONFIG_GROUP_ + "memory";
const std::string ConfigData::CONFIG_GROUP_STORAGE = ConfigData::CONFIG_GROUP_ + "storage";
const std::string ConfigData::CONFIG_GROUP_NET = ConfigData::CONFIG_GROUP_ + "net";
const std::string ConfigData::CONFIG_GROUP_SERVICE = ConfigData::CONFIG_GROUP_ + "service";
const std::string ConfigData::CONFIG_GROUP_ABILITY = ConfigData::CONFIG_GROUP_ + "ability";
const std::string ConfigData::CONFIG_GROUP_SYSTEM = ConfigData::CONFIG_GROUP_ + "system";
const std::string ConfigData::CONFIG_GROUP_SYSTEM_ = ConfigData::CONFIG_GROUP_SYSTEM + ConfigData::CONFIG_NAME_SPLIT;
const std::string ConfigData::CONFIG_GROUP_SYSTEM_BASE = ConfigData::CONFIG_GROUP_SYSTEM_ + "base";
const std::string ConfigData::CONFIG_GROUP_SYSTEM_SYSTEM = ConfigData::CONFIG_GROUP_SYSTEM_ + "system";
const std::string ConfigData::CONFIG_GROUP_PROCESSES = ConfigData::CONFIG_GROUP_ + "processes";
const std::string ConfigData::CONFIG_GROUP_PROCESSES_ENG = ConfigData::CONFIG_GROUP_ + "eng_processes";
const std::string ConfigData::CONFIG_GROUP_PROCESSES_PID = ConfigData::CONFIG_GROUP_ + "pid_processes";
const std::string ConfigData::CONFIG_GROUP_PROCESSES_PID_ENG = ConfigData::CONFIG_GROUP_ + "pid_eng_processes";
const std::string ConfigData::CONFIG_GROUP_FAULT_LOG = ConfigData::CONFIG_GROUP_ + "faultlog";
const std::string ConfigData::CONFIG_GROUP_STACK = ConfigData::CONFIG_GROUP_ + "stack";
const std::string ConfigData::CONFIG_GROUP_TEST = ConfigData::CONFIG_GROUP_ + "test";
const std::string ConfigData::STR_ABILITY = "ability";
const std::string ConfigData::STR_BASE = "base";
const std::string ConfigData::STR_SERVICE = "service";
const std::string ConfigData::STR_SYSTEM = "system";
const ConfigData::ItemCfg ConfigData::baseInfoDumper_[] = {
    {
        .name_ = "dumper_base_info",
        .desc_ = "Base Information",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::API_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
};

const ConfigData::ItemCfg ConfigData::kernelVersionDumper_[] = {
    {
        .name_ = "dumper_kernel_version",
        .desc_ = "Kernel Version",
        .target_ = "/proc/version",
        .section_ = "",
        .class_ = DumperConstant::FILE_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
};

const ConfigData::ItemCfg ConfigData::cmdlineDumper_[] = {
    {
        .name_ = "dumper_command_line",
        .desc_ = "Command Line",
        .target_ = "/proc/cmdline",
        .section_ = "",
        .class_ = DumperConstant::FILE_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
};

const ConfigData::ItemCfg ConfigData::kernelWakeSourcesDumper_[] = {
    {
        .name_ = "dumper_kernel_wake_sources",
        .desc_ = "KERNEL WAKE SOURCES",
        .target_ = "/sys/kernel/debug/wakeup_sources",
        .section_ = "",
        .class_ = DumperConstant::FILE_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
};

const ConfigData::ItemCfg ConfigData::kernelCpufreqDumper_[] = {
    {
        .name_ = "dumper_kernel_cpu_freq",
        .desc_ = "KERNEL CPUFREQ",
        .target_ = "cat /sys/devices/system/cpu/cpu%cpuid/cpufreq/cpuinfo_cur_freq",
        .section_ = "",
        .class_ = DumperConstant::CMD_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "cat /sys/devices/system/cpu/cpu%cpuid/cpufreq/cpuinfo_max_freq",
        .section_ = "",
        .class_ = DumperConstant::CMD_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
};

const ConfigData::ItemCfg ConfigData::uptimeDumper_[] = {
    {
        .name_ = "dumper_uptime",
        .desc_ = "Up Time",
        .target_ = "uptime -p",
        .section_ = "",
        .class_ = DumperConstant::CMD_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
};

const ConfigData::ItemCfg ConfigData::cpuUsageDumper_[] = {
    {
        .name_ = "dumper_cpu_usage",
        .desc_ = "CPU Usage",
        .target_ = "%pid",
        .section_ = "",
        .class_ = DumperConstant::CPU_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
};

const ConfigData::ItemCfg ConfigData::cpuFreqDumper_[] = {
    {
        .name_ = "dumper_cpu_freq",
        .desc_ = "CPU Frequency",
        .target_ = "cat /sys/devices/system/cpu/cpu%cpuid/cpufreq/cpuinfo_cur_freq",
        .section_ = "",
        .class_ = DumperConstant::CMD_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "cat /sys/devices/system/cpu/cpu%cpuid/cpufreq/cpuinfo_max_freq",
        .section_ = "",
        .class_ = DumperConstant::CMD_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
};

const ConfigData::ItemCfg ConfigData::memDumper_[] = {
    {
        .name_ = "dumper_mem",
        .desc_ = "Memory Information",
        .target_ = "%pid",
        .section_ = "",
        .class_ = DumperConstant::MEMORY_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::LOOP,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::LOOP,
        .filterCfg_ = "",
    },
};

const ConfigData::ItemCfg ConfigData::envDumper_[] = {
    {
        .name_ = "dumper_env",
        .desc_ = "Environment Variable",
        .target_ = "printenv",
        .section_ = "",
        .class_ = DumperConstant::CMD_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
};

const ConfigData::ItemCfg ConfigData::kernelModuleDumper_[] = {
    {
        .name_ = "dumper_kernel_module",
        .desc_ = "Kernel Module",
        .target_ = "/proc/modules",
        .section_ = "",
        .class_ = DumperConstant::FILE_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "lsmod",
        .section_ = "",
        .class_ = DumperConstant::CMD_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
};

const ConfigData::ItemCfg ConfigData::dumpFormatVersionDumper_[] = {
    {
        .name_ = "dumper_dump_format_version",
        .desc_ = "HiDumper Version",
        .target_ = "HiDumper version:x.x",
        .section_ = "",
        .class_ = DumperConstant::VERSION_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
};

const ConfigData::ItemCfg ConfigData::slabinfoDumper_[] = {
    {
        .name_ = "dumper_slabinfo",
        .desc_ = "SLAB INFO",
        .target_ = "/proc/slabinfo",
        .section_ = "",
        .class_ = DumperConstant::FILE_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
};

const ConfigData::ItemCfg ConfigData::zoneinfoDumper_[] = {
    {
        .name_ = "dumper_zoneinfo",
        .desc_ = "ZONE INFO",
        .target_ = "/proc/zoneinfo",
        .section_ = "",
        .class_ = DumperConstant::FILE_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
};

const ConfigData::ItemCfg ConfigData::vmStatDumper_[] = {
    {
        .name_ = "dumper_vmstat",
        .desc_ = "VIRTUAL MEMORY STATS",
        .target_ = "/proc/vmstat",
        .section_ = "",
        .class_ = DumperConstant::FILE_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
};

const ConfigData::ItemCfg ConfigData::vmAllocInfoDumper_[] = {
    {
        .name_ = "dumper_vmallocinfo",
        .desc_ = "VIRTUAL MEMORY STATS",
        .target_ = "/proc/vmallocinfo",
        .section_ = "",
        .class_ = DumperConstant::FILE_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
};

const ConfigData::ItemCfg ConfigData::crashDumper_[] = {
    {
        .name_ = "dumper_crash",
        .desc_ = "Crash Log",
        .target_ = "/data/log/faultlog/faultlogger",
        .section_ = "",
        .class_ = DumperConstant::FILE_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
};

const ConfigData::ItemCfg ConfigData::kernelLogDumper_[] = {
    {
        .name_ = "dumper_kernel_log",
        .desc_ = "Kernel Log",
        .target_ = "dmesg",
        .section_ = "",
        .class_ = DumperConstant::CMD_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::LOOP,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::LOOP,
        .filterCfg_ = "",
    },
};

const ConfigData::ItemCfg ConfigData::hilogDumper_[] = {
    {
        .name_ = "dumper_hilog",
        .desc_ = "Hilog",
        .target_ = "hilog -x --exit",
        .section_ = "",
        .class_ = DumperConstant::CMD_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::LOOP,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::LOOP,
        .filterCfg_ = "",
    },
};

const ConfigData::ItemCfg ConfigData::portDumper_[] = {
    {
        .name_ = "dumper_port",
        .desc_ = "Port Information",
        .target_ = "netstat -nW",
        .section_ = "",
        .class_ = DumperConstant::CMD_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
};

const ConfigData::ItemCfg ConfigData::packetDumper_[] = {
    {
        .name_ = "dumper_packet",
        .desc_ = "Packet State",
        .target_ = "/proc/net/dev",
        .section_ = "",
        .class_ = DumperConstant::FILE_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "/proc/net/xt_qtaguid/iface_stat_all",
        .section_ = "",
        .class_ = DumperConstant::FILE_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "/proc/net/xt_qtaguid/iface_stat_fmt",
        .section_ = "",
        .class_ = DumperConstant::FILE_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "/proc/net/xt_qtaguid/ctrl",
        .section_ = "",
        .class_ = DumperConstant::FILE_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "/proc/net/xt_qtaguid/stats",
        .section_ = "",
        .class_ = DumperConstant::FILE_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
};

const ConfigData::ItemCfg ConfigData::ipDumper_[] = {
    {
        .name_ = "dumper_ip",
        .desc_ = "IP v4/6 State",
        .target_ = "ifconfig -a",
        .section_ = "",
        .class_ = DumperConstant::CMD_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
};

const ConfigData::ItemCfg ConfigData::ipTableDumper_[] = {
    {
        .name_ = "dumper_ip_table",
        .desc_ = "IPTable v4/6 Information",
        .target_ = "iptables -L -nvx",
        .section_ = "",
        .class_ = DumperConstant::CMD_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "ip6tables -L -nvx",
        .section_ = "",
        .class_ = DumperConstant::CMD_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "iptables -t nat -L -nvx",
        .section_ = "",
        .class_ = DumperConstant::CMD_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "iptables -t mangle -L -nvx",
        .section_ = "",
        .class_ = DumperConstant::CMD_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "ip6tables -t mangle -L -nvx",
        .section_ = "",
        .class_ = DumperConstant::CMD_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "iptables -t raw -L -nvx",
        .section_ = "",
        .class_ = DumperConstant::CMD_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "ip6tables -t raw -L -nvx",
        .section_ = "",
        .class_ = DumperConstant::CMD_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
};

const ConfigData::ItemCfg ConfigData::routeTableDumper_[] = {
    {
        .name_ = "dumper_route_table",
        .desc_ = "IP Table v4/6 Information",
        .target_ = "/data/misc/net/rt_tables",
        .section_ = "",
        .class_ = DumperConstant::FILE_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
};

const ConfigData::ItemCfg ConfigData::ipcDumper_[] = {
    {
        .name_ = "dumper_ipc",
        .desc_ = "IPC Information",
        .target_ = "/sys/kernel/debug/binder/failed_transaction_log",
        .section_ = "",
        .class_ = DumperConstant::FILE_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "/sys/kernel/debug/binder/transaction_log",
        .section_ = "",
        .class_ = DumperConstant::FILE_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "/sys/kernel/debug/binder/transactions",
        .section_ = "",
        .class_ = DumperConstant::FILE_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "/sys/kernel/debug/binder/stats",
        .section_ = "",
        .class_ = DumperConstant::FILE_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "/sys/kernel/debug/binder/state",
        .section_ = "",
        .class_ = DumperConstant::FILE_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
};

const ConfigData::ItemCfg ConfigData::ipRulesDumper_[] = {
    {
        .name_ = "dumper_ip_rules",
        .desc_ = "IP RULES v4/6",
        .target_ = "ip link",
        .section_ = "",
        .class_ = DumperConstant::CMD_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "ip -4 addr show",
        .section_ = "",
        .class_ = DumperConstant::CMD_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "ip -6 addr show",
        .section_ = "",
        .class_ = DumperConstant::CMD_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "ip rule show",
        .section_ = "",
        .class_ = DumperConstant::CMD_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "ip -6 rule show",
        .section_ = "",
        .class_ = DumperConstant::CMD_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
};

const ConfigData::ItemCfg ConfigData::storageStateDumper_[] = {
    {
        .name_ = "dumper_storage_state",
        .desc_ = "Storage State",
        .target_ = "storaged -u -p",
        .section_ = "",
        .class_ = DumperConstant::CMD_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
};

const ConfigData::ItemCfg ConfigData::blockDumper_[] = {
    {
        .name_ = "dumper_block",
        .desc_ = "Block Information",
        .target_ = "df -k",
        .section_ = "",
        .class_ = DumperConstant::CMD_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
};

const ConfigData::ItemCfg ConfigData::fileDumper_[] = {
    {
        .name_ = "dumper_file",
        .desc_ = "File Information",
        .target_ = "lsof",
        .section_ = "",
        .class_ = DumperConstant::CMD_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
};

const ConfigData::ItemCfg ConfigData::topIoDumper_[] = {
    {
        .name_ = "dumper_top_io",
        .desc_ = "TOP IO Information",
        .target_ = "iotop -n 1 -m 100",
        .section_ = "",
        .class_ = DumperConstant::CMD_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FILE_FORMAT_DUMP_FILTER,
        .level_ = DumperConstant::LEVEL_ALL,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
};

const ConfigData::ItemCfg ConfigData::mountsDumper_[] = {
    {
        .name_ = "dumper_mounts",
        .desc_ = "Mount List",
        .target_ = "/proc/mounts",
        .section_ = "",
        .class_ = DumperConstant::FILE_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
};

const ConfigData::ItemCfg ConfigData::threadsDumper_[] = {
    {
        .name_ = "dumper_threads",
        .desc_ = "Processes/Threads List",
        .target_ = "ps -efT",
        .section_ = "",
        .class_ = DumperConstant::CMD_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
};

const ConfigData::ItemCfg ConfigData::threadsPidDumper_[] = {
    {
        .name_ = "dumper_threads_pid",
        .desc_ = "Processes/Threads List",
        .target_ = "ps -efT -p %pid",
        .section_ = "",
        .class_ = DumperConstant::CMD_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
};

const ConfigData::ItemCfg ConfigData::smapDumper_[] = {
    {
        .name_ = "dumper_smap",
        .desc_ = "Process SMAP Information",
        .target_ = "/proc/%pid/smaps",
        .section_ = "",
        .class_ = DumperConstant::FILE_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::LOOP,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::LOOP,
        .filterCfg_ = "",
    },
};

const ConfigData::ItemCfg ConfigData::mapDumper_[] = {
    {
        .name_ = "dumper_map",
        .desc_ = "Process MAP Information",
        .target_ = "/proc/%pid/maps",
        .section_ = "",
        .class_ = DumperConstant::FILE_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::LOOP,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::LOOP,
        .filterCfg_ = "",
    },
};

const ConfigData::ItemCfg ConfigData::blockChannelDumper_[] = {
    {
        .name_ = "dumper_block_channel",
        .desc_ = "Block Channel",
        .target_ = "/proc/%pid/wchan",
        .section_ = "",
        .class_ = DumperConstant::FILE_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
};

const ConfigData::ItemCfg ConfigData::excuteTimeDumper_[] = {
    {
        .name_ = "dumper_excute_time",
        .desc_ = "Excute Time",
        .target_ = "ps -o \"TIME\" -p %pid",
        .section_ = "",
        .class_ = DumperConstant::CMD_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
};

const ConfigData::ItemCfg ConfigData::mountInfoDumper_[] = {
    {
        .name_ = "dumper_mount_info",
        .desc_ = "Mount Information",
        .target_ = "/proc/%pid/mountinfo",
        .section_ = "",
        .class_ = DumperConstant::FILE_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::LOOP,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::LOOP,
        .filterCfg_ = "",
    },
};

const ConfigData::ItemCfg ConfigData::systemAbilityDumper_[] = {
    {
        .name_ = "dumper_system_ability",
        .desc_ = "System Ability Information",
        .target_ = ConfigData::STR_ABILITY,
        .section_ = "",
        .class_ = DumperConstant::SA_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
};

const ConfigData::ItemCfg ConfigData::stackDumper_[] = {
    {
        .name_ = "dumper_stack",
        .desc_ = "Dump Stack Info",
        .target_ = "dumpcatcher -p %pid",
        .section_ = "",
        .class_ = DumperConstant::CMD_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
};

const ConfigData::ItemCfg ConfigData::listServiceDumper_[] = {
    {
        .name_ = ConfigData::CONFIG_DUMPER_LIST_SERVICE,
        .desc_ = "list service",
        .target_ = ConfigData::STR_SERVICE,
        .section_ = "",
        .class_ = DumperConstant::LIST_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
};

const ConfigData::ItemCfg ConfigData::listSystemAbilityDumper_[] = {
    {
        .name_ = ConfigData::CONFIG_DUMPER_LIST_SYSTEM_ABILITY,
        .desc_ = "list system ability",
        .target_ = ConfigData::STR_ABILITY,
        .section_ = "",
        .class_ = DumperConstant::LIST_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
};

const ConfigData::ItemCfg ConfigData::listSystemDumper_[] = {
    {
        .name_ = ConfigData::CONFIG_DUMPER_LIST_SYSTEM,
        .desc_ = "list system",
        .target_ = ConfigData::STR_SYSTEM,
        .section_ = "",
        .class_ = DumperConstant::LIST_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
};

const ConfigData::ItemCfg ConfigData::testDumper_[] = {
    {
        .name_ = "dumper_test",
        .desc_ = "test used dumper",
        .target_ = "/data/local/tmp/hidumper_mockdata_0001.txt",
        .section_ = "",
        .class_ = DumperConstant::FILE_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "cat /data/local/tmp/hidumper_mockdata_0002.txt",
        .section_ = "",
        .class_ = DumperConstant::CMD_DUMPER,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
    {
        .name_ = "",
        .desc_ = "",
        .target_ = "",
        .section_ = "",
        .class_ = DumperConstant::FD_OUTPUT,
        .level_ = DumperConstant::NONE,
        .loop_ = DumperConstant::NONE,
        .filterCfg_ = "",
    },
};

const ConfigData::DumperCfg ConfigData::dumpers_[] = {
    {.name_ = baseInfoDumper_[0].name_,
     .desc_ = baseInfoDumper_[0].desc_,
     .list_ = baseInfoDumper_,
     .size_ = ARRAY_SIZE(baseInfoDumper_)},
    {.name_ = kernelVersionDumper_[0].name_,
     .desc_ = kernelVersionDumper_[0].desc_,
     .list_ = kernelVersionDumper_,
     .size_ = ARRAY_SIZE(kernelVersionDumper_)},
    {.name_ = cmdlineDumper_[0].name_,
     .desc_ = cmdlineDumper_[0].desc_,
     .list_ = cmdlineDumper_,
     .size_ = ARRAY_SIZE(cmdlineDumper_)},
    {.name_ = kernelWakeSourcesDumper_[0].name_,
     .desc_ = kernelWakeSourcesDumper_[0].desc_,
     .list_ = kernelWakeSourcesDumper_,
     .size_ = ARRAY_SIZE(kernelWakeSourcesDumper_)},
    {.name_ = kernelCpufreqDumper_[0].name_,
     .desc_ = kernelCpufreqDumper_[0].desc_,
     .list_ = kernelCpufreqDumper_,
     .size_ = ARRAY_SIZE(kernelCpufreqDumper_)},
    {.name_ = uptimeDumper_[0].name_,
     .desc_ = uptimeDumper_[0].desc_,
     .list_ = uptimeDumper_,
     .size_ = ARRAY_SIZE(uptimeDumper_)},
    {.name_ = cpuUsageDumper_[0].name_,
     .desc_ = cpuUsageDumper_[0].desc_,
     .list_ = cpuUsageDumper_,
     .size_ = ARRAY_SIZE(cpuUsageDumper_)},
    {.name_ = cpuFreqDumper_[0].name_,
     .desc_ = cpuFreqDumper_[0].desc_,
     .list_ = cpuFreqDumper_,
     .size_ = ARRAY_SIZE(cpuFreqDumper_)},
    {.name_ = memDumper_[0].name_, .desc_ = memDumper_[0].desc_, .list_ = memDumper_, .size_ = ARRAY_SIZE(memDumper_)},
    {.name_ = envDumper_[0].name_, .desc_ = envDumper_[0].desc_, .list_ = envDumper_, .size_ = ARRAY_SIZE(envDumper_)},
    {.name_ = kernelModuleDumper_[0].name_,
     .desc_ = kernelModuleDumper_[0].desc_,
     .list_ = kernelModuleDumper_,
     .size_ = ARRAY_SIZE(kernelModuleDumper_)},
    {.name_ = dumpFormatVersionDumper_[0].name_,
     .desc_ = dumpFormatVersionDumper_[0].desc_,
     .list_ = dumpFormatVersionDumper_,
     .size_ = ARRAY_SIZE(dumpFormatVersionDumper_)},
    {.name_ = slabinfoDumper_[0].name_,
     .desc_ = slabinfoDumper_[0].desc_,
     .list_ = slabinfoDumper_,
     .size_ = ARRAY_SIZE(slabinfoDumper_)},
    {.name_ = zoneinfoDumper_[0].name_,
     .desc_ = zoneinfoDumper_[0].desc_,
     .list_ = zoneinfoDumper_,
     .size_ = ARRAY_SIZE(zoneinfoDumper_)},
    {.name_ = vmStatDumper_[0].name_,
     .desc_ = vmStatDumper_[0].desc_,
     .list_ = vmStatDumper_,
     .size_ = ARRAY_SIZE(vmStatDumper_)},
    {.name_ = vmAllocInfoDumper_[0].name_,
     .desc_ = vmAllocInfoDumper_[0].desc_,
     .list_ = vmAllocInfoDumper_,
     .size_ = ARRAY_SIZE(vmAllocInfoDumper_)},
    {.name_ = crashDumper_[0].name_,
     .desc_ = crashDumper_[0].desc_,
     .list_ = crashDumper_,
     .size_ = ARRAY_SIZE(crashDumper_)},
    {.name_ = kernelLogDumper_[0].name_,
     .desc_ = kernelLogDumper_[0].desc_,
     .list_ = kernelLogDumper_,
     .size_ = ARRAY_SIZE(kernelLogDumper_)},
    {.name_ = hilogDumper_[0].name_,
     .desc_ = hilogDumper_[0].desc_,
     .list_ = hilogDumper_,
     .size_ = ARRAY_SIZE(hilogDumper_)},
    {.name_ = portDumper_[0].name_,
     .desc_ = portDumper_[0].desc_,
     .list_ = portDumper_,
     .size_ = ARRAY_SIZE(portDumper_)},
    {.name_ = packetDumper_[0].name_,
     .desc_ = packetDumper_[0].desc_,
     .list_ = packetDumper_,
     .size_ = ARRAY_SIZE(packetDumper_)},
    {.name_ = ipDumper_[0].name_, .desc_ = ipDumper_[0].desc_, .list_ = ipDumper_, .size_ = ARRAY_SIZE(ipDumper_)},
    {.name_ = ipTableDumper_[0].name_,
     .desc_ = ipTableDumper_[0].desc_,
     .list_ = ipTableDumper_,
     .size_ = ARRAY_SIZE(ipTableDumper_)},
    {.name_ = routeTableDumper_[0].name_,
     .desc_ = routeTableDumper_[0].desc_,
     .list_ = routeTableDumper_,
     .size_ = ARRAY_SIZE(routeTableDumper_)},
    {.name_ = ipcDumper_[0].name_, .desc_ = ipcDumper_[0].desc_, .list_ = ipcDumper_, .size_ = ARRAY_SIZE(ipcDumper_)},
    {.name_ = ipRulesDumper_[0].name_,
     .desc_ = ipRulesDumper_[0].desc_,
     .list_ = ipRulesDumper_,
     .size_ = ARRAY_SIZE(ipRulesDumper_)},
    {.name_ = storageStateDumper_[0].name_,
     .desc_ = storageStateDumper_[0].desc_,
     .list_ = storageStateDumper_,
     .size_ = ARRAY_SIZE(storageStateDumper_)},
    {.name_ = blockDumper_[0].name_,
     .desc_ = blockDumper_[0].desc_,
     .list_ = blockDumper_,
     .size_ = ARRAY_SIZE(blockDumper_)},
    {.name_ = fileDumper_[0].name_,
     .desc_ = fileDumper_[0].desc_,
     .list_ = fileDumper_,
     .size_ = ARRAY_SIZE(fileDumper_)},
    {.name_ = topIoDumper_[0].name_,
     .desc_ = topIoDumper_[0].desc_,
     .list_ = topIoDumper_,
     .size_ = ARRAY_SIZE(topIoDumper_)},
    {.name_ = mountsDumper_[0].name_,
     .desc_ = mountsDumper_[0].desc_,
     .list_ = mountsDumper_,
     .size_ = ARRAY_SIZE(mountsDumper_)},
    {.name_ = threadsDumper_[0].name_,
     .desc_ = threadsDumper_[0].desc_,
     .list_ = threadsDumper_,
     .size_ = ARRAY_SIZE(threadsDumper_)},
    {.name_ = threadsPidDumper_[0].name_,
     .desc_ = threadsPidDumper_[0].desc_,
     .list_ = threadsPidDumper_,
     .size_ = ARRAY_SIZE(threadsPidDumper_)},
    {.name_ = smapDumper_[0].name_,
     .desc_ = smapDumper_[0].desc_,
     .list_ = smapDumper_,
     .size_ = ARRAY_SIZE(smapDumper_)},
    {.name_ = mapDumper_[0].name_, .desc_ = mapDumper_[0].desc_, .list_ = mapDumper_, .size_ = ARRAY_SIZE(mapDumper_)},
    {.name_ = blockChannelDumper_[0].name_,
     .desc_ = blockChannelDumper_[0].desc_,
     .list_ = blockChannelDumper_,
     .size_ = ARRAY_SIZE(blockChannelDumper_)},
    {.name_ = excuteTimeDumper_[0].name_,
     .desc_ = excuteTimeDumper_[0].desc_,
     .list_ = excuteTimeDumper_,
     .size_ = ARRAY_SIZE(excuteTimeDumper_)},
    {.name_ = mountInfoDumper_[0].name_,
     .desc_ = mountInfoDumper_[0].desc_,
     .list_ = mountInfoDumper_,
     .size_ = ARRAY_SIZE(mountInfoDumper_)},
    {.name_ = systemAbilityDumper_[0].name_,
     .desc_ = systemAbilityDumper_[0].desc_,
     .list_ = systemAbilityDumper_,
     .size_ = ARRAY_SIZE(systemAbilityDumper_)},
    {.name_ = stackDumper_[0].name_,
     .desc_ = stackDumper_[0].desc_,
     .list_ = stackDumper_,
     .size_ = ARRAY_SIZE(stackDumper_)},
    {.name_ = listServiceDumper_[0].name_,
     .desc_ = listServiceDumper_[0].desc_,
     .list_ = listServiceDumper_,
     .size_ = ARRAY_SIZE(listServiceDumper_)},
    {.name_ = listSystemAbilityDumper_[0].name_,
     .desc_ = listSystemAbilityDumper_[0].desc_,
     .list_ = listSystemAbilityDumper_,
     .size_ = ARRAY_SIZE(listSystemAbilityDumper_)},
    {.name_ = listSystemDumper_[0].name_,
     .desc_ = listSystemDumper_[0].desc_,
     .list_ = listSystemDumper_,
     .size_ = ARRAY_SIZE(listSystemDumper_)},
    {.name_ = testDumper_[0].name_,
     .desc_ = testDumper_[0].desc_,
     .list_ = testDumper_,
     .size_ = ARRAY_SIZE(testDumper_)},
};

const std::string ConfigData::cpuFreqGroup_[] = {
    "dumper_cpu_freq",
};

const std::string ConfigData::cpuUsageGroup_[] = {
    "dumper_cpu_usage",
};

const std::string ConfigData::logKernelGroup_[] = {
    "dumper_kernel_log",
};

const std::string ConfigData::logHilogGroup_[] = {
    "dumper_hilog",
};

const std::string ConfigData::logInitGroup_[] = {
    "cpu_dumper_name_000",
};

const std::string ConfigData::memoryGroup_[] = {
    "dumper_mem",
};

const std::string ConfigData::storageGroup_[] = {
    "dumper_storage_state", "dumper_block", "dumper_file", "dumper_top_io", "dumper_mounts",
};

const std::string ConfigData::netGroup_[] = {
    "dumper_port",        "dumper_packet", "dumper_ip",       "dumper_ip_table",
    "dumper_route_table", "dumper_ipc",    "dumper_ip_rules",
};

const std::string ConfigData::serviceGroup_[] = {
    "mem_dumper_name_000",
};

const std::string ConfigData::systemAbilityGroup_[] = {
    "dumper_system_ability",
};

const std::string ConfigData::systemBaseGroup_[] = {
    "dumper_base_info",           "dumper_kernel_version", "dumper_command_line",
    "dumper_kernel_wake_sources", "dumper_uptime",
};

const std::string ConfigData::systemBaseCpuIdGroupMini_[] = {
    "dumper_kernel_cpu_freq",
};

const std::string ConfigData::systemSystemGroup_[] = {
    "dumper_env",
    "dumper_kernel_module",
    "dumper_dump_format_version",
    "dumper_slabinfo",
    "dumper_zoneinfo",
    "dumper_vmstat",
    "dumper_vmallocinfo",
    "groupmini_cpuid_expand_systemSystemGroup",
    "groupmini_pid_nonexpand_systemSystemGroup",
};

const std::string ConfigData::systemSystemCpuIdGroupMini_[] = {
    "dumper_cpu_freq",
};

const std::string ConfigData::systemSystemPidGroupMini_[] = {
    "dumper_cpu_usage",
    "dumper_mem",
};

const std::string ConfigData::processesGroup_[] = {
    "dumper_threads",
    "groupmini_expand_processesGroup",
};

const std::string ConfigData::processesGroup_eng_[] = {
    "dumper_threads",
    "groupmini_expand_processesGroup_eng",
};

const std::string ConfigData::processesPidGroup_[] = {
    "groupmini_pid_expand_processesGroup",
};

const std::string ConfigData::processesPidGroup_eng_[] = {
    "groupmini_pid_expand_processesGroup_eng",
};

const std::string ConfigData::processesGroupMini_[] = {
    "dumper_block_channel",
    "dumper_excute_time",
    "dumper_mount_info",
};

const std::string ConfigData::processesGroupMini_eng_[] = {
    "dumper_map",
    "dumper_block_channel",
    "dumper_excute_time",
    "dumper_mount_info",
};

const std::string ConfigData::processesPidGroupMini_[] = {
    "dumper_threads_pid",
    "dumper_block_channel",
    "dumper_excute_time",
    "dumper_mount_info",
};

const std::string ConfigData::processesPidGroupMini_eng_[] = {
    "dumper_threads_pid",
    "dumper_map",
    "dumper_block_channel",
    "dumper_excute_time",
    "dumper_mount_info",
};

const std::string ConfigData::faultLogGroup_[] = {
    "dumper_crash",
};

const std::string ConfigData::stackGroup_[] = {
    "dumper_stack",
};

const std::string ConfigData::testGroup_[] = {
    "dumper_test",
};

const ConfigData::GroupCfg ConfigData::groups_[] = {
    {
        .name_ = ConfigData::CONFIG_GROUP_CPU_FREQ,
        .desc_ = "group of cpu freq dumper",
        .list_ = cpuFreqGroup_,
        .size_ = ARRAY_SIZE(cpuFreqGroup_),
        .type_ = DumperConstant::GROUPTYPE_CPUID,
        .expand_ = true,
    },
    {
        .name_ = ConfigData::CONFIG_GROUP_CPU_USAGE,
        .desc_ = "group of cpu usage dumper",
        .list_ = cpuUsageGroup_,
        .size_ = ARRAY_SIZE(cpuUsageGroup_),
        .type_ = DumperConstant::GROUPTYPE_PID,
        .expand_ = false,
    },
    {
        .name_ = ConfigData::CONFIG_GROUP_LOG_KERNEL,
        .desc_ = "group of kernel log dumper",
        .list_ = logKernelGroup_,
        .size_ = ARRAY_SIZE(logKernelGroup_),
        .type_ = DumperConstant::NONE,
        .expand_ = false,
    },
    {
        .name_ = ConfigData::CONFIG_GROUP_LOG_HILOG,
        .desc_ = "group of hilog dumper",
        .list_ = logHilogGroup_,
        .size_ = ARRAY_SIZE(logHilogGroup_),
        .type_ = DumperConstant::NONE,
        .expand_ = false,
    },
    {
        .name_ = ConfigData::CONFIG_GROUP_LOG_INIT,
        .desc_ = "group of init log dumper",
        .list_ = logInitGroup_,
        .size_ = ARRAY_SIZE(logInitGroup_),
        .type_ = DumperConstant::NONE,
        .expand_ = false,
    },
    {
        .name_ = ConfigData::CONFIG_GROUP_MEMORY,
        .desc_ = "group of memory dumper",
        .list_ = memoryGroup_,
        .size_ = ARRAY_SIZE(memoryGroup_),
        .type_ = DumperConstant::GROUPTYPE_PID,
        .expand_ = false,
    },
    {
        .name_ = ConfigData::CONFIG_GROUP_STORAGE,
        .desc_ = "group of storage dumper",
        .list_ = storageGroup_,
        .size_ = ARRAY_SIZE(storageGroup_),
        .type_ = DumperConstant::NONE,
        .expand_ = false,
    },
    {
        .name_ = ConfigData::CONFIG_GROUP_NET,
        .desc_ = "group of net dumper",
        .list_ = netGroup_,
        .size_ = ARRAY_SIZE(netGroup_),
        .type_ = DumperConstant::NONE,
        .expand_ = false,
    },
    {
        .name_ = ConfigData::CONFIG_GROUP_SERVICE,
        .desc_ = "group of service dumper",
        .list_ = serviceGroup_,
        .size_ = ARRAY_SIZE(serviceGroup_),
        .type_ = DumperConstant::NONE,
        .expand_ = false,
    },
    {
        .name_ = ConfigData::CONFIG_GROUP_ABILITY,
        .desc_ = "group of ability dumper",
        .list_ = systemAbilityGroup_,
        .size_ = ARRAY_SIZE(systemAbilityGroup_),
        .type_ = DumperConstant::NONE,
        .expand_ = false,
    },
    {
        .name_ = ConfigData::CONFIG_GROUP_SYSTEM_BASE,
        .desc_ = "group of base log dumper",
        .list_ = systemBaseGroup_,
        .size_ = ARRAY_SIZE(systemBaseGroup_),
        .type_ = DumperConstant::NONE,
        .expand_ = false,
    },
    {
        .name_ = ConfigData::CONFIG_GROUP_SYSTEM_SYSTEM,
        .desc_ = "group of system log dumper",
        .list_ = systemSystemGroup_,
        .size_ = ARRAY_SIZE(systemSystemGroup_),
        .type_ = DumperConstant::NONE,
        .expand_ = false,
    },
    {
        .name_ = ConfigData::CONFIG_GROUP_PROCESSES,
        .desc_ = "group of processes dumper",
        .list_ = processesGroup_,
        .size_ = ARRAY_SIZE(processesGroup_),
        .type_ = DumperConstant::NONE,
        .expand_ = false,
    },
    {
        .name_ = ConfigData::CONFIG_GROUP_PROCESSES_ENG,
        .desc_ = "group of processes dumper by eng ",
        .list_ = processesGroup_eng_,
        .size_ = ARRAY_SIZE(processesGroup_eng_),
        .type_ = DumperConstant::NONE,
        .expand_ = false,
    },
    {
        .name_ = ConfigData::CONFIG_GROUP_PROCESSES_PID,
        .desc_ = "group of processes pid dumper",
        .list_ = processesPidGroup_,
        .size_ = ARRAY_SIZE(processesPidGroup_),
        .type_ = DumperConstant::NONE,
        .expand_ = false,
    },
    {
        .name_ = ConfigData::CONFIG_GROUP_PROCESSES_PID_ENG,
        .desc_ = "group of processes pid dumper by eng",
        .list_ = processesPidGroup_eng_,
        .size_ = ARRAY_SIZE(processesPidGroup_eng_),
        .type_ = DumperConstant::NONE,
        .expand_ = false,
    },
    {
        .name_ = ConfigData::CONFIG_GROUP_FAULT_LOG,
        .desc_ = "group of fault log dumper",
        .list_ = faultLogGroup_,
        .size_ = ARRAY_SIZE(faultLogGroup_),
        .type_ = DumperConstant::NONE,
        .expand_ = false,
    },
    {
        .name_ = ConfigData::CONFIG_GROUP_STACK,
        .desc_ = "group of stack dumper",
        .list_ = stackGroup_,
        .size_ = ARRAY_SIZE(stackGroup_),
        .type_ = DumperConstant::GROUPTYPE_PID,
        .expand_ = true,
    },
    {
        .name_ = ConfigData::CONFIG_GROUP_TEST,
        .desc_ = "group of test dumper",
        .list_ = testGroup_,
        .size_ = ARRAY_SIZE(testGroup_),
        .type_ = DumperConstant::NONE,
        .expand_ = false,
    },
    {
        .name_ = "groupmini_expand_processesGroup",
        .desc_ = "mini-group for of processes dumper",
        .list_ = processesGroupMini_,
        .size_ = ARRAY_SIZE(processesGroupMini_),
        .type_ = DumperConstant::GROUPTYPE_PID,
        .expand_ = true,
    },
    {
        .name_ = "groupmini_expand_processesGroup_eng",
        .desc_ = "mini-group for of processes dumper by eng",
        .list_ = processesGroupMini_eng_,
        .size_ = ARRAY_SIZE(processesGroupMini_eng_),
        .type_ = DumperConstant::GROUPTYPE_PID,
        .expand_ = true,
    },
    {
        .name_ = "groupmini_pid_expand_processesGroup",
        .desc_ = "mini-group for of processes dumper",
        .list_ = processesPidGroupMini_,
        .size_ = ARRAY_SIZE(processesPidGroupMini_),
        .type_ = DumperConstant::GROUPTYPE_PID,
        .expand_ = true,
    },
    {
        .name_ = "groupmini_pid_expand_processesGroup_eng",
        .desc_ = "mini-group for of processes dumper by eng",
        .list_ = processesPidGroupMini_eng_,
        .size_ = ARRAY_SIZE(processesPidGroupMini_eng_),
        .type_ = DumperConstant::GROUPTYPE_PID,
        .expand_ = true,
    },
    {
        .name_ = "groupmini_cpuid_expand_systemBaseGroup",
        .desc_ = "mini-group for of base dumper",
        .list_ = systemBaseCpuIdGroupMini_,
        .size_ = ARRAY_SIZE(systemBaseCpuIdGroupMini_),
        .type_ = DumperConstant::GROUPTYPE_CPUID,
        .expand_ = true,
    },
    {
        .name_ = "groupmini_cpuid_expand_systemSystemGroup",
        .desc_ = "mini-group for of system dumper",
        .list_ = systemSystemCpuIdGroupMini_,
        .size_ = ARRAY_SIZE(systemSystemCpuIdGroupMini_),
        .type_ = DumperConstant::GROUPTYPE_CPUID,
        .expand_ = true,
    },
    {
        .name_ = "groupmini_pid_nonexpand_systemSystemGroup",
        .desc_ = "group of cpu usage dumper",
        .list_ = systemSystemPidGroupMini_,
        .size_ = ARRAY_SIZE(systemSystemPidGroupMini_),
        .type_ = DumperConstant::GROUPTYPE_PID,
        .expand_ = false,
    },
};

const int ConfigData::dumperSum_ = ARRAY_SIZE(dumpers_);
const int ConfigData::groupSum_ = ARRAY_SIZE(groups_);
const int ConfigData::NEST_MAX = 10;

ConfigData::ConfigData()
{
}

ConfigData::~ConfigData()
{
}
} // namespace HiviewDFX
} // namespace OHOS
