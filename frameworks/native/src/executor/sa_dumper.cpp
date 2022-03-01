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
#include "executor/sa_dumper.h"
#include <cstdio>
#include <thread>
#include <unistd.h>
#include "securec.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
const int PIPE_LENGTH = 2;
const int PIPE_INIT = 0;
const int PIPE_READ = 0;
const int PIPE_WRITE = 1;
const int LINE_LENGTH = 256;
const char *SEPARATOR_TEMPLATE = "\n----------------------------------%04d---------------------------------\n";

using StringMatrix = std::shared_ptr<std::vector<std::vector<std::string>>>;

// UString class
class UString {
public:
    explicit UString(const std::u16string &String)
    {
        string_ = std::wstring_convert<std::codecvt_utf8<char16_t>, char16_t> {}.to_bytes(String);
    }

public:
    const char *GetString() const
    {
        return string_.c_str();
    }
    int GetInteger() const
    {
        return stoi(string_);
    }

private:
    std::string string_;
};

// MatrixWriter
class MatrixWriter {
public:
    explicit MatrixWriter(StringMatrix Data) : spData_(Data)
    {
    }

public:
    void WriteLine(const char *Line)
    {
        if (spData_) {
            std::vector<std::string> lines;
            lines.push_back(Line);
            spData_->push_back(lines);
        } else {
            printf("%s\n", Line);
        }
    }

private:
    StringMatrix spData_;
};

// PipeReader
class PipeReader {
public:
    PipeReader(int Id, StringMatrix Data) : id_(Id), spData_(Data)
    {
        fds_[PIPE_READ] = PIPE_INIT;
        fds_[PIPE_WRITE] = PIPE_INIT;
        pipe(fds_);
    }
    ~PipeReader()
    {
        if (wt_.joinable()) {
            wt_.join();
        }
        close(fds_[PIPE_READ]);
        close(fds_[PIPE_WRITE]);
    }

public:
    int GetWritePipe()
    {
        return fds_[PIPE_WRITE];
    }
    void Run()
    {
        wt_ = std::thread(Execute, id_, fds_[PIPE_READ], spData_);
    }
    void Stop()
    {
        close(fds_[PIPE_WRITE]);
    }
    static void Execute(int id, int fd, StringMatrix Data)
    {
        char *buff = (char *)malloc(LINE_LENGTH);
        if (buff == nullptr) {
            return;
        }
        FILE *fp = fdopen(fd, "r");
        if (fp != nullptr) {
            size_t size = LINE_LENGTH;
            for (int i = 1;; ++i) {
                int len = getline(&buff, &size, fp);
                if (len <= 0) {
                    break;
                }
                if (buff[--len] == '\n') {
                    buff[len] = 0;
                }
                MatrixWriter(Data).WriteLine(buff);
            }
            fclose(fp);
        }
        free(buff);
    }

private:
    int id_;
    StringMatrix spData_;
    int fds_[PIPE_LENGTH];
    std::thread wt_;
};
} // namespace

SADumper::SADumper(void)
{
}

SADumper::~SADumper(void)
{
}

DumpStatus SADumper::PreExecute(const std::shared_ptr<DumperParameter> &parameter, StringMatrix dump_datas)
{
    result_ = dump_datas;
    size_t size = ptrDumpCfg_->args_->GetNameList().size();
    if (size > 1) {
        names_ = ptrDumpCfg_->args_->GetNameList();
    } else if (size == 1) {
        svcName_ = ptrDumpCfg_->args_->GetNameList().front();
    }

    StringVector args = ptrDumpCfg_->args_->GetArgList();
    if (!args.empty()) {
        std::transform(args.begin(), args.end(), std::back_inserter(args_), Str8ToStr16);
    }

    return DumpStatus::DUMP_OK;
}

void SADumper::GetData(const std::string &name, const sptr<ISystemAbilityManager> &sam)
{
    int id = stoi(name);
    sptr<IRemoteObject> obj = sam->CheckSystemAbility(id);
    if (obj) {
        char line[LINE_LENGTH] = {};
        int ret = sprintf_s(line, sizeof(line), SEPARATOR_TEMPLATE, id);
        if (ret != -1) {
            MatrixWriter(result_).WriteLine(line);
            PipeReader reader(id, result_);
            reader.Run();
            U16StringVector args;
            obj->Dump(reader.GetWritePipe(), args);
            reader.Stop();
        }
    }
}

DumpStatus SADumper::Execute()
{
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!sam) {
        LOG_ERR("get null sa mgr for ability id '%s'\n", svcName_.c_str());
        return DumpStatus::DUMP_FAIL;
    }

    if (svcName_.empty()) {
        if (names_.empty()) {
            U16StringVector vct = sam->ListSystemAbilities();
            std::transform(vct.begin(), vct.end(), std::back_inserter(names_), Str16ToStr8);
        }
        for (size_t i = 0, l = names_.size(); i < l; ++i) {
            GetData(names_[i], sam);
        }
        return DumpStatus::DUMP_OK;
    }

    int id = svcName_.empty() ? 0 : stoi(svcName_);
    if (id == 0) {
        LOG_ERR("no such ability id '%s'\n", svcName_.c_str());
        return DumpStatus::DUMP_FAIL;
    }

    sptr<IRemoteObject> sa = sam->CheckSystemAbility(id);
    if (!sa) {
        LOG_ERR("no such system ability %s\n", svcName_.c_str());
        return DumpStatus::DUMP_FAIL;
    }

    PipeReader reader(id, result_);
    if (!reader.GetWritePipe()) {
        LOG_ERR("pipe create failed.\n");
        return DumpStatus::DUMP_FAIL;
    }

    DumpStatus result = DumpStatus::DUMP_OK;

    reader.Run();
    if (FAILED(sa->Dump(reader.GetWritePipe(), args_))) {
        LOG_ERR("dump failed, system ability %s \n", svcName_.c_str());
        result = DumpStatus::DUMP_FAIL;
    }
    reader.Stop();

    return result;
}

DumpStatus SADumper::AfterExecute()
{
    return DumpStatus::DUMP_OK;
}
} // namespace HiviewDFX
} // namespace OHOS
