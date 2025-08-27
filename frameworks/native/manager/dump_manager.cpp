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

#include "dump_manager.h"

#include "base/task_control.h"
#include "cmd_parse.h"
#include "dump_strategy_factory.h"
#include "hilog_wrapper.h"
#include "task/base/task_enable_config.h"

namespace OHOS {
namespace HiviewDFX {

DumpManager::DumpManager() = default;
DumpManager::~DumpManager() = default;

DumpStatus DumpManager::StartDump(int argc, char *argv[], DumpContext& context)
{
    if (CmdParse::GetInstance().Parse(argc, argv, context) != DumpStatus::DUMP_OK) {
        DUMPER_HILOGE(MODULE_COMMON, "Command parse failed");
        return DumpStatus::DUMP_FAIL;
    }

    if (TaskEnableConfig::GetInstance().LoadFromConfig() != DumpStatus::DUMP_OK) {
        DUMPER_HILOGE(MODULE_COMMON, "Config load failed");
        return DumpStatus::DUMP_FAIL;
    }

    std::vector<TaskId> rootTasks;
    if (DumpStrategyFactory::GetInstance().CollectRootTasks(context, rootTasks) != DumpStatus::DUMP_OK) {
        DUMPER_HILOGE(MODULE_COMMON, "Strategy execution failed");
        return DumpStatus::DUMP_FAIL;
    }
    
    DataInventory dataInventory;
    if (TaskControl::GetInstance().ExecuteTask(dataInventory, rootTasks, context) != DumpStatus::DUMP_OK) {
        DUMPER_HILOGE(MODULE_COMMON, "Task execution failed");
        return DumpStatus::DUMP_FAIL;
    }
    return DumpStatus::DUMP_OK;
}
} // namespace HiviewDFX
} // namespace OHOS
