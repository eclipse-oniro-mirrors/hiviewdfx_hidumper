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
#include <atomic>
#include <map>
#include <vector>
#include <system_ability.h>
#include "event_runner.h"
#include "event_handler.h"
#include "delayed_sp_singleton.h"
#include "dump_common_utils.h"
#include "dump_broker_stub.h"
#include "system_ability_ondemand_reason.h"
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
    // Used for scan pid list over limit
    int32_t ScanPidOverLimit(std::string requestType, int32_t limitSize, std::vector<int32_t> &pidList) override;
    // Used for count fd nums
    int32_t CountFdNums(int32_t pid, uint32_t &fdNums, std::string &detailFdInfo, std::string &topLeakedType) override;
public:
    int32_t OnIdle(const SystemAbilityOnDemandReason& idleReason) override;
    int32_t Dump(int32_t fd, const std::vector<std::u16string>& args) override;
    bool IsServiceStarted() const
    {
        return started_;
    }
    void DelayUnloadTask() override;
#ifdef DUMP_TEST_MODE // for mock test
    void SetTestMainFunc(DumpManagerServiceTestMainFunc testMainFunc);
#endif // for mock test
private:
    friend DumpDelayedSpSingleton<DumpManagerService>;
private:
    bool Init();
    std::shared_ptr<RawParam> AddRequestRawParam(std::vector<std::u16string> &args, int outfd);
    void EraseRequestRawParam(const std::shared_ptr<RawParam> rawParam);
    void CancelAllRequest();
    int GetRequestSum();
    void GetIdleRequest();
    uint32_t GetRequestId();
    int32_t StartRequest(const std::shared_ptr<RawParam> rawParam);
    void RequestMain(const std::shared_ptr<RawParam> rawParam);
    bool HasDumpPermission() const;
    uint32_t GetFileDescriptorNums(int32_t pid, std::string requestType) const;
    std::string GetFdLink(const std::string &linkPath) const;
    std::vector<std::string> GetFdLinks(int pid);
    std::string MaybeKnownType(const std::string &link);
    std::unordered_map<std::string, int> CountPaths(const std::vector<std::string>& links);
    std::vector<std::pair<std::string, int>> TopN(const std::unordered_map<std::string, int>& counter, size_t n);
    std::string GetSummary(const std::vector<std::pair<std::string, int>> &topLinks,
                           const std::vector<std::pair<std::string, int>> &topTypes);
    std::string GetTopFdInfo(const std::vector<std::pair<std::string, int>> &topLinks);
    std::string GetTopDirInfo(const std::vector<std::pair<std::string, int>> &topTypes,
                              const std::map<std::string, std::unordered_map<std::string, int>> &typePaths);
    void SetCpuSchedAffinity();
    void HandleRequestError(std::vector<std::u16string> &args, int outfd,
        const int32_t& errorCode, const std::string& errorMsg);
private:
    std::mutex mutex_;
    std::shared_ptr<AppExecFwk::EventRunner> eventRunner_;
    std::shared_ptr<AppExecFwk::EventHandler> handler_;
    std::atomic<bool> started_ = false;
    std::atomic<bool> blockRequest_ = false;
    uint32_t requestIndex_ {0};
    std::map<uint32_t, std::shared_ptr<RawParam>> requestRawParamMap_;
#ifdef DUMP_TEST_MODE // for mock test
    DumpManagerServiceTestMainFunc testMainFunc_ {nullptr};
#endif // for mock test
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIDUMPER_SERVICES_DUMP_MANAGER_SERVICE_H
