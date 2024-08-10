/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include <message_parcel.h>
#include "dump_broker_stub.h"
#include "hidumper_service_ipc_interface_code.h"
using namespace std;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::HiviewDFX;
namespace OHOS {
namespace HiviewDFX {
class HidumperZidlTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

class TestDumpBrokerStub : public DumpBrokerStub {
public:
    TestDumpBrokerStub() {};
    virtual ~TestDumpBrokerStub() {};
    int32_t Request(std::vector<std::u16string> &args, int outfd)
    {
        return 0;
    }
    int32_t ScanPidOverLimit(std::string requestType, int32_t limitSize, std::vector<int32_t> &pidList)
    {
        return 0;
    }
    int32_t CountFdNums(int32_t pid, uint32_t &fdNums, std::string &detailFdInfo, std::string &topLeakedType)
    {
        return 0;
    }
};

void HidumperZidlTest::SetUpTestCase(void)
{
}

void HidumperZidlTest::TearDownTestCase(void)
{
}

void HidumperZidlTest::SetUp(void)
{
}

void HidumperZidlTest::TearDown(void)
{
}

/**
 * @tc.name: HidumperZidlTest001
 * @tc.desc: Test hidumper service stub.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperZidlTest, HidumperZidlTest001, TestSize.Level3)
{
    DumpBrokerStub* stub = new(std::nothrow) TestDumpBrokerStub();
    MessageParcel data, reply;
    MessageOption option;
    stub->OnRemoteRequest(static_cast<uint32_t>(HidumperServiceInterfaceCode::DUMP_REQUEST_FILEFD),
        data, reply, option);
    ASSERT_TRUE(true);
    stub->OnRemoteRequest(static_cast<uint32_t>(HidumperServiceInterfaceCode::SCAN_PID_OVER_LIMIT),
        data, reply, option);
    ASSERT_TRUE(true);
    stub->OnRemoteRequest(static_cast<uint32_t>(HidumperServiceInterfaceCode::COUNT_FD_NUMS), data, reply, option);
    ASSERT_TRUE(true);
    stub->RequestFileFdStub(data, reply);
    stub->ScanPidOverLimitStub(data, reply);
    stub->CountFdNumsStub(data, reply);
}
} // namespace HiviewDFX
} // namespace OHOS