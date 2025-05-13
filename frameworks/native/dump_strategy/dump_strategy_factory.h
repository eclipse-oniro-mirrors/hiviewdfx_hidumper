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

#include "dump_strategy.h"

namespace OHOS {
namespace HiviewDFX {
class DumpStrategyFactory : public DumpStrategy {
public:
    explicit DumpStrategyFactory(const std::shared_ptr<DumperOptions>& opts)
    {
        CreateStrategies(opts);
    }
    DumpStatus CreateRootTaskList(const std::shared_ptr<DumpContext>& context,
                                  std::vector<TaskId>& rootTasks) override;
private:
    void CreateStrategies(const std::shared_ptr<DumperOptions>& opts);
    void HandleSystemInfoDumpStrategy(const std::shared_ptr<DumperOptions>& opts);
    void HandleCpuDumpStrategy(const std::shared_ptr<DumperOptions>& opts);
    void HandleMemDumpStrategy(const std::shared_ptr<DumperOptions>& opts);
    void HandleStorageDumpStrategy(const std::shared_ptr<DumperOptions>& opts);
    void HandleNetDumpStrategy(const std::shared_ptr<DumperOptions>& opts);
    void HandleSystemAbilityDumpStrategy(const std::shared_ptr<DumperOptions>& opts);
    void HandleProcessInfoDumpStrategy(const std::shared_ptr<DumperOptions>& opts);
    void HandleLogDumpStrategy(const std::shared_ptr<DumperOptions>& opts);
    void HandleStackDumpStrategy(const std::shared_ptr<DumperOptions>& opts);
    void HandleIpcStatDumpStrategy(const std::shared_ptr<DumperOptions>& opts);

    std::vector<std::unique_ptr<DumpStrategy>> strategies_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // DUMP_STRATEGY_FACTORY_H