/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#include <string>
#include <vector>
#include <memory>
#include "memory_filter.h"
#include "executor/cmd_dumper.h"
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

    using PairMatrix = std::vector<std::pair<std::string, uint64_t>>;
    using PairMatrixGroup = std::vector<std::pair<std::string, PairMatrix>>;

    std::string KB_UNIT_ = " kB";
    int BYTE_TO_KB_ = 1024;

    bool IsPss(const std::string &conent);
    bool GetRamValue(const std::vector<std::string> &ramInfo, uint64_t &value);
    bool GetMemMatchType(const std::string &content, std::string &type);
    void CalcGroup(const std::string &group, const std::string &type, const uint64_t &value, PairMatrixGroup &infos);
    void InsertGroupMap(const std::string &group, const std::string &type, const uint64_t &value,
                        PairMatrixGroup &infos);
    void GetMeminfoMatchType(const std::string &content, std::string &type);
    void GetGroupOfPids(const int &index, const int &size, const std::vector<int> &pids, std::vector<int> &groupResult);
    bool RunCMD(const std::string &cmd, std::vector<std::string> &result);
    size_t GetMaxThreadNum(const int &threadNum);
    bool GetKey(std::string &str);
    bool IsNameLine(const std::string &str, std::string &name);
    void ClacTotalByGroup(const PairMatrixGroup &infos, PairMatrixGroup &result);
    void ShowGroup(const PairMatrixGroup &infos);
    void ShowPairMatrix(const PairMatrix &infos);
    bool GetTypeValue(const std::string &str, const std::vector<std::string> &tag, std::string &type, uint64_t &value);
    void InitMemInfo(MemInfoData::MemInfo &memInfo);
    void InitMemUsage(MemInfoData::MemUsage &usage);
    bool GetTypeAndValue(const std::string &str, std::string &type, uint64_t &value);

private:
    void SpringMatrixTransToVector(const CMDDumper::StringMatrix dumpDatas, std::vector<std::string> &result);
    void InsertType(const std::string &group, const std::string &type, const uint64_t &value, PairMatrixGroup &infos);
};
} // namespace HiviewDFX
} // namespace OHOS
#endif
