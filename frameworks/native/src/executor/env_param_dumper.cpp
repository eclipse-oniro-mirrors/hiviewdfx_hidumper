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
#include "executor/env_param_dumper.h"

namespace OHOS {
namespace HiviewDFX {
EnvParamDumper::EnvParamDumper()
{
}

EnvParamDumper::~EnvParamDumper()
{
}

DumpStatus EnvParamDumper::PreExecute(const std::shared_ptr<DumperParameter>& parameter,
    StringMatrix dumpDatas)
{
    return DumpStatus::DUMP_OK;
}

DumpStatus EnvParamDumper::Execute()
{
    return DumpStatus::DUMP_OK;
}

DumpStatus EnvParamDumper::AfterExecute()
{
    return DumpStatus::DUMP_OK;
}

void EnvParamDumper::RunDump()
{
}

void EnvParamDumper::StartDump()
{
}

void EnvParamDumper::Dump()
{
}

void EnvParamDumper::StopDump()
{
}

void EnvParamDumper::ReadEnvVariable()
{
}
} // namespace HiviewDFX
} // namespace OHOS
