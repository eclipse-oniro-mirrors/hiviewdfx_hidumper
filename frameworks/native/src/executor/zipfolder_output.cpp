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
#include "executor/zipfolder_output.h"
#include <unistd.h>
#include "dump_utils.h"
#include "file_ex.h"
#include "directory_ex.h"
#include "hilog_wrapper.h"
#include "util/zip_utils.h"
#include "dump_common_utils.h"
namespace OHOS {
namespace HiviewDFX {
namespace {
static const int FD_UNSET = -1;
static const char NEW_LINE_BREAKS_CHAR = '\n';
static const std::string NEW_LINE_BREAKS_STR = "\n";
} // namespace
ZipFolderOutput::ZipFolderOutput() : fd_(FD_UNSET)
{
    DUMPER_HILOGD(MODULE_COMMON, "create|");
}

ZipFolderOutput::~ZipFolderOutput()
{
    DUMPER_HILOGD(MODULE_COMMON, "release|");

    if (fd_ > FD_UNSET) {
        close(fd_);
    }

    fd_ = FD_UNSET;
}

DumpStatus ZipFolderOutput::PreExecute(const std::shared_ptr<DumperParameter>& parameter,
    StringMatrix dumpDatas)
{
    if ((parameter == nullptr) || (dumpDatas == nullptr)) {
        DUMPER_HILOGE(MODULE_COMMON, "PreExecute error|parameter or dumpDatas is nullptr");
        return DumpStatus::DUMP_FAIL;
    }
    // init myself once
    if (logDefaultPath_.empty()) {
        param_ = parameter;
        auto callback = param_->getClientCallback();
        if (callback == nullptr) {
            DUMPER_HILOGE(MODULE_COMMON, "PreExecute error|callback is nullptr");
            return DumpStatus::DUMP_FAIL;
        }
        logDefaultPath_ = callback->GetFolder() + LOG_DEFAULT;
        fd_= DumpUtils::FdToWrite(logDefaultPath_);
    }

    if (fd_ < 0) {
        DUMPER_HILOGE(MODULE_COMMON, "PreExecute error|fd has issue");
        return DumpStatus::DUMP_FAIL;
    }

    dumpDatas_ = dumpDatas;
    return DumpStatus::DUMP_OK;
}

DumpStatus ZipFolderOutput::Execute()
{
    if ((dumpDatas_ == nullptr) || (fd_ < 0)) {
        DUMPER_HILOGE(MODULE_COMMON, "Execute error|dumpDatas or fd has issue");
        return DumpStatus::DUMP_FAIL;
    }

    std::string outstr;

    std::string line;
    for (auto &lineItems : (*dumpDatas_)) {
        for (auto &item : lineItems) {
            line.append(item);
        }
        lineItems.clear();

        if (line.empty() || (line.at(line.length() - 1) != NEW_LINE_BREAKS_CHAR)) { // No line breaks
            line.append(NEW_LINE_BREAKS_STR);
        }

        outstr.append(line);
        line.clear();
    }

    SaveStringToFd(fd_, outstr);

    outstr.clear();

    return DumpStatus::DUMP_OK;
}

DumpStatus ZipFolderOutput::AfterExecute()
{
    if (dumpDatas_ != nullptr) {
        dumpDatas_->clear();
    }

    return DumpStatus::DUMP_OK;
}

void ZipFolderOutput::Reset()
{
    if (fd_ > FD_UNSET) {
        DUMPER_HILOGD(MODULE_COMMON, "Reset debug|close");
        close(fd_);
    }
    fd_ = FD_UNSET;

    const std::shared_ptr<RawParam> callback = (param_ == nullptr) ? nullptr : param_->getClientCallback();
    if ((param_ != nullptr) && (callback != nullptr)) {
        DUMPER_HILOGD(MODULE_COMMON, "Reset debug|ZipFolder");
        auto logZipPath = param_->GetOpts().path_;
        auto logFolder = callback->GetFolder();
        ZipUtils::ZipFolder(logFolder, logZipPath, [callback] (int progress, int subprogress) {
            callback->UpdateProgress(0);
            return callback->IsCanceled();
        });
    }

    param_ = nullptr;

    HidumperExecutor::Reset();
}
} // namespace HiviewDFX
} // namespace OHOS
