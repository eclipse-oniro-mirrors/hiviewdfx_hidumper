/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include <map>
#include <stdlib.h>
#include <unistd.h>
#include <vector>

#define private public
#include "iservice_registry.h"
#include "manager/dump_implement.h"
#include "raw_param.h"
#undef private

using namespace testing::ext;
namespace OHOS {
namespace HiviewDFX {

const std::string TOOL_NAME = "hidumper";
char FILE_NAME[] = "/tmp/test.XXXXXX";
int g_fd = -1;
class HiDumperManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    int GetDumpResult(int argc, char *argv[]);
    int fd_;
};

void HiDumperManagerTest::SetUpTestCase(void)
{
    g_fd = mkstemp(FILE_NAME);
    if (g_fd == -1) {
        printf("create tmp file fail!");
        return;
    }
}
void HiDumperManagerTest::TearDownTestCase(void)
{
    if (g_fd != -1) {
        close(g_fd);
    }
    unlink(FILE_NAME);
}
void HiDumperManagerTest::SetUp(void)
{
}
void HiDumperManagerTest::TearDown(void)
{
}

int HiDumperManagerTest::GetDumpResult(int argc, char *argv[])
{
    std::vector<std::u16string> args;
    std::shared_ptr<RawParam> rawParam = std::make_shared<RawParam>(0, 1, 0, args, g_fd);
    return DumpImplement::GetInstance().Main(argc, argv, rawParam);
}

/**
 * @tc.name: MemoryDumperTest001
 * @tc.desc: Test help msg has correct ret.
 * @tc.type: FUNC
 * @tc.require: issueI5NWZQ
 */
HWTEST_F(HiDumperManagerTest, DumpTest001, TestSize.Level0)
{
    char *argv[] = {
        const_cast<char *>(TOOL_NAME.c_str()),
        const_cast<char *>("-h"),
        const_cast<char *>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    int ret = GetDumpResult(argc, argv);
    ASSERT_EQ(ret, DumpStatus::DUMP_HELP);
}

/**
 * @tc.name: MemoryDumperTest002
 * @tc.desc: Test list system has correct ret.
 * @tc.type: FUNC
 * @tc.require: issueI5NWZQ
 */
HWTEST_F(HiDumperManagerTest, DumpTest002, TestSize.Level0)
{
    char *argv[] = {
        const_cast<char *>(TOOL_NAME.c_str()),
        const_cast<char *>("-lc"),
        const_cast<char *>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    int ret = GetDumpResult(argc, argv);
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
}

/**
 * @tc.name: MemoryDumperTest003
 * @tc.desc: Test list aility has correct ret.
 * @tc.type: FUNC
 * @tc.require: issueI5NWZQ
 */
HWTEST_F(HiDumperManagerTest, DumpTest003, TestSize.Level0)
{
    char *argv[] = {
        const_cast<char *>(TOOL_NAME.c_str()),
        const_cast<char *>("-ls"),
        const_cast<char *>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    int ret = GetDumpResult(argc, argv);
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
}

/**
 * @tc.name: MemoryDumperTest004
 * @tc.desc: Test dump base information has correct ret.
 * @tc.type: FUNC
 * @tc.require: issueI5NWZQ
 */
HWTEST_F(HiDumperManagerTest, DumpTest004, TestSize.Level0)
{
    char *argv[] = {
        const_cast<char *>(TOOL_NAME.c_str()),
        const_cast<char *>("-c"),
        const_cast<char *>("base"),
        const_cast<char *>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    int ret = GetDumpResult(argc, argv);
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
}

/**
 * @tc.name: MemoryDumperTest005
 * @tc.desc: Test dump system information has correct ret.
 * @tc.type: FUNC
 * @tc.require: issueI5NWZQ
 */
HWTEST_F(HiDumperManagerTest, DumpTest005, TestSize.Level0)
{
    char *argv[] = {
        const_cast<char *>(TOOL_NAME.c_str()),
        const_cast<char *>("-c"),
        const_cast<char *>("system"),
        const_cast<char *>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    int ret = GetDumpResult(argc, argv);
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
}

/**
 * @tc.name: MemoryDumperTest006
 * @tc.desc: Test dump all system information has correct ret.
 * @tc.type: FUNC
 * @tc.require: issueI5NWZQ
 */
HWTEST_F(HiDumperManagerTest, DumpTest006, TestSize.Level0)
{
    char *argv[] = {
        const_cast<char *>(TOOL_NAME.c_str()),
        const_cast<char *>("-c"),
        const_cast<char *>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    int ret = GetDumpResult(argc, argv);
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
}

/**
 * @tc.name: MemoryDumperTest007
 * @tc.desc: Test dump all ability has correct ret.
 * @tc.type: FUNC
 * @tc.require: issueI5NWZQ
 */
HWTEST_F(HiDumperManagerTest, DumpTest007, TestSize.Level0)
{
    char *argv[] = {
        const_cast<char *>(TOOL_NAME.c_str()),
        const_cast<char *>("-s"),
        const_cast<char *>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    int ret = GetDumpResult(argc, argv);
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
}

/**
 * @tc.name: MemoryDumperTest007
 * @tc.desc: Test dump one ability has correct ret.
 * @tc.type: FUNC
 * @tc.require: issueI5NWZQ
 */
HWTEST_F(HiDumperManagerTest, DumpTest008, TestSize.Level0)
{
    char *argv[] = {
        const_cast<char *>(TOOL_NAME.c_str()),
        const_cast<char *>("-s"),
        const_cast<char *>("1212"),
        const_cast<char *>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    int ret = GetDumpResult(argc, argv);
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
}

/**
 * @tc.name: MemoryDumperTest009
 * @tc.desc: Test dump one ability with arg has correct ret.
 * @tc.type: FUNC
 * @tc.require: issueI5NWZQ
 */
HWTEST_F(HiDumperManagerTest, DumpTest009, TestSize.Level0)
{
    char *argv[] = {
        const_cast<char *>(TOOL_NAME.c_str()),
        const_cast<char *>("-s"),
        const_cast<char *>("SensorService"),
        const_cast<char *>("-a"),
        const_cast<char *>("-h"),
        const_cast<char *>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    int ret = GetDumpResult(argc, argv);
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
}

/**
 * @tc.name: MemoryDumperTest010
 * @tc.desc: Test dump faultlogs of crash history has correct ret.
 * @tc.type: FUNC
 * @tc.require: issueI5NWZQ
 */
HWTEST_F(HiDumperManagerTest, DumpTest010, TestSize.Level0)
{
    char *argv[] = {
        const_cast<char *>(TOOL_NAME.c_str()),
        const_cast<char *>("-e"),
        const_cast<char *>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    int ret = GetDumpResult(argc, argv);
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
}

/**
 * @tc.name: MemoryDumperTest011
 * @tc.desc: Test dump network information has correct ret.
 * @tc.type: FUNC
 * @tc.require: issueI5NWZQ
 */
HWTEST_F(HiDumperManagerTest, DumpTest011, TestSize.Level0)
{
    char *argv[] = {
        const_cast<char *>(TOOL_NAME.c_str()),
        const_cast<char *>("--net"),
        const_cast<char *>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    int ret = GetDumpResult(argc, argv);
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
}

/**
 * @tc.name: MemoryDumperTest012
 * @tc.desc: Test dump storage information has correct ret.
 * @tc.type: FUNC
 * @tc.require: issueI5NWZQ
 */
HWTEST_F(HiDumperManagerTest, DumpTest012, TestSize.Level0)
{
    char *argv[] = {
        const_cast<char *>(TOOL_NAME.c_str()),
        const_cast<char *>("--storage"),
        const_cast<char *>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    int ret = GetDumpResult(argc, argv);
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
}

/**
 * @tc.name: MemoryDumperTest013
 * @tc.desc: Test dump processes information has correct ret.
 * @tc.type: FUNC
 * @tc.require: issueI5NWZQ
 */
HWTEST_F(HiDumperManagerTest, DumpTest013, TestSize.Level0)
{
    char *argv[] = {
        const_cast<char *>(TOOL_NAME.c_str()),
        const_cast<char *>("-p"),
        const_cast<char *>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    int ret = GetDumpResult(argc, argv);
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
}

/**
 * @tc.name: MemoryDumperTest014
 * @tc.desc: Test dump one process information has correct ret.
 * @tc.type: FUNC
 * @tc.require: issueI5NWZQ
 */
HWTEST_F(HiDumperManagerTest, DumpTest014, TestSize.Level0)
{
    char *argv[] = {
        const_cast<char *>(TOOL_NAME.c_str()),
        const_cast<char *>("-p"),
        const_cast<char *>(std::to_string(getpid()).c_str()),
        const_cast<char *>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    int ret = GetDumpResult(argc, argv);
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
}

/**
 * @tc.name: MemoryDumperTest015
 * @tc.desc: Test dump all process cpu usage has correct ret.
 * @tc.type: FUNC
 * @tc.require: issueI5NWZQ
 */
HWTEST_F(HiDumperManagerTest, DumpTest015, TestSize.Level0)
{
    char *argv[] = {
        const_cast<char *>(TOOL_NAME.c_str()),
        const_cast<char *>("--cpuusage"),
        const_cast<char *>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    int ret = GetDumpResult(argc, argv);
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
}

/**
 * @tc.name: MemoryDumperTest016
 * @tc.desc: Test dump one process cpu usage has correct ret.
 * @tc.type: FUNC
 * @tc.require: issueI5NWZQ
 */
HWTEST_F(HiDumperManagerTest, DumpTest016, TestSize.Level0)
{
    char *argv[] = {
        const_cast<char *>(TOOL_NAME.c_str()),
        const_cast<char *>("--cpuusage"),
        const_cast<char *>(std::to_string(getpid()).c_str()),
        const_cast<char *>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    int ret = GetDumpResult(argc, argv);
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
}

/**
 * @tc.name: MemoryDumperTest017
 * @tc.desc: Test dumpreal CPU frequency has correct ret.
 * @tc.type: FUNC
 * @tc.require: issueI5NWZQ
 */
HWTEST_F(HiDumperManagerTest, DumpTest017, TestSize.Level0)
{
    char *argv[] = {
        const_cast<char *>(TOOL_NAME.c_str()),
        const_cast<char *>("--cpufreq"),
        const_cast<char *>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    int ret = GetDumpResult(argc, argv);
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
}

/**
 * @tc.name: MemoryDumperTest018
 * @tc.desc: Test dump all processes memory info has correct ret.
 * @tc.type: FUNC
 * @tc.require: issueI5NWZQ
 */
HWTEST_F(HiDumperManagerTest, DumpTest018, TestSize.Level0)
{
    char *argv[] = {
        const_cast<char *>(TOOL_NAME.c_str()),
        const_cast<char *>("--mem"),
        const_cast<char *>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    int ret = GetDumpResult(argc, argv);
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
}

/**
 * @tc.name: MemoryDumperTest019
 * @tc.desc: Test dump one process memory info has correct ret.
 * @tc.type: FUNC
 * @tc.require: issueI5NWZQ
 */
HWTEST_F(HiDumperManagerTest, DumpTest019, TestSize.Level0)
{
    char *argv[] = {
        const_cast<char *>(TOOL_NAME.c_str()),
        const_cast<char *>("--mem"),
        const_cast<char *>(std::to_string(getpid()).c_str()),
        const_cast<char *>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    int ret = GetDumpResult(argc, argv);
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
}

/**
 * @tc.name: MemoryDumperTest019
 * @tc.desc: Test compress has correct ret.
 * @tc.type: FUNC
 * @tc.require: issueI5NWZQ
 */
HWTEST_F(HiDumperManagerTest, DumpTest020, TestSize.Level0)
{
    char *argv[] = {
        const_cast<char *>(TOOL_NAME.c_str()),
        const_cast<char *>("-p"),
        const_cast<char *>("--zip"),
        const_cast<char *>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    int ret = GetDumpResult(argc, argv);
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
}
} // namespace HiviewDFX
} // namespace OHOS
