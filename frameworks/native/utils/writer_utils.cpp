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
#include <fstream>
#include <string>
#include <sstream>

#include "file_ex.h"
#include "hilog_wrapper.h"


namespace OHOS {
namespace HiviewDFX {

void WriteStringIntoFd(const std::string& str, int fd)
{
    if (fd == -1) {
        DUMPER_HILOGE(MODULE_COMMON, "Failed to get output fd");
        return;
    }
    if (str.empty()) {
        DUMPER_HILOGW(MODULE_COMMON, "str is empty");
        return;
    }
    std::string outputStr = str;
    if (outputStr.back() != '\n') {
        outputStr += '\n';
    }
    SaveStringToFd(fd, outputStr);
}

void WriteStringIntoFd(const std::vector<std::string>& strs, int fd)
{
    if (fd == -1) {
        DUMPER_HILOGE(MODULE_COMMON, "Failed to get output fd");
        return;
    }
    if (strs.empty()) {
        DUMPER_HILOGW(MODULE_COMMON, "strs is empty");
        return;
    }
    for (const auto& str : strs) {
        WriteStringIntoFd(str, fd);
    }
}

bool HandleStringFromFile(const std::string& path, const DataHandler& func)
{
    char canonicalPath[PATH_MAX] = {0};
    if (realpath(path.c_str(), canonicalPath) == nullptr) {
        DUMPER_HILOGE(MODULE_COMMON, "realpath failed, errno=%{public}d, path=%{public}s", errno, path.c_str());
        return false;
    }
    std::ifstream file(canonicalPath);
    if (!file.is_open()) {
        DUMPER_HILOGE(MODULE_COMMON, "Failed to open file, path=%{public}s", path.c_str());
        return false;
    }
    std::string line;
    while (std::getline(file, line)) {
        line += '\n';
        if (!func(line)) {
            break;
        }
    }
    return true;
}

bool HandleStringFromCommand(const std::string& command, const DataHandler& func)
{
    auto pipe = std::unique_ptr<FILE, decltype(&pclose)>{popen(command.c_str(), "r"), pclose};
    if (pipe == nullptr) {
        DUMPER_HILOGE(MODULE_COMMON, "popen failed, errno=%{public}d, command=%{public}s", errno, command.c_str());
        return false;
    }
    char *lineBuf = nullptr;
    ssize_t lineLen;
    size_t lineAlloc = 0;
    while ((lineLen = getline(&lineBuf, &lineAlloc, pipe.get())) > 0) {
        lineBuf[lineLen] = '\0';
        const std::string content = lineBuf;
        if (!func(content)) {
            break;
        }
    }
    if (lineBuf != nullptr) {
        free(lineBuf);
        lineBuf = nullptr;
    }
    return true;
}

bool LoadStringFromFile(const std::string& path, std::string& content)
{
    return OHOS::LoadStringFromFile(path, content);
}

void WriteTitle(const std::string& title, int fd)
{
    WriteStringIntoFd("\n", fd);
    WriteStringIntoFd(title, fd);
    WriteStringIntoFd("\n", fd);
}
}
}
