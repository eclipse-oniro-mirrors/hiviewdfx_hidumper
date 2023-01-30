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
#ifndef HIDUMPER_ZIDL_IDUMP_BROKER_H
#define HIDUMPER_ZIDL_IDUMP_BROKER_H
#include <string>
#include <iremote_broker.h>
#include <iremote_object.h>
namespace OHOS {
namespace HiviewDFX {
class IDumpBroker : public IRemoteBroker {
public:
    enum IDumpBrokerID {
        DUMP_REQUEST_FILEFD,
    };
    // Used for dump request
    // return: = 0 OK; < 0 Error;
    virtual int32_t Request(std::vector<std::u16string> &args, int outfd) = 0;
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.HiviewDFX.HiDumper.IDumpBroker");
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIDUMPER_ZIDL_IDUMP_BROKER_H
