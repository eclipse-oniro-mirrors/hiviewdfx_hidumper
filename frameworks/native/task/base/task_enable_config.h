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

#ifndef HIVIEWDFX_HIDUMPER_TASK_ENABLE_CONFIG_H
#define HIVIEWDFX_HIDUMPER_TASK_ENABLE_CONFIG_H

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>
#include "task/base/task_struct.h"
#include "common.h"
#include "cJSON.h"
#include "singleton.h"

namespace OHOS {
namespace HiviewDFX {

class TaskEnableConfig : public DelayedRefSingleton<TaskEnableConfig> {
    DECLARE_DELAYED_REF_SINGLETON(TaskEnableConfig)
public:
    DISALLOW_COPY_AND_MOVE(TaskEnableConfig);
    DumpStatus LoadFromConfig(const std::string& configPath = DEFAULT_CONFIG_PATH);
    bool IsTaskEnabled(TaskId taskId) const;
    bool IsConfigLoaded() const { return configLoaded_; }

private:
    DumpStatus ParseConfigFile(const std::string& configPath);
    DumpStatus ParseJsonContent(const std::string& content);
    
private:
    static constexpr const char* DEFAULT_CONFIG_PATH = "/system/etc/hidumper/task_enable_config.json";
    
    std::unordered_map<TaskId, bool> taskEnableMap_;
    bool configLoaded_ = false;
    mutable std::mutex mutex_;
};

} // namespace HiviewDFX
} // namespace OHOS
#endif // HIVIEWDFX_HIDUMPER_TASK_ENABLE_CONFIG_H