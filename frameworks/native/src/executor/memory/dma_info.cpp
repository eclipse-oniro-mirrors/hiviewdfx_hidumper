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
#include "executor/memory/memory_filter.h"
#include "executor/memory/memory_util.h"
#include "util/string_utils.h"
#include "util/file_utils.h"
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
    vector<string> datas;
    StringUtils::GetInstance().StringSplit(str, " ", datas);
    if (str.size() < 120 && str.find("size_bytes") != string::npos) {
        return;
    }
    MemInfoData::DmaInfo dmaInfo;
    dmaInfo.name = datas[0];
    dmaInfo.pid = stoi(datas[1]);
    dmaInfo.fd = stoi(datas[2]);                // 2: index of row
    dmaInfo.size = stoi(datas[3]) / BYTE_PER_KB;// 3: index of row
    dmaInfo.ino = stoi(datas[4]);               // 4: index of row
    dmaInfo.expPid = stoi(datas[5]);            // 5: index of row
    dmaInfo.status = 0;
    for (const auto &it : dmaInfos_) {
        if (it.name == dmaInfo.name && it.ino == dmaInfo.ino) {
            dmaInfo.status = REPETITIVE1;
            break; 
        }
    }
    for (auto &it : dmaInfos_) {
        if (it.name != dmaInfo.name && it.ino == dmaInfo.ino && it.status == NORMAL) {
            it.status = REPETITIVE2;
            break; 
        }
    }
    dmaInfos_.push_back(dmaInfo);
}

/**
 * @description: parse dma info
 * @param void
 * @return {bool}-the result of
 */
bool DmaInfo::ParseDmaInfo()
{
    if (isFirst_) {
        return true;
    }
    isFirst_ = true;
    string path = "/proc/process_dmabuf_info";
    bool ret = FileUtils::GetInstance().LoadStringFromProcCb(path, false, true, [&](const string &line) -> void {
        SetData(line);
    });
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
    for (const auto &it : dmaInfos_) {
        if (it.status == NORMAL) {
            totalDma += it.size;
        }
    }
    return totalDma;
}

/**
 * @description: parse dma by pid
 * @param {int32_t} &pid-process id
 * @return dma value of process
 */
uint64_t DmaInfo::GetDmaByPid(const int32_t &pid)
{
    uint64_t dma = 0;
    for (const auto &it : dmaInfos_) {
        if (it.pid == pid && it.status == NORMAL) {
            dma += it.size;
        }
    }
    return dma;
}
} // namespace HiviewDFX
} // namespace OHOS
