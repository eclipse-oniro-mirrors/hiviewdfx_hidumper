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
#ifndef EXECUTOR_FACTORY_H
#define EXECUTOR_FACTORY_H

#include <memory>
#include "hilog_wrapper.h"
#include "executor/hidumper_executor.h"

namespace OHOS {
namespace HiviewDFX {
class ExecutorFactory {
public:
    virtual ~ExecutorFactory() {};
    virtual std::shared_ptr<HidumperExecutor> CreateExecutor() = 0;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // EXECUTOR_FACTORY_H
