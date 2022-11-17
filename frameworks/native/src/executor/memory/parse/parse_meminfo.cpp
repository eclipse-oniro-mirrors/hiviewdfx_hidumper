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

#include "executor/memory/parse/parse_meminfo.h"
#include <fstream>
#include "executor/memory/memory_filter.h"
#include "executor/memory/memory_util.h"
#include "hilog_wrapper.h"
#include "util/string_utils.h"

using namespace std;
namespace OHOS {
namespace HiviewDFX {
ParseMeminfo::ParseMeminfo()
{
}

ParseMeminfo::~ParseMeminfo()
{
}

/**
 * @description: SetData
 * @param {string} &str-String to be inserted into result
 * @param {ValueMap} &result-Returned results
 * @return void
 */
void ParseMeminfo::SetData(const string &str, ValueMap &result)
{
    string type = "";
    uint64_t value = 0;
    if (MemoryUtil::GetInstance().GetTypeValue(str, MemoryFilter::GetInstance().MEMINFO_TAG_, type, value)) {
        result.insert(pair<string, uint64_t>(type, value));
    }
}

/**
 * @description: Get the data from meminfo
 * @param {ValueMap} &meminfo - the meminfo result
 * @return bool-true:success,false-fail
 */
bool ParseMeminfo::GetMeminfo(ValueMap &result)
{
    string filename = "/proc/meminfo";
    ifstream in(filename);
    if (!in) {
        DUMPER_HILOGE(MODULE_SERVICE, "File %s not found.\n", filename.c_str());
        return false;
    }

    string str;
    while (getline(in, str)) {
        SetData(str, result);
    }

    in.close();

    return true;
}
} // namespace HiviewDFX
} // namespace OHOS
