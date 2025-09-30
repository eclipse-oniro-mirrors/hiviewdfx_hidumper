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
    "Js Error", "Cpp Crash", "THREAD_BLOCK_6S", "APP_INPUT_BLOCK", "LIFECYCLE_TIMEOUT"
};

bool DumpEventInfo::DumpEventList(std::vector<HiSysEventRecord> &events, const EventQueryParam &param)
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
    return true;
}

bool DumpEventInfo::DumpFaultEventListByPK(std::vector<HiSysEventRecord> &events, EventQueryParam &param)
{
    std::vector<HiSysEventRecord> pkEvents;
    param.queryRule = {
        {"FRAMEWORK", {"PROCESS_KILL"}},
        {"KERNEL_VENDOR", {"PROCESS_KILL"}}
    };
    if (!DumpEventList(pkEvents, param)) {
        DUMPER_HILOGE(MODULE_SERVICE, "DumpFaultEventListByPK DumpEventList failed");
        return false;
    }
    if (pkEvents.empty()) {
        DUMPER_HILOGI(MODULE_SERVICE, "No PROCESS_KILL events found");
        return true;
    }
    std::unordered_set<std::string> faultEventQuerySet;
    std::unordered_set<std::string> pkRunningIdSet;
    if (!ExtractPkRunningIdsAndFaultTypes(pkEvents, pkRunningIdSet, faultEventQuerySet)) {
        DUMPER_HILOGI(MODULE_SERVICE, "No fault events to query");
        return true;
    }
    param.queryRule.clear();
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
    std::vector<HiSysEventRecord> faultEvents;
    if (!DumpEventList(faultEvents, param)) {
        DUMPER_HILOGE(MODULE_SERVICE, "DumpFaultEventListByPK DumpEventList failed");
        return false;
    }
    for (const auto &event : faultEvents) {
        std::string runningId;
        if (event.GetParamValue("APP_RUNNING_UNIQUE_ID", runningId) == 0 &&
            pkRunningIdSet.find(runningId) != pkRunningIdSet.end()) {
            events.emplace_back(event);
        }
    }
    return true;
}

bool DumpEventInfo::ExtractPkRunningIdsAndFaultTypes(const std::vector<HiSysEventRecord> &pkEvents,
                                                     std::unordered_set<std::string> &pkRunningIdSet,
                                                     std::unordered_set<std::string> &faultEventQuerySet)
{
    for (const auto &event : pkEvents) {
        std::string runningId;
        std::string reason;
        if (event.GetParamValue("APP_RUNNING_UNIQUE_ID", runningId) == 0 &&
            event.GetParamValue("REASON", reason) == 0 &&
            FAULTEVENTSET.find(reason) != FAULTEVENTSET.end()) {
            pkRunningIdSet.insert(runningId);
            if (reason == "Cpp Crash") {
                faultEventQuerySet.insert("CPP_CRASH");
            } else if (reason == "Js Error") {
                faultEventQuerySet.insert("JS_ERROR");
            } else if (reason == "LIFECYCLE_TIMEOUT") {
                faultEventQuerySet.insert("SYS_FREEZE");
            } else {
                faultEventQuerySet.insert("APP_FREEZE");
            }
        }
    }
    return !pkRunningIdSet.empty();
}
} // namespace HiviewDFX
} // namespace OHOS
