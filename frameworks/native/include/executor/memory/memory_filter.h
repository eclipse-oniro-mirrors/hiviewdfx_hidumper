/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef MEMORY_FILTER_H
#define MEMORY_FILTER_H
#include <map>
#include <string>
#include <vector>
#include <v1_0/imemory_tracker_interface.h>
#include "singleton.h"

namespace OHOS {
namespace HiviewDFX {
using namespace OHOS::HDI::Memorytracker::V1_0;
class MemoryFilter : public Singleton<MemoryFilter> {
public:
    MemoryFilter();
    ~MemoryFilter();
    MemoryFilter(MemoryFilter const &) = delete;
    void operator=(MemoryFilter const &) = delete;

    using MatchFunc = std::function<bool(std::string, std::string)>;

    enum MemoryType {
        APPOINT_PID,
        NOT_SPECIFIED_PID,
    };

    int SMAPS_THREAD_NUM_ = 5;
    size_t HARDWARE_USAGE_THREAD_NUM_ = 5;
    const std::string FILE_PAGE_TAG = "File-backed Page";
    const std::string ANON_PAGE_TAG = "Anonymous Page";
    const std::string DMA_TAG = "DMA";
    const std::string GL_OUT_LABEL = "GL";
    const std::string GRAPH_OUT_LABEL = "Graph";
    const std::vector<std::pair<MemoryTrackerType, std::string>> MEMORY_TRACKER_TYPES = {
        {MEMORY_TRACKER_TYPE_GL, "GL"}, {MEMORY_TRACKER_TYPE_GRAPH, "Graph"},
        {MEMORY_TRACKER_TYPE_OTHER, "Other"}
    };

    const std::vector<std::string> RECLAIM_PRIORITY = {"System", "Foreground", "Suspend-delay", "Perceived",
                                                       "Background", "Undefined"};

    const std::vector<std::string> VALUE_WITH_PID = {"Pss", "Shared_Clean", "Shared_Dirty", "Private_Clean",
                                                     "Private_Dirty", "Swap", "SwapPss"};

    const std::vector<std::string> TITLE_HAS_PID_ = {"Pss_Total",     "Shared_Clean", "Shared_Dirty", "Private_Clean",
                                                     "Private_Dirty", "Swap_Total",   "SwapPss_Total"};

    const std::vector<std::string> TITLE_NO_PID_ = {"Pss", "SwapPss"};

    std::vector<std::string> MEMINFO_TAG_ = {
        "MemTotal", "MemFree",       "Cached",       "SwapTotal", "KernelStack", "SUnreclaim", "PageTables",
        "Shmem",    "IonTotalCache", "IonTotalUsed", "Buffers",   "Mapped",      "Slab",       "VmallocUsed",
    };

    // The fields used to calculate kernel data
    std::vector<std::string> CALC_KERNEL_TOTAL_ = {"KernelStack", "SUnreclaim", "PageTables", "Shmem"};

    std::vector<std::string> CALC_PSS_TOTAL_ = {"Pss", "SwapPss"};
    std::vector<std::string> CALC_PROCESS_TOTAL_ = {"Pss", "SwapPss"};
    std::vector<std::string> CALC_TOTAL_PSS_ = {"Pss"};
    std::vector<std::string> CALC_TOTAL_SWAP_PSS_ = {"SwapPss"};
    std::vector<std::string> CALC_KERNEL_USED_ = {"Shmem", "Slab", "VmallocUsed", "PageTables", "KernelStack"};
    std::vector<std::string> CALC_FREE_ = {"MemFree"};
    std::vector<std::string> CALC_CACHED_ = {"Buffers", "Cached", "Mapped"};
    std::vector<std::string> CALC_TOTAL_ = {"MemTotal"};
    std::vector<std::string> CALC_ZARM_TOTAL_;
    std::vector<std::string> HAS_PID_ORDER_ = {"Pss",           "Shared_Clean", "Shared_Dirty", "Private_Clean",
                                               "Private_Dirty", "Swap",         "SwapPss"};
    std::vector<std::string> NO_PID_ORDER_ = {"Pss"};
    void ParseMemoryGroup(const std::string &name, std::string &group, uint64_t iNode);

private:
    const std::map<std::string, std::string> beginMap_ = {
        {"[heap]", "native heap"}, {"[stack]", "stack"}, {"[anon:stack", "stack"},
        {"[anon:native_heap:", "native heap"}, {"[anon:ArkJS Heap]", "ark js heap"},
        {"[anon:guard", "guard"}, {"/dev", "dev"}, {"[anon:signal_stack", "stack"},
        {"/dmabuf", "dmabuf"}, {"/data/storage", ".hap"}, {"[anon:libc_malloc", "native heap"},
    };
    const std::map<std::string, std::string> endMap_ = {
        {".so", ".so"}, {".so.1", ".so"}, {".ttf", ".ttf"},
        {".db", ".db"}, {".db-shm", ".db"},
    };

    bool GetGroupFromMap(const std::string &name, std::string &group,
                         const std::map<std::string, std::string> &map, MatchFunc func);
};
} // namespace HiviewDFX
} // namespace OHOS
#endif
