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
#include "data_inventory.h"
#include "hilog_wrapper.h"
#include "parameter.h"

using namespace OHOS::HiviewDFX;

class DeviceInfoTaskTest : public testing::Test {
public:
    void SetUp() override
    {
        inventory_ = std::make_shared<DataInventory>();
        task_ = std::make_shared<DeviceInfoTask>();
    }

protected:
    std::shared_ptr<DataInventory> inventory_;
    std::shared_ptr<DeviceInfoTask> task_;
};

HWTEST_F(DeviceInfoTaskTest, TaskEntry_NormalCase, TestSize.Level1)
{
    auto param = std::make_shared<DumperParameter>();
    auto ret = task_->TaskEntry(*inventory_, param);
    
    auto data = inventory_->GetPtr<std::vector<std::string>>(DataId::DEVICE_INFO);
    ASSERT_NE(data, nullptr);
    
    // 验证所有系统参数项都存在且格式正确
    for (const auto& item : *data) {
        EXPECT_FALSE(item.empty());
        EXPECT_NE(item.find(": "), std::string::npos);
    }
}

HWTEST_F(DeviceInfoTaskTest, TaskEntry_EmptyParameter, TestSize.Level1)
{
    auto ret = task_->TaskEntry(*inventory_, nullptr);
    EXPECT_EQ(ret, DUMP_OK);
}

HWTEST_F(DeviceInfoTaskTest, CheckTaskRegistration, TestSize.Level1)
{
    auto task = TaskRegister::GetTask(DUMP_DEVICE_INFO);
    ASSERT_NE(task, nullptr);
    EXPECT_NE(dynamic_cast<DeviceInfoTask*>(task.get()), nullptr);
}

HWTEST_F(DeviceInfoTaskTest, AddApiRet_HandleEmptyValue, TestSize.Level1)
{
    testing::internal::CaptureStdout();
    task_->AddApiRetIntoResult("", "EmptyValueTest");
    auto output = testing::internal::GetCapturedStdout();
    EXPECT_TRUE(output.find("EmptyValueTest: ") != std::string::npos);
}

HWTEST_F(DeviceInfoTaskTest, VerifyRegistrationMacroParams, TestSize.Level1)
{
    auto taskInfo = TaskRegister::GetTaskInfo(DUMP_DEVICE_INFO);
    ASSERT_TRUE(taskInfo.isEnabled);
    EXPECT_EQ(taskInfo.taskType, "system");
}

HWTEST_F(DeviceInfoTaskTest, HandleParameterFailure, TestSize.Level1)
{
    // 模拟参数获取失败
    auto oldFunc = GetDisplayVersion;
    GetDisplayVersion = []() { return ""; };
    
    auto param = std::make_shared<DumperParameter>();
    task_->TaskEntry(*inventory_, param);
    
    auto data = inventory_->GetPtr<std::vector<std::string>>(DataId::DEVICE_INFO);
    bool found = false;
    for (const auto& item : *data) {
        if (item.find("BuildId: ") != std::string::npos) {
            found = true;
            EXPECT_EQ(item, "BuildId: ");
        }
    }
    EXPECT_TRUE(found);
    GetDisplayVersion = oldFunc;
}