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
#include "factory/column_rows_filter_factory.h"
#include "executor/column_rows_filter.h"

namespace OHOS {
namespace HiviewDFX {
std::shared_ptr<HidumperExecutor> ColumnRowsFilterFactory::CreateExecutor()
{
    return std::make_shared<ColumnRowsFilter>();
}
} // namespace HiviewDFX
} // namespace OHOS
