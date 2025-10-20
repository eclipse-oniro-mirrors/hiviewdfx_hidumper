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

#include "executor/event/dump_event_info.h"
#include "hilog_wrapper.h"
#include "util/string_utils.h"
using namespace std;
namespace OHOS {
namespace HiviewDFX {

constexpr int TIMEOUT_PERIOD = 5; // seconds

DumpEventInfo::DumpEventInfo()
{
}

DumpEventInfo::~DumpEventInfo()
{
}

static const std::unordered_set<std::string> FAULTEVENTSET = {
    "Js Error", "Cpp Crash", "THREAD_BLOCK_6S", "APP_INPUT_BLOCK", "LIFECYCLE_TIMEOUT",
    "JsError", "CppCrash", "ThreadBlock6S", "AppInputBlock", "LifecycleTimeout"
};

bool DumpEventInfo::DumpEventList(std::vector<HiSysEventRecord> &events, EventQueryParam &param, bool isSort)
{
    long long startTime = param.startTime_ == 0 ? -1 : param.startTime_;
    long long endTime = param.endTime_ == 0 ? -1 : param.endTime_;
    QueryArg arg(startTime, endTime);
    std::vector<QueryRule> queryRules;
    for (const auto &[domain, eventList] : param.queryRule) {
        queryRules.emplace_back(domain, eventList);
    }
    auto queryCallback = std::make_shared<EventQueryCallback>();
    int ret = HiSysEventManager::Query(arg, queryRules, queryCallback);
    if (ret != 0 && ret != -ENFILE) {
        DUMPER_HILOGE(MODULE_SERVICE, "DumpEventList Query failed, ret:%{public}d", ret);
        return false;
    }
    if (!queryCallback->WaitForComplete(std::chrono::seconds(TIMEOUT_PERIOD))) {
        DUMPER_HILOGE(MODULE_SERVICE, "Event Query timeout");
        return false;
    }
    int reason = queryCallback->GetReason();
    if (reason != 0 && reason != -ENFILE) {
        DUMPER_HILOGE(MODULE_SERVICE, "Query failed with reason: %{public}d", reason);
        return false;
    }
    auto queriedEvents = queryCallback->GetEvents();
    events.insert(events.end(), queriedEvents.begin(), queriedEvents.end());
    if (isSort) {
        SortEventsByTimeDesc(events);
    }
    return true;
}

EventDumpResult DumpEventInfo::DumpFaultEventListByPK(std::vector<HiSysEventRecord> &events, EventQueryParam &param)
{
    std::vector<HiSysEventRecord> pkEvents;
    param.queryRule = {
        {"FRAMEWORK", {"PROCESS_KILL"}},
        {"KERNEL_VENDOR", {"PROCESS_KILL"}}
    };
    if (!DumpEventList(pkEvents, param)) {
        DUMPER_HILOGE(MODULE_SERVICE, "DumpFaultEventListByPK dump processkill events failed");
        return EventDumpResult::EVENT_DUMP_FAIL;
    }
    if (pkEvents.empty()) {
        return EventDumpResult::NONE_PROCESSKILL_EVENT;
    }
    std::unordered_set<std::string> faultEventQuerySet;
    std::unordered_set<std::string> pkRunningIdSet;
    auto eventResult = ExtractPkRunningIdsAndFaultTypes(pkEvents, pkRunningIdSet, faultEventQuerySet, param);
    if (eventResult != EventDumpResult::EVENT_DUMP_OK) {
        DUMPER_HILOGW(MODULE_SERVICE, "Match fault events failed");
        return eventResult;
    }

    FillQueryParam(param, faultEventQuerySet);

    std::vector<HiSysEventRecord> faultEvents;
    if (!DumpEventList(faultEvents, param)) {
        DUMPER_HILOGE(MODULE_SERVICE, "DumpFaultEventListByPK dump fault events failed");
        return EventDumpResult::EVENT_DUMP_FAIL;
    }
    for (const auto &event : faultEvents) {
        std::string runningId;
        if (event.GetParamValue("APP_RUNNING_UNIQUE_ID", runningId) == 0 &&
            pkRunningIdSet.find(runningId) != pkRunningIdSet.end()) {
            events.emplace_back(event);
        }
    }
    SortEventsByTimeDesc(events);
    return EventDumpResult::EVENT_DUMP_OK;
}

EventDumpResult DumpEventInfo::ExtractPkRunningIdsAndFaultTypes(const std::vector<HiSysEventRecord> &pkEvents,
                                                                std::unordered_set<std::string> &pkRunningIdSet,
                                                                std::unordered_set<std::string> &faultEventQuerySet,
                                                                const EventQueryParam &param)
{
    bool notFaultEvents = false;
    for (const auto &event : pkEvents) {
        std::string runningId;
        std::string reason;
        std::string processName;
        std::string eventId;
        if (event.GetParamValue("PROCESS_NAME", processName) != 0 ||
            event.GetParamValue("id_", eventId) != 0 ||
            event.GetParamValue("REASON", reason) != 0) {
            continue;
        }
        if (!param.processName_.empty() && processName.find(param.processName_) == std::string::npos) {
            continue;
        }

        if (!param.eventId_.empty() && eventId.find(param.eventId_) != 0) {
            continue;
        }

        if (FAULTEVENTSET.find(reason) == FAULTEVENTSET.end()) {
            notFaultEvents = true;
            continue;
        }
        if (event.GetParamValue("APP_RUNNING_UNIQUE_ID", runningId) == 0) {
            pkRunningIdSet.insert(runningId);
            if (reason == "Cpp Crash" || reason == "CppCrash") {
                faultEventQuerySet.insert("CPP_CRASH");
            } else if (reason == "Js Error" || reason == "JsError") {
                faultEventQuerySet.insert("JS_ERROR");
            } else if (reason == "LIFECYCLE_TIMEOUT" || reason == "LifecycleTimeout") {
                faultEventQuerySet.insert("SYS_FREEZE");
            } else {
                faultEventQuerySet.insert("APP_FREEZE");
            }
        }
    }
    if (pkRunningIdSet.empty()) {
        return notFaultEvents ? EventDumpResult::NOT_FAULT_EVENT : EventDumpResult::NONE_PROCESSKILL_EVENT;
    }
    return EventDumpResult::EVENT_DUMP_OK;
}

void DumpEventInfo::FillQueryParam(EventQueryParam &param, const std::unordered_set<std::string> &faultEventQuerySet)
{
    param.queryRule.clear();
    param.startTime_ = 0;
    param.endTime_ = 0;
    if (faultEventQuerySet.find("CPP_CRASH") != faultEventQuerySet.end()) {
        param.queryRule.emplace_back("RELIABILITY", std::vector<std::string>{"CPP_CRASH"});
    }
    if (faultEventQuerySet.find("JS_ERROR") != faultEventQuerySet.end()) {
        param.queryRule.emplace_back("AAFWK", std::vector<std::string>{"JS_ERROR"});
        param.queryRule.emplace_back("ACE", std::vector<std::string>{"JS_ERROR"});
    }
    if (faultEventQuerySet.find("SYS_FREEZE") != faultEventQuerySet.end()) {
        param.queryRule.emplace_back("RELIABILITY", std::vector<std::string>{"SYS_FREEZE"});
    }
    if (faultEventQuerySet.find("APP_FREEZE") != faultEventQuerySet.end()) {
        param.queryRule.emplace_back("RELIABILITY", std::vector<std::string>{"APP_FREEZE"});
    }
}

void DumpEventInfo::SortEventsByTimeDesc(std::vector<HiSysEventRecord>& events)
{
    std::sort(events.begin(), events.end(),
        [](const HiSysEventRecord& a, const HiSysEventRecord& b) {
            return a.GetTime() > b.GetTime();
        });
}
} // namespace HiviewDFX
} // namespace OHOS
