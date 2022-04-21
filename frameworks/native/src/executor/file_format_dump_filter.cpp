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
#include "executor/file_format_dump_filter.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
const char ASCII_CR = '\r';
const char ASCII_LF = '\n';
const char ASCII_ESC = '\033';
const char ASCII_OB = '[';
const char ASCII_UA = 'A';
const char ASCII_LA = 'a';
const char ASCII_UZ = 'Z';
const char ASCII_LZ = 'z';
}

FileFormatDumpFilter::FileFormatDumpFilter()
{
}

FileFormatDumpFilter::~FileFormatDumpFilter()
{
    dumpDatas_ = nullptr;
}

DumpStatus FileFormatDumpFilter::PreExecute(const std::shared_ptr<DumperParameter>& parameter,
    StringMatrix dumpDatas)
{
    dumpDatas_ = dumpDatas;
    return DumpStatus::DUMP_OK;
}

DumpStatus FileFormatDumpFilter::Execute()
{
    if (dumpDatas_ == nullptr) {
        return DumpStatus::DUMP_FAIL;
    }

    for (auto &lineItems : (*dumpDatas_)) {
        for (auto &item : lineItems) {
            FilterControlChar(item);
        }
    }

    return DumpStatus::DUMP_OK;
}

DumpStatus FileFormatDumpFilter::AfterExecute()
{
    dumpDatas_ = nullptr;
    return DumpStatus::DUMP_OK;
}

void FileFormatDumpFilter::FilterControlChar(std::string &str)
{
    std::string newStr;

    bool skip = false;
    const size_t sum = str.size();
    for (size_t pos = 0; pos < sum; pos++) {
        char &c = str.at(pos);

        if ((!skip) && (c == ASCII_ESC) && ((pos + 1) < sum)) {
            char &next_c = str.at(pos + 1);
            skip = (next_c == ASCII_OB);
        }

        if (skip && (((c >= ASCII_UA) && (c <= ASCII_UZ)) || ((c >= ASCII_LA) && (c <= ASCII_LZ)))) {
            skip = false;
            continue;
        }

        if (skip || (c == ASCII_CR) || (c == ASCII_LF)) {
            continue;
        }

        newStr.append(1, c);
    }

    str = newStr;
}
} // namespace HiviewDFX
} // namespace OHOS
