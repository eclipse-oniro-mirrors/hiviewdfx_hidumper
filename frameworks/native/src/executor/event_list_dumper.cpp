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

#include "executor/event_list_dumper.h"
#include "util/string_utils.h"

using namespace std;
namespace OHOS {
namespace HiviewDFX {

constexpr int LINE_SPACING = 6;
constexpr int MAX_WIDTH = 25;
static const std::string END_BLANK = "  ";

EventListDumper::EventListDumper()
{
}

EventListDumper::~EventListDumper()
{
}

const std::vector<std::string> EventListDumper::eventTitles = { "time", "process_name",
    "foreground", "reason", "record_id" };

const std::unordered_map<std::string, std::string> EventListDumper::fieldMap = {
    {"time", "time_"},
    {"process_name", "PROCESS_NAME"},
    {"foreground", "FOREGROUND"},
    {"reason", "REASON"},
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
    DUMPER_HILOGI(MODULE_COMMON, "showEventCount_=%{public}d, processName_=%{public}s, startTime_=%{public}lld, endTime_=%{public}lld",
        showEventCount_, processName_.c_str(), startTime_, endTime_);
    return DumpStatus::DUMP_OK;
}

DumpStatus EventListDumper::Execute()
{
    DUMPER_HILOGI(MODULE_COMMON, "info|EventListDumper Execute");

    if (!QueryEvents()) {
        DUMPER_HILOGE(MODULE_COMMON, "error|EventListDumper Execute DumpEventList failed");
        return DumpStatus::DUMP_FAIL;
    }
    std::unordered_map<std::string, int> columnWidths;
    for (const auto& title : eventTitles) {
        columnWidths[title] = static_cast<int>(title.size());
    }
    auto results = BuildResults(columnWidths);
    if (results.empty()) {
        DUMPER_HILOGI(MODULE_COMMON, "info|EventListDumper Execute no data");
        return DumpStatus::DUMP_OK;
    }

    if (showEventCount_ > 0 && results.size() > static_cast<size_t>(showEventCount_)) {
        results.erase(results.begin() + showEventCount_, results.end());
    }

    results.insert(results.begin(), eventTitles);

    FormatResults(results, columnWidths);
    DUMPER_HILOGI(MODULE_COMMON, "info|EventListDumper Execute end");
    return DumpStatus::DUMP_OK;
}

bool EventListDumper::QueryEvents()
{
    EventQueryParam param;
    param.startTime_ = startTime_;
    param.endTime_ = endTime_;
    param.domain = "";
    param.eventList = { "PROCESS_KILL" };

    std::shared_ptr<DumpEventInfo> dumpEventInfo = std::make_shared<DumpEventInfo>();
    return dumpEventInfo->DumpEventList(events_, param);
}

std::vector<std::vector<std::string>> EventListDumper::BuildResults(std::unordered_map<std::string, int> &columnWidths)
{
    std::vector<std::vector<std::string>> results;

    for (const auto &event : events_) {
        std::vector<std::string> row;
        std::string processName;
        if (!processName_.empty() && event.GetParamValue("PROCESS_NAME", processName) == 0) {
            if (processName.find(processName_) == std::string::npos) {
                continue;
            }
        }
        for (const auto& title : eventTitles) {
            std::string value;
            if (event.GetParamValue(fieldMap.at(title), value) != 0) {
                row.emplace_back("NULL");
                continue;
            }
            if (title == "time") {
                value = StringUtils::GetInstance().UnixMsToString(event.GetTime());
            } else if (title == "foreground") {
                value = (value == "1") ? "TRUE" : "FALSE";
            }
            if (static_cast<int>(value.size()) > columnWidths[title]) {
                columnWidths[title] = static_cast<int>(value.size());
            }
            row.emplace_back(value);
        }

        if (row.size() == eventTitles.size()) {
            results.emplace_back(row);
        }
        // DUMPER_HILOGI(MODULE_COMMON, "event[%{public}zu]: %{public}s", i, events_[i].AsJson().c_str());
    }
    return results;
}

void EventListDumper::FormatResults(const std::vector<std::vector<std::string>>& results,
                                    const std::unordered_map<std::string, int>& columnWidths)
{
    for (const auto& row : results) {
        std::ostringstream oss;
        for (size_t i = 0; i < eventTitles.size(); ++i) {
            const std::string& title = eventTitles[i];
            std::string value = (i < row.size()) ? row[i] : "NULL";
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