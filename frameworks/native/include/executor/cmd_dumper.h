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
#ifndef CMD_DUMPER_H
#define CMD_DUMPER_H

#include "hidumper_executor.h"

namespace OHOS {
namespace HiviewDFX {
class CMDDumper : public HidumperExecutor {
public:
    CMDDumper();
    ~CMDDumper();
    DumpStatus PreExecute(const std::shared_ptr<DumperParameter>& parameter,
        StringMatrix dumpDatas) override;
    DumpStatus Execute() override;
    DumpStatus AfterExecute() override;
    DumpStatus GetLineData(FILE* fp, StringMatrix dumpDatas);
    DumpStatus GetCmdInterface(const std::string& cmd, StringMatrix dumpDatas);

private:
    DumpStatus ReadLine();
    void ReplacePidInCmd(std::string& cmd, int pid);
    void ReplaceCpuIdInCmd(std::string &cmd, int cpuId);

private:
    std::string cmd_ = "";
    StringMatrix dumpDatas_;
    FILE *fp_;
    std::vector<std::string> lineData_;

    // MoreData Flag
    bool moreData_ = false;
    bool needLoop_ = false;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // CMD_DUMPER_H
