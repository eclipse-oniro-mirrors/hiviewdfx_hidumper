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
#ifndef DUMP_ARKTS_HEAP_INFO_H
#define DUMP_ARKTS_HEAP_INFO_H

#include "hilog_wrapper.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "app_mem_dump_info.h"
#include <string>

namespace OHOS {
namespace HiviewDFX {

class DumpArktsHeapInfo {
public:
    DumpArktsHeapInfo();
    ~DumpArktsHeapInfo();

    bool GetArktsHeapSize(int32_t pid, std::string &result);
};
} // namespace HiviewDFX
} // namespace OHOS
#endif
