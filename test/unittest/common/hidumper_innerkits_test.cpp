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
#include "cpu_dumper.h"
#include "executor/memory/get_heap_info.h"
#include "executor/memory/memory_info.h"
#include "executor/memory/parse/parse_smaps_info.h"
#include "hidumper_test_utils.h"
#include "securec.h"

#include <gtest/gtest.h>
#include <unistd.h>

using namespace testing::ext;
namespace OHOS {
namespace HiviewDFX {
static constexpr int MALLOC_SIZE = 1024;
static constexpr int LAUNCHER_PID_BUFFER_SIZE = 6;
static int g_pid = -1;
static int g_appManagerPid = -1;
const int HIDUMPER_SERVICE_RAM = 20480; //hidumper_service RAM:20MB
class HiDumperInnerkitsTest : public testing::Test {
public:
    using ValueMap = std::map<std::string, uint64_t>;
    using GroupMap = std::map<std::string, ValueMap>;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static void GetAppManagerPids();
    static int GetAppManagerPid(std::string process);
    static void StartTestProcess();
    static void StopProcess();
};

void HiDumperInnerkitsTest::SetUpTestCase(void)
{
    GetAppManagerPids();
    StartTestProcess();
    if (g_pid < 0) {
        printf("[SetUpTestCase] fork process failure!\n");
        return;
    }
    if (g_pid == 0) {
        printf("[SetUpTestCase] this process's pid is %d, it's should be child process\n", getpid());
        return;
    }
}
void HiDumperInnerkitsTest::TearDownTestCase(void)
{
    if (g_pid < 0) {
        printf("[TearDownTestCase] fork process failure!\n");
        return;
    }
    if (g_pid == 0) {
        printf("[TearDownTestCase] this process's pid is %d, it's should be child process\n", getpid());
        return;
    }
    StopProcess();
}
void HiDumperInnerkitsTest::SetUp(void)
{
}
void HiDumperInnerkitsTest::TearDown(void)
{
}

void HiDumperInnerkitsTest::StartTestProcess()
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
            usleep(1);
            free(p);
        }
    } else {
        g_pid = processNum;
    }
}

void HiDumperInnerkitsTest::StopProcess()
{
    std::string stopCmd = "kill " + std::to_string(g_pid);
    system(stopCmd.c_str());
}

void HiDumperInnerkitsTest::GetAppManagerPids()
{
    std::vector<std::string> processes = {"com.ohos.launcher", "com.ohos.medialibrary.medialibrarydata", "hiview",
        "com.ohos.settingsdata", "com.ohos.systemui", "render_service"};
    for (std::vector<std::string>::iterator iter = processes.begin(); iter != processes.end(); iter++) {
        int res = GetAppManagerPid(*iter);
        if (res > 0) {
            g_appManagerPid = res;
            break;
        }
    }
}

int HiDumperInnerkitsTest::GetAppManagerPid(std::string process)
{
    std::string cmd = "pidof " + process;
    char appManagerPidChar[LAUNCHER_PID_BUFFER_SIZE] = {"\0"};
    int appManagerPid = -1;
    FILE *fp = nullptr;
    fp = popen(cmd.c_str(), "r");
    if (fp == nullptr) {
        return -1;
    }
    if (fgets(appManagerPidChar, sizeof(appManagerPidChar), fp) != nullptr) {
        pclose(fp);
        int ret = sscanf_s(appManagerPidChar, "%d", &appManagerPid);
        if (ret <= 0) {
            return -1;
        }
        return appManagerPid;
    }
    pclose(fp);
    return -1;
}

/**
 * @tc.name: GetMemInfoTest001
 * @tc.desc: Test GetMemInfo.
 * @tc.type: FUNC
 * @tc.require: issueI5NWZQ
 */
HWTEST_F(HiDumperInnerkitsTest, GetMemInfoTest001, TestSize.Level1)
{
    std::unique_ptr<DumpUsage> dumpUsage = std::make_unique<DumpUsage>();
    MemInfoData::MemInfo info;
    EXPECT_TRUE(dumpUsage->GetMemInfo(g_pid, info));
    EXPECT_GT(info.pss, 0);
}

/**
 * @tc.name: GetPssTest001
 * @tc.desc: Test GetPss.
 * @tc.type: FUNC
 * @tc.require: issueI5NWZQ
 */
HWTEST_F(HiDumperInnerkitsTest, GetPssTest001, TestSize.Level1)
{
    std::unique_ptr<DumpUsage> dumpUsage = std::make_unique<DumpUsage>();
    EXPECT_GT(dumpUsage->GetPss(g_pid), 0);
}

/**
 * @tc.name: GetPssTest002
 * @tc.desc: Test GetPss for hidumper_service.
 * @tc.type: FUNC
 * @tc.require: issueI5NWZQ
 */
HWTEST_F(HiDumperInnerkitsTest, GetPssTest002, TestSize.Level1)
{
    std::string cmd = "hidumper --mem 1";
    std::string str = "memory";
    ASSERT_TRUE(HidumperTestUtils::GetInstance().IsExistInCmdResult(cmd, str));
    std::unique_ptr<DumpUsage> dumpUsage = std::make_unique<DumpUsage>();
    int hidumperServicePid = GetAppManagerPid("hidumper_service");
    uint64_t pss = dumpUsage->GetPss(hidumperServicePid);
    std::cout << "hidumper_service pss: " << pss << std::endl;
    EXPECT_LE(pss, HIDUMPER_SERVICE_RAM);
}

/**
 * @tc.name: GetPrivateDirtyTest001
 * @tc.desc: Test GetPrivateDirty.
 * @tc.type: FUNC
 * @tc.require: issueI5NWZQ
 */
HWTEST_F(HiDumperInnerkitsTest, GetPrivateDirtyTest001, TestSize.Level1)
{
    std::unique_ptr<DumpUsage> dumpUsage = std::make_unique<DumpUsage>();
    EXPECT_GE(dumpUsage->GetPrivateDirty(g_pid), 0);
}

/**
 * @tc.name: GetSharedDirtyTest001
 * @tc.desc: Test GetSharedDirty.
 * @tc.type: FUNC
 * @tc.require: issueI5NWZQ
 */
HWTEST_F(HiDumperInnerkitsTest, GetSharedDirtyTest001, TestSize.Level1)
{
    std::unique_ptr<DumpUsage> dumpUsage = std::make_unique<DumpUsage>();
    EXPECT_GE(dumpUsage->GetSharedDirty(g_pid), 0);
}

/**
 * @tc.name: GetCpuUsage001
 * @tc.desc: Test GetCpuUsage.
 * @tc.type: FUNC
 * @tc.require: issueI5NWZQ
 */
#ifdef HIDUMPER_HIVIEWDFX_HIVIEW_ENABLE
HWTEST_F(HiDumperInnerkitsTest, GetCpuUsage001, TestSize.Level1)
{
    std::unique_ptr<DumpUsage> dumpUsage = std::make_unique<DumpUsage>();
    EXPECT_GE(dumpUsage->GetCpuUsage(g_pid), 0);
}
#endif

/**
 * @tc.name: GetProcCpuInfo001
 * @tc.desc: Test GetProcCpuInfo when a new process appeared.
 * @tc.type: FUNC
 */
#ifdef HIDUMPER_HIVIEWDFX_HIVIEW_ENABLE
HWTEST_F(HiDumperInnerkitsTest, GetProcCpuInfo001, TestSize.Level1)
{
    auto parameter = std::make_shared<DumperParameter>();
    DumperOpts opts;
    opts.isDumpCpuUsage_ = true;
    opts.cpuUsagePid_ = g_pid;
    parameter->SetOpts(opts);
    auto dumpDatas = std::make_shared<std::vector<std::vector<std::string>>>();
    auto cpuDumper = std::make_shared<CPUDumper>();
    int ret = DumpStatus::DUMP_FAIL;
    ret = cpuDumper->PreExecute(parameter, dumpDatas);
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
    ret = cpuDumper->Execute();
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
    ret = cpuDumper->AfterExecute();
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
}
#endif

/**
 * @tc.name: GetHeapInfo001
 * @tc.desc: Test GetHeapInfo001.
 * @tc.type: FUNC
 */
HWTEST_F(HiDumperInnerkitsTest, GetHeapInfo001, TestSize.Level1)
{
    int testPid = g_pid;
    std::unique_ptr<MallHeapInfo> mallocHeapInfo = std::make_unique<MallHeapInfo>();
    std::unique_ptr<GetHeapInfo> getHeapInfo = std::make_unique<GetHeapInfo>();
    getHeapInfo->GetMallocHeapInfo(testPid, mallocHeapInfo);

    if (g_appManagerPid != -1) {
        testPid = g_appManagerPid;
        getHeapInfo->GetMallocHeapInfo(testPid, mallocHeapInfo);
    }
    ASSERT_TRUE(mallocHeapInfo);
}

/**
 * @tc.name: ParseSmapsInfoGetInfoTest001
 * @tc.desc: Test GetMemInfo.
 * @tc.type: FUNC
 * @tc.require: issueI5NWZQ
 */
HWTEST_F(HiDumperInnerkitsTest, ParseSmapsInfoGetInfoTest001, TestSize.Level1)
{
    std::unique_ptr<ParseSmapsInfo> parseSmapsInfo = std::make_unique<ParseSmapsInfo>();
    GroupMap groupMap;
    GroupMap nativeGroupMap;
    EXPECT_TRUE(parseSmapsInfo->GetInfo(MemoryFilter::APPOINT_PID, 1, nativeGroupMap, groupMap));
}
} // namespace HiviewDFX
} // namespace OHOS
