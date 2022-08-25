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

#include "hdf_base.h"

#include <v1_0/imemory_tracker_interface.h>

using namespace std;
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
} // namespace HiviewDFX
} // namespace OHOS
