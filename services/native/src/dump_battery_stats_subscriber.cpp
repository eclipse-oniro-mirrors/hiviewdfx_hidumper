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
#ifdef HIDUMPER_BATTERY_ENABLE
#include "dump_battery_stats_subscriber.h"
#include "common_event_data.h"
#include "common_event_manager.h"
#include "common_event_subscriber.h"
#include "common_event_subscribe_info.h"
#include "common_event_support.h"
#include "battery_info.h"
#include "hilog_wrapper.h"
#include "dump_manager_cpu_service.h"

namespace OHOS {
namespace HiviewDFX {
static constexpr int8_t INVALID_VALUE = -1;

void DumpBatteryStatsSubscriber::OnReceiveEvent(const OHOS::EventFwk::CommonEventData &data)
{
    std::string action = data.GetWant().GetAction();
    DUMPER_HILOGI(MODULE_CPU_SERVICE, "dump battery stats on receive event action=%{public}s", action.c_str());
    if (action == OHOS::EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_CHANGED) {
        int32_t capacity = data.GetWant().GetIntParam(PowerMgr::BatteryInfo::COMMON_EVENT_KEY_CAPACITY, INVALID_VALUE);
        DUMPER_HILOGI(MODULE_CPU_SERVICE,
            "Received COMMON_EVENT_BATTERY_CHANGED event, capacity=%{public}d, last capacity=%{public}d",
            capacity, lastCapacity_);
        if (capacity != lastCapacity_) {
            lastCapacity_ = capacity;
            auto dumpManagerService = DumpDelayedSpSingleton<DumpManagerCpuService>::GetInstance();
            if (dumpManagerService == nullptr) {
                DUMPER_HILOGE(MODULE_CPU_SERVICE, "get dump manager service null");
                return;
            }
            dumpManagerService->SendImmediateEvent();
        }
    }
}
} // namespace HiviewDFX
} // namespace OHOS
#endif
