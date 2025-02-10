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
#ifndef JSHEAP_MEMORY_DUMPER_H
#define JSHEAP_MEMORY_DUMPER_H
#include <vector>
#include <string>
#include <memory>
#include "hidumper_executor.h"
#include "app_jsheap_mem_info.h"
#include "memory/dump_jsheap_info.h"

namespace OHOS {
namespace HiviewDFX {
class JsHeapMemoryDumper : public HidumperExecutor {
public:
    JsHeapMemoryDumper();
    ~JsHeapMemoryDumper();

    DumpStatus PreExecute(const std::shared_ptr<DumperParameter> &parameter, StringMatrix dumpDatas) override;
    DumpStatus Execute() override;
    DumpStatus AfterExecute() override;
private:
    DumpStatus status_ = DUMP_FAIL;
    StringMatrix dumpDatas_;
    std::unique_ptr<DumpJsHeapInfo> jsHeapInfo_;

    bool needSnapshot_ = true;
    bool needGc_ = true;
    bool needLeakobj_ = false;
    bool needBinary_ = false;
    uint32_t pid_ = 0;
    uint32_t tid_ = 0;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif
