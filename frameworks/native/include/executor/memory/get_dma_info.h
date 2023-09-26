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
#ifndef GET_DMA_INFO_H
#define GET_DMA_INFO_H
#include <map>
#include <string>
#include <vector>
#include <memory>

#include "singleton.h"
#include "executor/memory/parse/meminfo_data.h"

namespace OHOS {
namespace HiviewDFX {
class GetDmaInfo : public Singleton<GetDmaInfo> {
public:
    GetDmaInfo();
    ~GetDmaInfo();

    GetDmaInfo(GetDmaInfo const &) = delete;
    void operator=(GetDmaInfo const &) = delete;

    using ValueMap = std::map<std::string, uint64_t>;
    using GroupMap = std::map<std::string, ValueMap>;
    bool GetDma();
    uint64_t GetDmaByPid(const int32_t &pid);
    uint64_t GetTotalDma();

private:
    enum Status
	{
		NORMAL,
		REPETITIVE1,
		REPETITIVE2
	};

    void SetData(const std::string &line);
    std::vector<MemInfoData::DmaInfo> GetDmaInfos();

    std::vector<MemInfoData::DmaInfo> dmaInfos;
    bool isFirst = false;
    const static int BYTE_PER_KB = 1024;
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // GET_DMA_INFO_H