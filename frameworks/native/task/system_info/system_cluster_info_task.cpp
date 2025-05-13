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


#include "task/system_info/system_cluster_info_task.h"

#include "data_inventory.h"
#include "hilog_wrapper.h"
#include "task/base/task_register.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
const std::vector<std::string> SYSTEM_CLUSTER = { "base", "system" };
}

DumpStatus SystemClusterInfoTask::TaskEntry(DataInventory& dataInventory,
                                            const std::shared_ptr<DumpContext>& dumpContext)
{
    std::string systemCluster = "";
    constexpr int spaceCount = 30;
    for (auto& cluster : SYSTEM_CLUSTER) {
        systemCluster += cluster;
        systemCluster.append(spaceCount, ' ');
    }
    std::vector<std::string> result = { systemCluster };
    dataInventory.Inject(DataId::SYSTEM_CLUSTER_INFO, std::make_shared<std::vector<std::string>>(result));
    return DUMP_OK;
}

REGISTER_TASK(DUMP_SYSTEM_CLUSTER_INFO, SystemClusterInfoTask, true);
} // namespace HiviewDFX
} // namespace OHOS