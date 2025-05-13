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

#ifndef SYSTEM_INFO_DUMP_STRATEGY_H
#define SYSTEM_INFO_DUMP_STRATEGY_H

#include "dump_strategy.h"
#include <memory>
#include "base/task_control.h"
#include "base/task_register.h"
#include "data_inventory.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace HiviewDFX {
class SystemInfoDumpStrategy : public DumpStrategy {
public:
    SystemInfoDumpStrategy() = default;

DumpStatus CreateRootTaskList(const std::shared_ptr<DumpContext>& context,
                              std::vector<TaskId>& rootTasks) override
{
    auto opt = context->GetDumperOpts();
    if (opt->isDumpList) {
        rootTasks.push_back(WRITE_SYSTEM_CLUSTER_INFO);
        return DUMP_OK;
    }

    const auto& systemArgs = opt->systemArgs;
    auto addTaskIfNeeded = [&rootTasks, &systemArgs](const std::string& arg, const std::vector<TaskId>& taskIds) {
        if (systemArgs.empty() || systemArgs.find(arg) != systemArgs.end()) {
            rootTasks.insert(rootTasks.end(), taskIds.begin(), taskIds.end());
        }
    };
    addTaskIfNeeded("base", { WRITE_SYSTEM_BASE_INFO });
    addTaskIfNeeded("system", { WRITE_KERNEL_MODULE_INFO, WRITE_KERNEL_MEM_INFO,
                                WRITE_CPU_FREQ_INFO, WRITE_ALL_PROCESS_MEM_INFO });
    return DUMP_OK;
}
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // SYSTEM_INFO_DUMP_STRATEGY_H
