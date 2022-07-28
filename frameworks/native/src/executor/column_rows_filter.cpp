/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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
#include "executor/column_rows_filter.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace HiviewDFX {
ColumnRowsFilter::ColumnRowsFilter()
{
}

ColumnRowsFilter::~ColumnRowsFilter()
{
}

DumpStatus ColumnRowsFilter::PreExecute(const std::shared_ptr<DumperParameter>& parameter,
    StringMatrix dumpDatas)
{
    DUMPER_HILOGD(MODULE_COMMON, "debug|ColumnRowsFilter PreExecute");
    return DumpStatus::DUMP_OK;
}

DumpStatus ColumnRowsFilter::Execute()
{
    return DumpStatus::DUMP_OK;
}

DumpStatus ColumnRowsFilter::AfterExecute()
{
    return DumpStatus::DUMP_OK;
}

void ColumnRowsFilter::GetRuleTable()
{
}
} // namespace HiviewDFX
} // namespace OHOS
