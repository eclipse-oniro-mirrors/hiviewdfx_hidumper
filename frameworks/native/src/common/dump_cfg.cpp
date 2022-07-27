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
#include "common/dump_cfg.h"
#include "hilog_wrapper.h"
namespace OHOS {
namespace HiviewDFX {
DumpCfg::DumpCfg()
{
}

DumpCfg::~DumpCfg()
{
    childs_.clear();
    parent_ = nullptr;
}

DumpCfg& DumpCfg::operator=(const DumpCfg& dumpCfg)
{
    name_ = dumpCfg.name_;
    desc_ = dumpCfg.desc_;
    target_ = dumpCfg.target_;
    section_ = dumpCfg.section_;
    class_ = dumpCfg.class_;
    level_ = dumpCfg.level_;
    loop_ = dumpCfg.loop_;
    filterCfg_ = dumpCfg.filterCfg_;
    args_ = dumpCfg.args_;
    return *this;
}

void DumpCfg::Dump(int deep) const
{
    Dump(*this, deep);
}

bool DumpCfg::IsGroup() const
{
    return ((class_ == DumperConstant::GROUP) || (!childs_.empty()));
}

bool DumpCfg::IsDumper() const
{
    return IsDumper(class_);
}

bool DumpCfg::IsFilter() const
{
    return IsFilter(class_);
}

bool DumpCfg::IsLevel() const
{
    return IsLevel(level_);
}

bool DumpCfg::CanLoop() const
{
    return CanLoop(loop_);
}

bool DumpCfg::IsOutput() const
{
    return IsOutput(class_);
}

bool DumpCfg::HasPid() const
{
    return HasPid(target_);
}

bool DumpCfg::HasCpuId() const
{
    return HasCpuId(target_);
}

std::shared_ptr<DumpCfg> DumpCfg::Create()
{
    return std::make_shared<DumpCfg>();
}

bool DumpCfg::IsDumper(int cls)
{
    return ((cls > DumperConstant::DUMPER_BEGIN) && (cls < DumperConstant::DUMPER_END));
}

bool DumpCfg::IsFilter(int cls)
{
    return ((cls > DumperConstant::FILTER_BEGIN) && (cls < DumperConstant::FILTER_END));
}

bool DumpCfg::IsLevel(int level)
{
    return ((level > DumperConstant::LEVEL_BEGIN) && (level < DumperConstant::LEVEL_END));
}

bool DumpCfg::CanLoop(int loop)
{
    return ((loop != DumperConstant::NONE) && (loop > 0));
}

bool DumpCfg::IsOutput(int cls)
{
    return ((cls > DumperConstant::OUTPUT_BEGIN) && (cls < DumperConstant::OUTPUT_END));
}

bool DumpCfg::HasPid(const std::string &target)
{
    return (target.find("%pid") != std::string::npos);
}

bool DumpCfg::HasCpuId(const std::string &target)
{
    return (target.find("%cpuid") != std::string::npos);
}

void DumpCfg::Dump(const DumpCfg& cfg, int nest)
{
    DUMPER_HILOGD(MODULE_COMMON, "debug|*********[%{public}d]********", nest);
    DUMPER_HILOGD(MODULE_COMMON, "debug|section=%{public}s,"
        " class=%{public}d[%{public}s], level=%{public}d[%{public}s],"
        " name=%{public}s, type=%{public}d[%{public}s], expand_=%{public}d, target=%{public}s,"
        " desc=%{public}s,"
        " filterCfg=%{public}s",
        cfg.section_.c_str(),
        cfg.class_, ToStr(cfg.class_).c_str(), cfg.level_, ToStr(cfg.level_).c_str(),
        cfg.name_.c_str(), cfg.type_, ToStr(cfg.type_).c_str(), cfg.expand_, cfg.target_.c_str(),
        cfg.desc_.c_str(),
        cfg.filterCfg_.c_str());
    if (cfg.args_ != nullptr) {
        cfg.args_->Dump();
    }
    if (cfg.parent_ != nullptr) {
        DUMPER_HILOGD(MODULE_COMMON, "debug|parent...name=%{public}s, desc=%{public}s",
            cfg.parent_->name_.c_str(), cfg.parent_->desc_.c_str());
    } else {
        for (auto dumpCfg : cfg.childs_) {
            dumpCfg->Dump(nest+1);
        }
    }
}

std::string DumpCfg::ToStr(int type)
{
    if (type == DumperConstant::NONE) {
        return "none";
    } else if (type == DumperConstant::GROUP) {
        return "group";
    } else if ((type >= DumperConstant::DUMPER_BEGIN) && (type <= DumperConstant::DUMPER_END)) {
        return ToDumperStr(type);
    } else if ((type >= DumperConstant::FILTER_BEGIN) && (type <= DumperConstant::FILTER_END)) {
        return ToFilterStr(type);
    } else if ((type >= DumperConstant::OUTPUT_BEGIN) && (type <= DumperConstant::OUTPUT_END)) {
        return ToOutputStr(type);
    } else if ((type >= DumperConstant::LEVEL_BEGIN) && (type <= DumperConstant::LEVEL_END)) {
        return ToLevelStr(type);
    } else if ((type >= DumperConstant::GROUPTYPE_BEGIN) && (type <= DumperConstant::GROUPTYPE_END)) {
        return ToTypeStr(type);
    } else if (type == DumperConstant::LOOP) {
        return "loop";
    }
    return "unknown_type";
};

std::string DumpCfg::ToDumperStr(int type)
{
    if (type == DumperConstant::CPU_DUMPER) {
        return "cpu_dumper";
    } else if (type == DumperConstant::FILE_DUMPER) {
        return "file_dumper";
    } else if (type == DumperConstant::ENV_PARAM_DUMPER) {
        return "env_dumper";
    } else if (type == DumperConstant::CMD_DUMPER) {
        return "cmd_dumper";
    } else if (type == DumperConstant::PROPERTIES_DUMPER) {
        return "prop_dumper";
    } else if (type == DumperConstant::API_DUMPER) {
        return "api_dumper";
    } else if (type == DumperConstant::LIST_DUMPER) {
        return "list_dumper";
    } else if (type == DumperConstant::VERSION_DUMPER) {
        return "ver_dumper";
    } else if (type == DumperConstant::SA_DUMPER) {
        return "sa_dumper";
    } else if (type == DumperConstant::MEMORY_DUMPER) {
        return "mem_dumper";
    } else if (type == DumperConstant::STACK_DUMPER) {
        return "stack_dumper";
    }
    return "unknown_dumper";
}

std::string DumpCfg::ToFilterStr(int type)
{
    if (type == DumperConstant::COLUMN_ROWS_FILTER) {
        return "col_row_filter";
    } else if (type == DumperConstant::FILE_FORMAT_DUMP_FILTER) {
        return "file_format_dump_filter";
    }
    return "unknown_filter";
}

std::string DumpCfg::ToOutputStr(int type)
{
    if (type == DumperConstant::STD_OUTPUT) {
        return "std_output";
    } else if (type == DumperConstant::FILE_OUTPUT) {
        return "file_output";
    } else if (type == DumperConstant::FD_OUTPUT) {
        return "fd_output";
    } else if (type == DumperConstant::ZIP_OUTPUT) {
        return "zip_output";
    }
    return "unknown_output";
}

std::string DumpCfg::ToLevelStr(int type)
{
    if (type == DumperConstant::LEVEL_NONE) {
        return "level_none";
    } else if (type == DumperConstant::LEVEL_MIDDLE) {
        return "level_middle";
    } else if (type == DumperConstant::LEVEL_HIGH) {
        return "level_high";
    } else if (type == DumperConstant::LEVEL_ALL) {
        return "level_all";
    }
    return "level_unknown";
}

std::string DumpCfg::ToTypeStr(int type)
{
    if (type == DumperConstant::LEVEL_NONE) {
        return "type_none";
    } else if (type == DumperConstant::GROUPTYPE_PID) {
        return "type_pid";
    } else if (type == DumperConstant::GROUPTYPE_CPUID) {
        return "type_cpuid";
    }
    return "type_unknown";
}
} // namespace HiviewDFX
} // namespace OHOS
