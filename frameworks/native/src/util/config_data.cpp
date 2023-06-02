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
        "dumper_base_info", "Base Information", "", "",
        DumperConstant::API_DUMPER, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
    },
};

const ConfigData::ItemCfg ConfigData::kernelVersionDumper_[] = {
    {
        "dumper_kernel_version", "Kernel Version", "/proc/version", "",
        DumperConstant::FILE_DUMPER, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
    },
};

const ConfigData::ItemCfg ConfigData::cmdlineDumper_[] = {
    {
        "dumper_command_line", "Command Line", "/proc/cmdline", "",
        DumperConstant::FILE_DUMPER, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
    },
};

const ConfigData::ItemCfg ConfigData::kernelWakeSourcesDumper_[] = {
    {
        "dumper_kernel_wake_sources", "KERNEL WAKE SOURCES", "/sys/kernel/debug/wakeup_sources", "",
        DumperConstant::FILE_DUMPER, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
    },
};

const ConfigData::ItemCfg ConfigData::kernelCpufreqDumper_[] = {
    {
        "dumper_kernel_cpu_freq", "KERNEL CPUFREQ", "cat /sys/devices/system/cpu/cpu%cpuid/cpufreq/cpuinfo_cur_freq",
        "", DumperConstant::CMD_DUMPER, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "cat /sys/devices/system/cpu/cpu%cpuid/cpufreq/cpuinfo_max_freq", "",
        DumperConstant::CMD_DUMPER, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
    },
};

const ConfigData::ItemCfg ConfigData::uptimeDumper_[] = {
    {
        "dumper_uptime", "Up Time", "uptime -p", "",
        DumperConstant::CMD_DUMPER, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "", "",
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
    },
};

const ConfigData::ItemCfg ConfigData::cpuUsageDumper_[] = {
    {
        "dumper_cpu_usage", "CPU Usage", "%pid", "",
        DumperConstant::CPU_DUMPER, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
    },
};

const ConfigData::ItemCfg ConfigData::cpuFreqDumper_[] = {
    {
        "dumper_cpu_freq", "CPU Frequency", "cat /sys/devices/system/cpu/cpu%cpuid/cpufreq/cpuinfo_cur_freq", "",
        DumperConstant::CMD_DUMPER, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "cat /sys/devices/system/cpu/cpu%cpuid/cpufreq/cpuinfo_max_freq", "",
        DumperConstant::CMD_DUMPER, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
    },
};

const ConfigData::ItemCfg ConfigData::memDumper_[] = {
    {
        "dumper_mem", "Memory Information", "%pid", "",
        DumperConstant::MEMORY_DUMPER, DumperConstant::NONE, DumperConstant::LOOP, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::LOOP, ""
    },
};

const ConfigData::ItemCfg ConfigData::envDumper_[] = {
    {
        "dumper_env", "Environment Variable", "printenv", "",
        DumperConstant::CMD_DUMPER, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
    },
};

const ConfigData::ItemCfg ConfigData::kernelModuleDumper_[] = {
    {
        "dumper_kernel_module", "Kernel Module", "/proc/modules", "",
        DumperConstant::FILE_DUMPER, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "lsmod", "", DumperConstant::CMD_DUMPER, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
    },
};

const ConfigData::ItemCfg ConfigData::dumpFormatVersionDumper_[] = {
    {
        "dumper_dump_format_version", "HiDumper Version", "HiDumper version:x.x", "",
        DumperConstant::VERSION_DUMPER, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
    },
};

const ConfigData::ItemCfg ConfigData::slabinfoDumper_[] = {
    {
        "dumper_slabinfo", "SLAB INFO", "/proc/slabinfo", "",
        DumperConstant::FILE_DUMPER, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
    },
};

const ConfigData::ItemCfg ConfigData::zoneinfoDumper_[] = {
    {
        "dumper_zoneinfo", "ZONE INFO", "/proc/zoneinfo", "",
        DumperConstant::FILE_DUMPER, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
    },
};

const ConfigData::ItemCfg ConfigData::vmStatDumper_[] = {
    {
        "dumper_vmstat", "VIRTUAL MEMORY STATS", "/proc/vmstat", "",
        DumperConstant::FILE_DUMPER, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
    },
};

const ConfigData::ItemCfg ConfigData::vmAllocInfoDumper_[] = {
    {
        "dumper_vmallocinfo", "VIRTUAL MEMORY STATS", "/proc/vmallocinfo", "",
        DumperConstant::FILE_DUMPER, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
    },
};

const ConfigData::ItemCfg ConfigData::crashDumper_[] = {
    {
        "dumper_crash", "Crash Log", "/data/log/faultlog/faultlogger", "",
        DumperConstant::FILE_DUMPER, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "", "",
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
    },
};

const ConfigData::ItemCfg ConfigData::kernelLogDumper_[] = {
    {
        "dumper_kernel_log", "Kernel Log", "dmesg", "",
        DumperConstant::CMD_DUMPER, DumperConstant::NONE, DumperConstant::LOOP, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::LOOP, ""
    },
};

const ConfigData::ItemCfg ConfigData::hilogDumper_[] = {
    {
        "dumper_hilog", "Hilog", "hilog -x --exit", "",
        DumperConstant::CMD_DUMPER, DumperConstant::NONE, DumperConstant::LOOP, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::LOOP, ""
    },
};

const ConfigData::ItemCfg ConfigData::portDumper_[] = {
    {
        "dumper_port", "Port Information", "netstat -nW", "",
        DumperConstant::CMD_DUMPER, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
    },
};

const ConfigData::ItemCfg ConfigData::packetDumper_[] = {
    {
        "dumper_packet", "Packet State", "/proc/net/dev", "",
        DumperConstant::FILE_DUMPER, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "/proc/net/xt_qtaguid/iface_stat_all", "",
        DumperConstant::FILE_DUMPER, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "/proc/net/xt_qtaguid/iface_stat_fmt", "",
        DumperConstant::FILE_DUMPER, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "/proc/net/xt_qtaguid/ctrl", "",
        DumperConstant::FILE_DUMPER, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "/proc/net/xt_qtaguid/stats", "",
        DumperConstant::FILE_DUMPER, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
    },
};

const ConfigData::ItemCfg ConfigData::ipDumper_[] = {
    {
        "dumper_ip", "IP v4/6 State", "ifconfig -a", "",
        DumperConstant::CMD_DUMPER, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
    },
};

const ConfigData::ItemCfg ConfigData::ipTableDumper_[] = {
    {
        "dumper_ip_table", "IPTable v4/6 Information", "iptables -L -nvx", "",
        DumperConstant::CMD_DUMPER, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "ip6tables -L -nvx", "",
        DumperConstant::CMD_DUMPER, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "iptables -t nat -L -nvx", "",
        DumperConstant::CMD_DUMPER, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "iptables -t mangle -L -nvx", "",
        DumperConstant::CMD_DUMPER, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "", "",
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "ip6tables -t mangle -L -nvx", "",
        DumperConstant::CMD_DUMPER, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "", "",
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "iptables -t raw -L -nvx", "",
        DumperConstant::CMD_DUMPER, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "ip6tables -t raw -L -nvx", "",
        DumperConstant::CMD_DUMPER, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
    },
};

const ConfigData::ItemCfg ConfigData::routeTableDumper_[] = {
    {
        "dumper_route_table", "IP Table v4/6 Information", "/data/misc/net/rt_tables", "",
        DumperConstant::FILE_DUMPER, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
    },
};

const ConfigData::ItemCfg ConfigData::ipcDumper_[] = {
    {
        "dumper_ipc", "IPC Information", "/sys/kernel/debug/binder/failed_transaction_log", "",
        DumperConstant::FILE_DUMPER, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "/sys/kernel/debug/binder/transaction_log", "",
        DumperConstant::FILE_DUMPER, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "/sys/kernel/debug/binder/transactions", "",
        DumperConstant::FILE_DUMPER, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "/sys/kernel/debug/binder/stats", "",
        DumperConstant::FILE_DUMPER, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "/sys/kernel/debug/binder/state", "",
        DumperConstant::FILE_DUMPER, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
    },
};

const ConfigData::ItemCfg ConfigData::ipRulesDumper_[] = {
    {
        "dumper_ip_rules", "IP RULES v4/6", "ip link", "",
        DumperConstant::CMD_DUMPER, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "ip -4 addr show", "",
        DumperConstant::CMD_DUMPER, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "ip -6 addr show", "",
        DumperConstant::CMD_DUMPER, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "ip rule show", "",
        DumperConstant::CMD_DUMPER, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "ip -6 rule show", "",
        DumperConstant::CMD_DUMPER, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
    },
};

const ConfigData::ItemCfg ConfigData::storageStateDumper_[] = {
    {
        "dumper_storage_state", "Storage State", "storaged -u -p", "",
        DumperConstant::CMD_DUMPER, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "", "",
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
    },
};

const ConfigData::ItemCfg ConfigData::blockDumper_[] = {
    {
        "dumper_block", "Block Information", "df -k", "",
        DumperConstant::CMD_DUMPER, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
    },
};

const ConfigData::ItemCfg ConfigData::fileDumper_[] = {
    {
        "dumper_file", "File Information", "lsof", "",
        DumperConstant::CMD_DUMPER, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
    },
};

const ConfigData::ItemCfg ConfigData::topIoDumper_[] = {
    {
        "dumper_top_io", "TOP IO Information", "iotop -n 1 -m 100", "",
        DumperConstant::CMD_DUMPER, DumperConstant::NONE, DumperConstant::NONE, "",
    },
    {
        "", "", "", "",
        DumperConstant::FILE_FORMAT_DUMP_FILTER, DumperConstant::LEVEL_ALL, DumperConstant::NONE, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
    },
};

const ConfigData::ItemCfg ConfigData::mountsDumper_[] = {
    {
        "dumper_mounts", "Mount List", "/proc/mounts", "",
        DumperConstant::FILE_DUMPER, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
    },
};

const ConfigData::ItemCfg ConfigData::threadsDumper_[] = {
    {
        "dumper_threads", "Processes/Threads List", "ps -efT", "",
        DumperConstant::CMD_DUMPER, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
    },
};

const ConfigData::ItemCfg ConfigData::threadsPidDumper_[] = {
    {
        "dumper_threads_pid", "Processes/Threads List", "ps -efT -p %pid", "",
        DumperConstant::CMD_DUMPER, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
    },
};

const ConfigData::ItemCfg ConfigData::smapDumper_[] = {
    {
        "dumper_smap", "Process SMAP Information", "/proc/%pid/smaps", "",
        DumperConstant::FILE_DUMPER, DumperConstant::NONE, DumperConstant::LOOP, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::LOOP, ""
    },
};

const ConfigData::ItemCfg ConfigData::mapDumper_[] = {
    {
        "dumper_map", "Process MAP Information", "/proc/%pid/maps", "",
        DumperConstant::FILE_DUMPER, DumperConstant::NONE, DumperConstant::LOOP, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::LOOP, ""
    },
};

const ConfigData::ItemCfg ConfigData::blockChannelDumper_[] = {
    {
        "dumper_block_channel", "Block Channel", "/proc/%pid/wchan", "",
        DumperConstant::FILE_DUMPER, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
    },
};

const ConfigData::ItemCfg ConfigData::excuteTimeDumper_[] = {
    {
        "dumper_excute_time", "Excute Time", "ps -o \"TIME\" -p %pid", "",
        DumperConstant::CMD_DUMPER, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
    },
};

const ConfigData::ItemCfg ConfigData::mountInfoDumper_[] = {
    {
        "dumper_mount_info", "Mount Information", "/proc/%pid/mountinfo", "",
        DumperConstant::FILE_DUMPER, DumperConstant::NONE, DumperConstant::LOOP, ""
    },
    {
        "", "", "", "",
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::LOOP, ""
    },
};

const ConfigData::ItemCfg ConfigData::systemAbilityDumper_[] = {
    {
        "dumper_system_ability", "System Ability Information", ConfigData::STR_ABILITY, "",
        DumperConstant::SA_DUMPER, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
    },
};

const ConfigData::ItemCfg ConfigData::stackDumper_[] = {
    {
        "dumper_stack", "Dump Stack Info", "dumpcatcher -p %pid", "",
        DumperConstant::CMD_DUMPER, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
    },
};

const ConfigData::ItemCfg ConfigData::listServiceDumper_[] = {
    {
        ConfigData::CONFIG_DUMPER_LIST_SERVICE, "list service", ConfigData::STR_SERVICE, "",
        DumperConstant::LIST_DUMPER, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
    },
};

const ConfigData::ItemCfg ConfigData::listSystemAbilityDumper_[] = {
    {
        ConfigData::CONFIG_DUMPER_LIST_SYSTEM_ABILITY, "list system ability", ConfigData::STR_ABILITY, "",
        DumperConstant::LIST_DUMPER, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
    },
};

const ConfigData::ItemCfg ConfigData::listSystemDumper_[] = {
    {
        ConfigData::CONFIG_DUMPER_LIST_SYSTEM, "list system", ConfigData::STR_SYSTEM, "",
        DumperConstant::LIST_DUMPER, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
    },
};

const ConfigData::ItemCfg ConfigData::testDumper_[] = {
    {
        "dumper_test", "test used dumper", "/data/local/tmp/hidumper_mockdata_0001.txt", "",
        DumperConstant::FILE_DUMPER, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "cat /data/local/tmp/hidumper_mockdata_0002.txt", "",
        DumperConstant::CMD_DUMPER, DumperConstant::NONE, DumperConstant::NONE, ""
    },
    {
        "", "", "", "", 
        DumperConstant::FD_OUTPUT, DumperConstant::NONE, DumperConstant::NONE, ""
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
