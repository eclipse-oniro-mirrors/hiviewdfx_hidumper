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
#include "executor/memory/parse/parse_vmallocinfo.h"
#include <cstdlib>
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
    const int base = 10;
    value = strtoull(words.at(1).c_str(), nullptr, base);
    totalValue += value;
}

bool ParseVmallocinfo::GetVmallocinfo(uint64_t &value)
{
    value = 0;

    string filename = "/proc/vmallocinfo";
    ifstream in(filename);
    if (!in) {
        DUMPER_HILOGE(MODULE_SERVICE, "File %s not found.\n", filename.c_str());
        return false;
    }

    string str;
    while (getline(in, str)) {
        if (str.find("pages=") == string::npos) {
            continue;
        }
        CaclVmalloclValue(str, value);
    }

    in.close();

    return true;
}
} // namespace HiviewDFX
} // namespace OHOS