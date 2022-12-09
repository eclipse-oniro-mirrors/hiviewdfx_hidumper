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
#include "executor/cmd_dumper.h"

#include "dump_common_utils.h"
#include "securec.h"

namespace OHOS {
namespace HiviewDFX {
const std::string CMD_PREFIX = "/system/bin/";
CMDDumper::CMDDumper() : fp_(nullptr)
{
}

CMDDumper::~CMDDumper()
{
    if (fp_ != nullptr) {
        pclose(fp_);
        fp_ = nullptr;
    }
}

DumpStatus CMDDumper::PreExecute(const std::shared_ptr<DumperParameter>& parameter,
    StringMatrix dumpDatas)
{
    if (dumpDatas.get() == nullptr) {
        return DumpStatus::DUMP_FAIL;
    }
    std::string cmd = ptrDumpCfg_->target_;
    if (ptrDumpCfg_->args_ != nullptr) {
        if (ptrDumpCfg_->args_->HasPid()) {
            int pid = ptrDumpCfg_->args_->GetPid();
            if (ptrDumpCfg_->name_ == "dumper_stack" && !DumpCommonUtils::IsUserPid(std::to_string(pid))) {
                return DumpStatus::DUMP_FAIL;
            }
            ReplacePidInCmd(cmd, pid);
        }
        if (ptrDumpCfg_->args_->HasCpuId()) {
            int cpuId = ptrDumpCfg_->args_->GetCpuId();
            ReplaceCpuIdInCmd(cmd, cpuId);
        }
    }
    cmd_ = cmd;
    needLoop_ = (ptrDumpCfg_->loop_ == DumperConstant::LOOP);
    if (fp_ == nullptr) {
        if ((fp_ = popen((CMD_PREFIX + cmd_).c_str(), "r")) == nullptr) {
            return DumpStatus::DUMP_FAIL;
        }
    }
    dumpDatas_ = dumpDatas;

    // Add only once!
    if (lineData_.size() <= 0) {
        std::string lineStr = "\ncmd is: ";
        lineStr = lineStr + cmd_ + "\n\n";
        lineData_.push_back(lineStr);
        dumpDatas_->push_back(lineData_);
    }
    return DumpStatus::DUMP_OK;
}

DumpStatus CMDDumper::Execute()
{
    DumpStatus ret = DumpStatus::DUMP_OK;
    if (needLoop_) {
        // dump one line
        return ReadLine();
    } else {
        // dump all line
        do {
            if (IsCanceled()) {
                break;
            }
            ret = ReadLine();
        } while (ret == DumpStatus::DUMP_MORE_DATA);
    }
    return ret;
}

DumpStatus CMDDumper::AfterExecute()
{
    if (moreData_ && IsTimeout()) {
        DUMPER_HILOGE(MODULE_COMMON, "error|cmd timeout");
        std::vector<std::string> line_vector_timeout;
        line_vector_timeout.push_back(GetTimeoutStr());
        dumpDatas_->push_back(line_vector_timeout);
        moreData_ = false;
    }

    if (!moreData_) {
        if (fp_ != nullptr) {
            pclose(fp_);
            fp_ = nullptr;
        }
        return DumpStatus::DUMP_OK;
    }
    return DumpStatus::DUMP_MORE_DATA;
}

// provide interface to others
DumpStatus CMDDumper::GetCmdInterface(const std::string& cmd, StringMatrix dumpDatas)
{
    DumpStatus ret = DumpStatus::DUMP_MORE_DATA;
    FILE* fp = popen((CMD_PREFIX + cmd).c_str(), "r");
    if (fp == nullptr) {
        return DumpStatus::DUMP_FAIL;
    }
    do {
        ret = GetLineData(fp, dumpDatas);
    } while (ret == DumpStatus::DUMP_MORE_DATA);
    pclose(fp);
    return ret;
}

DumpStatus CMDDumper::GetLineData(FILE* fp, StringMatrix dumpDatas)
{
    DumpStatus ret = DumpStatus::DUMP_MORE_DATA;
    char* line_buffer = nullptr;
    size_t len = 0;
    ssize_t read = 0;
    read = getline(&line_buffer, &len, fp);
    if (read != -1) {
        if (line_buffer[read - 1] == '\n') {
            line_buffer[read - 1] = '\0'; // replease \n
        }
        std::string line = line_buffer;
        std::vector<std::string> line_vector;
        line_vector.push_back(line);
        dumpDatas->push_back(line_vector);
    } else {
        if (feof(fp) == 0) { // ferror()
            ret = DumpStatus::DUMP_FAIL;
        } else {
            ret = DumpStatus::DUMP_OK;
        }
    }
    if (line_buffer != nullptr) {
        free(line_buffer);
        line_buffer = nullptr;
    }
    return ret;
}

// read one line
DumpStatus CMDDumper::ReadLine()
{
    if (fp_ == nullptr) {
        return DumpStatus::DUMP_FAIL;
    }
    DumpStatus ret = GetLineData(fp_, dumpDatas_);
    moreData_ = (ret == DumpStatus::DUMP_MORE_DATA);
    return ret;
}

void CMDDumper::ReplacePidInCmd(std::string &cmd, int pid)
{
    size_t pos = cmd.find("%pid");
    if (pos != std::string::npos) {
        cmd = cmd.replace(pos, strlen("%pid"), std::to_string(pid));
        cmd_ = cmd;
    }
}

void CMDDumper::ReplaceCpuIdInCmd(std::string &cmd, int cpuId)
{
    size_t pos = cmd.find("%cpuid");
    if (pos != std::string::npos) {
        cmd = cmd.replace(pos, strlen("%cpuid"), std::to_string(cpuId));
        cmd_ = cmd;
    }
}
} // namespace HiviewDFX
} // namespace OHOS
