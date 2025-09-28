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
#include "task/writer/kernel_mem_info_writer.h"
#include <memory>
#include "data_inventory.h"
#include "hilog_wrapper.h"
#include "writer_utils.h"
#include "task/base/task_register.h"

namespace OHOS {
namespace HiviewDFX {

DumpStatus KernelMemInfoWriter::TaskEntry(DataInventory& dataInventory, const DumpContext& dumpContext)
{
    const std::vector<InfoConfig> infoConfigs = {
        { "/proc/slabinfo", DataId::PROC_SLAB_INFO },
        { "/proc/devhost/root/slabinfo", DataId::PROC_DEVHOST_SLAB_INFO },
        { "/proc/zoneinfo", DataId::PROC_ZONE_INFO },
        { "/proc/vmstat", DataId::PROC_VMSTAT_INFO },
        { "/proc/vmallocinfo", DataId::PROC_VMALLOC_INFO },
    };
    for (const auto& config : infoConfigs) {
        if (!config.title.empty()) {
            WriteTitle(config.title, dumpContext.GetOutputFd());
        }
        auto data = dataInventory.GetPtr<std::vector<std::string>>(config.dataId);
        if (!data) {
            DUMPER_HILOGE(MODULE_COMMON, "Failed to read %{public}s", config.title.c_str());
            continue;
        }
        WriteStringIntoFd(*data, dumpContext.GetOutputFd());
    }
    return DUMP_OK;
}

REGISTER_TASK(TaskId::WRITE_KERNEL_MEM_INFO, KernelMemInfoWriter, false,
              TaskId::DUMP_SLAB_INFO, TaskId::DUMP_ZONE_INFO, TaskId::DUMP_VMSTAT_INFO, TaskId::DUMP_VMALLOC_INFO);
REGISTER_DEPENDENT_DATA(TaskId::WRITE_KERNEL_MEM_INFO, DataId::PROC_SLAB_INFO, DataId::PROC_ZONE_INFO,
                        DataId::PROC_VMSTAT_INFO, DataId::PROC_VMALLOC_INFO, DataId::PROC_DEVHOST_SLAB_INFO);
} // namespace HiviewDFX
} // namespace OHOS
