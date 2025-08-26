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

#include "dump_strategy_factory.h"
#include "hilog_wrapper.h"
#include "system_info_dump_strategy.h"
#include "cpu_dump_strategy.h"
#include "mem_dump_strategy.h"
#include "storage_dump_strategy.h"
#include "net_dump_strategy.h"
#include "system_ability_dump_strategy.h"
#include "process_info_dump_strategy.h"
#include "log_dump_strategy.h"
#include "stack_dump_strategy.h"
#include "ipc_stat_dump_strategy.h"

namespace OHOS {
namespace HiviewDFX {

std::vector<StrategyConfig> DumpStrategyFactory::strategyConfigs_ = {};

DumpStrategyFactory::DumpStrategyFactory() = default;
DumpStrategyFactory::~DumpStrategyFactory() = default;

DumpStatus DumpStrategyFactory::CollectRootTasks(const std::shared_ptr<DumpContext>& context,
                                                 std::vector<TaskId>& rootTasks)
{
    std::vector<std::unique_ptr<DumpStrategy>> strategies;
    auto opts = context->GetDumperOpts();
    if (opts == nullptr) {
        DUMPER_HILOGE(MODULE_COMMON, "Failed to get options from context");
        return DumpStatus::DUMP_FAIL;
    }
    
    for (const auto& config : strategyConfigs_) {
        if (config.condition(opts)) {
            auto strategy = config.creator();
            if (strategy) {
                strategies.emplace_back(std::move(strategy));
            }
        }
    }
    
    if (strategies.empty()) {
        DUMPER_HILOGE(MODULE_COMMON, "Strategy execution failed");
        return DumpStatus::DUMP_FAIL;
    }
    DumpStatus ret = DumpStatus::DUMP_OK;
    for (auto& strategy : strategies) {
        ret = strategy->CollectRootTasks(context, rootTasks);
        if (ret != DumpStatus::DUMP_OK) {
            DUMPER_HILOGE(MODULE_COMMON, "Strategy failed");
            break;
        }
    }
    return ret;
}
} // namespace HiviewDFX
} // namespace OHOS