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
#define private public
#include "executor/memory_dumper.h"
#include "executor/api_dumper.h"
#include "executor/cmd_dumper.h"
#include "executor/cpu_dumper.h"
#include "executor/file_stream_dumper.h"
#include "executor/list_dumper.h"
#include "executor/sa_dumper.h"
#include "executor/version_dumper.h"
#include "util/config_utils.h"
#undef private

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
 * @tc.name: FileDumperTest001
 * @tc.desc: Test FileDumper base function with loop = TRUE.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperDumpersTest, FileDumperTest001, TestSize.Level3)
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
 * @tc.name: FileDumperTest002
 * @tc.desc: Test FileDumper base function with loop = FALSE.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperDumpersTest, FileDumperTest002, TestSize.Level3)
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
 * @tc.name: FileDumperTest005
 * @tc.desc: Test FileDumper base function with replacing PID in file paths.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperDumpersTest, FileDumperTest005, TestSize.Level0)
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
 * @tc.name: APIDumperTest001
 * @tc.desc: Test APIDumper target is build_version.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperDumpersTest, APIDumperTest001, TestSize.Level3)
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
 * @tc.name: APIDumperTest002
 * @tc.desc: Test APIDumper target is hw_sc.build.os.releasetype.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperDumpersTest, APIDumperTest002, TestSize.Level3)
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
 * @tc.name: APIDumperTest003
 * @tc.desc: Test APIDumper target is hw_sc.build.os.version.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperDumpersTest, APIDumperTest003, TestSize.Level3)
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
 * @tc.name: APIDumperTest004
 * @tc.desc: Test APIDumper target is system_param.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperDumpersTest, APIDumperTest004, TestSize.Level3)
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
 * @tc.name: APIDumperTest005
 * @tc.desc: Test VersionDumper.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperDumpersTest, VersionDumperTest001, TestSize.Level3)
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
 * @tc.name: CMDDumperTest001
 * @tc.desc: CMD Dumper base function with loop = TRUE.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperDumpersTest, CMDDumperTest001, TestSize.Level3)
{
    auto parameter = std::make_shared<DumperParameter>();
    auto dump_datas = std::make_shared<std::vector<std::vector<std::string>>>();
    auto cmd_dumper = make_shared<CMDDumper>();
    auto config = std::make_shared<DumpCfg>();
    config->name_ = "CmdDumperTest";
    std::string cmd = "ps -ef";
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
 * @tc.name: CMDDumperTest002
 * @tc.desc: CMD Dumper base function with loop = False.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperDumpersTest, CMDDumperTest002, TestSize.Level3)
{
    auto parameter = std::make_shared<DumperParameter>();
    auto dump_datas = std::make_shared<std::vector<std::vector<std::string>>>();
    auto cmd_dumper = make_shared<CMDDumper>();
    auto config = std::make_shared<DumpCfg>();
    config->name_ = "CmdDumperTest";
    std::string cmd = "ps -ef";
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

/**
 * @tc.name: MemoryDumperTest001
 * @tc.desc: Test MemoryDumper one process has correct ret.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperDumpersTest, MemoryDumperTest001, TestSize.Level1)
{
    auto memoryDumper = std::make_shared<MemoryDumper>();
    memoryDumper->pid_ = 1;
    auto dumpDatas = std::make_shared<std::vector<std::vector<std::string>>>();
    memoryDumper->dumpDatas_ = dumpDatas;
    int ret = DumpStatus::DUMP_MORE_DATA;
    while (ret == DumpStatus::DUMP_MORE_DATA) {
        ret = memoryDumper->Execute();
    }
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
}

/**
 * @tc.name: MemoryDumperTest002
 * @tc.desc: Test MemoryDumper all process has correct ret.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperDumpersTest, MemoryDumperTest002, TestSize.Level1)
{
    auto memoryDumper = std::make_shared<MemoryDumper>();
    memoryDumper->pid_ = -1;
    auto dumpDatas = std::make_shared<std::vector<std::vector<std::string>>>();
    memoryDumper->dumpDatas_ = dumpDatas;
    int ret = DumpStatus::DUMP_MORE_DATA;
    while (ret == DumpStatus::DUMP_MORE_DATA) {
        ret = memoryDumper->Execute();
    }
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
}

/**
 * @tc.name: SADumperTest001
 * @tc.desc: Test SADumper no saname has correct ret.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperDumpersTest, SADumperTest001, TestSize.Level1)
{
    auto parameter = std::make_shared<DumperParameter>();
    auto dumpDatas = std::make_shared<std::vector<std::vector<std::string>>>();
    auto saDumper = std::make_shared<SADumper>();
    auto config = std::make_shared<DumpCfg>();
    config->args_ = OptionArgs::Create();
    saDumper->SetDumpConfig(config);
    int ret = DumpStatus::DUMP_FAIL;
    ret = saDumper->PreExecute(parameter, dumpDatas);
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
    ret = saDumper->Execute();
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
}

/**
 * @tc.name: SADumperTest002
 * @tc.desc: Test SADumper one saname has correct ret.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperDumpersTest, SADumperTest002, TestSize.Level1)
{
    auto parameter = std::make_shared<DumperParameter>();
    auto dumpDatas = std::make_shared<std::vector<std::vector<std::string>>>();
    auto saDumper = std::make_shared<SADumper>();
    auto config = std::make_shared<DumpCfg>();
    config->args_ = OptionArgs::Create();
    const std::vector<std::string> names = {"1202"};
    const std::vector<std::string> args;
    config->args_->SetNamesAndArgs(names, args);
    saDumper->SetDumpConfig(config);
    int ret = DumpStatus::DUMP_FAIL;
    ret = saDumper->PreExecute(parameter, dumpDatas);
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
    ret = saDumper->Execute();
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
}

/**
 * @tc.name: CpuDumperTest001
 * @tc.desc: Test CpuDumper dump all process has correct ret.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperDumpersTest, CpuDumperTest001, TestSize.Level1)
{
    auto parameter = std::make_shared<DumperParameter>();
    DumperOpts opts;
    opts.isDumpCpuUsage_ = true;
    opts.cpuUsagePid_ = -1;
    parameter->SetOpts(opts);
    auto dumpDatas = std::make_shared<std::vector<std::vector<std::string>>>();
    auto cpuDumper = std::make_shared<CPUDumper>();
    int ret = DumpStatus::DUMP_FAIL;
    ret = cpuDumper->PreExecute(parameter, dumpDatas);
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
    ret = cpuDumper->Execute();
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
    ret = cpuDumper->AfterExecute();
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
}

/**
 * @tc.name: CpuDumperTest002
 * @tc.desc: Test CpuDumper has correct ret when opts is vaild.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperDumpersTest, CpuDumperTest002, TestSize.Level1)
{
    auto parameter = std::make_shared<DumperParameter>();
    DumperOpts opts;
    opts.isDumpCpuUsage_ = false;
    opts.cpuUsagePid_ = getpid();
    parameter->SetOpts(opts);
    auto dumpDatas = std::make_shared<std::vector<std::vector<std::string>>>();
    auto cpuDumper = std::make_shared<CPUDumper>();
    int ret = DumpStatus::DUMP_FAIL;
    ret = cpuDumper->PreExecute(parameter, dumpDatas);
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
    ret = cpuDumper->Execute();
    ASSERT_EQ(ret, DumpStatus::DUMP_FAIL);
}

/**
 * @tc.name: ListDumperTest001
 * @tc.desc: Test ListDumper dump ABILITY has correct ret.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperDumpersTest, ListDumperTest001, TestSize.Level1)
{
    auto parameter = std::make_shared<DumperParameter>();
    auto dumpDatas = std::make_shared<std::vector<std::vector<std::string>>>();
    auto listDumper = std::make_shared<ListDumper>();
    auto config = std::make_shared<DumpCfg>();
    config->target_ = ConfigUtils::STR_ABILITY;
    listDumper->SetDumpConfig(config);

    int ret = DumpStatus::DUMP_FAIL;
    ret = listDumper->PreExecute(parameter, dumpDatas);
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
    ret = listDumper->Execute();
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
    ret = listDumper->AfterExecute();
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
}

/**
 * @tc.name: ListDumperTest002
 * @tc.desc: Test ListDumper dump SYSTEM has correct ret.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperDumpersTest, ListDumperTest002, TestSize.Level1)
{
    auto parameter = std::make_shared<DumperParameter>();
    auto dumpDatas = std::make_shared<std::vector<std::vector<std::string>>>();
    auto listDumper = std::make_shared<ListDumper>();
    auto config = std::make_shared<DumpCfg>();
    config->target_ = ConfigUtils::STR_SYSTEM;
    listDumper->SetDumpConfig(config);

    int ret = DumpStatus::DUMP_FAIL;
    ret = listDumper->PreExecute(parameter, dumpDatas);
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
    ret = listDumper->Execute();
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
    ret = listDumper->AfterExecute();
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
}
} // namespace HiviewDFX
} // namespace OHOS