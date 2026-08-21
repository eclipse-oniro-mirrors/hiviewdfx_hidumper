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
#include "executor/memory/dump_arkts_heap_info.h"

using namespace testing::ext;
namespace OHOS {
namespace HiviewDFX {
class DumpArktsHeapInfoTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: GetArktsHeapSizeTimeout_001
 * @tc.desc: Test GetArktsHeapSize returns false on IPC failure/timeout.
 * @tc.type: FUNC
 */
HWTEST_F(DumpArktsHeapInfoTest, GetArktsHeapSizeTimeout_001, TestSize.Level1)
{
    DumpArktsHeapInfo info;
    std::string result;
    bool ret = info.GetArktsHeapSize(1, result);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: GetArktsHeapSizeParseFail_001
 * @tc.desc: Test GetArktsHeapSize returns false on parse failure.
 * @tc.type: FUNC
 */
HWTEST_F(DumpArktsHeapInfoTest, GetArktsHeapSizeParseFail_001, TestSize.Level1)
{
    DumpArktsHeapInfo info;
    std::string result;
    bool ret = info.GetArktsHeapSize(99999, result);
    EXPECT_FALSE(ret);
}
} // namespace HiviewDFX
} // namespace OHOS
