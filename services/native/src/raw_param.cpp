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
#include "raw_param.h"
#include <cinttypes>
#include <securec.h>
#include <string_ex.h>
#include <unistd.h>
#include "hilog_wrapper.h"
#include "dump_manager_service.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
static const bool SHOW_PROGRESS_BAR = false;
static const int PROGRESS_LENGTH = 128;
static const char PROGRESS_STYLE = '=';
static const char PROGRESS_TICK[] = {'-', '\\', '|', '/'};
static const std::string TASK_ERASE_CALLBACK = "HiDumper_Service_EraseCallback";
} // namespace

RawParam::RawParam(int calllingUid, int calllingPid, uint32_t reqId, std::vector<std::u16string> &args, int outfd,
                   const sptr<IDumpCallbackBroker> &callback)
    : uid_(calllingUid), pid_(calllingPid), canceled_(false), finished_(false), reqId_(reqId), outfd_(outfd)
{
    DUMPER_HILOGD(MODULE_SERVICE, "create|pid=%{public}d, reqId=%{public}u", pid_, reqId_);
    Init(args);
    if (callback != nullptr) {
        auto object = callback->AsObject();
        if (object == nullptr) {
            return;
        }
        auto retIt = callbackSet_.insert(callback);
        if (retIt.second) {
            object->AddDeathRecipient(deathRecipient_);
        }
    }
}

RawParam::~RawParam()
{
DUMPER_HILOGD(MODULE_SERVICE, "release|pid=%{public}d, reqId=%{public}u", pid_, reqId_);
    Uninit();
}

bool RawParam::Init(std::vector<std::u16string> &args)
{
    DUMPER_HILOGD(MODULE_SERVICE, "enter|");
    if (deathRecipient_ == nullptr) {
        deathRecipient_ = sptr<IRemoteObject::DeathRecipient>(new ClientDeathRecipient(reqId_, canceled_));
    }
    DUMPER_HILOGD(MODULE_SERVICE, "debug|argc=%{public}d", args.size());
    for (size_t i = 0; i < args.size(); i++) {
        argValues_[i] = std::make_unique<ArgValue>();
        if (sprintf_s(argValues_[i]->value, SINGLE_ARG_MAXLEN, "%s", Str16ToStr8(args[i]).c_str()) < 0) {
            return false;
        }
        argHead_[i] = argValues_[i]->value;
    }
    DUMPER_HILOGD(MODULE_SERVICE, "leave|true");
    return true;
}

void RawParam::Uninit()
{
    DUMPER_HILOGD(MODULE_SERVICE, "enter|");
    CloseOutputFd();
    for (auto &callback : callbackSet_) {
        if (callback == nullptr) {
            continue;
        }
        auto object = callback->AsObject();
        if (object == nullptr) {
            continue;
        }
        object->RemoveDeathRecipient(deathRecipient_);
    }
    callbackSet_.clear();
    DUMPER_HILOGD(MODULE_SERVICE, "leave|");
}

#ifdef DUMP_TEST_MODE // for mock test
const sptr<IDumpCallbackBroker> RawParam::GetCallback()
{
    for (auto &callback : callbackSet_) {
        if (callback == nullptr) {
            continue;
        }
        return callback;
    }
    return nullptr;
}
#endif // for mock test

int &RawParam::GetOutputFd()
{
    return outfd_;
}

void RawParam::CloseOutputFd()
{
    DUMPER_HILOGD(MODULE_SERVICE, "enter|outfd=%{public}d", outfd_);
    if (outfd_ > -1) {
        DUMPER_HILOGD(MODULE_SERVICE, "debug|outfd=%{public}d", outfd_);
        close(outfd_);
    }
    outfd_ = -1;
    DUMPER_HILOGD(MODULE_SERVICE, "leave|outfd=%{public}d", outfd_);
}

int RawParam::GetArgc()
{
    int count = 0;
    for (int i = 0; i < ARG_MAX_COUNT; i++) {
        if (argHead_[i] == nullptr) {
            break;
        }
        count = i + 1;
    }
    return count;
}

char **RawParam::GetArgv()
{
    return argHead_;
}

int RawParam::GetUid()
{
    return uid_;
}

int RawParam::GetPid()
{
    return pid_;
}

uint32_t RawParam::GetRequestId()
{
    return reqId_;
}

bool RawParam::IsCanceled()
{
    return canceled_;
}

bool RawParam::IsFinished()
{
    return (canceled_ || finished_ || hasError_);
}

bool RawParam::HasError()
{
    return hasError_;
}

void RawParam::Cancel()
{
    canceled_ = true;
    DUMPER_HILOGD(MODULE_SERVICE, "debug|reqId=%{public}u", reqId_);
}

void RawParam::UpdateStatus(uint32_t status, bool force)
{
    if (force || (!IsFinished())) {
        switch (status) {
            case IDumpCallbackBroker::STATUS_DUMP_FINISHED: {
                finished_ = true;
                break;
            }
            case IDumpCallbackBroker::STATUS_DUMP_ERROR: {
                hasError_ = true;
                break;
            }
            default: {
                break;
            }
        }
        std::lock_guard lock(mutex_);
        for (auto &callback : callbackSet_) {
            callback->OnStatusChanged(status);
        }
    }
    if (IsFinished()) {
        CloseOutputFd();
    }
}

void RawParam::SetProgressEnabled(bool enable)
{
    progressEnabled_ = enable;
}

bool RawParam::IsProgressEnabled() const
{
    return progressEnabled_;
}

void RawParam::SetTitle(const std::string &path)
{
    path_ = path;
}

void RawParam::UpdateProgress(uint32_t total, uint32_t current)
{
    if ((!progressEnabled_) || (outfd_ < 0) || (total < 1) || (total < current)) {
        return;
    }
    progressCurrent_ = (current > progressCurrent_) ? current : progressCurrent_;
    uint64_t progress = (uint64_t(100) * progressCurrent_) / total;
    progressTick_ = (progressTick_ + 1) % sizeof(PROGRESS_TICK);
    if (SHOW_PROGRESS_BAR) {
        char barbuf[PROGRESS_LENGTH + 1] = {0};
        for (size_t i = 0; ((i < progress) && (i < PROGRESS_LENGTH)); i++) {
            barbuf[i] = PROGRESS_STYLE;
        }
        dprintf(
            outfd_, "\033[?25l\r[%-100s],%2" PRIu64 "%%,[%c]\033[?25h", barbuf, progress, PROGRESS_TICK[progressTick_]);
    } else {
        dprintf(outfd_, "\033[?25l\r%2" PRIu64 "%%,[%c]\033[?25h", progress, PROGRESS_TICK[progressTick_]);
    }
    if (progress == FINISH) {
        dprintf(outfd_, "%s", path_.c_str());
    }
}

RawParam::ClientDeathRecipient::ClientDeathRecipient(uint32_t reqId, bool &deathed) : reqId_(reqId), deathed_(deathed)
{}

void RawParam::ClientDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    if (remote == nullptr || remote.promote() == nullptr) {
        return;
    }
    auto dumpManagerService = DelayedSpSingleton<DumpManagerService>::GetInstance();
    if (dumpManagerService == nullptr) {
        return;
    }
    auto handler = dumpManagerService->GetHandler();
    if (handler == nullptr) {
        return;
    }
    DUMPER_HILOGD(MODULE_SERVICE, "enter|reqId=%{public}d", reqId_);
    deathed_ = true;
    sptr<IDumpCallbackBroker> callback = iface_cast<IDumpCallbackBroker>(remote.promote());
    std::function<void()> unRegFunc = std::bind(&DumpManagerService::EraseCallback, dumpManagerService, callback);
    handler->PostTask(unRegFunc, TASK_ERASE_CALLBACK);
    DUMPER_HILOGD(MODULE_SERVICE, "leave|reqId=%{public}d", reqId_);
}

void RawParam::EraseCallback(const sptr<IDumpCallbackBroker> &callback)
{
    if (callback == nullptr) {
        return;
    }
    auto object = callback->AsObject();
    if (object == nullptr) {
        return;
    }
    DUMPER_HILOGD(MODULE_SERVICE, "enter|reqId=%{public}d, canceled=%{public}d", reqId_, canceled_);
    std::lock_guard lock(mutex_);
    size_t eraseNum = callbackSet_.erase(callback);
    if (eraseNum != 0) {
        canceled_ = true;
        DUMPER_HILOGD(MODULE_SERVICE, "debug|reqId=%{public}d, eraseNum=%{public}d", reqId_, eraseNum);
        object->RemoveDeathRecipient(deathRecipient_);
    }
    DUMPER_HILOGD(MODULE_SERVICE, "leave|reqId=%{public}d, canceled=%{public}d", reqId_, canceled_);
}

void RawParam::Dump() const
{
    DUMPER_HILOGD(MODULE_SERVICE, "enter|");
    DUMPER_HILOGD(MODULE_SERVICE, "debug|uid=%{public}d, pid=%{public}d", uid_, pid_);
    DUMPER_HILOGD(MODULE_SERVICE, "debug|reqId=%{public}u", reqId_);
    DUMPER_HILOGD(MODULE_SERVICE, "leave|");
}
} // namespace HiviewDFX
} // namespace OHOS
