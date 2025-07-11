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
#include "task/storage/disk_info_task.h"
#include "task/storage/iotop_info_task.h"
#include "task/storage/lsof_info_task.h"
#include "task/storage/mounts_info_task.h"
#include "task/storage/storage_io_info_task.h"
#include "data_inventory.h"
#include "dump_context.h"

using namespace testing::ext;
using namespace std;
namespace OHOS {
namespace HiviewDFX {
class StorageInfoTaskTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    DataInventory inventory_;
    std::shared_ptr<DumpContext> dumpContext_ = std::make_shared<DumpContext>(getpid(), getuid(), STDOUT_FILENO);
};

void StorageInfoTaskTest::SetUpTestCase(void)
{
}
void StorageInfoTaskTest::TearDownTestCase(void)
{
}
void StorageInfoTaskTest::SetUp(void)
{
}
void StorageInfoTaskTest::TearDown(void)
{
}

HWTEST_F(StorageInfoTaskTest, DiskInfoTaskSuccess, TestSize.Level1)
{
    DiskInfoTask task;
    DumpStatus status = task.TaskEntry(inventory_, dumpContext_);
    ASSERT_EQ(status, DUMP_OK);
    auto data = inventory_.GetPtr<std::vector<std::string>>(DataId::STORAGE_STATE_INFO);
    ASSERT_NE(data, nullptr);
    ASSERT_FALSE(data->empty());

    data = inventory_.GetPtr<std::vector<std::string>>(DataId::DF_INFO);
    ASSERT_NE(data, nullptr);
    ASSERT_FALSE(data->empty());
}


HWTEST_F(StorageInfoTaskTest, IoTopInfoTaskSuccess, TestSize.Level1)
{
    IoTopInfoTask task;
    DumpStatus status = task.TaskEntry(inventory_, dumpContext_);
    ASSERT_EQ(status, DUMP_OK);
    auto data = inventory_.GetPtr<std::vector<std::string>>(DataId::IOTOP_INFO);
    ASSERT_NE(data, nullptr);
    ASSERT_FALSE(data->empty());
}

HWTEST_F(StorageInfoTaskTest, LsofInfoTaskSuccess, TestSize.Level1)
{
    LsofInfoTask task;
    DumpStatus status = task.TaskEntry(inventory_, dumpContext_);
    ASSERT_EQ(status, DUMP_OK);
    auto data = inventory_.GetPtr<std::vector<std::string>>(DataId::LSOF_INFO);
    ASSERT_NE(data, nullptr);
    ASSERT_FALSE(data->empty());
}

HWTEST_F(StorageInfoTaskTest, MountsInfoTaskSuccess, TestSize.Level1)
{
    MountsInfoTask task;
    DumpStatus status = task.TaskEntry(inventory_, dumpContext_);
    ASSERT_EQ(status, DUMP_OK);
    auto data = inventory_.GetPtr<std::vector<std::string>>(DataId::PROC_MOUNTS_INFO);
    ASSERT_NE(data, nullptr);
    ASSERT_FALSE(data->empty());
}

HWTEST_F(StorageInfoTaskTest, StorageIoInfoTaskSuccess, TestSize.Level1)
{
    StorageIoInfoTask task;
    DumpStatus status = task.TaskEntry(inventory_, dumpContext_);
    ASSERT_EQ(status, DUMP_OK);
    auto data = inventory_.GetPtr<std::vector<std::string>>(DataId::PROC_PID_IO_INFO);
    ASSERT_NE(data, nullptr);
    ASSERT_FALSE(data->empty());
}
} // namespace HiviewDFX
} // namespace OHOS