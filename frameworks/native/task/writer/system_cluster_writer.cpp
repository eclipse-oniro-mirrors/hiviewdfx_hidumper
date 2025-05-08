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
#include "task/writer/system_cluster_writer.h"
#include <memory>
#include "data_inventory.h"
#include "writer_utils.h"
#include "task/base/task_register.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
const std::string SYSTEM_CLUSTER_TITLE = "System cluster list:";
}

DumpStatus SystemClusterWriter::TaskEntry(DataInventory& dataInventory, const std::shared_ptr<DumpContext>& dumpContext)
{
    WriteStringIntoFd(SYSTEM_CLUSTER_TITLE, dumpContext->GetOutputFd());
    auto data = dataInventory.GetPtr<std::vector<std::string>>(DataId::SYSTEM_CLUSTER_INFO);
    if (!data) {
        return DUMP_FAIL;
    }
    WriteStringIntoFd(*data, dumpContext->GetOutputFd());
    return DUMP_OK;
}

REGISTER_TASK(WRITE_SYSTEM_CLUSTER_INFO, SystemClusterWriter, true, DUMP_SYSTEM_CLUSTER_INFO);
REGISTER_DEPENDENT_DATA(WRITE_SYSTEM_CLUSTER_INFO, SYSTEM_CLUSTER_INFO);
} // namespace HiviewDFX
} // namespace OHOS
