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
#ifndef STRING_UTILS_H
#define STRING_UTILS_H
#include <string>
#include <vector>
#include "singleton.h"
namespace OHOS {
namespace HiviewDFX {
class StringUtils : public Singleton<StringUtils> {
public:
    StringUtils();
    ~StringUtils();
    StringUtils(StringUtils const &) = delete;
    void operator=(StringUtils const &) = delete;

    void StringSplit(const std::string &str, const std::string &split, std::vector<std::string> &result);
    bool IsBegin(const std::string &content, const std::string &begin);
    bool IsEnd(const std::string &content, const std::string &end);
    bool IsContain(const std::string &content, const std::string &contain);
    bool IsSameStr(const std::string &first, const std::string &second);
    void ReplaceAll(std::string &str, const std::string &oldValue, const std::string &newValue);
    bool IsNum(std::string str);
    bool Compare(const std::string &str, const std::vector<std::string> &strs);
    void HexToDec(const std::string &str, uint64_t &value);
    void SetWidth(const int &width, const char &fileStr, const bool &left, std::string &str);
    char GetBlank();
    char GetSeparator();

private:
    const int HEX_STR = 16;
    const int DEC_STR = 10;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif