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
#include "dump_common_utils.h"
using namespace std;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::HiviewDFX;
namespace OHOS {
namespace HiviewDFX {
class DumpCommonUtilsTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DumpCommonUtilsTest::SetUpTestCase(void)
{
}

void DumpCommonUtilsTest::TearDownTestCase(void)
{
}

void DumpCommonUtilsTest::SetUp(void)
{
}

void DumpCommonUtilsTest::TearDown(void)
{
}

/**
 * @tc.name: FindFdClusterStartIndexTest
 * @tc.desc: Output the FD path of clustering rules.
 * @tc.type: FUNC
 */
HWTEST_F(DumpCommonUtilsTest, FindFdClusterStartIndexTest, TestSize.Level3)
{
    const string fdLinkPath1 = "/data/storage/el2/testFd/1/mockfdLinkPath";
    int index1 = DumpCommonUtils::FindFdClusterStartIndex(fdLinkPath1);
    std::string targetPath1(fdLinkPath1, 0, index1);
    printf("targetPath1 %s\n", targetPath1.c_str());
    ASSERT_TRUE(targetPath1 == "/data/storage/el2/testFd/");

    const string fdLinkPath2 = "/data/storage/el2/testFd/a/mockfdLinkPath";
    int index2 = DumpCommonUtils::FindFdClusterStartIndex(fdLinkPath2);
    std::string targetPath2(fdLinkPath2, 0, index2);
    printf("targetPath2 %s\n", targetPath2.c_str());
    ASSERT_TRUE(targetPath2 == "/data/storage/el2/testFd/a/mockfdLinkPath");
}

/**
 * @tc.name: FindNoSandBoxPathIndexTest
 * @tc.desc: Output the sandbox path of clustering rules.
 * @tc.type: FUNC
 */
HWTEST_F(DumpCommonUtilsTest, FindNonSandBoxPathIndexTest, TestSize.Level3)
{
    const string fdLinkPath1 = "/data/storage/ela/testFd/1/mockfdLinkPath";
    int index1 = DumpCommonUtils::FindNonSandBoxPathIndex(fdLinkPath1);
    std::string targetPath1(fdLinkPath1, 0, index1);
    printf("targetPath1 %s\n", targetPath1.c_str());
    ASSERT_TRUE(targetPath1 == "/data/storage/ela/testFd/");

    const string fdLinkPath2 = "/data/storage/ela/testFd/a/mockfdLinkPath";
    int index2 = DumpCommonUtils::FindNonSandBoxPathIndex(fdLinkPath2);
    std::string targetPath2(fdLinkPath2, 0, index2);
    printf("targetPath2 %s\n", targetPath2.c_str());
    ASSERT_TRUE(targetPath2 == "/data/storage/ela/testFd/a/mockfdLinkPath");
}
} // namespace HiviewDFX
} // namespace OHOS