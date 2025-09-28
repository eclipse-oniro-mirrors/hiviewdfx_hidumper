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


#include "task/cpu/cpu_freq_info_task.h"

#include "data_inventory.h"
#include "hilog_wrapper.h"
#include "file_ex.h"
#include "task/base/task_register.h"
#include "writer_utils.h"

namespace OHOS {
namespace HiviewDFX {
DumpStatus CpuFreqInfoTask::TaskEntry(DataInventory& dataInventory, const DumpContext& dumpContext)
{
    int platformCpuNum = static_cast<int>(sysconf(_SC_NPROCESSORS_ONLN));
    std::vector<CpuFreqInfo> cpuFreqInfoVec;
    for (int i = 0; i < platformCpuNum; i++) {
        std::string curFreqPath = "/sys/devices/system/cpu/cpu" + std::to_string(i) + "/cpufreq/cpuinfo_cur_freq";
        std::string maxFreqPath = "/sys/devices/system/cpu/cpu" + std::to_string(i) + "/cpufreq/cpuinfo_max_freq";
        std::string curFreq = "";
        std::string maxFreq = "";
        if (!LoadStringFromFile(curFreqPath, curFreq) || !LoadStringFromFile(maxFreqPath, maxFreq)) {
            DUMPER_HILOGE(MODULE_COMMON, "Failed to read cpu freq info");
            return DUMP_FAIL;
        }
        cpuFreqInfoVec.push_back({i, curFreq, maxFreq});
    }
    dataInventory.Inject(DataId::CPU_FREQ_INFO, std::make_shared<std::vector<CpuFreqInfo>>(cpuFreqInfoVec));
    return DUMP_OK;
}

REGISTER_TASK(TaskId::DUMP_CPU_FREQ_INFO, CpuFreqInfoTask, false);
} // namespace HiviewDFX
} // namespace OHOS