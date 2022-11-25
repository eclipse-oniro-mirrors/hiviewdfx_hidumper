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
        uint64_t rss;
        uint64_t pss;
        uint64_t sharedClean;
        uint64_t sharedDirty;
        uint64_t privateClean;
        uint64_t privateDirty;
        uint64_t swap;
        uint64_t swapPss;
    };

    struct MemUsage {
        uint64_t vss;
        uint64_t rss;
        uint64_t uss;
        uint64_t pss;
        uint64_t swapPss;
        uint64_t gl;
        uint64_t graph;
        int pid;
        std::string name;
        std::string adjLabel;
    };

    struct GraphicsMemory {
        uint64_t gl;
        uint64_t graph;
    };

private:
};
} // namespace HiviewDFX
} // namespace OHOS

#endif