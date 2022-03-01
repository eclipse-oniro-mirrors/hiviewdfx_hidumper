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
#ifndef LIST_DUMPER_H
#define LIST_DUMPER_H

#include "hidumper_executor.h"

namespace OHOS {
namespace HiviewDFX {
class ListDumper : public HidumperExecutor {
public:
    ListDumper();
    ~ListDumper();
    DumpStatus PreExecute(const std::shared_ptr<DumperParameter>& parameter,
        StringMatrix dumpDatas) override;
    DumpStatus Execute() override;
    DumpStatus AfterExecute() override;

private:
    const std::string AppendBlank(const std::string& str);
    std::string target_;
    StringMatrix result_;

private:
    static const std::string ABILITY_HEADER;
    static const std::string SYSTEM_HEADER;
    static const int ITEM_SUM_LINE;
    static const std::string NAME_SPACE;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // LIST_DUMPER_H
