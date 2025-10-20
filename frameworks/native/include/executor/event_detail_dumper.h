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
#ifndef EVENT_DETAIL_DUMPER_H
#define EVENT_DETAIL_DUMPER_H
#include <vector>
#include <string>
#include <memory>
#include "hidumper_executor.h"
#include "event/dump_event_info.h"

namespace OHOS {
namespace HiviewDFX {
class EventDetailDumper : public HidumperExecutor {
public:
    EventDetailDumper();
    ~EventDetailDumper();

    DumpStatus PreExecute(const std::shared_ptr<DumperParameter> &parameter, StringMatrix dumpDatas) override;
    DumpStatus Execute() override;
    DumpStatus AfterExecute() override;
private:
    EventDumpResult QueryFaultEvents();
    void CloseFd();
    std::vector<std::string> FilterLogPaths();
    void ReadLogsByPaths(const std::vector<std::string> &logPaths);
    void ReadSingleLogFile(const std::string &path);
private:
    StringMatrix dumpDatas_;
    std::string processName_;
    long long startTime_;
    long long endTime_;
    int showEventCount_;
    std::string eventId_;
    std::vector<HiSysEventRecord> events_;
    FILE* fp_;
    int fd_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif
