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
#ifndef SADUMPER_H
#define SADUMPER_H
#include <iservice_registry.h>
#include "../include/executor/hidumper_executor.h"

namespace OHOS {
namespace HiviewDFX {
class SADumper : public HidumperExecutor {
public:
    SADumper(void);
    ~SADumper(void);

public:
    virtual DumpStatus PreExecute(const std::shared_ptr<DumperParameter> &parameter, StringMatrix dumpDatas) override;
    virtual DumpStatus Execute() override;
    virtual DumpStatus AfterExecute() override;
    using StringVector = std::vector<std::string>;
    using U16StringVector = std::vector<std::u16string>;

private:
    StringMatrix result_;
    StringVector names_;
    U16StringVector args_;

    DumpStatus GetData(const std::string &name, const sptr<ISystemAbilityManager> &sam);
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // End of SADUMPER_H
