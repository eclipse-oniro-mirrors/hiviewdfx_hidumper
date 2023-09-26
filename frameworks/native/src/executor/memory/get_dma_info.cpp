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
#include "executor/memory/get_dma_info.h"
#include "executor/memory/memory_filter.h"
#include "executor/memory/memory_util.h"
#include "util/string_utils.h"
#include "util/file_utils.h"
using namespace std;
namespace OHOS {
namespace HiviewDFX {
GetDmaInfo::GetDmaInfo()
{
}
GetDmaInfo::~GetDmaInfo()
{
}

/**
 * @description: SetData
 * @param {string} &str-String to be inserted into result
 * @return void
 */
void GetDmaInfo::SetData(const string &str)
{
    vector<string> datas;
    StringUtils::GetInstance().StringSplit(str, " ", datas);
    if (!StringUtils::GetInstance().IsEnd(str, "NULL")) {
        return;
    }
    MemInfoData::DmaInfo dmaInfo;
    dmaInfo.name = datas[0];
    dmaInfo.pid = stoi(datas[1]);
    dmaInfo.fd = stoi(datas[2]);
    dmaInfo.size = stoi(datas[3]) / BYTE_PER_KB;
    dmaInfo.ino = stoi(datas[4]);
    dmaInfo.expPid = stoi(datas[5]);
    dmaInfo.status = 0;
    for (const auto &it : dmaInfos) {
        if (it.name == dmaInfo.name && it.ino == dmaInfo.ino) {
            dmaInfo.status = REPETITIVE1;
            break; 
        }
    }
    for (auto &it : dmaInfos) {
        if (it.name != dmaInfo.name && it.ino == dmaInfo.ino && it.status == NORMAL) {
            it.status = REPETITIVE2;
            break; 
        }
    }
    if (dmaInfo.status == NORMAL) {
        totalDma += dmaInfo.size;
    }
    dmaInfos.push_back(dmaInfo);
}

/**
 * @description: get the value of gpu usage
 * @param {GroupMap} &infos-gpu information
 * @return {bool}-the result of
 */
bool GetDmaInfo::GetDma()
{
    string path = "/proc/process_dmabuf_info";
    bool ret = FileUtils::GetInstance().LoadStringFromProcCb(path, false, true, [&](const string &line) -> void {
        SetData(line);
    });
    return ret;
}

uint64_t GetDmaInfo::GetTotalDma()
{
    return totalDma;
}

std::vector<MemInfoData::DmaInfo> GetDmaInfo::GetDmaInfos()
{
    return dmaInfos;
}

uint64_t GetDmaInfo::GetDmaByPid(const int32_t &pid)
{
    uint64_t gpu = 0;
    for (auto it : dmaInfos) {
        if (it.pid == pid && it.status == NORMAL) {
            gpu += it.size;
        }
    }
    return gpu;
}
} // namespace HiviewDFX
} // namespace OHOS
