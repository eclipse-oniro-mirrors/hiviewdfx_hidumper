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

#ifndef HIVIEWDFX_HIDUMPER_TASK_STRUCT_H
#define HIVIEWDFX_HIDUMPER_TASK_STRUCT_H

#include "task/base/task.h"

namespace OHOS {
namespace HiviewDFX {

enum TaskId : uint32_t {
    DUMP_DEVICE_INFO = 0,
    DUMP_SYSTEM_CLUSTER_INFO,
    DUMP_WAKEUP_SOURCES_INFO,
    DUMP_KERNEL_MODULE_INFO,
    DUMP_SLAB_INFO,
    DUMP_ZONE_INFO,
    DUMP_VMSTAT_INFO,
    DUMP_VMALLOC_INFO,
    DUMP_CPU_FREQ_INFO,
    ROOT_TASK_START = 1000, // ROOT_TASK is a special task, it will be executed last.
    WRITE_DEVICE_INFO,
    WRITE_SYSTEM_CLUSTER_INFO,
    WRITE_SYSTEM_BASE_INFO,
    WRITE_KERNEL_MODULE_INFO,
    WRITE_KERNEL_MEM_INFO,
    WRITE_CPU_FREQ_INFO,
    WRITE_ALL_PROCESS_MEM_INFO,
};

using TaskCreator = std::function<std::unique_ptr<Task>()>;

struct RegTaskInfo {
    TaskCreator creator;
    std::vector<DataId> dataDependency;
    std::vector<TaskId> taskDependency;
    std::string taskName; // for DFX
    bool mandatory; // 是否为关键任务，关键任务失败后整个dump失败
};

using TaskCollection = std::unordered_map<TaskId, RegTaskInfo>;
}
}
#endif
