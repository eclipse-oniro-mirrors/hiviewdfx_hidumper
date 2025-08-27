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

#ifndef HIVIEWDFX_HIDUMPER_TASK_CONTROL_H
#define HIVIEWDFX_HIDUMPER_TASK_CONTROL_H

#include <functional>
#include <memory>
#include <vector>
#include <map>
#include <unordered_map>
#include <list>

#include "data_inventory.h"
#include "dump_context.h"
#include "task/base/task_register.h"
#include "singleton.h"

namespace OHOS {
namespace HiviewDFX {

struct TaskStatistcs {
    std::string taskName;
    std::vector<std::string> dependTaskNames;
    DumpStatus dumpStatus;
    uint64_t startTime = 0;
    uint64_t duration = 0;
    bool mandatory = false;
};

struct LevelStat {
    bool executeResult = false;
    std::vector<TaskStatistcs> taskStats;
};

struct CachedTaskTopo {
    std::unique_ptr<TaskCollection> taskTopo;
};

class TaskControl : public DelayedRefSingleton<TaskControl> {
    DECLARE_DELAYED_REF_SINGLETON(TaskControl)
public:
    DISALLOW_COPY_AND_MOVE(TaskControl);

    DumpStatus ExecuteTask(DataInventory& dataInventory,
                           const std::vector<TaskId>& taskIds, const DumpContext& dumpContext);
private:
    DumpStatus ExecuteTaskInner(DataInventory& dataInventory,
                                TaskCollection& tasks, const DumpContext& dumpContext);
    bool VerifyTaskTopo(const TaskCollection& taskTopo);
    void BuildTaskTopo(TaskId rootTaskId, TaskCollection& taskTopo);
    TaskCollection SelectRunnableTasks(TaskCollection& tasks);
    void ReleaseNoUsedData(DataInventory& dataInventory, const TaskCollection& tasks);
    void SubmitRunnableTasks(TaskCollection& tasks, DataInventory& dataInventory,
                             const DumpContext& dumpContext, std::vector<TaskStatistcs>& stat);
    void UpdateTaskFailureCount(TaskId taskId, bool reset = false);
    DumpStatus ExecuteSingleTask(TaskId taskId, const RegTaskInfo& taskInfo, DataInventory& dataInventory,
                                 const DumpContext& dumpContext);
    bool IsTaskExcessivelyFailed(TaskId taskId);
    void UpdateTaskDependencies(TaskCollection& tasks, const TaskCollection& runnableTasks);
    DumpStatus HandleTaskRetry(TaskId taskId, const RegTaskInfo& taskInfo, DataInventory& dataInventory,
                               const DumpContext& dumpContext);
    std::unique_ptr<Task> CreateTask(const RegTaskInfo& taskInfo);
    bool GetTaskResult(std::vector<TaskStatistcs>&& taskStats, std::vector<LevelStat>& allLevelStats);
    void RecordTaskStat(int fd, std::vector<LevelStat>& allLevelStats);
    bool GetCachedTaskTopo(TaskId rootTaskId, TaskCollection& taskTopo);
    void CacheTaskTopo(TaskId rootTaskId, const TaskCollection& taskTopo);
    void EvictOldCache();
    void MoveToFront(TaskId rootTaskId);
    
    std::list<TaskId> lruList_;
    std::unordered_map<TaskId, std::list<TaskId>::iterator> lruMap_;
    std::unordered_map<TaskId, CachedTaskTopo> taskTopoCache_;
    mutable std::mutex cacheMutex_;
};

} // namespace HiviewDFX
} // namespace OHOS
#endif // HIVIEWDFX_HIDUMPER_TASK_CONTROL_H
