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
#include <unistd.h>
#include "executor/cpu_dumper.h"
#include "datetime_ex.h"
#include "dump_utils.h"
#include "securec.h"
#include "util/string_utils.h"
#include "dump_manager_cpu_client.h"

namespace OHOS {
namespace HiviewDFX {
CPUDumper::CPUDumper()
{
}

CPUDumper::~CPUDumper()
{
}

DumpStatus CPUDumper::PreExecute(const std::shared_ptr<DumperParameter> &parameter, StringMatrix dumpDatas)
{
    DUMPER_HILOGD(MODULE_COMMON, "debug|CPUDumper PreExecute");
    dumpCPUDatas_ = dumpDatas;
    isDumpCpuUsage_ = (parameter->GetOpts()).isDumpCpuUsage_;
    cpuUsagePid_ = (parameter->GetOpts()).cpuUsagePid_;
    return DumpStatus::DUMP_OK;
}

DumpStatus CPUDumper::Execute()
{
    if (isDumpCpuUsage_) {
        auto& dumpManagerCpuClient = DumpManagerCpuClient::GetInstance();
        DumpCpuData dumpData = GetDumpCpuData();
        dumpManagerCpuClient.Request(dumpData);
        dumpCPUDatas_->clear();
        for (auto it = dumpData.dumpCPUDatas_.begin(); it != dumpData.dumpCPUDatas_.end(); it++) {
            std::vector<std::string> strVec;
            for (auto str = it->begin(); str != it->end(); str++) {
                strVec.push_back(*str);
            }
            dumpCPUDatas_->push_back(strVec);
        }
        return DumpStatus::DUMP_OK;
    } else {
        return DumpStatus::DUMP_FAIL;
    }
}

DumpStatus CPUDumper::AfterExecute()
{
    return DumpStatus::DUMP_OK;
}

DumpCpuData CPUDumper::GetDumpCpuData()
{
    DumpCpuData dumpCpu;
    dumpCpu.startTime_ = startTime_;
    dumpCpu.endTime_ = endTime_;
    dumpCpu.dumpCPUDatas_ = *dumpCPUDatas_;
    dumpCpu.cpuUsagePid_ = cpuUsagePid_;
    return dumpCpu;
}
} // namespace HiviewDFX
} // namespace OHOS
