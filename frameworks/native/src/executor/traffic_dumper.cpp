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

#include "executor/traffic_dumper.h"
#ifdef HIDUMPER_ABILITY_RUNTIME_ENABLE
#include "net_stats_client.h"
#endif
#include "dump_common_utils.h"

namespace OHOS {
namespace HiviewDFX {
static const std::string RECEIVED_BYTES = "Received Bytes:";
static const std::string SEND_BYTES = "Sent Bytes:";

TrafficDumper::TrafficDumper()
{}

TrafficDumper::~TrafficDumper()
{}

DumpStatus TrafficDumper::PreExecute(const std::shared_ptr<DumperParameter> &parameter, StringMatrix dumpDatas)
{
    pid_ = parameter->GetOpts().netPid_;
    result_ = dumpDatas;
    return DumpStatus::DUMP_OK;
}

DumpStatus TrafficDumper::Execute()
{
    if (result_ != nullptr) {
        pid_ >= 0 ? GetApplicationUidBytes() : GetAllBytes();
    }
    return status_;
}

DumpStatus TrafficDumper::AfterExecute()
{
    return DumpStatus::DUMP_OK;
}

void TrafficDumper::GetAllBytes()
{
    DUMPER_HILOGD(MODULE_SERVICE, "debug|GetAllRxBytes.\n");
    uint64_t receivedStats = 0;
#ifdef HIDUMPER_ABILITY_RUNTIME_ENABLE
    int32_t ret = DelayedSingleton<NetManagerStandard::NetStatsClient>::GetInstance()->GetAllRxBytes(receivedStats);
    if (ret != NetManagerStandard::NETMANAGER_SUCCESS) {
        DUMPER_HILOGE(MODULE_SERVICE, "GetAllRxBytes failed, ret:%{public}d.\n", ret);
        status_ = DumpStatus::DUMP_FAIL;
    }
#endif
    std::vector<std::string> line_vector;
    line_vector.push_back(RECEIVED_BYTES + std::to_string(receivedStats));
    result_->push_back(line_vector);
#ifdef HIDUMPER_ABILITY_RUNTIME_ENABLE
    uint64_t sendStats = 0;
    ret = DelayedSingleton<NetManagerStandard::NetStatsClient>::GetInstance()->GetAllTxBytes(sendStats);
    if (ret != NetManagerStandard::NETMANAGER_SUCCESS) {
        DUMPER_HILOGE(MODULE_SERVICE, "GetAllRxBytes failed, ret:%{public}d.\n", ret);
        status_ = DumpStatus::DUMP_FAIL;
    }
#endif
    line_vector.clear();
    line_vector.push_back(SEND_BYTES + std::to_string(sendStats));
    result_->push_back(line_vector);
    status_ = DumpStatus::DUMP_OK;
}

void TrafficDumper::GetApplicationUidBytes()
{
    DUMPER_HILOGD(MODULE_SERVICE, "debug|GetApplicationUidBytes.\n");
    DumpCommonUtils::PidInfo currentPidInfo;
    currentPidInfo.pid_ = pid_;
    currentPidInfo.uid_ = -1;
    if (!DumpCommonUtils::GetProcessInfo(pid_, currentPidInfo)) {
        DUMPER_HILOGE(MODULE_SERVICE, "GetProcUid failed, pid:%{public}d, uid:%{public}d.\n",
            pid_, currentPidInfo.uid_);
        return;
    }
    uint64_t receivedStats = 0;
#ifdef HIDUMPER_ABILITY_RUNTIME_ENABLE
    int32_t ret = DelayedSingleton<NetManagerStandard::NetStatsClient>::GetInstance()->GetUidRxBytes(
        receivedStats, static_cast<uint32_t>(currentPidInfo.uid_));
    if (ret != NetManagerStandard::NETMANAGER_SUCCESS) {
        DUMPER_HILOGE(MODULE_SERVICE, "GetUidRxBytes failed, ret:%{public}d, uid:%{public}d.\n",
            ret, currentPidInfo.uid_);
        status_ = DumpStatus::DUMP_FAIL;
    }
#endif
    std::vector<std::string> line_vector;
    line_vector.push_back(RECEIVED_BYTES + std::to_string(receivedStats));
    result_->push_back(line_vector);

    uint64_t sendStats = 0;
#ifdef HIDUMPER_ABILITY_RUNTIME_ENABLE
    ret = DelayedSingleton<NetManagerStandard::NetStatsClient>::GetInstance()->GetUidTxBytes(
        sendStats, static_cast<uint32_t>(currentPidInfo.uid_));
    if (ret != NetManagerStandard::NETMANAGER_SUCCESS) {
        DUMPER_HILOGE(MODULE_SERVICE, "GetUidTxBytes failed, ret:%{public}d, uid:%{public}d.\n",
            ret, currentPidInfo.uid_);
        status_ = DumpStatus::DUMP_FAIL;
    }
#endif
    line_vector.clear();
    line_vector.push_back(SEND_BYTES + std::to_string(sendStats));
    result_->push_back(line_vector);
    status_ = DumpStatus::DUMP_OK;
    DUMPER_HILOGD(MODULE_SERVICE, "debug|GetApplicationUidBytes end, pid:%{public}d, uid:%{public}d.\n",
        pid_, currentPidInfo.uid_);
}
}  // namespace HiviewDFX
}  // namespace OHOS
