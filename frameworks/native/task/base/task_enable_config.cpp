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

#include "task/base/task_enable_config.h"
#include "hilog_wrapper.h"
#include "file_ex.h"
#include "string_ex.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <mutex>

namespace OHOS {
namespace HiviewDFX {
TaskEnableConfig::TaskEnableConfig() = default;
TaskEnableConfig::~TaskEnableConfig() = default;

DumpStatus TaskEnableConfig::LoadFromConfig(const std::string& configPath)
{
    if (configLoaded_) {
        DUMPER_HILOGD(MODULE_COMMON, "Task enable config already loaded");
        return DumpStatus::DUMP_OK;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (configLoaded_) {
        DUMPER_HILOGD(MODULE_COMMON, "Task enable config already loaded (double-check)");
        return DumpStatus::DUMP_OK;
    }
    
    DUMPER_HILOGD(MODULE_COMMON, "Loading task enable config from: %{public}s", configPath.c_str());
    DumpStatus status = ParseConfigFile(configPath);
    if (status == DumpStatus::DUMP_OK) {
        configLoaded_ = true;
        DUMPER_HILOGD(MODULE_COMMON, "Task enable config loaded successfully");
    } else {
        DUMPER_HILOGE(MODULE_COMMON, "Failed to load task enable config");
    }
    return status;
}

DumpStatus TaskEnableConfig::ParseConfigFile(const std::string& configPath)
{
    std::ifstream file(configPath);
    if (!file.is_open()) {
        DUMPER_HILOGE(MODULE_COMMON, "Failed to open config file: %{public}s", configPath.c_str());
        return DumpStatus::DUMP_FAIL;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    file.close();
    
    return ParseJsonContent(content);
}

DumpStatus TaskEnableConfig::ParseJsonContent(const std::string& content)
{
    auto root = std::unique_ptr<cJSON, decltype(&cJSON_Delete)>(cJSON_Parse(content.c_str()), cJSON_Delete);
    if (root == nullptr) {
        DUMPER_HILOGE(MODULE_COMMON, "Failed to parse JSON content");
        return DumpStatus::DUMP_FAIL;
    }
    
    cJSON* taskConfigs = cJSON_GetObjectItem(root.get(), "task_configs");
    if (taskConfigs == nullptr || !cJSON_IsObject(taskConfigs)) {
        DUMPER_HILOGE(MODULE_COMMON, "Missing or invalid 'task_configs' field");
        return DumpStatus::DUMP_FAIL;
    }
    
    int taskCount = cJSON_GetArraySize(taskConfigs);
    DUMPER_HILOGD(MODULE_COMMON, "Found %{public}d task configs", taskCount);
    
    cJSON* taskItem = taskConfigs->child;
    while (taskItem != nullptr) {
        if (cJSON_IsObject(taskItem)) {
            std::string taskName = taskItem->string;
            cJSON* taskIdItem = cJSON_GetObjectItem(taskItem, "task_id");
            if (taskIdItem == nullptr || !cJSON_IsNumber(taskIdItem)) {
                DUMPER_HILOGW(MODULE_COMMON, "Missing or invalid 'task_id' for task: %{public}s", taskName.c_str());
                taskItem = taskItem->next;
                continue;
            }
            
            TaskId taskId = static_cast<TaskId>(taskIdItem->valueint);
            cJSON* enabledItem = cJSON_GetObjectItem(taskItem, "enabled");
            bool enabled = true;
            if (enabledItem != nullptr && cJSON_IsBool(enabledItem)) {
                enabled = cJSON_IsTrue(enabledItem);
            }
            
            taskEnableMap_[taskId] = enabled;
            DUMPER_HILOGD(MODULE_COMMON, "Task %{public}s (ID: %{public}d) enabled: %{public}s",
                          taskName.c_str(), taskId, enabled ? "true" : "false");
        }
        taskItem = taskItem->next;
    }
    
    DUMPER_HILOGD(MODULE_COMMON, "Parsed %{public}zu task enable configs", taskEnableMap_.size());
    return DumpStatus::DUMP_OK;
}

bool TaskEnableConfig::IsTaskEnabled(TaskId taskId) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!configLoaded_) {
        DUMPER_HILOGW(MODULE_COMMON, "Config not loaded yet, defaulting to enabled for task ID: %{public}d", taskId);
        return true;
    }
    auto it = taskEnableMap_.find(taskId);
    if (it != taskEnableMap_.end()) {
        return it->second;
    }
    DUMPER_HILOGW(MODULE_COMMON, "Task ID %{public}d not found in config, defaulting to enabled", taskId);
    return true;
}

} // namespace HiviewDFX
} // namespace OHOS