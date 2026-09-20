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
#ifndef HIDUMPER_UTIL_DUMP_FD_RELAY_H
#define HIDUMPER_UTIL_DUMP_FD_RELAY_H

#include <atomic>
#include <cstdint>
#include <mutex>
#include <pthread.h>
#include <string>

namespace OHOS {
namespace HiviewDFX {

constexpr size_t RELAY_BUF_SIZE = 4096;
constexpr int32_t RELAY_POLL_TIMEOUT_MS = 200;
// Upper bound for waiting on EOF once Finish() closed our write end. Only reached
// when the remote side kept a copy of the producer fd open.
constexpr int32_t RELAY_EOF_GRACE_MS = 2000;
// Must stay within the 16 byte pthread name limit including the terminator.
constexpr char RELAY_THREAD_NAME[] = "hidumper_relay";

/*
 * FdRelay owns an anonymous pipe created inside hidumper_service and forwards
 * everything written into it to the output fd supplied by the dump caller.
 *
 * The remote SA is handed the pipe write end instead of the caller fd. An
 * anonymous pipe inode in pipefs carries the SELinux label of the creating
 * task, so the write end is labelled u:r:hidumper_service:s0 rather than the
 * label of the shell/hdcd pipe that reached us over IPC. Every sadomain and
 * hdfdomain is already allowed to use and write such an fd through the
 * use_hidumper macro in
 * sepolicy/ohos_policy/hiviewdfx/hidumper/system/hidumper_service.te, which
 * makes the dump output survive pipes, redirections and release builds without
 * any per-domain policy exception.
 *
 * The draining runs on its own thread because IRemoteObject::Dump() is a
 * synchronous call: the SA writes while the call is in flight and would block
 * forever once the pipe buffer fills up.
 *
 * EOF is normally guaranteed: IPCObjectStub::OnRemoteDump closes the fd it
 * duped out of the parcel before the binder reply goes out, so Finish() sees
 * the drain thread exit right away. Should a remote implementation keep its
 * own copy open, Finish() would otherwise join forever, so once our write end
 * is closed the drain thread gives up after RELAY_EOF_GRACE_MS of silence.
 * The grace window is gated on that close for a reason: before it, a remote
 * that thinks for a while without emitting anything is perfectly normal and
 * must not be cut off. Timing the silence out beats a cancel channel here --
 * it needs no extra fd, keeps the draining able to pick up whatever is still
 * buffered, and tells "the peer is idle but still producing" apart from "the
 * peer went silent and will never send EOF".
 *
 * The draining thread is created with pthread_create rather than std::thread on
 * purpose: this component is built with -fno-exceptions, so a std::thread whose
 * creation fails would throw std::system_error straight into std::terminate and
 * take hidumper_service down. pthread_create reports the failure as a return
 * code, which lets Start() fall back to the direct fd instead.
 */
class FdRelay {
public:
    explicit FdRelay(int outFd);
    ~FdRelay();
    FdRelay(const FdRelay &) = delete;
    FdRelay &operator=(const FdRelay &) = delete;

    // Create the pipe and start the draining thread. Returns false when the
    // relay cannot be used, in which case the caller keeps the old behaviour.
    bool Start();
    // Write end handed to the remote SA. Owned by this object, never by the caller.
    int GetProducerFd() const;
    // Queue hidumper_service own output (section headers) into the pipe so that
    // the output fd keeps a single writer and the ordering stays deterministic.
    bool Write(const std::string &data);
    // Close the write end, wait for the draining thread, then release the pipe.
    void Finish();
    uint64_t GetRelayedBytes() const;
    // True once writing to the caller fd failed, for example because grep exited.
    bool IsOutputBroken() const;
    // True when the drain thread gave up waiting for EOF, which means the remote
    // side kept a copy of the producer fd open.
    bool IsProducerLeakSuspected() const;

private:
    static void *DrainThreadEntry(void *arg);
    void DrainLoop();
    bool DrainOnce(char *buf, size_t cap, int32_t &idleMs);
    bool CountIdleOnce(int32_t &idleMs);
    bool ForwardChunk(char *buf, size_t cap);
    void CloseWriteEnd();
    void CloseReadEnd();
    static bool WriteFully(int fd, const char *buf, size_t len);
    static void CloseFd(int &fd);

    int outFd_ {-1};
    int readFd_ {-1};
    int writeFd_ {-1};
    bool started_ {false};
    std::atomic<bool> broken_ {false};
    // Set by Finish() after our write end is closed, i.e. once EOF became reachable.
    std::atomic<bool> closing_ {false};
    std::atomic<bool> producerLeak_ {false};
    std::atomic<uint64_t> relayed_ {0};
    pthread_t worker_ {};
    // Guards pthread_join: joining a pthread_t that was never created is undefined.
    bool threadStarted_ {false};
    std::mutex writeMutex_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIDUMPER_UTIL_DUMP_FD_RELAY_H
