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

 
#ifndef DUMP_STRATEGY_FACTORY_H
#define DUMP_STRATEGY_FACTORY_H

#include <memory>
#include <vector>
#include <functional>

#include "singleton.h"
#include "dump_strategy.h"

namespace OHOS {
namespace HiviewDFX {

struct StrategyConfig {
    std::function<bool(const std::shared_ptr<DumperOptions>&)> condition;
    std::function<std::unique_ptr<DumpStrategy>()> creator;
};

class DumpStrategyFactory : public DelayedRefSingleton<DumpStrategyFactory> {
    DECLARE_DELAYED_REF_SINGLETON(DumpStrategyFactory)
    DISALLOW_COPY_AND_MOVE(DumpStrategyFactory);
public:
    DumpStatus CollectRootTasks(const DumpContext& context, std::vector<TaskId>& rootTasks);

private:
    void CreateStrategies(const std::shared_ptr<DumperOptions>& opts,
                          std::vector<std::unique_ptr<DumpStrategy>>& strategies);
    DumpStatus ExecuteStrategies(const std::vector<std::unique_ptr<DumpStrategy>>& strategies,
                                 const DumpContext& context, std::vector<TaskId>& rootTasks);

private:
    static std::vector<StrategyConfig> strategyConfigs_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // DUMP_STRATEGY_FACTORY_H