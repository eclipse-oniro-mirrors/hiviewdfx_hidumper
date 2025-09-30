/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "executor/event_detail_dumper.h"
#include "dump_utils.h"

using namespace std;
namespace OHOS {
namespace HiviewDFX {
EventDetailDumper::EventDetailDumper() : startTime_(0), endTime_(0), showEventCount_(-1), fp_(nullptr), fd_(-1)
{
}

EventDetailDumper::~EventDetailDumper()
{
    CloseFd();
}

DumpStatus EventDetailDumper::PreExecute(const shared_ptr<DumperParameter> &parameter, StringMatrix dumpDatas)
{
    DUMPER_HILOGI(MODULE_COMMON, "info|EventDetailDumper PreExecute");
    showEventCount_ = parameter->GetOpts().showEventCount_;
    processName_ = parameter->GetOpts().processName_;
    startTime_ = parameter->GetOpts().startTime_;
    endTime_ = parameter->GetOpts().endTime_;
    eventId_ = parameter->GetOpts().eventId_;
    if (!eventId_.empty()) {
        showEventCount_ = -1;
        startTime_ = 0;
        endTime_ = 0;
    }
    dumpDatas_ = dumpDatas;
    return DumpStatus::DUMP_OK;
}

DumpStatus EventDetailDumper::Execute()
{
    DUMPER_HILOGI(MODULE_COMMON, "info|EventDetailDumper Execute");
    if (!QueryFaultEvents()) {
        DUMPER_HILOGE(MODULE_COMMON, "error|EventDetailDumper Execute DumpFaultEventListByPK failed");
        return DumpStatus::DUMP_FAIL;
    }

    auto logPaths = FilterLogPaths();
    if (logPaths.empty()) {
        DUMPER_HILOGI(MODULE_COMMON, "no fault logs found");
        return DumpStatus::DUMP_OK;
    }

    ReadLogsByPaths(logPaths);

    DUMPER_HILOGI(MODULE_COMMON, "info|EventDetailDumper Execute end");
    return DumpStatus::DUMP_OK;
}

bool EventDetailDumper::QueryFaultEvents()
{
    EventQueryParam param;
    param.startTime_ = startTime_;
    param.endTime_ = endTime_;

    std::shared_ptr<DumpEventInfo> dumpEventInfo = std::make_shared<DumpEventInfo>();
    return dumpEventInfo->DumpFaultEventListByPK(events_, param);
}

std::vector<std::string> EventDetailDumper::FilterLogPaths()
{
    std::vector<std::string> logPaths;
    for (const auto &event : events_) {
        std::string value;
        if (!processName_.empty() && event.GetParamValue("PROCESS_NAME", value) == 0) {
            if (value.find(processName_) == std::string::npos) {
                continue;
            }
        }
        if (!eventId_.empty() && event.GetParamValue("id_", value) == 0) {
            if (value.find(eventId_) != 0) {
                continue;
            }
        }
        if (event.GetParamValue("LOG_PATH", value) == 0) {
            logPaths.emplace_back(value);
        }
    }
    if (showEventCount_ > 0 && logPaths.size() > static_cast<size_t>(showEventCount_)) {
        logPaths.erase(logPaths.begin() + showEventCount_, logPaths.end());
    }
    return logPaths;
}

void EventDetailDumper::ReadLogsByPaths(const std::vector<std::string> &logPaths)
{
    for (const auto &path : logPaths) {
        std::vector<std::string> line_vector_blank;
        line_vector_blank.push_back("");
        std::vector<std::string> line_vector_filename;
        line_vector_filename.push_back(path);
        dumpDatas_->push_back(line_vector_blank);
        dumpDatas_->push_back(line_vector_filename);
        dumpDatas_->push_back(line_vector_blank);

        ReadSingleLogFile(path);

        if (IsCanceled()) {
            break;
        }
    }
}

void EventDetailDumper::ReadSingleLogFile(const std::string &path)
{
    CloseFd();

    fd_ = DumpUtils::FdToRead(path);
    if (fd_ == -1) {
        std::vector<std::string> expiryNote;
        expiryNote.emplace_back("Read failed, log may be outdated or file unreadable");
        dumpDatas_->push_back(expiryNote);
        return;
    }
    fp_ = fdopen(fd_, "r");
    if (fp_ == nullptr) {
        DUMPER_HILOGE(MODULE_COMMON, "logPaths fdopen failed");
        close(fd_);
        fd_ = -1;
        return;
    }

    char* lineBuffer = nullptr;
    size_t len = 0;
    ssize_t read = 0;

    while (!IsCanceled()) {
        read = getline(&lineBuffer, &len, fp_);
        if (read == -1) {
            if (feof(fp_) == 0) {
                DUMPER_HILOGE(MODULE_COMMON, "logPaths getline failed");
            }
            break;
        }

        if (lineBuffer[read - 1] == '\n') {
            lineBuffer[read - 1] = '\0';
        }
        std::string line = lineBuffer;
        std::vector<std::string> lineVector;
        lineVector.push_back(line);
        dumpDatas_->push_back(lineVector);
    }

    if (lineBuffer != nullptr) {
        free(lineBuffer);
        lineBuffer = nullptr;
    }
}

DumpStatus EventDetailDumper::AfterExecute()
{
    CloseFd();
    return DumpStatus::DUMP_OK;
}

void EventDetailDumper::CloseFd()
{
    if (fp_ != nullptr) {
        fclose(fp_);
        fp_ = nullptr;
        fd_ = -1;
    } else if (fd_ >= 0) {
        close(fd_);
        fd_ = -1;
    }
};
} // namespace HiviewDFX
} // namespace OHOS