/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include "dump_common_utils.h"
#include <file_ex.h>
#include <securec.h>
#include <string_ex.h>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include "hilog_wrapper.h"
using namespace std;
namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr int LINE_ITEM_MIN = 2;
constexpr int LINE_KEY = 0;
constexpr int LINE_VALUE = 1;
constexpr int LINE_VALUE_0 = 0;
constexpr int UNSET = -1;
static const std::string CPU_STR = "cpu";
}

DumpCommonUtils::CpuInfo::CpuInfo()
{
    id_ = UNSET;
}

bool DumpCommonUtils::GetCpuInfos(std::vector<CpuInfo> &infos)
{
    std::vector<std::string> names;
    if (!GetNamesInFolder("/sys/devices/system/cpu/", names)) {
        return false;
    }
    for (size_t i = 0; i < names.size(); i++) {
        std::string name = names[i];
        if (!StartWith(name, CPU_STR)) {
            continue;
        }
        std::string cpuId = name.substr(CPU_STR.size());
        if (cpuId.empty() || (!IsNumericStr(cpuId))) {
            continue;
        }
        CpuInfo cpuInfo;
        StrToInt(cpuId, cpuInfo.id_);
        infos.push_back(cpuInfo);
    }
    return true;
}

DumpCommonUtils::PidInfo::PidInfo()
{
    Reset();
}

void DumpCommonUtils::PidInfo::Reset()
{
    pid_ = UNSET;
    uid_ = UNSET;
    gid_ = UNSET;
    ppid_ = UNSET;
    name_.clear();
    cmdline_.clear();
}

bool DumpCommonUtils::GetPidInfos(std::vector<PidInfo> &infos, bool all)
{
    std::vector<std::string> names;
    if (!GetNamesInFolder("/proc/", names)) {
        return false;
    }
    for (size_t i = 0; i < names.size(); i++) {
        std::string name = names[i];
        if (name.empty()) {
            continue;
        }
        if (!IsNumericStr(name)) {
            continue;
        }
        PidInfo pidInfo;
        StrToInt(name, pidInfo.pid_);
        GetProcessInfo(pidInfo.pid_, pidInfo);
        if (all) {
            GetProcessNameByPid(pidInfo.pid_, pidInfo.cmdline_);
        }
        infos.push_back(pidInfo);
    }
    return true;
}

bool DumpCommonUtils::IsUserPid(const std::string &pid)
{
    string filename = "/proc/" + pid + "/smaps";
    std::ifstream in(filename);
    if (in) {
        string line;
        getline(in, line);
        if (!line.empty()) {
            return true;
        }
        in.close();
    }
    return false;
}

bool DumpCommonUtils::GetUserPids(std::vector<int> &pids)
{
    std::vector<std::string> files;
    if (!GetNamesInFolder("/proc/", files)) {
        return false;
    }

    for (auto file : files) {
        if (file.empty()) {
            continue;
        }
        if (!IsNumericStr(file)) {
            continue;
        }

        if (!IsUserPid(file)) {
            continue;
        }

        int pid;
        StrToInt(file, pid);

        pids.push_back(pid);
    }
    return true;
}

bool DumpCommonUtils::GetLinesInFile(const std::string& file, std::vector<std::string>& lines)
{
    std::string content;
    if (!LoadStringFromFile(file, content)) {
        return false;
    }
    SplitStr(content, "\n", lines);
    return true;
}

bool DumpCommonUtils::GetNamesInFolder(const std::string& folder, std::vector<std::string>& names)
{
    bool ret = false;
    DIR* dir = nullptr;
    if ((dir = opendir(folder.c_str())) != nullptr) {
        dirent* ptr = nullptr;
        while ((ptr = readdir(dir)) != nullptr) {
            std::string name = ptr->d_name;
            if ((name == ".") || (name == "..")) {
                continue;
            }
            names.push_back(name);
        }
        closedir(dir);
        ret = true;
    }
    return ret;
}

bool DumpCommonUtils::StartWith(const std::string& str, const std::string& head)
{
    if (str.length() < head.length()) {
        return false;
    }
    return (str.compare(0, head.size(), head) == 0);
}

bool DumpCommonUtils::GetProcessNameByPid(int pid, std::string& name)
{
    char filesysdir[128] = { 0 };
    if (sprintf_s(filesysdir, sizeof(filesysdir), "/proc/%d/cmdline", pid) < 0) {
        return false;
    }
    std::string filePath = filesysdir;
    std::string content;
    if (!LoadStringFromFile(filePath, content)) {
        return false;
    }
    name = content;
    return true;
}

bool DumpCommonUtils::GetProcessInfo(int pid, PidInfo &info)
{
    info.Reset();
    char filesysdir[128] = { 0 };
    if (sprintf_s(filesysdir, sizeof(filesysdir), "/proc/%d/status", pid) < 0) {
        return false;
    }
    std::string file = filesysdir;
    std::vector<std::string> lines;
    if (!GetLinesInFile(file, lines)) {
        return false;
    }
    const std::string splitKeyValueToken = ":";
    const std::string splitValuesToken = "\t";
    for (size_t i = 0; i < lines.size(); i++) {
        std::vector<std::string> keyValue;
        SplitStr(lines[i], splitKeyValueToken, keyValue);
        if (keyValue.size() < LINE_ITEM_MIN) {
            continue;
        }
        std::string key = keyValue[LINE_KEY];
        std::string val = TrimStr(keyValue[LINE_VALUE], '\t');
        std::vector<std::string> values;
        SplitStr(val, splitValuesToken, values, true);
        if (values.empty()) {
            continue;
        }
        std::string val0 = values[LINE_VALUE_0];
        if (key == "Pid") {
            StrToInt(val0, info.pid_);
        } else if (key == "PPid") {
            StrToInt(val0, info.ppid_);
        } else if (key == "Uid") {
            StrToInt(val0, info.uid_);
        } else if (key == "Gid") {
            StrToInt(val0, info.gid_);
        } else if (key == "Name") {
            info.name_ = val;
        } else {
            continue;
        }
        if ((info.pid_ > UNSET) && (info.ppid_ > UNSET) && (info.uid_ > UNSET) && (info.gid_ > UNSET)) {
            return true;
        }
    }
    return false;
}
} // namespace HiviewDFX
} // namespace OHOS
