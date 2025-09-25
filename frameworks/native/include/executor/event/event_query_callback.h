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
#ifndef EVENT_QUERY_CALLBACK_H
#define EVENT_QUERY_CALLBACK_H

#include "hisysevent_manager.h"

#include <condition_variable>
#include <mutex>
#include <atomic>
#include <vector>

namespace OHOS {
namespace HiviewDFX {
class EventQueryCallback : public HiSysEventQueryCallback {
public:
    EventQueryCallback();
    ~EventQueryCallback();

    void OnQuery(std::shared_ptr<std::vector<HiSysEventRecord>> sysEvents) override;
    void OnComplete(int32_t reason, int32_t total) override;
    bool WaitForComplete(std::chrono::milliseconds timeout);
    std::vector<HiSysEventRecord> GetEvents();
    int32_t GetReason();

private:
    std::mutex mutex_;
    std::condition_variable cv_;
    bool completed_;
    std::vector<HiSysEventRecord> events_;
    int32_t reason_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif