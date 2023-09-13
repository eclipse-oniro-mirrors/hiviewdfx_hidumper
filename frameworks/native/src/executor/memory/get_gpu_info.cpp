/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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
#include "executor/memory/get_gpu_info.h"
#include "executor/memory/memory_filter.h"
#include "executor/memory/memory_util.h"
#include "util/string_utils.h"
#include "util/file_utils.h"
using namespace std;
namespace OHOS {
namespace HiviewDFX {
GetGpuInfo::GetGpuInfo()
{
}
GetGpuInfo::~GetGpuInfo()
{
}

/**
 * @description: SetData
 * @param {string} &str-String to be inserted into result
 * @return void
 */
void GetGpuInfo::SetData(const string &str)
{
    vector<string> datas;
    StringUtils::GetInstance().StringSplit(str, " ", datas);
    if (datas.size() < 9 || str.find("size_bytes") != string::npos) { // 9:row count
        return;
    }
    MemInfoData::GpuInfo gpuInfo;
    gpuInfo.name = datas[0];
    gpuInfo.pid = stoi(datas[1]);
    gpuInfo.fd = stoi(datas[2]);    // 2:index
    gpuInfo.size = stoi(datas[3]) / BYTE_PER_KB;    // 3:index
    gpuInfo.ino = stoi(datas[4]);   // 4:index
    gpuInfo.expPid = stoi(datas[5]);// 5:index
    gpuInfo.status = 0;
    for (auto it : gpuInfos) {
        if (it.name == gpuInfo.name && it.ino == gpuInfo.ino) {
            gpuInfo.status = REPETITIVE1;
            break; 
        }
    }
    for (auto it : gpuInfos) {
        if (it.name != gpuInfo.name && it.ino == gpuInfo.ino && it.status == NORMAL) {
            it.status = REPETITIVE2;
            break; 
        }
    }
    if (gpuInfo.status == NORMAL) {
        totalGpu += gpuInfo.size;
    }
    gpuInfos.push_back(gpuInfo);
}

/**
 * @description: get the value of gpu usage
 * @param {GroupMap} &infos-gpu information
 * @return {bool}-the result of
 */
bool GetGpuInfo::GetGpu()
{
    string path = "/proc/process_dmabuf_info";
    bool ret = FileUtils::GetInstance().LoadStringFromProcCb(path, false, true, [&](const string &line) -> void {
        SetData(line);
    });
    return ret;
}

uint64_t GetGpuInfo::GetTotalGpu()
{
    return totalGpu;
}

std::vector<MemInfoData::GpuInfo> GetGpuInfo::GetGpuInfos()
{
    return gpuInfos;
}

bool GetGpuInfo::GetInfo(const int32_t &pid, GroupMap &infos)
{
    uint64_t gpu = 0;
    for (auto it : gpuInfos) {
        if (it.pid == pid && it.status == 0) {
            gpu += it.size;
        }
    }
    map<string, uint64_t> valueMap;
    valueMap.insert(pair<string, uint64_t>("Pss", gpu));
    valueMap.insert(pair<string, uint64_t>("Private_Dirty", gpu));
    infos.insert(pair<string, map<string, uint64_t>>("AnonPage # Gpu", valueMap));
    return true;
}

uint64_t GetGpuInfo::GetGpu(const int32_t &pid)
{
    uint64_t gpu = 0;
    for (auto it : gpuInfos) {
        if (it.pid == pid && it.status == NORMAL) {
            gpu += it.size;
        }
    }
    return gpu;
}
} // namespace HiviewDFX
} // namespace OHOS
