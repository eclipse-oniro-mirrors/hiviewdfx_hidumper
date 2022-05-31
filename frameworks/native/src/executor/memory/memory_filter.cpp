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

bool MemoryFilter::ParseMemoryGroup(const string &name, string &group)
{
    if (StringUtils::GetInstance().IsEnd(name, ".so") ||
        StringUtils::GetInstance().IsEnd(name, ".so.1")) {
        group = "so";
    } else if (StringUtils::GetInstance().IsBegin(name, "/system/bin/")) {
        group = "native";
    } else if (StringUtils::GetInstance().IsBegin(name, "[heap]")) {
        group = "heap";
    } else if (StringUtils::GetInstance().IsBegin(name, "[stack]")) {
        group = "stack";
    } else if (StringUtils::GetInstance().IsBegin(name, "[anon:native_heap:musl")) {
        group = "native heap";
    } else if (StringUtils::GetInstance().IsBegin(name, "[anon:Object Space]")) {
        group = "ark js heap";
    } else {
        group = "other";
    }
    return true;
}
} // namespace HiviewDFX
} // namespace OHOS
