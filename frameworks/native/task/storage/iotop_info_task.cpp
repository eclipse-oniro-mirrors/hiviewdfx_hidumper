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
#include "task/storage/iotop_info_task.h"

#include "data_inventory.h"
#include "hilog_wrapper.h"
#include "task/base/task_register.h"

namespace OHOS {
namespace HiviewDFX {

namespace {
const char ASCII_CR = '\r';
const char ASCII_LF = '\n';
const char ASCII_ESC = '\033';
const char ASCII_OB = '[';
const char ASCII_UA = 'A';
const char ASCII_LA = 'a';
const char ASCII_UZ = 'Z';
const char ASCII_LZ = 'z';
}

DumpStatus IoTopInfoTask::TaskEntry(DataInventory& dataInventory,
                                    const std::shared_ptr<DumpContext>& dumpContext)
{
    dataInventory.LoadAndInjectWithFilter("iotop -n 1 -m 100", IOTOP_INFO, false, [&](std::string &line) -> void {
        FilterControlChar(line);
    });
    return DUMP_OK;
}

void IoTopInfoTask::FilterControlChar(std::string &str)
    {
        std::string newStr;
        bool skip = false;
        const size_t sum = str.size();
        for (size_t pos = 0; pos < sum; pos++) {
            char &c = str.at(pos);
            if ((!skip) && (c == ASCII_ESC) && ((pos + 1) < sum)) {
                char &next_c = str.at(pos + 1);
                skip = (next_c == ASCII_OB);
            }
            if (skip && (((c >= ASCII_UA) && (c <= ASCII_UZ)) || ((c >= ASCII_LA) && (c <= ASCII_LZ)))) {
                skip = false;
                continue;
            }
            if (skip || (c == ASCII_CR) || (c == ASCII_LF)) {
                continue;
            }
            newStr.append(1, c);
        }
        str = newStr;
    }

REGISTER_TASK(DUMP_IOTOP_INFO, IoTopInfoTask, false);
} // namespace HiviewDFX
} // namespace OHOS