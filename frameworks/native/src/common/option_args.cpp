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
#include "common/option_args.h"
#include "hilog_wrapper.h"
namespace OHOS {
namespace HiviewDFX {
OptionArgs::OptionArgs()
{
}

OptionArgs::~OptionArgs()
{
    list_.clear();
    names_.clear();
    args_.clear();
}

OptionArgs& OptionArgs::operator=(const OptionArgs& optArgs)
{
    type_ = optArgs.type_;

    hasPid_ = optArgs.hasPid_;
    pid_ = optArgs.pid_;
    uid_ = optArgs.uid_;

    hasCpuId_ = optArgs.hasCpuId_;
    cpuId_ = optArgs.cpuId_;

    hasStr_ = optArgs.hasStr_;
    str_ = optArgs.str_;

    hasList_ = optArgs.hasList_;
    list_.assign(optArgs.list_.begin(), optArgs.list_.end());

    hasNamesAndArgs_ = optArgs.hasNamesAndArgs_;
    names_.assign(optArgs.names_.begin(), optArgs.names_.end());
    args_.assign(optArgs.args_.begin(), optArgs.args_.end());

    return *this;
}

bool OptionArgs::HasPid() const
{
    return hasPid_;
}

void OptionArgs::SetPid(int pid, int uid)
{
    hasPid_ = true;
    pid_ = pid;
    uid_ = uid;
}

int OptionArgs::GetPid() const
{
    return pid_;
}

int OptionArgs::GetUid() const
{
    return uid_;
}

bool OptionArgs::HasCpuId() const
{
    return hasCpuId_;
}

void OptionArgs::SetCpuId(int cpuid)
{
    hasCpuId_ = true;
    cpuId_ = cpuid;
}

int OptionArgs::GetCpuId() const
{
    return cpuId_;
}

bool OptionArgs::HasStr() const
{
    return hasStr_;
}

void OptionArgs::SetStr(const std::string& str)
{
    hasStr_ = true;
    str_ = str;
}

const std::string& OptionArgs::GetStr() const
{
    return str_;
}

bool OptionArgs::HasStrList() const
{
    return hasList_;
}

void OptionArgs::SetStrList(const std::vector<std::string>& list)
{
    hasList_ = true;
    list_.assign(list.begin(), list.end());
}

const std::vector<std::string>& OptionArgs::GetStrList() const
{
    return list_;
}

bool OptionArgs::HasNamesAndArgs() const
{
    return hasNamesAndArgs_;
}

void OptionArgs::SetNamesAndArgs(const std::vector<std::string>& names, const std::vector<std::string>& args)
{
    hasNamesAndArgs_ = true;
    names_.assign(names.begin(), names.end());
    args_.assign(args.begin(), args.end());
}

const std::vector<std::string>& OptionArgs::GetNameList() const
{
    return names_;
}

const std::vector<std::string>& OptionArgs::GetArgList() const
{
    return args_;
}

void OptionArgs::Dump() const
{
    if (HasPid()) {
        DUMPER_HILOGD(MODULE_COMMON, "debug|    pid=%{public}d, uid=%{public}d", pid_, uid_);
    }
    if (HasCpuId()) {
        DUMPER_HILOGD(MODULE_COMMON, "debug|    cpuId=%{public}d", cpuId_);
    }
    if (HasStr()) {
        DUMPER_HILOGD(MODULE_COMMON, "debug|    str=%{public}s", str_.c_str());
    }
    if (HasStrList()) {
        for (size_t i = 0; i < list_.size(); i++) {
            DUMPER_HILOGD(MODULE_COMMON, "debug|    list[%{public}zu]=%{public}s", i, list_[i].c_str());
        }
    }
    if (HasNamesAndArgs()) {
        for (size_t i = 0; i < names_.size(); i++) {
            DUMPER_HILOGD(MODULE_COMMON, "debug|    names[%{public}zu]=%{public}s", i, names_[i].c_str());
        }
        for (size_t i = 0; i < args_.size(); i++) {
            DUMPER_HILOGD(MODULE_COMMON, "debug|    args[%{public}zu]=%{public}s", i, args_[i].c_str());
        }
    }
}

std::shared_ptr<OptionArgs> OptionArgs::Clone(std::shared_ptr<OptionArgs>& optArgs)
{
    auto ret = Create();
    if (optArgs != nullptr) {
        ret->type_ = optArgs->type_;
        ret->hasPid_ = optArgs->hasPid_;
        ret->pid_ = optArgs->pid_;
        ret->uid_ = optArgs->uid_;
        ret->hasCpuId_ = optArgs->hasCpuId_;
        ret->cpuId_ = optArgs->cpuId_;
        ret->hasStr_ = optArgs->hasStr_;
        ret->str_ = optArgs->str_;
        ret->hasList_ = optArgs->hasList_;
        ret->list_.assign(optArgs->list_.begin(), optArgs->list_.end());
        ret->hasNamesAndArgs_ = optArgs->hasNamesAndArgs_;
        ret->names_.assign(optArgs->names_.begin(), optArgs->names_.end());
        ret->args_.assign(optArgs->args_.begin(), optArgs->args_.end());
    }
    return ret;
}

std::shared_ptr<OptionArgs> OptionArgs::Create()
{
    return std::make_shared<OptionArgs>();
}
} // namespace HiviewDFX
} // namespace OHOS
