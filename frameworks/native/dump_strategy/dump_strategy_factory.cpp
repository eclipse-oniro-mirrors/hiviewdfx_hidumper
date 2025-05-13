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

DumpStatus DumpStrategyFactory::CreateRootTaskList(const std::shared_ptr<DumpContext>& context,
                                                   std::vector<TaskId>& rootTasks)
{
    if (strategies_.empty()) {
        DUMPER_HILOGE(MODULE_COMMON, "No strategies available for execution");
        return DumpStatus::DUMP_FAIL;
    }
    DumpStatus ret = DumpStatus::DUMP_OK;
    for (auto& strategy : strategies_) {
        ret = strategy->CreateRootTaskList(context, rootTasks);
    }
    return ret;
}

void DumpStrategyFactory::CreateStrategies(const std::shared_ptr<DumperOptions>& opts)
{
    HandleSystemInfoDumpStrategy(opts);
    HandleCpuDumpStrategy(opts);
    HandleMemDumpStrategy(opts);
    HandleStorageDumpStrategy(opts);
    HandleNetDumpStrategy(opts);
    HandleSystemAbilityDumpStrategy(opts);
    HandleProcessInfoDumpStrategy(opts);
    HandleLogDumpStrategy(opts);
    HandleStackDumpStrategy(opts);
    HandleIpcStatDumpStrategy(opts);
    return;
}

void DumpStrategyFactory::HandleSystemInfoDumpStrategy(const std::shared_ptr<DumperOptions>& opts)
{
    if (opts->isDumpSystem) {
        DUMPER_HILOGI(MODULE_COMMON, "HandleSystemInfoDumpStrategy");
        strategies_.emplace_back(new SystemInfoDumpStrategy());
    }
    return;
}

void DumpStrategyFactory::HandleCpuDumpStrategy(const std::shared_ptr<DumperOptions>& opts)
{
    if (opts->isDumpCpuFreq || opts->isDumpCpuUsage) {
        DUMPER_HILOGI(MODULE_COMMON, "HandleDumpCpuDumpStrategy");
        strategies_.emplace_back(new CpuDumpStrategy());
    }
    return;
}

void DumpStrategyFactory::HandleMemDumpStrategy(const std::shared_ptr<DumperOptions>& opts)
{
    if (opts->isDumpMem) {
        DUMPER_HILOGI(MODULE_COMMON, "HandleDumpMemDumpStrategy");
        strategies_.emplace_back(new MemDumpStrategy());
    }
    return;
}

void DumpStrategyFactory::HandleStorageDumpStrategy(const std::shared_ptr<DumperOptions>& opts)
{
    if (opts->isDumpStorage) {
        DUMPER_HILOGI(MODULE_COMMON, "HandleDumpStorageDumpStrategy");
        strategies_.emplace_back(new StorageDumpStrategy());
    }
}
void DumpStrategyFactory::HandleNetDumpStrategy(const std::shared_ptr<DumperOptions>& opts)
{
    if (opts->isDumpNet) {
        DUMPER_HILOGI(MODULE_COMMON, "HandleDumpNetDumpStrategy");
        strategies_.emplace_back(new NetDumpStrategy());
    }
}

void DumpStrategyFactory::HandleSystemAbilityDumpStrategy(const std::shared_ptr<DumperOptions>& opts)
{
    if (opts->isDumpSystemAbility) {
        DUMPER_HILOGI(MODULE_COMMON, "HandleDumpSystemAbilityDumpStrategy");
        strategies_.emplace_back(new SystemAbilityDumpStrategy());
    }
}

void DumpStrategyFactory::HandleProcessInfoDumpStrategy(const std::shared_ptr<DumperOptions>& opts)
{
    if (opts->isDumpProcesses) {
        DUMPER_HILOGI(MODULE_COMMON, "HandleDumpProcessInfoDumpStrategy");
        strategies_.emplace_back(new ProcessInfoDumpStrategy());
    }
}

void DumpStrategyFactory::HandleLogDumpStrategy(const std::shared_ptr<DumperOptions>& opts)
{
    if (opts->isDumpLog) {
        DUMPER_HILOGI(MODULE_COMMON, "HandleDumpLogDumpStrategy");
        strategies_.emplace_back(new LogDumpStrategy());
    }
}

void DumpStrategyFactory::HandleStackDumpStrategy(const std::shared_ptr<DumperOptions>& opts)
{
    if (opts->isAppendix) {
        DUMPER_HILOGI(MODULE_COMMON, "HandleDumpStackDumpStrategy");
        strategies_.emplace_back(new StackDumpStrategy());
    }
}

void DumpStrategyFactory::HandleIpcStatDumpStrategy(const std::shared_ptr<DumperOptions>& opts)
{
    if (opts->isDumpIpcStat) {
        DUMPER_HILOGI(MODULE_COMMON, "HandleDumpIpcStatDumpStrategy");
        strategies_.emplace_back(new IpcStatDumpStrategy());
    }
}
}
}