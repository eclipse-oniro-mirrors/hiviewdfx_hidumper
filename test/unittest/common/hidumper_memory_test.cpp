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
#include <iostream>
#include <map>
#include <sstream>
#include <unistd.h>
#include <vector>
#include "dump_utils.h"
#include "executor/memory/get_hardware_info.h"
#include "executor/memory/get_process_info.h"
#include "executor/memory/get_kernel_info.h"
#include "executor/memory/memory_info.h"
#include "executor/memory/memory_filter.h"
#include "executor/memory/memory_util.h"
#include "executor/memory/parse/parse_ashmem_info.h"
#include "executor/memory/parse/parse_meminfo.h"
#include "executor/memory/parse/parse_smaps_info.h"
#include "executor/memory/parse/parse_smaps_rollup_info.h"
#include "executor/memory/smaps_memory_info.h"
#include "hidumper_test_utils.h"
#include "memory_collector.h"
#include "meminfo.h"
#include "string_ex.h"

using namespace std;
using namespace testing::ext;
namespace OHOS {
namespace HiviewDFX {
const int INIT_PID = 1;
const int INDEX = 1;
const uint64_t INVALID_PID = 0;
const int BUFFER_SIZE = 1024;
constexpr int TEST_PSS_VALUE = 100;
const std::vector<int> pssValues = {1000, 1050, 1020, 1030, 1010, 1020, 1040, 1050, 1030, 1060, 1010};
string NULL_STR = "";
const string FIRST_ROW_TMP = "                          Pss         Shared         Shared        Private";
const string FIRST_ROW = FIRST_ROW_TMP +
    "        Private           Swap        SwapPss           Heap           Heap           Heap ";
const string SECOND_ROW_TMP =
    "                        Total          Clean          Dirty          Clean          Dirty";
const string SECOND_ROW = SECOND_ROW_TMP +
    "          Total          Total           Size          Alloc           Free ";
const string THIRD_ROW_TMP =
    "                       ( kB )         ( kB )         ( kB )         ( kB )         ( kB )";
const string THIRD_ROW = THIRD_ROW_TMP +
    "         ( kB )         ( kB )         ( kB )         ( kB )         ( kB ) ";
const string FOURTH_ROW_TMP =
    "              ---------------------------------------------------------------------------";
const string FOURTH_ROW = FOURTH_ROW_TMP +
    "---------------------------------------------------------------------------";
const vector<string> MEMEORY_TITLE_VEC = {FIRST_ROW, SECOND_ROW, THIRD_ROW, FOURTH_ROW};

const std::map<MemoryItemType, MemoryClass> TYPE_TO_CLASS_MAP = {
    {MemoryItemType::MEMORY_ITEM_ENTITY_DB, MemoryClass::MEMORY_CLASS_DB},
    {MemoryItemType::MEMORY_ITEM_ENTITY_DB_SHM, MemoryClass::MEMORY_CLASS_DB},
    {MemoryItemType::MEMORY_ITEM_ENTITY_HAP, MemoryClass::MEMORY_CLASS_OTHER},
    {MemoryItemType::MEMORY_ITEM_ENTITY_HSP, MemoryClass::MEMORY_CLASS_OTHER},
    {MemoryItemType::MEMORY_ITEM_ENTITY_SO, MemoryClass::MEMORY_CLASS_SO},
    {MemoryItemType::MEMORY_ITEM_ENTITY_SO1, MemoryClass::MEMORY_CLASS_SO},
    {MemoryItemType::MEMORY_ITEM_ENTITY_TTF, MemoryClass::MEMORY_CLASS_TTF},
    {MemoryItemType::MEMORY_ITEM_ENTITY_DEV_PARAMETER, MemoryClass::MEMORY_CLASS_DEV},
    {MemoryItemType::MEMORY_ITEM_ENTITY_DEV_OTHER, MemoryClass::MEMORY_CLASS_DEV},
    {MemoryItemType::MEMORY_ITEM_ENTITY_DATA_STORAGE, MemoryClass::MEMORY_CLASS_HAP},
    {MemoryItemType::MEMORY_ITEM_ENTITY_DMABUF, MemoryClass::MEMORY_CLASS_DMABUF},
    {MemoryItemType::MEMORY_ITEM_ENTITY_OTHER, MemoryClass::MEMORY_CLASS_OTHER},
    {MemoryItemType::MEMORY_ITEM_TYPE_ANON_INODE, MemoryClass::MEMORY_CLASS_OTHER},
    {MemoryItemType::MEMORY_ITEM_TYPE_ANON_ARKTS_CODE, MemoryClass::MEMORY_CLASS_OTHER},
    {MemoryItemType::MEMORY_ITEM_TYPE_ANON_ARKTS_HEAP, MemoryClass::MEMORY_CLASS_ARK_TS_HEAP},
    {MemoryItemType::MEMORY_ITEM_TYPE_ANON_GUARD, MemoryClass::MEMORY_CLASS_GUARD},
    {MemoryItemType::MEMORY_ITEM_TYPE_ANON_BSS, MemoryClass::MEMORY_CLASS_OTHER},
    {MemoryItemType::MEMORY_ITEM_TYPE_ANON_NATIVE_HEAP_BRK, MemoryClass::MEMORY_CLASS_NATIVE_HEAP},
    {MemoryItemType::MEMORY_ITEM_TYPE_ANON_NATIVE_HEAP_JEMALLOC, MemoryClass::MEMORY_CLASS_NATIVE_HEAP},
    {MemoryItemType::MEMORY_ITEM_TYPE_ANON_NATIVE_HEAP_JEMALLOC_META, MemoryClass::MEMORY_CLASS_NATIVE_HEAP},
    {MemoryItemType::MEMORY_ITEM_TYPE_ANON_NATIVE_HEAP_JEMALLOC_TSD, MemoryClass::MEMORY_CLASS_NATIVE_HEAP},
    {MemoryItemType::MEMORY_ITEM_TYPE_ANON_NATIVE_HEAP_META, MemoryClass::MEMORY_CLASS_NATIVE_HEAP},
    {MemoryItemType::MEMORY_ITEM_TYPE_ANON_NATIVE_HEAP_MMAP, MemoryClass::MEMORY_CLASS_NATIVE_HEAP},
    {MemoryItemType::MEMORY_ITEM_TYPE_ANON_NATIVE_HEAP_OTHER, MemoryClass::MEMORY_CLASS_NATIVE_HEAP},
    {MemoryItemType::MEMORY_ITEM_TYPE_ANON_SIGNAL_STACK, MemoryClass::MEMORY_CLASS_STACK},
    {MemoryItemType::MEMORY_ITEM_TYPE_ANON_STACK, MemoryClass::MEMORY_CLASS_STACK},
    {MemoryItemType::MEMORY_ITEM_TYPE_ANON_V8, MemoryClass::MEMORY_CLASS_OTHER},
    {MemoryItemType::MEMORY_ITEM_TYPE_ANONYMOUS_OTHER, MemoryClass::MEMORY_CLASS_OTHER},
    {MemoryItemType::MEMORY_ITEM_TYPE_CONTIGUOUS, MemoryClass::MEMORY_CLASS_OTHER},
    {MemoryItemType::MEMORY_ITEM_TYPE_COPAGE, MemoryClass::MEMORY_CLASS_OTHER},
    {MemoryItemType::MEMORY_ITEM_TYPE_FILE, MemoryClass::MEMORY_CLASS_OTHER},
    {MemoryItemType::MEMORY_ITEM_TYPE_GUARD, MemoryClass::MEMORY_CLASS_OTHER},
    {MemoryItemType::MEMORY_ITEM_TYPE_HEAP, MemoryClass::MEMORY_CLASS_NATIVE_HEAP},
    {MemoryItemType::MEMORY_ITEM_TYPE_IO, MemoryClass::MEMORY_CLASS_OTHER},
    {MemoryItemType::MEMORY_ITEM_TYPE_KSHARE, MemoryClass::MEMORY_CLASS_OTHER},
    {MemoryItemType::MEMORY_ITEM_TYPE_MALLOC, MemoryClass::MEMORY_CLASS_NATIVE_HEAP},
    {MemoryItemType::MEMORY_ITEM_TYPE_PREHISTORIC, MemoryClass::MEMORY_CLASS_OTHER},
    {MemoryItemType::MEMORY_ITEM_TYPE_RESERVE, MemoryClass::MEMORY_CLASS_OTHER},
    {MemoryItemType::MEMORY_ITEM_TYPE_SHMM, MemoryClass::MEMORY_CLASS_OTHER},
    {MemoryItemType::MEMORY_ITEM_TYPE_STACK, MemoryClass::MEMORY_CLASS_STACK},
    {MemoryItemType::MEMORY_ITEM_TYPE_UNKNOWN, MemoryClass::MEMORY_CLASS_OTHER},
    {MemoryItemType::MEMORY_ITEM_TYPE_VNODES, MemoryClass::MEMORY_CLASS_OTHER},
    {MemoryItemType::MEMORY_ITEM_TYPE_OTHER, MemoryClass::MEMORY_CLASS_OTHER},
    {MemoryItemType::MEMORY_ITEM_TYPE_GRAPH_GL, MemoryClass::MEMORY_CLASS_GRAPH},
    {MemoryItemType::MEMORY_ITEM_TYPE_GRAPH_GRAPHICS, MemoryClass::MEMORY_CLASS_GRAPH},
};

const std::vector<std::pair<std::string, std::string>> TITLE_AND_VALUE = {
    {"GL", "100"},
    {"Graph", "100"},
    {"ark ts heap", "100"},
    {"guard", "100"},
    {"native heap", "900"},
    {".hap", "100"},
    {"AnonPage other", "1100"},
    {"stack", "300"},
    {".db", "200"},
    {".so", "200"},
    {"dev", "200"},
    {"dmabuf", "100"},
    {".ttf", "100"},
    {"FilePage other", "900"},
    {"---------------", ""},
    {"Total", "4500"},
    {"\n", ""},
    {"native heap:", ""},
    {"heap", "100"},
    {"jemalloc meta", "100"},
    {"jemalloc heap", "100"},
    {"brk heap", "100"},
    {"musl heap", "100"},
    {"mmap heap", "100"},
};

const std::vector<std::string> DMA_SHOW_TITLES = {
    "Process", "pid", "fd", "size_bytes", "ino", "exp_pid",
    "exp_task_comm", "buf_name", "exp_name", "buf_type", "leak_type"
};
using ValueMap = std::map<std::string, uint64_t>;
using GroupMap = std::map<std::string, ValueMap>;
using StringMatrix = std::shared_ptr<std::vector<std::vector<std::string>>>;

class HidumperMemoryTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    void CreateProcessMemoryDetail(ProcessMemoryDetail& processMemoryDetail);
    void GetMemoryDetail(MemoryDetail& detail);
    bool CheckMemoryPrint(const std::string& title, const std::string& value, const std::string& line);
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

void HidumperMemoryTest::CreateProcessMemoryDetail(ProcessMemoryDetail& processMemoryDetail)
{
    processMemoryDetail.pid = -1;
    processMemoryDetail.name = "test";
    std::vector<MemoryDetail> details;
    for (int i = 0; i <= static_cast<int>(MemoryClass::MEMORY_CLASS_OTHER); i++) {
        MemoryDetail detail;
        detail.memoryClass = static_cast<MemoryClass>(i);
        GetMemoryDetail(detail);
        details.push_back(detail);
        processMemoryDetail.totalRss += detail.totalRss;
        processMemoryDetail.totalPss += detail.totalPss;
        processMemoryDetail.totalSwapPss += detail.totalSwapPss;
        processMemoryDetail.totalSwap += detail.totalSwap;
        processMemoryDetail.totalAllPss += detail.totalAllPss;
        processMemoryDetail.totalAllSwap += detail.totalAllSwap;
        processMemoryDetail.totalSharedDirty += detail.totalSharedDirty;
        processMemoryDetail.totalPrivateDirty += detail.totalPrivateDirty;
        processMemoryDetail.totalSharedClean += detail.totalSharedClean;
        processMemoryDetail.totalPrivateClean += detail.totalPrivateClean;
    }
    processMemoryDetail.details = details;
}

void HidumperMemoryTest::GetMemoryDetail(MemoryDetail& detail)
{
    std::vector<MemoryItem> items;
    int first = 1; // 1 is the filePage of iNode
    int second = 2; // 2 is the class of iNode
    for (int j = 0; j <= static_cast<int>(MemoryItemType::MEMORY_ITEM_TYPE_OTHER); j++) {
        MemoryItem item;
        item.type = static_cast<MemoryItemType>(j);
        MemoryClass tempClass = TYPE_TO_CLASS_MAP.find(item.type) != TYPE_TO_CLASS_MAP.end()
            ? TYPE_TO_CLASS_MAP.at(item.type) : MemoryClass::MEMORY_CLASS_OTHER;
        if (tempClass != detail.memoryClass) {
            continue;
        }
        item.iNode = (detail.memoryClass == MemoryClass::MEMORY_CLASS_OTHER) && (j % second == 0) ? 0 : first;
        item.rss = TEST_PSS_VALUE;
        item.pss = TEST_PSS_VALUE;
        item.swapPss = TEST_PSS_VALUE;
        item.swap = TEST_PSS_VALUE;
        item.allPss = TEST_PSS_VALUE;
        item.allSwap = TEST_PSS_VALUE;
        item.sharedDirty = TEST_PSS_VALUE;
        tempClass == MemoryClass::MEMORY_CLASS_GRAPH ? item.privateDirty = 0 : item.privateDirty = TEST_PSS_VALUE;
        item.sharedClean = TEST_PSS_VALUE;
        item.privateClean = TEST_PSS_VALUE;
        items.push_back(item);

        detail.totalRss += item.rss;
        detail.totalPss += item.pss;
        detail.totalSwapPss += item.swapPss;
        detail.totalSwap += item.swap;
        detail.totalAllPss += item.allPss;
        detail.totalAllSwap += item.allSwap;
        detail.totalSharedDirty += item.sharedDirty;
        detail.totalPrivateDirty += item.privateDirty;
        detail.totalSharedClean += item.sharedClean;
        detail.totalPrivateClean += item.privateClean;
    }
    detail.items = items;
}

bool HidumperMemoryTest::CheckMemoryPrint(const std::string& title, const std::string& value, const std::string& line)
{
    if (line.find(title) == std::string::npos) {
        std::cout << "data error, line:" << line << " title:" << title << std::endl;
        return false;
    }
    std::string valueStr = "";
    valueStr = line.substr(line.find(title) + title.length(), line.length());
    if (title == "GL" || title == "Graph") {
        std::string graphPss = HidumperTestUtils::GetInstance().GetValueInLine(valueStr, 0);
        std::string graphPrivateDirty = HidumperTestUtils::GetInstance().GetValueInLine(valueStr, 4);
        if (graphPss != value || graphPrivateDirty != value) {
            std::cout << "graphPss:" << graphPss << " graphPrivateDirty:" << graphPrivateDirty << std::endl;
            return false;
        } else {
            return true;
        }
    }
    // [Pss Total] ... [SwapPss Total] is 0-6 line
    for (int i = 0; i <= 6; i++) {
        std::string tmp = HidumperTestUtils::GetInstance().GetValueInLine(valueStr, i);
        if (!IsNumericStr(tmp)) {
            continue;
        }
        if (tmp != value) {
            std::cout << "tmp != value" << std::endl;
            return false;
        }
    }
    return true;
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
 * @tc.desc: Test empty unique_ptr.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperMemoryTest, MemoryInfo003, TestSize.Level1)
{
    unique_ptr<OHOS::HiviewDFX::MemoryInfo> memoryInfo =
        make_unique<OHOS::HiviewDFX::MemoryInfo>();
    shared_ptr<vector<vector<string>>> result = make_shared<vector<vector<string>>>();
    unique_ptr<MemoryDetail> nativeHeapDetail = nullptr;
    memoryInfo->GetNativeHeap(nativeHeapDetail, result);
    ASSERT_TRUE(result->size() == 0);

    unique_ptr<MemoryDetail> anonPageDetail = make_unique<MemoryDetail>();
    unique_ptr<MemoryDetail> filePageDetail = make_unique<MemoryDetail>();
    unique_ptr<MemoryDetail> tempDetail = nullptr;
    memoryInfo->UpdatePageDetail(anonPageDetail, filePageDetail, tempDetail);
    ASSERT_TRUE(anonPageDetail);
    ASSERT_TRUE(filePageDetail);

    std::unique_ptr<MallHeapInfo> mallocHeapInfo = std::make_unique<MallHeapInfo>();
    unique_ptr<MemoryDetail> detail = nullptr;
    memoryInfo->SetDetailRet(MEMINFO_FILEPAGE_OTHER, detail, mallocHeapInfo, result);
    ASSERT_TRUE(result->size() == 0);
    unique_ptr<ProcessMemoryDetail> processMemoryDetail = nullptr;
    memoryInfo->UpdateTotalDetail(processMemoryDetail, mallocHeapInfo, result);
    ASSERT_TRUE(result->size() == 0);
}

/**
 * @tc.name: MemoryInfo004
 * @tc.desc: Test error GroupMap.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperMemoryTest, MemoryInfo004, TestSize.Level1)
{
    unique_ptr<OHOS::HiviewDFX::MemoryInfo> memoryInfo =
        make_unique<OHOS::HiviewDFX::MemoryInfo>();
    shared_ptr<vector<vector<string>>> result = make_shared<vector<vector<string>>>();
    GroupMap groupMap;
    ValueMap memInfo;
    groupMap.insert(pair<string, ValueMap>("test", memInfo));
    memoryInfo->GetPssTotal(groupMap, result);
    ASSERT_TRUE(result->size() == 0);
}

/**
 * @tc.name: MemoryInfo005
 * @tc.desc: Test about ashmem.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperMemoryTest, MemoryInfo005, TestSize.Level1)
{
    unique_ptr<OHOS::HiviewDFX::MemoryInfo> memoryInfo =
        make_unique<OHOS::HiviewDFX::MemoryInfo>();
    shared_ptr<vector<vector<string>>> result = make_shared<vector<vector<string>>>();
    memoryInfo->GetAshmem(INIT_PID, result, true);
    ASSERT_TRUE(result->size() == 0);

    FILE* file = popen("pidof render_service", "r");
    char buffer[BUFFER_SIZE];
    if (file) {
        if (fgets(buffer, sizeof(buffer), file) != nullptr) {};
        pclose(file);
    }
    int rsPid = strtol(buffer, nullptr, 10);
    memoryInfo->GetAshmem(rsPid, result, true);
    if (DumpUtils::IsHmKernel()) {
        ASSERT_TRUE(result->size() != 0);
    }
}

/**
 * @tc.name: MemoryInfo006
 * @tc.desc: Test CalculateStars ret.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperMemoryTest, MemoryInfo006, TestSize.Level1)
{
    unique_ptr<OHOS::HiviewDFX::MemoryInfo> memoryInfo =
        make_unique<OHOS::HiviewDFX::MemoryInfo>();
    int value = static_cast<int>(memoryInfo->CalculateStars(pssValues, INDEX));
    ASSERT_TRUE(value != 0);
}

/**
 * @tc.name: MemoryInfo007
 * @tc.desc: Test GenerateTimestamps ret.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperMemoryTest, MemoryInfo007, TestSize.Level1)
{
    unique_ptr<OHOS::HiviewDFX::MemoryInfo> memoryInfo =
        make_unique<OHOS::HiviewDFX::MemoryInfo>();
    std::string timeStamps = memoryInfo->GenerateTimestamps(pssValues);
    ASSERT_TRUE(timeStamps.size() != 0);
}

/**
 * @tc.name: MemoryInfo008
 * @tc.desc: Test GenerateLine ret.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperMemoryTest, MemoryInfo008, TestSize.Level1)
{
    unique_ptr<OHOS::HiviewDFX::MemoryInfo> memoryInfo =
        make_unique<OHOS::HiviewDFX::MemoryInfo>();
    std::string generateLine = memoryInfo->GenerateLine(pssValues, INDEX);
    ASSERT_TRUE(generateLine.size() != 0);
}

/**
 * @tc.name: MemoryInfo009
 * @tc.desc: Test CalculateMaxIdex index.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperMemoryTest, MemoryInfo009, TestSize.Level1)
{
    unique_ptr<OHOS::HiviewDFX::MemoryInfo> memoryInfo =
        make_unique<OHOS::HiviewDFX::MemoryInfo>();
    int index = 0;
    memoryInfo->CalculateMaxIdex(pssValues, &index);
    ASSERT_TRUE(index != 0);
}

/**
 * @tc.name: MemoryInfo010
 * @tc.desc: Test PrintMemoryInfo prevLineCount.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperMemoryTest, MemoryInfo010, TestSize.Level1)
{
    unique_ptr<OHOS::HiviewDFX::MemoryInfo> memoryInfo =
        make_unique<OHOS::HiviewDFX::MemoryInfo>();
    int prevLineCount = 0;
    memoryInfo->PrintMemoryInfo(pssValues, &prevLineCount);
    ASSERT_TRUE(prevLineCount != 0);
    prevLineCount = 1;
    memoryInfo->PrintMemoryInfo(pssValues, &prevLineCount);
    ASSERT_TRUE(prevLineCount != 0);
}

/**
 * @tc.name: MemoryInfo011
 * @tc.desc: Test PrintMemoryInfo GetScoreAdj.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperMemoryTest, MemoryInfo011, TestSize.Level1)
{
    unique_ptr<OHOS::HiviewDFX::MemoryInfo> memoryInfo =
        make_unique<OHOS::HiviewDFX::MemoryInfo>();
    int ret = memoryInfo->GetScoreAdj(1);
    ASSERT_TRUE(ret != -1);
}

/**
 * @tc.name: MemoryInfo012
 * @tc.desc: Test about dmabuf.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperMemoryTest, MemoryInfo012, TestSize.Level1)
{
    unique_ptr<OHOS::HiviewDFX::MemoryInfo> memoryInfo =
        make_unique<OHOS::HiviewDFX::MemoryInfo>();
    shared_ptr<vector<vector<string>>> result = make_shared<vector<vector<string>>>();
    string path = "/proc/" + to_string(INIT_PID) + "/mm_dmabuf_info";
    if (access(path.c_str(), F_OK) == 0) {
        system("aa start -a com.ohos.contacts.MainAbility -b com.ohos.contacts");
        sleep(2);
        pid_t cocPid = HidumperTestUtils::GetInstance().GetPidByName("com.ohos.contacts");
        ASSERT_TRUE(cocPid != -1);
        ASSERT_TRUE(memoryInfo->GetDmaBuf(cocPid, result, true));
        ASSERT_TRUE(memoryInfo->GetDmaBufByProc(cocPid, result, DMA_SHOW_TITLES));
    }
}

/**
 * @tc.name: MemoryInfo013
 * @tc.desc: Test about dmabuf.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperMemoryTest, MemoryInfo013, TestSize.Level1)
{
    unique_ptr<OHOS::HiviewDFX::MemoryInfo> memoryInfo =
        make_unique<OHOS::HiviewDFX::MemoryInfo>();
    shared_ptr<vector<vector<string>>> result = make_shared<vector<vector<string>>>();
    string path = "/proc/" + to_string(INIT_PID) + "/mm_dmabuf_info";
    if (access(path.c_str(), F_OK) == 0) {
        memoryInfo->GetDmaBuf(INIT_PID, result, false);
        ASSERT_TRUE(result->size() == 0);
        memoryInfo->GetDmaBuf(INIT_PID, result, true);
        ASSERT_TRUE(result->size() == 0);
    }
}

/**
 * @tc.name: MemoryInfo014
 * @tc.desc: Test about dmabuf.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperMemoryTest, MemoryInfo014, TestSize.Level1)
{
    std::vector<MemInfo::DmaNodeInfoWrapper> dmaVec;
    dmaVec.push_back({ "ProcessA", 1234, 10, 2048, 5678, 4321, "ExpTaskA", "BufferA", "ExpNameA", true,
                      true, "", "ReclaimInfoA", "LeakTypeA" });
    unique_ptr<OHOS::HiviewDFX::MemoryInfo> memoryInfo =
        make_unique<OHOS::HiviewDFX::MemoryInfo>();
    shared_ptr<vector<vector<string>>> result = make_shared<vector<vector<string>>>();
    memoryInfo->DisposeDmaBufInfo(dmaVec, DMA_SHOW_TITLES, result);
    ASSERT_TRUE(result->size() == 2);

    std::istringstream ss(result->at(1).at(0));
    std::vector<std::string> data;
    std::string token;
    while (ss >> token) {
        data.push_back(token);
    }
    ASSERT_TRUE(data.size() == DMA_SHOW_TITLES.size());
    ASSERT_TRUE(data[1] == "1234");
    ASSERT_TRUE(data[9] == "NULL");
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

/**
 * @tc.name: MemoryTitle001
 * @tc.desc: Test memory title.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperMemoryTest, MemoryTitle001, TestSize.Level1)
{
    shared_ptr<OHOS::HiviewDFX::MemoryInfo> memoryInfo =
        make_shared<OHOS::HiviewDFX::MemoryInfo>();
    StringMatrix result = std::make_shared<std::vector<std::vector<std::string>>>();
    memoryInfo->InsertMemoryTitle(result);
    ASSERT_TRUE(result->size() == MEMEORY_TITLE_VEC.size());
    std::vector<std::string> memoryTitle;
    for (size_t i = 0; i < result->size(); i++) {
        std::string title = "";
        std::vector<std::string> line = result->at(i);
        for (size_t j = 0; j < line.size(); j++) {
            std::string str = line[j];
            title += str;
        }
        memoryTitle.push_back(title);
    }
    ASSERT_TRUE(memoryTitle == MEMEORY_TITLE_VEC);
}

/**
 * @tc.name: CheckMemoryData001
 * @tc.desc: Test check memory data.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperMemoryTest, CheckMemoryData001, TestSize.Level1)
{
    shared_ptr<OHOS::HiviewDFX::MemoryInfo> memoryInfo =
        make_shared<OHOS::HiviewDFX::MemoryInfo>();
    StringMatrix result = std::make_shared<std::vector<std::vector<std::string>>>();
    ProcessMemoryDetail processMemoryDetail;
    CreateProcessMemoryDetail(processMemoryDetail);
    unique_ptr<MemoryDetail> nativeHeapDetail = {nullptr};
    unique_ptr<ProcessMemoryDetail> processMemoryDetailPtr = make_unique<ProcessMemoryDetail>(processMemoryDetail);
    memoryInfo->UpdateResult(-1, processMemoryDetailPtr, nativeHeapDetail, result);
    memoryInfo->GetNativeHeap(nativeHeapDetail, result);
    ASSERT_TRUE(result->size() == TITLE_AND_VALUE.size());
    for (size_t i = 0; i < result->size(); i++) {
        std::vector<std::string> line = result->at(i);
        std::stringstream outputSs;
        bool isFind = false;
        for (size_t j = 0; j < line.size(); j++) {
            std::string str = line[j];
            if (str == "\n" || str == "native heap:" || str == "---------------") {
                isFind = true;
                break;
            }
            outputSs << str;
        }
        if (isFind) {
            continue;
        }
        ASSERT_TRUE(CheckMemoryPrint(TITLE_AND_VALUE[i].first, TITLE_AND_VALUE[i].second, outputSs.str()));
    }
}

/**
 * @tc.name: ParseAshmemInfo001
 * @tc.desc: Test error ashmemInfo.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperMemoryTest, ParseAshmemInfo001, TestSize.Level1)
{
    unique_ptr<ParseAshmemInfo> parseAshmeminfo = make_unique<ParseAshmemInfo>();
    std::unordered_map<std::string, int64_t> ashmemOverviewMap;
    ASSERT_FALSE(parseAshmeminfo->UpdateAshmemOverviewMap("", ashmemOverviewMap));
    ASSERT_FALSE(parseAshmeminfo->UpdateAshmemOverviewMap("test", ashmemOverviewMap));
    ASSERT_FALSE(parseAshmeminfo->UpdateAshmemOverviewMap("test[", ashmemOverviewMap));
    ASSERT_FALSE(parseAshmeminfo->UpdateAshmemOverviewMap("test]", ashmemOverviewMap));
    ASSERT_FALSE(parseAshmeminfo->UpdateAshmemOverviewMap("test][", ashmemOverviewMap));
    ASSERT_FALSE(parseAshmeminfo->UpdateAshmemOverviewMap("test[]", ashmemOverviewMap));
    ASSERT_FALSE(parseAshmeminfo->UpdateAshmemOverviewMap("test[], physical size is testSize", ashmemOverviewMap));
    ASSERT_FALSE(parseAshmeminfo->UpdateAshmemOverviewMap("test[test], physical size is 12345678912345678912",
        ashmemOverviewMap));
    ASSERT_TRUE(ashmemOverviewMap.empty());
}

/**
 * @tc.name: ParseAshmemInfo002
 * @tc.desc: Test correct ashmemInfo.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperMemoryTest, ParseAshmemInfo002, TestSize.Level1)
{
    unique_ptr<ParseAshmemInfo> parseAshmeminfo = make_unique<ParseAshmemInfo>();
    std::unordered_map<std::string, int64_t> ashmemOverviewMap;
    ASSERT_TRUE(parseAshmeminfo->UpdateAshmemOverviewMap(
        "Total ashmem of [test] virtual size is 123, physical size is 123456789123456789",
        ashmemOverviewMap));
    ASSERT_FALSE(ashmemOverviewMap.empty());

    auto iter = ashmemOverviewMap.find("test");
    ASSERT_NE(iter, ashmemOverviewMap.end());
    int64_t expectedValue = 123456789123456789LL;
    ASSERT_EQ(iter->second, expectedValue);
}
} // namespace HiviewDFX
} // namespace OHOS