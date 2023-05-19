/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#ifndef DUMP_USAGE_H
#define DUMP_USAGE_H
#include <memory>
#include "executor/memory/parse/meminfo_data.h"
namespace OHOS {
namespace HiviewDFX {
class DumpUsage {
public:
    DumpUsage();
    ~DumpUsage();

    bool GetMemInfo(const int &pid, MemInfoData::MemInfo &info);
    uint64_t GetPss(const int &pid);
    uint64_t GetPrivateDirty(const int &pid);
    uint64_t GetSharedDirty(const int &pid);
    float GetCpuUsage(const int &pid);

private:
    void InitMemInfo(MemInfoData::MemInfo &memInfo);
};
} // namespace HiviewDFX
} // namespace OHOS
#endif
