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

class EventDumperTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    void SetEventListDumperTestData(std::shared_ptr<EventListDumper> &eventListDumper);
    void SetEventDetailDumperTestData(std::shared_ptr<EventDetailDumper> &EventDetailDumper);
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

void EventDumperTest::SetEventListDumperTestData(std::shared_ptr<EventListDumper> &eventListDumper)
{
    constexpr char origin1[] = "{\"domain_\":\"FRAMEWORK\",\"name_\":\"PROCESS_KILL\","
        "\"time_\":1502965663170,\"PROCESS_NAME\":\"hidumper1 unittest\","
        "\"FOREGROUND\":\"1\",\"id_\":\"14645518577780955344\","
        "\"REASON\":\"LIFECYCLE_TIMEOUT\"}";
    HiSysEventRecord record1(origin1);
    eventListDumper->events_.emplace_back(record1);
    constexpr char origin2[] = "{\"domain_\":\"FRAMEWORK\",\"name_\":\"PROCESS_KILL\","
        "\"time_\":1502965663170,\"PROCESS_NAME\":\"hidumper2 unittest\","
        "\"FOREGROUND\":\"1\",\"id_\":\"14645518577780955345\","
        "\"REASON\":\"LIFECYCLE_TIMEOUT\"}";
    HiSysEventRecord record2(origin2);
    eventListDumper->events_.emplace_back(record2);
    auto parameter = std::make_shared<DumperParameter>();
    auto dumpDatas = std::make_shared<std::vector<std::vector<std::string>>>();
    eventListDumper->PreExecute(parameter, dumpDatas);
}

void EventDumperTest::SetEventDetailDumperTestData(std::shared_ptr<EventDetailDumper> &eventDetailDumper)
{
    constexpr char origin1[] = "{\"domain_\":\"RELIABILITY\",\"name_\":\"CPP_CRASH\","
        "\"time_\":1502965663170,\"PROCESS_NAME\":\"hidumper1 unittest\","
        "\"FOREGROUND\":\"1\",\"id_\":\"14645518577780955344\","
        "\"REASON\":\"LIFECYCLE_TIMEOUT\",\"LOG_PATH\":\"/proc/cpuinfo\"}";
    HiSysEventRecord record1(origin1);
    eventDetailDumper->events_.emplace_back(record1);
        constexpr char origin2[] = "{\"domain_\":\"RELIABILITY\",\"name_\":\"CPP_CRASH\","
        "\"time_\":1502965663170,\"PROCESS_NAME\":\"hidumper2 unittest\","
        "\"FOREGROUND\":\"1\",\"id_\":\"14645518577780955345\","
        "\"REASON\":\"LIFECYCLE_TIMEOUT\",\"LOG_PATH\":\"/proc/cpuinfo\"}";
    HiSysEventRecord record2(origin2);
    eventDetailDumper->events_.emplace_back(record2);
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

HWTEST_F(EventDumperTest, DumpFaultEventListByPKSuccess, TestSize.Level1)
{
    EventQueryParam param;
    param.startTime_ = 0;
    param.endTime_ = 0;
    std::vector<HiSysEventRecord> events;
    std::shared_ptr<DumpEventInfo> dumpEventInfo = std::make_shared<DumpEventInfo>();
    ASSERT_TRUE(dumpEventInfo->DumpFaultEventListByPK(events, param));
}

HWTEST_F(EventDumperTest, EventListDumperSuccess, TestSize.Level1)
{
    auto parameter = std::make_shared<DumperParameter>();
    auto dumpDatas = std::make_shared<std::vector<std::vector<std::string>>>();
    std::shared_ptr<EventListDumper> eventListDumper = std::make_shared<EventListDumper>();
    SetEventListDumperTestData(eventListDumper);
    DumpStatus ret = DumpStatus::DUMP_FAIL;
    ret = eventListDumper->PreExecute(parameter, dumpDatas);
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
    ret = eventListDumper->Execute();
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
    ret = eventListDumper->AfterExecute();
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
}

HWTEST_F(EventDumperTest, EventListDumper_CheckData, TestSize.Level1)
{
    std::shared_ptr<EventListDumper> eventListDumper = std::make_shared<EventListDumper>();
    SetEventListDumperTestData(eventListDumper);

    auto results = eventListDumper->BuildResults(columnWidths);
    ASSERT_TRUE(results.size() == 2);
    ASSERT_TRUE(results[1][3] == "LifecycleTimeout");
}

HWTEST_F(EventDumperTest, EventListDumper_FilterData, TestSize.Level1)
{
    std::shared_ptr<EventListDumper> eventListDumper = std::make_shared<EventListDumper>();
    SetEventListDumperTestData(eventListDumper);

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
    SetEventListDumperTestData(eventListDumper);

    eventListDumper->showEventCount_ = 1;
    auto results = eventListDumper->BuildResults(columnWidths);
    ASSERT_TRUE(results.size() == 1);
}

HWTEST_F(EventDumperTest, EventDetailDumperSuccess, TestSize.Level1)
{
    auto parameter = std::make_shared<DumperParameter>();
    auto dumpDatas = std::make_shared<std::vector<std::vector<std::string>>>();
    std::shared_ptr<EventDetailDumper> eventDetailDumper = std::make_shared<EventDetailDumper>();
    SetEventDetailDumperTestData(eventDetailDumper);
    DumpStatus ret = DumpStatus::DUMP_FAIL;
    ret = eventDetailDumper->PreExecute(parameter, dumpDatas);
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
    ret = eventDetailDumper->Execute();
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
    ret = eventDetailDumper->AfterExecute();
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
}

HWTEST_F(EventDumperTest, EventDetailDumper_FilterDataByProcessName, TestSize.Level1)
{
    std::shared_ptr<EventDetailDumper> eventDetailDumper = std::make_shared<EventDetailDumper>();
    SetEventDetailDumperTestData(eventDetailDumper);

    auto logPaths = eventDetailDumper->FilterLogPaths();
    ASSERT_TRUE(logPaths.size() == 2);

    eventDetailDumper->processName_ = "hidumper1";
    logPaths = eventDetailDumper->FilterLogPaths();
    ASSERT_TRUE(logPaths.size() == 1);

    eventDetailDumper->processName_ = "hidumper";
    logPaths = eventDetailDumper->FilterLogPaths();
    ASSERT_TRUE(logPaths.size() == 2);
}

HWTEST_F(EventDumperTest, EventDetailDumper_FilterDataById, TestSize.Level1)
{
    std::shared_ptr<EventDetailDumper> eventDetailDumper = std::make_shared<EventDetailDumper>();
    SetEventDetailDumperTestData(eventDetailDumper);

    auto logPaths = eventDetailDumper->FilterLogPaths();
    ASSERT_TRUE(logPaths.size() == 2);

    eventDetailDumper->eventId_ = "14645518577780955344";
    logPaths = eventDetailDumper->FilterLogPaths();
    ASSERT_TRUE(logPaths.size() == 1);

    eventDetailDumper->eventId_ = "1464551857778095534";
    logPaths = eventDetailDumper->FilterLogPaths();
    ASSERT_TRUE(logPaths.size() == 2);
}

HWTEST_F(EventDumperTest, EventDetailDumper_LimitEventCount, TestSize.Level1)
{
    std::shared_ptr<EventDetailDumper> eventDetailDumper = std::make_shared<EventDetailDumper>();
    SetEventDetailDumperTestData(eventDetailDumper);

    eventDetailDumper->showEventCount_ = 1;
    auto logPaths = eventDetailDumper->FilterLogPaths();
    ASSERT_TRUE(logPaths.size() == 1);
}
} // namespace HiviewDFX
} // namespace OHOS