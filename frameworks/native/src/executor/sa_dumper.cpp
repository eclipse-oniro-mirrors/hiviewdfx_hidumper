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
#include "executor/sa_dumper.h"
#include <cstdio>
#include <sstream>
#include <thread>
#include <unistd.h>
#include "dump_utils.h"
#include "file_ex.h"
#include "securec.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
static const std::string SEPARATOR_TEMPLATE = "----------------------------------";
static const std::string ABILITY_LINE = "-------------------------------[ability]-------------------------------";
const std::string LOG_TXT = "log.txt";
using StringMatrix = std::shared_ptr<std::vector<std::vector<std::string>>>;

} // namespace

SADumper::SADumper(void)
{
}

SADumper::~SADumper(void)
{
}

DumpStatus SADumper::PreExecute(const std::shared_ptr<DumperParameter> &parameter, StringMatrix dump_datas)
{
    result_ = dump_datas;
    names_ = ptrDumpCfg_->args_->GetNameList();
    bool isZip = parameter->GetOpts().IsDumpZip();
    auto callback = parameter->getClientCallback();
    if (callback == nullptr) {
        DUMPER_HILOGE(MODULE_COMMON, "PreExecute error|callback is nullptr");
        return DumpStatus::DUMP_FAIL;
    }
    std::string logDefaultPath_ = callback->GetFolder() + LOG_TXT;
    if (isZip) {
        outputFd_ = DumpUtils::FdToWrite(logDefaultPath_);
    } else {
        outputFd_ = parameter->getClientCallback()->GetOutputFd();
    }

    StringVector args = ptrDumpCfg_->args_->GetArgList();
    if (!args.empty() && names_.size() == 1) {
        std::transform(args.begin(), args.end(), std::back_inserter(args_), Str8ToStr16);
        std::stringstream argsStr;
        for (std::string arg : args) {
            argsStr << arg << " ";
        }
        argsStr_ = argsStr.str();
    }
    return DumpStatus::DUMP_OK;
}

DumpStatus SADumper::GetData(const std::string &name, const sptr<ISystemAbilityManager> &sam)
{
    int id = DumpUtils::StrToId(name);
    if (id == -1) {
        DUMPER_HILOGE(MODULE_SERVICE, "no such ability id '%{public}s'\n", name.c_str());
        return DumpStatus::DUMP_FAIL;
    }
    sptr<IRemoteObject> sa = sam->CheckSystemAbility(id);
    if (sa == nullptr) {
        DUMPER_HILOGE(MODULE_SERVICE, "no such system ability %{public}s\n", name.c_str());
        return DumpStatus::DUMP_FAIL;
    }
    std::stringstream ss;
    ss << SEPARATOR_TEMPLATE << DumpUtils::ConvertSaIdToSaName(name) << SEPARATOR_TEMPLATE;
    std::lock_guard<std::mutex> lock(mutex_);
    SaveStringToFd(outputFd_, "\n" + ABILITY_LINE + "\n");
    SaveStringToFd(outputFd_, "\n\n" + ss.str() + "\n");
    int result = sa->Dump(outputFd_, args_);
    if (result != ERR_OK) {
        DUMPER_HILOGE(MODULE_SERVICE, "system ability:%{public}s dump fail!ret:%{public}d\n", name.c_str(), result);
    }
    DUMPER_HILOGI(MODULE_COMMON, "SA name:%{public}s dump success, cmd:%{public}s!", name.c_str(), argsStr_.c_str());
    return DumpStatus::DUMP_OK;
}

DumpStatus SADumper::Execute()
{
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        DUMPER_HILOGE(MODULE_SERVICE, "get samgr fail!");
        return DumpStatus::DUMP_FAIL;
    }
    if (names_.empty()) {
        U16StringVector vct = sam->ListSystemAbilities();
        std::transform(vct.begin(), vct.end(), std::back_inserter(names_), Str16ToStr8);
    }
    for (size_t i = 0; i < names_.size(); ++i) {
        if (GetData(names_[i], sam) != DumpStatus::DUMP_OK) {
            DUMPER_HILOGI(MODULE_SERVICE, "system ability:%{public}s execute fail!\n", names_[i].c_str());
        }
    }
    return DumpStatus::DUMP_OK;
}

DumpStatus SADumper::AfterExecute()
{
    return DumpStatus::DUMP_OK;
}
} // namespace HiviewDFX
} // namespace OHOS
