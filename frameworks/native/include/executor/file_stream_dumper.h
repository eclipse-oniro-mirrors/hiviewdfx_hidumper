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
#ifndef FILE_STREAM_DUMPER_H
#define FILE_STREAM_DUMPER_H

#include "hidumper_executor.h"

namespace OHOS {
namespace HiviewDFX {
class FileStreamDumper : public HidumperExecutor {
public:
    FileStreamDumper();
    ~FileStreamDumper();
    DumpStatus PreExecute(const std::shared_ptr<DumperParameter>& parameter,
        StringMatrix dumpDatas) override;
    DumpStatus Execute() override;
    DumpStatus AfterExecute() override;

private:
    DumpStatus ReadFile();
    DumpStatus ReadLine();
    void BuildFileNames(const std::string& target, bool arg_pid, int pid,
        bool arg_cpuid, int cpuid);
    void ReplacePidInFilename(std::string& filename, int pid);
    void ReplaceCpuidInFilename(std::string& filename, int cpuid);
    int OpenNextFile();
    void CloseFd();

private:
    std::vector<std::string> filenames_;
    FILE* fp_;
    int fd_;
    unsigned int next_file_index_;
    StringMatrix result_;

    // MoreData Flag
    bool more_data_;
    bool need_loop_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // FILE_STREAM_DUMPER_H
