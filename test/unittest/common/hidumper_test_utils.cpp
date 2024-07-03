/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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
#include <gtest/gtest.h>
#include "hidumper_test_utils.h"

using namespace std;
namespace OHOS {
namespace HiviewDFX {
HidumperTestUtils::HidumperTestUtils()
{
}

HidumperTestUtils::~HidumperTestUtils()
{
}

bool HidumperTestUtils::IsExistInCmdResult(const std::string &cmd, const std::string &str)
{
    bool res = false;
    size_t len = 0;
    FILE *fp = popen(cmd.c_str(), "r");
    char* buffer = nullptr;
    while (getline(&buffer, &len, fp) != -1) {
        std::string line = buffer;
        if (line.find(str) != string::npos) {
            res = true;
            break;
        }
    }
    pclose(fp);
    return res;
}
} // namespace HiviewDFX
} // namespace OHOS
