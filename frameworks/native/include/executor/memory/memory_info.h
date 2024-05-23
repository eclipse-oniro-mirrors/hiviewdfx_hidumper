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
#include <string>
#include <vector>
#include "executor/memory/dma_info.h"
#include "executor/memory/parse/meminfo_data.h"
#include "common.h"
#include "time.h"
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

// system app
constexpr int RECLAIM_PRIORITY_SYSTEM = -1000;
//ondemand system app
constexpr int RECLAIM_ONDEMAND_SYSTEM = -900;
// persist(killable) system app
constexpr int RECLAIM_PRIORITY_KILLABLE_SYSTEM = -800;
// foreground process priority
constexpr int RECLAIM_PRIORITY_FOREGROUND = 0;
// visible process priority
constexpr int RECLAIM_PRIORITY_VISIBLE = 1;
// perceived suspend delay case
constexpr int RECLAIM_PRIORITY_BG_SUSPEND_DELAY = 100;
// perceived background process priority
constexpr int RECLAIM_PRIORITY_BG_PERCEIVED = 200;
// background and connected by distribute device
constexpr int RECLAIM_PRIORITY_BG_DIST_DEVICE = 260;
// background priority
constexpr int RECLAIM_PRIORITY_BACKGROUND = 400;
// unknown process priority
constexpr int RECLAIM_PRIORITY_UNKNOWN = 1000;

const std::map<int, std::string> ReclaimPriorityMapping = {
    { RECLAIM_PRIORITY_SYSTEM, "System" },
    { RECLAIM_ONDEMAND_SYSTEM, "OnDemand_system" },
    { RECLAIM_PRIORITY_KILLABLE_SYSTEM, "Persistent" },
    { RECLAIM_PRIORITY_FOREGROUND, "Foreground" },
    { RECLAIM_PRIORITY_VISIBLE, "visible" },
    { RECLAIM_PRIORITY_BG_SUSPEND_DELAY, "Suspend-delay" },
    { RECLAIM_PRIORITY_BG_PERCEIVED, "Perceived" },
    { RECLAIM_PRIORITY_BG_DIST_DEVICE, "Dist-device" },
    { RECLAIM_PRIORITY_BACKGROUND, "Background" },
};

const std::string RECLAIM_PRIORITY_UNKNOWN_DESC = "Undefined";
}
class MemoryInfo {
public:
    MemoryInfo();
    ~MemoryInfo();

    using StringMatrix = std::shared_ptr<std::vector<std::vector<std::string>>>;
    using ValueMap = std::map<std::string, uint64_t>;
    using GroupMap = std::map<std::string, ValueMap>;
    using MemFun = std::function<void(MemInfoData::MemInfo&, uint64_t)>;

    bool GetMemoryInfoByPid(const int32_t &pid, StringMatrix result);
    DumpStatus GetMemoryInfoNoPid(StringMatrix result);
    DumpStatus DealResult(StringMatrix result);

private:
    enum Status {
        SUCCESS_MORE_DATA = 1,
        FAIL_MORE_DATA = 2,
        SUCCESS_NO_MORE_DATA = 3,
        FAIL_NO_MORE_DATA = 4,
    };

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
    const static int VSS_BIT = 4;
    const static int BYTE_PER_KB = 1024;
    bool isReady_ = false;
    bool dumpSmapsOnStart_ = false;
    uint64_t totalGL_ = 0;
    uint64_t totalGraph_ = 0;
    std::future<GroupMap> fut_;
    std::vector<int32_t> pids_;
    std::vector<MemInfoData::MemUsage> memUsages_;
    std::vector<std::pair<std::string, MemFun>> methodVec_;
    std::map<std::string, std::vector<MemInfoData::MemUsage>> adjMemResult_ = {
        {"System", {}}, {"Foreground", {}}, {"Suspend-delay", {}},
        {"Perceived", {}}, {"Background", {}}, {"Undefined", {}},
    };
    std::vector<std::string> NATIVE_HEAP_TAG_ = {"heap", "jemalloc meta", "jemalloc heap",
                                                 "brk heap", "musl heap", "mmap heap"};
    DmaInfo dmaInfo_;
    void insertMemoryTitle(StringMatrix result);
    void BuildResult(const GroupMap &infos, StringMatrix result);

    std::string AddKbUnit(const uint64_t &value) const;
    bool GetMemByProcessPid(const int32_t &pid, const DmaInfo &dmaInfo, MemInfoData::MemUsage &usage);
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
    void GetDmaByPid(const int32_t &pid, StringMatrix result);
    void GetHiaiServerIon(const int32_t &pid, StringMatrix result);
    void GetNativeHeap(const GroupMap& nativeGroupMap, StringMatrix result);
    void GetNativeValue(const std::string& tag, const GroupMap& nativeGroupMap, StringMatrix result);
    void GetRamCategory(const GroupMap &smapsinfos, const ValueMap &meminfos, StringMatrix result);
    void SetGraphGroupMap(GroupMap& groupMap, MemInfoData::GraphicsMemory &graphicsMemory);
    void AddBlankLine(StringMatrix result);
    void MemUsageToMatrix(const MemInfoData::MemUsage &memUsage, StringMatrix result);
    void PairToStringMatrix(const std::string &titleStr, std::vector<std::pair<std::string, uint64_t>> &vec,
                            StringMatrix result);
    void AddMemByProcessTitle(StringMatrix result, std::string sortType);
    
    static uint64_t GetVss(const int32_t &pid);
    static std::string GetProcName(const int32_t &pid);
    static std::string GetProcStatusName(const int32_t &pid);
    static uint64_t GetProcValue(const int32_t &pid, const std::string& key);
    static std::string GetProcessAdjLabel(const int32_t pid);
    static std::string GetReclaimPriorityString(int32_t priority);
    static void InitMemInfo(MemInfoData::MemInfo &memInfo);
    static void InitMemUsage(MemInfoData::MemUsage &usage);
    void CalcGroup(const GroupMap &infos, StringMatrix result);
    void GetSortedMemoryInfoNoPid(StringMatrix result);
#ifdef HIDUMPER_GRAPHIC_ENABLE
    static void GetMemGraphics();
#endif
    static bool GetGraphicsMemory(int32_t pid, MemInfoData::GraphicsMemory &graphicsMemory);
    static bool GetRenderServiceGraphics(int32_t pid, MemInfoData::GraphicsMemory &graphicsMemory);
    static bool IsRenderService(int32_t pid);
    static bool IsOHService(const int32_t &pid);
    static void GetNSPidByPid(const int32_t &pid, std::string &nsPid);
    void GetMemoryByAdj(StringMatrix result);
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
};
} // namespace HiviewDFX
} // namespace OHOS
#endif
