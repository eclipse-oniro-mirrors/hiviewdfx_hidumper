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
#include <fcntl.h>
#include <vector>
#include <unistd.h>
#include <cstdlib>
#include <gtest/gtest.h>
#include "dump_client_main.h"
#include "dump_controller.h"
#include "dump_manager_client.h"
using namespace testing::ext;
using namespace std;
using OHOS::HiviewDFX::DumpClientMain;
namespace OHOS {
namespace HiviewDFX {
const std::string TOOL_NAME = "hidumper";
class HidumperClientTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void HidumperClientTest::SetUpTestCase(void)
{
}

void HidumperClientTest::TearDownTestCase(void)
{
}

void HidumperClientTest::SetUp(void)
{
}

void HidumperClientTest::TearDown(void)
{
}

/**
 * @tc.name: ClientMainTest001
 * @tc.desc: Test too many arguments.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperClientTest, ClientMainTest001, TestSize.Level0)
{
    char *argv[] = {
        const_cast<char *>(TOOL_NAME.c_str()),
    };
    int argc = ARG_MAX_COUNT + 1;
    int ret = DumpClientMain::GetInstance().Main(argc, argv, STDOUT_FILENO);
    ASSERT_EQ(ret, DumpStatus::DUMP_INVALID_ARG);
}

/**
 * @tc.name: MemoryDumperTest002
 * @tc.desc: Test empty argument.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperClientTest, ClientTest002, TestSize.Level0)
{
    char *argv[] = {
        const_cast<char *>(TOOL_NAME.c_str()),
        const_cast<char *>("--mem"),
        const_cast<char *>("-1"),
        const_cast<char *>(""),
        const_cast<char *>("--mem"),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    int ret = DumpClientMain::GetInstance().Main(argc, argv, STDOUT_FILENO);
    ASSERT_EQ(ret, DumpStatus::DUMP_INVALID_ARG);
}

/**
 * @tc.name: ClientMainTest003
 * @tc.desc: Test too long argument .
 * @tc.type: FUNC
 */
HWTEST_F(HidumperClientTest, ClientMainTest003, TestSize.Level0)
{
    std::string longArg;
    longArg.assign(SINGLE_ARG_MAXLEN + 1, 'c');
    char *argv[] = {
        const_cast<char *>(TOOL_NAME.c_str()),
        const_cast<char *>("-h"),
        const_cast<char *>(longArg.c_str()),
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    int ret = DumpClientMain::GetInstance().Main(argc, argv, STDOUT_FILENO);
    ASSERT_EQ(ret, DumpStatus::DUMP_INVALID_ARG);
}

/**
 * @tc.name: ClientMainTest004
 * @tc.desc: Test null argument.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperClientTest, ClientMainTest004, TestSize.Level0)
{
    char *argv[] = {
        const_cast<char *>(TOOL_NAME.c_str()),
        nullptr,
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    int ret = DumpClientMain::GetInstance().Main(argc, argv, STDOUT_FILENO);
    ASSERT_EQ(ret, DumpStatus::DUMP_INVALID_ARG);
}

/**
 * @tc.name: ClientMainTest005
 * @tc.desc: Test null fd.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperClientTest, ClientMainTest005, TestSize.Level0)
{
    char *argv[] = {
        const_cast<char *>("hidumper"),
        const_cast<char *>("--mem"),
        const_cast<char *>("1"),
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    int fd = open("/dev/null", O_RDWR | O_CREAT | O_TRUNC, 0664);
    if (fd <= 0) {
        fd = STDERR_FILENO;
    }
    int ret = DumpClientMain::GetInstance().Main(argc, argv, fd);
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
}

/**
 * @tc.name: ManagerClientTest001
 * @tc.desc: Test emtpy argument list.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperClientTest, ManagerClientTest001, TestSize.Level0)
{
    vector<u16string> args{};
    int ret = DumpManagerClient::GetInstance().Request(args, STDOUT_FILENO);
    ASSERT_EQ(ret, DumpStatus::DUMP_FAIL);
}

/**
 * @tc.name: ManagerClientTest002
 * @tc.desc: Test emtpy argument.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperClientTest, ManagerClientTest002, TestSize.Level0)
{
    vector<u16string> args{
        std::u16string(),
    };
    int ret = DumpManagerClient::GetInstance().Request(args, STDOUT_FILENO);
    ASSERT_EQ(ret, DumpStatus::DUMP_FAIL);
}
} // namespace HiviewDFX
} // namespace OHOS