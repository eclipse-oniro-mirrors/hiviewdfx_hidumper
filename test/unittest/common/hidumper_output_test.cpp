/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include <vector>
#include <unistd.h>

#include "directory_ex.h"
#include "executor/zip_output.h"
#include "executor/fd_output.h"
#define private public
#include "raw_param.h"
#undef private

using namespace std;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::HiviewDFX;
namespace OHOS {
namespace HiviewDFX {
class HidumperOutputTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

protected:
    static const std::string FILE_ROOT;
};

void HidumperOutputTest::SetUpTestCase(void)
{
    ForceCreateDirectory(FILE_ROOT);
}
void HidumperOutputTest::TearDownTestCase(void)
{
    ForceRemoveDirectory(FILE_ROOT);
}
void HidumperOutputTest::SetUp(void)
{
}
void HidumperOutputTest::TearDown(void)
{
}

const std::string HidumperOutputTest::FILE_ROOT = "/data/local/tmp/hidumper_test/";

/**
 * @tc.name: HidumperOutputTest001
 * @tc.desc: Test ZipOutpu with multibytes content.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperOutputTest, HidumperOutputTest001, TestSize.Level3)
{
    auto parameter = std::make_shared<DumperParameter>();
    auto dump_datas = std::make_shared<std::vector<std::vector<std::string>>>();
    auto zip_output = make_shared<ZipOutput>();

    // multibytes content
    {
        std::string line_content;
        line_content = "根据输出结果（win32 + vc 编译器）可以看到,"
                       "strShort 对象的地址和 c_str() 的地址很接近,"
                       "都在 strShort 对象的栈控件内,"
                       "而 strLong 的 c_str() 地址很明显与 strLong 对象地址不同，是在 heap 中分配的.";
        std::vector<std::string> line_vector;
        line_vector.push_back(line_content);
        dump_datas->push_back(line_vector);
    }

    DumperOpts opts;
    opts.path_ = FILE_ROOT + "GZ_HidumperOutputTest001.gz";
    parameter->SetOpts(opts);

    auto config = std::make_shared<DumpCfg>();
    zip_output->SetDumpConfig(config);

    DumpStatus ret = zip_output->PreExecute(parameter, dump_datas);
    ASSERT_TRUE(ret == DumpStatus::DUMP_OK) << "PreExecute failed.";

    ret = zip_output->Execute();
    ASSERT_TRUE(ret == DumpStatus::DUMP_OK) << "Execute failed.";

    ret = zip_output->AfterExecute();
    ASSERT_TRUE(ret == DumpStatus::DUMP_OK) << "AfterExecute failed.";
}

/**
 * @tc.name: HidumperOutputTest002
 * @tc.desc: Test ZipOutpu with content size touch buffer size.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperOutputTest, HidumperOutputTest002, TestSize.Level3)
{
    auto parameter = std::make_shared<DumperParameter>();
    auto dump_datas = std::make_shared<std::vector<std::vector<std::string>>>();
    auto zip_output = make_shared<ZipOutput>();

    // Touch Buffer Size
    {
        std::string line_content;
        line_content = "HidumperOutputTest001 big one!HidumperOutputTest001 big one!"
                       "HidumperOutputTest001 big one!HidumperOutputTest001 big one!"
                       "HidumperOutputTest001 big one!HidumperOutputTest001 big one!"
                       "HidumperOutputTest001 big one!HidumperOutputTest001 big one!"
                       "HidumperOut123456";
        std::vector<std::string> line_vector;
        line_vector.push_back(line_content);
        dump_datas->push_back(line_vector);
    }

    DumperOpts opts;
    opts.path_ = FILE_ROOT + "GZ_HidumperOutputTest002.gz";
    parameter->SetOpts(opts);

    DumpStatus ret = zip_output->PreExecute(parameter, dump_datas);
    ASSERT_TRUE(ret == DumpStatus::DUMP_OK) << "PreExecute failed.";

    ret = zip_output->Execute();
    ASSERT_TRUE(ret == DumpStatus::DUMP_OK) << "Execute failed.";

    ret = zip_output->AfterExecute();
    ASSERT_TRUE(ret == DumpStatus::DUMP_OK) << "AfterExecute failed.";
}

/**
 * @tc.name: HidumperOutputTest003
 * @tc.desc: Test ZipOutpu with content size touch buffer size - 1.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperOutputTest, HidumperOutputTest003, TestSize.Level3)
{
    auto parameter = std::make_shared<DumperParameter>();
    auto dump_datas = std::make_shared<std::vector<std::vector<std::string>>>();
    auto zip_output = make_shared<ZipOutput>();

    // Touch Buffer Size - 1
    {
        std::string line_content;
        line_content = "HidumperOutputTest001 big one!"
                       "HidumperOutputTest001 big one!"
                       "HidumperOutputTest001 big one!"
                       "HidumperOutputTest001 big one!"
                       "HidumperOutputTest001 big one!"
                       "HidumperOutputTest001 big one!"
                       "HidumperOutputTest001 big one!"
                       "HidumperOutputTest001 big one!"
                       "HidumperOu12345";
        std::vector<std::string> line_vector;
        line_vector.push_back(line_content);
        dump_datas->push_back(line_vector);
    }

    DumperOpts opts;
    opts.path_ = FILE_ROOT + "GZ_HidumperOutputTest003.gz";
    parameter->SetOpts(opts);

    DumpStatus ret = zip_output->PreExecute(parameter, dump_datas);
    ASSERT_TRUE(ret == DumpStatus::DUMP_OK) << "PreExecute failed.";

    ret = zip_output->Execute();
    ASSERT_TRUE(ret == DumpStatus::DUMP_OK) << "Execute failed.";

    ret = zip_output->AfterExecute();
    ASSERT_TRUE(ret == DumpStatus::DUMP_OK) << "AfterExecute failed.";
}

/**
 * @tc.name: HidumperOutputTest004
 * @tc.desc: Test ZipOutpu with content size touch buffer size + 1.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperOutputTest, HidumperOutputTest004, TestSize.Level3)
{
    auto parameter = std::make_shared<DumperParameter>();
    auto dump_datas = std::make_shared<std::vector<std::vector<std::string>>>();
    auto zip_output = make_shared<ZipOutput>();

    // Touch Buffer Size + 1
    {
        std::string line_content;
        line_content = "HidumperOutputTest001 big one!"
                       "HidumperOutputTest001 big one!"
                       "HidumperOutputTest001 big one!"
                       "HidumperOutputTest001 big one!"
                       "HidumperOutputTest001 big one!"
                       "HidumperOutputTest001 big one!"
                       "HidumperOutputTest001 big one!"
                       "HidumperOutputTest001 big one!"
                       "HidumperOu1234567";
        std::vector<std::string> line_vector;
        line_vector.push_back(line_content);
        dump_datas->push_back(line_vector);
    }

    DumperOpts opts;
    opts.path_ = FILE_ROOT + "GZ_HidumperOutputTest004.gz";
    parameter->SetOpts(opts);

    DumpStatus ret = zip_output->PreExecute(parameter, dump_datas);
    ASSERT_TRUE(ret == DumpStatus::DUMP_OK) << "PreExecute failed.";

    ret = zip_output->Execute();
    ASSERT_TRUE(ret == DumpStatus::DUMP_OK) << "Execute failed.";

    ret = zip_output->AfterExecute();
    ASSERT_TRUE(ret == DumpStatus::DUMP_OK) << "AfterExecute failed.";
}

/**
 * @tc.name: HidumperOutputTest005
 * @tc.desc: Test ZipOutpu with multi lines content.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperOutputTest, HidumperOutputTest005, TestSize.Level3)
{
    auto parameter = std::make_shared<DumperParameter>();
    auto dump_datas = std::make_shared<std::vector<std::vector<std::string>>>();
    auto zip_output = make_shared<ZipOutput>();

    // Multi lines
    for (int i = 1; i < 1000; i++) {
        std::string line_content;
        line_content = "HidumperOutputTest001 big one!"
                       "HidumperOutputTest001 big one!"
                       "HidumperOutputTest001 big one!"
                       "HidumperOutputTest001 big one!"
                       "HidumperOutputTest001 big one!"
                       "HidumperOutputTest001 big one!"
                       "HidumperOutputTest001 big one!"
                       "HidumperOutputTest001 big one!"
                       "HidumperOutputTest001 big one!"
                       "HidumperOutputTest001 big one!"
                       "HidumperOutputTest001 big one!"
                       "HidumperOutputTest001 big one!"
                       "HidumperOutputTest001 big one!"
                       "HidumperOutputTest001 big one!";
        std::vector<std::string> line_vector;
        line_vector.push_back(line_content);
        dump_datas->push_back(line_vector);
    }

    DumperOpts opts;
    opts.path_ = FILE_ROOT + "GZ_HidumperOutputTest005.gz";
    parameter->SetOpts(opts);

    DumpStatus ret = zip_output->PreExecute(parameter, dump_datas);
    ASSERT_TRUE(ret == DumpStatus::DUMP_OK) << "PreExecute failed.";

    ret = zip_output->Execute();
    ASSERT_TRUE(ret == DumpStatus::DUMP_OK) << "Execute failed.";

    ret = zip_output->AfterExecute();
    ASSERT_TRUE(ret == DumpStatus::DUMP_OK) << "AfterExecute failed.";
}

/**
 * @tc.name: HidumperOutputTest006
 * @tc.desc: Test ZipOutpu with short string.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperOutputTest, HidumperOutputTest006, TestSize.Level3)
{
    auto parameter = std::make_shared<DumperParameter>();
    auto dump_datas = std::make_shared<std::vector<std::vector<std::string>>>();
    auto zip_output = make_shared<ZipOutput>();

    std::string line_content;
    line_content = "cmd is:";
    std::vector<std::string> line_vector;
    line_vector.push_back(line_content);
    dump_datas->push_back(line_vector);

    DumperOpts opts;
    opts.path_ = FILE_ROOT + "GZ_HidumperOutputTest006.gz";
    parameter->SetOpts(opts);

    DumpStatus ret = zip_output->PreExecute(parameter, dump_datas);
    ASSERT_TRUE(ret == DumpStatus::DUMP_OK) << "PreExecute failed.";

    ret = zip_output->Execute();
    ASSERT_TRUE(ret == DumpStatus::DUMP_OK) << "Execute failed.";

    ret = zip_output->AfterExecute();
    ASSERT_TRUE(ret == DumpStatus::DUMP_OK) << "AfterExecute failed.";
}

/**
 * @tc.name: HidumperOutputTest007
 * @tc.desc: Test FdOutput.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperOutputTest, HidumperOutputTest007, TestSize.Level3)
{
    auto parameter = std::make_shared<DumperParameter>();
    std::vector<std::u16string> args;
    auto reqCtl = std::make_shared<RawParam>(0, 0, 0, args, STDOUT_FILENO);
    parameter->setClientCallback(reqCtl);
    auto dumpDatas = std::make_shared<std::vector<std::vector<std::string>>>();
    auto fdOutput = make_shared<FDOutput>();

    std::string lineContent;
    lineContent = "this is FdOutputTest";
    std::vector<std::string> lineVector;
    lineVector.push_back(lineContent);
    dumpDatas->push_back(lineVector);

    DumpStatus ret = fdOutput->PreExecute(parameter, dumpDatas);
    ASSERT_TRUE(ret == DumpStatus::DUMP_OK) << "PreExecute failed.";

    ret = fdOutput->Execute();
    ASSERT_TRUE(ret == DumpStatus::DUMP_OK) << "Execute failed.";

    ret = fdOutput->AfterExecute();
    ASSERT_TRUE(ret == DumpStatus::DUMP_OK) << "AfterExecute failed.";
}
} // namespace HiviewDFX
} // namespace OHOS
