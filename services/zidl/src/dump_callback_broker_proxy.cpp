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
#include "dump_callback_broker_proxy.h"
#include <message_parcel.h>
#include "dump_errors.h"
#include "hilog_wrapper.h"
namespace OHOS {
namespace HiviewDFX {
void DumpCallbackBrokerProxy::OnStatusChanged(uint32_t status)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        return;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(DumpCallbackBrokerProxy::GetDescriptor())) {
        return;
    }
    if (!data.WriteUint32(status)) {
        return;
    }
    int ret = remote->SendRequest(static_cast<int>(IDumpCallbackBroker::ON_STATUS_CHANGED),
        data, reply, option);
    if (ret != ERR_OK) {
        DUMPER_HILOGE(MODULE_ZIDL, "error|SendRequest error code: %{public}d", ret);
    }
}
} // namespace HiviewDFX
} // namespace OHOS
