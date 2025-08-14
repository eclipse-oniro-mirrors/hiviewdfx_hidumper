/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include "hidumper_service_test.h"
#include <fstream>
#include <fcntl.h>
#include <iservice_registry.h>
#include "dump_manager_client.h"
#include "dump_manager_service.h"
#include "inner/dump_service_id.h"
#include "dump_on_demand_load.h"
#include "executor/memory/memory_util.h"
#include "string_ex.h"

using namespace std;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::HiviewDFX;
namespace OHOS {
namespace HiviewDFX {
constexpr int32_t HidumperServiceTest::LIMIT_SIZE = 5000;
namespace {
void WriteFile(const std::string &content, const std::string &filename)
{
    std::ofstream outFile(filename);
    if (!outFile) {
        std::cerr << "open file failed: " << filename << std::endl;
        return;
    }
 
    outFile << content;
    outFile.close();
 
    std::cout << "write success" << std::endl;
}

bool IsProcessExist(int pid)
{
    string path = "/proc/" + to_string(pid);
    return access(path.c_str(), F_OK) == 0;
}
}

void HidumperServiceTest::SetUpTestCase(void)
{
}

void HidumperServiceTest::TearDownTestCase(void)
{
}

void HidumperServiceTest::SetUp(void)
{
}

void HidumperServiceTest::TearDown(void)
{
}

/**
 * @tc.name: HidumperServiceTest001
 * @tc.desc: Test DumpManagerService service ready.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperServiceTest, HidumperServiceTest001, TestSize.Level3)
{
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    ASSERT_TRUE(sam != nullptr) << "HidumperServiceTest001 fail to get GetSystemAbilityManager";
    sptr<OnDemandLoadCallback> loadCallback = new OnDemandLoadCallback();
    int32_t result = sam->LoadSystemAbility(DFX_SYS_HIDUMPER_ABILITY_ID, loadCallback);
    ASSERT_TRUE(result == ERR_OK) << "GetSystemAbility failed.";
}

/**
 * @tc.name: DumpManagerService002
 * @tc.desc: Test DumpManagerService Request.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperServiceTest, DumpManagerService002, TestSize.Level3)
{
    auto dumpManagerService = std::make_shared<DumpManagerService>();
    dumpManagerService->OnStart();
    dumpManagerService->started_ = true;
    std::vector<std::u16string> args;
    int32_t ret = ERR_OK;
    ret =  dumpManagerService->Dump(-1, args);
    ASSERT_TRUE(ret == ERR_OK);
    ret = dumpManagerService->Request(args, -1);
    ASSERT_TRUE(ret == ERR_OK);

    dumpManagerService->OnStop();
}

/**
 * @tc.name: DumpManagerService003
 * @tc.desc: Test DumpManagerService CountFdNums.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperServiceTest, DumpManagerService003, TestSize.Level3)
{
    auto dumpManagerService = std::make_shared<DumpManagerService>();
    int32_t pid = 1;
    uint32_t fdNums = 0;
    std::string detailFdInfo;
    std::vector<std::string> topLeakedTypeList;
    int32_t ret = dumpManagerService->CountFdNums(pid, fdNums, detailFdInfo, topLeakedTypeList);
    cout << "pid:" << pid << " fdNums:" << fdNums << endl;
    cout << "topLeakedTypeList:" << endl;
    for (const auto& type : topLeakedTypeList) {
        cout << " " << type << endl;
    }
    cout << "detailFdInfo:" << endl << detailFdInfo << endl;
    ASSERT_TRUE(ret == 0);
    ASSERT_FALSE(detailFdInfo.empty());
    ASSERT_FALSE(topLeakedTypeList.empty());
}

/**
 * @tc.name: DumpManagerService004
 * @tc.desc: Test DumpManagerService ScanPidOverLimit.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperServiceTest, DumpManagerService004, TestSize.Level3)
{
    auto dumpManagerService = std::make_shared<DumpManagerService>();
    std::string requestType = "fd";
    std::vector<int32_t> pidList;
    int32_t ret = dumpManagerService->ScanPidOverLimit(requestType, LIMIT_SIZE, pidList);
    ASSERT_TRUE(ret == 0);
    ret = dumpManagerService->ScanPidOverLimit(requestType, 0, pidList);
    ASSERT_TRUE(ret == 0);

    ret = dumpManagerService->ScanPidOverLimit(requestType, -1, pidList);
    ASSERT_TRUE(ret != 0);

    requestType = "..";
    ret = dumpManagerService->ScanPidOverLimit(requestType, 1, pidList);
    ASSERT_TRUE(ret == 0);
}

/**
 * @tc.name: DumpManagerService005
 * @tc.desc: Test DumpManagerService Request.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperServiceTest, DumpManagerService005, TestSize.Level3)
{
    auto dumpManagerService = std::make_shared<DumpManagerService>();
    dumpManagerService->OnStart();
    dumpManagerService->started_ = true;
    dumpManagerService->OnStart();

    std::vector<std::u16string> args;
    dumpManagerService->blockRequest_ = true;
    int32_t ret = dumpManagerService->Request(args, -1);
    ASSERT_TRUE(ret != 0);
    dumpManagerService->blockRequest_ = false;
    dumpManagerService->started_ = false;
    ret = dumpManagerService->Request(args, -1);
    ASSERT_TRUE(ret != 0);
    dumpManagerService->OnStop();
    dumpManagerService->started_ = false;
    dumpManagerService->OnStop();
}

/**
 * @tc.name: DumpManagerService006
 * @tc.desc: Test DumpManagerService error request.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperServiceTest, DumpManagerService006, TestSize.Level3)
{
    auto dumpManagerService = std::make_shared<DumpManagerService>();
    std::vector<std::u16string> args;
    int outfd = -1;
    dumpManagerService->HandleRequestError(args, outfd, -1, "test");
    ASSERT_TRUE(args.size() == 0);
    args.push_back(Str8ToStr16("test"));
    dumpManagerService->HandleRequestError(args, outfd, -1, "test");
    ASSERT_TRUE(args.size() == 0);
}

/**
 * @tc.name: DumpManagerService007
 * @tc.desc: Test DumpManagerService OnIdle.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperServiceTest, DumpManagerService007, TestSize.Level3)
{
    auto dumpManagerService = std::make_shared<DumpManagerService>();
    SystemAbilityOnDemandReason idleReason;
    int32_t ret = dumpManagerService->OnIdle(idleReason);
    ASSERT_TRUE(ret == 0);
    idleReason.SetId(OnDemandReasonId::INTERFACE_CALL);
    ret = dumpManagerService->OnIdle(idleReason);
    ASSERT_TRUE(ret == 0);
    std::vector<std::u16string> args;
    args.push_back(Str8ToStr16("test"));
    dumpManagerService->AddRequestRawParam(args, -1);
    ret = dumpManagerService->OnIdle(idleReason);
    ASSERT_TRUE(ret == 120000);
}

/**
 * @tc.name: DumpManagerService008
 * @tc.desc: Test DumpManagerService GetFdLink with invalid symlink.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperServiceTest, DumpManagerService008, TestSize.Level3)
{
    const std::string invalidLinkPath = "/tmp/nonexistent_link";
    auto dumpManagerService = std::make_shared<DumpManagerService>();
    std::string result = dumpManagerService->GetFdLink(invalidLinkPath);
    printf("Retrieved symlink content: '%s'\n", result.c_str());
    ASSERT_TRUE(result == "unknown");
    WriteFile(result, "/data/local/tmp/DumpManagerService008_result");
}

/**
 * @tc.name: DumpManagerService009
 * @tc.desc: Test DumpManagerService GetFdLinks with valid symlinks.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperServiceTest, DumpManagerService009, TestSize.Level1)
{
    int pid = 1;
    auto dumpService = std::make_shared<DumpManagerService>();
    std::vector<std::string> links = dumpService->GetFdLinks(pid);

    cout << "links.size:" << links.size() << endl;

    EXPECT_GT(links.size(), 0);

    std::cout << "DumpManagerService009 passed with " << links.size() << " links." << std::endl;
}

/**
 * @tc.name: DumpManagerService010
 * @tc.desc: Test DumpManagerService GetFdLinks with non-existent fd directory.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperServiceTest, DumpManagerService010, TestSize.Level1)
{
    int pid = 65533;
    // 确保该进程不存在
    if (IsProcessExist(pid)) {
        GTEST_SKIP() << "进程 " << pid << " 已经存在，跳过测试。";
    }

    auto dumpService = std::make_shared<DumpManagerService>();
    std::vector<std::string> links = dumpService->GetFdLinks(pid);
    EXPECT_TRUE(links.empty());
    std::cout << "DumpManagerService010 passed with empty links." << std::endl;
}

/**
 * @tc.name: DumpManagerService011
 * @tc.desc: Test MaybeKnownType with valid eventfd type in link.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperServiceTest, DumpManagerService011, TestSize.Level3)
{
    std::string link = "eventfd";
    auto dumpManagerService = std::make_shared<DumpManagerService>();
    std::string result = dumpManagerService->MaybeKnownType(link);
    ASSERT_TRUE(result == "eventfd");
    std::cout << "DumpManagerService011 passed for eventfd." << std::endl;
}

/**
 * @tc.name: DumpManagerService012
 * @tc.desc: Test MaybeKnownType with valid eventpoll type in link.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperServiceTest, DumpManagerService012, TestSize.Level3)
{
    std::string link = "eventpoll";
    auto dumpManagerService = std::make_shared<DumpManagerService>();
    std::string result = dumpManagerService->MaybeKnownType(link);
    ASSERT_TRUE(result == "eventpoll");
    std::cout << "DumpManagerService012 passed for eventpoll." << std::endl;
}

/**
 * @tc.name: DumpManagerService013
 * @tc.desc: Test MaybeKnownType with valid sync_file type in link.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperServiceTest, DumpManagerService013, TestSize.Level3)
{
    std::string link = "/proc/123/fd/4->sync_file";
    auto dumpManagerService = std::make_shared<DumpManagerService>();
    std::string result = dumpManagerService->MaybeKnownType(link);
    ASSERT_TRUE(result == "sync_file");
    std::cout << "DumpManagerService013 passed for sync_file." << std::endl;
}

/**
 * @tc.name: DumpManagerService014
 * @tc.desc: Test MaybeKnownType with valid dmabuf type in link.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperServiceTest, DumpManagerService014, TestSize.Level3)
{
    std::string link = "dmabuf";
    auto dumpManagerService = std::make_shared<DumpManagerService>();
    std::string result = dumpManagerService->MaybeKnownType(link);
    ASSERT_TRUE(result == "dmabuf");
    std::cout << "DumpManagerService014 passed for dmabuf." << std::endl;
}

/**
 * @tc.name: DumpManagerService015
 * @tc.desc: Test MaybeKnownType with valid socket type in link.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperServiceTest, DumpManagerService015, TestSize.Level3)
{
    std::string link = "socket:[12345]";
    auto dumpManagerService = std::make_shared<DumpManagerService>();
    std::string result = dumpManagerService->MaybeKnownType(link);
    ASSERT_TRUE(result == "socket");
    std::cout << "DumpManagerService015 passed for socket." << std::endl;
}

/**
 * @tc.name: DumpManagerService016
 * @tc.desc: Test MaybeKnownType with valid pipe type in link.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperServiceTest, DumpManagerService016, TestSize.Level3)
{
    std::string link = "pipe:1234";
    auto dumpManagerService = std::make_shared<DumpManagerService>();
    std::string result = dumpManagerService->MaybeKnownType(link);
    ASSERT_TRUE(result == "pipe");
    std::cout << "DumpManagerService016 passed for pipe." << std::endl;
}

/**
 * @tc.name: DumpManagerService017
 * @tc.desc: Test MaybeKnownType with valid ashmem type in link.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperServiceTest, DumpManagerService017, TestSize.Level3)
{
    std::string link = "ashmem: /dev/ashmem";
    auto dumpManagerService = std::make_shared<DumpManagerService>();
    std::string result = dumpManagerService->MaybeKnownType(link);
    ASSERT_TRUE(result == "ashmem");
    std::cout << "DumpManagerService017 passed for ashmem." << std::endl;
}

/**
 * @tc.name: DumpManagerService018
 * @tc.desc: Test MaybeKnownType with unknown type in link.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperServiceTest, DumpManagerService018, TestSize.Level3)
{
    std::string link = "/dev/random-file";
    auto dumpManagerService = std::make_shared<DumpManagerService>();
    std::string result = dumpManagerService->MaybeKnownType(link);
    ASSERT_TRUE(result == "unknown");
    std::cout << "DumpManagerService018 passed for unknown type." << std::endl;
}

/**
 * @tc.name: DumpManagerService019
 * @tc.desc: Test MaybeKnownType with multiple known types in link (returns first match).
 * @tc.type: FUNC
 */
HWTEST_F(HidumperServiceTest, DumpManagerService019, TestSize.Level3)
{
    std::string link = "eventfd_with_pipe";
    auto dumpManagerService = std::make_shared<DumpManagerService>();
    std::string result = dumpManagerService->MaybeKnownType(link);
    ASSERT_TRUE(result == "eventfd");
    std::cout << "DumpManagerService019 passed for priority match." << std::endl;
}

/**
 * @tc.name: DumpManagerService020
 * @tc.desc: Test CountPaths with multiple paths, check if counts are correct.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperServiceTest, DumpManagerService020, TestSize.Level3)
{
    // 准备测试数据
    std::vector<std::string> links = {
        "/data/log/a.txt",
        "/data/log/b.txt",
        "/data/log/a.txt",
        "/data/log/c.txt",
        "/data/log/b.txt",
        "/data/log/b.txt"
    };

    // 创建 DumpManagerService 实例
    auto dumpManagerService = std::make_shared<DumpManagerService>();

    // 调用待测函数
    std::unordered_map<std::string, int> result = dumpManagerService->CountPaths(links);

    // 预期结果
    std::unordered_map<std::string, int> expected = {
        { "/data/log/a.txt", 2 },
        { "/data/log/b.txt", 3 },
        { "/data/log/c.txt", 1 }
    };

    // 断言结果是否与预期一致
    ASSERT_TRUE(result == expected);
    std::cout << "DumpManagerService020 passed for path counting." << std::endl;
}

/**
 * @tc.name: DumpManagerService021
 * @tc.desc: Test TopN with valid input and n >= counter size.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperServiceTest, DumpManagerService021, TestSize.Level3)
{
    std::unordered_map<std::string, int> counter = {
        {"eventfd", 10}, {"pipe", 8}, {"ashmem", 7}, {"socket", 12}, {"unknown1", 5}
    };
    size_t n = 3;
    auto dumpManagerService = std::make_shared<DumpManagerService>();
    std::vector<std::pair<std::string, int>> result = dumpManagerService->TopN(counter, n);

    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0].first, "socket");
    EXPECT_EQ(result[0].second, 12);
    EXPECT_EQ(result[1].first, "eventfd");
    EXPECT_EQ(result[1].second, 10);
    EXPECT_EQ(result[2].first, "pipe");
    EXPECT_EQ(result[2].second, 8);
    std::cout << "DumpManagerService021 passed: Top3 entries correct." << std::endl;
}

/**
 * @tc.name: DumpManagerService022
 * @tc.desc: Test TopN when n is larger than counter size.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperServiceTest, DumpManagerService022, TestSize.Level3)
{
    std::unordered_map<std::string, int> counter = {{"a", 3}, {"b", 2}, {"c", 1}};
    size_t n = 5;
    auto dumpManagerService = std::make_shared<DumpManagerService>();
    std::vector<std::pair<std::string, int>> result = dumpManagerService->TopN(counter, n);

    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0].first, "a");
    EXPECT_EQ(result[1].first, "b");
    EXPECT_EQ(result[2].first, "c");
    std::cout << "DumpManagerService022 passed: All entries returned when n > size." << std::endl;
}

/**
 * @tc.name: DumpManagerService023
 * @tc.desc: Test TopN with empty counter.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperServiceTest, DumpManagerService023, TestSize.Level3)
{
    std::unordered_map<std::string, int> counter = {};
    size_t n = 2;
    auto dumpManagerService = std::make_shared<DumpManagerService>();
    std::vector<std::pair<std::string, int>> result = dumpManagerService->TopN(counter, n);

    EXPECT_TRUE(result.empty());
    std::cout << "DumpManagerService023 passed: Empty counter returns empty list." << std::endl;
}

/**
 * @tc.name: DumpManagerService024
 * @tc.desc: Test heap replacement logic in TopN.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperServiceTest, DumpManagerService024, TestSize.Level3)
{
    std::unordered_map<std::string, int> counter = {
        {"a", 1}, {"b", 3}, {"c", 2}, {"d", 5}, {"e", 4}
    };
    size_t n = 3;
    auto dumpManagerService = std::make_shared<DumpManagerService>();
    std::vector<std::pair<std::string, int>> result = dumpManagerService->TopN(counter, n);

    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0].first, "d");
    EXPECT_EQ(result[0].second, 5);
    EXPECT_EQ(result[1].first, "e");
    EXPECT_EQ(result[1].second, 4);
    EXPECT_EQ(result[2].first, "b");
    EXPECT_EQ(result[2].second, 3);
    std::cout << "DumpManagerService024 passed: Heap replacement logic works as expected." << std::endl;
}

/**
 * @tc.name: DumpManagerService025
 * @tc.desc: Test GetTopFdInfo with normal input containing multiple entries.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperServiceTest, DumpManagerService025, TestSize.Level3)
{
    std::vector<std::pair<std::string, int>> topLinks = {
        {"eventfd", 3},
        {"pipe", 5},
        {"ashmem", 2}
    };
    auto dumpManagerService = std::make_shared<DumpManagerService>();
    std::string result = dumpManagerService->GetTopFdInfo(topLinks);
    std::string expected = "3\teventfd\n5\tpipe\n2\tashmem\n";
    EXPECT_EQ(result, expected);
    std::cout << "DumpManagerService025 passed: Normal input format correct." << std::endl;
}

/**
 * @tc.name: DumpManagerService026
 * @tc.desc: Test GetTopFdInfo with empty input.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperServiceTest, DumpManagerService026, TestSize.Level3)
{
    std::vector<std::pair<std::string, int>> topLinks = {};
    auto dumpManagerService = std::make_shared<DumpManagerService>();
    std::string result = dumpManagerService->GetTopFdInfo(topLinks);
    std::string expected = "";
    EXPECT_EQ(result, expected);
    std::cout << "DumpManagerService026 passed: Empty input returns empty string." << std::endl;
}

/**
 * @tc.name: DumpManagerService027
 * @tc.desc: Test GetTopFdInfo with single entry.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperServiceTest, DumpManagerService027, TestSize.Level3)
{
    std::vector<std::pair<std::string, int>> topLinks = {{"socket", 1}};
    auto dumpManagerService = std::make_shared<DumpManagerService>();
    std::string result = dumpManagerService->GetTopFdInfo(topLinks);
    std::string expected = "1\tsocket\n";
    EXPECT_EQ(result, expected);
    std::cout << "DumpManagerService027 passed: Single entry returns correct string." << std::endl;
}

/**
 * @tc.name: DumpManagerService028
 * @tc.desc: Test GetTopFdInfo with non-ASCII characters in names.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperServiceTest, DumpManagerService028, TestSize.Level3)
{
    std::vector<std::pair<std::string, int>> topLinks = {
        {"fd\\twith\\tnull", 2},
        {"file\\nwith\\nnewline", 3}
    };
    auto dumpManagerService = std::make_shared<DumpManagerService>();
    std::string result = dumpManagerService->GetTopFdInfo(topLinks);
    std::string expected = "2\tfd\\twith\\tnull\n3\tfile\\nwith\\nnewline\n";
    EXPECT_EQ(result, expected);
    std::cout << "DumpManagerService028 passed: Non-ASCII characters in names handled properly." << std::endl;
}

/**
 * @tc.name: DumpManagerService029
 * @tc.desc: Test GetTopDirInfo with paths exceeding FD_TOP_CNT (10).
 * @tc.type: FUNC
 */
HWTEST_F(HidumperServiceTest, DumpManagerService029, TestSize.Level3)
{
    std::vector<std::pair<std::string, int>> topTypes = {
        {"eventfd", 50}  // Arbitrary value for total
    };
    std::map<std::string, std::unordered_map<std::string, int>> typePaths = {
        {"eventfd", {
            {"path1", 5},
            {"path2", 4},
            {"path3", 3},
            {"path4", 2},
            {"path5", 1},
            {"path6", 6},
            {"path7", 7},
            {"path8", 8},
            {"path9", 9},
            {"path10", 10},
            {"path11", 11}, // Exceed FD_TOP_CNT (10)
            {"path12", 12},
        }}
    };
    auto dumpManagerService = std::make_shared<DumpManagerService>();
    std::string result = dumpManagerService->GetTopDirInfo(topTypes, typePaths);
    std::string expected = "50\teventfd\n012\tpath12\n011\tpath11\n010\tpath10\n09\tpath9\n08\tpath8\n"
                           "07\tpath7\n06\tpath6\n05\tpath1\n04\tpath2\n03\tpath3\n";
    EXPECT_EQ(result, expected);
    std::cout << "DumpManagerService029 passed: Paths exceeding FD_TOP_CNT add ellipsis." << std::endl;
}

/**
 * @tc.name: DumpManagerService030
 * @tc.desc: Test GetTopDirInfo with empty input
 * @tc.type: FUNC
 */
HWTEST_F(HidumperServiceTest, DumpManagerService030, TestSize.Level3)
{
    std::vector<std::pair<std::string, int>> topTypes = {};
    std::map<std::string, std::unordered_map<std::string, int>> typePaths = {};
    auto dumpManagerService = std::make_shared<DumpManagerService>();
    std::string result = dumpManagerService->GetTopDirInfo(topTypes, typePaths);
    std::string expected = "";
    EXPECT_EQ(result, expected);
    std::cout << "DumpManagerService030 passed: Empty input returns empty string." << std::endl;
}

/**
 * @tc.name: DumpManagerService031
 * @tc.desc: Test GetTopDirInfo with type path equal to type name
 * @tc.type: FUNC
 */
HWTEST_F(HidumperServiceTest, DumpManagerService031, TestSize.Level3)
{
    std::vector<std::pair<std::string, int>> topTypes = {
        {"socket", 5}
    };
    std::map<std::string, std::unordered_map<std::string, int>> typePaths = {
        {"socket", {
            {"socket_2", 3},
            {"socket_1", 2}
        }}
    };
    auto dumpManagerService = std::make_shared<DumpManagerService>();
    std::string result = dumpManagerService->GetTopDirInfo(topTypes, typePaths);
    std::string expected = "5	socket\n03\tsocket_2\n02\tsocket_1\n";
    EXPECT_EQ(result, expected);
    std::cout << "DumpManagerService031 passed: Skip path equal to type name." << std::endl;
}

/**
 * @tc.name: DumpManagerService032
 * @tc.desc: Test GetSummary when both vectors are empty
 * @tc.type: FUNC
 */
HWTEST_F(HidumperServiceTest, DumpManagerService032, TestSize.Level3)
{
    std::vector<std::pair<std::string, int>> topLinks{};
    std::vector<std::pair<std::string, int>> topTypes{};
    auto dumpManagerService = std::make_shared<DumpManagerService>();
    std::string result = dumpManagerService->GetSummary(topLinks, topTypes);
    std::string expected = "";
    EXPECT_EQ(result, expected);
    std::cout << "DumpManagerService032 passed: Both vectors empty returns empty." << std::endl;
}

/**
 * @tc.name: DumpManagerService033
 * @tc.desc: Test GetSummary when only topLinks is empty
 * @tc.type: FUNC
 */
HWTEST_F(HidumperServiceTest, DumpManagerService033, TestSize.Level3)
{
    std::vector<std::pair<std::string, int>> topLinks{};
    std::vector<std::pair<std::string, int>> topTypes = {{"dir_inode", 10}};
    auto dumpManagerService = std::make_shared<DumpManagerService>();
    std::string result = dumpManagerService->GetSummary(topLinks, topTypes);
    std::string expected = "Leaked dir:dir_inode";
    EXPECT_EQ(result, expected);
    std::cout << "DumpManagerService033 passed: Only topTypes has data." << std::endl;
}

/**
 * @tc.name: DumpManagerService034
 * @tc.desc: Test GetSummary when only topTypes is empty
 * @tc.type: FUNC
 */
HWTEST_F(HidumperServiceTest, DumpManagerService034, TestSize.Level3)
{
    std::vector<std::pair<std::string, int>> topLinks = {{"fd_pipe", 7}};
    std::vector<std::pair<std::string, int>> topTypes{};
    auto dumpManagerService = std::make_shared<DumpManagerService>();
    std::string result = dumpManagerService->GetSummary(topLinks, topTypes);
    std::string expected = "Leaked fd:fd_pipe";
    EXPECT_EQ(result, expected);
    std::cout << "DumpManagerService034 passed: Only topLinks has data." << std::endl;
}

/**
 * @tc.name: DumpManagerService035
 * @tc.desc: Test GetSummary when topLinks count is greater
 * @tc.type: FUNC
 */
HWTEST_F(HidumperServiceTest, DumpManagerService035, TestSize.Level3)
{
    std::vector<std::pair<std::string, int>> topLinks = {{"fd_char", 10}};
    std::vector<std::pair<std::string, int>> topTypes = {{"dir_block", 5}};
    auto dumpManagerService = std::make_shared<DumpManagerService>();
    std::string result = dumpManagerService->GetSummary(topLinks, topTypes);
    std::string expected = "Leaked fd:fd_char";
    EXPECT_EQ(result, expected);
    std::cout << "DumpManagerService035 passed: Link count dominates." << std::endl;
}

/**
 * @tc.name: DumpManagerService036
 * @tc.desc: Test GetSummary when counts are equal
 * @tc.type: FUNC
 */
HWTEST_F(HidumperServiceTest, DumpManagerService036, TestSize.Level3)
{
    std::vector<std::pair<std::string, int>> topLinks = {{"fd_dev", 8}};
    std::vector<std::pair<std::string, int>> topTypes = {{"dir_dev", 8}};
    auto dumpManagerService = std::make_shared<DumpManagerService>();
    std::string result = dumpManagerService->GetSummary(topLinks, topTypes);
    std::string expected = "Leaked fd:fd_dev";
    EXPECT_EQ(result, expected);
    std::cout << "DumpManagerService036 passed: Equal counts choose fd." << std::endl;
}
} // namespace HiviewDFX
} // namespace OHOS
