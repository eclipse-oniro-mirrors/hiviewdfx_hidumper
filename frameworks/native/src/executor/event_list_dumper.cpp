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
#include <fstream>
#include <sstream>

#include "executor/event_list_dumper.h"
#include "util/string_utils.h"

using namespace std;
namespace OHOS {
namespace HiviewDFX {

constexpr int LINE_SPACING = 6;
constexpr int MAX_WIDTH = 32;
static const std::string END_BLANK = "  ";

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
    {"reason", "REASON"},
    {"record_id", "id_"}
};

static constexpr const char* DEFAULT_CONFIG_PATH = "/system/etc/hidumper/event_reason_config.json";

DumpStatus EventListDumper::PreExecute(const shared_ptr<DumperParameter> &parameter, StringMatrix dumpDatas)
{
    DUMPER_HILOGI(MODULE_COMMON, "info|EventListDumper PreExecute");
    showEventCount_ = parameter->GetOpts().showEventCount_;
    processName_ = parameter->GetOpts().processName_;
    startTime_ = parameter->GetOpts().startTime_;
    endTime_ = parameter->GetOpts().endTime_;
    dumpDatas_ = dumpDatas;
    auto status = ParseConfigFile();
    if (status != DumpStatus::DUMP_OK) {
        DUMPER_HILOGE(MODULE_COMMON, "error|EventListDumper PreExecute ParseConfigFile failed");
        return status;
    }
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
    return dumpEventInfo->DumpEventList(events_, param);
}

std::vector<std::vector<std::string>> EventListDumper::BuildResults(std::unordered_map<std::string, int> &columnWidths)
{
    std::vector<std::vector<std::string>> results;

    for (const auto &event : events_) {
        if (ShouldSkipEvent(event)) {
            continue;
        }
        auto row = BuildRow(event, columnWidths);
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

bool EventListDumper::ShouldSkipEvent(const HiSysEventRecord& event)
{
    std::string processName;
    std::string reason;
    if (event.GetParamValue("PROCESS_NAME", processName) != 0) {
        return true;
    }
    if (event.GetParamValue("REASON", reason) != 0) {
        return true;
    }
    if (!processName_.empty() && processName.find(processName_) == std::string::npos) {
        return true;
    }
    return false;
}

std::vector<std::string> EventListDumper::BuildRow(const HiSysEventRecord& event,
                                                   std::unordered_map<std::string, int>& columnWidths)
{
    std::vector<std::string> row;
    for (const auto& title : EVENTTITLES) {
        std::string value;
        if (event.GetParamValue(FIELDMAP.at(title), value) != 0) {
            row.emplace_back("NULL");
            continue;
        }
        if (title == "time") {
            value = StringUtils::GetInstance().UnixMsToString(event.GetTime());
        } else if (title == "foreground") {
            value = (value == "1") ? "TRUE" : "FALSE";
        } else if (title == "reason") {
            std::string newValue = transformReason(value);
            if (newValue.empty()) {
                return row;
            } else {
                value = newValue;
            }
        }
        if (static_cast<int>(value.size()) > columnWidths[title]) {
            columnWidths[title] = static_cast<int>(value.size());
        }
        row.emplace_back(value);
    }
    return row;
}

std::string EventListDumper::transformReason(const std::string& value)
{
    auto it = eventReasonMap_.find(value);
    if (it != eventReasonMap_.end()) {
        return it->second;
    }

    auto valueIt = std::find_if(eventReasonMap_.begin(), eventReasonMap_.end(),
                                [&value](const auto& kv) { return kv.second == value; });
    if (valueIt != eventReasonMap_.end()) {
        return value;
    }
    return "";
}

void EventListDumper::FormatResults(const std::vector<std::vector<std::string>>& results,
                                    const std::unordered_map<std::string, int>& columnWidths)
{
    for (const auto& row : results) {
        std::ostringstream oss;
        for (size_t i = 0; i < EVENTTITLES.size(); ++i) {
            const std::string& title = EVENTTITLES[i];
            std::string value = (i < row.size()) ? row[i] : "NULL";
            int width = std::min(columnWidths.at(title), MAX_WIDTH);
            oss << std::left << std::setw(width + LINE_SPACING) << value << END_BLANK;
        }
        vector<string> tempResult;
        tempResult.push_back(oss.str());
        dumpDatas_->push_back(tempResult);
    }
}

DumpStatus EventListDumper::ParseConfigFile()
{
    std::ifstream file(DEFAULT_CONFIG_PATH);
    if (!file.is_open()) {
        DUMPER_HILOGE(MODULE_COMMON, "Failed to open config file: %{public}s", DEFAULT_CONFIG_PATH);
        return DumpStatus::DUMP_FAIL;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    file.close();

    return ParseJsonContent(content);
}

DumpStatus EventListDumper::ParseJsonContent(const std::string& content)
{
    auto root = std::unique_ptr<cJSON, decltype(&cJSON_Delete)>(cJSON_Parse(content.c_str()), cJSON_Delete);
    if (root == nullptr) {
        DUMPER_HILOGE(MODULE_COMMON, "Failed to parse JSON content");
        return DumpStatus::DUMP_FAIL;
    }

    cJSON* eventConfigs = cJSON_GetObjectItem(root.get(), "event_reason_configs");
    if (eventConfigs == nullptr || !cJSON_IsObject(eventConfigs)) {
        DUMPER_HILOGE(MODULE_COMMON, "Missing or invalid 'event_reason_configs' field");
        return DumpStatus::DUMP_FAIL;
    }

    cJSON* item = nullptr;
    cJSON_ArrayForEach(item, eventConfigs) {
        if (cJSON_IsString(item) && item->string != nullptr) {
            eventReasonMap_[item->string] = item->valuestring;
        }
    }

    return DumpStatus::DUMP_OK;
}

DumpStatus EventListDumper::AfterExecute()
{
    return DumpStatus::DUMP_OK;
}
} // namespace HiviewDFX
} // namespace OHOS