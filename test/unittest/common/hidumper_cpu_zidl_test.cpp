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
#include "dump_broker_cpu_stub.h"
#include "dump_cpu_data.h"
#include "hidumper_cpu_service_ipc_interface_code.h"
using namespace std;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::HiviewDFX;
namespace OHOS {
namespace HiviewDFX {
class HidumperCpuZidlTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

class TestDumpBrokerCpuStub : public DumpBrokerCpuStub {
public:
    TestDumpBrokerCpuStub() {};
    virtual ~TestDumpBrokerCpuStub() {};
    int32_t Request(DumpCpuData &dumpCpuData)
    {
        return 0;
    }
    int32_t GetCpuUsageByPid(int32_t pid, double &cpuUsage)
    {
        return 0;
    }
};

void HidumperCpuZidlTest::SetUpTestCase(void)
{
}

void HidumperCpuZidlTest::TearDownTestCase(void)
{
}

void HidumperCpuZidlTest::SetUp(void)
{
}

void HidumperCpuZidlTest::TearDown(void)
{
}

/**
 * @tc.name: HidumperCpuZidlTest001
 * @tc.desc: Test cpu service stub.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperCpuZidlTest, HidumperCpuZidlTest001, TestSize.Level3)
{
    DumpBrokerCpuStub* stub = new(std::nothrow) TestDumpBrokerCpuStub();
    MessageParcel data, reply;
    MessageOption option;
    int ret = stub->OnRemoteRequest(static_cast<uint32_t>(HidumperCpuServiceInterfaceCode::DUMP_REQUEST_CPUINFO),
        data, reply, option);
    ASSERT_TRUE(ret != 0);
    stub->OnRemoteRequest(static_cast<uint32_t>(HidumperCpuServiceInterfaceCode::DUMP_USAGE_ONLY),
        data, reply, option);
    ret = stub->DumpCpuUsageOnly(data, reply);
    ASSERT_TRUE(ret != 0);
}
} // namespace HiviewDFX
} // namespace OHOS