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
#ifndef HIVIEWDFX_HIDUMPER_STORAGE_IO_INFO_WRITER_H
#define HIVIEWDFX_HIDUMPER_STORAGE_IO_INFO_WRITER_H

#include "task/base/task.h"
#include "data_inventory.h"
#include "dump_context.h"

namespace OHOS {
namespace HiviewDFX {
class StorageIoInfoWriter : public Task {
public:
    StorageIoInfoWriter() = default;
    ~StorageIoInfoWriter() override = default;

private:
    DumpStatus TaskEntry(DataInventory& dataInventory, const std::shared_ptr<DumpContext>& dumpContext) override;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIVIEWDFX_HIDUMPER_STORAGE_IO_INFO_WRITER_H
