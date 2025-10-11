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
#ifndef EVENT_LIST_DUMPER_H
#define EVENT_LIST_DUMPER_H
#include <vector>
#include <string>
#include <memory>
#include "hidumper_executor.h"
#include "event/dump_event_info.h"
#include "cJSON.h"

namespace OHOS {
namespace HiviewDFX {
class EventListDumper : public HidumperExecutor {
public:
    EventListDumper();
    ~EventListDumper();

    DumpStatus PreExecute(const std::shared_ptr<DumperParameter> &parameter, StringMatrix dumpDatas) override;
    DumpStatus Execute() override;
    DumpStatus AfterExecute() override;

private:
    DumpStatus ParseConfigFile();
    DumpStatus ParseJsonContent(const std::string& content);
    bool QueryEvents();
    std::vector<std::vector<std::string>> BuildResults(std::unordered_map<std::string, int> &columnWidths);
    void FormatResults(const std::vector<std::vector<std::string>> &results,
                       const std::unordered_map<std::string, int> &columnWidths);
    std::string transformReason(const std::string& value);
    std::vector<std::string> BuildRow(const HiSysEventRecord& event,
                                      std::unordered_map<std::string, int>& columnWidths);
    bool ShouldSkipEvent(const HiSysEventRecord& event);

private:
    StringMatrix dumpDatas_;
    std::string processName_;
    long long startTime_;
    long long endTime_;
    int showEventCount_;
    std::vector<HiSysEventRecord> events_;
    std::unordered_map<std::string, std::string> eventReasonMap_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif
