/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include <thread>
#include <unistd.h>
#include "hidumper_test_utils.h"

using namespace testing::ext;
namespace OHOS {
namespace HiviewDFX {
const int THREAD_EXECUTE_NUM = 2;
class SADumperTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void SADumperTest::SetUpTestCase(void)
{
}
void SADumperTest::TearDownTestCase(void)
{
}
void SADumperTest::SetUp(void)
{
}
void SADumperTest::TearDown(void)
{
}

/**
 * @tc.name: SADumperTest001
 * @tc.desc: Test SA HiviewService result contain "log".
 * @tc.type: FUNC
 * @tc.require: issueI5NWZQ
 */
#ifdef HIDUMPER_HIVIEWDFX_HIVIEW_ENABLE
HWTEST_F(SADumperTest, SADumperTest001, TestSize.Level3)
{
    std::string cmd = "hidumper -s 1201 -a '-p Faultlogger'";
    std::string str = "log";
    ASSERT_TRUE(HidumperTestUtils::GetInstance().IsExistInCmdResult(cmd, str));
}
#endif

/**
 * @tc.name: SADumperTest002
 * @tc.desc: Test SA WindowManagerService result contain "WindowName".
 * @tc.type: FUNC
 * @tc.require: issueI5NX04
 */
HWTEST_F(SADumperTest, SADumperTest002, TestSize.Level3)
{
    std::string cmd = "hidumper -s WindowManagerService -a -a";
    std::string str = "WindowName";
    ASSERT_TRUE(HidumperTestUtils::GetInstance().IsExistInCmdResult(cmd, str));
}

/**
 * @tc.name: SADumperTest003
 * @tc.desc: Test SA RenderService result contain "ScreenInfo".
 * @tc.type: FUNC
 * @tc.require: issueI5NWZQ
 */
HWTEST_F(SADumperTest, SADumperTest003, TestSize.Level3)
{
    std::string cmd = "hidumper -s 10 -a allInfo";
    std::string str = "ScreenInfo";
    ASSERT_TRUE(HidumperTestUtils::GetInstance().IsExistInCmdResult(cmd, str));
}

/**
 * @tc.name: SADumperTest004
 * @tc.desc: Test SA WorkSchedule result contain "Work".
 * @tc.type: FUNC
 * @tc.require: issueI5NWZQ
 */
HWTEST_F(SADumperTest, SADumperTest004, TestSize.Level3)
{
    std::string cmd = "hidumper -s 1904 -a -a";
    std::string str = "Work";
    ASSERT_TRUE(HidumperTestUtils::GetInstance().IsExistInCmdResult(cmd, str));
}
/**
 * @tc.name: SADumperTest005
 * @tc.desc: Test SA AbilityManagerService result contain "User".
 * @tc.type: FUNC
 * @tc.require: issueI5NWZQ
 */
HWTEST_F(SADumperTest, SADumperTest005, TestSize.Level3)
{
    std::string cmd = "hidumper -s AbilityManagerService -a -l";
    std::string str = "User";
    ASSERT_TRUE(HidumperTestUtils::GetInstance().IsExistInCmdResult(cmd, str));
}

/**
 * @tc.name: SADumperTest006
 * @tc.desc: Test SA DisplayManagerService result contain "Screen".
 * @tc.type: FUNC
 * @tc.require: issueI5NWZQ
 */
HWTEST_F(SADumperTest, SADumperTest006, TestSize.Level3)
{
    std::string cmd = "hidumper -s DisplayManagerService -a '-s -a'";
    std::string str = "Screen";
    ASSERT_TRUE(HidumperTestUtils::GetInstance().IsExistInCmdResult(cmd, str));
}

/**
 * @tc.name: SADumperTest008
 * @tc.desc: Test SA MultimodalInput result contain "Windows".
 * @tc.type: FUNC
 * @tc.require: issueI5NWZQ
 */
HWTEST_F(SADumperTest, SADumperTest008, TestSize.Level3)
{
    std::string cmd = "hidumper -s MultimodalInput -a -w";
    std::string str = "MultimodalInput";
    ASSERT_TRUE(HidumperTestUtils::GetInstance().IsExistInCmdResult(cmd, str));
}

/**
 * @tc.name: SADumperTest009
 * @tc.desc: Test SA RenderService result contain "Graphic".
 * @tc.type: FUNC
 * @tc.require: issueI5NWZQ
 */
HWTEST_F(SADumperTest, SADumperTest009, TestSize.Level3)
{
    std::string path = "/data/log/hidumper/RenderService.txt";
    std::string cmd = "hidumper -s 10 -a -h > " + path;
    std::string str = "Graphic";
    ASSERT_TRUE(HidumperTestUtils::GetInstance().IsExistStrInFile(cmd, str, path));
    system("rm -rf /data/log/hidumper/RenderService.txt");
}

/**
 * @tc.name: SADumperTest010
 * @tc.desc: Test zip SA RenderService result not contain "Graphic".
 * @tc.type: FUNC
 * @tc.require: issueI5NWZQ
 */
HWTEST_F(SADumperTest, SADumperTest010, TestSize.Level3)
{
    std::string cmd = "hidumper -s 10 -a -h --zip";
    std::string str = "Graphic";
    ASSERT_FALSE(HidumperTestUtils::GetInstance().IsExistInCmdResult(cmd, str));
}

/**
 * @tc.name: SADumperTest011
 * @tc.desc: Test SA AbilityManagerService result contain "AppRunningRecord".
 * @tc.type: FUNC
 * @tc.require: issueI5NWZQ
 */
HWTEST_F(SADumperTest, SADumperTest011, TestSize.Level3)
{
    std::string cmd = "hidumper -s AbilityManagerService -a '-a'";
    std::string str = "AppRunningRecord";
    ASSERT_TRUE(HidumperTestUtils::GetInstance().IsExistInCmdResult(cmd, str));
}

/**
 * @tc.name: SADumperTest012
 * @tc.desc: Test mutilthread for class member variable.
 * @tc.type: FUNC
 */
HWTEST_F(SADumperTest, SADumperTest012, TestSize.Level3)
{
    for (int i = 0; i < THREAD_EXECUTE_NUM; i++) {
        std::thread([&]() mutable {
            std::string cmd = "hidumper --cpuusage 1";
            std::string str = "Pss";
            ASSERT_TRUE(HidumperTestUtils::GetInstance().IsExistInCmdResult(cmd, str));
        }).detach();
        std::thread([&]() mutable {
            std::string cmd = "hidumper -s 1201";
            std::string str = "Plugins";
            ASSERT_TRUE(HidumperTestUtils::GetInstance().IsExistInCmdResult(cmd, str));
        }).detach();
    }
}
} // namespace HiviewDFX
} // namespace OHOS