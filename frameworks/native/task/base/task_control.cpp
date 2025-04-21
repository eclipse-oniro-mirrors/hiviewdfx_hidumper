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

bool TaskControl::VerifyTasks(const TaskCollection& tasks)
{
    auto verifyTasks = tasks;
    auto runnableTasks = SelectRunnableTasks(verifyTasks);
    while (!runnableTasks.empty()) {
        runnableTasks = SelectRunnableTasks(verifyTasks);
    }
    return verifyTasks.empty();
}

DumpStatus TaskControl::ExcuteTask(DataInventory& dataInventory, TaskCollection& tasks,
                                   const std::shared_ptr<DumperParameter>& parameter)
{
    if (!VerifyTasks(tasks)) {
        return DUMP_FAIL;
    }
    auto taskCopy = tasks;
    auto runnableTasks = SelectRunnableTasks(tasks);
    while (!runnableTasks.empty()) {
        std::vector<TaskStatistcs> stat(runnableTasks.size());
        SubmitRunnableTasks(runnableTasks, dataInventory, parameter, stat);
        ffrt::wait();

        // for dfx
        FillStatistcsDependence(taskCopy, stat);
        auto ret = std::all_of(stat.begin(), stat.end(), [](const TaskStatistcs& node) {
            if (node.dumpStatus != DUMP_OK && node.mandatory) {
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
                                      const std::shared_ptr<DumperParameter>& parameter,
                                      std::vector<TaskStatistcs>& stat)
{
    size_t concurrentIndex = 0;
    for (auto& taskInfo : tasks) {
        ffrt::submit([&taskInfo, &dataInventory, parameter, concurrentIndex, &stat]() {
            auto startTime = std::chrono::steady_clock::now();
            if (taskInfo.second.creator == nullptr) {
                return;
            }
            auto task = taskInfo.second.creator();
            if (task != nullptr) {
                stat[concurrentIndex].dumpStatus = task->Run(dataInventory, parameter);
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
