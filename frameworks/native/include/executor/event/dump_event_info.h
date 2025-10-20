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
#ifndef HIVIEWDFX_HIDUMPER_DUMP_EVENT_INFO_H
#define HIVIEWDFX_HIDUMPER_DUMP_EVENT_INFO_H

#include <unordered_set>

#include "executor/event/event_query_callback.h"

namespace OHOS {
namespace HiviewDFX {
struct EventQueryParam {
    long long startTime_;
    long long endTime_;
    std::string processName_;
    std::string eventId_;
    std::vector<std::pair<std::string, std::vector<std::string>>> queryRule;
};

enum EventDumpResult {
    NONE_PROCESSKILL_EVENT = -3,
    NOT_FAULT_EVENT = -2,
    EVENT_DUMP_FAIL = -1,
    EVENT_DUMP_OK = 0,
};
class DumpEventInfo {
public:
    DumpEventInfo();
    ~DumpEventInfo();

    bool DumpEventList(std::vector<HiSysEventRecord> &events, EventQueryParam &param);
    EventDumpResult DumpFaultEventListByPK(std::vector<HiSysEventRecord> &events, EventQueryParam &param);
private:
    EventDumpResult ExtractPkRunningIdsAndFaultTypes(const std::vector<HiSysEventRecord> &pkEvents,
                                                     std::unordered_set<std::string> &pkRunningIdSet,
                                                     std::unordered_set<std::string> &faultEventQuerySet,
                                                     const EventQueryParam &param);
    void FillQueryParam(EventQueryParam &param, const std::unordered_set<std::string> &faultEventQuerySet);
};
} // namespace HiviewDFX
} // namespace OHOS
#endif