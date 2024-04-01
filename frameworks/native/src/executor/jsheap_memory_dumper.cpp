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
#include "executor/jsheap_memory_dumper.h"
#include "executor/memory/parse/parse_smaps_info.h"
#include "dump_common_utils.h"
#include "file_ex.h"
#include <fcntl.h>
#include <sstream>

using namespace std;
namespace OHOS {
namespace HiviewDFX {
const int THOUSAND = 1000; // 1000 : second to millisecond convert ratio
const int MILLION = 1000 * THOUSAND; // 1000000 : second to microsecond convert ratio
const int CLOSE_FD_TIMEOUT = 60; // close fd timeout
JsHeapMemoryDumper::JsHeapMemoryDumper()
{
    if (jsHeapInfo_ == nullptr) {
        jsHeapInfo_ = std::make_unique<DumpJsHeapInfo>();
    }
}

JsHeapMemoryDumper::~JsHeapMemoryDumper()
{
}

void JsHeapMemoryDumper::SendJsHeapDumpMessage(const std::string &errorStr)
{
    if (ptrReqCtl_ == nullptr) {
        DUMPER_HILOGE(MODULE_SERVICE, "ptrReqCtl_ is nullptr, return");
        return;
    }
    int rawParamFd = ptrReqCtl_->GetOutputFd();
    if (rawParamFd < 0) {
        DUMPER_HILOGE(MODULE_SERVICE, "rawParamFd(%{public}d) is invalid, return", rawParamFd);
        return;
    }
    SaveStringToFd(rawParamFd, errorStr + "\n");
}

DumpStatus JsHeapMemoryDumper::PreExecute(const shared_ptr<DumperParameter> &parameter, StringMatrix dumpDatas)
{
    needGc_ = true;
    needSnapshot_ = true;
    if (parameter->GetOpts().isDumpJsHeapMemGC_) {
        needSnapshot_ = false;
    }
    pid_ = static_cast<uint32_t>(parameter->GetOpts().dumpJsHeapMemPid_);
    tid_ = static_cast<uint32_t>(parameter->GetOpts().threadId_);
    dumpDatas_ = dumpDatas;
    return DumpStatus::DUMP_OK;
}

DumpStatus JsHeapMemoryDumper::Execute()
{
    DUMPER_HILOGI(MODULE_SERVICE, "JsHeapMemoryDumper enter, pid:%{public}d", pid_);
    SendJsHeapDumpMessage("start dump jsheap snapshot...");
    OHOS::AppExecFwk::JsHeapDumpInfo info;
    info.pid = pid_;
    info.tid = tid_;
    info.needGc = needGc_;
    info.needSnapshot = needSnapshot_;

    unique_ptr<ParseSmapsInfo> parseSmapsInfo = make_unique<ParseSmapsInfo>();
    std::vector<uint32_t> tidVec;
    if (!parseSmapsInfo->GetArkTsHeapTid(pid_, tidVec)) {
        DUMPER_HILOGE(MODULE_SERVICE, "GetArkTsHeapTid failed, pid:%{public}d", pid_);
        status_ = DumpStatus::DUMP_FAIL;
    }
    for (size_t i = 0; i < tidVec.size(); i++) {
        if (tid_ > 0 && tidVec[i] != tid_) {
            continue;
        }
        struct timespec ts;
        (void)clock_gettime(CLOCK_REALTIME, &ts);
        uint64_t time = ((uint64_t)ts.tv_sec * THOUSAND) + (((uint64_t)ts.tv_nsec) / MILLION);
        std::stringstream ssJsheapFile;
        ssJsheapFile << "/data/log/hidumper/jsheap_" << std::to_string(pid_) << "_" <<
            std::to_string(tidVec[i]) << "_" << std::to_string(time) << ".heapsnapshot";
        std::string fileName = ssJsheapFile.str();
        int fd = TEMP_FAILURE_RETRY(open(fileName.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC | O_NOFOLLOW,
                                         S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH));
        if (fd == -1) {
            DUMPER_HILOGE(MODULE_SERVICE, "Dump open [%{public}s], errno:[%{public}d]", fileName.c_str(), errno);
            continue;
        }
        info.fdVec.push_back(fd);
        info.tidVec.push_back(tidVec[i]);
    }
    fdVec_ = info.fdVec;
    if (dumpDatas_ != nullptr && jsHeapInfo_ != nullptr) {
        bool ret = jsHeapInfo_->DumpJsHeapMemory(info);
        if (ret) {
            status_ = DumpStatus::DUMP_OK;
        } else {
            DUMPER_HILOGE(MODULE_SERVICE, "DumpJsHeapMemory Execute failed");
            status_ = DumpStatus::DUMP_FAIL;
        }
    } else {
        DUMPER_HILOGE(MODULE_SERVICE, "JsHeapMemoryDumper Execute nullptr");
        status_ = DumpStatus::DUMP_FAIL;
    }
    DUMPER_HILOGI(MODULE_SERVICE, "JsHeapMemoryDumper end");
    return status_;
}

DumpStatus JsHeapMemoryDumper::AfterExecute()
{
    DUMPER_HILOGI(MODULE_SERVICE, "JsHeapMemoryDumper AfterExecute");
    if (status_ == DumpStatus::DUMP_OK) {
        //Due to the inability to determine the end time of Dump, need close fd after sleep 60s
        sleep(CLOSE_FD_TIMEOUT);
        for (auto &fd : fdVec_) {
            if (close(fd) == -1) {
                DUMPER_HILOGE(MODULE_SERVICE, "Dump close [%{public}d], errno:[%{public}d]", fd, errno);
            }
        }
        fdVec_.clear();
    }
    SendJsHeapDumpMessage("stop dump jsheap snapshot!");
    return status_;
}
} // namespace HiviewDFX
} // namespace OHOS