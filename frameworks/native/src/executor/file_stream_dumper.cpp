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
#include "executor/file_stream_dumper.h"
#include <dirent.h>
#include <unistd.h>
#include "dump_utils.h"

namespace OHOS {
namespace HiviewDFX {
FileStreamDumper::FileStreamDumper()
    :fp_(nullptr),
    fd_(-1),
    next_file_index_(0),
    more_data_(false),
    need_loop_(false)
{
}

FileStreamDumper::~FileStreamDumper()
{
    CloseFd();
}

DumpStatus FileStreamDumper::PreExecute(const std::shared_ptr<DumperParameter>& parameter,
    StringMatrix dumpDatas)
{
    if (dumpDatas.get()) {
        result_ = dumpDatas;
    }

    // open first file!
    if (next_file_index_ <= 0) {
        std::string target = ptrDumpCfg_->target_;
        bool arg_pid = false;
        bool arg_cpuid = false;
        int pid = 0;
        int cpuid = 0;
        if (ptrDumpCfg_->args_.get()) {
            arg_pid = ptrDumpCfg_->args_->HasPid();
            arg_cpuid = ptrDumpCfg_->args_->HasCpuId();
            pid = ptrDumpCfg_->args_->GetPid();
            cpuid = ptrDumpCfg_->args_->GetCpuId();
        }
        BuildFileNames(target, arg_pid, pid, arg_cpuid, cpuid);
        need_loop_ = (ptrDumpCfg_->loop_ == DumperConstant::LOOP);

        int ret = OpenNextFile();
        if (ret <= 0) {
            return DumpStatus::DUMP_FAIL;
        }
    }

    return DumpStatus::DUMP_OK;
}

int FileStreamDumper::OpenNextFile()
{
    // Close current file.
    CloseFd();
    if (next_file_index_ >= filenames_.size()) {
        // no more files
        return 0;
    }
    // Next file
    std::string filename = filenames_[next_file_index_];
    // Open fd and fp
    if ((fd_ = DumpUtils::FdToRead(filename)) == -1) {
        return -1;
    }
    if ((fp_ = fdopen(fd_, "r")) == nullptr) {
        close(fd_);
        fd_ = -1;
        return -1;
    }
    next_file_index_ ++;
    // add file name into buffer
    std::vector<std::string> line_vector_blank;
    line_vector_blank.push_back("");
    std::vector<std::string> line_vector_filename;
    line_vector_filename.push_back(filename);
    result_->push_back(line_vector_blank);
    result_->push_back(line_vector_filename);
    result_->push_back(line_vector_blank);
    return next_file_index_;
}

// read one line
DumpStatus FileStreamDumper::ReadLine()
{
    DumpStatus ret = DumpStatus::DUMP_MORE_DATA;
    if (fp_ == nullptr) {
        return DumpStatus::DUMP_FAIL;
    }
    char* line_buffer = nullptr;
    size_t len = 0;
    ssize_t read = 0;
    read = getline(&line_buffer, &len, fp_);
    if (read != -1) {
        if (line_buffer[read-1] == '\n') {
            line_buffer[read-1] = '\0'; // replease \n
        }
        std::string line = line_buffer;
        std::vector<std::string> line_vector;
        line_vector.push_back(line);
        result_->push_back(line_vector);
    } else {
        if (feof(fp_) == 0) { // ferror()
            ret = DumpStatus::DUMP_FAIL;
        }
    }
    // end of file
    if (feof(fp_) != 0) {
        int more_file = OpenNextFile();
        if (more_file > 0) {
            ret = DumpStatus::DUMP_MORE_DATA;
        } else if (more_file == 0) {
            ret = DumpStatus::DUMP_OK;
        } else { // Error!
            ret = DumpStatus::DUMP_FAIL;
        }
    }
    if (line_buffer != nullptr) {
        free(line_buffer);
        line_buffer = nullptr;
    }
    more_data_ = (ret == DumpStatus::DUMP_MORE_DATA);
    return ret;
}

DumpStatus FileStreamDumper::Execute()
{
    DumpStatus ret = DumpStatus::DUMP_OK;
    if (need_loop_) {
        // dump one line
        return ReadLine();
    } else {
        // dump all line
        do {
            if (IsCanceled()) {
                break;
            }
            ret = ReadLine();
        } while (ret == DumpStatus::DUMP_MORE_DATA);
    }
    return ret;
}

DumpStatus FileStreamDumper::AfterExecute()
{
    if (more_data_ && IsTimeout()) {
        DUMPER_HILOGE(MODULE_COMMON, "error|file timeout");
        std::vector<std::string> line_vector_timeout;
        line_vector_timeout.push_back(GetTimeoutStr());
        result_->push_back(line_vector_timeout);
        more_data_ = false;
    }

    if (!more_data_) {
        CloseFd();
        return DumpStatus::DUMP_OK;
    }
    return DumpStatus::DUMP_MORE_DATA;
}

void FileStreamDumper::BuildFileNames(const std::string& target, bool arg_pid, int pid,
    bool arg_cpuid, int cpuid)
{
    DIR* dir = opendir(target.c_str());
    if (dir != nullptr) {
        std::string dir_name = target;
        // add backslash at tail of target
        std::string backslash = "/";
        if (dir_name.compare(dir_name.size() - backslash.size(), backslash.size(), backslash) != 0) {
            dir_name.append(backslash);
        }
        dirent* ptr = nullptr;
        while ((ptr = readdir(dir)) != nullptr) {
            std::string d_name = ptr->d_name;
            if ((d_name == ".") || (d_name == "..")) {
                continue;
            }
            std::string filename = dir_name + d_name;

            if (arg_pid) {
                ReplacePidInFilename(filename, pid);
            }
            if (arg_cpuid) {
                ReplaceCpuidInFilename(filename, cpuid);
            }
            filenames_.push_back(filename);
        }
        closedir(dir);
    } else {
        std::string filename = target;
        if (arg_pid) {
            ReplacePidInFilename(filename, pid);
        }
        if (arg_cpuid) {
            ReplaceCpuidInFilename(filename, cpuid);
        }
        filenames_.push_back(filename);
    }
}

void FileStreamDumper::ReplacePidInFilename(std::string& filename, int pid)
{
    // replease %pid in filename
    size_t pos = filename.find("%pid");
    if (pos != std::string::npos) {
        filename = filename.replace(pos, strlen("%pid"), std::to_string(pid));
    }
}

void FileStreamDumper::ReplaceCpuidInFilename(std::string& filename, int cpuid)
{
    // replease %cpuid in filename
    size_t pos = filename.find("%cpuid");
    if (pos != std::string::npos) {
        filename = filename.replace(pos, strlen("%cpuid"), std::to_string(cpuid));
    }
}

void FileStreamDumper::CloseFd()
{
    if (fp_ != nullptr) {
        fclose(fp_);
        fp_ = nullptr;
    }
    if (fd_ >= 0) {
        close(fd_);
        fd_ = -1;
    }
};
} // namespace HiviewDFX
} // namespace OHOS
