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

using namespace testing::ext;
namespace OHOS {
namespace HiviewDFX {
class HidumperPrivacyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    bool IsExistInCmdResult(const std::string &cmd, const std::string &str);

    const int ROOT_UID = 0;
    const int SYSTEM_UID = 1000;
    const int SHELL_UID = 2000;
    const std::string KEY_WORD = "Usage";
    const std::string CMD = "hidumper -s 10";
};

void HidumperPrivacyTest::SetUpTestCase(void)
{
}
void HidumperPrivacyTest::TearDownTestCase(void)
{
}
void HidumperPrivacyTest::SetUp(void)
{
}
void HidumperPrivacyTest::TearDown(void)
{
}

bool HidumperPrivacyTest::IsExistInCmdResult(const std::string &cmd, const std::string &str)
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
 * @tc.name: HidumperPrivacyTest001
 * @tc.desc: When uid=root and apl=2,the content of sa can be obtained.
 * @tc.type: FUNC
 * @tc.require: issueI5GXTG
 */
HWTEST_F(HidumperPrivacyTest, HidumperPrivacyTest001, TestSize.Level3)
{
    setuid(ROOT_UID);
    ASSERT_TRUE(IsExistInCmdResult(CMD, KEY_WORD));
}
} // namespace HiviewDFX
} // namespace OHOS
