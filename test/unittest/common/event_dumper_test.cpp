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

#include "executor/event_detail_dumper.h"
#include "executor/event_list_dumper.h"


using namespace testing::ext;
using namespace std;
namespace OHOS {
namespace HiviewDFX {
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
    "\"REASON\":\"LIFECYCLE_TIMEOUT\",\"APP_RUNNING_UNIQUE_ID\":\"1\"}",

    "{\"domain_\":\"FRAMEWORK\",\"name_\":\"PROCESS_KILL\","
    "\"time_\":1502965663175,\"PROCESS_NAME\":\"hidumper2 unittest\","
    "\"FOREGROUND\":\"1\",\"id_\":\"14645518577780955345\","
    "\"REASON\":\"Cpp Crash\",\"APP_RUNNING_UNIQUE_ID\":\"2\"}",

    "{\"domain_\":\"FRAMEWORK\",\"name_\":\"PROCESS_KILL\","
    "\"time_\":1502965663178,\"PROCESS_NAME\":\"Js Error unittest\","
    "\"FOREGROUND\":\"1\",\"REASON\":\"Js Error\","
    "\"APP_RUNNING_UNIQUE_ID\":\"3\"}",

    "{\"domain_\":\"FRAMEWORK\",\"name_\":\"PROCESS_KILL\","
    "\"time_\":4916632891988,\"PROCESS_NAME\":\"Js Error unittest\","
    "\"FOREGROUND\":\"1\",\"id_\":\"3333518577780955356\","
    "\"REASON\":\"THREAD_BLOCK_6S\",\"APP_RUNNING_UNIQUE_ID\":\"4\"}",

    "{\"domain_\":\"FRAMEWORK\",\"name_\":\"PROCESS_KILL\","
    "\"time_\":4916632892000,\"PROCESS_NAME\":\"Js Error unittest\","
    "\"FOREGROUND\":\"1\",\"id_\":\"3333518577780955378\","
    "\"REASON\":\"test5\",\"APP_RUNNING_UNIQUE_ID\":\"5\"}",

    "{\"domain_\":\"FRAMEWORK\",\"name_\":\"PROCESS_KILL\","
    "\"time_\":4916632892000,\"PROCESS_NAME\":\"\","
    "\"FOREGROUND\":\"1\",\"id_\":\"333351857778095654656\","
    "\"REASON\":\"THREAD_BLOCK_6S\",\"APP_RUNNING_UNIQUE_ID\":\"6\"}",

    "{\"domain_\":\"FRAMEWORK\",\"name_\":\"PROCESS_KILL\","
    "\"time_\":4916632892000,\"PROCESS_NAME\":\"Js Error unittest\","
    "\"FOREGROUND\":\"1\",\"id_\":\"333351857778095564567\","
    "\"REASON\":\"\",\"APP_RUNNING_UNIQUE_ID\":\"7\"}"
};

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
    ASSERT_TRUE(results.size() == 4);
    ASSERT_TRUE(results[0][2] == "LifecycleTimeout");
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
    logPaths.emplace_back("/system/etc/hidumper/event_reason_config.json");
    eventDetailDumper->ReadLogsByPaths(logPaths);
    ASSERT_TRUE(!eventDetailDumper->dumpDatas_->empty());
}
} // namespace HiviewDFX
} // namespace OHOS