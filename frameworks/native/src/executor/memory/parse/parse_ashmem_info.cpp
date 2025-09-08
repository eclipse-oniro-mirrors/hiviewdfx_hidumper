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
#include "executor/memory/parse/parse_ashmem_info.h"
#include "executor/memory/memory_util.h"
#include "hilog_wrapper.h"
#include "util/file_utils.h"
#include "string_ex.h"

using namespace std;
namespace OHOS {
namespace HiviewDFX {
const int PHYSICAL_SIZE = 17; // 17 is the length of "physical size is "
const int START_POS = 1;
ParseAshmemInfo::ParseAshmemInfo()
{
}

ParseAshmemInfo::~ParseAshmemInfo()
{
}

bool ParseAshmemInfo::UpdateAshmemOverviewMap(
    const std::string &line, std::unordered_map<std::string, int64_t> &ashmemOverviewMap)
{
    std::string tempLine = line;
    // delete the whitespace characters at the end of the line
    size_t lastNonSpace = tempLine.find_last_not_of(' ');
    if (lastNonSpace != std::string::npos) {
        tempLine.erase(lastNonSpace + START_POS);
    }
    size_t leftBracketPos = tempLine.find("[");
    if (leftBracketPos == std::string::npos) {
        DUMPER_HILOGE(MODULE_SERVICE, "tempLine missing [:%{public}s.", tempLine.c_str());
        return false;
    }
    size_t startPos = leftBracketPos + START_POS;
    size_t endPos = tempLine.find("]", startPos);
    if (startPos == std::string::npos || endPos == std::string::npos || endPos <= startPos) {
        DUMPER_HILOGE(MODULE_SERVICE, "tempLine is error data, tempLine:%{public}s.", tempLine.c_str());
        return false;
    }
    std::string tmpProcessName = tempLine.substr(startPos, endPos - startPos);
    size_t pos = tempLine.find("physical size is ");
    if (pos == std::string::npos) {
        DUMPER_HILOGE(MODULE_SERVICE, "tempLine not find [physical size is], tempLine:%{public}s.", tempLine.c_str());
        return false;
    }
    std::string physicalSize = tempLine.substr(pos + PHYSICAL_SIZE);
    if (!IsNumericStr(physicalSize)) {
        DUMPER_HILOGE(MODULE_SERVICE, "physicalSize is not number, physicalSize:%{public}s.", physicalSize.c_str());
        return false;
    }
    int64_t pss = 0;
    if (!StringUtils::GetInstance().IsStringToIntSuccess(physicalSize, pss)) {
        DUMPER_HILOGE(MODULE_SERVICE, "physicalSize is too big, physicalSize:%{public}s.", physicalSize.c_str());
        return false;
    }
    ashmemOverviewMap[tmpProcessName] = pss;
    return true;
}

bool ParseAshmemInfo::GetAshmemInfo(const int32_t &pid, pair<int, vector<string>> &result)
{
    DUMPER_HILOGD(MODULE_SERVICE, "GetAshmemInfo begin, pid:%{public}d", pid);
    bool inDetailSection = false;
    std::string path = "/proc/ashmem_process_info";
    std::string processName = "unknown";
    std::string detailTitle = "";
    std::vector<string> details;
    std::unordered_map<std::string, int64_t> ashmemOverviewMap;
    bool ret = FileUtils::GetInstance().LoadStringFromProcCb(path, false, true, [&](const string &line) -> void {
        if (line.find("Total ashmem  of") != std::string::npos) {
            UpdateAshmemOverviewMap(line, ashmemOverviewMap);
        }
        if (line.find("Process_name") != std::string::npos) {
            detailTitle = line;
            inDetailSection = true;
            return;
        }
        if (inDetailSection) {
            std::istringstream lineStream(line);
            int targetPid = 0;
            std::string name;
            lineStream >> name >> targetPid;
            if (pid == targetPid) {
                processName = name;
                details.push_back(line);
            }
        }
    });
    if (details.empty()) {
        DUMPER_HILOGE(MODULE_SERVICE, "detail ashmem is empty.");
        return false;
    }
    details.insert(details.begin(), detailTitle);
    if (ashmemOverviewMap.find(processName) == ashmemOverviewMap.end()) {
        DUMPER_HILOGE(MODULE_SERVICE, "not find processName:%{public}s.", processName.c_str());
        return false;
    }
    result.first = ashmemOverviewMap[processName] / static_cast<int64_t>(MemoryUtil::GetInstance().BYTE_TO_KB_); // KB
    result.second = details;
    DUMPER_HILOGD(MODULE_SERVICE, "GetAshmemInfo end, pid:%{public}d, ret:%{public}d", pid, ret);
    return ret;
}
} // namespace HiviewDFX
} // namespace OHOS