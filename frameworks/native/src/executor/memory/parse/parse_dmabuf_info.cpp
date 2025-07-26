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

using namespace std;
namespace OHOS {
namespace HiviewDFX {
ParseDmaBufInfo::ParseDmaBufInfo()
{
}

ParseDmaBufInfo::~ParseDmaBufInfo()
{
}

bool ParseDmaBufInfo::GetDmaBufInfo(const int32_t &pid, vector<string> &result, unordered_map<string, int> &headerMap,
                                    vector<int> &columnWidths, vector<string> &titles)
{
    DUMPER_HILOGD(MODULE_SERVICE, "GetDmaBufInfo begin, pid:%{public}d", pid);
    bool inDetailSection = false;
    std::string path = "/proc/" + to_string(pid) + "/mm_dmabuf_info";
    std::string detailTitle = "";
    std::vector<string> details;
    std::unordered_map<string, int> headerMapTmp;
    std::vector<int> columnWidthsTmp;
    std::vector<std::string> titlesTmp;
    bool ret = FileUtils::GetInstance().LoadStringFromProcCb(path, false, true, [&](const string &line) -> void {
        if (!inDetailSection && line.find("Process") != std::string::npos) {
            std::istringstream lineStream(line);
            vector<string> headers;
            string header;
            while (lineStream >> header) {
                headers.push_back(header);
            }
            for (size_t i = 0; i < headers.size(); ++i) {
                headerMapTmp[headers[i]] = i;
                columnWidthsTmp.push_back(static_cast<int>(headers[i].length()));
            }
            titlesTmp = headers;
            detailTitle = line;
            inDetailSection = true;
            return;
        }
        if (inDetailSection) {
            std::istringstream lineStream(line);
            vector<string> row;
            string item;
            while (lineStream >> item) {
                row.push_back(item);
            }
            for (size_t i = 0; i < row.size(); ++i) {
                if (i >= columnWidthsTmp.size()) {
                    columnWidthsTmp.push_back(static_cast<int>(row[i].length()));
                } else if (row[i].length() > static_cast<size_t>(columnWidthsTmp[i])) {
                    columnWidthsTmp[i] = static_cast<int>(row[i].length());
                }
            }
            details.push_back(line);
        }
    });
    if (details.empty()) {
        DUMPER_HILOGE(MODULE_SERVICE, "detail dmabuf is empty.");
        return false;
    }
    details.insert(details.begin(), detailTitle);
    result = details;
    headerMap = headerMapTmp;
    columnWidths = columnWidthsTmp;
    titles = titlesTmp;
    DUMPER_HILOGD(MODULE_SERVICE, "GetDmaBufInfo end, pid:%{public}d, ret:%{public}d", pid, ret);
    return ret;
}
} // namespace HiviewDFX
} // namespace OHOS