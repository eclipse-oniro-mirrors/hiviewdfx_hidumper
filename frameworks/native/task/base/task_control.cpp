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
#include "task/base/task_control.h"

#include <algorithm>
#include <iostream>
#include <chrono>
#include <iterator>
#include <sstream>
#include <locale>
#include <set>
#include <queue>
#include "ffrt.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
void GetDepTaskNames(const RegTaskInfo& taskInfo, const TaskCollection& allRegTasks,
                     std::vector<std::string>& depTaskNames)
{
    for (const auto& depTaskId : taskInfo.taskDependency) {
        auto it = allRegTasks.find(depTaskId);
        if (it == allRegTasks.end()) {
            continue;
        }
        depTaskNames.emplace_back(it->second.taskName);
    }
}

void FillStatistcsDependence(const TaskCollection& regTask, std::vector<TaskStatistcs>& stat)
{
    for (auto& node : stat) {
        for (const auto& regPair : regTask) {
            if (node.taskName == regPair.second.taskName) {
                GetDepTaskNames(regPair.second, regTask, node.dependTaskNames);
            }
        }
    }
}
}

TaskControl::TaskControl() = default;
TaskControl::~TaskControl() = default;

bool TaskControl::VerifyTaskTopo(const TaskCollection& tasks)
{
    if (tasks.empty()) {
        DUMPER_HILOGE(MODULE_COMMON, "Failed to verify taskTopo, tasks is empty");
        return false;
    }
    auto verifyTasks = tasks;
    auto runnableTasks = SelectRunnableTasks(verifyTasks);
    while (!runnableTasks.empty()) {
        runnableTasks = SelectRunnableTasks(verifyTasks);
    }
    return verifyTasks.empty();
}

DumpStatus TaskControl::ExcuteTask(DataInventory& dataInventory, const std::vector<TaskId>& taskIds,
                                   const std::shared_ptr<DumpContext>& dumpContext)
{
    TaskCollection taskTopo;
    for (size_t i = 0; i < taskIds.size(); i++) {
        if (taskIds[i] <= ROOT_TASK_START) {
            DUMPER_HILOGE(MODULE_COMMON, "Taskid is not root task: %{public}d", taskIds[i]);
            return DUMP_FAIL;
        }
        BuildTaskTopo(taskIds[i], taskTopo);
        if (i > 0 && taskTopo.find(taskIds[i]) != taskTopo.end()) {
            taskTopo[taskIds[i]].taskDependency.push_back(taskIds[i - 1]);
        }
    }
    if (!VerifyTaskTopo(taskTopo)) {
        DUMPER_HILOGE(MODULE_COMMON, "Failed to verify taskTopo");
        return DUMP_FAIL;
    }
    DumpStatus ret = ExcuteTaskInner(dataInventory, taskTopo, dumpContext);
    if (ret != DUMP_OK) {
        return ret;
    }
    return DUMP_OK;
}

void TaskControl::BuildTaskTopo(TaskId rootTaskId, TaskCollection& taskTopo)
{
    std::queue<TaskId> taskQueue;
    std::set<TaskId> visited;
    taskQueue.push(rootTaskId);
    visited.insert(rootTaskId);

    const auto& container = TaskRegister::GetContainer();
    while (!taskQueue.empty()) {
        TaskId taskId = taskQueue.front();
        taskQueue.pop();

        auto it = container.find(taskId);
        if (it == container.end()) {
            continue;
        }
        taskTopo.emplace(taskId, it->second);

        const auto& depTasks = it->second.taskDependency;
        for (TaskId depTaskId : depTasks) {
            if (visited.count(depTaskId) == 0) {
                taskQueue.push(depTaskId);
                visited.insert(depTaskId);
            }
        }
    }
}

DumpStatus TaskControl::ExcuteTaskInner(DataInventory& dataInventory, TaskCollection& tasks,
                                        const std::shared_ptr<DumpContext>& dumpContext)
{
    auto taskCopy = tasks;
    auto runnableTasks = SelectRunnableTasks(tasks);
    while (!runnableTasks.empty()) {
        std::vector<TaskStatistcs> stat(runnableTasks.size());
        SubmitRunnableTasks(runnableTasks, dataInventory, dumpContext, stat);
        ffrt::wait();
        // for dfx
        FillStatistcsDependence(taskCopy, stat);
        auto ret = std::all_of(stat.begin(), stat.end(), [](const TaskStatistcs& node) {
            if (node.dumpStatus != DUMP_OK && node.mandatory) {
                DUMPER_HILOGE(MODULE_COMMON, "Failed to dump task: %{public}s", node.taskName.c_str());
                return false;
            }
            return true;
        });
        if (!ret) {
            return DUMP_FAIL;
        }
        ReleaseNoUsedData(dataInventory, tasks);
        runnableTasks = SelectRunnableTasks(tasks);
    }
    return DUMP_OK;
}

void TaskControl::SubmitRunnableTasks(TaskCollection& tasks, DataInventory& dataInventory,
                                      const std::shared_ptr<DumpContext>& dumpContext,
                                      std::vector<TaskStatistcs>& stat)
{
    size_t concurrentIndex = 0;
    for (auto& taskInfo : tasks) {
        ffrt::submit([&taskInfo, &dataInventory, dumpContext, concurrentIndex, &stat]() {
            auto startTime = std::chrono::steady_clock::now();
            if (taskInfo.second.creator == nullptr) {
                return;
            }
            auto task = taskInfo.second.creator();
            if (task != nullptr) {
                stat[concurrentIndex].dumpStatus = task->Run(dataInventory, dumpContext);
                stat[concurrentIndex].mandatory = taskInfo.second.mandatory;
            }
            auto endTime = std::chrono::steady_clock::now();
            stat[concurrentIndex].startTime =
                 std::chrono::duration_cast<std::chrono::milliseconds>(startTime.time_since_epoch()).count();
            stat[concurrentIndex].duration =
                 std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
            stat[concurrentIndex].taskName = taskInfo.second.taskName;
        });
        ++concurrentIndex;
    }
}

void TaskControl::ReleaseNoUsedData(DataInventory& dataInventory, const TaskCollection& tasks)
{
    std::set<DataId> usingData;
    for (auto& task : tasks) {
        for (auto& data : task.second.dataDependency) {
            usingData.insert(data);
        }
    }
    dataInventory.RemoveRestData(usingData);
}

TaskCollection TaskControl::SelectRunnableTasks(TaskCollection& tasks)
{
    TaskCollection runnableTasks;
    for (auto it = tasks.begin(); it != tasks.end();) {
        if (it->second.taskDependency.empty()) {
            runnableTasks.emplace(it->first, it->second);
            it = tasks.erase(it);
            continue;
        }
        ++it;
    }

    for (auto& task : tasks) {
        auto& taskDependency = task.second.taskDependency;
        for (const auto& prepareTask : runnableTasks) {
            taskDependency.erase(std::remove(taskDependency.begin(), taskDependency.end(), prepareTask.first),
                                 taskDependency.end());
        }
    }
    return runnableTasks;
}
}
}
