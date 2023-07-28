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
#include "util/file_utils.h"

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
    string path = "/proc/vmallocinfo";
    bool ret = FileUtils::GetInstance().LoadStringFromProcCb(path, false, true, [&](const string& line) -> void {
        if (line.find("pages=") != string::npos) {
            CaclVmalloclValue(line, value);
        }
    });
    return ret;
}
} // namespace HiviewDFX
} // namespace OHOS