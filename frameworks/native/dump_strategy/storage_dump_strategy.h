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

#ifndef STORAGE_DUMP_STRATEGY_H
#define STORAGE_DUMP_STRATEGY_H

#include "dump_strategy.h"
#include <memory>
#include "base/task_control.h"
#include "data_inventory.h"

namespace OHOS {
namespace HiviewDFX {
class StorageDumpStrategy : public DumpStrategy {
public:
    StorageDumpStrategy() = default;

    DumpStatus CreateRootTaskList(const std::shared_ptr<DumpContext>& context,
                                  std::vector<TaskId>& rootTasks) override
    {
        auto opt = context->GetDumperOpts();
        if (opt->isDumpStorage) {
            if (opt->storagePid > 0) {
                rootTasks.push_back(WRITE_STORAGE_IO_INFO);
            } else {
                rootTasks.push_back(WRITE_STORAGE_INFO);
            }
        }
        return DUMP_OK;
    }
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // STORAGE_DUMP_STRATEGY_H
