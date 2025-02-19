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

#include "executor/memory/get_heap_info.h"
#include <gtest/gtest.h>
#include <unistd.h>

using namespace testing::ext;
namespace OHOS {
namespace HiviewDFX {
class GetHeapInfoTest : public testing::Test {
public:
    using ValueMap = std::map<std::string, uint64_t>;
    using GroupMap = std::map<std::string, ValueMap>;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void GetHeapInfoTest::SetUpTestCase(void)
{
}
void GetHeapInfoTest::TearDownTestCase(void)
{
}
void GetHeapInfoTest::SetUp(void)
{
}
void GetHeapInfoTest::TearDown(void)
{
}

/**
 * @tc.name: GetHeapInfo001
 * @tc.desc: Test appManager is nullptr .
 * @tc.type: FUNC
 */
HWTEST_F(GetHeapInfoTest, GetHeapInfo001, TestSize.Level1)
{
    std::unique_ptr<MallHeapInfo> mallocHeapInfo = std::make_unique<MallHeapInfo>();
    std::unique_ptr<GetHeapInfo> getHeapInfo = std::make_unique<GetHeapInfo>();
    getHeapInfo->GetMallocHeapInfo(1, mallocHeapInfo);
    ASSERT_TRUE(mallocHeapInfo != nullptr);
}
} // namespace HiviewDFX
} // namespace OHOS
