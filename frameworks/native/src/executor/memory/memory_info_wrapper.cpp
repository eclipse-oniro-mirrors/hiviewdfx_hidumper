/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
#include "executor/memory/memory_info_wrapper.h"
#include <dlfcn.h>
#include <vector>
#include <string>
#include "hilog_wrapper.h"

#ifdef __cplusplus
extern "C" {
#endif

using StringMatrix = std::shared_ptr<std::vector<std::vector<std::string>>>;

int GetMemoryInfoByPid(int pid, StringMatrix data)
{
    std::unique_ptr<OHOS::HiviewDFX::MemoryInfo> memoryInfo = std::make_unique<OHOS::HiviewDFX::MemoryInfo>();
    if (!memoryInfo->GetMemoryInfoByPid(pid, data)) {
        DUMPER_HILOGE(MODULE_SERVICE, "debug|GetMemoryInfoByPid error, pid:%{public}d", pid);
        return OHOS::HiviewDFX::DumpStatus::DUMP_FAIL;
    }
    return OHOS::HiviewDFX::DumpStatus::DUMP_OK;
}

int GetMemoryInfoNoPid(int fd, StringMatrix data)
{
    std::unique_ptr<OHOS::HiviewDFX::MemoryInfo> memoryInfo = std::make_unique<OHOS::HiviewDFX::MemoryInfo>();
    int ret = memoryInfo->GetMemoryInfoNoPid(fd, data);
    return ret;
}

#ifdef __cplusplus
}
#endif