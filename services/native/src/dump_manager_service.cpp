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
#include <sched.h>
#include <string_ex.h>
#include <sstream>
#include <sys/syscall.h>
#include <system_ability_definition.h>
#include <thread>
#include <unistd.h>

#include "common.h"
#include "common/dumper_constant.h"
#include "dump_log_manager.h"
#include "inner/dump_service_id.h"
#include "hilog_wrapper.h"
#include "manager/dump_implement.h"
#include "raw_param.h"
#include "token_setproc.h"
#include "accesstoken_kit.h"
#include "system_ability_ondemand_reason.h"

using namespace std;
namespace OHOS {
namespace HiviewDFX {
namespace {
const std::string DUMPMGR_SERVICE_NAME = "HiDumperManagerService";
auto dumpManagerService = DumpDelayedSpSingleton<DumpManagerService>::GetInstance();
const bool G_REGISTER_RESULT = SystemAbility::MakeAndRegisterAbility(dumpManagerService.GetRefPtr());
static const int32_t HIPORFILER_UID = 3063;
static const int32_t STOP_WAIT = 3;
static const int32_t REQUEST_MAX = 5;
static const uint32_t REQUESTID_MAX = 100000;
static const int SMALL_CPU_SIZE = 4;
const std::string TASK_ID = "unload";
constexpr int32_t DYNAMIC_EXIT_DELAY_TIME = 120000;
constexpr int32_t UNLOAD_IMMEDIATELY = 0;
} // namespace
namespace {
static const int32_t FD_LOG_NUM = 10;
std::map<std::string, WpId> g_fdLeakWp {
    {"eventfd", FDLEAK_WP_EVENTFD},
    {"eventpoll", FDLEAK_WP_EVENTPOLL},
    {"sync_file", FDLEAK_WP_SYNCFENCE},
    {"dmabuf", FDLEAK_WP_DMABUF},
    {"socket", FDLEAK_WP_SOCKET},
    {"pipe", FDLEAK_WP_PIPE},
    {"ashmem", FDLEAK_WP_ASHMEM},
};
}
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
    if (!Publish(DumpDelayedSpSingleton<DumpManagerService>::GetInstance())) {
        DUMPER_HILOGE(MODULE_SERVICE, "error|register to system ability manager failed.");
        return;
    }
    started_ = true;
    SetCpuSchedAffinity();
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
        {
            unique_lock<mutex> lock(mutex_);
            if (requestRawParamMap_.empty()) {
                break;
            }
        }
        sleep(1);
    }
    started_ = false;
    blockRequest_ = false;
    DUMPER_HILOGD(MODULE_SERVICE, "leave|");
}

int32_t DumpManagerService::OnIdle(const SystemAbilityOnDemandReason& idleReason)
{
    DUMPER_HILOGI(MODULE_SERVICE, "on idle enter, idle reason %{public}d, %{public}s, request sum=%{public}d",
        idleReason.GetId(), idleReason.GetName().c_str(), GetRequestSum());

    if (idleReason.GetId() == OnDemandReasonId::INTERFACE_CALL) {
        if (GetRequestSum() == 0) {
            started_ = false;
            return UNLOAD_IMMEDIATELY;
        } else {
            GetIdleRequest();
            return DYNAMIC_EXIT_DELAY_TIME;
        }
    } else {
        return UNLOAD_IMMEDIATELY;
    }
}

void DumpManagerService::SetCpuSchedAffinity()
{
    int curTid = syscall(SYS_gettid);
    cpu_set_t mask;
    CPU_ZERO(&mask);
    for (int i = 0; i < SMALL_CPU_SIZE; i++) {
        CPU_SET(i, &mask);
    }
    if (sched_setaffinity(curTid, sizeof(mask), &mask) < 0) {
        DUMPER_HILOGE(MODULE_SERVICE, "error|sched_setaffinity failed");
    }
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

void DumpManagerService::HandleRequestError(std::vector<std::u16string> &args, int outfd,
    const int32_t& errorCode, const std::string& errorMsg)
{
    close(outfd);
    int callerPpid = -1;
    if (args.size() >= ARG_MIN_COUNT) {
        StrToInt(Str16ToStr8(args[args.size() - 1]), callerPpid);
        args.pop_back();
    }
    std::stringstream dumpCmdSs;
    for (size_t i = 0; i < args.size(); i++) {
        dumpCmdSs << Str16ToStr8(args[i]).c_str() << " ";
    }
    std::unique_ptr<DumperSysEventParams> param = std::make_unique<DumperSysEventParams>();
    param->errorCode = errorCode;
    param->callerPpid = callerPpid;
    param->arguments = dumpCmdSs.str();
    param->errorMsg = errorMsg;
    param->opt = "";
    param->subOpt = "";
    param->target = "";
    DumpCommonUtils::ReportCmdUsage(param);
}

int32_t DumpManagerService::Request(std::vector<std::u16string> &args, int outfd)
{
    SetCpuSchedAffinity();
    if (blockRequest_) {
        HandleRequestError(args, outfd, static_cast<int32_t>(DumpStatus::DUMP_FAIL), "request has blocked");
        return DumpStatus::DUMP_FAIL;
    }
    if (!started_) {
        DUMPER_HILOGE(MODULE_SERVICE, "hidumper_service has stopped.");
        HandleRequestError(args, outfd, static_cast<int32_t>(DumpStatus::DUMP_FAIL), "request has stopped");
        return DumpStatus::DUMP_FAIL;
    }
    int32_t uid = IPCSkeleton::GetCallingUid();
    if (!HasDumpPermission() && uid != HIPORFILER_UID) {
        DUMPER_HILOGE(MODULE_SERVICE, "No dump permission, please check!, uid:%{public}d.", uid);
        HandleRequestError(args, outfd, static_cast<int32_t>(DumpStatus::DUMP_FAIL), "no dump permission");
        return DumpStatus::DUMP_FAIL;
    }
    int sum = GetRequestSum();
    DUMPER_HILOGD(MODULE_SERVICE, "debug|sum=%{public}d", sum);
    if (sum >= REQUEST_MAX) {
        DUMPER_HILOGE(MODULE_SERVICE, "sum is greater than the request max, sum:%{public}d.", sum);
        HandleRequestError(args, outfd, static_cast<int32_t>(DumpStatus::DUMP_FAIL), "request sum reached max");
        return DumpStatus::DUMP_REQUEST_MAX;
    } else if (sum == 0) {
        DumpLogManager::Init();
    }
    DelayUnloadTask();
    DUMPER_HILOGD(MODULE_SERVICE, "enter|");
    const std::shared_ptr<RawParam> rawParam = AddRequestRawParam(args, outfd);
    int32_t ret = StartRequest(rawParam);
    DUMPER_HILOGD(MODULE_SERVICE, "leave|ret=%{public}d", ret);
    return ret;
}

// Authenticate dump permissions
bool DumpManagerService::HasDumpPermission() const
{
    uint32_t callingTokenID = IPCSkeleton::GetCallingTokenID();
    int res = Security::AccessToken::AccessTokenKit::VerifyAccessToken(callingTokenID, "ohos.permission.DUMP");
    if (res != Security::AccessToken::PermissionState::PERMISSION_GRANTED) {
        DUMPER_HILOGI(MODULE_SERVICE, "No dump permission, please check!");
        return false;
    }
    return true;
}

uint32_t DumpManagerService::GetFileDescriptorNums(int32_t pid, std::string requestType) const
{
    if (requestType.find("..") != std::string::npos) {
        DUMPER_HILOGE(MODULE_SERVICE, "requestType is invalid, please check!");
        return 0;
    }
    std::string taskPath = "/proc/" + std::to_string(pid) + "/" + requestType;
    std::vector<std::string> fdList = DumpCommonUtils::GetSubNodes(taskPath, true);
    return fdList.size();
}

int32_t DumpManagerService::ScanPidOverLimit(std::string requestType, int32_t limitSize, std::vector<int32_t> &pidList)
{
    if (!HasDumpPermission()) {
        return DumpStatus::DUMP_FAIL;
    }
    if (limitSize < 0) {
        return DumpStatus::DUMP_FAIL;
    }
    int32_t ret = DumpStatus::DUMP_OK;
    std::vector<int32_t> pids = DumpCommonUtils::GetAllPids();
    for (const auto &pid : pids) {
        uint32_t num = GetFileDescriptorNums(pid, requestType);
        if (num < static_cast<uint32_t>(limitSize)) {
            continue;
        }
        auto it = std::find(pidList.begin(), pidList.end(), pid);
        if (it != pidList.end()) {
            continue;
        }
        pidList.push_back(pid);
    }
    return ret;
}

std::string DumpManagerService::GetFdLinkNum(const std::string &linkPath) const
{
    char linkDest[PATH_MAX] = {0};
    ssize_t linkDestSize = readlink(linkPath.c_str(), linkDest, sizeof(linkDest) - 1);
    if (linkDestSize < 0) {
        return "unknown";
    }
    linkDest[linkDestSize] = '\0';
    return linkDest;
}

void DumpManagerService::RecordDetailFdInfo(std::string &detailFdInfo, std::string &topLeakedType)
{
    lock_guard<mutex> lock(linkCntMutex_);
    if (linkCnt_.empty()) {
        DUMPER_HILOGE(MODULE_SERVICE, "linkCnt_ is empty!");
        return;
    }
    topLeakedType = linkCnt_[0].first;
    for (size_t i = 0; i < linkCnt_.size() && i < FD_LOG_NUM; i++) {
        detailFdInfo += std::to_string(linkCnt_[i].second) + "\t" + linkCnt_[i].first + "\n";
    }
}

void DumpManagerService::RecordDirFdInfo(std::string &detailFdInfo)
{
    std::unordered_map<std::string, int> fileTypeMap;
    std::vector<pair<std::string, int>> fileTypeList;
    {
        lock_guard<mutex> lock(linkCntMutex_);
        for (const auto &each : linkCnt_) {
            if (g_fdLeakWp.find(each.first) == g_fdLeakWp.end()) {
                std::string fileName(each.first, 0, DumpCommonUtils::FindStorageDirSecondDigitIndex(each.first));
                if (fileTypeMap.find(fileName) == fileTypeMap.end()) {
                    fileTypeMap[fileName] = each.second;
                } else {
                    fileTypeMap[fileName] += each.second;
                }
            }
        }
    }
    for (std::pair<std::string, int> fileNamePair : fileTypeMap) {
        fileTypeList.push_back(fileNamePair);
    }
    sort(fileTypeList.begin(), fileTypeList.end(),
        [](const std::pair<std::string, int> &p1, const std::pair<std::string, int> &p2) {
            return p1.second > p2.second;
    });
    detailFdInfo += "\nTop Dir Type 10:\n";
    for (size_t i = 0; i < fileTypeList.size() && i < FD_LOG_NUM; i++) {
        detailFdInfo += std::to_string(fileTypeList[i].second) + "\t" + fileTypeList[i].first + "\n";
    }
}

int32_t DumpManagerService::CountFdNums(int32_t pid, uint32_t &fdNums,
    std::string &detailFdInfo, std::string &topLeakedType)
{
    if (!HasDumpPermission()) {
        return DumpStatus::DUMP_FAIL;
    }
    // transfor to vector to sort by map value.
    int32_t ret = DumpStatus::DUMP_OK;
    std::map<std::string, int64_t> linkNameCnt;
    {
        lock_guard<mutex> lock(linkCntMutex_);
        if (!linkCnt_.empty()) {
            linkCnt_.clear();
        }
    }
    std::string taskPath = "/proc/" + std::to_string(pid) + "/fd";
    std::vector<std::string> fdList = DumpCommonUtils::GetSubNodes(taskPath, true);
    fdNums = GetFileDescriptorNums(pid, "fd");
    for (const auto &each : fdList) {
        std::string linkPath = taskPath + "/" + each;
        std::string linkName = GetFdLinkNum(linkPath);
        // we count the fd number by name contained the keywords socket/dmabuf...
        bool contained = false;
        for (const auto &fdWp : g_fdLeakWp) {
            if (linkName.find(fdWp.first) != std::string::npos) {
                linkNameCnt[fdWp.first]++;
                contained = true;
                break;
            }
        }
        if (!contained) {
            linkNameCnt[linkName]++;
        }
    }
    {
        lock_guard<mutex> lock(linkCntMutex_);
        for (const auto &each : linkNameCnt) {
            linkCnt_.push_back(each);
        }
        if (linkCnt_.empty()) {
            return DumpStatus::DUMP_FAIL;
        }
        std::sort(linkCnt_.begin(), linkCnt_.end(),
            [](const std::pair<std::string, int> &a, const std::pair<std::string, int> &b) {
                return a.second > b.second;
            });
    }
    RecordDetailFdInfo(detailFdInfo, topLeakedType);
    RecordDirFdInfo(detailFdInfo);
    return ret;
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
        handler_ = std::make_shared<AppExecFwk::EventHandler>(eventRunner_);
        if (handler_ == nullptr) {
            DUMPER_HILOGE(MODULE_SERVICE, "error|create EventHandler");
            return false;
        }
    }
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

void DumpManagerService::GetIdleRequest()
{
    unique_lock<mutex> lock(mutex_);
    for (auto &requestIt : requestRawParamMap_) {
        if (requestIt.second == nullptr) {
            continue;
        }
        int argC = requestIt.second->GetArgc();
        char **argV = requestIt.second->GetArgv();
        if (argV == nullptr) {
            continue;
        }
        std::stringstream dumpCmdSs;
        for (int i = 0; i < argC; i++) {
            dumpCmdSs << std::string(argV[i]) << " ";
        }
        DUMPER_HILOGI(MODULE_SERVICE, "idle cmd:%{public}s, calllingUid=%{public}d, calllingPid=%{public}d.",
            dumpCmdSs.str().c_str(), requestIt.second->GetUid(), requestIt.second->GetPid());
    }
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

void DumpManagerService::DelayUnloadTask()
{
    int32_t calllingPid = IPCSkeleton::GetCallingPid();
    DUMPER_HILOGI(MODULE_SERVICE, "recieve new request, delay unload task begin, calllingPid=%{public}d", calllingPid);
    auto task = [this]() {
        DUMPER_HILOGI(MODULE_SERVICE, "do unload task, request sum=%{public}d", GetRequestSum());
        if (GetRequestSum() != 0) {
            GetIdleRequest();
            return;
        }
        auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (samgrProxy == nullptr) {
            DUMPER_HILOGE(MODULE_SERVICE, "get samgr failed");
            return;
        }
        int32_t ret = samgrProxy->UnloadSystemAbility(DFX_SYS_HIDUMPER_ABILITY_ID);
        if (ret != ERR_OK) {
            DUMPER_HILOGE(MODULE_SERVICE, "remove system ability failed");
            return;
        }
    };
    handler_->RemoveTask(TASK_ID);
    handler_->PostTask(task, TASK_ID, DYNAMIC_EXIT_DELAY_TIME);
}
} // namespace HiviewDFX
} // namespace OHOS
