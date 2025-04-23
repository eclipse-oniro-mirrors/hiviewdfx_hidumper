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
#include "task/writer/device_info_writer.h"
#include <memory>
#include "data_id.h"
#include "writer_utils.h"
#include "task/base/task_register.h"
#include "task/base/task_id.h"

namespace OHOS {
namespace HiviewDFX {

DumpStatus DeviceInfoWriter::TaskEntry(DataInventory& dataInventory, const std::shared_ptr<DumperParameter>& parameter)
{
    auto data = dataInventory.GetPtr<std::vector<std::string>>(DataId::DEVICE_INFO);
    if (!data) {
        return DUMP_FAIL;
    }
    WriteStringIntoFd(*data, parameter);
    return DUMP_OK;
}

REGISTER_TASK(WRITE_DEVICE_INFO, DeviceInfoWriter, true, DUMP_DEVICE_INFO);
REGISTER_DEPENDENT_DATA(WRITE_DEVICE_INFO, DEVICE_INFO);

} // namespace HiviewDFX
} // namespace OHOS
