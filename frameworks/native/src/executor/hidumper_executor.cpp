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
}

DumpStatus HidumperExecutor::DoPreExecute(const std::shared_ptr<DumperParameter>& parameter, StringMatrix dumpDatas)
{
    if (parameter != nullptr) {
        rawParam_ = parameter->getClientCallback();
    }

    DumpStatus ret = PreExecute(parameter, dumpDatas);
    return ret;
}

DumpStatus HidumperExecutor::DoExecute()
{
    DumpStatus ret = Execute();
    return ret;
}

DumpStatus HidumperExecutor::DoAfterExecute()
{
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

bool HidumperExecutor::IsCanceled() const
{
    return ((rawParam_ != nullptr) && rawParam_->IsCanceled());
}
} // namespace HiviewDFX
} // namespace OHOS
