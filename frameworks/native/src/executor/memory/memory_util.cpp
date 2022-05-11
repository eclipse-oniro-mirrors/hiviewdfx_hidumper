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
#include "executor/memory/memory_util.h"
#include <cstdlib>
#include <iostream>
#include <thread>
#include <vector>
#include "executor/cmd_dumper.h"
#include "executor/memory/memory_filter.h"
#include "util/string_utils.h"
#include "securec_p.h"
using namespace std;
namespace OHOS {
namespace HiviewDFX {
MemoryUtil::MemoryUtil()
{
}

MemoryUtil::~MemoryUtil()
{
}

bool MemoryUtil::IsNameLine(const string &str, string &name)
{
    uint64_t iNode = 0;
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

void MemoryUtil::CalcGroup(const string &group, const string &type, const uint64_t &value, PairMatrixGroup &infos)
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

bool MemoryUtil::IsPss(const string &type)
{
    for (auto str : MemoryFilter::GetInstance().CALC_PSS_TOTAL_) {
        if (type == str) {
            return true;
        }
    }
    return false;
}

void MemoryUtil::SpringMatrixTransToVector(const CMDDumper::StringMatrix dumpDatas, vector<string> &result)
{
    for (size_t i = 0; i < dumpDatas->size(); i++) {
        vector<string> line = dumpDatas->at(i);
        for (size_t j = 0; j < line.size(); j++) {
            string str = line[j];
            StringUtils::GetInstance().ReplaceAll(str, "\n", "");
            result.push_back(str);
        }
    }
}

bool MemoryUtil::RunCMD(const string &cmd, vector<string> &result)
{
    CMDDumper::StringMatrix dumpDatas = make_unique<vector<vector<string>>>();

    shared_ptr<CMDDumper> cmdDumper = make_shared<CMDDumper>();
    DumpStatus status = cmdDumper->GetCmdInterface(cmd, dumpDatas);
    if (status == DumpStatus::DUMP_OK) {
        SpringMatrixTransToVector(dumpDatas, result);
    } else {
        return false;
    }

    return true;
}

size_t MemoryUtil::GetMaxThreadNum(const int &threadNum)
{
    int maxThreadNum = 0;
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

/**
 * @description: Get the key and determine whether to add or subtract
 * @param {string} &str-string
 * @param {string} &key-get the key
 * @return {bool}-true:subtract,false-add
 */
bool MemoryUtil::GetKey(std::string &str)
{
    bool subtract = false;
    if (StringUtils::GetInstance().IsBegin(str, "-")) {
        StringUtils::GetInstance().ReplaceAll(str, "-", "");
        subtract = true;
    }
    return subtract;
}

/**
 * @description: Calculate the sum according to group
 * @param {PairMatrixGroup} &infos-To calculate the information
 * @param {PairMatrixGroup} &result-Result of calculation
 * @return {*}
 */
/**
void MemoryUtil::ClacTotalByGroup(const PairMatrixGroup &infos, PairMatrixGroup &result)
{
    if (infos.size() > 0) {
        if (result.size() == 0) {
            result.assign(infos.begin(), infos.end());
        } else {
            for (size_t i = 0; i < infos.size(); i++) {
                auto info = infos.at(i);
                string group = info.first;
                auto pairMatrix = info.second;
                for (auto pair : pairMatrix) {
                    string type = pair.first;
                    uint64_t value = pair.second;
                    MemoryUtil::GetInstance().CalcGroup(group, type, value, result);
                }
            }
        }
    }
}
**/

void MemoryUtil::ShowGroup(const PairMatrixGroup &infos)
{
    for (auto info : infos) {
        string group = info.first;
        auto pairs = info.second;
        for (auto pair : pairs) {
            cout << "group:" << group << ",type:" << pair.first << ",value:" << pair.second << endl;
        }
    }
}

void MemoryUtil::ShowPairMatrix(const PairMatrix &infos)
{
    for (auto info : infos) {
        cout << "type:" << info.first << ",value:" << info.second << endl;
    }
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
}

void MemoryUtil::InitMemUsage(MemInfoData::MemUsage &usage)
{
    usage.vss = 0;
    usage.rss = 0;
    usage.uss = 0;
    usage.pss = 0;
    usage.pid = 0;
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
} // namespace HiviewDFX
} // namespace OHOS
