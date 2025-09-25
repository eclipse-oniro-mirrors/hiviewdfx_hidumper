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
#ifndef DUMP_EVENT_INFO_H
#define DUMP_EVENT_INFO_H

#include <unordered_set>

#include "executor/event/event_query_callback.h"


namespace OHOS {
namespace HiviewDFX {
struct EventQueryParam {
    long long startTime_;
    long long endTime_;
    std::string domain;
    std::vector<std::string> eventList;
};
class DumpEventInfo {
public:
    DumpEventInfo();
    ~DumpEventInfo();

    bool DumpEventList(std::vector<HiSysEventRecord> &events, const EventQueryParam &param);
    bool DumpFaultEventListByPK(std::vector<HiSysEventRecord> &events, EventQueryParam &param);
private:
    static const std::unordered_set<std::string> faultEventSet_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif