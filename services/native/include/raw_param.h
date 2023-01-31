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
#ifndef HIDUMPER_SERVICES_DUMP_RAW_PARAM_H
#define HIDUMPER_SERVICES_DUMP_RAW_PARAM_H
#include <set>
#include <string_ex.h>
#include <vector>
#include "dump_controller.h"
#include "idump_broker.h"
namespace OHOS {
namespace HiviewDFX {
class DumpManagerService;
class DumpManagerServiceClient;
class RawParam {
public:
    RawParam(int calllingUid, int calllingPid, uint32_t reqId, std::vector<std::u16string>& args, int outfd);
    ~RawParam();
    int GetArgc();
    char** GetArgv();
    int GetUid();
    int GetPid();
    uint32_t GetRequestId();
    bool IsCanceled();
    bool IsFinished();
    bool HasError();
public:
    void Cancel();
    void UpdateStatus(uint32_t status, bool force = false);
    void SetProgressEnabled(bool enable);
    bool IsProgressEnabled() const;
    void UpdateProgress(uint32_t total, uint32_t current);
    void UpdateProgress(uint64_t progress);
    int& GetOutputFd();
    void CloseOutputFd();
    bool Init(std::vector<std::u16string>& args);
    void Uninit();
    void SetTitle(const std::string &path);
    void SetFolder(const std::string &folder);
    std::string GetFolder();
private:
    void Dump() const;
    class ClientDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        ClientDeathRecipient(uint32_t reqId, bool& deathed);
        virtual ~ClientDeathRecipient() = default;
    public:
        virtual void OnRemoteDied(const wptr<IRemoteObject>& remote);
    private:
        uint32_t reqId_;
        bool& deathed_;
    };
private:
    struct ArgValue {
        char value[SINGLE_ARG_MAXLEN + 1] = {0};
    };
    std::mutex mutex_;
    int uid_ {-1};
    int pid_ {-1};
    bool canceled_ {false};
    bool finished_ {false};
    bool hasError_ {false};
    uint32_t reqId_ {0};
    char* argHead_[ARG_MAX_COUNT + 1] = {nullptr};
    std::unique_ptr<ArgValue> argValues_[ARG_MAX_COUNT + 1] = {nullptr};
    int outfd_ {-1};
    sptr<IRemoteObject::DeathRecipient> deathRecipient_;
    bool progressEnabled_ {false};
    uint32_t progressTick_ {0};
    uint64_t progress_ {0};
    std::string path_;
    std::string folder_;
    const uint64_t FINISH = 100;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIDUMPER_SERVICES_DUMP_RAW_PARAM_H
