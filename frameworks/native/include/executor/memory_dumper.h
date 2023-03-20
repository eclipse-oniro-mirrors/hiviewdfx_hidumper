/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#ifndef MEMORY_DUMPER_H
#define MEMORY_DUMPER_H
#include <vector>
#include <string>
#include <memory>
#include "hidumper_executor.h"
#include "executor/memory/memory_info.h"
#include "executor/memory/smaps_memory_info.h"

namespace OHOS {
namespace HiviewDFX {
class MemoryDumper : public HidumperExecutor {
public:
    MemoryDumper();
    ~MemoryDumper();

    using ValueMap = std::map<std::string, uint64_t>;
    using GroupMap = std::map<std::string, ValueMap>;

    DumpStatus PreExecute(const std::shared_ptr<DumperParameter> &parameter, StringMatrix dumpDatas) override;
    DumpStatus Execute() override;
    DumpStatus AfterExecute() override;
  
private:
    int pid_ = 0;
    bool isShowMaps_ = false;
    DumpStatus status_ = DUMP_FAIL;
    StringMatrix dumpDatas_;
    std::unique_ptr<MemoryInfo> memoryInfo_;
	std::unique_ptr<SmapsMemoryInfo> smapsMemoryInfo_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif
