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

#include "dump_usage.h"

#include <gtest/gtest.h>
#include <unistd.h>

using namespace testing::ext;
namespace OHOS {
namespace HiviewDFX {
static constexpr int MALLOC_SIZE = 1024;
class HiDumperInnerkitsTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    void StartTestProcess(int& pid);
    void StopProcess(int processNum);
};

void HiDumperInnerkitsTest::SetUpTestCase(void)
{
}
void HiDumperInnerkitsTest::TearDownTestCase(void)
{
}
void HiDumperInnerkitsTest::SetUp(void)
{
}
void HiDumperInnerkitsTest::TearDown(void)
{
}

void HiDumperInnerkitsTest::StartTestProcess(int& pid)
{
    int processNum = fork();
    if (processNum == 0) {
        while (true) {
            void* p = malloc(MALLOC_SIZE);
            if (p == nullptr) {
                const int bufSize = 256;
                char buf[bufSize] = { 0 };
                (void)strerror_r(errno, buf, bufSize);
                printf("malloc failure, errno(%d:%s)", errno, buf);
                return;
            }
            free(p);
        }
    } else {
        pid = processNum;
    }
}

void HiDumperInnerkitsTest::StopProcess(int processNum)
{
    std::string stopCmd = "kill -9 " + std::to_string(processNum);
    system(stopCmd.c_str());
}

/**
 * @tc.name: GetMemInfoTest001
 * @tc.desc: Test GetMemInfo.
 * @tc.type: FUNC
 * @tc.require: issueI5NWZQ
 */
HWTEST_F(HiDumperInnerkitsTest, GetMemInfoTest001, TestSize.Level1)
{
    int pid = -1;
    StartTestProcess(pid);
    if (pid < 0) {
        printf("fork process failure!");
        return;
    }
    std::unique_ptr<DumpUsage> dumpUsage = std::make_unique<DumpUsage>();
    MemInfoData::MemInfo info;
    EXPECT_TRUE(dumpUsage->GetMemInfo(pid, info));
    EXPECT_GT(info.pss, 0);
    StopProcess(pid);
}

/**
 * @tc.name: GetPssTest001
 * @tc.desc: Test GetPss.
 * @tc.type: FUNC
 * @tc.require: issueI5NWZQ
 */
HWTEST_F(HiDumperInnerkitsTest, GetPssTest001, TestSize.Level1)
{
    int pid = -1;
    StartTestProcess(pid);
    if (pid < 0) {
        printf("fork process failure!");
        return;
    }
    std::unique_ptr<DumpUsage> dumpUsage = std::make_unique<DumpUsage>();
    EXPECT_GT(dumpUsage->GetPss(pid), 0);
    StopProcess(pid);
}

/**
 * @tc.name: GetPrivateDirtyTest001
 * @tc.desc: Test GetPrivateDirty.
 * @tc.type: FUNC
 * @tc.require: issueI5NWZQ
 */
HWTEST_F(HiDumperInnerkitsTest, GetPrivateDirtyTest001, TestSize.Level1)
{
    int pid = getpid();
    std::unique_ptr<DumpUsage> dumpUsage = std::make_unique<DumpUsage>();
    EXPECT_GE(dumpUsage->GetPrivateDirty(pid), 0);
}

/**
 * @tc.name: GetSharedDirtyTest001
 * @tc.desc: Test GetSharedDirty.
 * @tc.type: FUNC
 * @tc.require: issueI5NWZQ
 */
HWTEST_F(HiDumperInnerkitsTest, GetSharedDirtyTest001, TestSize.Level1)
{
    int pid = getpid();
    std::unique_ptr<DumpUsage> dumpUsage = std::make_unique<DumpUsage>();
    EXPECT_GE(dumpUsage->GetSharedDirty(pid), 0);
}

/**
 * @tc.name: GetCpuUsage001
 * @tc.desc: Test GetCpuUsage.
 * @tc.type: FUNC
 * @tc.require: issueI5NWZQ
 */
HWTEST_F(HiDumperInnerkitsTest, GetCpuUsage001, TestSize.Level1)
{
    int pid = -1;
    StartTestProcess(pid);
    if (pid < 0) {
        printf("fork process failure!");
        return;
    }
    sleep(1);
    std::unique_ptr<DumpUsage> dumpUsage = std::make_unique<DumpUsage>();
    EXPECT_GT(dumpUsage->GetCpuUsage(pid), 0);
    StopProcess(pid);
}
} // namespace HiviewDFX
} // namespace OHOS
