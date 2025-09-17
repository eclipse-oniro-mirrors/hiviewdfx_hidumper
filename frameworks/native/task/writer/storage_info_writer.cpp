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
#include <memory>

#include "data_inventory.h"
#include "hilog_wrapper.h"
#include "writer_utils.h"
#include "task/base/task_register.h"
#include "task/writer/storage_info_writer.h"

namespace OHOS {
namespace HiviewDFX {

DumpStatus StorageInfoWriter::TaskEntry(DataInventory& dataInventory, const DumpContext& dumpContext)
{
    const std::string title = "-------------------------------[storage]-------------------------------";
    WriteTitle(title, dumpContext.GetOutputFd());

    const std::vector<InfoConfig> infoConfigs = {
        { "cmd is: storaged -u -p", DataId::STORAGE_STATE_INFO },
        { "cmd is: df -k", DataId::DF_INFO },
        { "cmd is: lsof", DataId::LSOF_INFO },
        { "cmd is: iotop -n 1 -m 100", DataId::IOTOP_INFO },
        { "/proc/mounts", DataId::PROC_MOUNTS_INFO }
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

REGISTER_TASK(TaskId::WRITE_STORAGE_INFO, StorageInfoWriter, true, TaskId::DUMP_DISK_INFO, TaskId::DUMP_LSOF_INFO,
              TaskId::DUMP_IOTOP_INFO, TaskId::DUMP_MOUNTS_INFO);
REGISTER_DEPENDENT_DATA(TaskId::WRITE_STORAGE_INFO, DataId::STORAGE_STATE_INFO, DataId::DF_INFO, DataId::LSOF_INFO,
                        DataId::IOTOP_INFO, DataId::PROC_MOUNTS_INFO);
} // namespace HiviewDFX
} // namespace OHOS
