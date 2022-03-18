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
#include "executor/api_dumper.h"
#include "executor/cmd_dumper.h"
#include "executor/file_stream_dumper.h"
#include "executor/version_dumper.h"

using namespace std;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::HiviewDFX;
namespace OHOS {
namespace HiviewDFX {
class HidumperDumpersTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

protected:
    static constexpr auto& FILE_CPUINFO = "/proc/cpuinfo";
    static constexpr auto DEFAULT_PID = 1;
    static constexpr auto DEFAULT_UID = 0;
};

void HidumperDumpersTest::SetUpTestCase(void)
{
}

void HidumperDumpersTest::TearDownTestCase(void)
{
}

void HidumperDumpersTest::SetUp(void)
{
}

void HidumperDumpersTest::TearDown(void)
{
}

/**
 * @tc.name: HidumperDumpers001
 * @tc.desc: Test FileDumper base function with loop = TRUE.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperDumpersTest, HidumperDumpers001, TestSize.Level3)
{
    auto parameter = std::make_shared<DumperParameter>();
    auto dump_datas = std::make_shared<std::vector<std::vector<std::string>>>();
    auto file_dumper = make_shared<FileStreamDumper>();
    auto config = std::make_shared<DumpCfg>();
    config->name_ = "FileDumperTest";
    std::string file_name = FILE_CPUINFO;
    config->target_ = file_name;
    config->loop_ = DumperConstant::LOOP;
    config->args_ = OptionArgs::Create();
    config->args_->SetPid(DEFAULT_PID, DEFAULT_UID);
    file_dumper->SetDumpConfig(config);
    DumpStatus ret = file_dumper->DoPreExecute(parameter, dump_datas);
    ASSERT_TRUE(ret == DumpStatus::DUMP_OK) << "PreExecute failed.";

    ret = DumpStatus::DUMP_MORE_DATA;
    while (ret == DumpStatus::DUMP_MORE_DATA) {
        // loop for all lines
        ret = file_dumper->DoExecute();
        ASSERT_TRUE(ret == DumpStatus::DUMP_OK || ret == DumpStatus::DUMP_MORE_DATA) << "Execute failed.";
        ret = file_dumper->DoAfterExecute();
        ASSERT_TRUE(ret == DumpStatus::DUMP_OK || ret == DumpStatus::DUMP_MORE_DATA) << "Execute failed.";
    }
}

/**
 * @tc.name: HidumperDumpers002
 * @tc.desc: Test FileDumper base function with loop = FALSE.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperDumpersTest, HidumperDumpers002, TestSize.Level3)
{
    auto parameter = std::make_shared<DumperParameter>();
    auto dump_datas = std::make_shared<std::vector<std::vector<std::string>>>();
    auto file_dumper = make_shared<FileStreamDumper>();
    auto config = std::make_shared<DumpCfg>();
    config->name_ = "FileDumperTest";
    std::string file_name = FILE_CPUINFO;
    config->target_ = file_name;
    config->loop_ = DumperConstant::NONE;
    file_dumper->SetDumpConfig(config);
    DumpStatus ret = file_dumper->DoPreExecute(parameter, dump_datas);
    ASSERT_TRUE(ret == DumpStatus::DUMP_OK) << "PreExecute failed.";

    ret = DumpStatus::DUMP_MORE_DATA;
    while (ret == DumpStatus::DUMP_MORE_DATA) {
        // loop for all lines
        ret = file_dumper->DoExecute();
        ASSERT_TRUE(ret == DumpStatus::DUMP_OK || ret == DumpStatus::DUMP_MORE_DATA) << "Execute failed.";
        ret = file_dumper->DoAfterExecute();
        ASSERT_TRUE(ret == DumpStatus::DUMP_OK || ret == DumpStatus::DUMP_MORE_DATA) << "Execute failed.";
    }
}

/**
 * @tc.name: HidumperDumpers003
 * @tc.desc: Test FileDumper base function with inputing dir(end with /) and LOOP = TRUE.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperDumpersTest, HidumperDumpers003, TestSize.Level3)
{
    auto parameter = std::make_shared<DumperParameter>();
    auto dump_datas = std::make_shared<std::vector<std::vector<std::string>>>();
    auto file_dumper = make_shared<FileStreamDumper>();
    auto config = std::make_shared<DumpCfg>();
    config->name_ = "FileDumperTest";
    std::string file_name = "/data/log/faultlog/temp/";
    config->target_ = file_name;
    config->loop_ = DumperConstant::LOOP;
    file_dumper->SetDumpConfig(config);
    DumpStatus ret = file_dumper->DoPreExecute(parameter, dump_datas);
    ASSERT_TRUE(ret == DumpStatus::DUMP_OK) << "PreExecute failed.";

    ret = DumpStatus::DUMP_MORE_DATA;
    while (ret == DumpStatus::DUMP_MORE_DATA) {
        // loop for all lines
        ret = file_dumper->DoExecute();
        ASSERT_TRUE(ret == DumpStatus::DUMP_OK || ret == DumpStatus::DUMP_MORE_DATA) << "Execute failed.";
        ret = file_dumper->DoAfterExecute();
        ASSERT_TRUE(ret == DumpStatus::DUMP_OK || ret == DumpStatus::DUMP_MORE_DATA) << "Execute failed.";
    }
}

/**
 * @tc.name: HidumperDumpers003
 * @tc.desc: Test FileDumper base function with inputing dir.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperDumpersTest, HidumperDumpers004, TestSize.Level3)
{
    auto parameter = std::make_shared<DumperParameter>();
    auto dump_datas = std::make_shared<std::vector<std::vector<std::string>>>();
    auto file_dumper = make_shared<FileStreamDumper>();
    auto config = std::make_shared<DumpCfg>();
    config->name_ = "FileDumperTest";
    std::string file_name = "/data/log/faultlog/temp";
    config->target_ = file_name;
    config->loop_ = DumperConstant::NONE;
    file_dumper->SetDumpConfig(config);
    DumpStatus ret = file_dumper->DoPreExecute(parameter, dump_datas);
    ASSERT_TRUE(ret == DumpStatus::DUMP_OK) << "PreExecute failed.";

    ret = DumpStatus::DUMP_MORE_DATA;
    while (ret == DumpStatus::DUMP_MORE_DATA) {
        // loop for all lines
        ret = file_dumper->DoExecute();
        ASSERT_TRUE(ret == DumpStatus::DUMP_OK || ret == DumpStatus::DUMP_MORE_DATA) << "Execute failed.";
        ret = file_dumper->DoAfterExecute();
        ASSERT_TRUE(ret == DumpStatus::DUMP_OK || ret == DumpStatus::DUMP_MORE_DATA) << "Execute failed.";
    }
}
/**
 * @tc.name: HidumperDumpers005
 * @tc.desc: Test FileDumper base function with replacing PID in file paths.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperDumpersTest, HidumperDumpers005, TestSize.Level0)
{
    auto parameter = std::make_shared<DumperParameter>();
    auto dump_datas = std::make_shared<std::vector<std::vector<std::string>>>();
    auto file_dumper = make_shared<FileStreamDumper>();
    auto config = std::make_shared<DumpCfg>();
    config->name_ = "FileDumperTest";
    std::string file_name = "/proc/%pid/smaps";
    config->target_ = file_name;
    config->loop_ = DumperConstant::LOOP;
    config->args_ = OptionArgs::Create();
    config->args_->SetPid(DEFAULT_PID, DEFAULT_UID);
    file_dumper->SetDumpConfig(config);
    DumpStatus ret = file_dumper->DoPreExecute(parameter, dump_datas);
    ASSERT_TRUE(ret == DumpStatus::DUMP_OK) << "PreExecute failed.";

    ret = DumpStatus::DUMP_MORE_DATA;
    while (ret == DumpStatus::DUMP_MORE_DATA) {
        // loop for all lines
        ret = file_dumper->DoExecute();
        ASSERT_TRUE(ret == DumpStatus::DUMP_OK || ret == DumpStatus::DUMP_MORE_DATA) << "Execute failed.";
        ret = file_dumper->DoAfterExecute();
        ASSERT_TRUE(ret == DumpStatus::DUMP_OK || ret == DumpStatus::DUMP_MORE_DATA) << "Execute failed.";
    }
}

/**
 * @tc.name: HidumperDumpers006
 * @tc.desc: Test APIDumper target is build_version.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperDumpersTest, HidumperDumpers006, TestSize.Level3)
{
    auto parameter = std::make_shared<DumperParameter>();
    auto dump_datas = std::make_shared<std::vector<std::vector<std::string>>>();
    auto fapi_dumper = make_shared<APIDumper>();
    auto config = std::make_shared<DumpCfg>();
    config->name_ = "dumper_build_id";
    config->target_ = "build_version";
    fapi_dumper->SetDumpConfig(config);
    DumpStatus ret = fapi_dumper->DoPreExecute(parameter, dump_datas);
    ASSERT_TRUE(ret == DumpStatus::DUMP_OK) << "PreExecute failed.";
    ret = fapi_dumper->DoExecute();
    ASSERT_TRUE(ret == DumpStatus::DUMP_OK) << "Execute failed.";
    ret = fapi_dumper->DoAfterExecute();
    ASSERT_TRUE(ret == DumpStatus::DUMP_OK) << "AfterExecute failed.";
}

/**
 * @tc.name: HidumperDumpers007
 * @tc.desc: Test APIDumper target is hw_sc.build.os.releasetype.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperDumpersTest, HidumperDumpers007, TestSize.Level3)
{
    auto parameter = std::make_shared<DumperParameter>();
    auto dump_datas = std::make_shared<std::vector<std::vector<std::string>>>();
    auto fapi_dumper = make_shared<APIDumper>();
    auto config = std::make_shared<DumpCfg>();
    config->name_ = "dumper_release_type";
    config->target_ = "hw_sc.build.os.releasetype";
    fapi_dumper->SetDumpConfig(config);
    DumpStatus ret = fapi_dumper->DoPreExecute(parameter, dump_datas);
    ASSERT_TRUE(ret == DumpStatus::DUMP_OK) << "PreExecute failed.";
    ret = fapi_dumper->DoExecute();
    ASSERT_TRUE(ret == DumpStatus::DUMP_OK) << "Execute failed.";
    ret = fapi_dumper->DoAfterExecute();
    ASSERT_TRUE(ret == DumpStatus::DUMP_OK) << "AfterExecute failed.";
}

/**
 * @tc.name: HidumperDumpers008
 * @tc.desc: Test APIDumper target is hw_sc.build.os.version.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperDumpersTest, HidumperDumpers008, TestSize.Level3)
{
    auto parameter = std::make_shared<DumperParameter>();
    auto dump_datas = std::make_shared<std::vector<std::vector<std::string>>>();
    auto fapi_dumper = make_shared<APIDumper>();
    auto config = std::make_shared<DumpCfg>();
    config->name_ = "dumper_os_version";
    config->target_ = "hw_sc.build.os.version";
    fapi_dumper->SetDumpConfig(config);
    DumpStatus ret = fapi_dumper->DoPreExecute(parameter, dump_datas);
    ASSERT_TRUE(ret == DumpStatus::DUMP_OK) << "PreExecute failed.";
    ret = fapi_dumper->DoExecute();
    ASSERT_TRUE(ret == DumpStatus::DUMP_OK) << "Execute failed.";
    ret = fapi_dumper->DoAfterExecute();
    ASSERT_TRUE(ret == DumpStatus::DUMP_OK) << "AfterExecute failed.";
}

/**
 * @tc.name: HidumperDumpers009
 * @tc.desc: Test APIDumper target is system_param.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperDumpersTest, HidumperDumpers009, TestSize.Level3)
{
    auto parameter = std::make_shared<DumperParameter>();
    auto dump_datas = std::make_shared<std::vector<std::vector<std::string>>>();
    auto fapi_dumper = make_shared<APIDumper>();
    auto config = std::make_shared<DumpCfg>();
    config->name_ = "dumper_system_param";
    config->target_ = "system_param";
    fapi_dumper->SetDumpConfig(config);
    DumpStatus ret = fapi_dumper->DoPreExecute(parameter, dump_datas);
    ASSERT_TRUE(ret == DumpStatus::DUMP_OK) << "PreExecute failed.";
    ret = fapi_dumper->DoExecute();
    ASSERT_TRUE(ret == DumpStatus::DUMP_OK) << "Execute failed.";
    ret = fapi_dumper->DoAfterExecute();
    ASSERT_TRUE(ret == DumpStatus::DUMP_OK) << "AfterExecute failed.";
}

/**
 * @tc.name: DumpManagerService006
 * @tc.desc: Test VersionDumper.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperDumpersTest, HidumperDumpers010, TestSize.Level3)
{
    auto parameter = std::make_shared<DumperParameter>();
    auto dump_datas = std::make_shared<std::vector<std::vector<std::string>>>();
    auto fapi_dumper = make_shared<VersionDumper>();
    DumpStatus ret = fapi_dumper->DoPreExecute(parameter, dump_datas);
    ASSERT_TRUE(ret == DumpStatus::DUMP_OK) << "PreExecute failed.";
    ret = fapi_dumper->DoExecute();
    ASSERT_TRUE(ret == DumpStatus::DUMP_OK) << "Execute failed.";
    ret = fapi_dumper->DoAfterExecute();
    ASSERT_TRUE(ret == DumpStatus::DUMP_OK) << "AfterExecute failed.";
}

/**
 * @tc.name: HidumperDumpers011
 * @tc.desc: CMD Dumper base function with loop = TRUE.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperDumpersTest, HidumperDumpers011, TestSize.Level3)
{
    auto parameter = std::make_shared<DumperParameter>();
    auto dump_datas = std::make_shared<std::vector<std::vector<std::string>>>();
    auto cmd_dumper = make_shared<CMDDumper>();
    auto config = std::make_shared<DumpCfg>();
    config->name_ = "CmdDumperTest";
    std::string cmd = "hilog -x";
    config->target_ = cmd;
    config->loop_ = DumperConstant::LOOP;
    cmd_dumper->SetDumpConfig(config);
    DumpStatus ret = cmd_dumper->DoPreExecute(parameter, dump_datas);
    ASSERT_TRUE(ret == DumpStatus::DUMP_OK) << "PreExecute failed.";

    ret = DumpStatus::DUMP_MORE_DATA;
    while (ret == DumpStatus::DUMP_MORE_DATA) {
        // loop for all lines
        ret = cmd_dumper->DoExecute();
        ASSERT_TRUE(ret == DumpStatus::DUMP_OK || ret == DumpStatus::DUMP_MORE_DATA) << "Execute failed.";
        ret = cmd_dumper->DoAfterExecute();
        ASSERT_TRUE(ret == DumpStatus::DUMP_OK || ret == DumpStatus::DUMP_MORE_DATA) << "Execute failed.";
    }
}

/**
 * @tc.name: HidumperDumpers011
 * @tc.desc: CMD Dumper base function with loop = False.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperDumpersTest, HidumperDumpers012, TestSize.Level3)
{
    auto parameter = std::make_shared<DumperParameter>();
    auto dump_datas = std::make_shared<std::vector<std::vector<std::string>>>();
    auto cmd_dumper = make_shared<CMDDumper>();
    auto config = std::make_shared<DumpCfg>();
    config->name_ = "CmdDumperTest";
    std::string cmd = "hilog -x";
    config->target_ = cmd;
    config->loop_ = DumperConstant::NONE;
    cmd_dumper->SetDumpConfig(config);
    DumpStatus ret = cmd_dumper->DoPreExecute(parameter, dump_datas);
    ASSERT_TRUE(ret == DumpStatus::DUMP_OK) << "PreExecute failed.";

    ret = DumpStatus::DUMP_MORE_DATA;
    while (ret == DumpStatus::DUMP_MORE_DATA) {
        // loop for all lines
        ret = cmd_dumper->DoExecute();
        ASSERT_TRUE(ret == DumpStatus::DUMP_OK || ret == DumpStatus::DUMP_MORE_DATA) << "Execute failed.";
        ret = cmd_dumper->DoAfterExecute();
        ASSERT_TRUE(ret == DumpStatus::DUMP_OK || ret == DumpStatus::DUMP_MORE_DATA) << "Execute failed.";
    }
}
} // namespace HiviewDFX
} // namespace OHOS