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
#include <map>
#include <unistd.h>
#include <vector>
#include "executor/memory/get_hardware_info.h"
#include "executor/memory/get_process_info.h"
#include "executor/memory/get_kernel_info.h"
#include "executor/memory/memory_info.h"
#include "executor/memory/memory_filter.h"
#include "executor/memory/memory_util.h"
#include "executor/memory/parse/parse_meminfo.h"
#include "executor/memory/parse/parse_smaps_info.h"
#include "executor/memory/parse/parse_smaps_rollup_info.h"
#include "executor/memory/smaps_memory_info.h"

using namespace std;
using namespace testing::ext;
namespace OHOS {
namespace HiviewDFX {
const int INIT_PID = 1;
const uint64_t INVALID_PID = 0;
const int BUFFER_SIZE = 1024;
string NULL_STR = "";

using ValueMap = std::map<std::string, uint64_t>;
using GroupMap = std::map<std::string, ValueMap>;
using StringMatrix = std::shared_ptr<std::vector<std::vector<std::string>>>;

class HidumperMemoryTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void HidumperMemoryTest::SetUpTestCase(void)
{
}
void HidumperMemoryTest::TearDownTestCase(void)
{
}
void HidumperMemoryTest::SetUp(void)
{
}
void HidumperMemoryTest::TearDown(void)
{
}

/**
 * @tc.name: MemoryParse001
 * @tc.desc: Test ParseMeminfo invalid ret.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperMemoryTest, MemoryParse001, TestSize.Level1)
{
    unique_ptr<OHOS::HiviewDFX::ParseMeminfo> parseMeminfo = make_unique<OHOS::HiviewDFX::ParseMeminfo>();
    ValueMap result;
    parseMeminfo->SetData("", result);
    ASSERT_EQ(result.size(), 0);
}

/**
 * @tc.name: ParseSmapsInfo001
 * @tc.desc: Test parseSmapsInfo invalid ret.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperMemoryTest, ParseSmapsInfo001, TestSize.Level1)
{
    unique_ptr<OHOS::HiviewDFX::ParseSmapsInfo> parseSmapsInfo = make_unique<OHOS::HiviewDFX::ParseSmapsInfo>();
    uint64_t pid = 0;
    ASSERT_FALSE(parseSmapsInfo->GetHasPidValue("RSS", NULL_STR, pid));
    ASSERT_FALSE(parseSmapsInfo->GetHasPidValue("PSS", NULL_STR, pid));
    ASSERT_FALSE(parseSmapsInfo->GetHasPidValue("Size", NULL_STR, pid));
    ASSERT_FALSE(parseSmapsInfo->GetSmapsValue(MemoryFilter::MemoryType::NOT_SPECIFIED_PID, NULL_STR,
        NULL_STR, pid));
    GroupMap groupMapResult;
    std::vector<std::map<std::string, std::string>> vectMap;
    parseSmapsInfo->memMap_.clear();
    ASSERT_FALSE(parseSmapsInfo->ShowSmapsData(OHOS::HiviewDFX::MemoryFilter::MemoryType::NOT_SPECIFIED_PID,
        0, groupMapResult, false, vectMap));
    ASSERT_TRUE(parseSmapsInfo->ShowSmapsData(OHOS::HiviewDFX::MemoryFilter::MemoryType::NOT_SPECIFIED_PID,
        INIT_PID, groupMapResult, true, vectMap));
    parseSmapsInfo->memMap_.clear();
    parseSmapsInfo->SetMapByNameLine("", "");
    ASSERT_TRUE(parseSmapsInfo->memMap_.size() == 1);
}

/**
 * @tc.name: ParseSmapsRollupInfo001
 * @tc.desc: Test ParseSmapsRollupInfo invalid ret.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperMemoryTest, ParseSmapsRollupInfo001, TestSize.Level1)
{
    unique_ptr<OHOS::HiviewDFX::ParseSmapsRollupInfo> parseSmapsRollup =
        make_unique<OHOS::HiviewDFX::ParseSmapsRollupInfo>();
    MemInfoData::MemInfo memInfo;
    parseSmapsRollup->GetValue("RSS", memInfo);
    parseSmapsRollup->GetValue("PSS", memInfo);
    parseSmapsRollup->GetValue("Size", memInfo);
    ASSERT_TRUE(memInfo.rss == 0);
}

/**
 * @tc.name: SmapsMemoryInfo001
 * @tc.desc: Test SmapsMemoryInfo ret.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperMemoryTest, SmapsMemoryInfo001, TestSize.Level1)
{
    shared_ptr<OHOS::HiviewDFX::SmapsMemoryInfo> smapsMemoryInfo =
        make_shared<OHOS::HiviewDFX::SmapsMemoryInfo>();
    shared_ptr<vector<vector<string>>> result = make_shared<vector<vector<string>>>();
    ASSERT_TRUE(smapsMemoryInfo->ShowMemorySmapsByPid(INIT_PID, result, true));
    ASSERT_FALSE(smapsMemoryInfo->ShowMemorySmapsByPid(INVALID_PID, result, true));
}

/**
 * @tc.name: GetHardwareInfo001
 * @tc.desc: Test GetHardwareInfo ret.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperMemoryTest, GetHardwareInfo001, TestSize.Level1)
{
    unique_ptr<OHOS::HiviewDFX::GetHardwareInfo> getHardwareInfo =
        make_unique<OHOS::HiviewDFX::GetHardwareInfo>();
    getHardwareInfo->GetResverRegPath(NULL_STR);
    vector<string> paths;
    uint64_t value = getHardwareInfo->CalcHardware(paths);
    ASSERT_TRUE(value == 0);
    size_t groupSize = 0;
    getHardwareInfo->GetGroupOfPaths(groupSize, groupSize, paths, paths);
    ASSERT_TRUE(paths.size() == 0);
}

/**
 * @tc.name: MemoryInfo001
 * @tc.desc: Test MemoryInfo ret.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperMemoryTest, MemoryInfo001, TestSize.Level1)
{
    unique_ptr<OHOS::HiviewDFX::MemoryInfo> memoryInfo =
        make_unique<OHOS::HiviewDFX::MemoryInfo>();
    int value = static_cast<int>(memoryInfo->GetProcValue(INVALID_PID, NULL_STR));
    ASSERT_EQ(value, 0);
    value = static_cast<int>(memoryInfo->GetVss(INVALID_PID));
    ASSERT_EQ(value, 0);
    MemInfoData::MemUsage usage;
    OHOS::HiviewDFX::DmaInfo dmaInfo;
    ASSERT_FALSE(memoryInfo->GetMemByProcessPid(INVALID_PID, usage));
#ifdef HIDUMPER_MEMMGR_ENABLE
    memoryInfo->GetProcessAdjLabel(INVALID_PID);
#endif
}

/**
 * @tc.name: MemoryInfo002
 * @tc.desc: Test MemoryInfo ret.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperMemoryTest, MemoryInfo002, TestSize.Level1)
{
    unique_ptr<OHOS::HiviewDFX::MemoryInfo> memoryInfo =
        make_unique<OHOS::HiviewDFX::MemoryInfo>();
    shared_ptr<vector<vector<string>>> result = make_shared<vector<vector<string>>>();
    ValueMap memInfo;
    memoryInfo->GetPurgTotal(memInfo, result);
    ASSERT_TRUE(memInfo.size() == 0);
}

/**
 * @tc.name: MemoryInfo003
 * @tc.desc: Test MemoryInfo BuildResult for error pageTag.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperMemoryTest, MemoryInfo003, TestSize.Level1)
{
    unique_ptr<OHOS::HiviewDFX::MemoryInfo> memoryInfo =
        make_unique<OHOS::HiviewDFX::MemoryInfo>();
    shared_ptr<vector<vector<string>>> result = make_shared<vector<vector<string>>>();
    GroupMap infos;
    ValueMap memInfo;
    infos.insert(pair<string, ValueMap>("test", memInfo));
    memoryInfo->BuildResult(infos, result);
    ASSERT_TRUE(result->size() != 0);
}

/**
 * @tc.name: GetProcessInfo001
 * @tc.desc: Test GetProcessInfo ret.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperMemoryTest, GetProcessInfo001, TestSize.Level1)
{
    unique_ptr<OHOS::HiviewDFX::GetProcessInfo> getProcessInfo = make_unique<OHOS::HiviewDFX::GetProcessInfo>();
    GroupMap groupMap;
    ValueMap memInfo;
    uint64_t pid = 0;
    memInfo.insert(pair<string, uint64_t>("Name", pid));
    groupMap.insert(pair<string, ValueMap>("test", memInfo));
    int value = static_cast<int>(getProcessInfo->GetProcess(groupMap));
    ASSERT_EQ(value, 0);
}

/**
 * @tc.name: GetKernelInfo001
 * @tc.desc: Test GetKernelInfo ret.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperMemoryTest, GetKernelInfo001, TestSize.Level1)
{
    unique_ptr<OHOS::HiviewDFX::GetKernelInfo> getGetKernelInfo = make_unique<OHOS::HiviewDFX::GetKernelInfo>();
    ValueMap memInfo;
    uint64_t value = 0;
    ASSERT_TRUE(getGetKernelInfo->GetKernel(memInfo, value));
}


/**
 * @tc.name: GraphicMemory001
 * @tc.desc: Test GetGraphicMemory.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperMemoryTest, GraphicMemory001, TestSize.Level1)
{
    shared_ptr<OHOS::HiviewDFX::MemoryInfo> memoryInfo =
        make_shared<OHOS::HiviewDFX::MemoryInfo>();
    MemInfoData::GraphicsMemory graphicsMemory;
    FILE* file = popen("pidof render_service", "r");
    char buffer[BUFFER_SIZE];
    if (file) {
        if (fgets(buffer, sizeof(buffer), file) != nullptr) {};
        pclose(file);
    }
    int pid = strtol(buffer, nullptr, 10);
    int ret = memoryInfo->GetGraphicsMemory(pid, graphicsMemory, GraphicType::GRAPH);
    ASSERT_TRUE(ret);
    memoryInfo->GetGraphicsMemory(pid, graphicsMemory, GraphicType::GL);
    ASSERT_TRUE(ret);
}
} // namespace HiviewDFX
} // namespace OHOS