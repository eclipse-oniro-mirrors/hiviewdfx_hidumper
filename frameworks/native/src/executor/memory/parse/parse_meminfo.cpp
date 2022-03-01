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

#include "executor/memory/parse/parse_meminfo.h"
#include <fstream>
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
 * @param {PairMatrix} &result-Returned results
 * @return void
 */
void ParseMeminfo::SetData(const string &str, PairMatrix &result)
{
    string type = "";
    uint64_t value = 0;
    bool success = MemoryUtil::GetInstance().GetTypeValue(str, MemoryFilter::GetInstance().MEMINFO_TAG_, type, value);
    if (success) {
        result.push_back(make_pair(type, value));
    }
}

/**
 * @description: Get the data from meminfo
 * @param {PairMatrix} &meminfo - the meminfo result
 * @return bool-true:success,false-fail
 */
bool ParseMeminfo::GetMeminfo(PairMatrix &result)
{
    bool success = false;
    string filename = "/proc/meminfo";
    vector<string> strings;
    ifstream in(filename);
    string str;
    if (in) {
        while (getline(in, str)) {
            strings.push_back(str);
        }
        for (auto str : strings) {
            SetData(str, result);
        }
        success = true;
        in.close();
    } else {
        DUMPER_HILOGE(MODULE_SERVICE, "File %s not found.\n", filename.c_str());
    }

    return success;
}
} // namespace HiviewDFX
} // namespace OHOS
