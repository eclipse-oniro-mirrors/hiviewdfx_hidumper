/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef MEMORY_INFO_H
#define MEMORY_INFO_H
#include <future>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include "executor/memory/get_heap_info.h"
#include "executor/memory/parse/meminfo_data.h"
#include "common.h"
#include "time.h"
#include "graphic_memory_collector.h"
#include "memory_collector.h"
#include "meminfo.h"
namespace OHOS {
namespace HiviewDFX {
namespace {
static const std::string MEMINFO_PSS = "Pss";
static const std::string MEMINFO_SHARED_CLEAN = "Shared_Clean";
static const std::string MEMINFO_SHARED_DIRTY = "Shared_Dirty";
static const std::string MEMINFO_PRIVATE_CLEAN = "Private_Clean";
static const std::string MEMINFO_PRIVATE_DIRTY = "Private_Dirty";
static const std::string MEMINFO_SWAP = "Swap";
static const std::string MEMINFO_SWAP_PSS = "SwapPss";
static const std::string MEMINFO_HEAP_SIZE = "Heap_Size";
static const std::string MEMINFO_HEAP_ALLOC = "Heap_Alloc";
static const std::string MEMINFO_HEAP_FREE = "Heap_Free";
static const std::string MEMINFO_DMA = "Dma";
static const std::string MEMINFO_OTHER = "other";
static const std::string MEMINFO_ANONPAGE_OTHER = "AnonPage other";
static const std::string MEMINFO_FILEPAGE_OTHER = "FilePage other";
static const std::string MEMINFO_ASHMEM = "Ashmem";
static const std::string MEMINFO_TOTAL = "Total";
}
class MemoryInfo {
public:
    MemoryInfo();
    ~MemoryInfo();

    using StringMatrix = std::shared_ptr<std::vector<std::vector<std::string>>>;
    using ValueMap = std::map<std::string, uint64_t>;
    using GroupMap = std::map<std::string, ValueMap>;
    using MemFun = std::function<void(MemInfoData::MemInfo&, uint64_t)>;
    using MemoryItemMap = std::map<std::string, MemoryItem>;
    using PageType = std::vector<std::pair<std::string, uint64_t>>;
    using PageTypePair = std::pair<PageType, PageType>;

    bool GetMemoryInfoByPid(const int32_t &pid, StringMatrix result, bool showAshmem, bool showDmaBuf);
    void GetMemoryInfoByTimeInterval(int fd, const int32_t &pid, const int32_t &timeInterval);
    void SetReceivedSigInt(bool isReceivedSigInt);
    DumpStatus GetMemoryInfoNoPid(int fd, StringMatrix result);
    DumpStatus GetMemoryInfoPrune(int fd, StringMatrix result);
    DumpStatus DealResult(StringMatrix result);

private:
    enum Status {
        SUCCESS_MORE_DATA = 1,
        FAIL_MORE_DATA = 2,
        SUCCESS_NO_MORE_DATA = 3,
        FAIL_NO_MORE_DATA = 4,
    };
    int rawParamFd_ = 0;
    const int LINE_WIDTH_ = 14;
    const int RAM_WIDTH_ = 16;
    const size_t TYPE_SIZE = 2;
    const char SEPARATOR_ = '-';
    const char BLANK_ = ' ';
    const static int NAME_SIZE_ = 2;
    const int PID_WIDTH_ = 5;
    const int NAME_WIDTH_ = 20;
    const int PSS_WIDTH_ = 30;
    const int KB_WIDTH_ = 12;
    const int NAME_AND_PID_WIDTH = 30;
    const int MALLOC_HEAP_TYPES = 3;
    const static int VSS_BIT = 4;
    const static int BYTE_PER_KB = 1024;
    const std::string ZERO = "0";
    const std::vector<std::string> MEMORY_CLASS_VEC = {
        "graph", "ark ts heap", ".db", "dev", "dmabuf", "guard", ".hap",
        "native heap", ".so", "stack", ".ttf", "other"
    };
    const std::vector<std::string> MEMORY_PRINT_ORDER_VEC = {
        "ark ts heap", "guard", "native heap", ".hap", "AnonPage other",
        "stack", ".db", ".so", "dev", "dmabuf", ".ttf"
    };
    const std::vector<std::string> NATIVE_HEAP_TAG = {"heap", "jemalloc meta", "jemalloc heap",
        "brk heap", "musl heap", "mmap heap"};
    const std::map<MemoryItemType, std::string> NATIVE_HEAP_MAP = {
        {MemoryItemType::MEMORY_ITEM_TYPE_HEAP, "heap"},
        {MemoryItemType::MEMORY_ITEM_TYPE_ANON_NATIVE_HEAP_JEMALLOC_META, "jemalloc meta"},
        {MemoryItemType::MEMORY_ITEM_TYPE_ANON_NATIVE_HEAP_JEMALLOC, "jemalloc heap"},
        {MemoryItemType::MEMORY_ITEM_TYPE_ANON_NATIVE_HEAP_BRK, "brk heap"},
        {MemoryItemType::MEMORY_ITEM_TYPE_ANON_NATIVE_HEAP_META, "musl heap"},
        {MemoryItemType::MEMORY_ITEM_TYPE_ANON_NATIVE_HEAP_MMAP, "mmap heap"},
    };
    const std::map<MemoryItemType, std::string> GRAPHIC_MAP = {
        {MemoryItemType::MEMORY_ITEM_TYPE_GRAPH_GL, "GL"},
        {MemoryItemType::MEMORY_ITEM_TYPE_GRAPH_GRAPHICS, "Graph"},
    };

    bool isReady_ = false;
    bool dumpPrune_ = false;
    bool dumpSmapsOnStart_ = false;
    uint64_t totalGL_ = 0;
    uint64_t totalGraph_ = 0;
    uint64_t totalDma_ = 0;
    uint64_t currentPss_ = 0;
    std::string startTime_;
    std::mutex mutex_;
    std::mutex timeIntervalMutex_;
    std::future<GroupMap> fut_;
    std::vector<int32_t> pids_;
    std::vector<MemInfoData::MemUsage> memUsages_;
    std::vector<std::pair<std::string, MemFun>> methodVec_;
    std::map<std::string, std::vector<MemInfoData::MemUsage>> adjMemResult_ = {
        {"System", {}}, {"Foreground", {}}, {"Suspend-delay", {}},
        {"Perceived", {}}, {"Background", {}}, {"Undefined", {}},
    };
    MemoryItemMap memoryItemMap_;
    MemInfoData::GraphicsMemory graphicsMemory_ = {0};

    void InsertMemoryTitle(StringMatrix result);
    void GetResult(const int32_t& pid, StringMatrix result);
    std::string AddKbUnit(const uint64_t &value) const;
    bool GetMemByProcessPid(const int32_t &pid, MemInfoData::MemUsage &usage);
    static bool GetSmapsInfoNoPid(const int32_t &pid, GroupMap &result);
    bool GetMeminfo(ValueMap &result);
    bool GetHardWareUsage(StringMatrix result);
    bool GetCMAUsage(StringMatrix result);
    bool GetKernelUsage(const ValueMap &infos, StringMatrix result);
    void GetProcesses(const GroupMap &infos, StringMatrix result);
    bool GetPids();
    void GetPssTotal(const GroupMap &infos, StringMatrix result);
    void GetRamUsage(const GroupMap &smapsinfos, const ValueMap &meminfo, StringMatrix result);
    void GetPurgTotal(const ValueMap &meminfo, StringMatrix result);
    void GetPurgByPid(const int32_t &pid, StringMatrix result);
    void GetDma(const uint64_t& dma, StringMatrix result);
    void GetHiaiServerIon(const int32_t &pid, StringMatrix result);
    void GetNativeHeap(const std::unique_ptr<MemoryDetail>& detail, StringMatrix result);
    void GetNativeValue(const std::string& tag, const GroupMap& nativeGroupMap, StringMatrix result);
    void GetRamCategory(const GroupMap &smapsinfos, const ValueMap &meminfos, StringMatrix result);
    void UpdateGraphicsMemoryRet(const std::string& title, const uint64_t& value, StringMatrix result);
    void AddBlankLine(StringMatrix result);
    void MemUsageToMatrix(const MemInfoData::MemUsage &memUsage, StringMatrix result);
    void PairToStringMatrix(const std::string &titleStr, std::vector<std::pair<std::string, uint64_t>> &vec,
                            StringMatrix result);
    void AddMemByProcessTitle(StringMatrix result, std::string sortType);
    bool GetMemoryInfoInit(StringMatrix result);
    void GetMemoryUsageInfo(StringMatrix result);
    
    static uint64_t GetVss(const int32_t &pid);
    static std::string GetProcName(const int32_t &pid);
    static int32_t GetProcUid(const int32_t &pid);
    static uint64_t GetProcValue(const int32_t &pid, const std::string& key);
#ifdef HIDUMPER_MEMMGR_ENABLE
    static std::string GetProcessAdjLabel(const int32_t pid);
#endif
    static int GetScoreAdj(const int32_t pid);
    static void InitMemInfo(MemInfoData::MemInfo &memInfo);
    static void InitMemUsage(MemInfoData::MemUsage &usage);
    void GetSortedMemoryInfoNoPid(StringMatrix result);
    bool GetGraphicsMemory(int32_t pid, MemInfoData::GraphicsMemory &graphicsMemory, GraphicType graphicType);
#ifdef HIDUMPER_MEMMGR_ENABLE
    void GetMemoryByAdj(StringMatrix result);
#endif
    void SetPss(MemInfoData::MemInfo &meminfo, uint64_t value);
    void SetSharedClean(MemInfoData::MemInfo &meminfo, uint64_t value);
    void SetSharedDirty(MemInfoData::MemInfo &meminfo, uint64_t value);
    void SetPrivateClean(MemInfoData::MemInfo &meminfo, uint64_t value);
    void SetPrivateDirty(MemInfoData::MemInfo &meminfo, uint64_t value);
    void SetSwap(MemInfoData::MemInfo &meminfo, uint64_t value);
    void SetSwapPss(MemInfoData::MemInfo &meminfo, uint64_t value);
    void SetHeapSize(MemInfoData::MemInfo &meminfo, uint64_t value);
    void SetHeapAlloc(MemInfoData::MemInfo &meminfo, uint64_t value);
    void SetHeapFree(MemInfoData::MemInfo &meminfo, uint64_t value);

    // hidumper --mem pid
    void UpdateResult(const int32_t& pid, const std::unique_ptr<ProcessMemoryDetail>& processMemoryDetail,
        std::unique_ptr<MemoryDetail>& nativeHeapDetail, StringMatrix result);
    void GetGraphicsMemoryByDetail(const std::unique_ptr<MemoryDetail>& detail, StringMatrix result);
    void UpdatePageDetail(std::unique_ptr<MemoryDetail>& anonPageDetail,
        std::unique_ptr<MemoryDetail>& filePageDetail, const std::unique_ptr<MemoryDetail>& tempDetail);
    void UpdateTotalDetail(const std::unique_ptr<ProcessMemoryDetail>& detail,
        const std::unique_ptr<MallHeapInfo>& heapInfo, StringMatrix result);
    void SetDetailRet(const std::string& memoryClassStr, const std::unique_ptr<MemoryDetail>& detail,
        const std::unique_ptr<MallHeapInfo>& heapInfo, StringMatrix result);
    void SetValueForRet(const std::string& value, std::vector<std::string>& tempResult);
    void SetNativeDetailRet(const std::string& nativeClassStr, const std::unique_ptr<MemoryItem>& item,
        StringMatrix result);
    void GetAshmem(const int32_t &pid, StringMatrix result, bool showAshmem);
    bool GetDmaBuf(const int32_t &pid, StringMatrix result, bool showDmaBuf);
    bool GetDmaBufByProc(const int32_t &pid, StringMatrix result, const std::vector<std::string>& showTitles);
    bool DisposeDmaBufInfo(const std::vector<MemInfo::DmaNodeInfoWrapper>& dmaBufInfos,
        const std::vector<std::string>& showTitles, StringMatrix result);
    bool FillResultForDmaBuf(const std::vector<std::vector<std::string>>& dmaBufResults,
        const std::vector<int>& columnWidths, StringMatrix result);
    int CalculateStars(const std::vector<int>& pssValues, int currentPSS);
    void WriteStdout(const std::string& s);
    void ClearPreviousLines(int lineCount);
    std::string GenerateTimestamps(const std::vector<int>& pssValues);
    std::string GenerateLine(const std::vector<int>& pssValues, int index);
    void CalculateMaxIdex(const std::vector<int>& pssValues, int *maxIndex);
    void PrintMemoryInfo(const std::vector<int>& pssValues, int* prevLineCount);
    void RedirectMemoryInfo(int timeIndex, StringMatrix result);
};
} // namespace HiviewDFX
} // namespace OHOS
#endif
