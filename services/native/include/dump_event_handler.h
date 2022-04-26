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
#ifndef HIDUMPER_SERVICES_DUMP_EVENT_HANDLER_H
#define HIDUMPER_SERVICES_DUMP_EVENT_HANDLER_H
#include <event_handler.h>
#include <refbase.h>
namespace OHOS {
namespace HiviewDFX {
class DumpManagerService;
class DumpEventHandler : public AppExecFwk::EventHandler {
public:
    DumpEventHandler(const std::shared_ptr<AppExecFwk::EventRunner>& runner,
        const wptr<DumpManagerService>& service);
    ~DumpEventHandler() = default;
    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event) override;
public:
    static const int MSG_GET_CPU_INFO_ID;
    static const int GET_CPU_INFO_DELAY_TIME;
private:
    wptr<DumpManagerService> service_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIDUMPER_SERVICES_DUMP_EVENT_HANDLER_H
