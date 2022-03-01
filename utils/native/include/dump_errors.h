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
#ifndef HIDUMPER_UTILS_DUMP_ERRORS_H
#define HIDUMPER_UTILS_DUMP_ERRORS_H
#include <errors.h>
namespace OHOS {
namespace HiviewDFX {
enum DumperModuleType {
    DUMPER_MODULE_TYPE_SERVICE = 0,
    DUMPER_MODULE_TYPE_KIT = 1
};
constexpr ErrCode DUMPER_SERVICE_ERR_OFFSET = ErrCodeOffset(SUBSYS_POWERMNG, DUMPER_MODULE_TYPE_SERVICE);
enum DumperError {
    ERROR_WRITE_PARCEL = DUMPER_SERVICE_ERR_OFFSET,
    ERROR_READ_PARCEL,
    ERROR_GET_SYSTEM_ABILITY_MANAGER,
    ERROR_GET_DUMPER_SERVICE,
    ERROR_ADD_DEATH_RECIPIENT
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIDUMPER_UTILS_DUMP_ERRORS_H