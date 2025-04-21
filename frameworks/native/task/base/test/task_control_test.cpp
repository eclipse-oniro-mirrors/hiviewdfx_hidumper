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

#include "task/base/task_control.h"
#include "task/base/task_register.h"
#include "task/base/task_struct.h"
#include "data_inventory.h"
#include <gtest/gtest.h>

using namespace OHOS::HiviewDFX;

class MockDataInventory : public DataInventory {
public:
    void Inject(DataId id, std::shared_ptr<void> data) override {}
    std::shared_ptr<void> Acquire(DataId id) override { return nullptr; }
    std::set<DataId> RemoveRestData(const std::set<DataId>& usingData) override { return {}; }
};

class TaskControlTest : public testing::Test {
protected:
    void SetUp() override
    {
        parameter_ = std::make_shared<DumperParameter>();
        TaskRegister::CopyTaskInfo().swap(originalTasks_);
    }

    void TearDown() override
    {
        TaskRegister::CopyTaskInfo().swap(originalTasks_);
    }

    TaskCollection originalTasks_;
    std::shared_ptr<DumperParameter> parameter_;
    MockDataInventory dataInventory_;
};

HWTEST_F(TaskControlTest, VerifyEmptyTaskCollection, TestSize.Level0)
{
    TaskControl controller;
    TaskCollection tasks;
    EXPECT_TRUE(controller.VerifyTasks(tasks));
}

HWTEST_F(TaskControlTest, DetectCyclicDependency, TestSize.Level0)
{
    TaskCollection tasks = {
        {1, {[]{ return nullptr; }, {}, {2}, "Task1", true}},
        {2, {[]{ return nullptr; }, {}, {3}, "Task2", true}},
        {3, {[]{ return nullptr; }, {}, {1}, "Task3", true}}
    };
    TaskControl controller;
    EXPECT_FALSE(controller.VerifyTasks(tasks));
}

HWTEST_F(TaskControlTest, ExecuteMandatoryTaskFailure, TestSize.Level0)
{
    TaskCollection tasks = {
        {1, {[]{ return nullptr; }, {}, {}, "Task1", true}}
    };
    TaskControl controller;
    EXPECT_EQ(controller.ExcuteTask(dataInventory_, tasks, parameter_), DUMP_FAIL);
}

HWTEST_F(TaskControlTest, MultiLevelDependencyChain, TestSize.Level0)
{
    TaskCollection tasks = {
        {1, {[]{ return nullptr; }, {}, {}, "Task1", false}},
        {2, {[]{ return nullptr; }, {}, {1}, "Task2", false}},
        {3, {[]{ return nullptr; }, {}, {2}, "Task3", false}}
    };
    TaskControl controller;
    EXPECT_EQ(controller.ExcuteTask(dataInventory_, tasks, parameter_), DUMP_OK);
}

HWTEST_F(TaskControlTest, DataCleanupAfterExecution, TestSize.Level0)
{
    TaskCollection tasks = {
        {1, {[]{ return nullptr; }, {DataId::DEVICE_INFO}, {}, "Task1", false}}
    };
    TaskControl controller;
    controller.ExcuteTask(dataInventory_, tasks, parameter_);
    EXPECT_TRUE(tasks.empty());
}