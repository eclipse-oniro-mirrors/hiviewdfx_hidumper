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
#include <memory>
#include <vector>
#include <string>
#include <set>
#include <fstream>
#include <thread>
#include <algorithm>

#include "data_inventory.h"
#include "dump_context.h"

using namespace testing::ext;
using namespace std;
namespace OHOS {
namespace HiviewDFX {

class DataInventoryTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    static std::string RemoveLineEndings(const std::string& str)
    {
        std::string result = str;
        while (!result.empty() && (result.back() == '\n' || result.back() == '\r')) {
            result.pop_back();
        }
        return result;
    }

    DataInventory inventory_;
};

void DataInventoryTest::SetUpTestCase(void)
{
}

void DataInventoryTest::TearDownTestCase(void)
{
}

void DataInventoryTest::SetUp(void)
{
}

void DataInventoryTest::TearDown(void)
{
}

HWTEST_F(DataInventoryTest, BasicInjectAndGet001, TestSize.Level1)
{
    std::vector<std::string> testData = {"test1", "test2", "test3"};
    auto dataPtr = std::make_shared<std::vector<std::string>>(testData);
    
    bool result = inventory_.Inject(DataId::ALL_PROCESS_NAME_INFO, dataPtr);
    ASSERT_TRUE(result);
    
    auto retrievedData = inventory_.GetPtr<std::vector<std::string>>(DataId::ALL_PROCESS_NAME_INFO);
    ASSERT_NE(retrievedData, nullptr);
    
    ASSERT_EQ(retrievedData->size(), 3);
    ASSERT_EQ((*retrievedData)[0], "test1");
    ASSERT_EQ((*retrievedData)[1], "test2");
    ASSERT_EQ((*retrievedData)[2], "test3");
}

HWTEST_F(DataInventoryTest, InjectDuplicateProtection, TestSize.Level1)
{
    std::vector<std::string> firstData = {"first"};
    std::vector<std::string> secondData = {"second"};
    
    auto firstPtr = std::make_shared<std::vector<std::string>>(firstData);
    auto secondPtr = std::make_shared<std::vector<std::string>>(secondData);
    
    bool result1 = inventory_.Inject(DataId::ALL_PROCESS_NAME_INFO, firstPtr);
    ASSERT_TRUE(result1);
    
    bool result2 = inventory_.Inject(DataId::ALL_PROCESS_NAME_INFO, secondPtr);
    ASSERT_FALSE(result2);
    
    auto retrievedData = inventory_.GetPtr<std::vector<std::string>>(DataId::ALL_PROCESS_NAME_INFO);
    ASSERT_NE(retrievedData, nullptr);
    
    ASSERT_EQ(retrievedData->size(), 1);
    ASSERT_EQ((*retrievedData)[0], "first");
}

HWTEST_F(DataInventoryTest, SizeCalculation, TestSize.Level1)
{
    ASSERT_EQ(inventory_.Size(), 0);
    
    std::vector<std::string> testData = {"test"};
    auto dataPtr = std::make_shared<std::vector<std::string>>(testData);
    
    inventory_.Inject(DataId::ALL_PROCESS_NAME_INFO, dataPtr);
    ASSERT_EQ(inventory_.Size(), 1);
    
    inventory_.Inject(DataId::VSS_INFO, dataPtr);
    ASSERT_EQ(inventory_.Size(), 2);
}

HWTEST_F(DataInventoryTest, RemoveRestData, TestSize.Level1)
{
    std::vector<std::string> testData = {"test"};
    auto dataPtr = std::make_shared<std::vector<std::string>>(testData);
    
    inventory_.Inject(DataId::ALL_PROCESS_NAME_INFO, dataPtr);
    inventory_.Inject(DataId::VSS_INFO, dataPtr);
    inventory_.Inject(DataId::GRAPHIC_MEM_INFO, dataPtr);
    
    ASSERT_EQ(inventory_.Size(), 3);
    
    std::set<DataId> keepingTypes = {DataId::ALL_PROCESS_NAME_INFO, DataId::VSS_INFO};
    auto removedTypes = inventory_.RemoveRestData(keepingTypes);
    
    ASSERT_EQ(inventory_.Size(), 2);
    ASSERT_EQ(removedTypes.size(), 1);
    ASSERT_TRUE(removedTypes.find(DataId::GRAPHIC_MEM_INFO) != removedTypes.end());
    
    ASSERT_NE(inventory_.GetPtr<std::vector<std::string>>(DataId::ALL_PROCESS_NAME_INFO), nullptr);
    ASSERT_NE(inventory_.GetPtr<std::vector<std::string>>(DataId::VSS_INFO), nullptr);
    ASSERT_EQ(inventory_.GetPtr<std::vector<std::string>>(DataId::GRAPHIC_MEM_INFO), nullptr);
}

HWTEST_F(DataInventoryTest, DifferentDataTypeHandling, TestSize.Level1)
{
    std::vector<std::string> stringData = {"string1", "string2"};
    auto stringPtr = std::make_shared<std::vector<std::string>>(stringData);
    ASSERT_TRUE(inventory_.Inject(DataId::ALL_PROCESS_NAME_INFO, stringPtr));
    
    std::vector<int> intData = {1, 2, 3, 4, 5};
    auto intPtr = std::make_shared<std::vector<int>>(intData);
    ASSERT_TRUE(inventory_.Inject(DataId::ALL_PID_ADJ_INFO, intPtr));
    
    std::vector<uint64_t> uint64Data = {1000, 2000, 3000};
    auto uint64Ptr = std::make_shared<std::vector<uint64_t>>(uint64Data);
    ASSERT_TRUE(inventory_.Inject(DataId::VSS_INFO, uint64Ptr));
    
    auto retrievedString = inventory_.GetPtr<std::vector<std::string>>(DataId::ALL_PROCESS_NAME_INFO);
    auto retrievedInt = inventory_.GetPtr<std::vector<int>>(DataId::ALL_PID_ADJ_INFO);
    auto retrievedUint64 = inventory_.GetPtr<std::vector<uint64_t>>(DataId::VSS_INFO);
    
    ASSERT_NE(retrievedString, nullptr);
    ASSERT_NE(retrievedInt, nullptr);
    ASSERT_NE(retrievedUint64, nullptr);
    
    ASSERT_EQ(retrievedString->size(), 2);
    ASSERT_EQ(retrievedInt->size(), 5);
    ASSERT_EQ(retrievedUint64->size(), 3);
}

HWTEST_F(DataInventoryTest, EdgeCases, TestSize.Level1)
{
    std::vector<std::string> emptyData;
    auto emptyPtr = std::make_shared<std::vector<std::string>>(emptyData);
    ASSERT_TRUE(inventory_.Inject(DataId::ALL_PROCESS_NAME_INFO, emptyPtr));
    
    auto retrievedEmpty = inventory_.GetPtr<std::vector<std::string>>(DataId::ALL_PROCESS_NAME_INFO);
    ASSERT_NE(retrievedEmpty, nullptr);
    ASSERT_EQ(retrievedEmpty->size(), 0);
    
    std::vector<std::string> largeData;
    for (int i = 0; i < 1000; i++) {
        largeData.push_back("data_" + std::to_string(i));
    }
    auto largePtr = std::make_shared<std::vector<std::string>>(largeData);
    ASSERT_TRUE(inventory_.Inject(DataId::VSS_INFO, largePtr));
    
    auto retrievedLarge = inventory_.GetPtr<std::vector<std::string>>(DataId::VSS_INFO);
    ASSERT_NE(retrievedLarge, nullptr);
    ASSERT_EQ(retrievedLarge->size(), 1000);
}

HWTEST_F(DataInventoryTest, DataIntegrity, TestSize.Level1)
{
    std::vector<std::string> originalData = {"original1", "original2", "original3"};
    auto originalPtr = std::make_shared<std::vector<std::string>>(originalData);
    inventory_.Inject(DataId::ALL_PROCESS_NAME_INFO, originalPtr);
    
    auto retrievedData = inventory_.GetPtr<std::vector<std::string>>(DataId::ALL_PROCESS_NAME_INFO);
    ASSERT_NE(retrievedData, nullptr);
    
    ASSERT_EQ(retrievedData->size(), originalData.size());
    for (size_t i = 0; i < originalData.size(); i++) {
        ASSERT_EQ((*retrievedData)[i], originalData[i]);
    }
}

HWTEST_F(DataInventoryTest, InjectTemplateFeatures, TestSize.Level1)
{
    struct TestStruct {
        int id;
        std::string name;
        double value;
    };
    
    std::vector<TestStruct> structData = {
        {1, "test1", 1.5},
        {2, "test2", 2.5}
    };
    auto structPtr = std::make_shared<std::vector<TestStruct>>(structData);
    
    bool result = inventory_.Inject(DataId::DEVICE_INFO, structPtr);
    ASSERT_TRUE(result);
    
    auto retrievedStruct = inventory_.GetPtr<std::vector<TestStruct>>(DataId::DEVICE_INFO);
    ASSERT_NE(retrievedStruct, nullptr);
    ASSERT_EQ(retrievedStruct->size(), 2);
    ASSERT_EQ((*retrievedStruct)[0].id, 1);
    ASSERT_EQ((*retrievedStruct)[0].name, "test1");
    ASSERT_EQ((*retrievedStruct)[1].id, 2);
    ASSERT_EQ((*retrievedStruct)[1].name, "test2");
}

HWTEST_F(DataInventoryTest, InjectProtectionMechanism, TestSize.Level1)
{
    std::vector<std::string> testData = {"test"};
    auto dataPtr = std::make_shared<std::vector<std::string>>(testData);
    
    bool result1 = inventory_.Inject(DataId::ALL_PROCESS_NAME_INFO, dataPtr);
    ASSERT_TRUE(result1);
    ASSERT_EQ(inventory_.Size(), 1);
    
    std::vector<std::string> differentData = {"different"};
    auto differentPtr = std::make_shared<std::vector<std::string>>(differentData);
    bool result2 = inventory_.Inject(DataId::ALL_PROCESS_NAME_INFO, differentPtr);
    ASSERT_FALSE(result2);
    
    ASSERT_EQ(inventory_.Size(), 1);
    
    auto retrievedData = inventory_.GetPtr<std::vector<std::string>>(DataId::ALL_PROCESS_NAME_INFO);
    ASSERT_NE(retrievedData, nullptr);
    ASSERT_EQ((*retrievedData)[0], "test");
}

HWTEST_F(DataInventoryTest, InjectStringFromFile, TestSize.Level1)
{
    std::string tempFile = "temp_test_file.txt";
    std::ofstream file(tempFile);
    ASSERT_TRUE(file.is_open());
    
    file << "line1" << std::endl;
    file << "line2" << std::endl;
    file << "line3" << std::endl;
    file.close();
    
    bool result = inventory_.InjectString(tempFile, DataId::ALL_PROCESS_NAME_INFO, true);
    ASSERT_TRUE(result);
    
    auto retrievedData = inventory_.GetPtr<std::vector<std::string>>(DataId::ALL_PROCESS_NAME_INFO);
    ASSERT_NE(retrievedData, nullptr);
    ASSERT_EQ(retrievedData->size(), 3);
    ASSERT_EQ(RemoveLineEndings((*retrievedData)[0]), "line1");
    ASSERT_EQ(RemoveLineEndings((*retrievedData)[1]), "line2");
    ASSERT_EQ(RemoveLineEndings((*retrievedData)[2]), "line3");
    
    std::remove(tempFile.c_str());
}

HWTEST_F(DataInventoryTest, InjectStringLineEndingHandling, TestSize.Level1)
{
    std::string tempFile = "line_ending_test_file.txt";
    std::ofstream file(tempFile);
    ASSERT_TRUE(file.is_open());
    
    file << "test_line1" << std::endl;
    file << "test_line2" << std::endl;
    file.close();
    
    bool result = inventory_.InjectString(tempFile, DataId::SYSTEM_CLUSTER_INFO, true);
    ASSERT_TRUE(result);
    
    auto retrievedData = inventory_.GetPtr<std::vector<std::string>>(DataId::SYSTEM_CLUSTER_INFO);
    ASSERT_NE(retrievedData, nullptr);
    ASSERT_EQ(retrievedData->size(), 2);
    
    std::string line1 = (*retrievedData)[0];
    std::string line2 = (*retrievedData)[1];
    
    bool hasLineEnding1 = !line1.empty() && (line1.back() == '\n' || line1.back() == '\r');
    bool hasLineEnding2 = !line2.empty() && (line2.back() == '\n' || line2.back() == '\r');
    
    ASSERT_TRUE(hasLineEnding1) << "Line 1 should contain line ending characters";
    ASSERT_TRUE(hasLineEnding2) << "Line 2 should contain line ending characters";
    
    ASSERT_EQ(RemoveLineEndings(line1), "test_line1");
    ASSERT_EQ(RemoveLineEndings(line2), "test_line2");
    
    std::remove(tempFile.c_str());
}

HWTEST_F(DataInventoryTest, InjectStringFromCommand, TestSize.Level1)
{
    std::string command = "echo -e \"cmd_line1\\ncmd_line2\\ncmd_line3\"";
    bool result = inventory_.InjectString(command, DataId::VSS_INFO, false);

    if (result) {
        auto retrievedData = inventory_.GetPtr<std::vector<std::string>>(DataId::VSS_INFO);
        ASSERT_NE(retrievedData, nullptr);
        ASSERT_GT(retrievedData->size(), 0);
    }
}

HWTEST_F(DataInventoryTest, InjectStringEmptyFile, TestSize.Level1)
{
    std::string emptyFile = "empty_test_file.txt";
    std::ofstream file(emptyFile);
    ASSERT_TRUE(file.is_open());
    file.close();
    
    bool result = inventory_.InjectString(emptyFile, DataId::DEVICE_INFO, true);
    ASSERT_TRUE(result);
    
    auto retrievedData = inventory_.GetPtr<std::vector<std::string>>(DataId::DEVICE_INFO);
    ASSERT_NE(retrievedData, nullptr);
    ASSERT_EQ(retrievedData->size(), 0);
    
    std::remove(emptyFile.c_str());
}

HWTEST_F(DataInventoryTest, InjectStringNonExistentFile, TestSize.Level1)
{
    std::string nonExistentFile = "non_existent_file_12345.txt";
    bool result = inventory_.InjectString(nonExistentFile, DataId::DEVICE_INFO, true);
    ASSERT_FALSE(result);
    
    auto retrievedData = inventory_.GetPtr<std::vector<std::string>>(DataId::DEVICE_INFO);
    ASSERT_EQ(retrievedData, nullptr);
}

HWTEST_F(DataInventoryTest, InjectStringWithFilterBasic, TestSize.Level1)
{
    std::string tempFile = "filter_test_file.txt";
    std::ofstream file(tempFile);
    ASSERT_TRUE(file.is_open());
    
    file << "original_line1" << std::endl;
    file << "original_line2" << std::endl;
    file << "original_line3" << std::endl;
    file.close();
    
    auto filterFunc = [](std::string& line) {
        line = "filtered_" + line;
    };
    
    bool result = inventory_.InjectStringWithFilter(tempFile, DataId::GRAPHIC_MEM_INFO, true, filterFunc);
    ASSERT_TRUE(result);
    
    auto retrievedData = inventory_.GetPtr<std::vector<std::string>>(DataId::GRAPHIC_MEM_INFO);
    ASSERT_NE(retrievedData, nullptr);
    ASSERT_EQ(retrievedData->size(), 3);
    ASSERT_EQ(RemoveLineEndings((*retrievedData)[0]), "filtered_original_line1");
    ASSERT_EQ(RemoveLineEndings((*retrievedData)[1]), "filtered_original_line2");
    ASSERT_EQ(RemoveLineEndings((*retrievedData)[2]), "filtered_original_line3");
    
    std::remove(tempFile.c_str());
}

HWTEST_F(DataInventoryTest, InjectStringWithFilterComplex, TestSize.Level1)
{
    std::string tempFile = "complex_filter_test_file.txt";
    std::ofstream file(tempFile);
    ASSERT_TRUE(file.is_open());
    
    file << "data1:100" << std::endl;
    file << "data2:200" << std::endl;
    file << "data3:300" << std::endl;
    file.close();
    
    auto filterFunc = [](std::string& line) {
        size_t pos = line.find(':');
        if (pos != std::string::npos) {
            std::string name = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            line = name + " = " + value + " units";
        }
    };
    
    bool result = inventory_.InjectStringWithFilter(tempFile, DataId::PROC_SMAPS_INFO, true, filterFunc);
    ASSERT_TRUE(result);
    
    auto retrievedData = inventory_.GetPtr<std::vector<std::string>>(DataId::PROC_SMAPS_INFO);
    ASSERT_NE(retrievedData, nullptr);
    ASSERT_EQ(retrievedData->size(), 3);
    ASSERT_EQ(RemoveLineEndings((*retrievedData)[0]), "data1 = 100\n units");
    ASSERT_EQ(RemoveLineEndings((*retrievedData)[1]), "data2 = 200\n units");
    ASSERT_EQ(RemoveLineEndings((*retrievedData)[2]), "data3 = 300\n units");
    
    std::remove(tempFile.c_str());
}

HWTEST_F(DataInventoryTest, InjectStringWithFilterEmpty, TestSize.Level1)
{
    std::string tempFile = "empty_filter_test_file.txt";
    std::ofstream file(tempFile);
    ASSERT_TRUE(file.is_open());
    
    file << "line1" << std::endl;
    file << "line2" << std::endl;
    file.close();
    
    auto filterFunc = [](std::string& line) {
    };
    
    bool result = inventory_.InjectStringWithFilter(tempFile, DataId::PROC_VERSION_INFO, true, filterFunc);
    ASSERT_TRUE(result);
    
    auto retrievedData = inventory_.GetPtr<std::vector<std::string>>(DataId::PROC_VERSION_INFO);
    ASSERT_NE(retrievedData, nullptr);
    ASSERT_EQ(retrievedData->size(), 2);
    ASSERT_EQ(RemoveLineEndings((*retrievedData)[0]), "line1");
    ASSERT_EQ(RemoveLineEndings((*retrievedData)[1]), "line2");
    
    std::remove(tempFile.c_str());
}

HWTEST_F(DataInventoryTest, GetPtrEmptyDataId, TestSize.Level1)
{
    auto retrievedData = inventory_.GetPtr<std::vector<std::string>>(DataId::DEVICE_INFO);
    ASSERT_EQ(retrievedData, nullptr);
    
    auto retrievedIntData = inventory_.GetPtr<std::vector<int>>(DataId::DEVICE_INFO);
    ASSERT_EQ(retrievedIntData, nullptr);
}

HWTEST_F(DataInventoryTest, ConcurrentSafety, TestSize.Level1)
{
    std::vector<std::string> testData = {"test"};
    auto dataPtr = std::make_shared<std::vector<std::string>>(testData);
    
    inventory_.Inject(DataId::ALL_PROCESS_NAME_INFO, dataPtr);
    
    std::vector<std::thread> threads;
    std::vector<bool> results(10, false);
    
    for (int i = 0; i < 10; i++) {
        threads.emplace_back([this, i, &results]() {
            auto data = inventory_.GetPtr<std::vector<std::string>>(DataId::ALL_PROCESS_NAME_INFO);
            results[i] = (data != nullptr);
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    for (bool result : results) {
        ASSERT_TRUE(result);
    }
}

HWTEST_F(DataInventoryTest, DataLifecycleManagement, TestSize.Level1)
{
    auto dataPtr = std::make_shared<std::vector<std::string>>(std::vector<std::string>{"test"});
    
    bool result = inventory_.Inject(DataId::ALL_PROCESS_NAME_INFO, dataPtr);
    ASSERT_TRUE(result);
    
    auto retrievedData = inventory_.GetPtr<std::vector<std::string>>(DataId::ALL_PROCESS_NAME_INFO);
    ASSERT_NE(retrievedData, nullptr);
    
    dataPtr.reset();
    
    auto retrievedData2 = inventory_.GetPtr<std::vector<std::string>>(DataId::ALL_PROCESS_NAME_INFO);
    ASSERT_NE(retrievedData2, nullptr);
    ASSERT_EQ((*retrievedData2)[0], "test");
}

} // namespace HiviewDFX
} // namespace OHOS