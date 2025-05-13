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

#ifndef DUMP_CONTEXT_H
#define DUMP_CONTEXT_H

#include <memory>
#include <set>
#include <string>
#include <unistd.h>
#include <vector>

namespace OHOS {
namespace HiviewDFX {

struct RequestInfo {
    int32_t callingUid;
    int32_t calllingPid;
    int outFd = -1;
};

struct DumperOptions {
    int cpuUsagePid = -1;
    int memPid = -1;
    int storagePid = -1;
    int netPid = -1;
    int processPid = -1;
    int dumpJsHeapMemPid = -1;
    int threadId = -1;
    int ipcStatPid = -1;
    bool isDumpCpuFreq = false;
    bool isDumpCpuUsage = false;
    bool isDumpLog = false;
    bool isDumpMem = false;
    bool isDumpStorage = false;
    bool isDumpNet = false;
    bool isDumpList = false;
    bool isDumpService = false;
    bool isDumpSystemAbility = false;
    bool isDumpSystem = false;
    bool isDumpProcesses = false;
    bool isFaultLog = false;
    bool isAppendix = false;
    bool isShowSmaps = false;
    bool isShowSmapsInfo = false;
    bool isDumpJsHeapMem = false;
    bool isDumpJsHeapMemGC = false;
    bool isDumpJsHeapLeakobj = false;
    bool isDumpAllIpc = false;
    bool isDumpIpc = false;
    bool isDumpIpcStartStat = false;
    bool isDumpIpcStopStat = false;
    bool isDumpIpcStat = false;
    bool dumpJsRawHeap = false;
    std::vector<std::string> abilitieArgs = {};
    std::set<std::string> abilitieNames = {};
    std::set<std::string> systemArgs = {};
};

class DumpContext {
public:
    DumpContext(int32_t uid, int32_t pid, int outFd) :
                requestInfo_(std::make_shared<RequestInfo>(RequestInfo{uid, pid, outFd}))
    {
        dumperOpts_ = std::make_shared<DumperOptions>();
    }
    ~DumpContext()
    {
        if (GetOutputFd() >= 0) {
            close(GetOutputFd());
        }
    }

    std::shared_ptr<RequestInfo> GetRequestInfo() const { return requestInfo_; }
    std::shared_ptr<DumperOptions> GetDumperOpts() const { return dumperOpts_; }
    int GetOutputFd() const { return requestInfo_->outFd; }
private:
    std::shared_ptr<RequestInfo> requestInfo_;
    std::shared_ptr<DumperOptions> dumperOpts_;
};

} // namespace HiviewDFX
} // namespace OHOS
#endif // DUMP_CONTEXT_H
