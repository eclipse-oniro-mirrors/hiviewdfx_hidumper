/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include "dump_manager_service.h"
#include <file_ex.h>
#include <if_system_ability_manager.h>
#include <ipc_skeleton.h>
#include <iservice_registry.h>
#include <string_ex.h>
#include <system_ability_definition.h>
#include <thread>
#include <unistd.h>

#include "common.h"
#include "common/dumper_constant.h"
#include "dump_log_manager.h"
#include "hilog_wrapper.h"
#include "inner/dump_service_id.h"
#include "manager/dump_implement.h"
#include "raw_param.h"
#include "token_setproc.h"
using namespace std;
namespace OHOS {
namespace HiviewDFX {
namespace {
const std::string DUMPMGR_SERVICE_NAME = "HiDumperManagerService";
auto dumpManagerService = DelayedSpSingleton<DumpManagerService>::GetInstance();
const bool G_REGISTER_RESULT = SystemAbility::MakeAndRegisterAbility(dumpManagerService.GetRefPtr());
static const int32_t STOP_WAIT = 3;
static const int32_t REQUEST_MAX = 2;
static const uint32_t REQUESTID_MAX = 100000;
} // namespace
DumpManagerService::DumpManagerService() : SystemAbility(DFX_SYS_HIDUMPER_ABILITY_ID, true)
{
}

DumpManagerService::~DumpManagerService()
{
}

void DumpManagerService::OnStart()
{
    if (started_) {
        DUMPER_HILOGE(MODULE_SERVICE, "error|it's ready, nothing to do.");
        return;
    }

    if (!Init()) {
        DUMPER_HILOGE(MODULE_SERVICE, "error|init fail, nothing to do.");
        return;
    }

    if (eventRunner_ != nullptr) {
        eventRunner_->Run();
    }

    if (!Publish(DelayedSpSingleton<DumpManagerService>::GetInstance())) {
        DUMPER_HILOGE(MODULE_SERVICE, "error|register to system ability manager failed.");
        return;
    }
    started_ = true;
}

void DumpManagerService::OnStop()
{
    if (!started_) {
        return;
    }
    DUMPER_HILOGD(MODULE_SERVICE, "enter|");
    blockRequest_ = true;
    CancelAllRequest();
    for (int i = 0; i < STOP_WAIT; i++) {
        if (requestRawParamMap_.empty()) {
            break;
        }
        sleep(1);
    }
    started_ = false;
    blockRequest_ = false;
    DUMPER_HILOGD(MODULE_SERVICE, "leave|");
}

int32_t DumpManagerService::Dump(int32_t fd, const std::vector<std::u16string> &args)
{
    std::string result = DUMPMGR_SERVICE_NAME;
    if (!SaveStringToFd(fd, result)) {
        DUMPER_HILOGE(MODULE_SERVICE, "DumpManagerService::Dump failed, save to fd failed.");
        DUMPER_HILOGE(MODULE_SERVICE, "Dump Info:\n");
        DUMPER_HILOGE(MODULE_SERVICE, "%{public}s", result.c_str());
        return ERR_OK;
    }
    return ERR_OK;
}

int32_t DumpManagerService::Request(std::vector<std::u16string> &args, int outfd)
{
    if (blockRequest_) {
        return DumpStatus::DUMP_FAIL;
    }
    if (!started_) {
        return DumpStatus::DUMP_FAIL;
    }
    int sum = GetRequestSum();
    DUMPER_HILOGD(MODULE_SERVICE, "debug|sum=%{public}d", sum);
    if (sum >= REQUEST_MAX) {
        return DumpStatus::DUMP_REQUEST_MAX;
    } else if (sum == 0) {
        DumpLogManager::Init();
    }
    DUMPER_HILOGD(MODULE_SERVICE, "enter|");
    const std::shared_ptr<RawParam> rawParam = AddRequestRawParam(args, outfd);
    int32_t ret = StartRequest(rawParam);
    DUMPER_HILOGD(MODULE_SERVICE, "leave|ret=%{public}d", ret);
    return ret;
}

std::shared_ptr<DumpEventHandler> DumpManagerService::GetHandler() const
{
    return handler_;
}

#ifdef DUMP_TEST_MODE // for mock test
void DumpManagerService::SetTestMainFunc(DumpManagerServiceTestMainFunc testMainFunc)
{
    testMainFunc_ = testMainFunc;
}
#endif // for mock test

bool DumpManagerService::Init()
{
    if (!eventRunner_) {
        eventRunner_ = AppExecFwk::EventRunner::Create(DUMPMGR_SERVICE_NAME);
        if (eventRunner_ == nullptr) {
            DUMPER_HILOGE(MODULE_SERVICE, "error|create EventRunner");
            return false;
        }
    }
    if (!handler_) {
        handler_ = std::make_shared<DumpEventHandler>(eventRunner_, dumpManagerService);
    }
    handler_->SendEvent(DumpEventHandler::MSG_GET_CPU_INFO_ID, DumpEventHandler::GET_CPU_INFO_DELAY_TIME);
    return true;
}

int DumpManagerService::GetRequestSum()
{
    unique_lock<mutex> lock(mutex_);
    return requestRawParamMap_.size();
}

std::shared_ptr<RawParam> DumpManagerService::AddRequestRawParam(std::vector<std::u16string> &args, int outfd)
{
    unique_lock<mutex> lock(mutex_);
    uint32_t requestId = 0;
    do { // find a requestId
        requestId = GetRequestId();
    } while (requestRawParamMap_.count(requestId) > 0);
    int32_t calllingUid = IPCSkeleton::GetCallingUid();
    int32_t calllingPid = IPCSkeleton::GetCallingPid();
    auto calllingTokenID = IPCSkeleton::GetCallingTokenID();
    SetFirstCallerTokenID(calllingTokenID);
    DUMPER_HILOGD(MODULE_SERVICE, "debug|requestId=%{public}u, calllingUid=%{public}d, calllingPid=%{public}d",
                  requestId, calllingUid, calllingPid);
    std::shared_ptr<RawParam> requestHandle =
        std::make_shared<RawParam>(calllingUid, calllingPid, requestId, args, outfd);
    requestRawParamMap_.insert(std::make_pair(requestId, requestHandle));
    return requestHandle;
}

void DumpManagerService::EraseRequestRawParam(const std::shared_ptr<RawParam> rawParam)
{
    if (rawParam == nullptr) {
        return;
    }
    DUMPER_HILOGD(MODULE_SERVICE, "enter|");
    unique_lock<mutex> lock(mutex_);
    uint32_t requestId = rawParam->GetRequestId();
    DUMPER_HILOGD(MODULE_SERVICE, "debug|requestId=%{public}u", requestId);
    if (requestRawParamMap_.count(requestId) > 0) {
        requestRawParamMap_.erase(requestId);
        DUMPER_HILOGD(MODULE_SERVICE, "debug|erase");
    }
    DUMPER_HILOGD(MODULE_SERVICE, "leave|");
}

void DumpManagerService::CancelAllRequest()
{
    DUMPER_HILOGD(MODULE_SERVICE, "enter|");
    unique_lock<mutex> lock(mutex_);
    for (auto &requestIt : requestRawParamMap_) {
        if (requestIt.second == nullptr) {
            continue;
        }
        requestIt.second->Cancel();
    }
    DUMPER_HILOGD(MODULE_SERVICE, "leave|");
}

uint32_t DumpManagerService::GetRequestId()
{
    requestIndex_ = (requestIndex_ + 1) % REQUESTID_MAX;
    return requestIndex_;
}

int32_t DumpManagerService::StartRequest(const std::shared_ptr<RawParam> rawParam)
{
    RequestMain(rawParam);
    return DumpStatus::DUMP_OK;
}

void DumpManagerService::RequestMain(const std::shared_ptr<RawParam> rawParam)
{
    DUMPER_HILOGD(MODULE_SERVICE, "enter|");
    int argC = rawParam->GetArgc();
    char **argV = rawParam->GetArgv();
    std::string folder = DumpLogManager::CreateTmpFolder(rawParam->GetRequestId());
    rawParam->SetFolder(folder);
    if ((argC > 0) && (argV != nullptr)) {
        DUMPER_HILOGD(MODULE_SERVICE, "debug|enter task, argC=%{public}d", argC);
        for (int i = 0; i < argC; i++) {
            DUMPER_HILOGD(MODULE_SERVICE, "debug|argV[%{public}d]=%{public}s", i, argV[i]);
        }
        DumpImplement::GetInstance().Main(argC, argV, rawParam);
        DUMPER_HILOGD(MODULE_SERVICE, "debug|leave task");
    }
    DumpLogManager::EraseTmpFolder(rawParam->GetRequestId());
    DumpLogManager::EraseLogs();
    rawParam->CloseOutputFd();
    EraseRequestRawParam(rawParam);
    DUMPER_HILOGD(MODULE_SERVICE, "leave|");
}
} // namespace HiviewDFX
} // namespace OHOS
