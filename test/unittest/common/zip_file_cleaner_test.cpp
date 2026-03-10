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
#include <unistd.h>
#include <sys/stat.h>
#include <fstream>
#define private public
#include "util/zip_file_cleaner.h"
#undef private
#include "directory_ex.h"
#include "dump_utils.h"
#include "file_ex.h"

using namespace std;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::HiviewDFX;

namespace OHOS {
namespace HiviewDFX {

constexpr int TEST_FILE_SIZE_1KB = 1 * BYTES_PER_KB;
constexpr int TEST_FILE_SIZE_30MB = 30 * BYTES_PER_MB;

class ZipFileCleanerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::string testDir_;
};

void ZipFileCleanerTest::SetUpTestCase(void)
{
}

void ZipFileCleanerTest::TearDownTestCase(void)
{
}

void ZipFileCleanerTest::SetUp(void)
{
    char tempDir[] = "/data/local/tmp/hidumper_test_XXXXXX";
    char* result = mkdtemp(tempDir);
    if (result != nullptr) {
        testDir_ = std::string(result) + "/";
    } else {
        testDir_ = "/data/local/tmp/hidumper_test/";
    }
}

void ZipFileCleanerTest::TearDown(void)
{
    if (DumpUtils::DirectoryExists(testDir_)) {
        std::vector<std::string> files;
        GetDirFiles(testDir_, files);
        for (const auto& file : files) {
            remove(file.c_str());
        }
        rmdir(testDir_.c_str());
    }
}

bool CreateTestFile(const std::string& path, size_t size)
{
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    std::vector<char> buffer(size, 'A');
    file.write(buffer.data(), size);
    file.close();
    return true;
}

/**
 * @tc.name: GetDirTotalSizeTest001
 * @tc.desc: Test GetDirTotalSize with empty directory
 * @tc.type: FUNC
 */
HWTEST_F(ZipFileCleanerTest, GetDirTotalSizeTest001, TestSize.Level3)
{
    std::vector<FileInfo> files;
    int64_t size = ZipFileCleaner::GetDirTotalSize(testDir_, files);
    ASSERT_EQ(size, 0);
    ASSERT_TRUE(files.empty());
}

/**
 * @tc.name: DeleteOldestFilesTest001
 * @tc.desc: Test DeleteOldestFiles with empty file list
 * @tc.type: FUNC
 */
HWTEST_F(ZipFileCleanerTest, DeleteOldestFilesTest001, TestSize.Level3)
{
    std::vector<FileInfo> files;
    ASSERT_TRUE(ZipFileCleaner::DeleteOldestFiles(0, files));
}

/**
 * @tc.name: DeleteOldestFilesTest002
 * @tc.desc: Test DeleteOldestFiles deletes oldest files when size exceeds limit
 * @tc.type: FUNC
 */
HWTEST_F(ZipFileCleanerTest, DeleteOldestFilesTest002, TestSize.Level3)
{
    std::string file1 = testDir_ + "20250305-143025-001.zip";
    std::string file2 = testDir_ + "20250305-143025-002.zip";
    std::string file3 = testDir_ + "20250305-143025-003.zip";
    std::string file4 = testDir_ + "20250305-143025-004.zip";

    ASSERT_TRUE(CreateTestFile(file1, TEST_FILE_SIZE_30MB));
    ASSERT_TRUE(CreateTestFile(file2, TEST_FILE_SIZE_30MB));
    ASSERT_TRUE(CreateTestFile(file3, TEST_FILE_SIZE_30MB));
    ASSERT_TRUE(CreateTestFile(file4, TEST_FILE_SIZE_30MB));

    std::vector<FileInfo> files;
    int64_t fileSize = ZipFileCleaner::GetDirTotalSize(testDir_, files);

    ASSERT_TRUE(ZipFileCleaner::DeleteOldestFiles(fileSize, files));

    ASSERT_FALSE(access(file1.c_str(), F_OK) == 0);
    ASSERT_FALSE(access(file2.c_str(), F_OK) == 0);
    ASSERT_TRUE(access(file3.c_str(), F_OK) == 0);
    ASSERT_TRUE(access(file4.c_str(), F_OK) == 0);
}

/**
 * @tc.name: CleanOldFilesTest001
 * @tc.desc: Test CleanOldFiles with empty directory
 * @tc.type: FUNC
 */
HWTEST_F(ZipFileCleanerTest, CleanOldFilesTest001, TestSize.Level3)
{
    ASSERT_TRUE(ZipFileCleaner::CleanOldFiles(testDir_));
}

/**
 * @tc.name: CleanOldFilesTest002
 * @tc.desc: Test CleanOldFiles with files under size limit
 * @tc.type: FUNC
 */
HWTEST_F(ZipFileCleanerTest, CleanOldFilesTest002, TestSize.Level3)
{
    std::string file1 = testDir_ + "20250305-143025-001.zip";
    std::string file2 = testDir_ + "20250305-143025-002002.zip";

    ASSERT_TRUE(CreateTestFile(file1, TEST_FILE_SIZE_1KB));
    ASSERT_TRUE(CreateTestFile(file2, TEST_FILE_SIZE_1KB));

    ASSERT_TRUE(ZipFileCleaner::CleanOldFiles(testDir_));

    ASSERT_TRUE(access(file1.c_str(), F_OK) == 0);
    ASSERT_TRUE(access(file2.c_str(), F_OK) == 0);
}

/**
 * @tc.name: CleanOldFilesTest003
 * @tc.desc: Test CleanOldFiles with files exceeding size limit
 * @tc.type: FUNC
 */
HWTEST_F(ZipFileCleanerTest, CleanOldFilesTest003, TestSize.Level3)
{
    std::string file1 = testDir_ + "20250305-143025-001.zip";
    std::string file2 = testDir_ + "20250305-143025-002.zip";
    std::string file3 = testDir_ + "20250305-143025-003.zip";
    std::string file4 = testDir_ + "20250305-143025-004.zip";

    ASSERT_TRUE(CreateTestFile(file1, TEST_FILE_SIZE_30MB));
    ASSERT_TRUE(CreateTestFile(file2, TEST_FILE_SIZE_30MB));
    ASSERT_TRUE(CreateTestFile(file3, TEST_FILE_SIZE_30MB));
    ASSERT_TRUE(CreateTestFile(file4, TEST_FILE_SIZE_30MB));

    ASSERT_TRUE(ZipFileCleaner::CleanOldFiles(testDir_));

    ASSERT_FALSE(access(file1.c_str(), F_OK) == 0);
    ASSERT_FALSE(access(file2.c_str(), F_OK) == 0);
    ASSERT_TRUE(access(file3.c_str(), F_OK) == 0);
    ASSERT_TRUE(access(file4.c_str(), F_OK) == 0);
}

/**
 * @tc.name: CleanOldFilesTest004
 * @tc.desc: Test CleanOldFiles cleans files when count exceeds LOGFILE_MAX
 * @tc.type: FUNC
 */
HWTEST_F(ZipFileCleanerTest, CleanOldFilesTest004, TestSize.Level3)
{
    for (int i = 0; i < LOGFILE_MAX + 5; i++) {
        std::string fileName = testDir_ + "20250305-143025-" + to_string(i) + ".zip";
        ASSERT_TRUE(CreateTestFile(fileName, TEST_FILE_SIZE_1KB));
    }

    ASSERT_TRUE(ZipFileCleaner::CleanOldFiles(testDir_));

    std::vector<FileInfo> files;
    ZipFileCleaner::GetDirTotalSize(testDir_, files);
    ASSERT_EQ(static_cast<int>(files.size()), LOGFILE_MAX);
}

} // namespace HiviewDFX
} // namespace OHOS
