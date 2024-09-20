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
#ifndef HIDUMPER_EXECUTOR_H
#define HIDUMPER_EXECUTOR_H
#include <memory>
#include <string>
#include <vector>
#include "hilog_wrapper.h"
#include "common.h"
#include "common/dumper_parameter.h"
namespace OHOS {
namespace HiviewDFX {
class HidumperExecutor {
public:
    virtual ~HidumperExecutor();

    using StringMatrix = std::shared_ptr<std::vector<std::vector<std::string>>>;

    virtual DumpStatus PreExecute(const std::shared_ptr<DumperParameter>& parameter, StringMatrix dumpDatas) = 0;
    virtual DumpStatus Execute() = 0;
    virtual DumpStatus AfterExecute() = 0;
    virtual void Reset();
public:
    DumpStatus DoPreExecute(const std::shared_ptr<DumperParameter>& parameter, StringMatrix dumpDatas);
    DumpStatus DoExecute();
    DumpStatus DoAfterExecute();

    void SetDumpConfig(const std::shared_ptr<DumpCfg>& config);
    const std::shared_ptr<DumpCfg>& GetDumpConfig() const;

    bool IsCanceled() const;
protected:
    std::shared_ptr<DumpCfg> ptrDumpCfg_;
private:
private:
    std::shared_ptr<RawParam> rawParam_;
    std::shared_ptr<HidumperExecutor> ptrParent_;
    static const std::string TIME_OUT_STR;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIDUMPER_EXECUTOR_H
