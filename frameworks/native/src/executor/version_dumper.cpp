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


#include "executor/version_dumper.h"

namespace OHOS {
namespace HiviewDFX {
static constexpr auto& VERSION_DUMPER_VERSION_STRING = "HiDumper version:0.8.8";

VersionDumper::VersionDumper()
{
}

VersionDumper::~VersionDumper()
{
}

DumpStatus VersionDumper::PreExecute(const std::shared_ptr<DumperParameter>& parameter,
    StringMatrix dumpDatas)
{
    if (dumpDatas.get()) {
        result_ = dumpDatas;
        return DumpStatus::DUMP_OK;
    }
    return DumpStatus::DUMP_FAIL;
}

DumpStatus VersionDumper::Execute()
{
    std::vector<std::string> line_vector;
    line_vector.push_back(VERSION_DUMPER_VERSION_STRING);
    result_->push_back(line_vector);
    return DumpStatus::DUMP_OK;
}

DumpStatus VersionDumper::AfterExecute()
{
    return DumpStatus::DUMP_OK;
}
} // namespace HiviewDFX
} // namespace OHOS
