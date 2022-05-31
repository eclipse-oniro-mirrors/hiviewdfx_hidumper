/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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

#include "executor/memory/get_cma_info.h"

#include <cstdlib>

#include "executor/cmd_dumper.h"
#include "executor/memory/memory_util.h"
#include "util/string_utils.h"
#include "hilog_wrapper.h"

using namespace std;
namespace OHOS {
namespace HiviewDFX {
GetCMAInfo::GetCMAInfo()
{
}

GetCMAInfo::~GetCMAInfo()
{
}

bool GetCMAInfo::GetUsed(uint64_t &value)
{
    string cmd = "cat /sys/kernel/debug/cma/*/used";
    vector<string> usedInfos;
    if (MemoryUtil::GetInstance().RunCMD(cmd, usedInfos)) {
        const int base = 10;
        for (size_t i = 0; i < usedInfos.size(); i++) {
            string used = usedInfos.at(i);
            if (StringUtils::GetInstance().IsNum(used)) {
                value += strtoull(usedInfos.at(i).c_str(), nullptr, base);
            } else {
                DUMPER_HILOGE(MODULE_SERVICE, "CMA Get Used Data error\n");
                return false;
            }
        }
    } else {
        DUMPER_HILOGE(MODULE_SERVICE, "Get CMA used by run cmd failed\n");
    }
    return true;
}
} // namespace HiviewDFX
} // namespace OHOS
