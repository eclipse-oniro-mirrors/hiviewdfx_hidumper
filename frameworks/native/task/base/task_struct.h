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
#include <unordered_set>

namespace OHOS {
namespace HiviewDFX {

enum class TaskId : uint32_t {
    DUMP_DEVICE_INFO = 0,
    DUMP_SYSTEM_CLUSTER_INFO = 1,
    DUMP_WAKEUP_SOURCES_INFO = 2,
    DUMP_KERNEL_MODULE_INFO = 3,
    DUMP_SLAB_INFO = 4,
    DUMP_ZONE_INFO = 5,
    DUMP_VMSTAT_INFO = 6,
    DUMP_VMALLOC_INFO = 7,
    DUMP_CPU_FREQ_INFO = 8,
    DUMP_DISK_INFO = 9,
    DUMP_LSOF_INFO = 10,
    DUMP_IOTOP_INFO = 11,
    DUMP_MOUNTS_INFO = 12,
    DUMP_STORAGE_IO_INFO = 13,
    DUMP_SYSTEM_ABILITY_LIST = 14,
    ROOT_TASK_START = 10000, // ROOT_TASK is a special task, it will be executed last.
    WRITE_SYSTEM_CLUSTER_INFO = 10001,
    WRITE_SYSTEM_BASE_INFO = 10002,
    WRITE_KERNEL_MODULE_INFO = 10003,
    WRITE_KERNEL_MEM_INFO = 10004,
    WRITE_CPU_FREQ_INFO = 10005,
    WRITE_ALL_PROCESS_MEM_INFO = 10006,
    WRITE_STORAGE_INFO = 10007,
    WRITE_STORAGE_IO_INFO = 10008,
    WRITE_SYSTEM_ABILITY_INFO = 10009,
    WRITE_SYSTEM_ABILITY_LIST = 10010,
};

using TaskCreator = std::function<std::unique_ptr<Task>()>;

struct RegTaskInfo {
    TaskCreator creator;
    std::unordered_set<DataId> dataDependency;
    std::unordered_set<TaskId> taskDependency;
    std::string taskName;
    uint32_t failureCount = 0;
    bool mandatory = false;
};

using TaskCollection = std::unordered_map<TaskId, RegTaskInfo>;
}
}
#endif
