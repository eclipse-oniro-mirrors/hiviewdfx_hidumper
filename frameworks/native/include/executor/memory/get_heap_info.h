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
#ifndef GET_HEAP_INFO_H
#define GET_HEAP_INFO_H

#include "executor/memory/memory_filter.h"
#ifdef HIDUMPER_ABILITY_RUNTIME_ENABLE
#include "app_malloc_info.h"
#include "app_mgr_interface.h"
#endif
#include "iservice_registry.h"
#include "system_ability_definition.h"
namespace OHOS {
namespace HiviewDFX {
class GetHeapInfo {
public:
    GetHeapInfo();
    ~GetHeapInfo();

    using ValueMap = std::map<std::string, uint64_t>;
    using GroupMap = std::map<std::string, ValueMap>;
    bool GetInfo(const MemoryFilter::MemoryType &memType, const int &pid, GroupMap &infos);

private:
    struct MallHeapInfo {
        int size;
        int alloc;
        int free;
    };

    const std::string groupNative = "native heap";
#ifdef HIDUMPER_ABILITY_RUNTIME_ENABLE
    const int numberSys = 1024;
    OHOS::sptr<OHOS::AppExecFwk::IAppMgr> GetAppManagerInstance();
#endif
};
} // namespace HiviewDFX
} // namespace OHOS
#endif