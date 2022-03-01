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
#include "dump_event_handler.h"
#include "hilog_wrapper.h"
#include "dump_manager_service.h"
namespace OHOS {
namespace HiviewDFX {
DumpEventHandler::DumpEventHandler(const std::shared_ptr<AppExecFwk::EventRunner>& runner,
    const wptr<DumpManagerService>& service)
    : AppExecFwk::EventHandler(runner), service_(service)
{
}

void DumpEventHandler::ProcessEvent([[maybe_unused]] const AppExecFwk::InnerEvent::Pointer& event)
{
    auto dmsptr = service_.promote();
    if (dmsptr == nullptr) {
        return;
    }
    auto eventId = event->GetInnerEventId();
    DUMPER_HILOGD(MODULE_SERVICE, "debug|eventId=%{public}d", eventId);
}
} // namespace HiviewDFX
} // namespace OHOS
