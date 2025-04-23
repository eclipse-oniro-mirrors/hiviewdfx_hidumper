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
#include "writer_utils.h"
#include <unistd.h>
#include <fcntl.h>
#include "common/dumper_parameter.h"
#include "dump_utils.h"
#include "hilog_wrapper.h"


namespace OHOS {
namespace HiviewDFX {

void WriteStringIntoFd(const std::string& str, const std::shared_ptr<DumperParameter>& parameter)
{
    if (parameter == nullptr) {
        return;
    }
    int fd = parameter->getClientCallback()->GetOutputFd();
    std::string strWithNewline = str + "\n";
    if (write(fd, strWithNewline.c_str(), strlen(strWithNewline.c_str())) == -1) {
        DUMPER_HILOGE(MODULE_COMMON, "write to fd failed, errno: %{public}d", errno);
    }
}

void WriteStringIntoFd(const std::vector<std::string>& strs, const std::shared_ptr<DumperParameter>& parameter)
{
    if (parameter == nullptr) {
        return;
    }
    int fd = parameter->getClientCallback()->GetOutputFd();
    for (const auto& str : strs) {
        std::string strWithNewline = str + "\n";
        if (write(fd, strWithNewline.c_str(), strWithNewline.length()) == -1) {
            DUMPER_HILOGE(MODULE_COMMON, "write to fd failed, errno: %{public}d", errno);
        }
    }
}
}
}
