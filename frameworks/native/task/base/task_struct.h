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
#include "task/base/task_id.h"

namespace OHOS {
namespace HiviewDFX {

using TaskCreator = std::function<std::unique_ptr<Task>()>;

struct RegTaskInfo {
    TaskCreator creator;
    std::vector<DataId> dataDependency; // 当前task需要的数据类型
    std::vector<TaskId> taskDependency; // 当前task的前向依赖
    std::string taskName; // for DFX
    bool mandatory; // 是否为关键任务，关键任务失败后整个dump失败
};

using TaskCollection = std::unordered_map<TaskId, RegTaskInfo>;
}
}
#endif
