/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#ifndef DUMP_HEAP_INFO_H
#define DUMP_HEAP_INFO_H

#include "executor/memory/memory_filter.h"
#include "hilog_wrapper.h"
#ifdef HIDUMPER_ABILITY_RUNTIME_ENABLE
#include "iremote_object.h"
#include "iremote_stub.h"
#include "message_parcel.h"
#include "message_option.h"
#include "app_malloc_info.h"
#include "app_mgr_interface.h"
#include "mem_dump_callback_interface.h"
#endif
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "app_mem_dump_info.h"
#include <mutex>
#include <condition_variable>

namespace OHOS {
namespace HiviewDFX {
#ifdef HIDUMPER_ABILITY_RUNTIME_ENABLE
class MemDumpCallbackImpl : public IRemoteStub<AppExecFwk::IMemDumpCallback> {
public:
    MemDumpCallbackImpl() : completed_(false) {}
    ~MemDumpCallbackImpl()
    {
        DUMPER_HILOGI(MODULE_SERVICE, "MemDumpCallbackImpl destructor called, callback destroyed normally");
    }

    int32_t OnRemoteRequest(uint32_t code, OHOS::MessageParcel &data,
        OHOS::MessageParcel &reply, OHOS::MessageOption &option) override
    {
        if (data.ReadInterfaceToken() != AppExecFwk::IMemDumpCallback::GetDescriptor()) {
            return ERR_INVALID_STATE;
        }
        if (code == static_cast<uint32_t>(AppExecFwk::IMemDumpCallback::Message::ON_MEM_DUMP_DONE)) {
            std::string dumpResult = data.ReadString();
            OnMemDumpDone(dumpResult);
            return NO_ERROR;
        }
        return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }

    void OnMemDumpDone(const std::string &dumpResult) override
    {
        std::lock_guard<std::mutex> lock(mutex_);
        dumpResult_ = dumpResult;
        completed_ = true;
        cv_.notify_one();
    }

    bool WaitForResult(std::string &dumpResult, int32_t timeoutMs = 10000)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (!cv_.wait_for(lock, std::chrono::milliseconds(timeoutMs), [this] { return completed_; })) {
            return false;
        }
        dumpResult = dumpResult_;
        return true;
    }

private:
    std::string dumpResult_;
    bool completed_;
    std::mutex mutex_;
    std::condition_variable cv_;
};
#endif

class DumpHeapInfo {
public:
    DumpHeapInfo();
    ~DumpHeapInfo();

    bool DumpHeapMemory(OHOS::AppExecFwk::MemDumpInfo &info, std::string &dumpResult);
};
} // namespace HiviewDFX
} // namespace OHOS
#endif