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
#include <regex>
#include "string_ex.h"
using namespace std;
namespace OHOS {
namespace HiviewDFX {

constexpr int MILLISECONDS_PER_SECOND = 1000;

StringUtils::StringUtils()
{
}

StringUtils::~StringUtils()
{
}

bool StringUtils::IsStringToIntSuccess(const std::string &str, int64_t &val)
{
    char *endPtr = nullptr;
    errno = 0;
    long long num = 0;
    num = std::strtoll(str.c_str(), &endPtr, 10); // 10 : decimal scale
    if (errno != 0 || num > INT64_MAX || num < INT64_MIN) {
        return false;
    }
    val = static_cast<int64_t>(num);
    return true;
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

long long StringUtils::StringToUnixMs(const std::string& datetime)
{
    std::tm tm_time = {};
    std::istringstream ss(datetime);
    ss >> std::get_time(&tm_time, "%Y-%m-%d %H:%M:%S");
    if (ss.fail()) {
        return -1;
    }
    time_t time_sec = mktime(&tm_time);
    if (time_sec == -1) {
        return -1;
    }
    return static_cast<long long>(time_sec) * MILLISECONDS_PER_SECOND;
}

std::string StringUtils::UnixMsToString(uint64_t ms_timestamp)
{
    if (ms_timestamp > static_cast<uint64_t>(INT64_MAX)) {
        return "";
    }
    int64_t ms_signed = static_cast<int64_t>(ms_timestamp);
    time_t time_sec = static_cast<time_t>(ms_signed / MILLISECONDS_PER_SECOND);
    std::tm tm_time;
    localtime_r(&time_sec, &tm_time);

    char buffer[20];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm_time);
    return std::string(buffer);
}

} // namespace HiviewDFX
} // namespace OHOS
