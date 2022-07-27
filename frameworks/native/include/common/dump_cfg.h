/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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
#ifndef HIDUMPER_SERVICES_DUMP_CFG_H
#define HIDUMPER_SERVICES_DUMP_CFG_H
#include <string>
#include <vector>
#include "common/dumper_constant.h"
#include "common/option_args.h"
namespace OHOS {
namespace HiviewDFX {
class HidumperExecutor;
struct DumpCfg {
    std::string name_;
    std::string desc_;
    std::string target_;
    std::string section_;
    int class_ {DumperConstant::NONE};
    int level_ {DumperConstant::NONE};
    int loop_ {DumperConstant::NONE};
    std::string filterCfg_;
    std::shared_ptr<OptionArgs> args_;
    int type_ {DumperConstant::NONE}; // DumperConstant::NONE, GROUPTYPE_PID, GROUPTYPE_CPUID
    bool expand_ {false}; // true: expand; false: non-expand
    std::shared_ptr<DumpCfg> parent_;
    std::vector<std::shared_ptr<DumpCfg>> childs_; // GROUP, must clear
    std::shared_ptr<HidumperExecutor> executor_; // GROUP, must clear
public:
    DumpCfg();
    ~DumpCfg();
    DumpCfg& operator=(const DumpCfg& dumpCfg);
    bool IsGroup() const;
    bool IsDumper() const;
    bool IsFilter() const;
    bool IsLevel() const;
    bool CanLoop() const;
    bool IsOutput() const;
    bool HasPid() const;
    bool HasCpuId() const;
    void Dump(int deep = 0) const;
    static std::shared_ptr<DumpCfg> Create();
    static bool IsDumper(int cls);
    static bool IsFilter(int cls);
    static bool IsLevel(int level);
    static bool CanLoop(int loop);
    static bool IsOutput(int cls);
    static bool HasPid(const std::string &target);
    static bool HasCpuId(const std::string &target);
    static std::string ToStr(int type);
    static void Dump(const DumpCfg& cfg, int nest = 0);
private:
    static std::string ToDumperStr(int type);
    static std::string ToFilterStr(int type);
    static std::string ToOutputStr(int type);
    static std::string ToLevelStr(int type);
    static std::string ToTypeStr(int type);
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIDUMPER_SERVICES_DUMP_CFG_H
