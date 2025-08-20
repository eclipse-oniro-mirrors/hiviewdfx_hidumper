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

#ifndef HIVIEWDFX_HIDUMPER_TASK_REGISTER_H
#define HIVIEWDFX_HIDUMPER_TASK_REGISTER_H

#include <cstdint>
#include <functional>
#include <memory>
#include <vector>
#include <map>
#include <unordered_map>
#include <string>
#include "task/base/task_struct.h"

namespace OHOS {
namespace HiviewDFX {
template <typename T>
std::unique_ptr<Task> TCreator()
{
    return std::make_unique<T>();
}

class TaskRegister {
public:
    TaskRegister(TaskId selfId, TaskCreator creator, bool mandatory,
                 std::string taskName, std::vector<TaskId>&& taskDependency);
    TaskRegister(TaskId selfId, std::vector<DataId>&& dataDependency);

    static TaskCollection CopyTaskInfo();
    static TaskCollection& GetContainer();
};

#define REGISTER_TASK(TaskId, TaskClass, mandatory, ...) \
static TaskRegister taskRegister(TaskId, TCreator<TaskClass>, mandatory, #TaskClass, {__VA_ARGS__})

#define REGISTER_DEPENDENT_DATA(TaskId, ...) \
static TaskRegister dataRegister(TaskId, {__VA_ARGS__})
}
}
#endif
