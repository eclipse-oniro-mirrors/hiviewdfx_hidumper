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
#include "hidumper_test_default_callback.h"
namespace OHOS {
namespace HiviewDFX {
HidumperTestDefaultCallback::HidumperTestDefaultCallback(uint32_t& statusStart,
    uint32_t& statusFinish, uint32_t& statusError)
    : statusStart_(statusStart), statusFinish_(statusFinish), statusError_(statusError)
{
}

void HidumperTestDefaultCallback::OnStatusChanged(uint32_t status)
{
    switch (status) {
        case IDumpCallbackBroker::STATUS_DUMP_STARTED: {
            statusStart_ = IDumpCallbackBroker::STATUS_DUMP_STARTED;
            break;
        }
        case IDumpCallbackBroker::STATUS_DUMP_FINISHED: {
            statusFinish_ = IDumpCallbackBroker::STATUS_DUMP_FINISHED;
            break;
        }
        case IDumpCallbackBroker::STATUS_DUMP_ERROR: {
            statusError_ = IDumpCallbackBroker::STATUS_DUMP_ERROR;
            break;
        }
        default:
            break;
    }
}
} // namespace HiviewDFX
} // namespace OHOS