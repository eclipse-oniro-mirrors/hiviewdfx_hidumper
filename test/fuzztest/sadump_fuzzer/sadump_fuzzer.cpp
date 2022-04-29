/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "sadump_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <string>
namespace OHOS {
bool SADumpFuzzTest(const uint8_t* data, size_t size)
{
    if (size == 0 || data == nullptr) {
        return true;
    }
    std::string cmd = "hidumper -s " + std::to_string(reinterpret_cast<int>(data));
    system(cmd.c_str());
    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::SADumpFuzzTest(data, size);
    return 0;
}
