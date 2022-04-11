/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include "dump_utils.h"

#include <cerrno>
#include <csignal>
#include <fcntl.h>
#include <poll.h>
#include <set>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include <iostream>

#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "common.h"
#include "datetime_ex.h"
#include "securec.h"
#include "string_ex.h"

#include "system_ability_definition.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace HiviewDFX {
void DumpUtils::IgnorePipeQuit()
{
    signal(SIGPIPE, SIG_IGN); // protect from pipe quit
}

void DumpUtils::IgnoreStdoutCache()
{
    setvbuf(stdout, nullptr, _IONBF, 0); // never cache stdout
}

void DumpUtils::IgnoreOom()
{
    setpriority(PRIO_PROCESS, 0, TOP_PRIORITY); // protect from OOM
}

void DumpUtils::SetAdj(int adj)
{
    int fd = FdToWrite(FILE_CUR_OOM_ADJ);
    if (fd < 0) {
        return;
    }
    dprintf(fd, "%d", adj);
    close(fd);
}

void DumpUtils::BoostPriority()
{
    IgnoreOom();
    IgnorePipeQuit();
    IgnoreStdoutCache();
    SetAdj(TOP_OOM_ADJ);
}

std::string DumpUtils::ErrnoToMsg(const int &error)
{
    const int bufSize = 128;
    char buf[bufSize] = {0};
    strerror_r(error, buf, bufSize);
    return buf;
}

int DumpUtils::FdToRead(const std::string &file)
{
    char path[PATH_MAX] = {0};
    if (realpath(file.c_str(), path) == nullptr) {
        DUMPER_HILOGD(MODULE_COMMON, "realpath, no such file. path=[%{public}s], file=[%{public}s]",
            path, file.c_str());
        return -1;
    }

    if (file != std::string(path)) {
        DUMPER_HILOGI(MODULE_COMMON, "fail to check consistency. path=[%{public}s], file=[%{public}s]",
            path, file.c_str());
    }

    int fd = TEMP_FAILURE_RETRY(open(path, O_RDONLY | O_CLOEXEC | O_NONBLOCK));
    if (fd == -1) {
        DUMPER_HILOGD(MODULE_COMMON, "open [%{public}s] %{public}s", path, ErrnoToMsg(errno).c_str());
    }
    return fd;
}

int DumpUtils::FdToWrite(const std::string &file)
{
    std::string split = "/";
    auto pos = file.find_last_of(split);
    if (pos == std::string::npos) {
        DUMPER_HILOGD(MODULE_COMMON, "file path:[%{public}s] error", file.c_str());
        return -1;
    }
    std::string tempPath = file.substr(0, pos + 1);
    std::string name = file.substr(pos + 1);

    char path[PATH_MAX] = {0};
    if (realpath(tempPath.c_str(), path) != nullptr) {
        std::string fileName = path + split + name;
        int fd = TEMP_FAILURE_RETRY(open(fileName.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC | O_NOFOLLOW,
                                         S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH));
        if (fd == -1) {
            DUMPER_HILOGD(MODULE_COMMON, "open [%{public}s] %{public}s",
                fileName.c_str(), ErrnoToMsg(errno).c_str());
        }
        return fd;
    }
    DUMPER_HILOGD(MODULE_COMMON, "realpath, no such file. path=[%{public}s], tempPath=[%{public}s]",
        path, tempPath.c_str());
    return -1;
}

bool DumpUtils::FileWriteable(const std::string &file)
{
    int fd = FdToWrite(file);
    if (fd >= 0) {
        close(fd);
        return true;
    }
    return false;
}

bool DumpUtils::CheckProcessAlive(uint32_t pid)
{
    char path[PATH_MAX] = {0};
    char pathPid[PATH_MAX] = {0};
    int ret = sprintf_s(pathPid, PATH_MAX, "/proc/%u", pid);
    if (ret < 0) {
        return false;
    }
    if (realpath(pathPid, path) != nullptr) { // path exist
        if (access(path, F_OK) == 0) {        // can be read
            return true;
        }
    }
    return false;
}

void DumpUtils::RemoveDuplicateString(std::vector<std::string> &strList)
{
    std::vector<std::string> tmpVtr;
    std::set<std::string> tmpSet;
    for (auto &it : strList) {
        auto ret = tmpSet.insert(it);
        if (ret.second) {
            tmpVtr.push_back(it);
        }
    }
    strList = tmpVtr;
}

int DumpUtils::StrToId(const std::string &name)
{
    int id = 0;
    auto iter = std::find_if(saNameMap_.begin(), saNameMap_.end(), [&](const std::pair<int, std::string> &item) {
        return name.compare(item.second) == 0;
    });
    if (iter == saNameMap_.end()) {
        if (!StrToInt(name, id)) { // Decimal string
            return 0; // invalid ability ID
        }
    } else {
        id = iter->first;
    }
    return id;
}

std::string DumpUtils::ConvertSaIdToSaName(const std::string &saIdStr)
{
    int saId = StrToId(saIdStr);
    auto iter = saNameMap_.find(saId);
    if (iter == saNameMap_.end()) {
        return saIdStr;
    }
    return iter->second;
}

bool DumpUtils::DirectoryExists(const std::string &path)
{
    struct stat fileInfo;
    if (stat(path.c_str(), &fileInfo) == 0) {
        return S_ISDIR(fileInfo.st_mode);
    }
    return false;
}

bool DumpUtils::PathIsValid(const std::string &path)
{
    return access(path.c_str(), F_OK) == 0;
}

bool DumpUtils::CopyFile(const std::string &src, const std::string &des)
{
    std::ifstream fin(src);
    std::ofstream fout(des);
    if ((!fin.is_open()) || (!fout.is_open())) {
        return false;
    }
    fout << fin.rdbuf();
    if (fout.fail()) {
        fout.clear();
    }
    fout.flush();
    return true;
}
} // namespace HiviewDFX
} // namespace OHOS
