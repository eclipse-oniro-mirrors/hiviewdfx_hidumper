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

#ifndef HIVIEWDFX_HIDUMPER_WRITER_UTILS_H
#define HIVIEWDFX_HIDUMPER_WRITER_UTILS_H

#include <string>
#include <vector>
#include <memory>

namespace OHOS {
namespace HiviewDFX {
using DataHandler = std::function<bool(const std::string& content)>;

void WriteStringIntoFd(const std::string& str, int fd);
void WriteStringIntoFd(const std::vector<std::string>& strs, int fd);
void WriteTitle(const std::string& title, int fd);
bool HandleStringFromFile(const std::string& path, const DataHandler& func);
bool HandleStringFromCommand(const std::string& command, const DataHandler& func);
bool LoadStringFromFile(const std::string& path, std::string& content);
}
}
#endif