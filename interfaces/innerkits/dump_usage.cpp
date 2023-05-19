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

#include "executor/memory/parse/parse_smaps_rollup_info.h"
#include "util/dump_cpu_info_util.h"

using namespace std;
namespace OHOS {
namespace HiviewDFX {
DumpUsage::DumpUsage()
{
}

DumpUsage::~DumpUsage()
{
}

void DumpUsage::InitMemInfo(MemInfoData::MemInfo &memInfo)
{
    memInfo.rss = 0;
    memInfo.pss = 0;
    memInfo.sharedClean = 0;
    memInfo.sharedDirty = 0;
    memInfo.privateClean = 0;
    memInfo.privateDirty = 0;
    memInfo.swap = 0;
    memInfo.swapPss = 0;
}

bool DumpUsage::GetMemInfo(const int &pid, MemInfoData::MemInfo &info)
{
    InitMemInfo(info);
    unique_ptr<ParseSmapsRollupInfo> parseSmapsRollupInfo = make_unique<ParseSmapsRollupInfo>();
    return parseSmapsRollupInfo->GetMemInfo(pid, info);
}

uint64_t DumpUsage::GetPss(const int &pid)
{
    MemInfoData::MemInfo info;
    return GetMemInfo(pid, info) ? info.pss : 0;
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

float DumpUsage::GetCpuUsage(const int &pid)
{
    float ret = DumpCpuInfoUtil::GetInstance().GetCpuUsage(pid);
    return ret < 0 ? 0 : ret;
}
} // namespace HiviewDFX
} // namespace OHOS