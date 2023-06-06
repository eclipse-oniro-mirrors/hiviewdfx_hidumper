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
#ifndef HIDUMPER_SERVICES_CONFIG_DATA_H
#define HIDUMPER_SERVICES_CONFIG_DATA_H
#include <string>
namespace OHOS {
namespace HiviewDFX {
class ConfigData {
public:
    ConfigData();
    ~ConfigData();
    static const std::string CONFIG_NAME_SPLIT;
    static const std::string CONFIG_GROUP;
    static const std::string CONFIG_GROUP_;
    static const std::string CONFIG_MINIGROUP;
    static const std::string CONFIG_MINIGROUP_;
    static const std::string CONFIG_DUMPER;
    static const std::string CONFIG_DUMPER_;
    static const std::string CONFIG_NAME_BASE;
    static const std::string CONFIG_NAME_SYSTEM;
    static const std::string CONFIG_NAME_ABILITY;
    static const std::string CONFIG_GROUP_CPU_FREQ;
    static const std::string CONFIG_GROUP_CPU_USAGE;
    static const std::string CONFIG_GROUP_LOG;
    static const std::string CONFIG_GROUP_LOG_;
    static const std::string CONFIG_GROUP_LOG_KERNEL;
    static const std::string CONFIG_GROUP_LOG_HILOG;
    static const std::string CONFIG_GROUP_LOG_INIT;
    static const std::string CONFIG_GROUP_MEMORY;
    static const std::string CONFIG_GROUP_STORAGE;
    static const std::string CONFIG_GROUP_NET;
    static const std::string CONFIG_GROUP_SERVICE;
    static const std::string CONFIG_GROUP_ABILITY;
    static const std::string CONFIG_GROUP_SYSTEM;
    static const std::string CONFIG_GROUP_SYSTEM_;
    static const std::string CONFIG_GROUP_SYSTEM_BASE;
    static const std::string CONFIG_GROUP_SYSTEM_SYSTEM;
    static const std::string CONFIG_GROUP_PROCESSES;
    static const std::string CONFIG_GROUP_PROCESSES_ENG;
    static const std::string CONFIG_GROUP_PROCESSES_PID;
    static const std::string CONFIG_GROUP_PROCESSES_PID_ENG;
    static const std::string CONFIG_GROUP_FAULT_LOG;
    static const std::string CONFIG_GROUP_STACK;
    static const std::string CONFIG_GROUP_TEST;
    static const std::string CONFIG_DUMPER_LIST;
    static const std::string CONFIG_DUMPER_LIST_;
    static const std::string CONFIG_DUMPER_LIST_SERVICE;
    static const std::string CONFIG_DUMPER_LIST_SYSTEM_ABILITY;
    static const std::string CONFIG_DUMPER_LIST_SYSTEM;
    static const std::string STR_ABILITY;
    static const std::string STR_BASE;
    static const std::string STR_SERVICE;
    static const std::string STR_SYSTEM;
protected:
    struct ItemCfg {
        const std::string &name_;
        const std::string &desc_;
        const std::string &target_;
        const std::string &section_;
        const int &class_;
        const int &level_;
        const int &loop_;
        const std::string &filterCfg_;
    };
    struct DumperCfg {
        const std::string &name_;
        const std::string &desc_;
        const ItemCfg * const &list_;
        const int &size_;
    };
    struct GroupCfg {
        const std::string &name_;
        const std::string &desc_;
        const std::string * const &list_;
        const int &size_;
        const int type_; // DumperConstant::NONE, GROUPTYPE_PID, GROUPTYPE_CPUID
        const bool expand_; // true: expand; false: non-expand
    };
    static const GroupCfg groups_[];
    static const int groupSum_;
    static const DumperCfg dumpers_[];
    static const int dumperSum_;
    static const int NEST_MAX;
private:
    static const ItemCfg baseInfoDumper_[];
    static const ItemCfg versionDumper_[];
    static const ItemCfg kernelVersionDumper_[];
    static const ItemCfg cmdlineDumper_[];
    static const ItemCfg kernelWakeSourcesDumper_[];
    static const ItemCfg kernelCpufreqDumper_[];
    static const ItemCfg uptimeDumper_[];
    static const ItemCfg cpuUsageDumper_[];
    static const ItemCfg cpuFreqDumper_[];
    static const ItemCfg memDumper_[];
    static const ItemCfg envDumper_[];
    static const ItemCfg kernelModuleDumper_[];
    static const ItemCfg dumpFormatVersionDumper_[];
    static const ItemCfg slabinfoDumper_[];
    static const ItemCfg zoneinfoDumper_[];
    static const ItemCfg vmStatDumper_[];
    static const ItemCfg vmAllocInfoDumper_[];
    static const ItemCfg crashDumper_[];
    static const ItemCfg tombStoneDumper_[];
    static const ItemCfg kernelLogDumper_[];
    static const ItemCfg hilogDumper_[];
    static const ItemCfg portDumper_[];
    static const ItemCfg packetDumper_[];
    static const ItemCfg ipDumper_[];
    static const ItemCfg ipTableDumper_[];
    static const ItemCfg routeTableDumper_[];
    static const ItemCfg ipcDumper_[];
    static const ItemCfg ipRulesDumper_[];
    static const ItemCfg storageStateDumper_[];
    static const ItemCfg blockDumper_[];
    static const ItemCfg fileDumper_[];
    static const ItemCfg topIoDumper_[];
    static const ItemCfg mountsDumper_[];
    static const ItemCfg threadsDumper_[];
    static const ItemCfg threadsPidDumper_[];
    static const ItemCfg smapDumper_[];
    static const ItemCfg mapDumper_[];
    static const ItemCfg blockChannelDumper_[];
    static const ItemCfg excuteTimeDumper_[];
    static const ItemCfg mountInfoDumper_[];
    static const ItemCfg systemAbilityDumper_[];
    static const ItemCfg stackDumper_[];
    static const ItemCfg listServiceDumper_[];
    static const ItemCfg listSystemAbilityDumper_[];
    static const ItemCfg listSystemDumper_[];
    static const ItemCfg testDumper_[];
    static const std::string cpuFreqGroup_[];
    static const std::string cpuUsageGroup_[];
    static const std::string logKernelGroup_[];
    static const std::string logHilogGroup_[];
    static const std::string logInitGroup_[];
    static const std::string memoryGroup_[];
    static const std::string storageGroup_[];
    static const std::string netGroup_[];
    static const std::string serviceGroup_[];
    static const std::string systemAbilityGroup_[];
    static const std::string systemBaseGroup_[];
    static const std::string systemSystemGroup_[];
    static const std::string processesGroup_[];
    static const std::string processesGroup_eng_[];
    static const std::string processesPidGroup_[];
    static const std::string processesPidGroup_eng_[];
    static const std::string faultLogGroup_[];
    static const std::string stackGroup_[];
    static const std::string testGroup_[];
    static const std::string processesGroupMini_[];
    static const std::string processesGroupMini_eng_[];
    static const std::string processesPidGroupMini_[];
    static const std::string processesPidGroupMini_eng_[];
    static const std::string systemBaseCpuIdGroupMini_[];
    static const std::string systemSystemCpuIdGroupMini_[];
    static const std::string systemSystemPidGroupMini_[];
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIDUMPER_SERVICES_CONFIG_DATA_H
