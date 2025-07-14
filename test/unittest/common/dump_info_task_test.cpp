/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "task/system_info/device_info_task.h"
#include "task/system_info/kernel_module_info_task.h"
#include "task/system_info/system_cluster_info_task.h"
#include "task/system_info/wakeup_sources_info_task.h"
#include "data_inventory.h"
#include "dump_context.h"

using namespace testing::ext;
using namespace std;
namespace OHOS {
namespace HiviewDFX {
class DumpInfoTaskTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    DataInventory inventory_;
    std::shared_ptr<DumpContext> dumpContext_ = std::make_shared<DumpContext>(getpid(), getuid(), STDOUT_FILENO);
};

void DumpInfoTaskTest::SetUpTestCase(void)
{
}
void DumpInfoTaskTest::TearDownTestCase(void)
{
}
void DumpInfoTaskTest::SetUp(void)
{
}
void DumpInfoTaskTest::TearDown(void)
{
}


HWTEST_F(DumpInfoTaskTest, DeviceInfoTaskSuccess, TestSize.Level1)
{
    DeviceInfoTask task;
    DumpStatus status = task.TaskEntry(inventory_, dumpContext_);
    ASSERT_EQ(status, DUMP_OK);
    auto data = inventory_.GetPtr<std::vector<std::string>>(DataId::DEVICE_INFO);
    ASSERT_NE(data, nullptr);
    ASSERT_FALSE(data->empty());

    data = inventory_.GetPtr<std::vector<std::string>>(DataId::PROC_VERSION_INFO);
    ASSERT_NE(data, nullptr);
    ASSERT_FALSE(data->empty());

    data = inventory_.GetPtr<std::vector<std::string>>(DataId::PROC_CMDLINE_INFO);
    ASSERT_NE(data, nullptr);
    ASSERT_FALSE(data->empty());

    data = inventory_.GetPtr<std::vector<std::string>>(DataId::UPTIME_INFO);
    ASSERT_NE(data, nullptr);
    ASSERT_FALSE(data->empty());
}

HWTEST_F(DumpInfoTaskTest, KernelModuleInfoTaskSuccess, TestSize.Level1)
{
    KernelModuleInfoTask task;
    DumpStatus status = task.TaskEntry(inventory_, dumpContext_);
    ASSERT_EQ(status, DUMP_OK);
    auto data = inventory_.GetPtr<std::vector<std::string>>(DataId::PRINTENV_INFO);
    ASSERT_NE(data, nullptr);
    ASSERT_FALSE(data->empty());

    data = inventory_.GetPtr<std::vector<std::string>>(DataId::LSMOD_INFO);
    ASSERT_NE(data, nullptr);
    ASSERT_FALSE(data->empty());

#if defined(_WIN64) || defined(WIN64) || defined(__LP64__) || defined(_LP64)
    data = inventory_.GetPtr<std::vector<std::string>>(DataId::PROC_MODULES_INFO);
    ASSERT_NE(data, nullptr);
    ASSERT_FALSE(data->empty());
#endif
}

HWTEST_F(DumpInfoTaskTest, SystemClusterInfoTaskSuccess, TestSize.Level1)
{
    SystemClusterInfoTask task;
    DumpStatus status = task.TaskEntry(inventory_, dumpContext_);
    ASSERT_EQ(status, DUMP_OK);
    auto data = inventory_.GetPtr<std::vector<std::string>>(DataId::SYSTEM_CLUSTER_INFO);
    ASSERT_NE(data, nullptr);
    ASSERT_FALSE(data->empty());
}

HWTEST_F(DumpInfoTaskTest, WakeupSourcesInfoTaskSuccess, TestSize.Level1)
{
    WakeupSourcesInfoTask task;
    DumpStatus status = task.TaskEntry(inventory_, dumpContext_);
    ASSERT_EQ(status, DUMP_OK);
    auto data = inventory_.GetPtr<std::vector<std::string>>(DataId::WAKEUP_SOURCES_INFO);
    ASSERT_NE(data, nullptr);
    ASSERT_FALSE(data->empty());
}
} // namespace HiviewDFX
} // namespace OHOS