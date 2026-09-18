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
#include <iomanip>
#include <sstream>

#include "executor/event_list_dumper.h"
#include "util/string_utils.h"
#include "xcollie/process_kill_reason.h"

using namespace std;
namespace OHOS {
namespace HiviewDFX {

constexpr int LINE_SPACING = 6;
constexpr int MAX_WIDTH = 32;
static const std::string END_BLANK = "  ";
static constexpr const char* INVALID_KILL_ID = "InvalidKillId";

EventListDumper::EventListDumper() : startTime_(0), endTime_(0), showEventCount_(-1)
{
}

EventListDumper::~EventListDumper()
{
}

static const std::vector<std::string> EVENTTITLES = { "time", "foreground", "reason", "record_id", "process_name" };

static const std::unordered_map<std::string, std::string> FIELDMAP = {
    {"time", "time_"},
    {"process_name", "PROCESS_NAME"},
    {"foreground", "FOREGROUND"},
    {"reason", KILL_ID_KEY},
    {"record_id", "id_"}
};

DumpStatus EventListDumper::PreExecute(const shared_ptr<DumperParameter> &parameter, StringMatrix dumpDatas)
{
    DUMPER_HILOGI(MODULE_COMMON, "info|EventListDumper PreExecute");
    showEventCount_ = parameter->GetOpts().showEventCount_;
    processName_ = parameter->GetOpts().processName_;
    startTime_ = parameter->GetOpts().startTime_;
    endTime_ = parameter->GetOpts().endTime_;
    dumpDatas_ = dumpDatas;
    return DumpStatus::DUMP_OK;
}

DumpStatus EventListDumper::Execute()
{
    DUMPER_HILOGI(MODULE_COMMON, "info|EventListDumper Execute");

    if (!QueryEvents()) {
        return DumpStatus::DUMP_FAIL;
    }
    std::unordered_map<std::string, int> columnWidths;
    for (const auto& title : EVENTTITLES) {
        columnWidths[title] = static_cast<int>(title.size());
    }
    auto results = BuildResults(columnWidths);
    if (results.empty()) {
        vector<string> emptyResults;
        emptyResults.emplace_back("no records found.");
        dumpDatas_->push_back(emptyResults);
        DUMPER_HILOGI(MODULE_COMMON, "info|EventListDumper Execute no data");
        return DumpStatus::DUMP_OK;
    }

    results.insert(results.begin(), EVENTTITLES);

    FormatResults(results, columnWidths);
    DUMPER_HILOGI(MODULE_COMMON, "info|EventListDumper Execute end");
    return DumpStatus::DUMP_OK;
}

bool EventListDumper::QueryEvents()
{
    EventQueryParam param;
    param.startTime_ = startTime_;
    param.endTime_ = endTime_;
    param.queryRule = {
        {"FRAMEWORK", {"PROCESS_KILL"}},
        {"KERNEL_VENDOR", {"PROCESS_KILL"}}
    };
    std::shared_ptr<DumpEventInfo> dumpEventInfo = std::make_shared<DumpEventInfo>();
    bool ret = dumpEventInfo->DumpEventList(events_, param, true);
    DUMPER_HILOGI(MODULE_COMMON, "info|EventListDumper QueryEvents ret=%{public}d, queried=%{public}zu",
                  static_cast<int>(ret), events_.size());
    return ret;
}

std::vector<std::vector<std::string>> EventListDumper::BuildResults(std::unordered_map<std::string, int> &columnWidths)
{
    std::vector<std::vector<std::string>> results;

    for (const auto &event : events_) {
        int64_t killId = 0;
        if (ShouldSkipEvent(event, killId)) {
            continue;
        }
        auto row = BuildRow(event, killId, columnWidths);
        if (row.empty() || row.size() != EVENTTITLES.size()) {
            continue;
        }
        results.emplace_back(std::move(row));
    }

    if (showEventCount_ > 0 && results.size() > static_cast<size_t>(showEventCount_)) {
        results.resize(static_cast<size_t>(showEventCount_));
    }
    return results;
}

bool EventListDumper::ShouldSkipEvent(const HiSysEventRecord& event, int64_t &killId)
{
    std::string processName;
    if (event.GetParamValue("PROCESS_NAME", processName) != 0 || processName.empty()) {
        return true;
    }
    if (event.GetParamValue(KILL_ID_KEY, killId) != 0 || killId < INT32_MIN || killId > INT32_MAX) {
        return true;
    }
    if (!processName_.empty() && processName.find(processName_) == std::string::npos) {
        return true;
    }
    return false;
}

std::vector<std::string> EventListDumper::BuildRow(const HiSysEventRecord& event, int64_t killId,
                                                   std::unordered_map<std::string, int>& columnWidths)
{
    std::vector<std::string> row;
    for (const auto& title : EVENTTITLES) {
        std::string value;
        if (event.GetParamValue(FIELDMAP.at(title), value) != 0) {
            row.emplace_back("Null");
            continue;
        }
        if (title == "time") {
            value = StringUtils::GetInstance().UnixMsToString(event.GetTime());
        } else if (title == "foreground") {
            value = (value == "1") ? "True" : "False";
        } else if (title == "reason") {
            value = ProcessKillReason::GetAppExitReason(static_cast<int>(killId));
            if (value == INVALID_KILL_ID) {
                return row;
            }
        }
        if (static_cast<int>(value.size()) > columnWidths[title]) {
            columnWidths[title] = static_cast<int>(value.size());
        }
        row.emplace_back(value);
    }
    return row;
}

void EventListDumper::FormatResults(const std::vector<std::vector<std::string>>& results,
                                    const std::unordered_map<std::string, int>& columnWidths)
{
    for (const auto& row : results) {
        std::ostringstream oss;
        for (size_t i = 0; i < EVENTTITLES.size(); ++i) {
            const std::string& title = EVENTTITLES[i];
            std::string value = (i < row.size()) ? row[i] : "Null";
            int width = std::min(columnWidths.at(title), MAX_WIDTH);
            oss << std::left << std::setw(width + LINE_SPACING) << value << END_BLANK;
        }
        vector<string> tempResult;
        tempResult.push_back(oss.str());
        dumpDatas_->push_back(tempResult);
    }
}

DumpStatus EventListDumper::AfterExecute()
{
    return DumpStatus::DUMP_OK;
}
} // namespace HiviewDFX
} // namespace OHOS