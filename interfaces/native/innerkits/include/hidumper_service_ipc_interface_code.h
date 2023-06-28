/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#ifndef HIDUMPER_SERVICE_IPC_INTERFACE_CODE_H
#define HIDUMPER_SERVICE_IPC_INTERFACE_CODE_H

/* SAID:1212 */
namespace OHOS {
namespace HiviewDFX {
enum class HidumperServiceInterfaceCode{
    DUMP_REQUEST_FILEFD,
    SCAN_PID_OVER_LIMIT,
    COUNT_FD_NUMS
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIDUMPER_SERVICE_IPC_INTERFACE_CODE_H
