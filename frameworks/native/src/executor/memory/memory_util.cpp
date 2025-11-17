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
#include "executor/memory/memory_util.h"
#include <cstdlib>
#include <fstream>
#include <thread>
#include <vector>
#include "securec.h"
#include "util/string_utils.h"
#include "hilog_wrapper.h"
using namespace std;
namespace OHOS {
namespace HiviewDFX {
MemoryUtil::MemoryUtil()
{
}

MemoryUtil::~MemoryUtil()
{
}

bool MemoryUtil::IsNameLine(const string &str, string &name, uint64_t &iNode)
{
    uint32_t len = 0;
    if (sscanf_s(str.c_str(), "%*llx-%*llx %*s %*llx %*s %llu%n", &iNode, &len) != 1) {
        return false;
    }

    while (len < str.size() && str[len] == ' ') {
        len++;
    }
    if (len < str.size()) {
        name = str.substr(len, str.size());
    }
    if (name.empty()) {
        name = "[anon]";
    }
    return true;
}

bool MemoryUtil::GetTypeValue(const string &str, const vector<string> &tags, string &type, uint64_t &value)
{
    type = "";
    string tempType = "";
    bool getSuccess = GetTypeAndValue(str, tempType, value);
    if (!getSuccess) {
        return false;
    }

    bool hasTag = false;
    auto iter = find(tags.begin(), tags.end(), tempType);
    if (iter != tags.end()) {
        hasTag = true;
    }
    if (!hasTag) {
        value = 0;
    }
    type = tempType;
    return true;
}

void MemoryUtil::CalcGroup(const string &group, const string &type, const uint64_t &value, GroupMap &infos)
{
    if (infos.find(group) == infos.end()) {
        map<string, uint64_t> valueMap;
        valueMap.insert(pair<string, uint64_t>(type, value));
        infos.insert(pair<string, map<string, uint64_t>>(group, valueMap));
    } else {
        if (infos[group].find(type) == infos[group].end()) {
            infos[group].insert(pair<string, uint64_t>(type, value));
        } else {
            infos[group][type] += value;
        }
    }
}

bool MemoryUtil::RunCMD(const string &cmd, vector<string> &result)
{
    std::lock_guard<std::mutex> lock(mutex_);
    FILE* fp = popen(("/system/bin/" + cmd).c_str(), "r");
    if (fp == nullptr) {
        return false;
    }
    char* buffer = nullptr;
    size_t len = 0;
    while (getline(&buffer, &len, fp) != -1) {
        std::string line = buffer;
        StringUtils::GetInstance().ReplaceAll(line, "\n", "");
        result.push_back(line);
    }
    // free memory
    if (buffer != nullptr) {
        free(buffer);
        buffer = nullptr;
    }
    pclose(fp);
    return true;
}

size_t MemoryUtil::GetMaxThreadNum(const size_t &threadNum)
{
    size_t maxThreadNum = 0;
    size_t const hardwareThreads = std::thread::hardware_concurrency();
    if (hardwareThreads == 0) {
        maxThreadNum = threadNum;
    } else if (hardwareThreads < threadNum) {
        maxThreadNum = hardwareThreads;
    } else {
        maxThreadNum = threadNum;
    }
    if (maxThreadNum == 0) {
        maxThreadNum = 1;
    }
    return maxThreadNum;
}

void MemoryUtil::InitMemInfo(MemInfoData::MemInfo &memInfo)
{
    memInfo.rss = 0;
    memInfo.pss = 0;
    memInfo.sharedClean = 0;
    memInfo.sharedDirty = 0;
    memInfo.privateClean = 0;
    memInfo.privateDirty = 0;
    memInfo.swap = 0;
    memInfo.swapPss = 0;
    memInfo.heapSize = 0;
    memInfo.heapAlloc = 0;
    memInfo.heapFree = 0;
}


void MemoryUtil::InitMemSmapsInfo(MemInfoData::MemSmapsInfo &memInfo)
{
    memInfo.rss = 0;
    memInfo.pss = 0;
    memInfo.sharedClean = 0;
    memInfo.sharedDirty = 0;
    memInfo.privateClean = 0;
    memInfo.privateDirty = 0;
    memInfo.swap = 0;
    memInfo.swapPss = 0;
    memInfo.name = "";
    memInfo.size = 0;
    memInfo.counts = 0;
    memInfo.start = "";
    memInfo.end = "";
    memInfo.perm = "";
}


void MemoryUtil::InitMemUsage(MemInfoData::MemUsage &usage)
{
    usage.vss = 0;
    usage.rss = 0;
    usage.uss = 0;
    usage.pss = 0;
    usage.gl = 0;
    usage.graph = 0;
    usage.purgSum = 0;
    usage.purgPin = 0;
    usage.pid = 0;
}

void MemoryUtil::InitGraphicsMemory(MemInfoData::GraphicsMemory &graphicsMemory)
{
    graphicsMemory.gl = 0;
    graphicsMemory.graph = 0;
}

bool MemoryUtil::GetTypeAndValue(const string &str, string &type, uint64_t &value)
{
    string::size_type typePos = str.find(":");
    if (typePos != str.npos) {
        type = str.substr(0, typePos);
        string valueStr = str.substr(typePos + 1);
        const int base = 10;
        value = strtoull(valueStr.c_str(), nullptr, base);
        return true;
    }
    return false;
}

void MemoryUtil::SetMemTotalValue(const string &value, vector<string> &lines, vector<string> &values, bool flag)
{
    if (!flag) {
        string separator = "-";
        string space = " ";
        StringUtils::GetInstance().SetWidth(LINE_WIDTH_, BLANK_, false, space);
        StringUtils::GetInstance().SetWidth(LINE_WIDTH_, SEPARATOR_, false, separator);
        lines.push_back(separator + SEPARATOR_);
    }
    string tempValue = value;
    if (flag) {
        if (StringUtils::GetInstance().IsSameStr(value, "Summary")) {
            constexpr int SMPAPS_INFO_WIDTH = 25;
            StringUtils::GetInstance().SetWidth(SMPAPS_INFO_WIDTH, BLANK_, false, tempValue);
        } else {
            StringUtils::GetInstance().SetWidth(SMAPS_LINE_WIDTH_, BLANK_, true, tempValue);
        }
    } else {
        StringUtils::GetInstance().SetWidth(LINE_WIDTH_, BLANK_, false, tempValue);
        tempValue += BLANK_;
    }
    values.push_back(tempValue);
}

uint64_t MemoryUtil::PermToInt(const string& perm)
{
    uint64_t iPerm = 0;
    for (size_t i = 0; i < perm.size(); i++) {
        if (perm[i] != '-' && perm[i] != 's') {
            iPerm |= (1 << i);
        }
    }
    return iPerm;
}
} // namespace HiviewDFX
} // namespace OHOS
