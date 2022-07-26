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
#ifndef HIDUMPER_SERVICES_OPTION_ARGS_H
#define HIDUMPER_SERVICES_OPTION_ARGS_H
#include <string>
#include <vector>
#include "common/dumper_constant.h"
namespace OHOS {
namespace HiviewDFX {
class OptionArgs {
public:
    OptionArgs();
    virtual ~OptionArgs();
    OptionArgs& operator=(const OptionArgs& optArgs);

    bool HasPid() const;
    void SetPid(int pid, int uid);
    int GetPid() const;
    int GetUid() const;

    bool HasCpuId() const;
    void SetCpuId(int cpuid);
    int GetCpuId() const;

    bool HasStr() const;
    void SetStr(const std::string& str);
    const std::string& GetStr() const;

    bool HasStrList() const;
    void SetStrList(const std::vector<std::string>& list);
    const std::vector<std::string>& GetStrList() const;

    bool HasNamesAndArgs() const;
    void SetNamesAndArgs(const std::vector<std::string>& names, const std::vector<std::string>& args);
    const std::vector<std::string>& GetNameList() const;
    const std::vector<std::string>& GetArgList() const;

    void Dump() const;
    static std::shared_ptr<OptionArgs> Create();
    static std::shared_ptr<OptionArgs> Clone(std::shared_ptr<OptionArgs>& args);
private:
    static const int UNSET = -1;
    int type_ {DumperConstant::NONE};
    bool hasPid_ {false};
    int pid_ {UNSET};
    int uid_ {UNSET};
    bool hasCpuId_ {false};
    int cpuId_ {UNSET};
    bool hasStr_ {false};
    std::string str_;
    bool hasList_ {false};
    std::vector<std::string> list_;
    bool hasNamesAndArgs_ {false};
    std::vector<std::string> names_;
    std::vector<std::string> args_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIDUMPER_SERVICES_OPTION_ARGS_H
