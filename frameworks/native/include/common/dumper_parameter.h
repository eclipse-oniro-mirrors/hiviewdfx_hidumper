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
#ifndef HIDUMPER_SERVICES_DUMPER_PARAMETER_H
#define HIDUMPER_SERVICES_DUMPER_PARAMETER_H

#include <vector>

#include "common/dump_cfg.h"
#include "common/dumper_opts.h"
#include "raw_param.h"
namespace OHOS {
namespace HiviewDFX {
class DumperParameter {
public:
    DumperParameter();
    ~DumperParameter();
public:
    void SetOpts(const DumperOpts &opts);
    const DumperOpts &GetOpts() const;
    // set output file path
    void SetOutputFilePath(std::string& path);
    // get output file path
    std::string& GetOutputFilePath();
    // set config list
    void SetExecutorConfigList(std::vector<std::shared_ptr<DumpCfg>>& list);
    // get config list
    std::vector<std::shared_ptr<DumpCfg>>& GetExecutorConfigList();
    // set client callback
    void setClientCallback(const std::shared_ptr<RawParam>& reqCtl);
    // get client callback
    std::shared_ptr<RawParam> getClientCallback();
    // set IPC flag
    // check IPC flag
    void SetUid(int uid)
    {
        uid_ = uid;
    }
    int GetUid() const
    {
        return uid_;
    }
    void SetPid(int pid)
    {
        pid_ = pid;
    }
    int GetPid() const
    {
        return pid_;
    };
    void Dump() const;
private:
    int uid_ {-1};
    int pid_ {-1};
    DumperOpts opts_;
    std::vector<std::shared_ptr<DumpCfg>> list_; // list
    std::shared_ptr<RawParam> mPtrReqCtl;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIDUMPER_SERVICES_DUMPER_PARAMETER_H
