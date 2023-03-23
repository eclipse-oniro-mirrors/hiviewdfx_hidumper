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
#include "executor/memory_dumper.h"
#include "dump_common_utils.h"

using namespace std;
namespace OHOS {
namespace HiviewDFX {
MemoryDumper::MemoryDumper()
{
    if (memoryInfo_ == nullptr) {
        memoryInfo_ = std::make_unique<MemoryInfo>();
    }
    if (smapsMemoryInfo_ == nullptr) {
        smapsMemoryInfo_ = std::make_unique<SmapsMemoryInfo>();
    }
}

MemoryDumper::~MemoryDumper()
{
}

DumpStatus MemoryDumper::PreExecute(const shared_ptr<DumperParameter> &parameter, StringMatrix dumpDatas)
{
    pid_ = parameter->GetOpts().memPid_;
    isShowMaps_ = parameter->GetOpts().isShowSmaps_;
    DUMPER_HILOGI(MODULE_SERVICE, "MemoryDumper pid:%{public}d\n", pid_);
    DUMPER_HILOGI(MODULE_SERVICE, "MemoryDumper isShowMapsvalue is:%{public}d\n", isShowMaps_);
    dumpDatas_ = dumpDatas;
    return DumpStatus::DUMP_OK;
}

DumpStatus MemoryDumper::Execute()
{
    if (dumpDatas_ != nullptr && memoryInfo_ != nullptr) {
        if (pid_ >= 0) {
            bool isShowMapsFlag = false;
            if (isShowMaps_) {
                DUMPER_HILOGI(MODULE_SERVICE, "isShowMaps is true and current is debugmodel");
                isShowMapsFlag = smapsMemoryInfo_->ShowMemorySmapsByPid(pid_, dumpDatas_);
                status_ = isShowMapsFlag ? DumpStatus::DUMP_OK : DumpStatus::DUMP_FAIL;
                return status_;
            }
            bool success = memoryInfo_->GetMemoryInfoByPid(pid_, dumpDatas_);
            if (success) {
                status_ = DumpStatus::DUMP_OK;
            } else {
                status_ = DumpStatus::DUMP_FAIL;
            }
        } else {
            status_ = memoryInfo_->GetMemoryInfoNoPid(dumpDatas_);
        }
    }
    return status_;
}

DumpStatus MemoryDumper::AfterExecute()
{
    return status_;
}
} // namespace HiviewDFX
} // namespace OHOS