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

#include "executor/fd_output.h"
#include <unistd.h>
#include <fcntl.h>

namespace OHOS {
namespace HiviewDFX {
const mode_t FDOutput::OPEN_ARGV = 0664;

FDOutput::FDOutput() : fd_(-1)
{
}

FDOutput::~FDOutput()
{
    if (fd_ >= 0) {
        close(fd_);
    }
    fd_ = -1;
}

DumpStatus FDOutput::PreExecute(const std::shared_ptr<DumperParameter>& parameter,
    StringMatrix dumpDatas)
{
    if (dumpDatas.get()) {
        dumpDatas_ = dumpDatas;
    } else {
        return DumpStatus::DUMP_FAIL;
    }
    ptrReqCtl_ = parameter->getClientCallback();
    path_ = parameter->GetOutputFilePath();
    if ((fd_ < 0) && (!path_.empty())) {
        fd_ = open(path_.c_str(), O_WRONLY | O_CREAT | O_APPEND, OPEN_ARGV);
        if (fd_ < 0) {
            return DumpStatus::DUMP_FAIL;
        }
    }
    return DumpStatus::DUMP_OK;
}

DumpStatus FDOutput::Execute()
{
    if ((ptrReqCtl_ != nullptr) && (dumpDatas_ != nullptr)) {
        OutMethod();
    }
    return DumpStatus::DUMP_OK;
}

DumpStatus FDOutput::AfterExecute()
{
    if (dumpDatas_ != nullptr) {
        dumpDatas_->clear();
    }
    return DumpStatus::DUMP_OK;
}

void FDOutput::OutMethod()
{
    for (size_t i = 0; i < dumpDatas_->size(); i++) {
        std::vector<std::string> line = dumpDatas_->at(i);
        for (size_t j = 0; j < line.size(); j++) {
            std::string str = line[j];
            if ((i < dumpDatas_->size()) && (j == (line.size() - 1))) {
                NewLineMethod(str);
            }
            int rawParamFd = ptrReqCtl_->GetOutputFd();
            if (rawParamFd > -1) {
                write(rawParamFd, str.c_str(), strlen(str.c_str()));
                fsync(rawParamFd);
            }
            if (fd_ > -1) {
                dprintf(fd_, "%s", str.c_str());
            }
        }
    }
}

void FDOutput::NewLineMethod(std::string &str)
{
    if (str.find("\n") == std::string::npos) { // No line breaks
        str = str + "\n";
    }
}
} // namespace HiviewDFX
} // namespace OHOS
