/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include <fstream>
#include <sstream>
#include "executor/memory/parse/parse_dmabuf_info.h"
#include "hilog_wrapper.h"
#include "util/file_utils.h"
#include "string_ex.h"

namespace OHOS {
namespace HiviewDFX {
ParseDmaBufInfo::ParseDmaBufInfo()
{
}

ParseDmaBufInfo::~ParseDmaBufInfo()
{
}

void ParseDmaBufInfo::ParseHeaderLine(const std::string &line)
{
    std::istringstream lineStream(line);
    std::vector<std::string> headers;
    std::string header;
    while (lineStream >> header) {
        headers.emplace_back(header);
    }

    headerMapTmp_.clear();
    columnWidthsTmp_.clear();
    headerMapTmp_.reserve(headers.size());
    columnWidthsTmp_.reserve(headers.size());

    for (size_t i = 0; i < headers.size(); ++i) {
        headerMapTmp_[headers[i]] = static_cast<int>(i);
        columnWidthsTmp_.emplace_back(static_cast<int>(headers[i].length()));
    }

    titlesTmp_ = headers;
    detailTitle_ = line;
    inDetailSection_ = true;
}

void ParseDmaBufInfo::ParseDataLine(const std::string &line)
{
    std::istringstream lineStream(line);
    std::vector<std::string> row;
    std::string item;
    while (lineStream >> item) {
        row.emplace_back(item);
    }

    for (size_t i = 0; i < row.size(); ++i) {
        if (i >= columnWidthsTmp_.size()) {
            columnWidthsTmp_.emplace_back(static_cast<int>(row[i].length()));
        } else if (row[i].length() > static_cast<size_t>(columnWidthsTmp_[i])) {
            columnWidthsTmp_[i] = static_cast<int>(row[i].length());
        }
    }

    details_.emplace_back(line);
}

bool ParseDmaBufInfo::GetDmaBufInfo(const int32_t &pid, std::vector<std::string> &result,
                                    std::unordered_map<std::string, int> &headerMap,
                                    std::vector<int> &columnWidths,
                                    std::vector<std::string> &titles)
{
    DUMPER_HILOGD(MODULE_SERVICE, "GetDmaBufInfo begin, pid:%{public}d", pid);

    inDetailSection_ = false;
    detailTitle_.clear();
    details_.clear();
    headerMapTmp_.clear();
    columnWidthsTmp_.clear();
    titlesTmp_.clear();

    const std::string path = "/proc/" + std::to_string(pid) + "/mm_dmabuf_info";

    bool ret = FileUtils::GetInstance().LoadStringFromProcCb(path, false, true,
        [&](const std::string &line) -> void {
            if (!inDetailSection_ && line.find("Process") != std::string::npos) {
                ParseHeaderLine(line);
                return;
            }
            if (inDetailSection_) {
                ParseDataLine(line);
            }
        });

    if (details_.empty()) {
        DUMPER_HILOGI(MODULE_SERVICE, "detail dmabuf is empty.");
        return ret;
    }

    details_.insert(details_.begin(), detailTitle_);
    result = std::move(details_);
    headerMap = std::move(headerMapTmp_);
    columnWidths = std::move(columnWidthsTmp_);
    titles = std::move(titlesTmp_);

    DUMPER_HILOGD(MODULE_SERVICE, "GetDmaBufInfo end, pid:%{public}d, ret:%{public}d", pid, ret);
    return ret;
}
} // namespace HiviewDFX
} // namespace OHOS