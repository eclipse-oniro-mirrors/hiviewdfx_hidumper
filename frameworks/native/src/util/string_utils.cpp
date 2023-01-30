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
#include "util/string_utils.h"
#include <cmath>
#include <sstream>
#include <iomanip>
#include "string_ex.h"
using namespace std;
namespace OHOS {
namespace HiviewDFX {
StringUtils::StringUtils()
{
}

StringUtils::~StringUtils()
{
}

void StringUtils::StringSplit(const string &content, const string &split, vector<string> &result)
{
    SplitStr(content, split, result, false, false);
}

bool StringUtils::IsBegin(const string &content, const string &begin)
{
    if (content.find(begin) == 0) {
        return true;
    }
    return false;
}

bool StringUtils::IsEnd(const string &content, const string &end)
{
    bool result = false;
    if (content.length() >= end.length()) {
        result = (0 == content.compare(content.length() - end.length(), end.length(), end));
    }
    return result;
}

bool StringUtils::IsContain(const string &content, const string &contain)
{
    return IsSubStr(content, contain);
}

bool StringUtils::IsSameStr(const string &first, const string &second)
{
    if (first == second) {
        return true;
    }
    return false;
}

void StringUtils::ReplaceAll(string &str, const string &oldValue, const string &newValue)
{
    str = ReplaceStr(str, oldValue, newValue);
}

bool StringUtils::IsNum(string str)
{
    return IsNumericStr(str);
}

bool StringUtils::Compare(const string &str, const vector<string> &strs)
{
    bool success = false;
    for (const string &tempStr : strs) {
        if (str == tempStr) {
            success = true;
        }
    }
    return success;
}

void StringUtils::HexToDec(const string &str, uint64_t &value)
{
    size_t l = str.length();
    for (size_t i = 0; i < l; i++) {
        if (str[i] >= '0' && str[i] <= '9')
            value += (str[i] - '0') * pow(HEX_STR, l - 1 - i);
        else
            value += (str[i] - 'A' + DEC_STR) * pow(HEX_STR, l - 1 - i);
    }
}

char StringUtils::GetBlank()
{
    char blank = ' ';
    return blank;
};

char StringUtils::GetSeparator()
{
    char separator = '-';
    return separator;
}

/**
 * @description: The character length is insufficient to complement
 * @param {string} &str-The string to be filled
 * @param {int} &length-The length of the string to be set
 * @param {char} &fileStr-A character to be added when the length is insufficient
 * @param {bool} &left-true:Add characters on the left,false:Add characters to the right
 * @return {*}
 */

void StringUtils::SetWidth(const int &width, const char &fileStr, const bool &left, string &str)
{
    ostringstream s;
    s.clear();

    if (left) {
        s << setw(width) << setfill(fileStr) << setiosflags(ios::left) << str;
    } else {
        s << setw(width) << setfill(fileStr) << setiosflags(ios::right) << str;
    }
    str = s.str();
}
} // namespace HiviewDFX
} // namespace OHOS
