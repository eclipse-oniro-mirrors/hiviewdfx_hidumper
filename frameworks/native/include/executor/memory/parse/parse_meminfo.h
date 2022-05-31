/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef PARSE_MEMINFO_H
#define PARSE_MEMINFO_H
#include <map>
#include <string>
#include <vector>

namespace OHOS {
namespace HiviewDFX {
class ParseMeminfo {
public:
    ParseMeminfo();
    ~ParseMeminfo();

    using ValueMap = std::map<std::string, uint64_t>;
    bool GetMeminfo(ValueMap &meminfo);

private:
    void SetData(const std::string &str, ValueMap &result);
};
} // namespace HiviewDFX
} // namespace OHOS

#endif
