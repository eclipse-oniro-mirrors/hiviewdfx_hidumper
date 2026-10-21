/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include "util/dump_fd_relay.h"

#include <cerrno>
#include <cinttypes>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

#include "common/dumper_constant.h"
#include "dump_utils.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace HiviewDFX {

FdRelay::FdRelay(int outFd) : outFd_(outFd)
{
}

FdRelay::~FdRelay()
{
    Finish();
}

bool FdRelay::Start()
{
    if (started_) {
        return true;
    }
    if (outFd_ < 0) {
        DUMPER_HILOGE(MODULE_COMMON, "invalid output fd %{public}d, relay disabled", outFd_);
        return false;
    }
    int fds[2] = {-1, -1};
    if (pipe2(fds, O_CLOEXEC) != 0) {
        DUMPER_HILOGE(MODULE_COMMON, "pipe2 failed, errno=%{public}d", errno);
        return false;
    }
    readFd_ = fds[0];
    writeFd_ = fds[1];

    // The consumer of outFd_ may disappear at any time, for example "hidumper ... | grep -m1 x".
    // Writing into a closed pipe must never take hidumper_service down with SIGPIPE.
    DumpUtils::IgnorePipeQuit();

    broken_.store(false);
    closing_.store(false);
    producerLeak_.store(false);
    relayed_.store(0);

    // pthread_create instead of std::thread: with -fno-exceptions a failing std::thread
    // constructor throws std::system_error straight into std::terminate, which would take
    // hidumper_service down. A return code lets us release the pipe and fall back instead.
    int ret = pthread_create(&worker_, nullptr, DrainThreadEntry, this);
    if (ret != 0) {
        DUMPER_HILOGE(MODULE_COMMON, "pthread_create failed, ret=%{public}d, relay disabled", ret);
        CloseWriteEnd();
        CloseReadEnd();
        return false;
    }
    threadStarted_ = true;
    started_ = true;
    DUMPER_HILOGI(MODULE_COMMON, "relay started, readFd=%{public}d writeFd=%{public}d outFd=%{public}d",
        readFd_, writeFd_, outFd_);
    return true;
}

int FdRelay::GetProducerFd() const
{
    return writeFd_;
}

bool FdRelay::Write(const std::string &data)
{
    if (!started_ || data.empty()) {
        return false;
    }
    if (broken_.load()) {
        // Nobody drains the pipe any more, writing would only block the caller.
        return false;
    }
    std::lock_guard<std::mutex> lock(writeMutex_);
    if (writeFd_ < 0) {
        return false;
    }
    if (!WriteFully(writeFd_, data.c_str(), data.size())) {
        DUMPER_HILOGE(MODULE_COMMON, "failed to queue %{public}zu bytes into the relay", data.size());
        return false;
    }
    return true;
}

void FdRelay::Finish()
{
    if (!started_) {
        return;
    }
    started_ = false;
    // The write end has to go first, otherwise the draining thread never observes EOF.
    // The SA side copy is closed by IPCObjectStub::OnRemoteDump as soon as Dump returns,
    // so after this point the pipe has no writer left.
    CloseWriteEnd();
    // Only now that our writer is gone can EOF arrive, so this is also the point
    // where waiting for it becomes measurable against RELAY_EOF_GRACE_MS.
    closing_.store(true);
    if (threadStarted_) {
        pthread_join(worker_, nullptr);
        threadStarted_ = false;
    }
    CloseReadEnd();
    DUMPER_HILOGI(MODULE_COMMON, "relay finished, relayed=%{public}" PRIu64 " bytes, broken=%{public}d, "
        "producerLeak=%{public}d", relayed_.load(), broken_.load() ? 1 : 0, producerLeak_.load() ? 1 : 0);
}

uint64_t FdRelay::GetRelayedBytes() const
{
    return relayed_.load();
}

bool FdRelay::IsOutputBroken() const
{
    return broken_.load();
}

bool FdRelay::IsProducerLeakSuspected() const
{
    return producerLeak_.load();
}

void *FdRelay::DrainThreadEntry(void *arg)
{
    if (arg == nullptr) {
        return nullptr;
    }
    pthread_setname_np(pthread_self(), RELAY_THREAD_NAME);
    static_cast<FdRelay *>(arg)->DrainLoop();
    return nullptr;
}

void FdRelay::DrainLoop()
{
    char buf[RELAY_BUF_SIZE] = {0};
    int32_t idleMs = 0;
    // Total time spent in the closing_ phase. Each loop iteration costs at most
    // RELAY_POLL_TIMEOUT_MS wall time, so counting the nominal timeout per iteration
    // over-estimates the elapsed time and the hard cap fires at or before its real
    // value -- the safe direction when the peer misbehaves.
    int32_t closingMs = 0;
    bool running = true;
    while (running) {
        running = DrainOnce(buf, RELAY_BUF_SIZE, idleMs, closingMs);
    }
    // Release the read end so that a producer still writing gets EPIPE right away
    // instead of blocking on a pipe nobody drains any more.
    CloseReadEnd();
}

// One poll / read / forward cycle. Returns false once the loop has to end.
bool FdRelay::DrainOnce(char *buf, size_t cap, int32_t &idleMs, int32_t &closingMs)
{
    struct pollfd pfd = {};
    pfd.fd = readFd_;
    pfd.events = POLLIN;
    int rc = poll(&pfd, 1, RELAY_POLL_TIMEOUT_MS);
    if (closing_.load()) {
        // Every legitimate writer is already closed at this point, so the drain only
        // waits for the buffered tail plus EOF. Bound the TOTAL wait as well: the
        // idle-based grace alone is defeated by a remote that keeps writing.
        closingMs += RELAY_POLL_TIMEOUT_MS;
        if (closingMs >= RELAY_EOF_HARD_LIMIT_MS) {
            producerLeak_.store(true);
            DUMPER_HILOGE(MODULE_COMMON, "producer fd still open %{public}d ms after closing the write end "
                "while data keeps arriving, the remote writes periodically, stop relaying", closingMs);
            return false;
        }
    }
    if (rc < 0) {
        if (errno == EINTR) {
            return true;
        }
        DUMPER_HILOGE(MODULE_COMMON, "poll on readFd %{public}d failed, errno=%{public}d", readFd_, errno);
        return false;
    }
    if (rc == 0) {
        return CountIdleOnce(idleMs);
    }
    idleMs = 0;
    if ((pfd.revents & (POLLERR | POLLNVAL)) != 0) {
        DUMPER_HILOGE(MODULE_COMMON, "pipe error, revents=0x%{public}x", pfd.revents);
        return false;
    }
    return ForwardChunk(buf, cap);
}

// Silence is normal while the remote side is still working, so counting only starts
// once Finish() closed our write end, i.e. from the moment EOF became reachable.
bool FdRelay::CountIdleOnce(int32_t &idleMs)
{
    if (!closing_.load()) {
        return true;
    }
    idleMs += RELAY_POLL_TIMEOUT_MS;
    if (idleMs < RELAY_EOF_GRACE_MS) {
        return true;
    }
    producerLeak_.store(true);
    DUMPER_HILOGE(MODULE_COMMON, "no EOF %{public}d ms after closing the write end, "
        "the remote side kept the producer fd open, stop relaying", idleMs);
    return false;
}

bool FdRelay::ForwardChunk(char *buf, size_t cap)
{
    ssize_t n = read(readFd_, buf, cap);
    if (n < 0) {
        if (errno == EINTR) {
            return true;
        }
        DUMPER_HILOGE(MODULE_COMMON, "read from readFd %{public}d failed, errno=%{public}d", readFd_, errno);
        return false;
    }
    if (n == 0) {
        return false; // EOF, every writer closed its end
    }
    if (!WriteFully(outFd_, buf, static_cast<size_t>(n))) {
        broken_.store(true);
        DUMPER_HILOGW(MODULE_COMMON, "output fd %{public}d is gone, stop relaying", outFd_);
        return false;
    }
    relayed_.fetch_add(static_cast<uint64_t>(n));
    return true;
}

bool FdRelay::WriteFully(int fd, const char *buf, size_t len)
{
    if (fd < 0 || buf == nullptr) {
        return false;
    }
    size_t written = 0;
    while (written < len) {
        ssize_t n = write(fd, buf + written, len - written);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            DUMPER_HILOGE(MODULE_COMMON, "write to fd %{public}d failed, errno=%{public}d", fd, errno);
            return false;
        }
        if (n == 0) {
            DUMPER_HILOGE(MODULE_COMMON, "write to fd %{public}d made no progress", fd);
            return false;
        }
        written += static_cast<size_t>(n);
    }
    return true;
}

void FdRelay::CloseWriteEnd()
{
    std::lock_guard<std::mutex> lock(writeMutex_);
    CloseFd(writeFd_);
}

void FdRelay::CloseReadEnd()
{
    CloseFd(readFd_);
}

void FdRelay::CloseFd(int &fd)
{
    if (fd < 0) {
        return;
    }
    int target = fd;
    fd = -1;
    fdsan_exchange_owner_tag(target, 0, FDTAG);
    fdsan_close_with_tag(target, FDTAG);
}
} // namespace HiviewDFX
} // namespace OHOS
