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


#include <gtest/gtest.h>
#include <unistd.h>
#include <cstdint>

#include "executor/event_detail_dumper.h"
#include "executor/event_list_dumper.h"
#include "xcollie/process_kill_reason.h"


using namespace testing::ext;
using namespace std;
namespace OHOS {
namespace HiviewDFX {
constexpr size_t RECORD_COUNT_WITH_VALID_KILL_ID = 4;
constexpr size_t REASON_COLUMN_INDEX = 2;
constexpr size_t FOREGROUND_COLUMN_INDEX = 1;
constexpr size_t LINE_COUNT_OF_ONE_ROW = 1;
constexpr size_t REASON_CASE_COUNT = 12;
constexpr size_t CLAMPED_LINE_LEN = 76;
constexpr int MAX_COLUMN_WIDTH = 32;
constexpr int OVER_WIDTH_DELTA = 100;
constexpr int SHOW_COUNT_NOT_TRUNCATE = 100;
constexpr int64_t KILL_ID_TRUNCATED_TO_JS_ERROR = 4294969299LL;
constexpr char INVALID_KILL_ID_STR[] = "InvalidKillId";
constexpr size_t SEGMENT_BOUNDARY_COUNT = 9;
constexpr size_t SHARE_REASON_CASE_COUNT = 8;
constexpr size_t KILL_APPLICATION_GROUP_COUNT = 4;
constexpr size_t FAULT_TYPE_CASE_COUNT = 5;
constexpr size_t FAULT_TYPE_KIND_COUNT = 4;
constexpr size_t SINGLE_FAULT_RULE_COUNT = 1;
constexpr size_t JS_ERROR_RULE_COUNT = 2;
constexpr long long QUERY_START_TIME = 100;
constexpr long long QUERY_END_TIME = 200;
std::unordered_map<std::string, int> columnWidths = {
    {"time", 0},
    {"process_name", 0},
    {"foreground", 0},
    {"reason", 0},
    {"record_id", 0},
};

constexpr const char* ORIGINS[] = {
    "{\"domain_\":\"FRAMEWORK\",\"name_\":\"PROCESS_KILL\","
    "\"time_\":1502965663170,\"PROCESS_NAME\":\"hidumper1 unittest\","
    "\"FOREGROUND\":\"1\",\"id_\":\"14645518577780955344\","
    "\"REASON\":\"LIFECYCLE_TIMEOUT\",\"KILL_ID\":2002,\"APP_RUNNING_UNIQUE_ID\":\"1\"}",

    "{\"domain_\":\"FRAMEWORK\",\"name_\":\"PROCESS_KILL\","
    "\"time_\":1502965663175,\"PROCESS_NAME\":\"hidumper2 unittest\","
    "\"FOREGROUND\":\"1\",\"id_\":\"14645518577780955345\","
    "\"REASON\":\"Cpp Crash\",\"KILL_ID\":2004,\"APP_RUNNING_UNIQUE_ID\":\"2\"}",

    "{\"domain_\":\"FRAMEWORK\",\"name_\":\"PROCESS_KILL\","
    "\"time_\":1502965663178,\"PROCESS_NAME\":\"Js Error unittest\","
    "\"FOREGROUND\":\"1\",\"REASON\":\"Js Error\","
    "\"KILL_ID\":2003,\"APP_RUNNING_UNIQUE_ID\":\"3\"}",

    "{\"domain_\":\"FRAMEWORK\",\"name_\":\"PROCESS_KILL\","
    "\"time_\":4916632891988,\"PROCESS_NAME\":\"Js Error unittest\","
    "\"FOREGROUND\":\"1\",\"id_\":\"3333518577780955356\","
    "\"REASON\":\"THREAD_BLOCK_6S\",\"KILL_ID\":2000,\"APP_RUNNING_UNIQUE_ID\":\"4\"}",

    "{\"domain_\":\"FRAMEWORK\",\"name_\":\"PROCESS_KILL\","
    "\"time_\":4916632892000,\"PROCESS_NAME\":\"Js Error unittest\","
    "\"FOREGROUND\":\"1\",\"id_\":\"3333518577780955378\","
    "\"REASON\":\"test5\",\"KILL_ID\":-1,\"APP_RUNNING_UNIQUE_ID\":\"5\"}",

    "{\"domain_\":\"FRAMEWORK\",\"name_\":\"PROCESS_KILL\","
    "\"time_\":4916632892000,\"PROCESS_NAME\":\"\","
    "\"FOREGROUND\":\"1\",\"id_\":\"333351857778095654656\","
    "\"REASON\":\"THREAD_BLOCK_6S\",\"KILL_ID\":2000,\"APP_RUNNING_UNIQUE_ID\":\"6\"}",

    "{\"domain_\":\"FRAMEWORK\",\"name_\":\"PROCESS_KILL\","
    "\"time_\":4916632892000,\"PROCESS_NAME\":\"Js Error unittest\","
    "\"FOREGROUND\":\"1\",\"id_\":\"333351857778095564567\","
    "\"REASON\":\"\",\"APP_RUNNING_UNIQUE_ID\":\"7\"}"
};

static void SetRecords(std::vector<HiSysEventRecord> &events, const std::vector<std::string> &origins)
{
    events.clear();
    for (const auto &origin : origins) {
        HiSysEventRecord record(origin);
        events.emplace_back(record);
    }
}

static std::string MakeKillRecordRaw(const std::string &killIdJson, const std::string &runningId = "1")
{
    return "{\"domain_\":\"FRAMEWORK\",\"name_\":\"PROCESS_KILL\",\"time_\":1502965663170,"
        "\"PROCESS_NAME\":\"hidumper unittest\",\"FOREGROUND\":\"1\",\"id_\":\"14645518577780955344\","
        "\"KILL_ID\":" + killIdJson + ",\"APP_RUNNING_UNIQUE_ID\":\"" + runningId + "\"}";
}

static std::string MakeKillRecord(int64_t killId)
{
    return MakeKillRecordRaw(std::to_string(killId));
}

static std::string MakePkRecord(int64_t killId, const std::string &runningId)
{
    return MakeKillRecordRaw(std::to_string(killId), runningId);
}

static int64_t FindUnmappedKillId()
{
    for (int64_t id = 1; id <= ProcessKillReason::REASON_MAX; ++id) {
        if (ProcessKillReason::GetAppExitReason(static_cast<int>(id)) == INVALID_KILL_ID_STR) {
            return id;
        }
    }
    return 0;
}

static std::vector<std::string> BuildReasons(EventListDumper &dumper, const std::vector<int64_t> &killIds)
{
    std::vector<std::string> origins;
    for (auto killId : killIds) {
        origins.emplace_back(MakeKillRecord(killId));
    }
    SetRecords(dumper.events_, origins);
    auto results = dumper.BuildResults(columnWidths);
    std::vector<std::string> reasons;
    for (const auto &row : results) {
        reasons.emplace_back(row[REASON_COLUMN_INDEX]);
    }
    return reasons;
}

struct PkExtractOutcome {
    EventDumpResult result_ = EventDumpResult::EVENT_DUMP_OK;
    std::unordered_set<std::string> runningIds_;
    std::unordered_set<std::string> faultTypes_;
};

static PkExtractOutcome RunExtractPk(const std::vector<std::string> &origins)
{
    EventQueryParam param = {};
    std::vector<HiSysEventRecord> events;
    SetRecords(events, origins);
    PkExtractOutcome outcome;
    auto dumpEventInfo = std::make_shared<DumpEventInfo>();
    outcome.result_ = dumpEventInfo->ExtractPkRunningIdsAndFaultTypes(events, outcome.runningIds_,
                                                                      outcome.faultTypes_, param);
    return outcome;
}

class EventDumperTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    void SetProcessKillEvents(std::vector<HiSysEventRecord> &events);
    void SetFaultEvents(std::vector<HiSysEventRecord> &events);
    void EventListPreExecute(std::shared_ptr<EventListDumper> &eventListDumper);
};

void EventDumperTest::SetUpTestCase(void)
{
}
void EventDumperTest::TearDownTestCase(void)
{
}
void EventDumperTest::SetUp(void)
{
    columnWidths = {
        {"time", 0},
        {"process_name", 0},
        {"foreground", 0},
        {"reason", 0},
        {"record_id", 0},
    };
}
void EventDumperTest::TearDown(void)
{
}

void EventDumperTest::SetProcessKillEvents(std::vector<HiSysEventRecord> &events)
{
    events.clear();
    for (const auto& origin : ORIGINS) {
        HiSysEventRecord record(origin);
        events.emplace_back(record);
    }
}
void EventDumperTest::SetFaultEvents(std::vector<HiSysEventRecord> &events)
{
    constexpr char origin1[] = "{\"domain_\":\"RELIABILITY\",\"name_\":\"CPP_CRASH\","
        "\"time_\":1502965663170,\"PROCESS_NAME\":\"hidumper1 unittest\","
        "\"FOREGROUND\":\"1\",\"id_\":\"14645518577780955344\","
        "\"REASON\":\"LIFECYCLE_TIMEOUT\",\"LOG_PATH\":\"/proc/cpuinfo\"}";
    HiSysEventRecord record1(origin1);
    events.emplace_back(record1);

    constexpr char origin2[] = "{\"domain_\":\"RELIABILITY\",\"name_\":\"SYS_FREEZE\","
        "\"time_\":1502965663179,\"PROCESS_NAME\":\"hidumper2 unittest\","
        "\"FOREGROUND\":\"1\",\"id_\":\"14645518577780955345\","
        "\"REASON\":\"LIFECYCLE_TIMEOUT\",\"LOG_PATH\":\"/proc/cpuinfo\"}";
    HiSysEventRecord record2(origin2);
    events.emplace_back(record2);
}
void EventDumperTest::EventListPreExecute(std::shared_ptr<EventListDumper> &eventListDumper)
{
    auto parameter = std::make_shared<DumperParameter>();
    auto dumpDatas = std::make_shared<std::vector<std::vector<std::string>>>();
    eventListDumper->PreExecute(parameter, dumpDatas);
}

HWTEST_F(EventDumperTest, DumpEventListSuccess, TestSize.Level1)
{
    EventQueryParam param;
    param.startTime_ = 0;
    param.endTime_ = 0;
    param.queryRule = {
        {"FRAMEWORK", {"PROCESS_KILL"}},
        {"KERNEL_VENDOR", {"PROCESS_KILL"}}
    };
    std::vector<HiSysEventRecord> events;
    std::shared_ptr<DumpEventInfo> dumpEventInfo = std::make_shared<DumpEventInfo>();
    ASSERT_TRUE(dumpEventInfo->DumpEventList(events, param));
}

HWTEST_F(EventDumperTest, DumpEventInfo_ExtractPkRunningId_ByProcessName, TestSize.Level1)
{
    EventQueryParam param;
    std::vector<HiSysEventRecord> events;
    SetProcessKillEvents(events);
    std::unordered_set<std::string> faultEventQuerySet;
    std::unordered_set<std::string> pkRunningIdSet;
    std::shared_ptr<DumpEventInfo> dumpEventInfo = std::make_shared<DumpEventInfo>();
    dumpEventInfo->ExtractPkRunningIdsAndFaultTypes(events, pkRunningIdSet, faultEventQuerySet, param);
    ASSERT_TRUE(pkRunningIdSet.size() == 4);
    param.processName_ = "hidumper";
    pkRunningIdSet.clear();
    dumpEventInfo->ExtractPkRunningIdsAndFaultTypes(events, pkRunningIdSet, faultEventQuerySet, param);
    ASSERT_TRUE(pkRunningIdSet.size() == 2);
}

HWTEST_F(EventDumperTest, DumpEventInfo_ExtractPkRunningId_ByEventId, TestSize.Level1)
{
    EventQueryParam param;
    param.eventId_ = "14645518577780955344";
    std::vector<HiSysEventRecord> events;
    SetProcessKillEvents(events);
    std::unordered_set<std::string> faultEventQuerySet;
    std::unordered_set<std::string> pkRunningIdSet;
    std::shared_ptr<DumpEventInfo> dumpEventInfo = std::make_shared<DumpEventInfo>();
    dumpEventInfo->ExtractPkRunningIdsAndFaultTypes(events, pkRunningIdSet, faultEventQuerySet, param);
    ASSERT_TRUE(pkRunningIdSet.size() == 1);
    param.eventId_ = "1464551857778095534";
    pkRunningIdSet.clear();
    dumpEventInfo->ExtractPkRunningIdsAndFaultTypes(events, pkRunningIdSet, faultEventQuerySet, param);
    ASSERT_TRUE(pkRunningIdSet.size() == 2);
}

HWTEST_F(EventDumperTest, DumpEventInfo_ExtractPkRunningId_NONEDATA, TestSize.Level1)
{
    EventQueryParam param;
    param.eventId_ = "3333518577780955378";
    std::vector<HiSysEventRecord> events;
    SetProcessKillEvents(events);
    std::unordered_set<std::string> faultEventQuerySet;
    std::unordered_set<std::string> pkRunningIdSet;
    std::shared_ptr<DumpEventInfo> dumpEventInfo = std::make_shared<DumpEventInfo>();
    auto result = dumpEventInfo->ExtractPkRunningIdsAndFaultTypes(events, pkRunningIdSet, faultEventQuerySet, param);
    ASSERT_TRUE(result == EventDumpResult::NOT_FAULT_EVENT);
    param.eventId_ = "3333518577780955379";
    pkRunningIdSet.clear();
    result = dumpEventInfo->ExtractPkRunningIdsAndFaultTypes(events, pkRunningIdSet, faultEventQuerySet, param);
    ASSERT_TRUE(result == EventDumpResult::NONE_PROCESSKILL_EVENT);
}

HWTEST_F(EventDumperTest, DumpEventInfo_ExtractPkRunningId_ByKillIdNotReason, TestSize.Level1)
{
    constexpr char driftedReason[] = "{\"domain_\":\"FRAMEWORK\",\"name_\":\"PROCESS_KILL\","
        "\"time_\":1502965663180,\"PROCESS_NAME\":\"hidumper3 unittest\","
        "\"FOREGROUND\":\"1\",\"id_\":\"14645518577780955346\","
        "\"REASON\":\"Kill Reason:app exit\",\"KILL_ID\":2003,\"APP_RUNNING_UNIQUE_ID\":\"8\"}";
    constexpr char withoutKillId[] = "{\"domain_\":\"FRAMEWORK\",\"name_\":\"PROCESS_KILL\","
        "\"time_\":1502965663181,\"PROCESS_NAME\":\"hidumper4 unittest\","
        "\"FOREGROUND\":\"1\",\"id_\":\"14645518577780955347\","
        "\"REASON\":\"Js Error\",\"APP_RUNNING_UNIQUE_ID\":\"9\"}";

    auto outcome = RunExtractPk({ driftedReason, withoutKillId });
    ASSERT_TRUE(outcome.result_ == EventDumpResult::EVENT_DUMP_OK);
    ASSERT_TRUE(outcome.runningIds_.size() == 1);
    ASSERT_TRUE(outcome.runningIds_.count("8") == 1);
    ASSERT_TRUE(outcome.faultTypes_.size() == 1);
    ASSERT_TRUE(outcome.faultTypes_.count("JS_ERROR") == 1);
}

HWTEST_F(EventDumperTest, DumpEventInfo_ExtractPkRunningId_OutOfRangeKillId, TestSize.Level1)
{
    auto outcome = RunExtractPk({ MakeKillRecordRaw(std::to_string(KILL_ID_TRUNCATED_TO_JS_ERROR), "14") });
    ASSERT_TRUE(outcome.result_ == EventDumpResult::NONE_PROCESSKILL_EVENT);
    ASSERT_TRUE(outcome.runningIds_.empty());
    ASSERT_TRUE(outcome.faultTypes_.empty());
}

HWTEST_F(EventDumperTest, DumpEventInfo_ExtractPkRunningId_MissingProcessName, TestSize.Level1)
{
    constexpr char noProcessName[] = "{\"domain_\":\"FRAMEWORK\",\"name_\":\"PROCESS_KILL\","
        "\"time_\":1502965663183,\"FOREGROUND\":\"1\",\"id_\":\"14645518577780955353\","
        "\"REASON\":\"Js Error\",\"KILL_ID\":2003,\"APP_RUNNING_UNIQUE_ID\":\"15\"}";

    auto outcome = RunExtractPk({ noProcessName });
    ASSERT_TRUE(outcome.result_ == EventDumpResult::NONE_PROCESSKILL_EVENT);
    ASSERT_TRUE(outcome.runningIds_.empty());
    ASSERT_TRUE(outcome.faultTypes_.empty());
}

HWTEST_F(EventDumperTest, DumpEventInfo_ExtractAllFaultTypes, TestSize.Level1)
{
    auto outcome = RunExtractPk({
        MakePkRecord(ProcessKillReason::REASON_THREAD_BLOCK_6S, "101"),
        MakePkRecord(ProcessKillReason::REASON_APP_INPUT_BLOCK, "102"),
        MakePkRecord(ProcessKillReason::REASON_LIFECYCLE_TIMEOUT, "103"),
        MakePkRecord(ProcessKillReason::REASON_JS_ERROR, "104"),
        MakePkRecord(ProcessKillReason::REASON_CPP_CRASH, "105"),
    });
    ASSERT_TRUE(outcome.result_ == EventDumpResult::EVENT_DUMP_OK);
    ASSERT_EQ(outcome.runningIds_.size(), FAULT_TYPE_CASE_COUNT);
    ASSERT_EQ(outcome.faultTypes_.size(), FAULT_TYPE_KIND_COUNT);
    ASSERT_TRUE(outcome.faultTypes_.count("APP_FREEZE") == 1);
    ASSERT_TRUE(outcome.faultTypes_.count("SYS_FREEZE") == 1);
    ASSERT_TRUE(outcome.faultTypes_.count("JS_ERROR") == 1);
    ASSERT_TRUE(outcome.faultTypes_.count("CPP_CRASH") == 1);
}

HWTEST_F(EventDumperTest, DumpEventInfo_FillQueryParam, TestSize.Level1)
{
    EventQueryParam param;
    std::unordered_set<std::string> faultEventQuerySet = {
        "CPP_CRASH",
        "JS_ERROR",
        "SYS_FREEZE",
        "APP_FREEZE",
    };
    std::shared_ptr<DumpEventInfo> dumpEventInfo = std::make_shared<DumpEventInfo>();
    dumpEventInfo->FillQueryParam(param, faultEventQuerySet);
    ASSERT_TRUE(param.queryRule.size() == 5);
}

HWTEST_F(EventDumperTest, DumpEventInfo_FillQueryParamPartialSet, TestSize.Level1)
{
    EventQueryParam param;
    param.startTime_ = QUERY_START_TIME;
    param.endTime_ = QUERY_END_TIME;
    param.queryRule = {{"FRAMEWORK", {"PROCESS_KILL"}}};
    std::shared_ptr<DumpEventInfo> dumpEventInfo = std::make_shared<DumpEventInfo>();

    std::unordered_set<std::string> emptySet;
    dumpEventInfo->FillQueryParam(param, emptySet);
    ASSERT_TRUE(param.queryRule.empty());
    ASSERT_TRUE(param.startTime_ == 0);
    ASSERT_TRUE(param.endTime_ == 0);

    dumpEventInfo->FillQueryParam(param, {"CPP_CRASH"});
    ASSERT_TRUE(param.queryRule.size() == SINGLE_FAULT_RULE_COUNT);

    dumpEventInfo->FillQueryParam(param, {"SYS_FREEZE"});
    ASSERT_TRUE(param.queryRule.size() == SINGLE_FAULT_RULE_COUNT);

    dumpEventInfo->FillQueryParam(param, {"APP_FREEZE"});
    ASSERT_TRUE(param.queryRule.size() == SINGLE_FAULT_RULE_COUNT);

    dumpEventInfo->FillQueryParam(param, {"JS_ERROR"});
    ASSERT_TRUE(param.queryRule.size() == JS_ERROR_RULE_COUNT);
}

HWTEST_F(EventDumperTest, DumpFaultEventListByPKSuccess, TestSize.Level1)
{
    EventQueryParam param;
    param.startTime_ = 0;
    param.endTime_ = 0;
    std::vector<HiSysEventRecord> events;
    std::shared_ptr<DumpEventInfo> dumpEventInfo = std::make_shared<DumpEventInfo>();
    auto result = dumpEventInfo->DumpFaultEventListByPK(events, param);
    ASSERT_TRUE(result != EventDumpResult::EVENT_DUMP_FAIL);
}

HWTEST_F(EventDumperTest, EventListDumperSuccess, TestSize.Level1)
{
    auto parameter = std::make_shared<DumperParameter>();
    auto dumpDatas = std::make_shared<std::vector<std::vector<std::string>>>();
    std::shared_ptr<EventListDumper> eventListDumper = std::make_shared<EventListDumper>();
    EventListPreExecute(eventListDumper);
    SetProcessKillEvents(eventListDumper->events_);
    DumpStatus ret = DumpStatus::DUMP_FAIL;
    ret = eventListDumper->PreExecute(parameter, dumpDatas);
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
    ret = eventListDumper->Execute();
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
    ret = eventListDumper->AfterExecute();
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
    ASSERT_EQ(eventListDumper->events_[0].GetTime(), 4916632892000);
}

HWTEST_F(EventDumperTest, EventListDumper_CheckData, TestSize.Level1)
{
    std::shared_ptr<EventListDumper> eventListDumper = std::make_shared<EventListDumper>();
    EventListPreExecute(eventListDumper);
    SetProcessKillEvents(eventListDumper->events_);

    auto results = eventListDumper->BuildResults(columnWidths);
    ASSERT_EQ(results.size(), RECORD_COUNT_WITH_VALID_KILL_ID);
    ASSERT_EQ(results[0][REASON_COLUMN_INDEX], "LifecycleTimeout");
}

HWTEST_F(EventDumperTest, EventListDumper_FilterData, TestSize.Level1)
{
    std::shared_ptr<EventListDumper> eventListDumper = std::make_shared<EventListDumper>();
    EventListPreExecute(eventListDumper);
    SetProcessKillEvents(eventListDumper->events_);

    eventListDumper->processName_ = "hidumper1";
    auto results = eventListDumper->BuildResults(columnWidths);
    ASSERT_TRUE(results.size() == 1);

    eventListDumper->processName_ = "hidumper";
    results = eventListDumper->BuildResults(columnWidths);
    ASSERT_TRUE(results.size() == 2);
}

HWTEST_F(EventDumperTest, EventListDumper_LimitEventCount, TestSize.Level1)
{
    std::shared_ptr<EventListDumper> eventListDumper = std::make_shared<EventListDumper>();
    EventListPreExecute(eventListDumper);
    SetProcessKillEvents(eventListDumper->events_);

    eventListDumper->showEventCount_ = 1;
    auto results = eventListDumper->BuildResults(columnWidths);
    ASSERT_TRUE(results.size() == 1);
}

HWTEST_F(EventDumperTest, EventListDumper_ReasonByKillId, TestSize.Level1)
{
    std::shared_ptr<EventListDumper> eventListDumper = std::make_shared<EventListDumper>();
    EventListPreExecute(eventListDumper);

    auto reasons = BuildReasons(*eventListDumper, {
        ProcessKillReason::REASON_THREAD_BLOCK_6S,
        ProcessKillReason::REASON_LIFECYCLE_TIMEOUT,
        ProcessKillReason::REASON_JS_ERROR,
        ProcessKillReason::REASON_CPP_CRASH,
        ProcessKillReason::REASON_SWAP_FULL,
        ProcessKillReason::REASON_OOM_KILLER,
        ProcessKillReason::REASON_MEMORY_PRESSURE,
        ProcessKillReason::REASON_CPA_KILLER,
        ProcessKillReason::REASON_UNINSTALL_APP,
        ProcessKillReason::REASON_RESOURCE_LEAK_FD_LEAK,
        ProcessKillReason::REASON_TEMPERATURE_CONTROL,
        ProcessKillReason::REASON_STORAGE_CARD_UNINSTALL,
    });
    ASSERT_EQ(reasons.size(), REASON_CASE_COUNT);
    ASSERT_EQ(reasons[0], "ThreadBlock6S");
    ASSERT_EQ(reasons[1], "LifecycleTimeout");
    ASSERT_EQ(reasons[2], "JsError");
    ASSERT_EQ(reasons[3], "CppCrash");
    ASSERT_EQ(reasons[4], "SwapFull");
    ASSERT_EQ(reasons[5], "OomKiller");
    ASSERT_EQ(reasons[6], "LowMemoryKill");
    ASSERT_EQ(reasons[7], "CpaKiller");
    ASSERT_EQ(reasons[8], "Uninstall");
    ASSERT_EQ(reasons[9], "ResourceLeak(FDLeak)");
    ASSERT_EQ(reasons[10], "HighTemperature");
    ASSERT_EQ(reasons[11], "UninstallStorage");
}

HWTEST_F(EventDumperTest, EventListDumper_ReasonAtSegmentBoundary, TestSize.Level1)
{
    std::shared_ptr<EventListDumper> eventListDumper = std::make_shared<EventListDumper>();
    EventListPreExecute(eventListDumper);

    auto reasons = BuildReasons(*eventListDumper, {
        ProcessKillReason::REASON_APP_EXIT_MIN,
        ProcessKillReason::REASON_KILL_CGROUP,
        ProcessKillReason::REASON_USER_EXIT_MIN,
        ProcessKillReason::REASON_AA_FORCE_STOP,
        ProcessKillReason::REASON_CRASH_EXIT_MIN,
        ProcessKillReason::REASON_RS_TRANSACTION_DATA_OVERLIMIT,
        ProcessKillReason::REASON_SYS_EXIT_MIN,
        ProcessKillReason::REASON_KERNEL_EXIT_MIN,
        ProcessKillReason::REASON_MAX,
    });
    ASSERT_EQ(reasons.size(), SEGMENT_BOUNDARY_COUNT);
    ASSERT_EQ(reasons[0], "KillApplication");
    ASSERT_EQ(reasons[1], "KillApplication");
    ASSERT_EQ(reasons[2], "UserRequest");
    ASSERT_EQ(reasons[3], "aaForceStop");
    ASSERT_EQ(reasons[4], "ThreadBlock6S");
    ASSERT_EQ(reasons[5], "RsDataOverflow");
    ASSERT_EQ(reasons[6], "PowerSaveClean");
    ASSERT_EQ(reasons[7], "OomKiller");
    ASSERT_EQ(reasons[8], "UninstallStorage");
}

HWTEST_F(EventDumperTest, EventListDumper_ManyKillIdsShareOneReason, TestSize.Level1)
{
    std::shared_ptr<EventListDumper> eventListDumper = std::make_shared<EventListDumper>();
    EventListPreExecute(eventListDumper);

    auto reasons = BuildReasons(*eventListDumper, {
        ProcessKillReason::REASON_KILL_APPLICATION_BY_BUNDLE_NAME,
        ProcessKillReason::REASON_APP_EXIT,
        ProcessKillReason::REASON_KILL_APPLICATION_SELF,
        ProcessKillReason::REASON_KILL_CGROUP,
        ProcessKillReason::REASON_USER_REQUEST,
        ProcessKillReason::REASON_CLEAR_SESSION,
        ProcessKillReason::REASON_USER_LOGOUT,
        ProcessKillReason::REASON_USER_STOP,
    });
    ASSERT_EQ(reasons.size(), SHARE_REASON_CASE_COUNT);
    for (size_t i = 0; i < KILL_APPLICATION_GROUP_COUNT; ++i) {
        ASSERT_EQ(reasons[i], "KillApplication");
    }
    ASSERT_EQ(reasons[KILL_APPLICATION_GROUP_COUNT], "UserRequest");
    ASSERT_EQ(reasons[KILL_APPLICATION_GROUP_COUNT + 1], "UserRequest");
    ASSERT_EQ(reasons[KILL_APPLICATION_GROUP_COUNT + 2], "Logout");
    ASSERT_EQ(reasons[KILL_APPLICATION_GROUP_COUNT + 3], "Logout");
}

HWTEST_F(EventDumperTest, EventListDumper_AppReasonDiffersFromFaultReason, TestSize.Level1)
{
    std::shared_ptr<EventListDumper> eventListDumper = std::make_shared<EventListDumper>();
    EventListPreExecute(eventListDumper);

    auto reasons = BuildReasons(*eventListDumper, { ProcessKillReason::REASON_JS_ERROR });
    ASSERT_TRUE(reasons.size() == 1);
    ASSERT_EQ(reasons[0], "JsError");
    ASSERT_NE(ProcessKillReason::GetAppExitReason(ProcessKillReason::REASON_JS_ERROR),
              ProcessKillReason::GetKillReason(ProcessKillReason::REASON_JS_ERROR));

    auto outcome = RunExtractPk({ MakePkRecord(ProcessKillReason::REASON_JS_ERROR, "201") });
    ASSERT_TRUE(outcome.result_ == EventDumpResult::EVENT_DUMP_OK);
    ASSERT_TRUE(outcome.faultTypes_.count("JS_ERROR") == 1);
}

HWTEST_F(EventDumperTest, EventListDumper_KillIdLenientJsonTypes, TestSize.Level1)
{
    std::shared_ptr<EventListDumper> eventListDumper = std::make_shared<EventListDumper>();
    EventListPreExecute(eventListDumper);

    SetRecords(eventListDumper->events_, { MakeKillRecordRaw("null") });
    ASSERT_TRUE(eventListDumper->BuildResults(columnWidths).empty());

    SetRecords(eventListDumper->events_, { MakeKillRecordRaw("true") });
    auto results = eventListDumper->BuildResults(columnWidths);
    ASSERT_TRUE(results.size() == 1);
    ASSERT_EQ(results[0][REASON_COLUMN_INDEX], "KillApplication");

    SetRecords(eventListDumper->events_, { MakeKillRecordRaw("false") });
    ASSERT_TRUE(eventListDumper->BuildResults(columnWidths).empty());

    SetRecords(eventListDumper->events_, { MakeKillRecordRaw("2002.0") });
    results = eventListDumper->BuildResults(columnWidths);
    ASSERT_TRUE(results.size() == 1);
    ASSERT_EQ(results[0][REASON_COLUMN_INDEX], "LifecycleTimeout");

    SetRecords(eventListDumper->events_, { MakeKillRecordRaw("2002.5") });
    ASSERT_TRUE(eventListDumper->BuildResults(columnWidths).empty());
}

HWTEST_F(EventDumperTest, EventListDumper_InvalidKillIdIsFiltered, TestSize.Level1)
{
    std::shared_ptr<EventListDumper> eventListDumper = std::make_shared<EventListDumper>();
    EventListPreExecute(eventListDumper);
    std::vector<int64_t> invalidIds = { 0, -1, INT32_MAX, INT32_MIN };
    int64_t inRangeHole = FindUnmappedKillId();
    if (inRangeHole > 0) {
        invalidIds.emplace_back(inRangeHole);
    }
    ASSERT_TRUE(BuildReasons(*eventListDumper, invalidIds).empty());

    SetProcessKillEvents(eventListDumper->events_);
    auto results = eventListDumper->BuildResults(columnWidths);
    ASSERT_EQ(results.size(), RECORD_COUNT_WITH_VALID_KILL_ID);
    for (const auto &row : results) {
        ASSERT_EQ(row.size(), columnWidths.size());
        ASSERT_FALSE(row[REASON_COLUMN_INDEX].empty());
        ASSERT_TRUE(row[REASON_COLUMN_INDEX].find(INVALID_KILL_ID_STR) == std::string::npos);
    }
}

HWTEST_F(EventDumperTest, EventListDumper_ExecuteWithPresetEvents, TestSize.Level1)
{
    auto parameter = std::make_shared<DumperParameter>();
    auto dumpDatas = std::make_shared<std::vector<std::vector<std::string>>>();
    std::shared_ptr<EventListDumper> eventListDumper = std::make_shared<EventListDumper>();
    ASSERT_EQ(eventListDumper->PreExecute(parameter, dumpDatas), DumpStatus::DUMP_OK);
    SetProcessKillEvents(eventListDumper->events_);

    ASSERT_EQ(eventListDumper->Execute(), DumpStatus::DUMP_OK);
    ASSERT_GE(dumpDatas->size(), RECORD_COUNT_WITH_VALID_KILL_ID + 1);
    for (const auto &line : *dumpDatas) {
        ASSERT_EQ(line.size(), LINE_COUNT_OF_ONE_ROW);
        ASSERT_TRUE(line[0].find(INVALID_KILL_ID_STR) == std::string::npos);
    }
}

HWTEST_F(EventDumperTest, EventListDumper_ShowCountLargerThanResults, TestSize.Level1)
{
    std::shared_ptr<EventListDumper> eventListDumper = std::make_shared<EventListDumper>();
    EventListPreExecute(eventListDumper);
    SetProcessKillEvents(eventListDumper->events_);

    eventListDumper->showEventCount_ = SHOW_COUNT_NOT_TRUNCATE;
    auto results = eventListDumper->BuildResults(columnWidths);
    ASSERT_EQ(results.size(), RECORD_COUNT_WITH_VALID_KILL_ID);
}

HWTEST_F(EventDumperTest, EventListDumper_MissingProcessNameIsSkipped, TestSize.Level1)
{
    constexpr char noProcessName[] = "{\"domain_\":\"FRAMEWORK\",\"name_\":\"PROCESS_KILL\","
        "\"time_\":1502965663170,\"FOREGROUND\":\"1\",\"id_\":\"14645518577780955344\","
        "\"KILL_ID\":2002,\"APP_RUNNING_UNIQUE_ID\":\"1\"}";
    std::shared_ptr<EventListDumper> eventListDumper = std::make_shared<EventListDumper>();
    EventListPreExecute(eventListDumper);
    SetRecords(eventListDumper->events_, { noProcessName });

    auto results = eventListDumper->BuildResults(columnWidths);
    ASSERT_TRUE(results.empty());
}

HWTEST_F(EventDumperTest, EventListDumper_KillIdTypeMismatchIsSkipped, TestSize.Level1)
{
    constexpr char killIdArray[] = "{\"domain_\":\"FRAMEWORK\",\"name_\":\"PROCESS_KILL\","
        "\"time_\":1502965663170,\"PROCESS_NAME\":\"hidumper5 unittest\",\"FOREGROUND\":\"1\","
        "\"id_\":\"14645518577780955348\",\"KILL_ID\":[2002],\"APP_RUNNING_UNIQUE_ID\":\"10\"}";
    std::shared_ptr<EventListDumper> eventListDumper = std::make_shared<EventListDumper>();
    EventListPreExecute(eventListDumper);
    SetRecords(eventListDumper->events_, { killIdArray });

    auto results = eventListDumper->BuildResults(columnWidths);
    ASSERT_TRUE(results.empty());
}

HWTEST_F(EventDumperTest, EventListDumper_KillIdAsStringIsRejected, TestSize.Level1)
{
    constexpr char killIdStr[] = "{\"domain_\":\"FRAMEWORK\",\"name_\":\"PROCESS_KILL\","
        "\"time_\":1502965663170,\"PROCESS_NAME\":\"hidumper6 unittest\",\"FOREGROUND\":\"1\","
        "\"id_\":\"14645518577780955349\",\"KILL_ID\":\"2002\",\"APP_RUNNING_UNIQUE_ID\":\"11\"}";
    std::shared_ptr<EventListDumper> eventListDumper = std::make_shared<EventListDumper>();
    EventListPreExecute(eventListDumper);
    SetRecords(eventListDumper->events_, { killIdStr });

    auto results = eventListDumper->BuildResults(columnWidths);
    ASSERT_TRUE(results.empty());
}

HWTEST_F(EventDumperTest, EventListDumper_KillIdOutOfInt32IsSkipped, TestSize.Level1)
{
    std::shared_ptr<EventListDumper> eventListDumper = std::make_shared<EventListDumper>();
    EventListPreExecute(eventListDumper);
    SetRecords(eventListDumper->events_, {
        MakeKillRecord(KILL_ID_TRUNCATED_TO_JS_ERROR),
        MakeKillRecord(-KILL_ID_TRUNCATED_TO_JS_ERROR),
        MakeKillRecord(INT64_MAX),
        MakeKillRecord(INT64_MIN),
        MakeKillRecord(ProcessKillReason::REASON_JS_ERROR),
    });

    auto results = eventListDumper->BuildResults(columnWidths);
    ASSERT_TRUE(results.size() == 1);
    ASSERT_EQ(results[0][REASON_COLUMN_INDEX], "JsError");
}

HWTEST_F(EventDumperTest, EventListDumper_ForegroundFalseAndNullColumn, TestSize.Level1)
{
    constexpr char foregroundZero[] = "{\"domain_\":\"FRAMEWORK\",\"name_\":\"PROCESS_KILL\","
        "\"time_\":1502965663170,\"PROCESS_NAME\":\"hidumper7 unittest\",\"FOREGROUND\":\"0\","
        "\"id_\":\"14645518577780955350\",\"KILL_ID\":2002,\"APP_RUNNING_UNIQUE_ID\":\"12\"}";
    constexpr char noForeground[] = "{\"domain_\":\"FRAMEWORK\",\"name_\":\"PROCESS_KILL\","
        "\"time_\":1502965663175,\"PROCESS_NAME\":\"hidumper8 unittest\","
        "\"id_\":\"14645518577780955351\",\"KILL_ID\":2004,\"APP_RUNNING_UNIQUE_ID\":\"13\"}";
    std::shared_ptr<EventListDumper> eventListDumper = std::make_shared<EventListDumper>();
    EventListPreExecute(eventListDumper);
    SetRecords(eventListDumper->events_, { foregroundZero, noForeground });

    auto results = eventListDumper->BuildResults(columnWidths);
    ASSERT_TRUE(results.size() == 2);
    ASSERT_EQ(results[0][FOREGROUND_COLUMN_INDEX], "False");
    ASSERT_EQ(results[1][FOREGROUND_COLUMN_INDEX], "Null");
    ASSERT_EQ(results[1][REASON_COLUMN_INDEX], "CppCrash");
}

HWTEST_F(EventDumperTest, EventListDumper_FormatResultsClampToMaxWidth, TestSize.Level1)
{
    std::shared_ptr<EventListDumper> eventListDumper = std::make_shared<EventListDumper>();
    EventListPreExecute(eventListDumper);
    std::unordered_map<std::string, int> wideColumn = {
        {"time", MAX_COLUMN_WIDTH + OVER_WIDTH_DELTA},
        {"foreground", 1},
        {"reason", 1},
        {"record_id", 1},
        {"process_name", 1},
    };
    std::vector<std::vector<std::string>> results = { { "a", "b", "c", "d", "e" } };

    eventListDumper->FormatResults(results, wideColumn);
    ASSERT_TRUE(eventListDumper->dumpDatas_->size() == 1);
    ASSERT_EQ((*eventListDumper->dumpDatas_)[0][0].size(), CLAMPED_LINE_LEN);
}

HWTEST_F(EventDumperTest, EventDetailDumperSuccess, TestSize.Level1)
{
    auto parameter = std::make_shared<DumperParameter>();
    auto dumpDatas = std::make_shared<std::vector<std::vector<std::string>>>();
    std::shared_ptr<EventDetailDumper> eventDetailDumper = std::make_shared<EventDetailDumper>();
    SetFaultEvents(eventDetailDumper->events_);
    DumpStatus ret = DumpStatus::DUMP_FAIL;
    ret = eventDetailDumper->PreExecute(parameter, dumpDatas);
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
    ret = eventDetailDumper->Execute();
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
    ret = eventDetailDumper->AfterExecute();
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
}

HWTEST_F(EventDumperTest, EventDetailDumper_LimitEventCount, TestSize.Level1)
{
    std::shared_ptr<EventDetailDumper> eventDetailDumper = std::make_shared<EventDetailDumper>();
    SetFaultEvents(eventDetailDumper->events_);

    auto logPaths = eventDetailDumper->FilterLogPaths();
    ASSERT_TRUE(logPaths.size() == 2);
    eventDetailDumper->showEventCount_ = 1;
    logPaths = eventDetailDumper->FilterLogPaths();
    ASSERT_TRUE(logPaths.size() == 1);
}

HWTEST_F(EventDumperTest, EventDetailDumper_ReadLogsByPaths, TestSize.Level1)
{
    auto parameter = std::make_shared<DumperParameter>();
    auto dumpDatas = std::make_shared<std::vector<std::vector<std::string>>>();
    std::shared_ptr<EventDetailDumper> eventDetailDumper = std::make_shared<EventDetailDumper>();
    SetFaultEvents(eventDetailDumper->events_);
    eventDetailDumper->PreExecute(parameter, dumpDatas);

    std::vector<std::string> logPaths;
    logPaths.emplace_back("/system/etc/init/hidumper_service.cfg");
    eventDetailDumper->ReadLogsByPaths(logPaths);
    ASSERT_TRUE(!eventDetailDumper->dumpDatas_->empty());
}
} // namespace HiviewDFX
} // namespace OHOS