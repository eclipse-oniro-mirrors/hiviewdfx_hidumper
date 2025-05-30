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
#include "dump_utils.h"
#include <dlfcn.h>

using namespace std;
namespace OHOS {
namespace HiviewDFX {
static const std::string MEM_LIB = "libhidumpermemory.z.so";

MemoryDumper::MemoryDumper()
{
}

MemoryDumper::~MemoryDumper()
{
}

DumpStatus MemoryDumper::PreExecute(const shared_ptr<DumperParameter> &parameter, StringMatrix dumpDatas)
{
    pid_ = parameter->GetOpts().memPid_;
    isShowMaps_ = parameter->GetOpts().isShowSmaps_;
    isShowSmapsInfo_ =  parameter->GetOpts().isShowSmapsInfo_;
    dumpMemPrune_ = parameter->GetOpts().dumpMemPrune_;
    dumpDatas_ = dumpDatas;

    bool isZip = parameter->GetOpts().IsDumpZip();
    auto callback = parameter->getClientCallback();
    if (callback == nullptr) {
        DUMPER_HILOGE(MODULE_SERVICE, "PreExecute error|callback is nullptr");
        return DumpStatus::DUMP_FAIL;
    }
    std::string logDefaultPath_ = callback->GetFolder() + "log.txt";
    if (isZip) {
        rawParamFd_ = DumpUtils::FdToWrite(logDefaultPath_);
    } else {
        rawParamFd_ = parameter->getClientCallback()->GetOutputFd();
    }
    return DumpStatus::DUMP_OK;
}

DumpStatus MemoryDumper::Execute()
{
    DUMPER_HILOGI(MODULE_SERVICE, "info|MemoryDumper Execute enter");
    if (dumpDatas_ != nullptr) {
        if (pid_ >= 0) {
            if (isShowMaps_) {
                GetMemSmapsByPid();
                DUMPER_HILOGI(MODULE_SERVICE, "get mem smaps end,pid:%{public}d", pid_);
                return status_;
            }
            GetMemByPid();
        } else {
            if (dumpMemPrune_) {
                GetMemPruneNoPid();
            } else {
                GetMemNoPid();
            }
        }
    }
    DUMPER_HILOGI(MODULE_SERVICE, "info|MemoryDumper Execute end");
    return status_;
}

void MemoryDumper::GetMemByPid()
{
    void *handle = dlopen(MEM_LIB.c_str(), RTLD_LAZY | RTLD_NODELETE);
    if (handle == nullptr) {
        DUMPER_HILOGE(MODULE_SERVICE, "fail to open %{public}s. errno:%{public}s", MEM_LIB.c_str(), dlerror());
        return;
    }
    GetMemByPidFunc pfn = reinterpret_cast<GetMemByPidFunc>(dlsym(handle, "GetMemoryInfoByPid"));
    if (pfn == nullptr) {
        DUMPER_HILOGE(MODULE_SERVICE, "fail to dlsym GetMemoryInfoByPid. errno:%{public}s", dlerror());
        dlclose(handle);
        return;
    }
    if (!pfn(pid_, dumpDatas_)) {
        status_ = DumpStatus::DUMP_OK;
    } else {
        DUMPER_HILOGE(MODULE_SERVICE, "MemoryDumper Execute failed, pid:%{public}d", pid_);
        status_ = DumpStatus::DUMP_FAIL;
    }
    dlclose(handle);
}

void MemoryDumper::GetMemNoPid()
{
    void* handle = dlopen(MEM_LIB.c_str(), RTLD_LAZY | RTLD_NODELETE);
    if (handle == nullptr) {
        DUMPER_HILOGE(MODULE_SERVICE, "fail to open %{public}s. errno:%{public}s", MEM_LIB.c_str(), dlerror());
        return;
    }
    GetMemNoPidFunc getMemNoPidFunc = reinterpret_cast<GetMemNoPidFunc>(dlsym(handle, "GetMemoryInfoNoPid"));
    if (getMemNoPidFunc == nullptr) {
        DUMPER_HILOGE(MODULE_SERVICE, "fail to dlsym GetMemoryInfoNoPid. errno:%{public}s", dlerror());
        dlclose(handle);
        status_ = DUMP_FAIL;
        return;
    }
    status_ = (DumpStatus)(getMemNoPidFunc(rawParamFd_, dumpDatas_));
    dlclose(handle);
}

void MemoryDumper::GetMemPruneNoPid()
{
    void* handle = dlopen(MEM_LIB.c_str(), RTLD_LAZY | RTLD_NODELETE);
    if (handle == nullptr) {
        DUMPER_HILOGE(MODULE_SERVICE, "fail to open %{public}s. errno:%{public}s", MEM_LIB.c_str(), dlerror());
        return;
    }
    GetMemPruneNoPidFunc getMemPruneNoPidFunc = reinterpret_cast<GetMemPruneNoPidFunc>(
        dlsym(handle, "GetMemoryInfoPrune"));
    if (getMemPruneNoPidFunc == nullptr) {
        DUMPER_HILOGE(MODULE_SERVICE, "fail to dlsym GetMemoryInfoPrune. errno:%{public}s", dlerror());
        dlclose(handle);
        status_ = DUMP_FAIL;
        return;
    }
    status_ = (DumpStatus)(getMemPruneNoPidFunc(rawParamFd_, dumpDatas_));
    dlclose(handle);
}

void MemoryDumper::GetMemSmapsByPid()
{
    void *handle = dlopen(MEM_LIB.c_str(), RTLD_LAZY | RTLD_NODELETE);
    if (handle == nullptr) {
        DUMPER_HILOGE(MODULE_SERVICE, "fail to open %{public}s. errno:%{public}s", MEM_LIB.c_str(), dlerror());
        return;
    }
    GetMemSmapsByPidFunc pfn = reinterpret_cast<GetMemSmapsByPidFunc>(dlsym(handle, "ShowMemorySmapsByPid"));
    if (pfn == nullptr) {
        DUMPER_HILOGE(MODULE_SERVICE, "fail to dlsym ShowMemorySmapsByPid. errno:%{public}s", dlerror());
        dlclose(handle);
        return;
    }
    if (!pfn(pid_, dumpDatas_, isShowSmapsInfo_)) {
        status_ = DumpStatus::DUMP_OK;
    } else {
        DUMPER_HILOGE(MODULE_SERVICE, "GetMemSmapsByPid failed, pid:%{public}d", pid_);
        status_ = DumpStatus::DUMP_FAIL;
    }
    dlclose(handle);
}

DumpStatus MemoryDumper::AfterExecute()
{
    return status_;
}
} // namespace HiviewDFX
} // namespace OHOS