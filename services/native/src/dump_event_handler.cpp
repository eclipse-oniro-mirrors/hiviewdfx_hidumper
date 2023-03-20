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
#include "util/dump_cpu_info_util.h"
#include "dump_manager_service.h"
#include "hilog_wrapper.h"
namespace OHOS {
namespace HiviewDFX {
const int DumpEventHandler::MSG_GET_CPU_INFO_ID = 1;
const int DumpEventHandler::GET_CPU_INFO_DELAY_TIME = 5 * 1000;

DumpEventHandler::DumpEventHandler(const std::shared_ptr<AppExecFwk::EventRunner>& runner,
    const wptr<DumpManagerService>& service)
    : AppExecFwk::EventHandler(runner), service_(service)
{
}

void DumpEventHandler::ProcessEvent([[maybe_unused]] const AppExecFwk::InnerEvent::Pointer& event)
{
    auto dmsptr = service_.promote();
    if (dmsptr == nullptr) {
        DUMPER_HILOGD(MODULE_SERVICE, "service is nullptr!");
        return;
    }

    if (event == nullptr) {
        DUMPER_HILOGD(MODULE_SERVICE, "event is nullptr!");
        return;
    }
    auto eventId = event->GetInnerEventId();
    DUMPER_HILOGD(MODULE_SERVICE, "debug|eventId=%{public}d", eventId);
    switch (eventId) {
        case MSG_GET_CPU_INFO_ID: {
            DUMPER_HILOGD(MODULE_SERVICE, "MSG_GET_CPU_INFO_ID!");
            DumpCpuInfoUtil::GetInstance().UpdateCpuInfo();
            break;
        }
        default:
            break;
    }
}
} // namespace HiviewDFX
} // namespace OHOS
