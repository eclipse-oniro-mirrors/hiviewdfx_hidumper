/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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
#ifndef MEMORY_UTIL_H
#define MEMORY_UTIL_H
#include <map>
#include <string>
#include <vector>
#include <memory>
#include "singleton.h"
#include "executor/memory/parse/meminfo_data.h"
namespace OHOS {
namespace HiviewDFX {
class MemoryUtil : public Singleton<MemoryUtil> {
public:
    MemoryUtil();
    ~MemoryUtil();

    MemoryUtil(MemoryUtil const &) = delete;
    void operator=(MemoryUtil const &) = delete;

    using ValueMap = std::map<std::string, uint64_t>;
    using GroupMap = std::map<std::string, ValueMap>;

    std::string KB_UNIT_ = " kB";
    uint64_t BYTE_TO_KB_ = 1024;

    void CalcGroup(const std::string &group, const std::string &type, const uint64_t &value, GroupMap &infos);
    bool RunCMD(const std::string &cmd, std::vector<std::string> &result);
    size_t GetMaxThreadNum(const size_t &threadNum);
    bool IsNameLine(const std::string &str, std::string &name, uint64_t &iNode);
    bool GetTypeValue(const std::string &str, const std::vector<std::string> &tag, std::string &type, uint64_t &value);
    void InitMemInfo(MemInfoData::MemInfo &memInfo);
    void InitMemUsage(MemInfoData::MemUsage &usage);
    void InitGraphicsMemory(MemInfoData::GraphicsMemory &graphicsMemory);
    bool GetTypeAndValue(const std::string &str, std::string &type, uint64_t &value);

private:
};
} // namespace HiviewDFX
} // namespace OHOS
#endif
