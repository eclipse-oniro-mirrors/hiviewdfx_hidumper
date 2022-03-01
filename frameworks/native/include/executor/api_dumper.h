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
#ifndef API_DUMPER_H
#define API_DUMPER_H

#include "hidumper_executor.h"

namespace OHOS {
namespace HiviewDFX {
class APIDumper : public HidumperExecutor {
public:
    APIDumper();
    ~APIDumper();
    DumpStatus PreExecute(const std::shared_ptr<DumperParameter>& parameter,
        StringMatrix dumpDatas) override;
    DumpStatus Execute() override;
    DumpStatus AfterExecute() override;

private:
    void AddApiRetIntoResult(const char* content, const std::string& title);
    void AddApiRetIntoResult(int content, const std::string& title);

private:
    StringMatrix result_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // API_DUMPER_H
