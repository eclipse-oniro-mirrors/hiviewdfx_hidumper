/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#ifndef HEAP_MEMORY_DUMPER_H
#define HEAP_MEMORY_DUMPER_H
#include <vector>
#include <string>
#include <memory>
#include "hidumper_executor.h"
#include "app_mem_dump_info.h"
#include "memory/dump_heap_info.h"

namespace OHOS {
namespace HiviewDFX {
class HeapMemoryDumper : public HidumperExecutor {
public:
    HeapMemoryDumper();
    ~HeapMemoryDumper();

    DumpStatus PreExecute(const std::shared_ptr<DumperParameter> &parameter, StringMatrix dumpDatas) override;
    DumpStatus Execute() override;
    DumpStatus AfterExecute() override;
private:
    AppExecFwk::MemDumpType DetermineDumpType(const std::shared_ptr<DumperParameter> &parameter) const;

    DumpStatus status_ = DUMP_FAIL;
    StringMatrix dumpDatas_;
    std::unique_ptr<DumpHeapInfo> heapInfo_;
    AppExecFwk::MemDumpType dumpType_ = AppExecFwk::MemDumpType::INVALID;
    bool needLeakobj_ = false;
    bool needRaw_ = false;
    std::string dumpResult_;
    int pid_ = 0;
    int tid_ = 0;
    int renderPid_ = 0;
    bool needDump_ = true;
    bool needGc_ = false;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif
