/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
#ifndef HIDUMPER_APP_STATE_OBSERVER_H
#define HIDUMPER_APP_STATE_OBSERVER_H
#include <functional>
#include <map>
#include <string>

#include "application_state_observer_stub.h"
#include "singleton.h"

namespace OHOS {
namespace HiviewDFX {
class DumpAppStateObserver : public Singleton<DumpAppStateObserver> {
public:
	DumpAppStateObserver();
    ~DumpAppStateObserver();
    bool SubscribeAppState();
    bool UnsubscribeAppState();
    void SendUpdateCpuInfoEvent();
private:
    OHOS::sptr<OHOS::AppExecFwk::IAppMgr> GetAppManagerInstance();

    class AppProcessState : public AppExecFwk::ApplicationStateObserverStub {
    public:
        explicit AppProcessState(DumpAppStateObserver &appStateObserver) {}
        ~AppProcessState() override = default;

        void OnProcessCreated(const AppExecFwk::ProcessData &processData) override;
        void OnProcessDied(const AppExecFwk::ProcessData &processData) override;
    };
    sptr<AppProcessState> appProcessState_{nullptr};
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIDUMPER_APP_STATE_OBSERVER_H
