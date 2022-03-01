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
#ifndef HIDUMPER_ZIDL_IDUMP_CALLBACK_BROKER_H
#define HIDUMPER_ZIDL_IDUMP_CALLBACK_BROKER_H
#include <iremote_broker.h>
#include <iremote_object.h>
namespace OHOS {
namespace HiviewDFX {
class IDumpCallbackBroker : public IRemoteBroker {
public:
    enum Status {
        STATUS_INIT = 0,
        STATUS_DUMP_STARTED, // dump started
        STATUS_DUMP_FINISHED, // dump finished
        STATUS_DUMP_ERROR, // dump error
    };
    virtual void OnStatusChanged(uint32_t status) = 0;
public:
    enum IDumpCallbackBrokerID {
        ON_STATUS_CHANGED,
    };
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.HiviewDFX.HiDumper.IDumpCallbackBroker");
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIDUMPER_ZIDL_IDUMP_CALLBACK_BROKER_H