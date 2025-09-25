/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "executor/event/event_query_callback.h"
#include "hilog_wrapper.h"

using namespace std;
namespace OHOS {
namespace HiviewDFX {
EventQueryCallback::EventQueryCallback() : completed_(false)
{
}

EventQueryCallback::~EventQueryCallback()
{
}

void EventQueryCallback::OnQuery(std::shared_ptr<std::vector<HiSysEventRecord>> sysEvents)
{
    if (sysEvents == nullptr) {
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    events_.insert(events_.end(), sysEvents->begin(), sysEvents->end());
}

void EventQueryCallback::OnComplete(int32_t reason, int32_t total)
{
    std::lock_guard<std::mutex> lock(mutex_);
    completed_ = true;
    reason_ = reason;
    cv_.notify_one();
}

bool EventQueryCallback::WaitForComplete(std::chrono::milliseconds timeout)
{
    std::unique_lock<std::mutex> lock(mutex_);
    return cv_.wait_for(lock, timeout, [this] { return completed_; });
}

std::vector<HiSysEventRecord> EventQueryCallback::GetEvents()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return events_;
}

int32_t EventQueryCallback::GetReason()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return reason_;
}

} // namespace HiviewDFX
} // namespace OHOS
