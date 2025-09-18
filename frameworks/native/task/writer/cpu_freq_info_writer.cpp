/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "task/writer/cpu_freq_info_writer.h"
#include <memory>
#include "data_inventory.h"
#include "hilog_wrapper.h"
#include "writer_utils.h"
#include "task/base/task_register.h"

namespace OHOS {
namespace HiviewDFX {

DumpStatus CpuFreqInfoWriter::TaskEntry(DataInventory& dataInventory, const DumpContext& dumpContext)
{
    if (!dumpContext.GetDumperOpts()->isDumpSystem) {
        const std::string title = "-------------------------------[cpufreq]-------------------------------";
        WriteTitle(title, dumpContext.GetOutputFd());
    }

    auto data = dataInventory.GetPtr<std::vector<CpuFreqInfo>>(DataId::CPU_FREQ_INFO);
    if (!data || data->empty()) {
        DUMPER_HILOGW(MODULE_COMMON, "Failed to read cpu freq info");
        return DUMP_FAIL;
    }
    for (auto& info : *data) {
        std::string curFreqTitle = "cmd is: cat /sys/devices/system/cpu/cpu" + std::to_string(info.cpuId)
                                   + "/cpufreq/cpuinfo_cur_freq";
        WriteTitle(curFreqTitle, dumpContext.GetOutputFd());
        WriteStringIntoFd(info.curFreq, dumpContext.GetOutputFd());
        std::string maxFreqTitle = "cmd is: cat /sys/devices/system/cpu/cpu" + std::to_string(info.cpuId)
                                   + "/cpufreq/cpuinfo_max_freq";
        WriteTitle(maxFreqTitle, dumpContext.GetOutputFd());
        WriteStringIntoFd(info.maxFreq, dumpContext.GetOutputFd());
    }
    return DUMP_OK;
}

REGISTER_TASK(TaskId::WRITE_CPU_FREQ_INFO, CpuFreqInfoWriter, false, TaskId::DUMP_CPU_FREQ_INFO);
REGISTER_DEPENDENT_DATA(TaskId::WRITE_CPU_FREQ_INFO, DataId::CPU_FREQ_INFO);
} // namespace HiviewDFX
} // namespace OHOS
