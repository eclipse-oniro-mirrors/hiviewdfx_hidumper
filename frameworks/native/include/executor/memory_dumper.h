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
#ifndef MEMORY_DUMPER_H
#define MEMORY_DUMPER_H
#include <vector>
#include <string>
#include <map>
#include <memory>
#include "hidumper_executor.h"

namespace OHOS {
namespace HiviewDFX {
class MemoryDumper : public HidumperExecutor {
public:
    MemoryDumper();
    ~MemoryDumper();

    using ValueMap = std::map<std::string, uint64_t>;
    using GroupMap = std::map<std::string, ValueMap>;

    DumpStatus PreExecute(const std::shared_ptr<DumperParameter> &parameter, StringMatrix dumpDatas) override;
    DumpStatus Execute() override;
    DumpStatus AfterExecute() override;
private:
    int pid_ = 0;
    int rawParamFd_ = 0;
    int timeInterval_ = 0;
    bool isShowMaps_ = false;
    bool isShowSmapsInfo_ = false;
    bool isReceivedSigInt_ = false;
    bool dumpMemPrune_ = false;
    bool showAshmem_ = false;

    DumpStatus status_ = DUMP_FAIL;
    StringMatrix dumpDatas_;
    using GetMemByPidFunc = int (*)(int, StringMatrix, bool);
    using GetMemNoPidFunc = int (*)(int, StringMatrix);
    using GetMemPruneNoPidFunc = int (*)(int, StringMatrix);
    using GetMemSmapsByPidFunc = int (*)(int, StringMatrix, bool);
    using GetMemByTimeIntervalFunc = void (*)(int, int, int);
    using SetReceivedSigIntFunc = void (*)(bool);

    void GetMemByPid();
    void GetMemNoPid();
    void GetMemPruneNoPid();
    void GetMemSmapsByPid();
    void GetMemByTimeInterval();
    void SetReceivedSigInt();
};
} // namespace HiviewDFX
} // namespace OHOS
#endif
