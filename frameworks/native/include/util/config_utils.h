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
#ifndef HIDUMPER_SERVICES_CONFIG_UTILS_H
#define HIDUMPER_SERVICES_CONFIG_UTILS_H
#include <string>
#include <vector>
#include "dump_common_utils.h"
#include "common/dump_cfg.h"
#include "common/dumper_parameter.h"
#include "util/config_data.h"
namespace OHOS {
namespace HiviewDFX {
class ConfigUtils : public ConfigData {
public:
    explicit ConfigUtils(const std::shared_ptr<DumperParameter> &param);
    ~ConfigUtils();
public:
    // Used for get dumper configs
    static DumpStatus GetDumperConfigs(const std::shared_ptr<DumperParameter> &param);
    // Used for get section names
    static DumpStatus GetSectionNames(const std::string &name, std::vector<std::string> &nameList);
    // Used for get dump level
    static int GetDumpLevelByPid(int uid, const DumpCommonUtils::PidInfo &pidInfo);
#ifdef DUMP_TEST_MODE // for mock test
public:
#else // for mock test
private:
#endif // for mock test
    DumpStatus GetDumperConfigs();
    bool MergePidInfos(std::vector<DumpCommonUtils::PidInfo> &outInfos, int pid);
    bool HandleDumpLog(std::vector<std::shared_ptr<DumpCfg>> &dumpCfgs);
    bool HandleDumpList(std::vector<std::shared_ptr<DumpCfg>> &dumpCfgs);
    bool HandleDumpService(std::vector<std::shared_ptr<DumpCfg>> &dumpCfgs);
    bool HandleDumpAbility(std::vector<std::shared_ptr<DumpCfg>> &dumpCfgs);
    bool HandleDumpSystem(std::vector<std::shared_ptr<DumpCfg>> &dumpCfgs);
    bool HandleDumpCpuFreq(std::vector<std::shared_ptr<DumpCfg>> &dumpCfgs);
    bool HandleDumpCpuUsage(std::vector<std::shared_ptr<DumpCfg>> &dumpCfgs);
    bool HandleDumpMem(std::vector<std::shared_ptr<DumpCfg>> &dumpCfgs);
    bool HandleDumpStorage(std::vector<std::shared_ptr<DumpCfg>> &dumpCfgs);
    bool HandleDumpNet(std::vector<std::shared_ptr<DumpCfg>> &dumpCfgs);
    bool HandleDumpProcesses(std::vector<std::shared_ptr<DumpCfg>> &dumpCfgs);
    bool HandleDumpFaultLog(std::vector<std::shared_ptr<DumpCfg>> &dumpCfgs);
    bool HandleDumpAppendix(std::vector<std::shared_ptr<DumpCfg>> &dumpCfgs);
    bool HandleDumpTest(std::vector<std::shared_ptr<DumpCfg>> &dumpCfgs);
    bool CopySmaps();
    DumpStatus GetConfig(const std::string &name, std::vector<std::shared_ptr<DumpCfg>> &result,
        std::shared_ptr<OptionArgs> args);
    DumpStatus GetDumper(int index, std::vector<std::shared_ptr<DumpCfg>> &result,
        std::shared_ptr<OptionArgs> args, int level = DumperConstant::NONE);
    DumpStatus GetDumper(const std::string &name, std::vector<std::shared_ptr<DumpCfg>> &result,
        std::shared_ptr<OptionArgs> args, int level = DumperConstant::NONE);
    DumpStatus GetGroup(int index, std::vector<std::shared_ptr<DumpCfg>> &result,
        std::shared_ptr<OptionArgs> args, int level = DumperConstant::NONE, int nest = 0);
    DumpStatus GetGroup(const std::string &name, std::vector<std::shared_ptr<DumpCfg>> &result,
        std::shared_ptr<OptionArgs> args, int level = DumperConstant::NONE, int nest = 0);
    // Used for get section name from group name
    static std::string GetSectionName(const std::string &name);
private:
    DumpStatus GetGroupSimple(const GroupCfg& groupCfg, std::vector<std::shared_ptr<DumpCfg>> &result,
        std::shared_ptr<OptionArgs> args, int level = DumperConstant::NONE, int nest = 0);
    static DumpStatus GetGroupNames(const std::string &name, std::vector<std::string> &nameList);
    static void ConvertTreeToList(std::vector<std::shared_ptr<DumpCfg>> &tree,
        std::vector<std::shared_ptr<DumpCfg>> &list, int nest = 0);
    static void SetSection(std::vector<std::shared_ptr<DumpCfg>> &dumpCfgs, const std::string &section, int nest = 0);
private:
    const std::shared_ptr<DumperParameter> dumperParam_;
    std::vector<DumpCommonUtils::PidInfo> pidInfos_;
    std::vector<DumpCommonUtils::CpuInfo> cpuInfos_;
    std::vector<DumpCommonUtils::PidInfo> currentPidInfos_;
    DumpCommonUtils::PidInfo currentPidInfo_;
    bool isDumpSystemSystem {false};
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIDUMPER_SERVICES_CONFIG_UTILS_H
