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
#ifndef HIDUMPER_SERVICES_DUMPER_OPTS_H
#define HIDUMPER_SERVICES_DUMPER_OPTS_H
#include <string>
#include <vector>
namespace OHOS {
namespace HiviewDFX {
struct DumperOpts {
    bool isDumpCpuFreq_;
    bool isDumpCpuUsage_;
    int cpuUsagePid_;
    bool isDumpLog_;
    std::vector<std::string> logArgs_;
    bool isDumpMem_;
    int memPid_;
    bool isDumpStorage_;
    bool isDumpNet_;
    bool isDumpList_;
    bool isDumpService_;
    bool isDumpSystemAbility_;
    std::vector<std::string> abilitieNames_;
    std::vector<std::string> abilitieArgs_;
    bool isDumpSystem_;
    std::vector<std::string> systemArgs_;
    bool isDumpProcesses_;
    int processPid_;
    bool isFaultLog_;
    int timeout_;
    int limitSize_;
    std::string path_; // for zip
    bool isAppendix_;
    bool isTest_;
public:
    DumperOpts();
    void Reset();
    DumperOpts& operator = (const DumperOpts& opts);
    void AddSelectAll();
    bool IsSelectAny() const;
    bool IsDumpZip() const;
    bool CheckOptions(std::string& errStr) const;
    void Dump() const;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIDUMPER_SERVICES_DUMPER_OPTS_H
