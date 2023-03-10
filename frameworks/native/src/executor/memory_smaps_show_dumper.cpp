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
#include "executor/memory_smap_show_dumper.h"
#include "dump_common_utils.h"

using namespace std;
namespace OHOS {
namespace HiviewDFX {
MemorySmapsShowDumper::MemorySmapsShowDumper()
{
    if (memoryInfo_ == nullptr) {
        memoryInfo_ = std::make_unique<MemoryInfo>();
    }
}

MemorySmapsShowDumper::~MemorySmapsShowDumper()
{
}

DumpStatus MemorySmapsShowDumper::PreExecute(const shared_ptr<DumperParameter> &parameter, StringMatrix dumpDatas)
{
    pid_ = parameter->GetOpts().memPid_;
    DUMPER_HILOGD(MODULE_SERVICE, "MemorySmapsShowDumper pid:%d\n", pid_);
    dumpDatas_ = dumpDatas;
    return DumpStatus::DUMP_OK;
}

DumpStatus MemorySmapsShowDumper::Execute()
{
    if (dumpDatas_ != nullptr && memoryInfo_ != nullptr) {
        if (pid_ >= 0) {
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

DumpStatus MemorySmapsShowDumper::AfterExecute()
{
    return status_;
}
} // namespace HiviewDFX
} // namespace OHOS