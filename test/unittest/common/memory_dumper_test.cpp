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
#include <unistd.h>
#include <vector>
#include <v1_0/imemory_tracker_interface.h>

#define private public
#include "executor/memory_dumper.h"
#undef private
#include "dump_client_main.h"
#include "hdf_base.h"
#include "executor/memory/memory_filter.h"
#include "executor/memory/memory_util.h"

using namespace testing::ext;
using namespace OHOS::HDI::Memorytracker::V1_0;
namespace OHOS {
namespace HiviewDFX {
class MemoryDumperTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    bool IsExistInCmdResult(const std::string &cmd, const std::string &str);
};

void MemoryDumperTest::SetUpTestCase(void)
{
}
void MemoryDumperTest::TearDownTestCase(void)
{
}
void MemoryDumperTest::SetUp(void)
{
}
void MemoryDumperTest::TearDown(void)
{
}

bool MemoryDumperTest::IsExistInCmdResult(const std::string &cmd, const std::string &str)
{
    bool ret = false;
    char* buffer = nullptr;
    size_t len = 0;
    FILE *fp = popen(cmd.c_str(), "r");
    while (getline(&buffer, &len, fp) != -1) {
        std::string line = buffer;
        if (line.find(str) != string::npos) {
            ret = true;
            break;
        }
    }
    pclose(fp);
    return ret;
}

/**
 * @tc.name: MemoryDumperTest001
 * @tc.desc: Test MemoryDumper has correct group.
 * @tc.type: FUNC
 * @tc.require: issueI5NWZQ
 */
HWTEST_F(MemoryDumperTest, MemoryDumperTest001, TestSize.Level3)
{
    std::string cmd = "hidumper --mem";
    std::string str = "Anonymous Page";
    ASSERT_TRUE(IsExistInCmdResult(cmd, str));
}

/**
 * @tc.name: MemoryDumperTest002
 * @tc.desc: Test MemoryDumper has DMA group.
 * @tc.type: FUNC
 * @tc.require: issueI5NX04
 */
HWTEST_F(MemoryDumperTest, MemoryDumperTest002, TestSize.Level3)
{
    sptr<IMemoryTrackerInterface> memtrack = IMemoryTrackerInterface::Get();
    if (memtrack == nullptr) {
        return;
    }
    std::string cmd = "hidumper --mem";
    std::string str = "DMA";
    ASSERT_TRUE(IsExistInCmdResult(cmd, str));
}

/**
 * @tc.name: MemoryUtilTest001
 * @tc.desc: Test IsNameLine has correct ret.
 * @tc.type: FUNC
 */
HWTEST_F(MemoryDumperTest, MemoryUtilTest001, TestSize.Level1)
{
    const std::string valueLine = "Rss:                  24 kB";
    std::string name;
    uint64_t iNode = 0;
    ASSERT_FALSE(MemoryUtil::GetInstance().IsNameLine(valueLine, name, iNode));
    ASSERT_EQ(name, "");
    const std::string nameLine = "ffb84000-ffba5000 rw-p 00000000 00:00 0                                  [stack]";
    ASSERT_TRUE(MemoryUtil::GetInstance().IsNameLine(nameLine, name, iNode));
    ASSERT_EQ(name, "[stack]");
}

/**
 * @tc.name: MemoryUtilTest002
 * @tc.desc: Test GetTypeAndValue has correct ret.
 * @tc.type: FUNC
 */
HWTEST_F(MemoryDumperTest, MemoryUtilTest002, TestSize.Level1)
{
    std::string type;
    uint64_t value = 0;
    const std::string illegalStr = "aaaaaa";
    ASSERT_FALSE(MemoryUtil::GetInstance().GetTypeAndValue(illegalStr, type, value));
    const std::string valueStr = "MemTotal:        2010244 kB";
    ASSERT_TRUE(MemoryUtil::GetInstance().GetTypeAndValue(valueStr, type, value));
    ASSERT_EQ(type, "MemTotal");
    ASSERT_EQ(value, 2010244);
}

/**
 * @tc.name: MemoryUtilTest003
 * @tc.desc: Test RunCMD has correct ret.
 * @tc.type: FUNC
 */
HWTEST_F(MemoryDumperTest, MemoryUtilTest003, TestSize.Level1)
{
    const std::string cmd = "ps -ef";
    std::vector<std::string> vec;
    ASSERT_TRUE(MemoryUtil::GetInstance().RunCMD(cmd, vec));
    ASSERT_GT(vec.size(), 0);
}
} // namespace HiviewDFX
} // namespace OHOS


