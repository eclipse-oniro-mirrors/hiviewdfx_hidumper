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
#ifndef HIVIEWDFX_HIDUMPER_CMD_PARSE_H
#define HIVIEWDFX_HIDUMPER_CMD_PARSE_H

#include <getopt.h>
#include <string>
#include <vector>
#include <memory>

#include "common.h"
#include "dump_context.h"
#include "singleton.h"

namespace OHOS {
namespace HiviewDFX {
class CmdParse final : public DelayedRefSingleton<CmdParse> {
    DECLARE_DELAYED_REF_SINGLETON(CmdParse)
public:
    DISALLOW_COPY_AND_MOVE(CmdParse);
    DumpStatus Parse(int argc, char *argv[], std::shared_ptr<DumpContext> dumpContext);
private:
    DumpStatus CheckArgs(int argc, char* argv[]);
    DumpStatus CmdParseHelper(int argc, char *argv[], std::shared_ptr<DumpContext> dumpContext);
    DumpStatus ParseLongCmdOption(int argc, std::shared_ptr<DumpContext> dumpContext, const struct option longOptions[],
                                  const int &optionIndex, char *argv[]);
    bool ParseSubLongCmdOption(int argc, std::shared_ptr<DumpContext> dumpContext, const struct option longOptions[],
                               const int &optionIndex, char *argv[]);
    void CheckIncorrectCmdOption(const char *optStr, char *argv[], std::shared_ptr<DumpContext> dumpContext);
    DumpStatus ParseShortCmdOption(int c, std::shared_ptr<DumpContext> dumpContext, int argc, char *argv[]);
    bool IsShortOptionReqArg(const char *optStr, std::shared_ptr<DumpContext> dumpContext);
    DumpStatus SetCmdParameter(int argc, char *argv[], std::shared_ptr<DumpContext> dumpContext);
    DumpStatus SetCmdIntegerParameter(const std::string &str, int &value, std::shared_ptr<DumpContext> dumpContext);
    DumpStatus SetMemJsheapParam(std::shared_ptr<DumpContext> dumpContext);
    DumpStatus SetRawParam(std::shared_ptr<DumpContext> dumpContext);
    bool SetIpcStatParam(std::shared_ptr<DumpContext> dumpContext, const std::string& param);
    std::string RemoveCharacterFromStr(const std::string &str, const char character);
    DumpStatus ParseCmdOptionForA(std::shared_ptr<DumpContext> dumpContext, char *argv[]);
    DumpStatus CheckProcessAlive(std::shared_ptr<DumpContext> dumpContext);
    bool IsSADumperOption(char *argv[]);
    DumpStatus VerifyOptions(const std::shared_ptr<DumpContext>& dumpContext);
};
}
}

#endif
