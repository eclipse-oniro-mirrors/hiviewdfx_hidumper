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
#ifndef HIDUMPER_UTILS_HILOG_WRAPPER_H
#define HIDUMPER_UTILS_HILOG_WRAPPER_H
#define CONFIG_HILOG
#ifdef CONFIG_HILOG
#include "hilog/log.h"
namespace OHOS {
namespace HiviewDFX {
#ifdef DUMPER_HILOGF
#undef DUMPER_HILOGF
#endif
#ifdef DUMPER_HILOGE
#undef DUMPER_HILOGE
#endif
#ifdef DUMPER_HILOGW
#undef DUMPER_HILOGW
#endif
#ifdef DUMPER_HILOGI
#undef DUMPER_HILOGI
#endif
#ifdef DUMPER_HILOGD
#undef DUMPER_HILOGD
#endif
// param of log interface, such as DUMPER_HILOGF.
enum DumperSubModule {
    MODULE_COMMON = 0,
    MODULE_ZIDL,
    MODULE_CLIENT,
    MODULE_SERVICE,
    MODULE_MAX,
};
// 0xD002900: subsystem:DumperMgr module:DumperManager, 8 bits reserved.
static constexpr unsigned int BASE_DUMPER_DOMAIN_ID = 0xD002900;
enum DumperMgrDomainId {
    DUMPER_COMMON_DOMAIN = BASE_DUMPER_DOMAIN_ID + MODULE_COMMON,
    DUMPER_ZIDL_DOMAIN,
    DUMPER_CLIENT_DOMAIN,
    DUMPER_SERVICE_DOMAIN,
};
static constexpr OHOS::HiviewDFX::HiLogLabel DUMPER_LABEL[MODULE_MAX] = {
    {LOG_CORE, DUMPER_COMMON_DOMAIN, "DumperCommon"},
    {LOG_CORE, DUMPER_ZIDL_DOMAIN, "DumperZIDL"},
    {LOG_CORE, DUMPER_CLIENT_DOMAIN, "DumperClient"},
    {LOG_CORE, DUMPER_SERVICE_DOMAIN, "DumperService"},
};
// In order to improve performance, do not check the module range.
// Besides, make sure module is less than DUMPER_MAX_DOMAIN.
#define DUMPER_HILOGF(module, fmt, ...) \
    (void)OHOS::HiviewDFX::HiLog::Fatal(DUMPER_LABEL[module], "%{public}s# " fmt, __FUNCTION__, ##__VA_ARGS__)
#define DUMPER_HILOGE(module, fmt, ...) \
    (void)OHOS::HiviewDFX::HiLog::Error(DUMPER_LABEL[module], "%{public}s# " fmt, __FUNCTION__, ##__VA_ARGS__)
#define DUMPER_HILOGW(module, fmt, ...) \
    (void)OHOS::HiviewDFX::HiLog::Warn(DUMPER_LABEL[module], "%{public}s# " fmt, __FUNCTION__, ##__VA_ARGS__)
#define DUMPER_HILOGI(module, fmt, ...) \
    (void)OHOS::HiviewDFX::HiLog::Info(DUMPER_LABEL[module], "%{public}s# " fmt, __FUNCTION__, ##__VA_ARGS__)
#define DUMPER_HILOGD(module, fmt, ...) \
    (void)OHOS::HiviewDFX::HiLog::Debug(DUMPER_LABEL[module], "%{public}s# " fmt, __FUNCTION__, ##__VA_ARGS__)
} // namespace HiviewDFX
} // namespace OHOS
#else
#define DUMPER_HILOGF(...)
#define DUMPER_HILOGE(...)
#define DUMPER_HILOGW(...)
#define DUMPER_HILOGI(...)
#define DUMPER_HILOGD(...)
#endif // CONFIG_HILOG
#endif // HIDUMPER_UTILS_HILOG_WRAPPER_H
