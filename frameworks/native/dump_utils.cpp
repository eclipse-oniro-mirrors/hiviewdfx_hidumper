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
#include <unistd.h>

#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "common.h"
#include "datetime_ex.h"
#include "securec.h"
#include "string_ex.h"
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

bool DumpUtils::String2Uint32(const std::string &str, uint32_t &val)
{
    char *ptr = nullptr;
    const int base = 10; // Numerical base
    long lVal = strtol(str.c_str(), &ptr, base);
    if (*ptr != '\0' || lVal < 0 || lVal > INT32_MAX) {
        return false;
    }
    val = static_cast<uint32_t>(lVal);
    return true;
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
        LOG_ERR("no such file %s\n", path);
        return -1;
    }

    if (file != std::string(path)) {
        LOG_ERR("fail to check consistency.");
        return -1;
    }

    int fd = TEMP_FAILURE_RETRY(open(path, O_RDONLY | O_CLOEXEC | O_NONBLOCK));
    if (fd == -1) {
        LOG_ERR("open %s %s\n", path, ErrnoToMsg(errno).c_str());
    }
    return fd;
}

int DumpUtils::FdToWrite(const std::string &file)
{
    std::string split = "/";
    auto pos = file.find_last_of(split);
    if (pos == std::string::npos) {
        LOG_ERR("file path:%s error\n", file.c_str());
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
            LOG_ERR("open %s %s\n", fileName.c_str(), ErrnoToMsg(errno).c_str());
        }
        return fd;
    } else {
        return -1;
    }
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

bool DumpUtils::WaitPid(pid_t pid, uint32_t timeoutMs, int *status)
{
    sigset_t childMask, parentMask;
    int64_t tempTimeoutMs = static_cast<int64_t>(timeoutMs);
    if (timeoutMs > 0) {
        sigemptyset(&childMask);
        sigaddset(&childMask, SIGCHLD); // monitor child
        if (sigprocmask(SIG_BLOCK, &childMask, &parentMask) == -1) {
            LOG_ERR("sigprocmask failed: %s\n", ErrnoToMsg(errno).c_str());
            return false;
        }
        timespec ts;
        ts.tv_sec = tempTimeoutMs / SEC_TO_MILLISEC;
        ts.tv_nsec = (tempTimeoutMs % SEC_TO_MILLISEC) * SEC_TO_MILLISEC * SEC_TO_MILLISEC;
        int ret = TEMP_FAILURE_RETRY(sigtimedwait(&childMask, nullptr, &ts));
        int oldErrno = errno;
        if (sigprocmask(SIG_SETMASK, &parentMask, nullptr) == -1) {
            LOG_ERR("sigprocmask failed: %s\n", ErrnoToMsg(errno).c_str());
            if (ret == 0) {
                return false;
            }
        }
        if (ret == -1) {
            errno = oldErrno;
            if (errno == EAGAIN) {
                errno = ETIMEDOUT;
            } else {
                LOG_ERR("sigtimedwait failed: %s\n", ErrnoToMsg(errno).c_str());
            }
            return false;
        }
    }
    if (waitpid(pid, status, (timeoutMs == 0) ? 0 : WNOHANG) != pid) {
        LOG_ERR("wait error(%s)\n", ErrnoToMsg(errno).c_str());
        return false;
    }
    return true;
}

void DumpUtils::LiveWithParent()
{
    prctl(PR_SET_PDEATHSIG, SIGKILL); // father died child die
}

bool DumpUtils::StrToCmdBuf(const std::string &cmd, std::vector<const char *> &argVtr, std::vector<std::string> &cmdVtr)
{
    SplitStr(cmd, DumpUtils::SPACE, cmdVtr);
    size_t size = cmdVtr.size();
    if (size == 0) {
        LOG_ERR("empty cmd.\n");
        return false;
    }
    argVtr.resize(size + 1);
    for (size_t i = 0; i < size; i++) { // combine command array
        argVtr[i] = cmdVtr[i].data();
    }
    argVtr[size] = nullptr; // end of command.
    return true;
}

int DumpUtils::StrToId(const std::string &name)
{
    int id;
    if (!StrToInt(name, id)) { // Decimal string
        return 0;              // invalid ability ID
    }
    return id;
}

} // namespace HiviewDFX
} // namespace OHOS
