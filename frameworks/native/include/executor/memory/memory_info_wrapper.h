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
#ifndef HIVIEWDFX_HIDUMPER_INFO_WRAPPER_H
#define HIVIEWDFX_HIDUMPER_INFO_WRAPPER_H

#include "executor/memory/memory_info.h"
#include "executor/memory/smaps_memory_info.h"
#include <dlfcn.h>
#include <vector>
#include <string>

#define EXPORT_API __attribute__((visibility("default")))

#ifdef __cplusplus
extern "C" {
#endif

using StringMatrix = std::shared_ptr<std::vector<std::vector<std::string>>>;

EXPORT_API int GetMemoryInfoByPid(int pid, StringMatrix data, bool showAshmem);
EXPORT_API int GetMemoryInfoNoPid(int fd, StringMatrix data);
EXPORT_API int GetMemoryInfoPrune(int fd, StringMatrix data);
EXPORT_API int ShowMemorySmapsByPid(int pid, StringMatrix data, bool isShowSmapsInfo);
EXPORT_API void GetMemoryInfoByTimeInterval(int fd, int pid, int timeInterval);
EXPORT_API void SetReceivedSigInt(bool isReceivedSigInt);

#ifdef __cplusplus
}
#endif

#endif // HIVIEWDFX_HIDUMPER_INFO_WRAPPER_H