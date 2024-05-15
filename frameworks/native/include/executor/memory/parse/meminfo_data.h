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
#ifndef MEMINFO_DATA_H
#define MEMINFO_DATA_H
#include <memory>
#include <sys/types.h>
#include <string>
namespace OHOS {
namespace HiviewDFX {
class MemInfoData {
public:
    MemInfoData();
    ~MemInfoData();

    struct MemInfo {
        uint64_t rss = 0;
        uint64_t pss = 0;
        uint64_t sharedClean = 0;
        uint64_t sharedDirty = 0;
        uint64_t privateClean = 0;
        uint64_t privateDirty = 0;
        uint64_t swap = 0;
        uint64_t swapPss = 0;
        uint64_t heapSize = 0;
        uint64_t heapAlloc = 0;
        uint64_t heapFree = 0;
    };

    struct MemSmapsInfo {
        uint64_t rss = 0;
        uint64_t pss = 0;
        uint64_t sharedClean = 0;
        uint64_t sharedDirty = 0;
        uint64_t privateClean = 0;
        uint64_t privateDirty = 0;
        uint64_t swap = 0;
        uint64_t swapPss = 0;
        std::string name;
        std::string start;
        std::string end;
        std::string perm;
        uint64_t size = 0;
        uint64_t counts = 0;
    };

    struct MemUsage {
        uint64_t vss = 0;
        uint64_t rss = 0;
        uint64_t uss = 0;
        uint64_t pss = 0;
        uint64_t swapPss = 0;
        uint64_t gl = 0;
        uint64_t graph = 0;
        uint64_t dma = 0;
        uint64_t purgSum = 0;
        uint64_t purgPin = 0;
        int pid = -1;
        std::string name;
        std::string adjLabel;
    };

    struct DmaInfo {
        uint64_t size = 0;
        uint64_t ino = 0;
        uint32_t pid = 0;
        std::string name;
    };

    struct GraphicsMemory {
        uint64_t gl = 0;
        uint64_t graph = 0;
    };

    struct HiaiUserAllocatedMemInfo {
        int pid = -1;
        int size = 0;
    };

private:
};
} // namespace HiviewDFX
} // namespace OHOS

#endif
