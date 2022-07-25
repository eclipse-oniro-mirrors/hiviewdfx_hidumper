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
#ifndef DUMP_IMPLEMENT_H
#define DUMP_IMPLEMENT_H
#include <map>
#include <memory>
#include <getopt.h>
#include <mutex>
#include "if_system_ability_manager.h"
#include "common.h"
#include "singleton.h"
#include "raw_param.h"
#include "common/dumper_parameter.h"
#include "common/dump_cfg.h"
#include "common/dumper_constant.h"
#include "executor/hidumper_executor.h"
#include "factory/executor_factory.h"

namespace OHOS {
namespace HiviewDFX {
class DumpImplement : public Singleton<DumpImplement> {
public:
    DumpImplement();
    ~DumpImplement();
    DumpImplement(DumpImplement const &) = delete;
    void operator=(DumpImplement const &) = delete;
    DumpStatus Main(int argc, char *argv[], const std::shared_ptr<RawParam>& reqCtl);
    const sptr<ISystemAbilityManager> GetSystemAbilityManager();

private:
    DumpStatus CmdParse(int argc, char* argv[], std::shared_ptr<DumperParameter>& dumpParameter);
    /**
     * Check client is hidumper.
     *
     * @param pid, process id of client.
     * @return bool, hidumper client return true, not return false.
     */
    bool IsHidumperClientProcess(int pid);
    DumpStatus CmdParseWithParameter(int argc, char* argv[], DumperOpts& opts_);
    DumpStatus CmdParseWithParameter(std::shared_ptr<DumperParameter>& dumpParameter,
        int argc, char* argv[], DumperOpts& opts_);
    DumpStatus SetCmdParameter(int argc, char* argv[], DumperOpts& opts_);
    DumpStatus SetCmdIntegerParameter(const std::string& str, int& value);
    void CmdHelp();
    void setExecutorList(std::vector<std::shared_ptr<HidumperExecutor>>& executors,
        const std::vector<std::shared_ptr<DumpCfg>>& configs, bool isZip);
    DumpStatus DumpDatas(const std::vector<std::shared_ptr<HidumperExecutor>>& executors,
        const std::shared_ptr<DumperParameter>& dumpParameter,
        HidumperExecutor::StringMatrix dumpDatas);
    void AddGroupTitle(const std::string& groupName, HidumperExecutor::StringMatrix dumpDatas);
    void AddExecutorFactoryToMap();
    /**
     * Check group name changed.
     *
     * @param lastName, last group name.
     * @param curName, current group name.
     * @return bool, changed true, not changed false.
     */
    bool CheckGroupName(std::string& lastName, const std::string& curName);
    bool IsShortOptionReqArg(const char* optStr);
    void SendErrorMessage(const std::string& errorStr);
    void SendPidErrorMessage(int pid);
    DumpStatus ParseLongCmdOption(DumperOpts& opts_, const struct option longOptions[],
        const int& optionIndex, char* argv[]);
    DumpStatus ParseShortCmdOption(int c, DumperOpts& opts_, int argc, char* argv[]);
    void CheckIncorrectCmdOption(const char* optStr, char* argv[]);
    std::string RemoveCharacterFromStr(const std::string& str, const char character);
    bool IsLongOption(const std::string& str, const struct option longOptions[], int size);
    bool IsSADumperOption(char* argv[]);
    DumpStatus CheckProcessAlive(const DumperOpts& opts_);
    void RemoveDuplicateString(DumperOpts& opts_);

private:
    using ExecutorFactoryMap = std::map<int, std::shared_ptr<ExecutorFactory>>;
    static const int ARG_INDEX_OFFSET_LAST_OPTION = 2;
    const std::string unrecognizedError_ = "hidumper: unrecognized option: ";
    const std::string invalidError_ = "hidumper: invalid arg: ";
    const std::string requireError_ = "hidumper: option requires an argument: ";
    const std::string pidError_ = "hidumper: No such process: %d\n";
    std::shared_ptr<ExecutorFactoryMap> ptrExecutorFactoryMap_;
    mutable std::mutex mutexCmdLock_;
    std::shared_ptr<RawParam> ptrReqCtl_;
    sptr<ISystemAbilityManager> sam_;
    std::string GetTime();
    std::string path_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // DUMP_IMPLEMENT_H
