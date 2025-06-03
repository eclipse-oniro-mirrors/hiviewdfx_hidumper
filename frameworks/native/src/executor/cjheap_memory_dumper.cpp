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
#include "executor/cjheap_memory_dumper.h"
#include "dump_common_utils.h"

using namespace std;
namespace OHOS {
namespace HiviewDFX {
CjHeapMemoryDumper::CjHeapMemoryDumper()
{
    if (cjHeapInfo_ == nullptr) {
        cjHeapInfo_ = std::make_unique<DumpCjHeapInfo>();
    }
}

CjHeapMemoryDumper::~CjHeapMemoryDumper()
{
}

DumpStatus CjHeapMemoryDumper::PreExecute(const shared_ptr<DumperParameter> &parameter, StringMatrix dumpDatas)
{
    if (parameter == nullptr) {
        DUMPER_HILOGE(MODULE_SERVICE, "parameter is nullptr");
        return DumpStatus::DUMP_FAIL;
    }
    needGc_ = true;
    needSnapshot_ = true;
    if (parameter->GetOpts().isDumpCjHeapMemGC_) {
        needSnapshot_ = false;
        DUMPER_HILOGD(MODULE_SERVICE, "trigger gc");
    }
    pid_ = static_cast<uint32_t>(parameter->GetOpts().dumpCjHeapMemPid_);
    dumpDatas_ = dumpDatas;
    return DumpStatus::DUMP_OK;
}

DumpStatus CjHeapMemoryDumper::Execute()
{
    OHOS::AppExecFwk::CjHeapDumpInfo info;
    info.pid = pid_;
    info.needGc = needGc_;
    info.needSnapshot = needSnapshot_;

    if (dumpDatas_ != nullptr && cjHeapInfo_ != nullptr) {
        bool ret = cjHeapInfo_->DumpCjHeapMemory(info);
        ret ? status_ = DumpStatus::DUMP_OK : status_ = DumpStatus::DUMP_FAIL;
    } else {
        DUMPER_HILOGE(MODULE_SERVICE, "CjHeapMemoryDumper Execute nullptr");
        status_ = DumpStatus::DUMP_FAIL;
    }
    return status_;
}

DumpStatus CjHeapMemoryDumper::AfterExecute()
{
    return status_;
}
} // namespace HiviewDFX
} // namespace OHOS