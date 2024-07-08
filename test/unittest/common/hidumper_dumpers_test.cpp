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
#include <fcntl.h>
#include <gtest/gtest.h>
#define private public
#include "executor/memory_dumper.h"
#include "executor/api_dumper.h"
#include "executor/cmd_dumper.h"
#include "executor/cpu_dumper.h"
#include "executor/file_stream_dumper.h"
#include "executor/ipc_stat_dumper.h"
#include "executor/list_dumper.h"
#include "executor/sa_dumper.h"
#include "executor/version_dumper.h"
#include "executor/traffic_dumper.h"
#include "util/config_utils.h"
#include "util/string_utils.h"
#undef private

using namespace std;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::HiviewDFX;
namespace OHOS {
namespace HiviewDFX {
static std::shared_ptr<DumperParameter> g_parameter;
static std::shared_ptr<std::vector<std::vector<std::string>>> g_dump_datas;
static std::shared_ptr<DumpCfg> g_config;
class HidumperDumpersTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static void HandleDumperComon(std::string dumperType);
    static void HandleDumperExcute(std::string dumperType);
    static void HandleCpuDumperTest(int pid);
    static void HandleMemoryDumperTest(int pid);
    static void HandleTrafficDumperTest(int pid);
    static void GetDumperVariable();
    static void HandleIpcStatDumperTest(void);

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
    GetDumperVariable();
}

void HidumperDumpersTest::TearDown(void)
{
}

void HidumperDumpersTest::GetDumperVariable()
{
    g_parameter = std::make_shared<DumperParameter>();
    g_dump_datas = std::make_shared<std::vector<std::vector<std::string>>>();
    g_config = std::make_shared<DumpCfg>();
}

void HidumperDumpersTest::HandleDumperExcute(std::string dumperType)
{
    std::shared_ptr<HidumperExecutor> dumper = std::make_shared<FileStreamDumper>();
    if (StringUtils::GetInstance().IsSameStr(dumperType, "APIDumper")) {
        dumper = std::make_shared<APIDumper>();
    } else if (StringUtils::GetInstance().IsSameStr(dumperType, "VersionDumper")) {
        dumper = std::make_shared<VersionDumper>();
    } else if (StringUtils::GetInstance().IsSameStr(dumperType, "CMDDumper")) {
        dumper = std::make_shared<CMDDumper>();
    } else if (StringUtils::GetInstance().IsSameStr(dumperType, "SADumper")) {
        dumper = std::make_shared<SADumper>();
    } else {
        dumper = std::make_shared<ListDumper>();
    }
    dumper->SetDumpConfig(g_config);
    if (StringUtils::GetInstance().IsSameStr(dumperType, "SADumper")
        || StringUtils::GetInstance().IsSameStr(dumperType, "ListDumper")) {
        int fd = open("/dev/null", O_RDWR | O_CREAT | O_TRUNC, 0664);
        if (fd <= 0) {
            fd = STDERR_FILENO;
        }
        std::vector<std::u16string> args;
        std::shared_ptr<RawParam> rawParam = std::make_shared<RawParam>(0, 1, 0, args, fd);
        g_parameter->setClientCallback(rawParam);
        DumpStatus ret = DumpStatus::DUMP_FAIL;
        ret = dumper->PreExecute(g_parameter, g_dump_datas);
        ASSERT_EQ(ret, DumpStatus::DUMP_OK);
        ret = dumper->Execute();
        ASSERT_EQ(ret, DumpStatus::DUMP_OK);
        if (StringUtils::GetInstance().IsSameStr(dumperType, "ListDumper")) {
            ret = dumper->AfterExecute();
            ASSERT_EQ(ret, DumpStatus::DUMP_OK);
        }
    } else {
        DumpStatus ret = dumper->DoPreExecute(g_parameter, g_dump_datas);
        ASSERT_TRUE(ret == DumpStatus::DUMP_OK) << "PreExecute failed.";
        ret = dumper->DoExecute();
        ASSERT_TRUE(ret == DumpStatus::DUMP_OK) << "Execute failed.";
        ret = dumper->DoAfterExecute();
        ASSERT_TRUE(ret == DumpStatus::DUMP_OK) << "AfterExecute failed.";
    }
}

void HidumperDumpersTest::HandleCpuDumperTest(int pid)
{
    auto parameter = std::make_shared<DumperParameter>();
    DumperOpts opts;
    opts.isDumpCpuUsage_ = pid == -1 ? true : false;
    opts.cpuUsagePid_ = pid;
    parameter->SetOpts(opts);
    auto dumpDatas = std::make_shared<std::vector<std::vector<std::string>>>();
    auto cpuDumper = std::make_shared<CPUDumper>();
    DumpStatus ret = DumpStatus::DUMP_FAIL;
    ret = cpuDumper->PreExecute(parameter, dumpDatas);
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
    ret = cpuDumper->Execute();
    if (pid == -1) {
        ASSERT_EQ(ret, DumpStatus::DUMP_OK);
        ret = cpuDumper->AfterExecute();
        ASSERT_EQ(ret, DumpStatus::DUMP_OK);
    } else {
        ASSERT_EQ(ret, DumpStatus::DUMP_FAIL);
    }
}


void HidumperDumpersTest::HandleMemoryDumperTest(int pid)
{
    auto memoryDumper = std::make_shared<MemoryDumper>();
    memoryDumper->pid_ = pid;
    auto dumpDatas = std::make_shared<std::vector<std::vector<std::string>>>();
    memoryDumper->dumpDatas_ = dumpDatas;
    int res = DumpStatus::DUMP_MORE_DATA;
    while (res == DumpStatus::DUMP_MORE_DATA) {
        res = memoryDumper->Execute();
    }
    ASSERT_EQ(res, DumpStatus::DUMP_OK);
}

void HidumperDumpersTest::HandleTrafficDumperTest(int pid)
{
    auto trafficDumper = std::make_shared<TrafficDumper>();
    trafficDumper->pid_ = pid;
    auto dumpDatas = std::make_shared<std::vector<std::vector<std::string>>>();
    ASSERT_NE(dumpDatas, nullptr);
    trafficDumper->result_ = dumpDatas;
    DumpStatus ret = DumpStatus::DUMP_FAIL;
    ret = trafficDumper->Execute();
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
    ret = trafficDumper->AfterExecute();
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
}

void HidumperDumpersTest::HandleDumperComon(std::string dumperType)
{
    std::shared_ptr<HidumperExecutor> dumper = std::make_shared<FileStreamDumper>();
    if (StringUtils::GetInstance().IsSameStr(dumperType, "CMDDumper")) {
        dumper = std::make_shared<CMDDumper>();
    }
    dumper->SetDumpConfig(g_config);
    DumpStatus ret = dumper->DoPreExecute(g_parameter, g_dump_datas);
    ASSERT_TRUE(ret == DumpStatus::DUMP_OK) << "PreExecute failed.";
    ret = DumpStatus::DUMP_MORE_DATA;
    while (ret == DumpStatus::DUMP_MORE_DATA) {
        // loop for all lines
        ret = dumper->DoExecute();
        ASSERT_TRUE(ret == DumpStatus::DUMP_OK || ret == DumpStatus::DUMP_MORE_DATA) << "Execute failed.";
        ret = dumper->DoAfterExecute();
        ASSERT_TRUE(ret == DumpStatus::DUMP_OK || ret == DumpStatus::DUMP_MORE_DATA) << "Execute failed.";
    }
}

void HidumperDumpersTest::HandleIpcStatDumperTest(void)
{
    auto parameter = std::make_shared<DumperParameter>();
    std::vector<std::u16string> args;
    DumperOpts opts;
    auto dumpDatas = std::make_shared<std::vector<std::vector<std::string>>>();
    auto ipcStatDumper = std::make_shared<IPCStatDumper>();

    int fd = open("/dev/null", O_RDWR | O_CREAT | O_TRUNC, 0664);
    if (fd <= 0) {
        fd = STDERR_FILENO;
    }
    std::shared_ptr<RawParam> rawParam = std::make_shared<RawParam>(0, 1, 0, args, fd);

    opts.isDumpIpc_ = true;
    opts.isDumpAllIpc_ = true;
    opts.isDumpIpcStartStat_ = true;
    parameter->SetOpts(opts);
    parameter->setClientCallback(rawParam);

    DumpStatus ret = DumpStatus::DUMP_FAIL;
    ret = ipcStatDumper->PreExecute(parameter, dumpDatas);
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
    ret = ipcStatDumper->Execute();
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
    ret = ipcStatDumper->AfterExecute();
    ASSERT_EQ(ret, DumpStatus::DUMP_OK);
}

/**
 * @tc.name: FileDumperTest001
 * @tc.desc: Test FileDumper base function with loop = TRUE.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperDumpersTest, FileDumperTest001, TestSize.Level3)
{
    g_config->name_ = "FileDumperTest";
    std::string file_name = FILE_CPUINFO;
    g_config->target_ = file_name;
    g_config->loop_ = DumperConstant::LOOP;
    g_config->args_ = OptionArgs::Create();
    g_config->args_->SetPid(DEFAULT_PID, DEFAULT_UID);
    g_config->args_->SetCpuId(0);
    HandleDumperComon("FileStreamDumper");
}

/**
 * @tc.name: FileDumperTest002
 * @tc.desc: Test FileDumper base function with loop = FALSE.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperDumpersTest, FileDumperTest002, TestSize.Level3)
{
    g_config->name_ = "FileDumperTest";
    std::string file_name = FILE_CPUINFO;
    g_config->target_ = file_name;
    g_config->loop_ = DumperConstant::NONE;
    HandleDumperComon("FileStreamDumper");
}

/**
 * @tc.name: FileDumperTest005
 * @tc.desc: Test FileDumper base function with replacing PID in file paths.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperDumpersTest, FileDumperTest005, TestSize.Level0)
{
    g_config->name_ = "FileDumperTest";
    std::string file_name = "/proc/%pid/smaps";
    g_config->target_ = file_name;
    g_config->loop_ = DumperConstant::LOOP;
    g_config->args_ = OptionArgs::Create();
    g_config->args_->SetPid(DEFAULT_PID, DEFAULT_UID);
    g_config->args_->SetCpuId(1);
    HandleDumperComon("FileStreamDumper");
}

/**
 * @tc.name: APIDumperTest001
 * @tc.desc: Test APIDumper target is build_version.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperDumpersTest, APIDumperTest001, TestSize.Level3)
{
    g_config->name_ = "dumper_build_id";
    g_config->target_ = "build_version";
    HandleDumperExcute("APIDumper");
}

/**
 * @tc.name: APIDumperTest002
 * @tc.desc: Test APIDumper target is hw_sc.build.os.releasetype.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperDumpersTest, APIDumperTest002, TestSize.Level3)
{
    g_config->name_ = "dumper_release_type";
    g_config->target_ = "hw_sc.build.os.releasetype";
    HandleDumperExcute("APIDumper");
}

/**
 * @tc.name: APIDumperTest003
 * @tc.desc: Test APIDumper target is hw_sc.build.os.version.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperDumpersTest, APIDumperTest003, TestSize.Level3)
{
    g_config->name_ = "dumper_os_version";
    g_config->target_ = "hw_sc.build.os.version";
    HandleDumperExcute("APIDumper");
}

/**
 * @tc.name: APIDumperTest004
 * @tc.desc: Test APIDumper target is system_param.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperDumpersTest, APIDumperTest004, TestSize.Level3)
{
    g_config->name_ = "dumper_system_param";
    g_config->target_ = "system_param";
    HandleDumperExcute("APIDumper");
}

/**
 * @tc.name: VersionDumperTest001
 * @tc.desc: Test VersionDumper.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperDumpersTest, VersionDumperTest001, TestSize.Level3)
{
    HandleDumperExcute("VersionDumper");
}

/**
 * @tc.name: CMDDumperTest001
 * @tc.desc: CMD Dumper base function with loop = TRUE.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperDumpersTest, CMDDumperTest001, TestSize.Level3)
{
    g_config->name_ = "CmdDumperTest";
    std::string cmd = "ps -ef";
    g_config->target_ = cmd;
    g_config->loop_ = DumperConstant::LOOP;
    HandleDumperComon("CMDDumper");
}

/**
 * @tc.name: CMDDumperTest002
 * @tc.desc: CMD Dumper base function with loop = False.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperDumpersTest, CMDDumperTest002, TestSize.Level3)
{
    g_config->name_ = "CmdDumperTest";
    std::string cmd = "ps -ef";
    g_config->target_ = cmd;
    g_config->loop_ = DumperConstant::NONE;
    HandleDumperComon("CMDDumper");
}

/**
 * @tc.name: MemoryDumperTest001
 * @tc.desc: Test MemoryDumper one process has correct ret.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperDumpersTest, MemoryDumperTest001, TestSize.Level1)
{
    HandleMemoryDumperTest(1);
}

/**
 * @tc.name: MemoryDumperTest002
 * @tc.desc: Test MemoryDumper all process has correct ret.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperDumpersTest, MemoryDumperTest002, TestSize.Level1)
{
    HandleMemoryDumperTest(-1);
}

/**
 * @tc.name: SADumperTest001
 * @tc.desc: Test SADumper no saname has correct ret.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperDumpersTest, SADumperTest001, TestSize.Level1)
{
    g_config->args_ = OptionArgs::Create();
    HandleDumperExcute("SADumper");
}

/**
 * @tc.name: SADumperTest002
 * @tc.desc: Test SADumper one saname has correct ret.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperDumpersTest, SADumperTest002, TestSize.Level1)
{
    g_config->args_ = OptionArgs::Create();
    const std::vector<std::string> names = {"1202"};
    const std::vector<std::string> args;
    g_config->args_->SetNamesAndArgs(names, args);
    HandleDumperExcute("SADumper");
}

/**
 * @tc.name: CpuDumperTest001
 * @tc.desc: Test CpuDumper dump all process has correct ret.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperDumpersTest, CpuDumperTest001, TestSize.Level1)
{
    HandleCpuDumperTest(-1);
}

/**
 * @tc.name: CpuDumperTest002
 * @tc.desc: Test CpuDumper has correct ret when opts is vaild.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperDumpersTest, CpuDumperTest002, TestSize.Level1)
{
    HandleCpuDumperTest(getpid());
}

/**
 * @tc.name: ListDumperTest001
 * @tc.desc: Test ListDumper dump ABILITY has correct ret.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperDumpersTest, ListDumperTest001, TestSize.Level1)
{
    g_config->target_ = ConfigUtils::STR_ABILITY;
    HandleDumperExcute("ListDumper");
}

/**
 * @tc.name: ListDumperTest002
 * @tc.desc: Test ListDumper dump SYSTEM has correct ret.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperDumpersTest, ListDumperTest002, TestSize.Level1)
{
    g_config->target_ = ConfigUtils::STR_SYSTEM;
    HandleDumperExcute("ListDumper");
}

/**
 * @tc.name: TrafficDumperTest001
 * @tc.desc: Test TrafficDumper dump valid process traffic information.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperDumpersTest, TrafficDumperTest001, TestSize.Level1)
{
    HandleTrafficDumperTest(1);
}

/**
 * @tc.name: TrafficDumperTest002
 * @tc.desc: Test TrafficDumper dump invalid process traffic information.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperDumpersTest, TrafficDumperTest002, TestSize.Level1)
{
    HandleTrafficDumperTest(-1);
}

/**
 * @tc.name: IpcStatDumperTest001
 * @tc.desc: Test IpcDumper dump all processes.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperDumpersTest, IpcStatDumperTest001, TestSize.Level1)
{
    HandleIpcStatDumperTest();
}
} // namespace HiviewDFX
} // namespace OHOS