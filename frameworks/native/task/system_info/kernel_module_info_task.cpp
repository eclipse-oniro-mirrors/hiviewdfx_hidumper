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


#include "task/system_info/kernel_module_info_task.h"

#include "data_inventory.h"
#include "hilog_wrapper.h"
#include "task/base/task_register.h"
#include "writer_utils.h"

namespace OHOS {
namespace HiviewDFX {
DumpStatus KernelModuleInfoTask::TaskEntry(DataInventory& dataInventory, const DumpContext& dumpContext)
{
    dataInventory.InjectString("printenv", DataId::PRINTENV_INFO, false);
    dataInventory.InjectString("lsmod", DataId::LSMOD_INFO, false);
    dataInventory.InjectString("/proc/modules", DataId::PROC_MODULES_INFO, true);
    return DUMP_OK;
}

REGISTER_TASK(TaskId::DUMP_KERNEL_MODULE_INFO, KernelModuleInfoTask, false);
} // namespace HiviewDFX
} // namespace OHOS