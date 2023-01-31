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
#ifndef HIDUMPER_SERVICES_DUMP_MANAGER_SERVICE_H
#define HIDUMPER_SERVICES_DUMP_MANAGER_SERVICE_H
#include <map>
#include <vector>
#include <system_ability.h>
#include "delayed_sp_singleton.h"
#include "dump_common_utils.h"
#include "dump_broker_stub.h"
#include "dump_event_handler.h"
namespace OHOS {
namespace HiviewDFX {
class RawParam;
#ifdef DUMP_TEST_MODE // for mock test
using DumpManagerServiceTestMainFunc = std::function<void(int argc, char *argv[],
    const std::shared_ptr<RawParam>& reqCtl)>;
#endif // for mock test
class DumpManagerService final : public SystemAbility, public DumpBrokerStub {
    DECLARE_SYSTEM_ABILITY(DumpManagerService)
public:
    DumpManagerService();
    ~DumpManagerService();
public:
    virtual void OnStart() override;
    virtual void OnStop() override;
public:
    // Used for dump request
    int32_t Request(std::vector<std::u16string> &args, int outfd) override;
public:
    int32_t Dump(int32_t fd, const std::vector<std::u16string>& args) override;
    bool IsServiceStarted() const
    {
        return started_;
    }
    std::shared_ptr<DumpEventHandler> GetHandler() const;
#ifdef DUMP_TEST_MODE // for mock test
    void SetTestMainFunc(DumpManagerServiceTestMainFunc testMainFunc);
#endif // for mock test
private:
    friend DelayedSpSingleton<DumpManagerService>;
private:
    bool Init();
    std::shared_ptr<RawParam> AddRequestRawParam(std::vector<std::u16string> &args, int outfd);
    void EraseRequestRawParam(const std::shared_ptr<RawParam> rawParam);
    void CancelAllRequest();
    int GetRequestSum();
    uint32_t GetRequestId();
    int32_t StartRequest(const std::shared_ptr<RawParam> rawParam);
    void RequestMain(const std::shared_ptr<RawParam> rawParam);
private:
    std::mutex mutex_;
    std::shared_ptr<AppExecFwk::EventRunner> eventRunner_;
    std::shared_ptr<DumpEventHandler> handler_;
    bool started_ {false};
    bool blockRequest_ {false};
    uint32_t requestIndex_ {0};
    std::map<uint32_t, std::shared_ptr<RawParam>> requestRawParamMap_;
#ifdef DUMP_TEST_MODE // for mock test
    DumpManagerServiceTestMainFunc testMainFunc_ {nullptr};
#endif // for mock test
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIDUMPER_SERVICES_DUMP_MANAGER_SERVICE_H
