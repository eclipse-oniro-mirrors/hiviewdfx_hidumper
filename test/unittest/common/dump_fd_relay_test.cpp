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

#include <gtest/gtest.h>
#include <atomic>
#include <chrono>
#include <dirent.h>
#include <fcntl.h>
#include <fstream>
#include <sstream>
#include <sys/resource.h>
#include <sys/stat.h>
#include <thread>
#include <unistd.h>
#include <vector>

#include "directory_ex.h"
#include "util/dump_fd_relay.h"

using namespace std;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::HiviewDFX;

namespace OHOS {
namespace HiviewDFX {
namespace {
const std::string RELAY_TEST_ROOT = "/data/local/tmp/hidumper_relay_test/";
const size_t RELAY_LARGE_PAYLOAD_SIZE = 256 * 1024;
const int RELAY_REPEAT_TIMES = 20;
const int RELAY_WAIT_STEP_MS = 20;
const int RELAY_WAIT_MAX_MS = 3000;
const int RELAY_LEAK_SLACK_MS = 3000;
// REQUEST_MAX in dump_manager_service.cpp: five dump requests may be in flight at once.
const int RELAY_CONC_INSTANCES = 5;
const size_t RELAY_CONC_PAYLOAD = 192 * 1024;
const int RELAY_WRITER_THREADS = 4;
const int RELAY_RECORDS_PER_WRITER = 500;
const int RELAY_RECORD_LEN = 64;
const int RELAY_SLOW_CHUNKS = 8;
const int RELAY_SLOW_GAP_MS = 400;
const int RELAY_SLOW_RECORD_LEN = 48;
const size_t RELAY_BROKEN_PAYLOAD = 1024 * 1024;
const rlim_t RELAY_TIGHT_NOFILE = 32;
const size_t RELAY_BALLAST_CAP = 128;
const rlim_t RELAY_THREAD_SLACK = 0;
const std::string RELAY_HEADER = "-----[relay-header]-----\n";
const std::string RELAY_BODY = "capacity: 100\n";

std::string ReadWholeFile(const std::string &path)
{
    std::ifstream ifs(path);
    std::stringstream ss;
    ss << ifs.rdbuf();
    return ss.str();
}

int CountThreads()
{
    DIR *dir = opendir("/proc/self/task");
    if (dir == nullptr) {
        return -1;
    }
    int count = 0;
    struct dirent *entry = nullptr;
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_name[0] != '.') {
            count++;
        }
    }
    closedir(dir);
    return count;
}

int CountOpenFds()
{
    int count = 0;
    DIR *dir = opendir("/proc/self/fd");
    if (dir == nullptr) {
        return -1;
    }
    while (readdir(dir) != nullptr) {
        count++;
    }
    closedir(dir);
    // opendir itself holds one fd
    return count - 1;
}

// A remote SA never sees the relay's own write end: it gets a dup installed by the
// binder driver and IPCObjectStub::OnRemoteDump closes that dup right after Dump
// returns. Mirror that exactly, otherwise the test would close FdRelay's write end
// and Finish() would close it a second time.
int DupProducer(int producerFd)
{
    return dup(producerFd);
}

void WriteAllAndClose(int localFd, const std::string &payload, std::atomic<bool> *gotEpipe)
{
    size_t written = 0;
    while (written < payload.size()) {
        ssize_t n = write(localFd, payload.c_str() + written, payload.size() - written);
        if (n < 0) {
            if ((errno == EPIPE) && (gotEpipe != nullptr)) {
                gotEpipe->store(true);
            }
            break;
        }
        if (n == 0) {
            break;
        }
        written += static_cast<size_t>(n);
    }
    close(localFd);
}

// Emulate a remote SA: dup the producer fd, write from another thread, close the dup.
void ProduceAndClose(int producerFd, const std::string &payload)
{
    int local = DupProducer(producerFd);
    if (local < 0) {
        return;
    }
    std::thread worker([local, payload]() {
        WriteAllAndClose(local, payload, nullptr);
    });
    worker.join();
}

// Same, but hands the running thread back so the caller can overlap it with Finish().
std::thread ProduceAsync(int producerFd, const std::string &payload, std::atomic<bool> *gotEpipe)
{
    int local = DupProducer(producerFd);
    if (local < 0) {
        return std::thread();
    }
    return std::thread([local, payload, gotEpipe]() {
        WriteAllAndClose(local, payload, gotEpipe);
    });
}

// A producer that keeps emitting slowly, to prove the EOF grace window does not
// cut off a peer that is merely thinking. Returns the expected content via expect.
std::thread ProduceSlowly(int producerFd, int chunks, int gapMs, int recordLen, std::string &expect)
{
    int local = DupProducer(producerFd);
    if (local < 0) {
        return std::thread();
    }
    for (int i = 0; i < chunks; i++) {
        std::string rec = "chunk-" + std::to_string(i);
        if (recordLen > static_cast<int>(rec.size()) + 1) {
            rec += std::string(static_cast<size_t>(recordLen - 1) - rec.size(), '.');
        }
        rec += '\n';
        expect += rec;
    }
    const std::string payload = expect;
    return std::thread([local, chunks, gapMs, payload]() {
        size_t one = payload.size() / static_cast<size_t>(chunks);
        for (int i = 0; i < chunks; i++) {
            std::string rec = payload.substr(static_cast<size_t>(i) * one, one);
            ssize_t n = write(local, rec.c_str(), rec.size());
            if (n != static_cast<ssize_t>(rec.size())) {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(gapMs));
        }
        close(local);
    });
}

std::string BuildRecord(int tid, int seq)
{
    std::string rec = std::to_string(tid) + "-" + std::to_string(seq);
    rec += std::string(static_cast<size_t>(RELAY_RECORD_LEN - 1) - rec.size(), '.');
    rec += '\n';
    return rec;
}

// Thread body of the concurrent Write test: fixed size records, so any interleaving
// or loss shows up as a malformed record.
void WriteRecords(FdRelay *relay, int tid, std::atomic<int> *failedWrites)
{
    for (int seq = 0; seq < RELAY_RECORDS_PER_WRITER; seq++) {
        if (!relay->Write(BuildRecord(tid, seq))) {
            failedWrites->fetch_add(1);
        }
    }
}

bool AllRecordsSeen(const std::vector<std::vector<bool>> &seen)
{
    for (const auto &row : seen) {
        for (bool one : row) {
            if (!one) {
                return false;
            }
        }
    }
    return true;
}

// Every RELAY_RECORD_LEN sized slot must parse as "tid-seq...\n", appear exactly once,
// and together the slots must cover the whole tid x seq matrix.
bool RecordsIntact(const std::string &got)
{
    std::vector<std::vector<bool>> seen(static_cast<size_t>(RELAY_WRITER_THREADS),
        std::vector<bool>(RELAY_RECORDS_PER_WRITER, false));
    for (size_t off = 0; off + RELAY_RECORD_LEN <= got.size(); off += RELAY_RECORD_LEN) {
        std::string rec = got.substr(off, RELAY_RECORD_LEN);
        size_t dash = rec.find('-');
        if ((rec[RELAY_RECORD_LEN - 1] != '\n') || (dash == std::string::npos)) {
            return false;
        }
        int tid = atoi(rec.substr(0, dash).c_str());
        int seq = atoi(rec.substr(dash + 1).c_str());
        if ((tid < 0) || (tid >= RELAY_WRITER_THREADS) || (seq < 0) || (seq >= RELAY_RECORDS_PER_WRITER)) {
            return false;
        }
        if (seen[static_cast<size_t>(tid)][static_cast<size_t>(seq)]) {
            return false;
        }
        seen[static_cast<size_t>(tid)][static_cast<size_t>(seq)] = true;
    }
    return AllRecordsSeen(seen);
}
} // namespace

class DumpFdRelayTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DumpFdRelayTest::SetUpTestCase(void)
{
    ForceCreateDirectory(RELAY_TEST_ROOT);
}

void DumpFdRelayTest::TearDownTestCase(void)
{
    ForceRemoveDirectory(RELAY_TEST_ROOT);
}

void DumpFdRelayTest::SetUp(void)
{
}

void DumpFdRelayTest::TearDown(void)
{
}

/**
 * @tc.name: DumpFdRelayTest001
 * @tc.desc: FdRelay refuses to start with an invalid output fd.
 * @tc.type: FUNC
 */
HWTEST_F(DumpFdRelayTest, DumpFdRelayTest001, TestSize.Level1)
{
    FdRelay relay(-1);
    ASSERT_FALSE(relay.Start());
    ASSERT_EQ(relay.GetProducerFd(), -1);
    ASSERT_EQ(relay.GetRelayedBytes(), 0U);
    relay.Finish();
}

/**
 * @tc.name: DumpFdRelayTest002
 * @tc.desc: Data written by the producer reaches the output fd.
 * @tc.type: FUNC
 */
HWTEST_F(DumpFdRelayTest, DumpFdRelayTest002, TestSize.Level1)
{
    std::string path = RELAY_TEST_ROOT + "relay_basic.txt";
    int outFd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    ASSERT_GE(outFd, 0);

    FdRelay relay(outFd);
    ASSERT_TRUE(relay.Start());
    ProduceAndClose(relay.GetProducerFd(), RELAY_BODY);
    relay.Finish();
    close(outFd);

    ASSERT_EQ(ReadWholeFile(path), RELAY_BODY);
    ASSERT_EQ(relay.GetRelayedBytes(), RELAY_BODY.size());
    ASSERT_FALSE(relay.IsOutputBroken());
}

/**
 * @tc.name: DumpFdRelayTest003
 * @tc.desc: A payload far beyond the 64KiB pipe capacity is relayed without deadlock or loss.
 * @tc.type: FUNC
 */
HWTEST_F(DumpFdRelayTest, DumpFdRelayTest003, TestSize.Level1)
{
    std::string path = RELAY_TEST_ROOT + "relay_large.txt";
    int outFd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    ASSERT_GE(outFd, 0);

    std::string payload(RELAY_LARGE_PAYLOAD_SIZE, 'x');
    FdRelay relay(outFd);
    ASSERT_TRUE(relay.Start());
    ProduceAndClose(relay.GetProducerFd(), payload);
    relay.Finish();
    close(outFd);

    ASSERT_EQ(ReadWholeFile(path).size(), RELAY_LARGE_PAYLOAD_SIZE);
    ASSERT_EQ(relay.GetRelayedBytes(), RELAY_LARGE_PAYLOAD_SIZE);
}

/**
 * @tc.name: DumpFdRelayTest004
 * @tc.desc: Header queued through Write keeps its order against producer data.
 * @tc.type: FUNC
 */
HWTEST_F(DumpFdRelayTest, DumpFdRelayTest004, TestSize.Level1)
{
    std::string path = RELAY_TEST_ROOT + "relay_order.txt";
    int outFd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    ASSERT_GE(outFd, 0);

    FdRelay relay(outFd);
    ASSERT_TRUE(relay.Start());
    ASSERT_TRUE(relay.Write(RELAY_HEADER));
    ProduceAndClose(relay.GetProducerFd(), RELAY_BODY);
    relay.Finish();
    close(outFd);

    ASSERT_EQ(ReadWholeFile(path), RELAY_HEADER + RELAY_BODY);
}

/**
 * @tc.name: DumpFdRelayTest005
 * @tc.desc: A vanished consumer is reported and does not hang Finish nor kill the process.
 * @tc.type: FUNC
 */
HWTEST_F(DumpFdRelayTest, DumpFdRelayTest005, TestSize.Level1)
{
    int pipeFds[2] = {-1, -1};
    ASSERT_EQ(pipe(pipeFds), 0);
    // Simulate "hidumper ... | grep -m1 x": the reader is already gone.
    close(pipeFds[0]);

    FdRelay relay(pipeFds[1]);
    ASSERT_TRUE(relay.Start());
    ProduceAndClose(relay.GetProducerFd(), RELAY_BODY);
    relay.Finish();
    close(pipeFds[1]);

    ASSERT_TRUE(relay.IsOutputBroken());
}

/**
 * @tc.name: DumpFdRelayTest006
 * @tc.desc: Repeated Start/Finish cycles do not leak file descriptors.
 * @tc.type: FUNC
 */
HWTEST_F(DumpFdRelayTest, DumpFdRelayTest006, TestSize.Level1)
{
    std::string path = RELAY_TEST_ROOT + "relay_leak.txt";
    int outFd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    ASSERT_GE(outFd, 0);

    int before = CountOpenFds();
    ASSERT_GE(before, 0);
    for (int i = 0; i < RELAY_REPEAT_TIMES; i++) {
        FdRelay relay(outFd);
        ASSERT_TRUE(relay.Start());
        ProduceAndClose(relay.GetProducerFd(), RELAY_BODY);
        relay.Finish();
    }

    int after = before;
    int waited = 0;
    while (waited < RELAY_WAIT_MAX_MS) {
        after = CountOpenFds();
        if (after <= before) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(RELAY_WAIT_STEP_MS));
        waited += RELAY_WAIT_STEP_MS;
    }
    close(outFd);
    ASSERT_LE(after, before);
}

/**
 * @tc.name: DumpFdRelayTest007
 * @tc.desc: Finish stays bounded when the producer keeps a copy of the fd open.
 * @tc.type: FUNC
 */
HWTEST_F(DumpFdRelayTest, DumpFdRelayTest007, TestSize.Level1)
{
    std::string path = RELAY_TEST_ROOT + "relay_producer_leak.txt";
    int outFd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    ASSERT_GE(outFd, 0);

    FdRelay relay(outFd);
    ASSERT_TRUE(relay.Start());
    // Mimic a remote implementation that dups the fd it received over binder and forgets it,
    // so that EOF can never arrive on the relay pipe.
    int leakedFd = dup(relay.GetProducerFd());
    ASSERT_GE(leakedFd, 0);
    ASSERT_TRUE(relay.Write(RELAY_BODY));

    auto begin = std::chrono::steady_clock::now();
    relay.Finish();
    int elapsed = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - begin).count());

    close(leakedFd);
    close(outFd);

    ASSERT_TRUE(relay.IsProducerLeakSuspected());
    ASSERT_FALSE(relay.IsOutputBroken());
    ASSERT_LT(elapsed, RELAY_EOF_GRACE_MS + RELAY_LEAK_SLACK_MS);
    // Data already buffered before Finish must still land, the grace window drains it.
    ASSERT_EQ(ReadWholeFile(path), RELAY_BODY);
}

/**
 * @tc.name: DumpFdRelayTest008
 * @tc.desc: Several relays running at once stay independent, as with concurrent dump requests.
 * @tc.type: FUNC
 */
HWTEST_F(DumpFdRelayTest, DumpFdRelayTest008, TestSize.Level1)
{
    std::vector<std::string> paths;
    std::vector<int> outFds;
    for (int i = 0; i < RELAY_CONC_INSTANCES; i++) {
        std::string path = RELAY_TEST_ROOT + "relay_conc_" + std::to_string(i) + ".txt";
        int fd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        ASSERT_GE(fd, 0);
        paths.push_back(path);
        outFds.push_back(fd);
    }

    std::atomic<int> okCount {0};
    std::vector<std::thread> workers;
    for (int i = 0; i < RELAY_CONC_INSTANCES; i++) {
        workers.emplace_back([i, &outFds, &okCount]() {
            std::string payload(RELAY_CONC_PAYLOAD, static_cast<char>('a' + i));
            FdRelay relay(outFds[static_cast<size_t>(i)]);
            if (!relay.Start()) {
                return;
            }
            std::thread sa = ProduceAsync(relay.GetProducerFd(), payload, nullptr);
            sa.join();
            relay.Finish();
            if (relay.GetRelayedBytes() == RELAY_CONC_PAYLOAD) {
                okCount.fetch_add(1);
            }
        });
    }
    for (auto &w : workers) {
        w.join();
    }
    for (int fd : outFds) {
        close(fd);
    }

    ASSERT_EQ(okCount.load(), RELAY_CONC_INSTANCES);
    for (int i = 0; i < RELAY_CONC_INSTANCES; i++) {
        std::string got = ReadWholeFile(paths[i]);
        ASSERT_EQ(got.size(), RELAY_CONC_PAYLOAD);
        // No cross talk between the concurrent relays.
        ASSERT_EQ(got.find_first_not_of(static_cast<char>('a' + i)), std::string::npos);
    }
}

/**
 * @tc.name: DumpFdRelayTest009
 * @tc.desc: Concurrent Write callers keep every record intact, none is interleaved.
 * @tc.type: FUNC
 */
HWTEST_F(DumpFdRelayTest, DumpFdRelayTest009, TestSize.Level1)
{
    std::string path = RELAY_TEST_ROOT + "relay_conc_write.txt";
    int outFd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    ASSERT_GE(outFd, 0);

    FdRelay relay(outFd);
    ASSERT_TRUE(relay.Start());

    std::vector<std::thread> writers;
    std::atomic<int> failedWrites {0};
    for (int t = 0; t < RELAY_WRITER_THREADS; t++) {
        writers.emplace_back(WriteRecords, &relay, t, &failedWrites);
    }
    for (auto &w : writers) {
        w.join();
    }
    relay.Finish();
    close(outFd);

    const size_t expectSize =
        static_cast<size_t>(RELAY_WRITER_THREADS) * RELAY_RECORDS_PER_WRITER * RELAY_RECORD_LEN;
    std::string got = ReadWholeFile(path);
    ASSERT_EQ(failedWrites.load(), 0);
    ASSERT_EQ(got.size(), expectSize);
    ASSERT_EQ(relay.GetRelayedBytes(), expectSize);
    ASSERT_TRUE(RecordsIntact(got));
}

/**
 * @tc.name: DumpFdRelayTest010
 * @tc.desc: Finish is idempotent and the destructor alone drains the relay.
 * @tc.type: FUNC
 */
HWTEST_F(DumpFdRelayTest, DumpFdRelayTest010, TestSize.Level1)
{
    std::string path = RELAY_TEST_ROOT + "relay_idempotent.txt";
    int outFd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    ASSERT_GE(outFd, 0);
    {
        FdRelay relay(outFd);
        ASSERT_TRUE(relay.Start());
        ProduceAndClose(relay.GetProducerFd(), RELAY_BODY);
        relay.Finish();
        relay.Finish();
        relay.Finish();
        ASSERT_EQ(relay.GetRelayedBytes(), RELAY_BODY.size());
    }
    ASSERT_EQ(ReadWholeFile(path), RELAY_BODY);

    // Second scenario: never call Finish, rely on the destructor alone.
    int outFd2 = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    ASSERT_GE(outFd2, 0);
    {
        FdRelay relay(outFd2);
        ASSERT_TRUE(relay.Start());
        ProduceAndClose(relay.GetProducerFd(), RELAY_HEADER);
    }
    close(outFd);
    close(outFd2);
    ASSERT_EQ(ReadWholeFile(path), RELAY_HEADER);
}

/**
 * @tc.name: DumpFdRelayTest011
 * @tc.desc: A producer still writing is released with EPIPE once the consumer is gone.
 * @tc.type: FUNC
 */
HWTEST_F(DumpFdRelayTest, DumpFdRelayTest011, TestSize.Level1)
{
    int pipeFds[2] = {-1, -1};
    ASSERT_EQ(pipe(pipeFds), 0);
    close(pipeFds[0]);

    std::atomic<bool> gotEpipe {false};
    std::string payload(RELAY_BROKEN_PAYLOAD, 'x');
    FdRelay relay(pipeFds[1]);
    ASSERT_TRUE(relay.Start());
    std::thread sa = ProduceAsync(relay.GetProducerFd(), payload, &gotEpipe);

    int waited = 0;
    while (!gotEpipe.load() && (waited < RELAY_WAIT_MAX_MS)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(RELAY_WAIT_STEP_MS));
        waited += RELAY_WAIT_STEP_MS;
    }
    relay.Finish();
    sa.join();
    close(pipeFds[1]);

    ASSERT_TRUE(gotEpipe.load());
    ASSERT_TRUE(relay.IsOutputBroken());
    ASSERT_LT(relay.GetRelayedBytes(), RELAY_BROKEN_PAYLOAD);
}

/**
 * @tc.name: DumpFdRelayTest012
 * @tc.desc: A slow but live producer is never mistaken for a leaked one.
 * @tc.type: FUNC
 */
HWTEST_F(DumpFdRelayTest, DumpFdRelayTest012, TestSize.Level1)
{
    std::string path = RELAY_TEST_ROOT + "relay_slow.txt";
    int outFd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    ASSERT_GE(outFd, 0);

    FdRelay relay(outFd);
    ASSERT_TRUE(relay.Start());
    std::string expect;
    // RELAY_SLOW_GAP_MS is above the poll timeout but well below the EOF grace window.
    std::thread sa = ProduceSlowly(relay.GetProducerFd(), RELAY_SLOW_CHUNKS,
                                   RELAY_SLOW_GAP_MS, RELAY_SLOW_RECORD_LEN, expect);
    relay.Finish();
    sa.join();
    close(outFd);

    ASSERT_FALSE(relay.IsProducerLeakSuspected());
    ASSERT_FALSE(relay.IsOutputBroken());
    ASSERT_EQ(ReadWholeFile(path), expect);
    ASSERT_EQ(relay.GetRelayedBytes(), expect.size());
}

/**
 * @tc.name: DumpFdRelayTest013
 * @tc.desc: Write refuses at once on a broken relay instead of blocking on a full pipe.
 * @tc.type: FUNC
 */
HWTEST_F(DumpFdRelayTest, DumpFdRelayTest013, TestSize.Level1)
{
    int pipeFds[2] = {-1, -1};
    ASSERT_EQ(pipe(pipeFds), 0);
    close(pipeFds[0]);

    std::atomic<bool> gotEpipe {false};
    std::string payload(RELAY_BROKEN_PAYLOAD, 'y');
    FdRelay relay(pipeFds[1]);
    ASSERT_TRUE(relay.Start());
    std::thread sa = ProduceAsync(relay.GetProducerFd(), payload, &gotEpipe);

    int waited = 0;
    while (!relay.IsOutputBroken() && (waited < RELAY_WAIT_MAX_MS)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(RELAY_WAIT_STEP_MS));
        waited += RELAY_WAIT_STEP_MS;
    }
    ASSERT_TRUE(relay.IsOutputBroken());

    auto begin = std::chrono::steady_clock::now();
    bool wrote = relay.Write(std::string(256 * 1024, 'H'));
    int elapsed = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - begin).count());

    relay.Finish();
    sa.join();
    close(pipeFds[1]);

    ASSERT_FALSE(wrote);
    ASSERT_LT(elapsed, 1000);
}

/**
 * @tc.name: DumpFdRelayTest014
 * @tc.desc: Start is idempotent, Write guards its preconditions, an empty producer ends at once.
 * @tc.type: FUNC
 */
HWTEST_F(DumpFdRelayTest, DumpFdRelayTest014, TestSize.Level1)
{
    std::string path = RELAY_TEST_ROOT + "relay_precond.txt";
    int outFd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    ASSERT_GE(outFd, 0);

    FdRelay relay(outFd);
    ASSERT_FALSE(relay.Write(RELAY_BODY));          // not started yet
    ASSERT_TRUE(relay.Start());
    int firstProducer = relay.GetProducerFd();
    ASSERT_GE(firstProducer, 0);
    ASSERT_TRUE(relay.Start());                     // second Start must not rebuild the pipe
    ASSERT_EQ(relay.GetProducerFd(), firstProducer);
    ASSERT_FALSE(relay.Write(""));                  // empty payload

    ProduceAndClose(relay.GetProducerFd(), "");      // producer emits nothing at all
    relay.Finish();
    close(outFd);

    ASSERT_EQ(relay.GetRelayedBytes(), 0U);
    ASSERT_FALSE(relay.IsProducerLeakSuspected());
    ASSERT_EQ(ReadWholeFile(path), "");
    ASSERT_FALSE(relay.Write(RELAY_BODY));          // after Finish
}

/**
 * @tc.name: DumpFdRelayTest015
 * @tc.desc: Start reports failure when a pipe cannot be created, so the caller can fall back.
 * @tc.type: FUNC
 */
HWTEST_F(DumpFdRelayTest, DumpFdRelayTest015, TestSize.Level1)
{
    std::string path = RELAY_TEST_ROOT + "relay_nofd.txt";
    int outFd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    ASSERT_GE(outFd, 0);

    // Lower the fd ceiling first, then drain what is left. Only lowering RLIMIT_NOFILE
    // is not enough because it bounds the fd number and a sparse table would still
    // leave low numbers free; only draining is not enough because the ceiling may be
    // far too high to reach in a unit test.
    struct rlimit saved = {};
    ASSERT_EQ(getrlimit(RLIMIT_NOFILE, &saved), 0);
    struct rlimit tight = saved;
    tight.rlim_cur = RELAY_TIGHT_NOFILE;
    ASSERT_EQ(setrlimit(RLIMIT_NOFILE, &tight), 0);

    std::vector<int> ballast;
    while (ballast.size() < RELAY_BALLAST_CAP) {
        int fd = open("/dev/null", O_RDONLY);
        if (fd < 0) {
            break;
        }
        ballast.push_back(fd);
    }
    int probe = open("/dev/null", O_RDONLY);
    if (probe >= 0) {
        close(probe);
    }

    FdRelay relay(outFd);
    bool started = relay.Start();
    bool wroteWhileExhausted = relay.Write(RELAY_BODY);

    for (int fd : ballast) {
        close(fd);
    }
    ballast.clear();
    ASSERT_EQ(setrlimit(RLIMIT_NOFILE, &saved), 0);

    ASSERT_LT(probe, 0);
    ASSERT_FALSE(started);
    ASSERT_EQ(relay.GetProducerFd(), -1);
    ASSERT_FALSE(wroteWhileExhausted);
    relay.Finish();
    close(outFd);

    // With the fds back the relay works again, and the dump is delivered either way.
    int outFd2 = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    ASSERT_GE(outFd2, 0);
    FdRelay recovered(outFd2);
    ASSERT_TRUE(recovered.Start());
    ProduceAndClose(recovered.GetProducerFd(), RELAY_BODY);
    recovered.Finish();
    close(outFd2);
    ASSERT_EQ(ReadWholeFile(path), RELAY_BODY);
}

/**
 * @tc.name: DumpFdRelayTest016
 * @tc.desc: Start degrades to a fallback instead of aborting when a thread cannot be created.
 * @tc.type: FUNC
 */
HWTEST_F(DumpFdRelayTest, DumpFdRelayTest016, TestSize.Level1)
{
    // copy_process() only enforces RLIMIT_NPROC when the caller lacks both
    // CAP_SYS_RESOURCE and CAP_SYS_ADMIN, so as root the limit is bypassed and
    // pthread_create keeps succeeding. The branch is then unreachable here.
    if (getuid() == 0) {
        GTEST_SKIP() << "RLIMIT_NPROC is bypassed for root, run as non-root to cover this branch";
    }

    std::string path = RELAY_TEST_ROOT + "relay_nothread.txt";
    int outFd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    ASSERT_GE(outFd, 0);

    int before = CountOpenFds();
    ASSERT_GT(before, 0);
    struct rlimit saved = {};
    ASSERT_EQ(getrlimit(RLIMIT_NPROC, &saved), 0);
    int threads = CountThreads();
    ASSERT_GT(threads, 0);

    // Pin RLIMIT_NPROC at the current thread count so pthread_create is refused.
    // Under -fno-exceptions a throwing std::thread constructor would terminate here.
    struct rlimit tight = saved;
    tight.rlim_cur = static_cast<rlim_t>(threads) + RELAY_THREAD_SLACK;
    if (setrlimit(RLIMIT_NPROC, &tight) != 0) {
        close(outFd);
        GTEST_SKIP() << "setrlimit(RLIMIT_NPROC) not permitted";
    }

    FdRelay relay(outFd);
    bool started = relay.Start();
    // RLIMIT_NPROC is per real uid, not per process, so restore it before asserting
    // anything: an early ASSERT return would otherwise leave the whole uid throttled.
    int restoreRet = setrlimit(RLIMIT_NPROC, &saved);
    bool wrote = relay.Write(RELAY_BODY);
    relay.Finish();
    close(outFd);

    ASSERT_EQ(restoreRet, 0);
    ASSERT_FALSE(started);
    ASSERT_EQ(relay.GetProducerFd(), -1);
    ASSERT_FALSE(wrote);
    ASSERT_LE(CountOpenFds(), before);

    // Recovery: the caller can dump straight into the fd again.
    int outFd2 = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    ASSERT_GE(outFd2, 0);
    FdRelay recovered(outFd2);
    ASSERT_TRUE(recovered.Start());
    ProduceAndClose(recovered.GetProducerFd(), RELAY_BODY);
    recovered.Finish();
    close(outFd2);
    ASSERT_EQ(ReadWholeFile(path), RELAY_BODY);
}
} // namespace HiviewDFX
} // namespace OHOS
