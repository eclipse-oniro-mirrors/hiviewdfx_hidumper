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

#include "common/dumper_parameter.h"
#include "task/base/task_register.h"

namespace OHOS {
namespace HiviewDFX {

struct TaskStatistcs {
    std::string taskName;
    std::vector<std::string> dependTaskNames;
    DumpStatus dumpStatus;
    uint64_t startTime;
    uint64_t duration;
    bool mandatory;
};

class TaskControl final {
public:
    TaskControl() = default;
    ~TaskControl() = default;

    DumpStatus ExcuteTask(DataInventory& dataInventory,
                          TaskCollection& tasks, const std::shared_ptr<DumperParameter>& parameter);

private:
    bool VerifyTasks(const TaskCollection& tasks);
    TaskCollection SelectRunnableTasks(TaskCollection& tasks);
    void ReleaseNoUsedData(DataInventory& dataInventory, const TaskCollection& tasks);
    void SubmitRunnableTasks(TaskCollection& tasks, DataInventory& dataInventory,
                             const std::shared_ptr<DumperParameter>& parameter, std::vector<TaskStatistcs>& stat);
};

}
}
#endif
