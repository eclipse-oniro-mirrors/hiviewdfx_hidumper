/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "common/dumper_parameter.h"

#include "hilog_wrapper.h"
#include "securec.h"
namespace OHOS {
namespace HiviewDFX {
DumperParameter::DumperParameter()
{
    DUMPER_HILOGD(MODULE_COMMON, "enter|");
    (void)memset_s(&opts_, sizeof(opts_), 0, sizeof(opts_));
    opts_.Reset();

    DUMPER_HILOGD(MODULE_COMMON, "leave|");
}

DumperParameter::~DumperParameter()
{
    DUMPER_HILOGD(MODULE_COMMON, "enter|");

    opts_.Reset();

    DUMPER_HILOGD(MODULE_COMMON, "leave|");
}

void DumperParameter::SetOpts(const DumperOpts &opts)
{
    opts_ = opts;
}

const DumperOpts &DumperParameter::GetOpts() const
{
    return opts_;
}

void DumperParameter::SetOutputFilePath(std::string &path)
{
    opts_.path_ = path;
}

std::string &DumperParameter::GetOutputFilePath()
{
    return opts_.path_;
}

void DumperParameter::SetExecutorConfigList(std::vector<std::shared_ptr<DumpCfg>> &list)
{
    list_ = list;
}

std::vector<std::shared_ptr<DumpCfg>> &DumperParameter::GetExecutorConfigList()
{
    return list_;
}

void DumperParameter::setClientCallback(const std::shared_ptr<RawParam> &reqCtl)
{
    mPtrReqCtl = reqCtl;
}

std::shared_ptr<RawParam> DumperParameter::getClientCallback()
{
    return mPtrReqCtl;
}

void DumperParameter::Dump() const
{
    opts_.Dump();
}
} // namespace HiviewDFX
} // namespace OHOS
