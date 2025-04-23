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
#include "task/base/task_register.h"

using namespace OHOS::HiviewDFX;

class TaskRegisterTest : public testing::Test {
protected:
    void TearDown() override
    {
        TaskRegister::GetContainer().clear();
    }
};

HWTEST_F(TaskRegisterTest, ShouldRegisterTaskWithAllParameters, TestSize.Level0)
{
    constexpr TaskId TEST_ID = 1001;
    auto creator = []{ return std::make_unique<Task>(); };
    
    TaskRegister tr(TEST_ID, creator, true, "TestTask", {2001, 2002});
    
    auto& container = TaskRegister::GetContainer();
    ASSERT_TRUE(container.count(TEST_ID));
    EXPECT_EQ(container[TEST_ID].taskName, "TestTask");
    EXPECT_EQ(container[TEST_ID].mandatory, true);
    EXPECT_EQ(container[TEST_ID].taskDependency, (std::vector<TaskId>{2001, 2002}));
}

HWTEST_F(TaskRegisterTest, ShouldRegisterDataDependency, TestSize.Level0)
{
    constexpr TaskId TEST_ID = 1002;
    
    TaskRegister tr(TEST_ID, {3001, 3002});
    
    auto& container = TaskRegister::GetContainer();
    ASSERT_TRUE(container.count(TEST_ID));
    EXPECT_EQ(container[TEST_ID].dataDependency, (std::vector<DataId>{3001, 3002}));
}

HWTEST_F(TaskRegisterTest, GetContainerShouldBeSingleton, TestSize.Level0)
{
    auto& container1 = TaskRegister::GetContainer();
    auto& container2 = TaskRegister::GetContainer();
    EXPECT_EQ(&container1, &container2);
}

HWTEST_F(TaskRegisterTest, CopyTaskInfoShouldCreateDeepCopy, TestSize.Level0)
{
    constexpr TaskId TEST_ID = 1003;
    TaskRegister tr(TEST_ID, []{ return nullptr; }, false, "TempTask", {});
    
    auto original = TaskRegister::GetContainer();
    auto copy = TaskRegister::CopyTaskInfo();
    
    original[TEST_ID].taskName = "Modified";
    EXPECT_NE(original[TEST_ID].taskName, copy[TEST_ID].taskName);
}

// 测试宏展开
struct TestTask1 : public Task {};
struct TestTask2 : public Task {};

REGISTER_TASK(2001, TestTask1, true, 3001)
REGISTER_DEPENDENT_DATA(2001, 4001)

HWTEST_F(TaskRegisterTest, MacroRegistrationShouldWork, TestSize.Level0)
{
    auto& container = TaskRegister::GetContainer();
    ASSERT_TRUE(container.count(2001));
    
    EXPECT_NE(container[2001].creator(), nullptr);
    EXPECT_EQ(container[2001].taskDependency, (std::vector<TaskId>{3001}));
    EXPECT_EQ(container[2001].dataDependency, (std::vector<DataId>{4001}));
}