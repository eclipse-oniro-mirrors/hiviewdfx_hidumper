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

#include "executor/memory/memory_filter.h"
#include <sstream>
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

const MemoryFilter::MemGroup MemoryFilter::memGroups_[] = {
    {
        .group_ = "so",
        .matchRule_ = "end",
        .matchFile_ = {".so", ".so.1"},
    },
    {
        .group_ = "heap",
        .matchRule_ = "begin",
        .matchFile_ = {"[heap]"},
    },
    {
        .group_ = "native",
        .matchRule_ = "begin",
        .matchFile_ = {"/system/bin/"},
    },
    {
        .group_ = "stack",
        .matchRule_ = "begin",
        .matchFile_ = {"[stack]"},
    },
    {
        .group_ = "ark js heap",
        .matchRule_ = "begin",
        .matchFile_ = {"[anon:Object Space]"},
    },
    {
        .group_ = "native heap",
        .matchRule_ = "begin",
        .matchFile_ = {"[anon:native_heap:musl"},
    },
};

bool MemoryFilter::ParseMemoryGroup(const string &content, const string &name, string &group)
{
    size_t groupSize = sizeof(MemoryFilter::memGroups_) / sizeof(MemoryFilter::MemGroup);

    for (size_t i = 0; i < groupSize; i++) {
        MemoryFilter::MemGroup memGroup = MemoryFilter::memGroups_[i];
        string rule = memGroup.matchRule_;
        vector<string> files = memGroup.matchFile_;

        for (auto file : files) {
            if (rule == "contain") {
                if (StringUtils::GetInstance().IsContain(name, file)) {
                    group = memGroup.group_;
                    break;
                }
            } else if (rule == "end") {
                if (StringUtils::GetInstance().IsEnd(name, file)) {
                    group = memGroup.group_;
                    break;
                }
            } else if (rule == "begin") {
                if (StringUtils::GetInstance().IsBegin(name, file)) {
                    group = memGroup.group_;
                    break;
                }
            }
        }
        if (!group.empty()) {
            break;
        }
    }
    return true;
}
} // namespace HiviewDFX
} // namespace OHOS
