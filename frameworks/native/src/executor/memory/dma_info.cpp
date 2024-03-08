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
#include "executor/memory/dma_info.h"
#include "util/file_utils.h"
#include "securec.h"

using namespace std;

namespace OHOS {
namespace HiviewDFX {
static constexpr int BYTE_PER_KB = 1024;
DmaInfo::DmaInfo()
{
}

DmaInfo::~DmaInfo()
{
}

/**
 * @description: create dma info
 * @param {string} &str-string to be inserted into result
 * @return void
 */
void DmaInfo::CreateDmaInfo(const string &str)
{
    /*
        Dma-buf objects usage of processes:
        Process          pid              fd               size_bytes       ino              exp_pid          exp_task_comm    buf_name          exp_name
        composer_host    552              11               3686400          7135             552              composer_host    NULL      rockchipdrm
        composer_host    552              18               3686400          30235            543              allocator_host   NULL      rockchipdrm
        Total dmabuf size of composer_host: 29491200 bytes
        render_service   575              28               3686400          28052            543              allocator_host   NULL      rockchipdrm
        render_service   575              31               3686400          31024            543              allocator_host   NULL      rockchipdrm
        Total dmabuf size of render_service: 35520512 bytes
    */
    MemInfoData::DmaInfo dmaInfo;
    char name[256] = {0}; // 256: max size of name
    if (sscanf_s(str.c_str(), "%255s %llu %*llu %llu %llu %*n %*s %*s %*s",
                 name, sizeof(name), &dmaInfo.pid, &dmaInfo.size, &dmaInfo.ino) == 0) {
        return;
    }
    dmaInfo.name = name;
    dmaInfo.size /= BYTE_PER_KB;
    auto it = dmaInfos_.find(dmaInfo.ino);
    if (it == dmaInfos_.end()) {
        dmaInfos_.insert(pair<uint64_t, MemInfoData::DmaInfo>(dmaInfo.ino, dmaInfo));
    } else if ((dmaInfo.name != "composer_host" && dmaInfo.name != "render_service") ||
               (dmaInfo.name == "render_service" && it->second.name == "composer_host")) {
        it->second.pid = dmaInfo.pid;
    }
}

/**
 * @description: parse dma info
 * @param void
 * @return {bool}-the result of
 */
bool DmaInfo::ParseDmaInfo()
{
    if (initialized_) {
        return true;
    }
    initialized_ = true;
    string path = "/proc/process_dmabuf_info";
    bool ret = FileUtils::GetInstance().LoadStringFromProcCb(path, false, true, [&](const string &line) -> void {
        CreateDmaInfo(line);
    });
    for (const auto &it : dmaInfos_) {
        auto dma = dmaMap_.find(it.second.pid);
        if (dma != dmaMap_.end()) {
            dma->second += it.second.size;
        } else {
            dmaMap_.insert(pair<uint32_t, uint64_t>(it.second.pid, it.second.size));
        }
    }
    return ret;
}

/**
 * @description: get dma total
 * @param void
 * @return {uint64_t} dma total
 */
uint64_t DmaInfo::GetTotalDma()
{
    uint64_t totalDma = 0;
    for (const auto &it : dmaMap_) {
        totalDma += it.second;
    }
    return totalDma;
}

/**
 * @description: get dma by pid
 * @param {int32_t} &pid-process id
 * @return dma value of process
 */
uint64_t DmaInfo::GetDmaByPid(const int32_t &pid) const
{
    auto it = dmaMap_.find(pid);
    if (it != dmaMap_.end()) {
        return it->second;
    }
    return 0;
}
} // namespace HiviewDFX
} // namespace OHOS
