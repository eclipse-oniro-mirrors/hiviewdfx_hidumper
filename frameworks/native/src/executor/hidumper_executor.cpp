/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#include "executor/hidumper_executor.h"
#include "datetime_ex.h"
namespace OHOS {
namespace HiviewDFX {
const std::string HidumperExecutor::TIME_OUT_STR = "time out";

HidumperExecutor::~HidumperExecutor()
{
}

void HidumperExecutor::Reset()
{
    rawParam_ = nullptr;
    ptrParent_ = nullptr;
    hasOnce_ = false;
    hasOnceTimeout_ = false;
    hitTickCount_ = 0;
    timeOutMillSec_ = 0;
}

DumpStatus HidumperExecutor::DoPreExecute(const std::shared_ptr<DumperParameter>& parameter, StringMatrix dumpDatas)
{
    if (parameter != nullptr) {
        rawParam_ = parameter->getClientCallback();
        // to mill-second.
        timeOutMillSec_ = parameter->GetOpts().timeout_ * SEC_TO_MILLISEC;
    }

    if ((!hasOnce_) && (ptrDumpCfg_ != nullptr) && (!ptrDumpCfg_->IsOutput())) {
        // run here, record the hit tick count.
        hitTickCount_ = static_cast<uint64_t>(GetTickCount());
        if (IsTimeout()) {
            DUMPER_HILOGI(MODULE_COMMON, "info|timeout");
            hasOnceTimeout_ = true;
            std::vector<std::string> line;
            line.push_back(GetTimeoutStr());
            dumpDatas->push_back(line);
            return DumpStatus::DUMP_OK;
        }
    }

    hasOnce_ = true;
    DumpStatus ret = PreExecute(parameter, dumpDatas);
    return ret;
}

DumpStatus HidumperExecutor::DoExecute()
{
    if (hasOnceTimeout_) {
        return DumpStatus::DUMP_OK;
    }

    DumpStatus ret = Execute();
    return ret;
}

DumpStatus HidumperExecutor::DoAfterExecute()
{
    if (hasOnceTimeout_) {
        return DumpStatus::DUMP_OK;
    }

    DumpStatus ret = AfterExecute();

    rawParam_ = nullptr;
    return ret;
}

void HidumperExecutor::SetDumpConfig(const std::shared_ptr<DumpCfg>& config)
{
    if (config == nullptr) {
        DUMPER_HILOGF(MODULE_COMMON, "fatal|config is null");
        return;
    }

    ptrDumpCfg_ = config;

    if ((!ptrDumpCfg_->IsOutput()) && (ptrDumpCfg_->parent_ != nullptr)) {
        ptrParent_ = ptrDumpCfg_->parent_->executor_;
    }
}

const std::shared_ptr<DumpCfg>& HidumperExecutor::GetDumpConfig() const
{
    return ptrDumpCfg_;
}

uint64_t HidumperExecutor::GetTimeRemain() const
{
    uint64_t timeRemain = 0;

    uint64_t hitTickCount = GetHitTickCount();
    uint64_t nowTickCount = static_cast<uint64_t>(GetTickCount());
    if (nowTickCount < hitTickCount) {
        DUMPER_HILOGE(MODULE_COMMON, "error|time jumped");
    } else {
        uint64_t tickSpend = nowTickCount - hitTickCount;
        timeRemain = (timeOutMillSec_ > tickSpend) ? (timeOutMillSec_ - tickSpend) : 0;
    }
    return timeRemain;
}

bool HidumperExecutor::IsTimeout() const
{
    if (hasOnceTimeout_) {
        return true;
    }

    return IsTimeout(GetTimeRemain());
}

bool HidumperExecutor::IsCanceled() const
{
    return ((rawParam_ != nullptr) && rawParam_->IsCanceled());
}

std::string HidumperExecutor::GetTimeoutStr(std::string head) const
{
    return head + TIME_OUT_STR;
}

bool HidumperExecutor::IsTimeout(uint64_t remain)
{
    return (remain < 1);
}

uint64_t HidumperExecutor::GetHitTickCount(int nest) const
{
    // no-parent, return this tick count.
    if (ptrParent_ == nullptr) {
        return hitTickCount_;
    }

    // when meet pid group, return tick count.
    if ((ptrDumpCfg_ != nullptr) && ptrDumpCfg_->IsGroup()) {
        if (ptrDumpCfg_->type_ == DumperConstant::GROUPTYPE_PID) {
            return hitTickCount_;
        }
        if (ptrDumpCfg_->type_ == DumperConstant::NONE) {
            return hitTickCount_;
        }
    }

    // get parent tick count, this maybe nesting.
    return ptrParent_->GetHitTickCount(nest + 1);
}
} // namespace HiviewDFX
} // namespace OHOS
