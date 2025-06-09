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
#include <map>
#include <unistd.h>
#include <vector>
#include <v1_0/imemory_tracker_interface.h>

#define private public
#include "executor/memory_dumper.h"
#undef private
#include "dump_client_main.h"
#include "dump_utils.h"
#include "hdf_base.h"
#include "executor/memory/memory_filter.h"
#include "executor/memory/memory_util.h"
#include "hidumper_test_utils.h"
#include "util/string_utils.h"

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
    ASSERT_TRUE(HidumperTestUtils::GetInstance().IsExistInCmdResult(cmd, str));
}

/**
 * @tc.name: MemoryDumperTest002
 * @tc.desc: Test MemoryDumper has DMA group.
 * @tc.type: FUNC
 * @tc.require: issueI5NX04
 */
HWTEST_F(MemoryDumperTest, MemoryDumperTest002, TestSize.Level3)
{
    std::string cmd = "hidumper --mem";
    std::string str = "DMA";
    ASSERT_TRUE(HidumperTestUtils::GetInstance().IsExistInCmdResult(cmd, str));
}

/**
 * @tc.name: MemoryDumperTest003
 * @tc.desc: Test MemoryDumper has correct group.
 * @tc.type: FUNC
 * @tc.require: issueI5NWZQ
 */
HWTEST_F(MemoryDumperTest, MemoryDumperTest003, TestSize.Level3)
{
    std::string cmd = "hidumper --mem 1";
    std::string str = "Total";
    ASSERT_TRUE(HidumperTestUtils::GetInstance().IsExistInCmdResult(cmd, str));
    str = "native heap:";
    ASSERT_TRUE(HidumperTestUtils::GetInstance().IsExistInCmdResult(cmd, str));
    str = "Purgeable:";
    ASSERT_TRUE(HidumperTestUtils::GetInstance().IsExistInCmdResult(cmd, str));
    str = "        PurgSum:0 kB";
    ASSERT_TRUE(HidumperTestUtils::GetInstance().IsExistInCmdResult(cmd, str));
    str = "        PurgPin:0 kB";
    ASSERT_TRUE(HidumperTestUtils::GetInstance().IsExistInCmdResult(cmd, str));
    str = "DMA:";
    ASSERT_TRUE(HidumperTestUtils::GetInstance().IsExistInCmdResult(cmd, str));
}

/**
 * @tc.name: MemoryDumperTest004
 * @tc.desc: Test MemoryDumper has GPU group.
 * @tc.type: FUNC
 * @tc.require: issueI5NWZQ
 */
HWTEST_F(MemoryDumperTest, MemoryDumperTest004, TestSize.Level3)
{
    std::string cmd = "hidumper --mem";
    std::string str = "GPU";
    ASSERT_TRUE(HidumperTestUtils::GetInstance().IsExistInCmdResult(cmd, str));
}

/**
 * @tc.name: MemoryDumperTest005
 * @tc.desc: Test MemoryDumper has Purgeable group.
 * @tc.type: FUNC
 * @tc.require: issueI5NWZQ
 */
HWTEST_F(MemoryDumperTest, MemoryDumperTest005, TestSize.Level3)
{
    std::string cmd = "hidumper --mem";
    std::string str = "Total Purgeable";
    ASSERT_TRUE(HidumperTestUtils::GetInstance().IsExistInCmdResult(cmd, str));
    str = "Total PurgSum";
    ASSERT_TRUE(HidumperTestUtils::GetInstance().IsExistInCmdResult(cmd, str));
    str = "Total PurgPin";
    ASSERT_TRUE(HidumperTestUtils::GetInstance().IsExistInCmdResult(cmd, str));
}

/**
 * @tc.name: MemoryDumperTest006
 * @tc.desc: Test MemoryDumper has Dma/PurgSum/PurgPin group.
 * @tc.type: FUNC
 * @tc.require: issueI5NWZQ
 */
HWTEST_F(MemoryDumperTest, MemoryDumperTest006, TestSize.Level3)
{
    std::string cmd = "hidumper --mem";
    std::string str = "Dma      PurgSum      PurgPin";
    ASSERT_TRUE(HidumperTestUtils::GetInstance().IsExistInCmdResult(cmd, str));
}

/**
 * @tc.name: MemoryDumperTest007
 * @tc.desc: Test zip memory not contain "Total Memory Usage by PID".
 * @tc.type: FUNC
 * @tc.require: issueI5NWZQ
 */
HWTEST_F(MemoryDumperTest, MemoryDumperTest007, TestSize.Level3)
{
    std::string cmd = "hidumper --mem --zip";
    std::string str = "Total Memory Usage by PID";
    ASSERT_FALSE(HidumperTestUtils::GetInstance().IsExistInCmdResult(cmd, str));
}

/**
 * @tc.name: MemoryDumperTest008
 * @tc.desc: Test dma is equal graph.
 * @tc.type: FUNC
 * @tc.require: issueI5NWZQ
 */
HWTEST_F(MemoryDumperTest, MemoryDumperTest008, TestSize.Level3)
{
    auto rsPid = static_cast<int32_t>(HidumperTestUtils::GetInstance().GetPidByName("render_service"));
    int pid = rsPid > 0 ? rsPid : 1;
    ASSERT_GT(pid, 0);
    std::string cmd = "hidumper --mem " + std::to_string(pid);
    std::string str = "Graph";
    std::string result = "";
    ASSERT_TRUE(HidumperTestUtils::GetInstance().GetSpecialLine(cmd, str, result));
    std::string graphPss = HidumperTestUtils::GetInstance().GetValueInLine(result, 1);
    ASSERT_TRUE(IsNumericStr(graphPss));
    str = "Dma";
    result = "";
    ASSERT_TRUE(HidumperTestUtils::GetInstance().GetSpecialLine(cmd, str, result));
    vector<string> values;
    StringUtils::GetInstance().StringSplit(result, ":", values); // Dma:0 kB
    std::string dmaStr = "";
    if (!values.empty() && values.size() >= 2) { // 2: Dma, 0 kB
        dmaStr = values[1];
        if (dmaStr.size() >= 3) {
            dmaStr = dmaStr.substr(0, dmaStr.size() - 4);  // 4: ' kB' + 1(index from to 0,1,2...)
            ASSERT_TRUE(IsNumericStr(dmaStr));
        }
    }
    ASSERT_TRUE(graphPss == dmaStr);
}

/**
 * @tc.name: MemoryDumperTest009
 * @tc.desc: Test GL not out of bounds.
 * @tc.type: FUNC
 * @tc.require: issueI5NWZQ
 */
HWTEST_F(MemoryDumperTest, MemoryDumperTest009, TestSize.Level3)
{
    auto rsPid = static_cast<int32_t>(HidumperTestUtils::GetInstance().GetPidByName("render_service"));
    int pid = rsPid > 0 ? rsPid : 1;
    ASSERT_GT(pid, 0);
    std::string cmd = "hidumper --mem " + std::to_string(pid);
    std::string str = "GL";
    std::string result = "";
    ASSERT_TRUE(HidumperTestUtils::GetInstance().GetSpecialLine(cmd, str, result));
    std::string glPss = HidumperTestUtils::GetInstance().GetValueInLine(result, 1);
    ASSERT_TRUE(IsNumericStr(glPss));
    uint64_t gl = static_cast<uint64_t>(std::stoi(glPss));
    ASSERT_FALSE(gl < 0 || gl > UINT64_MAX);
}

/**
 * @tc.name: MemoryDumperTest010
 * @tc.desc: Test HeapSize for "hidumper --mem `pidof com.ohos.sceneboard`"".
 * @tc.type: FUNC
 * @tc.require: issueI5NWZQ
 */
HWTEST_F(MemoryDumperTest, MemoryDumperTest010, TestSize.Level3)
{
    auto sceneboardPid = static_cast<int32_t>(HidumperTestUtils::GetInstance().GetPidByName("com.ohos.sceneboard"));
    int pid = sceneboardPid > 0 ? sceneboardPid : 1;
    ASSERT_GT(pid, 0);
    std::string cmd = "hidumper --mem " + std::to_string(pid);
    std::string str = "native heap";
    std::string result = "";
    ASSERT_TRUE(HidumperTestUtils::GetInstance().GetSpecialLine(cmd, str, result));
    // 9: HeapSize index
    std::string nativeHeapSizeStr = HidumperTestUtils::GetInstance().GetValueInLine(result, 9);
    ASSERT_TRUE(IsNumericStr(nativeHeapSizeStr));
    if (DumpUtils::IsHmKernel()) {
        uint64_t nativeHeapSize = static_cast<uint64_t>(std::stoi(nativeHeapSizeStr));
        ASSERT_TRUE(nativeHeapSize > 0);
    }
}

/**
 * @tc.name: MemoryDumperTest011
 * @tc.desc: Test MemoryDumper has correct print.
 * @tc.type: FUNC
 * @tc.require: issueI5NWZQ
 */
HWTEST_F(MemoryDumperTest, MemoryDumperTest011, TestSize.Level3)
{
    std::string cmd = "hidumper --mem";
    std::string str = "-------------------------------[memory]-------------------------------";
    ASSERT_TRUE(HidumperTestUtils::GetInstance().IsExistInCmdResult(cmd, str));
    str = "Total Memory Usage by PID:";
    ASSERT_TRUE(HidumperTestUtils::GetInstance().IsExistInCmdResult(cmd, str));
    std::string titleTmp = "PID        Total Pss(xxx in SwapPss)    Total Vss    Total Rss    Total Uss";
    str = titleTmp + "           GL        Graph          Dma      PurgSum      PurgPin     Name";
    ASSERT_TRUE(HidumperTestUtils::GetInstance().IsExistInCmdResult(cmd, str));
    str = "Total Memory Usage by Size:";
    ASSERT_TRUE(HidumperTestUtils::GetInstance().IsExistInCmdResult(cmd, str));
    str = "Total Pss by OOM adjustment:";
    ASSERT_TRUE(HidumperTestUtils::GetInstance().IsExistInCmdResult(cmd, str));
}

/**
 * @tc.name: MemoryDumperTest012
 * @tc.desc: Test --mem-smaps has correct print.
 * @tc.type: FUNC
 * @tc.require: issueI5NWZQ
 */
HWTEST_F(MemoryDumperTest, MemoryDumperTest012, TestSize.Level3)
{
    std::string cmd = "hidumper --mem-smaps 1";
    std::string str = "-------------------------------[memory]-------------------------------";
    ASSERT_TRUE(HidumperTestUtils::GetInstance().IsExistInCmdResult(cmd, str));
    std::string titleTmp = "Size        Rss         Pss         Clean       Dirty       Clean       Dirty";
    str = titleTmp + "       Swap        SwapPss     Counts      Category                         Name";
    ASSERT_TRUE(HidumperTestUtils::GetInstance().IsExistInCmdResult(cmd, str));
    str = ".so                              /system";
    ASSERT_TRUE(HidumperTestUtils::GetInstance().IsExistInCmdResult(cmd, str));
    str = "                                 Summary";
    ASSERT_TRUE(HidumperTestUtils::GetInstance().IsExistInCmdResult(cmd, str));
}

/**
 * @tc.name: MemoryDumperTest013
 * @tc.desc: Test --mem-smaps -v has correct print.
 * @tc.type: FUNC
 * @tc.require: issueI5NWZQ
 */
HWTEST_F(MemoryDumperTest, MemoryDumperTest013, TestSize.Level3)
{
    std::string cmd = "hidumper --mem-smaps 1 -v";
    std::string str = "-------------------------------[memory]-------------------------------";
    ASSERT_TRUE(HidumperTestUtils::GetInstance().IsExistInCmdResult(cmd, str));
    std::string titleTmp = "Size        Rss         Pss         Clean       Dirty       Clean       Dirty       Swap";
    str = titleTmp + "        SwapPss     Perm        Start             End         Category                   Name";
    ASSERT_TRUE(HidumperTestUtils::GetInstance().IsExistInCmdResult(cmd, str));
    str = "dev                        /dev";
    ASSERT_TRUE(HidumperTestUtils::GetInstance().IsExistInCmdResult(cmd, str));
    str = "                                 Summary";
    ASSERT_TRUE(HidumperTestUtils::GetInstance().IsExistInCmdResult(cmd, str));
}

/**
 * @tc.name: MemoryDumperTest014
 * @tc.desc: Test hidumper --mem --prune has correct print.
 * @tc.type: FUNC
 * @tc.require: issueI5NWZQ
 */
HWTEST_F(MemoryDumperTest, MemoryDumperTest014, TestSize.Level3)
{
    std::string cmd = "hidumper --mem --prune";
    std::string str = "-------------------------------[memory]-------------------------------";
    ASSERT_TRUE(HidumperTestUtils::GetInstance().IsExistInCmdResult(cmd, str));
    str = "Total Memory Usage by PID:";
    ASSERT_TRUE(HidumperTestUtils::GetInstance().IsExistInCmdResult(cmd, str));
    str = "PID        Total Pss(xxx in SwapPss)           GL     AdjLabel     Name";
    ASSERT_TRUE(HidumperTestUtils::GetInstance().IsExistInCmdResult(cmd, str));
}

/**
 * @tc.name: MemoryUtilTest001
 * @tc.desc: Test IsNameLine has correct ret.
 * @tc.type: FUNC
 */
HWTEST_F(MemoryDumperTest, MemoryUtilTest001, TestSize.Level1)
{
    const std::string valueLine = "Rss:                  24 kB";
    std::string name;
    uint64_t iNode = 0;
    ASSERT_FALSE(MemoryUtil::GetInstance().IsNameLine(valueLine, name, iNode));
    ASSERT_EQ(name, "");
    const std::string nameLine = "ffb84000-ffba5000 rw-p 00000000 00:00 0                                  [stack]";
    ASSERT_TRUE(MemoryUtil::GetInstance().IsNameLine(nameLine, name, iNode));
    ASSERT_EQ(name, "[stack]");
}

/**
 * @tc.name: MemoryUtilTest002
 * @tc.desc: Test GetTypeAndValue has correct ret.
 * @tc.type: FUNC
 */
HWTEST_F(MemoryDumperTest, MemoryUtilTest002, TestSize.Level1)
{
    std::string type;
    uint64_t value = 0;
    const std::string illegalStr = "aaaaaa";
    ASSERT_FALSE(MemoryUtil::GetInstance().GetTypeAndValue(illegalStr, type, value));
    const std::string valueStr = "MemTotal:        2010244 kB";
    ASSERT_TRUE(MemoryUtil::GetInstance().GetTypeAndValue(valueStr, type, value));
    ASSERT_EQ(type, "MemTotal");
    ASSERT_EQ(value, 2010244);
}

/**
 * @tc.name: MemoryUtilTest003
 * @tc.desc: Test RunCMD has correct ret.
 * @tc.type: FUNC
 */
HWTEST_F(MemoryDumperTest, MemoryUtilTest003, TestSize.Level1)
{
    const std::string cmd = "ps -ef";
    std::vector<std::string> vec;
    ASSERT_TRUE(MemoryUtil::GetInstance().RunCMD(cmd, vec));
    ASSERT_GT(vec.size(), 0);
}

/**
 * @tc.name: MemoryUtilTest004
 * @tc.desc: Test hidumper some cmd.
 * @tc.type: FUNC
 */
HWTEST_F(MemoryDumperTest, MemoryUtilTest004, TestSize.Level1)
{
    std::string cmd = "hidumper --mem-smaps 1";
    std::vector<std::string> vec;
    ASSERT_TRUE(MemoryUtil::GetInstance().RunCMD(cmd, vec));
    cmd = "hidumper --mem-smaps 1 -v";
    ASSERT_TRUE(MemoryUtil::GetInstance().RunCMD(cmd, vec));
    cmd = "hidumper --net 1";
    ASSERT_TRUE(MemoryUtil::GetInstance().RunCMD(cmd, vec));
    cmd = "hidumper --storage 1";
    ASSERT_TRUE(MemoryUtil::GetInstance().RunCMD(cmd, vec));
}

/**
 * @tc.name: MemoryUtilTest005
 * @tc.desc: Test no such pid.
 * @tc.type: FUNC
 */
HWTEST_F(MemoryDumperTest, MemoryUtilTest005, TestSize.Level1)
{
    std::string cmd = "hidumper --mem-smaps 100000";
    std::vector<std::string> vec;
    ASSERT_TRUE(MemoryUtil::GetInstance().RunCMD(cmd, vec));
    cmd = "hidumper --mem-smaps 100000 -v";
    ASSERT_TRUE(MemoryUtil::GetInstance().RunCMD(cmd, vec));
    cmd = "hidumper --cpuusage 100000";
    ASSERT_TRUE(MemoryUtil::GetInstance().RunCMD(cmd, vec));
    cmd = "hidumper --mem 100000";
    ASSERT_TRUE(MemoryUtil::GetInstance().RunCMD(cmd, vec));
    cmd = "hidumper -p 100000";
    ASSERT_TRUE(MemoryUtil::GetInstance().RunCMD(cmd, vec));
    cmd = "hidumper --storage 100000";
    ASSERT_TRUE(MemoryUtil::GetInstance().RunCMD(cmd, vec));
    cmd = "hidumper --net 100000";
    ASSERT_TRUE(MemoryUtil::GetInstance().RunCMD(cmd, vec));
    cmd = "hidumper --mem-jsheap 100000";
    ASSERT_TRUE(MemoryUtil::GetInstance().RunCMD(cmd, vec));
    cmd = "hidumper --mem-jsheap 0";
    ASSERT_TRUE(MemoryUtil::GetInstance().RunCMD(cmd, vec));
    cmd = "hidumper --mem-cjheap 100000";
    ASSERT_TRUE(MemoryUtil::GetInstance().RunCMD(cmd, vec));
    cmd = "hidumper --mem-cjheap 0";
    ASSERT_TRUE(MemoryUtil::GetInstance().RunCMD(cmd, vec));
    cmd = "hidumper --ipc --stat 100000";
    ASSERT_TRUE(MemoryUtil::GetInstance().RunCMD(cmd, vec));
}

/**
 * @tc.name: MemoryUtilTest006
 * @tc.desc: Test hidumper process exit success.
 * @tc.type: FUNC
 */
HWTEST_F(MemoryDumperTest, MemoryUtilTest006, TestSize.Level1)
{
    std::vector<std::string> vec;
    std::string cmd = "hidumper -h";
    ASSERT_TRUE(MemoryUtil::GetInstance().RunCMD(cmd, vec));
    ASSERT_GT(vec.size(), 0);
    vec.clear();
    cmd = "pidof hidumper";
    ASSERT_TRUE(MemoryUtil::GetInstance().RunCMD(cmd, vec));
    ASSERT_EQ(vec.size(), 0);
}
} // namespace HiviewDFX
} // namespace OHOS