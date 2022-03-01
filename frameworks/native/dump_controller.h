/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#ifndef DUMP_CONTROLLER_H
#define DUMP_CONTROLLER_H
#include "common.h"
namespace OHOS {
namespace HiviewDFX {
const int ARG_MAX_COUNT = 64;
const int SINGLE_ARG_MAXLEN = 256;
const uint32_t SIZE_KB = 1024;
const uint32_t LIMIT_SIZE = 32; // 32KB
const uint32_t DEFAULT_LIMITSIZE = LIMIT_SIZE * SIZE_KB;
const uint32_t DEFAULT_TIMEOUT = 30; // 30 seconds
} // namespace HiviewDFX
} // namespace OHOS
#endif
