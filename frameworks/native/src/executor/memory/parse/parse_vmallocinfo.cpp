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
#include "executor/memory/parse/parse_vmallocinfo.h"
#include <fstream>
#include <sstream>
#include "executor/memory/memory_util.h"
#include "hilog_wrapper.h"

using namespace std;
namespace OHOS {
namespace HiviewDFX {
ParseVmallocinfo::ParseVmallocinfo()
{
}

ParseVmallocinfo::~ParseVmallocinfo()
{
}

void ParseVmallocinfo::CaclVmalloclValue(const string &str, uint64_t &totalValue)
{
    uint64_t value = 0;
    istringstream ss(str);
    string word;
    vector<string> words;
    while (ss >> word) {
        words.push_back(word);
    }
    value = atol(words.at(1).c_str());
    totalValue += value;
}

bool ParseVmallocinfo::GetVmallocinfo(uint64_t &value)
{
    bool success = false;
    string filename = "/proc/vmallocinfo";
    value = 0;
    ifstream in(filename);
    string str;
    vector<string> strs;
    if (in) {
        while (getline(in, str)) {
            strs.push_back(str);
        }
        for (auto str : strs) {
            CaclVmalloclValue(str, value);
        }
        in.close();
        success = true;
    } else {
        DUMPER_HILOGE(MODULE_SERVICE, "File %s not found.\n", filename.c_str());
    }
    return success;
}
} // namespace HiviewDFX
} // namespace OHOS