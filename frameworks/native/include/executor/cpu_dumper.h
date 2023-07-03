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
#ifndef CPU_DUMPER_H
#define CPU_DUMPER_H
#include "util/dump_cpu_info_util.h"
#include "hidumper_executor.h"
#include "dump_cpu_data.h"

namespace OHOS {
namespace HiviewDFX {
class CPUDumper : public HidumperExecutor {
public:
    CPUDumper();
    ~CPUDumper();
    DumpStatus PreExecute(const std::shared_ptr<DumperParameter>& parameter,
        StringMatrix dumpDatas) override;
    DumpStatus Execute() override;
    DumpStatus AfterExecute() override;

private:
    DumpCpuData GetDumpCpuData();

    StringMatrix dumpCPUDatas_;
    bool isDumpCpuUsage_ = false;
    int cpuUsagePid_ = -1;
    std::shared_ptr<CPUInfo> curCPUInfo_;
    std::shared_ptr<CPUInfo> oldCPUInfo_;
    std::vector<std::shared_ptr<ProcInfo>> curProcs_;
    std::vector<std::shared_ptr<ProcInfo>> oldProcs_;
    std::shared_ptr<ProcInfo> curSpecProc_;
    std::shared_ptr<ProcInfo> oldSpecProc_;
    std::string startTime_;
    std::string endTime_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // CPU_DUMPER_H
