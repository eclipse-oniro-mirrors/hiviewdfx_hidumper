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
#ifndef HIDUMPER_ZIDL_COMMON_UTILS_H
#define HIDUMPER_ZIDL_COMMON_UTILS_H
#include <string>
#include <vector>
namespace OHOS {
namespace HiviewDFX {
class DumpCommonUtils {
public:
    struct CpuInfo {
        int id_;
    public:
        CpuInfo();
    };
    // get all cpu information in device.
    static bool GetCpuInfos(std::vector<CpuInfo> &infos);
    struct PidInfo {
        int pid_;
        int ppid_;
        int uid_;
        int gid_;
        std::string name_;
        std::string cmdline_;
    public:
        PidInfo();
        void Reset();
    };
    // get all process information in device.
    static bool GetPidInfos(std::vector<PidInfo> &infos, bool all = false);
    // get process name by pid.
    static bool GetProcessNameByPid(int pid, std::string &name);
    // get process information by pid.
    static bool GetProcessInfo(int pid, PidInfo &info);
    // check head of string.
    static bool StartWith(const std::string& str, const std::string& head);
    static bool GetUserPids(std::vector<int> &pids);
    static bool IsUserPid(const std::string &pid);
private:
    static bool GetLinesInFile(const std::string& file, std::vector<std::string>& lines);
    static bool GetNamesInFolder(const std::string& folder, std::vector<std::string>& names);
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIDUMPER_ZIDL_COMMON_UTILS_H
