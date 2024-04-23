/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
#ifndef IPC_STAT_DUMPER_H
#define IPC_STAT_DUMPER_H

#include "hidumper_executor.h"
#include "raw_param.h"
#include <iservice_registry.h>

namespace OHOS {
namespace HiviewDFX {
class IPCStatDumper : public HidumperExecutor {
public:
    IPCStatDumper();
    ~IPCStatDumper();
    DumpStatus PreExecute(const std::shared_ptr<DumperParameter> &parameter, StringMatrix dumpDatas) override;
    DumpStatus Execute() override;
    DumpStatus AfterExecute() override;
    using U16StringVector = std::vector<std::u16string>;

private:
    StringMatrix result_;
    int pid_ = 0;
    bool isDumpIpc_ = false;
    bool isDumpAllIpc_ = false;
    bool isDumpIpcStartStat_ = false;
    bool isDumpIpcStopStat_ = false;
    bool isDumpIpcStat_ = false;
    std::shared_ptr<RawParam> ptrReqCtl_;

    U16StringVector ipcDumpCmd_;
    std::mutex mutex_;

    void SendErrorMessage(const std::string &errorStr);
    DumpStatus SetIpcStatCmd(const sptr<ISystemAbilityManager> &sam, sptr<IRemoteObject> &sa);
    DumpStatus DumpIpcStat(const sptr<ISystemAbilityManager> &sam, sptr<IRemoteObject> &sa);
    DumpStatus DoDumpIpcStat(sptr<IRemoteObject> &sa);
    bool CheckPidIsSa(const sptr<ISystemAbilityManager> &sam, std::vector<std::string> &args,
                      sptr<IRemoteObject> &sa);
    bool CheckPidIsApp(const sptr<ISystemAbilityManager> &sam, std::vector<std::string> &args,
                       sptr<IRemoteObject> &sa);
};
}  // namespace HiviewDFX
}  // namespace OHOS
#endif  // IPC_STAT_DUMPER_H
