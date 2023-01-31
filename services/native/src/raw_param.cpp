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
#include <thread>
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
} // namespace

RawParam::RawParam(int calllingUid, int calllingPid, uint32_t reqId, std::vector<std::u16string> &args, int outfd)
    : uid_(calllingUid), pid_(calllingPid), canceled_(false), finished_(false), reqId_(reqId), outfd_(outfd)
{
    DUMPER_HILOGD(MODULE_SERVICE, "create|pid=%{public}d, reqId=%{public}u", pid_, reqId_);
    Init(args);
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
    DUMPER_HILOGD(MODULE_SERVICE, "debug|argc=%{public}zu", args.size());
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
    DUMPER_HILOGD(MODULE_SERVICE, "leave|");
}

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

void RawParam::SetFolder(const std::string &folder)
{
    folder_ = folder;
}

std::string RawParam::GetFolder()
{
    return folder_;
}

void RawParam::UpdateProgress(uint32_t total, uint32_t current)
{
    if ((!progressEnabled_) || (outfd_ < 0) || (total < 1) || (total < current)) {
        return;
    }
    UpdateProgress((uint64_t(FINISH) * current) / total);
}

void RawParam::UpdateProgress(uint64_t progress)
{
    if ((!progressEnabled_) || (outfd_ < 0) || (progress > FINISH)) {
        return;
    }
    progress_ = std::max(progress, progress_);
    progressTick_ = (progressTick_ + 1) % sizeof(PROGRESS_TICK);
    if (SHOW_PROGRESS_BAR) {
        char barbuf[PROGRESS_LENGTH + 1] = {0};
        for (size_t i = 0; ((i < progress_) && (i < PROGRESS_LENGTH)); i++) {
            barbuf[i] = PROGRESS_STYLE;
        }
        dprintf(outfd_, "\033[?25l\r[%-100s],%2" PRIu64 "%%,[%c]\033[?25h",
            barbuf, progress_, PROGRESS_TICK[progressTick_]);
    } else {
        dprintf(outfd_, "\033[?25l\r%2" PRIu64 "%%,[%c]\033[?25h", progress_, PROGRESS_TICK[progressTick_]);
    }
    if (progress_ == FINISH) {
        dprintf(outfd_, "%s\n", path_.c_str());
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
    DUMPER_HILOGD(MODULE_SERVICE, "enter|reqId=%{public}d", reqId_);
    deathed_ = true;
    DUMPER_HILOGD(MODULE_SERVICE, "leave|reqId=%{public}d", reqId_);
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
