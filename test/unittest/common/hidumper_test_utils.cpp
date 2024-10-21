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
#include <fstream>
#include <sstream>
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
    std::string line = "";
    return GetSpecialLine(cmd, str, line);
}

bool HidumperTestUtils::IsExistStrInFile(const std::string &cmd, const std::string &str, const std::string &filePath)
{
    bool res = false;
    FILE *fp = popen(cmd.c_str(), "r");
    pclose(fp);

    std::ifstream file(filePath);
    if (!file.is_open()) {
        return res;
    }
    std::string line;
    while (std::getline(file, line)) {
        if (line.find(str) != string::npos) {
            res = true;
            break;
        }
    }
    file.close();
    return res;
}

pid_t HidumperTestUtils::GetPidByName(const std::string& processName)
{
    FILE *fp = nullptr;
    char buf[100]; // 100: buf size
    pid_t pid = -1;
    std::string cmd = "pidof " + processName;
    if ((fp = popen(cmd.c_str(), "r")) != nullptr) {
        if (fgets(buf, sizeof(buf), fp) != nullptr) {
            pid = std::atoi(buf);
        }
        pclose(fp);
    }
    return pid;
}

bool HidumperTestUtils::GetSpecialLine(const std::string &cmd, const std::string &str, std::string &specialLine)
{
    bool res = false;
    size_t len = 0;
    FILE *fp = popen(cmd.c_str(), "r");
    char* buffer = nullptr;
    while (getline(&buffer, &len, fp) != -1) {
        std::string line = buffer;
        if (line.find(str) != string::npos) {
            res = true;
            specialLine = line;
            break;
        }
    }
    pclose(fp);
    return res;
}

std::string HidumperTestUtils::GetValueInLine(const std::string &line, int index)
{
    std::istringstream iss(line);
    std::vector<std::string> tokens;
    std::string token;
    while (iss >> token) {
        tokens.push_back(token);
    }
    if (index < tokens.size()) {
        return tokens[index];
    }
    return "";
}
} // namespace HiviewDFX
} // namespace OHOS
