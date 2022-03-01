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
#ifndef FILE_UTILS_H
#define FILE_UTILS_H
#include <string>
#include "singleton.h"
namespace OHOS {
namespace HiviewDFX {
class FileUtils : public Singleton<FileUtils> {
public:
    FileUtils();
    ~FileUtils();
    FileUtils(FileUtils const &) = delete;
    void operator=(FileUtils const &) = delete;

    bool CreateFolder(const std::string &path);

private:
};
} // namespace HiviewDFX
} // namespace OHOS

#endif