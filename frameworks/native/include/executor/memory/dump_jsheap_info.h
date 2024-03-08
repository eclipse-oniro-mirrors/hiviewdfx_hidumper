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
#ifndef DUMP_JSHEAP_INFO_H
#define DUMP_JSHEAP_INFO_H

#include "executor/memory/memory_filter.h"
#ifdef HIDUMPER_ABILITY_RUNTIME_ENABLE
#include "app_malloc_info.h"
#include "app_mgr_interface.h"
#endif
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "app_jsheap_mem_info.h"

namespace OHOS {
namespace HiviewDFX {
class DumpJsHeapInfo {
public:
    DumpJsHeapInfo();
    ~DumpJsHeapInfo();

    bool DumpJsHeapMemory(OHOS::AppExecFwk::JsHeapDumpInfo &info);
};
} // namespace HiviewDFX
} // namespace OHOS
#endif