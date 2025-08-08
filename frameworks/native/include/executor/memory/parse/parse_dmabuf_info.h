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

#ifndef PARSE_DMABUF_INFO_H
#define PARSE_DMABUF_INFO_H
#include <map>
#include <string>
#include <vector>

namespace OHOS {
namespace HiviewDFX {
class ParseDmaBufInfo {
public:
    ParseDmaBufInfo();
    ~ParseDmaBufInfo();

    bool GetDmaBufInfo(const int32_t &pid, std::vector<std::string> &result,
                       std::unordered_map<std::string, int> &headerMap,
                       std::vector<int> &columnWidths,
                       std::vector<std::string> &titles);

private:
    void ParseHeaderLine(const std::string &line);
    void ParseDataLine(const std::string &line);

    std::unordered_map<std::string, int> headerMapTmp_;
    std::vector<int> columnWidthsTmp_;
    std::vector<std::string> titlesTmp_;
    std::string detailTitle_;
    bool inDetailSection_ = false;
    std::vector<std::string> details_;
};
} // namespace HiviewDFX
} // namespace OHOS

#endif