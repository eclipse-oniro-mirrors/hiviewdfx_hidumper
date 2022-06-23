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

#include "executor/memory/memory_filter.h"
#include "util/string_utils.h"

using namespace std;
namespace OHOS {
namespace HiviewDFX {
MemoryFilter::MemoryFilter()
{
}
MemoryFilter::~MemoryFilter()
{
}

void MemoryFilter::ParseMemoryGroup(const string &name, string &group, uint64_t iNode)
{
    group = iNode > 0 ? FILE_PAGE_TAG : ANON_PAGE_TAG;
    group += "#";
    if (GetGroupFromMap(name, group, endMap_, bind(
                        &StringUtils::IsEnd, &StringUtils::GetInstance(), placeholders::_1, placeholders::_2)) ||
        GetGroupFromMap(name, group, beginMap_, bind(
            &StringUtils::IsBegin, &StringUtils::GetInstance(), placeholders::_1, placeholders::_2))) {
        return;
    }
    group += "other";
}

bool MemoryFilter::GetGroupFromMap(const string &name, string &group,
                                   const std::map<std::string, std::string> &map, MatchFunc func)
{
    for (const auto &p : map) {
        if (func(name, p.first)) {
            group += p.second;
            return true;
        }
    }
    return false;
}
} // namespace HiviewDFX
} // namespace OHOS
