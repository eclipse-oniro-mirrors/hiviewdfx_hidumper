/*
* Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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
#include "include/dump_usage.h"
#include "dump_manager_cpu_client.h"
#include "executor/memory/parse/parse_smaps_rollup_info.h"
#include "executor/memory/memory_util.h"
#include "hilog_wrapper.h"

using namespace std;
namespace OHOS {
namespace HiviewDFX {
DumpUsage::DumpUsage()
{
}

DumpUsage::~DumpUsage()
{
}

bool DumpUsage::GetMemInfo(const int &pid, MemInfoData::MemInfo &info)
{
    MemoryUtil::GetInstance().InitMemInfo(info);
    unique_ptr<ParseSmapsRollupInfo> parseSmapsRollupInfo = make_unique<ParseSmapsRollupInfo>();
    return parseSmapsRollupInfo->GetMemInfo(pid, info);
}

uint64_t DumpUsage::GetPss(const int &pid)
{
    MemInfoData::MemInfo info;
    return GetMemInfo(pid, info) ? info.pss + info.swapPss : 0;
}

uint64_t DumpUsage::GetPrivateDirty(const int &pid)
{
    MemInfoData::MemInfo info;
    return GetMemInfo(pid, info) ? info.privateDirty : 0;
}

uint64_t DumpUsage::GetSharedDirty(const int &pid)
{
    MemInfoData::MemInfo info;
    return GetMemInfo(pid, info) ? info.sharedDirty : 0;
}

double DumpUsage::GetCpuUsage(const int &pid)
{
    double cpuUsage = 0.00;
    auto& dumpManagerCpuClient = DumpManagerCpuClient::GetInstance();
    dumpManagerCpuClient.GetCpuUsageByPid(pid, cpuUsage);
    DUMPER_HILOGD(MODULE_CPU_SERVICE, "GetCpuUsage end, pid = %{public}d, cpuUsage = %{public}f", pid, cpuUsage);
    return cpuUsage;
}
} // namespace HiviewDFX
} // namespace OHOS