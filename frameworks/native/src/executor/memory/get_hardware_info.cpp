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

#include "executor/memory/get_hardware_info.h"
#include <future>
#include <sstream>
#include <thread>
#include "executor/memory/memory_filter.h"
#include "executor/memory/memory_util.h"
#include "hilog_wrapper.h"
#include "util/string_utils.h"

using namespace std;
namespace OHOS {
namespace HiviewDFX {
GetHardwareInfo::GetHardwareInfo()
{
}
GetHardwareInfo::~GetHardwareInfo()
{
}

/**
 * @description: Find the list of directory that contain the no-map node
 * @param {vector<string>} &result-A list of directories found
 * @return {bool}-true:success,false:fail
 */
bool GetHardwareInfo::FindFilePaths(vector<string> &result)
{
    string cmd = "find /proc/device-tree/ -name \"no-map\"";
    bool success = MemoryUtil::GetInstance().RunCMD(cmd, result);
    return success;
}

/**
 * @description: Get the reg node directory
 * @param {string} &path-Reg node directory
 * @return {*}
 */
void GetHardwareInfo::GetResverRegPath(string &path)
{
    vector<string> strs;
    StringUtils::GetInstance().StringSplit(path, "/", strs);
    path = "";
    strs[strs.size() - 1] = "reg";
    for (size_t i = 0; i < strs.size(); i++) {
        path += "/";
        path += strs.at(i);
    }
}

/**
 * @description: Get value from the string
 * @param {string} &str:string
 * @param {string} &result:the value
 * @return {*}
 */
void GetHardwareInfo::GetValue(const std::string &str, uint64_t &value)
{
    value = 0;
    istringstream ss(str);
    string word;
    vector<string> words;
    while (ss >> word) {
        words.push_back(word);
    }
    if (words.size() >= WORD_SIZE) {
        string tempWord = words.at(3);
        StringUtils::GetInstance().HexToDec(tempWord, value);
    }
}

bool GetHardwareInfo::RegStrToRegValue(const vector<string> &infos, uint64_t &value)
{
    string tempValueStr = "";
    vector<string> tempStrs;
    for (string str : infos) {
        StringUtils::GetInstance().StringSplit(str, ":", tempStrs);
        if (tempStrs.size() > 0) {
            tempValueStr = tempStrs[1];
            uint64_t tempValue = 0;
            GetValue(tempValueStr, tempValue);
            value += tempValue;
        } else {
            DUMPER_HILOGE(MODULE_SERVICE, "Hardware Usage fail, get reg value fail .\n");
            return false;
        }
    }
    return true;
}

bool GetHardwareInfo::GetResverRegValue(const string &path, uint64_t &value)
{
    string cmd = "xxd -g 4 " + path;
    vector<string> valueStrs;
    bool success = MemoryUtil::GetInstance().RunCMD(cmd, valueStrs);
    if (success) {
        RegStrToRegValue(valueStrs, value);
    } else {
        DUMPER_HILOGE(MODULE_SERVICE, "Hardware Usage fail, get reg value by run cmd fail .\n");
        return false;
    }
    return true;
}

uint64_t GetHardwareInfo::CalcHardware(const vector<string> &paths)
{
    uint64_t totalValue = 0;
    if (paths.size() > 0) {
        for (string path : paths) {
            GetResverRegPath(path);
            if (!path.empty()) {
                uint64_t value = 0;
                GetResverRegValue(path, value);
                totalValue += value;
            }
        }
    }
    return totalValue;
}

void GetHardwareInfo::GetGroupOfPaths(const size_t &index, const size_t &size, const std::vector<std::string> &paths,
                                      std::vector<string> &groupPaths)
{
    for (size_t i = index * size; i <= (index + 1) * size - 1; i++) {
        if (i <= paths.size() - 1) {
            groupPaths.push_back(paths.at(i));
        }
    }
}

bool GetHardwareInfo::GetHardwareUsage(uint64_t &totalValue)
{
    totalValue = 0;
    vector<string> paths;
    bool findPathSuccess = FindFilePaths(paths);
    if (findPathSuccess) {
        size_t size = paths.size();
        if (size > 0) {
            size_t threadNum =
                MemoryUtil::GetInstance().GetMaxThreadNum(MemoryFilter::GetInstance().HARDWARE_USAGE_THREAD_NUM_);
            if (threadNum == 0) {
                threadNum = 1;
            }
            size_t groupSize = (size - 1) / threadNum + 1;

            std::vector<future<uint64_t>> results;

            for (size_t i = 0; i < threadNum; i++) {
                vector<string> groupPaths;
                GetGroupOfPaths(i, groupSize, paths, groupPaths);
                auto future = std::async(std::launch::async, CalcHardware, groupPaths);
                results.emplace_back(std::move(future));
            }

            for (auto &tempResult : results) {
                uint64_t value = tempResult.get();
                totalValue += value;
            }
            totalValue = totalValue / MemoryUtil::GetInstance().BYTE_TO_KB_;
        } else {
            DUMPER_HILOGD(MODULE_SERVICE, "GetHardwareInfo file path size is 0\n");
        }

        return true;
    } else {
        return false;
    }
}
} // namespace HiviewDFX
} // namespace OHOS
