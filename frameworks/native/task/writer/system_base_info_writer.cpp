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
#include "task/writer/system_base_info_writer.h"
#include <memory>
#include <string>
#include <vector>
#include "data_inventory.h"
#include "dump_context.h"
#include "hilog_wrapper.h"
#include "writer_utils.h"
#include "task/base/task_register.h"

namespace OHOS {
namespace HiviewDFX {

DumpStatus SystemBaseInfoWriter::WriteSystemBaseInfo(DataInventory& dataInventory,
                                                     const std::shared_ptr<DumpContext>& dumpContext,
                                                     const InfoConfig& config)
{
    if (!config.title.empty()) {
        WriteTitle(config.title, dumpContext->GetOutputFd());
    }
    auto data = dataInventory.GetPtr<std::vector<std::string>>(config.dataId);
    if (!data) {
        DUMPER_HILOGE(MODULE_COMMON, "Failed to read %{public}s", config.title.c_str());
        return DUMP_FAIL;
    }
    WriteStringIntoFd(*data, dumpContext->GetOutputFd());
    return DUMP_OK;
}

DumpStatus SystemBaseInfoWriter::TaskEntry(DataInventory& dataInventory,
                                           const std::shared_ptr<DumpContext>& dumpContext)
{
    const std::string title = "-------------------------------[base]-------------------------------";
    WriteTitle(title, dumpContext->GetOutputFd());

    const std::vector<InfoConfig> infoConfigs = {
        { "", DataId::DEVICE_INFO },
        { "/proc/version", DataId::PROC_VERSION_INFO },
        { "/proc/cmdline", DataId::PROC_CMDLINE_INFO },
        { "/sys/kernel/debug/wakeup_sources", DataId::WAKEUP_SOURCES_INFO },
        { "cmd is: uptime -p", DataId::UPTIME_INFO }
    };
    for (const auto& config : infoConfigs) {
        if (WriteSystemBaseInfo(dataInventory, dumpContext, config) != DUMP_OK) {
            return DUMP_FAIL;
        }
    }
    return DUMP_OK;
}

REGISTER_TASK(WRITE_SYSTEM_BASE_INFO, SystemBaseInfoWriter, true, DUMP_DEVICE_INFO, DUMP_WAKEUP_SOURCES_INFO);
REGISTER_DEPENDENT_DATA(WRITE_SYSTEM_BASE_INFO, DEVICE_INFO, PROC_VERSION_INFO,
                        PROC_CMDLINE_INFO, WAKEUP_SOURCES_INFO, UPTIME_INFO);
} // namespace HiviewDFX
} // namespace OHOS
