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

#ifndef HIVIEWDFX_HIDUMPER_TASK_H
#define HIVIEWDFX_HIDUMPER_TASK_H

#include <cstdint>
#include <functional>
#include <vector>
#include <memory>
#include "data_inventory.h"
#include "common.h"
#include "common/dumper_parameter.h"

namespace OHOS {
namespace HiviewDFX {

class Task {
public:
    Task() = default;
    virtual ~Task() = default;

    DumpStatus Run(DataInventory& dataInventory, const std::shared_ptr<DumperParameter>& parameter)
    {
        PreDump(dataInventory, parameter);
        auto ret = TaskEntry(dataInventory, parameter);
        PostDump(dataInventory, parameter);
        return ret;
    }

private:
    virtual DumpStatus TaskEntry(DataInventory& dataInventory, const std::shared_ptr<DumperParameter>& parameter) = 0;
    virtual void PreDump(DataInventory& dataInventory, const std::shared_ptr<DumperParameter>& parameter) const {};
    virtual void PostDump(DataInventory& dataInventory, const std::shared_ptr<DumperParameter>& parameter) const {};
};

}
}
#endif
