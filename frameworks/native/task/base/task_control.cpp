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
#include "task/base/task_register.h"
#include "task/base/task_enable_config.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr uint32_t MAX_FAILURE_COUNT = 10;
constexpr size_t MAX_CACHE_SIZE = 10;
void GetDepTaskNames(const RegTaskInfo& taskInfo, const TaskCollection& allRegTasks,
                     std::vector<std::string>& depTaskNames)
{
#ifdef HIDUMPER_DFX
    for (const auto& depTaskId : taskInfo.taskDependency) {
        auto it = allRegTasks.find(depTaskId);
        if (it == allRegTasks.end()) {
            continue;
        }
        depTaskNames.emplace_back(it->second.taskName);
    }
#endif
}

void FillStatistcsDependence(const TaskCollection& regTask, std::vector<TaskStatistcs>& stat)
{
#ifdef HIDUMPER_DFX
    for (auto& node : stat) {
        for (const auto& regPair : regTask) {
            if (node.taskName == regPair.second.taskName) {
                GetDepTaskNames(regPair.second, regTask, node.dependTaskNames);
            }
        }
    }
#endif
}

void UpdateTaskStatistics(TaskStatistcs& stat, const RegTaskInfo& taskInfo,
                          const std::chrono::steady_clock::time_point& startTime,
                          const std::chrono::steady_clock::time_point& endTime)
{
#ifdef HIDUMPER_DFX
    stat.startTime = std::chrono::duration_cast<std::chrono::milliseconds>(startTime.time_since_epoch()).count();
    stat.duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    stat.taskName = taskInfo.taskName;
#endif
}
}

TaskControl::TaskControl() = default;
TaskControl::~TaskControl() = default;

void TaskControl::UpdateTaskFailureCount(TaskId taskId, bool reset)
{
    auto& container = TaskRegister::GetContainer();
    auto it = container.find(taskId);
    if (it != container.end()) {
        if (reset) {
            it->second.failureCount = 0;
            DUMPER_HILOGD(MODULE_COMMON, "Task %{public}s executed successfully, failure count reset to 0",
                          it->second.taskName.c_str());
        } else {
            it->second.failureCount++;
            DUMPER_HILOGW(MODULE_COMMON, "Task %{public}s failed, failure count: %{public}u",
                          it->second.taskName.c_str(), it->second.failureCount);
        }
    }
}

DumpStatus TaskControl::HandleTaskRetry(TaskId taskId, const RegTaskInfo& taskInfo, DataInventory& dataInventory,
                                        const DumpContext& dumpContext)
{
    DUMPER_HILOGW(MODULE_COMMON, "Task %{public}s failed first time, retrying...", taskInfo.taskName.c_str());
    
    auto task = taskInfo.creator();
    if (task == nullptr) {
        UpdateTaskFailureCount(taskId, false);
        DUMPER_HILOGE(MODULE_COMMON, "Task %{public}s creator failed during retry", taskInfo.taskName.c_str());
        return DUMP_FAIL;
    }
    
    auto dumpStatus = task->Run(dataInventory, dumpContext);
    if (dumpStatus != DUMP_OK) {
        UpdateTaskFailureCount(taskId, false);
        DUMPER_HILOGE(MODULE_COMMON, "Task %{public}s failed after retry", taskInfo.taskName.c_str());
    } else {
        UpdateTaskFailureCount(taskId, true);
        DUMPER_HILOGI(MODULE_COMMON, "Task %{public}s succeeded after retry", taskInfo.taskName.c_str());
    }
    
    return dumpStatus;
}

std::unique_ptr<Task> TaskControl::CreateTask(const RegTaskInfo& taskInfo)
{
    if (taskInfo.creator == nullptr) {
        DUMPER_HILOGE(MODULE_COMMON, "Task creator is null for task: %{public}s", taskInfo.taskName.c_str());
        return nullptr;
    }

    auto task = taskInfo.creator();
    if (task == nullptr) {
        DUMPER_HILOGE(MODULE_COMMON, "Failed to create task: %{public}s", taskInfo.taskName.c_str());
        return nullptr;
    }
    return task;
}

DumpStatus TaskControl::ExecuteSingleTask(TaskId taskId, const RegTaskInfo& taskInfo, DataInventory& dataInventory,
                                          const DumpContext& dumpContext)
{
    auto task = CreateTask(taskInfo);
    if (task == nullptr) {
        return DUMP_FAIL;
    }
    
    auto dumpStatus = task->Run(dataInventory, dumpContext);
    if (dumpStatus != DUMP_OK) {
        return HandleTaskRetry(taskId, taskInfo, dataInventory, dumpContext);
    }
    UpdateTaskFailureCount(taskId, true);
    return dumpStatus;
}

bool TaskControl::IsTaskExcessivelyFailed(TaskId taskId)
{
    auto& container = TaskRegister::GetContainer();
    auto regIt = container.find(taskId);
    if (regIt == container.end()) {
        DUMPER_HILOGE(MODULE_COMMON, "Task %{public}d is not registered", taskId);
        return false;
    }
    if (regIt->second.failureCount >= MAX_FAILURE_COUNT) {
        DUMPER_HILOGW(MODULE_COMMON, "Task %{public}s is disabled due to excessive failures.",
                      regIt->second.taskName.c_str());
        return true;
    }
    return false;
}

void TaskControl::UpdateTaskDependencies(TaskCollection& tasks, const TaskCollection& runnableTasks)
{
    for (auto& task : tasks) {
        auto& taskDependency = task.second.taskDependency;
        for (const auto& prepareTask : runnableTasks) {
            taskDependency.erase(prepareTask.first);
        }
    }
}

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
    if (!verifyTasks.empty()) {
        for (const auto& task : verifyTasks) {
            DUMPER_HILOGE(MODULE_COMMON, "Failed to verify taskTopo, taskId: %{public}d, taskName: %{public}s",
                          task.first, task.second.taskName.c_str());
        }
        return false;
    }
    return true;
}

DumpStatus TaskControl::ExecuteTask(DataInventory& dataInventory, const std::vector<TaskId>& taskIds,
                                    const DumpContext& dumpContext)
{
    TaskCollection taskTopo;
    for (size_t i = 0; i < taskIds.size(); i++) {
        if (taskIds[i] <= TaskId::ROOT_TASK_START) {
            DUMPER_HILOGE(MODULE_COMMON, "Taskid is not root task: %{public}d", taskIds[i]);
            return DUMP_FAIL;
        }
        BuildTaskTopo(taskIds[i], taskTopo);
        if (i > 0 && taskTopo.find(taskIds[i]) != taskTopo.end()) {
            taskTopo[taskIds[i]].taskDependency.insert(taskIds[i - 1]);
        }
    }
    if (!VerifyTaskTopo(taskTopo)) {
        DUMPER_HILOGE(MODULE_COMMON, "Failed to verify taskTopo");
        return DUMP_FAIL;
    }
    DumpStatus ret = ExecuteTaskInner(dataInventory, taskTopo, dumpContext);
    if (ret != DUMP_OK) {
        return ret;
    }
    return DUMP_OK;
}

void TaskControl::BuildTaskTopo(TaskId rootTaskId, TaskCollection& taskTopo)
{
    TaskCollection cachedTopo;
    if (GetCachedTaskTopo(rootTaskId, cachedTopo)) {
        DUMPER_HILOGD(MODULE_COMMON, "Using cached task topo for rootTaskId %{public}d", rootTaskId);
        for (const auto& task : cachedTopo) {
            taskTopo.emplace(task.first, task.second);
        }
        return;
    }
    DUMPER_HILOGD(MODULE_COMMON, "Building new task topo for rootTaskId %{public}d", rootTaskId);
    
    TaskCollection independentTopo;
    std::queue<TaskId> taskQueue;
    std::set<TaskId> visited;
    taskQueue.push(rootTaskId);
    visited.insert(rootTaskId);

    const auto& container = TaskRegister::GetContainer();
    auto& config = TaskEnableConfig::GetInstance();
    
    while (!taskQueue.empty()) {
        TaskId taskId = taskQueue.front();
        taskQueue.pop();

        auto it = container.find(taskId);
        if (it == container.end()) {
            continue;
        }
        if (!config.IsTaskEnabled(taskId)) {
            DUMPER_HILOGW(MODULE_COMMON, "Task ID %{public}d is disabled in config, skipping", taskId);
            continue;
        }
        
        taskTopo.emplace(taskId, it->second);
        independentTopo.emplace(taskId, it->second);
        
        const auto& depTasks = it->second.taskDependency;
        for (TaskId depTaskId : depTasks) {
            if (visited.count(depTaskId) == 0) {
                taskQueue.push(depTaskId);
                visited.insert(depTaskId);
            }
        }
    }
    CacheTaskTopo(rootTaskId, independentTopo);
    DUMPER_HILOGD(MODULE_COMMON,
                  "Built and cached independent task topo for rootTaskId %{public}d with %{public}zu tasks",
                  rootTaskId, independentTopo.size());
}

DumpStatus TaskControl::ExecuteTaskInner(DataInventory& dataInventory, TaskCollection& tasks,
                                         const DumpContext& dumpContext)
{
    std::vector<LevelStat> allLevelStats;
    auto taskCopy = tasks;
    auto runnableTasks = SelectRunnableTasks(tasks);
    while (!runnableTasks.empty()) {
        std::vector<TaskStatistcs> stat(runnableTasks.size());
        SubmitRunnableTasks(runnableTasks, dataInventory, dumpContext, stat);
        ffrt::wait();
        FillStatistcsDependence(taskCopy, stat);
        auto ret = GetTaskResult(std::move(stat), allLevelStats);
        if (!ret) {
            RecordTaskStat(dumpContext.GetOutputFd(), allLevelStats);
            return DUMP_FAIL;
        }
        ReleaseNoUsedData(dataInventory, tasks);
        runnableTasks = SelectRunnableTasks(tasks);
    }
    RecordTaskStat(dumpContext.GetOutputFd(), allLevelStats);
    return DUMP_OK;
}

bool TaskControl::GetTaskResult(std::vector<TaskStatistcs>&& taskStats, std::vector<LevelStat>& allLevelStats)
{
    auto executeResult = std::all_of(taskStats.begin(), taskStats.end(), [](const TaskStatistcs& node) {
        if (node.dumpStatus != DUMP_OK && node.mandatory) {
            DUMPER_HILOGE(MODULE_COMMON, "Failed to dump task: %{public}s", node.taskName.c_str());
            return false;
        }
        return true;
    });
    allLevelStats.emplace_back(LevelStat{executeResult, std::move(taskStats)});
    return executeResult;
}

void TaskControl::SubmitRunnableTasks(TaskCollection& tasks, DataInventory& dataInventory,
                                      const DumpContext& dumpContext, std::vector<TaskStatistcs>& stat)
{
    size_t concurrentIndex = 0;
    for (auto& taskInfo : tasks) {
        ffrt::submit([&taskInfo, &dataInventory, dumpContext, concurrentIndex, &stat, this]() {
            auto startTime = std::chrono::steady_clock::now();
            auto dumpStatus = ExecuteSingleTask(taskInfo.first, taskInfo.second, dataInventory, dumpContext);
            
            stat[concurrentIndex].dumpStatus = dumpStatus;
            stat[concurrentIndex].mandatory = taskInfo.second.mandatory;
            auto endTime = std::chrono::steady_clock::now();
            UpdateTaskStatistics(stat[concurrentIndex], taskInfo.second, startTime, endTime);
        });
        ++concurrentIndex;
    }
}

void TaskControl::ReleaseNoUsedData(DataInventory& dataInventory, const TaskCollection& tasks)
{
    std::set<DataId> usingData;
    for (auto& task : tasks) {
        usingData.insert(task.second.dataDependency.begin(), task.second.dataDependency.end());
    }
    dataInventory.RemoveRestData(usingData);
}

TaskCollection TaskControl::SelectRunnableTasks(TaskCollection& tasks)
{
    TaskCollection runnableTasks;
    auto it = tasks.begin();
    while (it != tasks.end()) {
        if (IsTaskExcessivelyFailed(it->first)) {
            it = tasks.erase(it);
            continue;
        }
        
        if (it->second.taskDependency.empty()) {
            runnableTasks.emplace(it->first, it->second);
            it = tasks.erase(it);
            continue;
        }
        ++it;
    }
    UpdateTaskDependencies(tasks, runnableTasks);
    return runnableTasks;
}

void TaskControl::RecordTaskStat(int fd, std::vector<LevelStat>& allLevelStats)
{
#ifdef HIDUMPER_DFX
    auto levelCount = allLevelStats.size();
    std::stringstream ss;
    ss << "\nTask levels: " << levelCount << std::endl;
    std::string statLine = ss.str();
    write(fd, statLine.c_str(), statLine.size());
    for (size_t i = 0; i < levelCount; i++) {
        auto& levelStat = allLevelStats[i];
        ss.str("");
        ss << "-----------------------------------------------------------------------------------" << std::endl;
        ss << "Level[" << i << "]: " << (levelStat.executeResult ? "Success" : "Failed")
        << ", Task number: " << levelStat.taskStats.size() << std::endl;
        for (auto& taskStat : levelStat.taskStats) {
            ss << "\tTask[" << taskStat.taskName << "]: " << (taskStat.dumpStatus == DUMP_OK ? "Success" : "Failed")
            << ", Mandatory: " << taskStat.mandatory;
            ss << "\t\tStart time: " << taskStat.startTime << ", Duration: " << taskStat.duration << std::endl;
            ss << "\t\tDependence: ";
            for (const auto& depTask : taskStat.dependTaskNames) {
                ss << depTask << " ";
            }
            ss << std::endl;
        }
        statLine = ss.str();
        write(fd, statLine.c_str(), statLine.size());
    }
#endif
}

bool TaskControl::GetCachedTaskTopo(TaskId rootTaskId, TaskCollection& taskTopo)
{
    std::lock_guard<std::mutex> lock(cacheMutex_);
    
    auto it = taskTopoCache_.find(rootTaskId);
    if (it != taskTopoCache_.end()) {
        taskTopo = *(it->second.taskTopo);
        MoveToFront(rootTaskId);
        DUMPER_HILOGD(MODULE_COMMON, "Cache hit for rootTaskId %{public}d", rootTaskId);
        return true;
    }
    
    DUMPER_HILOGD(MODULE_COMMON, "Cache miss for rootTaskId %{public}d", rootTaskId);
    return false;
}

void TaskControl::CacheTaskTopo(TaskId rootTaskId, const TaskCollection& taskTopo)
{
    CachedTaskTopo cachedItem;
    cachedItem.taskTopo = std::make_unique<TaskCollection>(taskTopo);
    
    {
        std::lock_guard<std::mutex> lock(cacheMutex_);
        if (taskTopoCache_.size() >= MAX_CACHE_SIZE) {
            EvictOldCache();
        }
        taskTopoCache_[rootTaskId] = std::move(cachedItem);
        lruList_.push_front(rootTaskId);
        lruMap_[rootTaskId] = lruList_.begin();
    }
    
    DUMPER_HILOGD(MODULE_COMMON, "Cached task topo for rootTaskId %{public}d, cache size: %{public}zu",
                  rootTaskId, taskTopoCache_.size());
}

void TaskControl::EvictOldCache()
{
    if (lruList_.empty()) {
        return;
    }
    TaskId oldestTaskId = lruList_.back();
    lruList_.pop_back();
    lruMap_.erase(oldestTaskId);
    taskTopoCache_.erase(oldestTaskId);
    
    DUMPER_HILOGD(MODULE_COMMON, "Evicted cache for rootTaskId %{public}d (LRU)", oldestTaskId);
}

void TaskControl::MoveToFront(TaskId rootTaskId)
{
    auto it = lruMap_.find(rootTaskId);
    if (it != lruMap_.end()) {
        lruList_.erase(it->second);
        lruList_.push_front(rootTaskId);
        it->second = lruList_.begin();
    }
}

} // namespace HiviewDFX
} // namespace OHOS
