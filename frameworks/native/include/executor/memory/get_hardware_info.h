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
#ifndef GET_HARDWARE_INFO_H
#define GET_HARDWARE_INFO_H
#include <string>
#include <vector>
namespace OHOS {
namespace HiviewDFX {
class GetHardwareInfo {
public:
    GetHardwareInfo();
    ~GetHardwareInfo();

    bool GetHardwareUsage(uint64_t &totalValue);
    void Stop();

private:
    const static int WORD_SIZE = 4;
    bool FindFilePaths(std::vector<std::string> &result);
    void GetGroupOfPaths(const size_t &index, const size_t &size, const std::vector<std::string> &paths,
                         std::vector<std::string> &groupPaths);
    void static GetResverRegPath(std::string &path);
    void static GetValue(const std::string &str, uint64_t &value);
    bool static RegStrToRegValue(const std::vector<std::string> &infos, uint64_t &value);
    bool static GetResverRegValue(const std::string &path, uint64_t &value);
    uint64_t static CalcHardware(const std::vector<std::string> &paths);
};
} // namespace HiviewDFX
} // namespace OHOS

#endif