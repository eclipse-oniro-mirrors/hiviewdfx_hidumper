/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "dump_client_main.h"

#include <ipc_skeleton.h>
#include <sstream>
#include <string_ex.h>
#include <vector>

#include "common.h"
#include "dump_controller.h"
#include "dump_manager_client.h"
#include "dump_utils.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace HiviewDFX {
DumpClientMain::DumpClientMain()
{
}

DumpClientMain::~DumpClientMain()
{
}

int DumpClientMain::Main(int argc, char* argv[], int outFd)
{
    if (argc > ARG_MAX_COUNT) {
        LOG_ERR("too many arguments(%d), limit size %d.\n", argc, ARG_MAX_COUNT);
        return DumpStatus::DUMP_INVALID_ARG;
    }
    if (argv == nullptr) {
        LOG_ERR("argument is null.\n");
        return DumpStatus::DUMP_INVALID_ARG;
    }
    for (int i = 0; i < argc; i++) {
        if (argv[i] == nullptr) {
            LOG_ERR("argument(%d) is null.\n", i);
            return DumpStatus::DUMP_INVALID_ARG;
        }
        size_t len = strlen(argv[i]);
        if (len == 0) {
            LOG_ERR("argument(%d) is empty.\n", i);
            return DumpStatus::DUMP_INVALID_ARG;
        }
        if (len > SINGLE_ARG_MAXLEN) {
            LOG_ERR("too long argument(%d), limit size %d.\n", i, SINGLE_ARG_MAXLEN);
            return DumpStatus::DUMP_INVALID_ARG;
        }
    }
    std::vector<std::u16string> args;
    SetCmdArgs(argc, argv, args);
    auto& dumpManagerClient = DumpManagerClient::GetInstance();
    if (dumpManagerClient.Connect() != ERR_OK) {
        (void)dprintf(outFd, "connect error\n");
        DUMPER_HILOGE(MODULE_SERVICE, "connect error.");
        return -1;
    }
    DumpUtils::IgnoreStdoutCache();
    int32_t ret = dumpManagerClient.Request(args, outFd);
    if (ret < DumpStatus::DUMP_OK) {
        if (ret != DumpStatus::DUMP_INVALID_ARG) {
            (void)dprintf(outFd, "request error\n");
            DUMPER_HILOGE(MODULE_SERVICE, "request error, ret: %{public}d.", ret);
        }
        return ret;
    }
    if (!dumpManagerClient.IsConnected()) {
        (void)dprintf(outFd, "service error\n");
        DUMPER_HILOGE(MODULE_SERVICE, "service error.");
    }
    return ret;
}

void DumpClientMain::SetCmdArgs(int argc, char* argv[], std::vector<std::u16string>& args)
{
    std::stringstream dumpCmdSs;
    for (int i = 0; i < argc; i++) {
        args.push_back(Str8ToStr16(std::string(argv[i])));
        dumpCmdSs << std::string(argv[i]) << " ";
    }
    std::string ppid = std::to_string(getppid());
    args.push_back(Str8ToStr16(ppid));
    int32_t calllingUid = IPCSkeleton::GetCallingUid();
    int32_t calllingPid = IPCSkeleton::GetCallingPid();
    DUMPER_HILOGI(MODULE_SERVICE, "hidumper cmd:%{public}s, calllingUid=%{public}d, calllingPid=%{public}d,"
        " ppid:%{public}s.", dumpCmdSs.str().c_str(), calllingUid, calllingPid, ppid.c_str());
}
} // namespace HiviewDFX
} // namespace OHOS
