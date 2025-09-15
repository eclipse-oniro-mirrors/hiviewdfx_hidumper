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
#include "data_inventory.h"
#include "hilog_wrapper.h"
#include "task/base/task_register.h"
#include "task/storage/lsof_info_task.h"

namespace OHOS {
namespace HiviewDFX {
DumpStatus LsofInfoTask::TaskEntry(DataInventory& dataInventory, const DumpContext& dumpContext)
{
    dataInventory.InjectString("lsof", DataId::LSOF_INFO, false);
    return DUMP_OK;
}
REGISTER_TASK(TaskId::DUMP_LSOF_INFO, LsofInfoTask, false);
} // namespace HiviewDFX
} // namespace OHOS