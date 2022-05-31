/*
* Copyright (C) 2021 Huawei Device Co., Ltd.
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#include <memory>
#include <iostream>
#include "dump_usage.h"

using namespace std;
using namespace OHOS::HiviewDFX;

static uint64_t GetPss(const int &pid)
{
    unique_ptr<DumpUsage> dumpUsage = make_unique<DumpUsage>();
    return dumpUsage->GetPss(pid);
}

static uint64_t GetPrivateDirty(const int &pid)
{
    unique_ptr<DumpUsage> dumpUsage = make_unique<DumpUsage>();
    return dumpUsage->GetPrivateDirty(pid);
}

static uint64_t GetSharedDirty(const int &pid)
{
    unique_ptr<DumpUsage> dumpUsage = make_unique<DumpUsage>();
    return dumpUsage->GetSharedDirty(pid);
}

static bool GetMemInfo(const int &pid, OHOS::HiviewDFX::MemInfoData::MemInfo &data)
{
    unique_ptr<DumpUsage> dumpUsage = make_unique<DumpUsage>();
    return dumpUsage->GetMemInfo(pid, data);
}

static float GetCpuUsage(const int &pid)
{
    unique_ptr<DumpUsage> dumpUsage = make_unique<DumpUsage>();
    return dumpUsage->GetCpuUsage(pid);
}

int main(int argc, char *argv[])
{
    int pid;
    cout << "Please input pid:";
    cin >> pid;

    uint64_t pss = GetPss(pid);
    std::cout << "pss:" << pss << std::endl;

    uint64_t privateDirty = GetPrivateDirty(pid);
    std::cout << "privateDirty:" << privateDirty << std::endl;

    uint64_t shareDirty = GetSharedDirty(pid);
    std::cout << "shareDirty:" << shareDirty << std::endl;

    float cpuUsage = GetCpuUsage(pid);
    printf("cpuUsage:%.1f\n", cpuUsage);

    OHOS::HiviewDFX::MemInfoData::MemInfo info;
    bool success = GetMemInfo(pid, info);
    std::cout << "GetMemInfo success:" << success << ",rss:" << info.rss << ",pss:" << info.pss
              << ",sharedClean:" << info.sharedClean << ",sharedDirty:" << info.sharedDirty
              << ",privateClean:" << info.privateClean << ",privateDirty:" << info.privateDirty << ",swap:" << info.swap
              << ",swapPss:" << info.swapPss << std::endl;
}
