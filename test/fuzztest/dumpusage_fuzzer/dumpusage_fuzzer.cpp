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

#include <cstddef>
#include <cstdint>
#include <memory>
#include <iostream>
#include "dump_usage.h"

using namespace std;
using OHOS::HiviewDFX::DumpUsage;

namespace OHOS {
    constexpr int OFFSET = 8;
    int GetPidFromData(const uint8_t* data, size_t size)
    {
        int pid = 0;
        if (size == 1) {
            pid = static_cast<int>(data[0]);
        } else {
            pid = static_cast<int>(data[0]) | (static_cast<int>(data[1]) << OFFSET);
        }
        return pid;
    }

    bool GetCpuUsageFuzzTest(const uint8_t* data, size_t size)
    {
        int pid = GetPidFromData(data, size);
        unique_ptr<DumpUsage> dumpUsage = make_unique<DumpUsage>();
        auto result = dumpUsage->GetCpuUsage(pid);
        std::cout << "pid:" << pid << " cpuusage:" << result << std::endl;
        return true;
    }

    bool GetPssFuzzTest(const uint8_t* data, size_t size)
    {
        int pid = GetPidFromData(data, size);
        unique_ptr<DumpUsage> dumpUsage = make_unique<DumpUsage>();
        auto result = dumpUsage->GetPss(pid);
        std::cout << "pid:" << pid << " pss:" << result << std::endl;
        return true;
    }

    bool GetPrivateDirtyFuzzTest(const uint8_t* data, size_t size)
    {
        int pid = GetPidFromData(data, size);
        unique_ptr<DumpUsage> dumpUsage = make_unique<DumpUsage>();
        auto result = dumpUsage->GetPrivateDirty(pid);
        std::cout << "pid:" << pid << " private dirty:" << result << std::endl;
        return true;
    }

    bool GetSharedDirtyFuzzTest(const uint8_t* data, size_t size)
    {
        int pid = GetPidFromData(data, size);
        unique_ptr<DumpUsage> dumpUsage = make_unique<DumpUsage>();
        auto result = dumpUsage->GetSharedDirty(pid);
        std::cout << "pid:" << pid << " shared dirty:" << result << std::endl;
        return true;
    }

    bool GetMemInfoFuzzTest(const uint8_t* data, size_t size)
    {
        int pid = GetPidFromData(data, size);
        unique_ptr<HiviewDFX::DumpUsage> dumpUsage = make_unique<HiviewDFX::DumpUsage>();
        HiviewDFX::MemInfoData::MemInfo memInfo;
        auto ret = dumpUsage->GetMemInfo(pid, memInfo);
        std::cout << "pid:" << pid << " ret:" << ret << std::endl;
        return true;
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::GetCpuUsageFuzzTest(data, size);
    OHOS::GetPssFuzzTest(data, size);
    OHOS::GetPrivateDirtyFuzzTest(data, size);
    OHOS::GetSharedDirtyFuzzTest(data, size);
    OHOS::GetMemInfoFuzzTest(data, size);
    return 0;
}