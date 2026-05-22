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
#include "executor/heap_memory_dumper.h"
#include "dump_common_utils.h"

using namespace std;
namespace OHOS {
namespace HiviewDFX {
HeapMemoryDumper::HeapMemoryDumper()
{
    heapInfo_ = std::make_unique<DumpHeapInfo>();
}

HeapMemoryDumper::~HeapMemoryDumper()
{
}

AppExecFwk::MemDumpType HeapMemoryDumper::DetermineDumpType(
    const std::shared_ptr<DumperParameter> &parameter) const
{
    bool isKotlin = parameter->GetOpts().isDumpHeapKotlin_;
    bool isNative = parameter->GetOpts().isDumpHeapNative_;
    bool isJsvm = parameter->GetOpts().isDumpHeapJsvm_;
    bool isArkwebJs = parameter->GetOpts().isDumpHeapArkwebJs_;
    if (isKotlin) {
        return AppExecFwk::MemDumpType::KMP_KOTLIN;
    } else if (isNative) {
        return AppExecFwk::MemDumpType::NATIVE;
    } else if (isJsvm) {
        return AppExecFwk::MemDumpType::JSVM;
    } else if (isArkwebJs) {
        return AppExecFwk::MemDumpType::ARKWEB_JS;
    }
    return AppExecFwk::MemDumpType::INVALID;
}

DumpStatus HeapMemoryDumper::PreExecute(const shared_ptr<DumperParameter> &parameter, StringMatrix dumpDatas)
{
    if (parameter == nullptr) {
        DUMPER_HILOGE(MODULE_SERVICE, "parameter is nullptr");
        return DumpStatus::DUMP_FAIL;
    }
    needLeakobj_ = parameter->GetOpts().isDumpHeapLeakobj_;
    needRaw_ = parameter->GetOpts().dumpRawHeap_;
    if (parameter->GetOpts().dumpHeapArgPid_ > 0) {
        pid_ = parameter->GetOpts().dumpHeapArgPid_;
    } else {
        pid_ = parameter->GetOpts().dumpHeapMemPid_;
    }
    tid_ = parameter->GetOpts().threadId_;
    renderPid_ = parameter->GetOpts().dumpHeapRenderPid_;
    needGc_ = parameter->GetOpts().isDumpHeapMemGc_;
    // If gc is requested, no dump needed
    if (needGc_) {
        needDump_ = false;
    }
    dumpDatas_ = dumpDatas;
    dumpType_ = DetermineDumpType(parameter);
    return DumpStatus::DUMP_OK;
}

DumpStatus HeapMemoryDumper::Execute()
{
    OHOS::AppExecFwk::MemDumpInfo info;
    info.pid = pid_;
    info.tid = tid_;
    info.dumpType = dumpType_;
    info.needLeakobj = needLeakobj_;
    info.needRaw = needRaw_;
    info.mayReportToOEM = false;
    info.renderPid = renderPid_;
    info.needDump = needDump_;
    info.needGc = needGc_;

    if (info.dumpType == AppExecFwk::MemDumpType::NATIVE && info.needLeakobj) {
        info.isSync = true;
    } else {
        info.isSync = false;
    }
    if (dumpDatas_ != nullptr && heapInfo_ != nullptr) {
        bool ret = heapInfo_->DumpHeapMemory(info, dumpResult_);
        ret ? status_ = DumpStatus::DUMP_OK : status_ = DumpStatus::DUMP_FAIL;
    } else {
        DUMPER_HILOGE(MODULE_SERVICE, "HeapMemoryDumper Execute nullptr");
        status_ = DumpStatus::DUMP_FAIL;
    }
    return status_;
}

DumpStatus HeapMemoryDumper::AfterExecute()
{
    if (dumpDatas_ == nullptr) {
        DUMPER_HILOGE(MODULE_SERVICE, "HeapMemoryDumper AfterExecute dumpDatas_ is nullptr");
        return DumpStatus::DUMP_FAIL;
    }
    if (needLeakobj_) {
        vector<string> result;
        if (!dumpResult_.empty()) {
            result.push_back(dumpResult_);
        } else {
            result.push_back("Failed to get native heap leak object info. "
                "Please ensure the application is in foreground with screen on.");
        }
        dumpDatas_->push_back(result);
    }
    return status_;
}
} // namespace HiviewDFX
} // namespace OHOS
