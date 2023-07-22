/*
* Copyright (C) 2021 Huawei Device Co., Ltd.
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#include <unistd.h>
#include <sys/stat.h>
#include "util/file_utils.h"
using namespace std;
namespace OHOS {
namespace HiviewDFX {
FileUtils::FileUtils()
{
}
FileUtils::~FileUtils()
{
}

bool FileUtils::CreateFolder(const string &path)
{
    if (!access(path.c_str(), F_OK) || path == "") {
        return true;
    }

    size_t pos = path.rfind("/");
    if (pos == string::npos) {
        return false;
    }

    string upperPath = path.substr(0, pos);
    if (CreateFolder(upperPath)) {
        if (mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IRWXO)) {
            if (errno != EEXIST) {
                return false;
            }
        }
        return true;
    }
    return false;
}

bool FileUtils::LoadStringFromProc(const std::string& path, std::string& content, bool oneLine)
{
    auto fp = std::unique_ptr<FILE, decltype(&fclose)>{fopen(path.c_str(), "re"), fclose};
    if (fp == nullptr) {
        return false;
    }
    char *line = nullptr;
    ssize_t lineLen;
    size_t lineAlloc = 0;
    while ((lineLen = getline(&line, &lineAlloc, fp.get())) > 0) {
        line[lineLen] = '\0';
        content += line;
        if (oneLine) {
            break;
        }
    }
    if (line != nullptr) {
        free(line);
        line = nullptr;
    }
    return true;
}

} // namespace HiviewDFX
} // namespace OHOS