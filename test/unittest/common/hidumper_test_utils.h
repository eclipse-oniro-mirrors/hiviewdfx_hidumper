/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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
#ifndef HIDUMPER_TEST_UTILS_H
#define HIDUMPER_TEST_UTILS_H
#include <string>
#include "singleton.h"
namespace OHOS {
namespace HiviewDFX {
class HidumperTestUtils : public Singleton<HidumperTestUtils> {
public:
    HidumperTestUtils();
    ~HidumperTestUtils();

    HidumperTestUtils(HidumperTestUtils const &) = delete;
    void operator=(HidumperTestUtils const &) = delete;
    bool IsExistInCmdResult(const std::string &cmd, const std::string &str);
    bool IsExistStrInFile(const std::string &cmd, const std::string &str, const std::string &filePath);
    pid_t GetPidByName(const std::string& processName);
    bool GetSpecialLine(const std::string &cmd, const std::string &str, std::string &specialLine);
    std::string GetValueInLine(const std::string &line, int index);
};
} // namespace HiviewDFX
} // namespace OHOS
#endif
