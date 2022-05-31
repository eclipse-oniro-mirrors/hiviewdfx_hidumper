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
#ifndef PARSE_SMAPS_INFO_H
#define PARSE_SMAPS_INFO_H
#include <map>
#include <string>
#include <vector>
#include "executor/memory/memory_filter.h"
namespace OHOS {
namespace HiviewDFX {
class ParseSmapsInfo {
public:
    ParseSmapsInfo();
    ~ParseSmapsInfo();

    using ValueMap = std::map<std::string, uint64_t>;
    using GroupMap = std::map<std::string, ValueMap>;

    bool GetInfo(const MemoryFilter::MemoryType &memType, const int &pid, GroupMap &result);

private:
    std::string memGroup_ = "";

    bool GetValue(const MemoryFilter::MemoryType &memType, const std::string &str, std::string &type, uint64_t &value);
    bool GetHasPidValue(const std::string &str, std::string &type, uint64_t &value);
    bool GetNoPidValue(const std::string &str, std::string &type, uint64_t &value);
};
} // namespace HiviewDFX
} // namespace OHOS
#endif
