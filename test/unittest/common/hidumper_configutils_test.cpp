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
#include <vector>
#include "hidumper_configutils_test.h"
#include "dump_common_utils.h"
#include "dumper_opts.h"
#include "raw_param.h"
using namespace std;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::HiviewDFX;
namespace OHOS {
namespace HiviewDFX {
const int HidumperConfigUtilsTest::PID_EMPTY = -1;
const int HidumperConfigUtilsTest::UID_EMPTY = -1;
const int HidumperConfigUtilsTest::PID_TEST = 100;
const std::string HidumperConfigUtilsTest::DUMPER_NAME = "dumper_kernel_version";
const OHOS::HiviewDFX::DumpCommonUtils DUMP_COMMON_UTILS = *(std::make_shared<OHOS::HiviewDFX::DumpCommonUtils>());
void HidumperConfigUtilsTest::SetUpTestCase(void)
{
}

void HidumperConfigUtilsTest::TearDownTestCase(void)
{
}

void HidumperConfigUtilsTest::SetUp(void)
{
}

void HidumperConfigUtilsTest::TearDown(void)
{
}

/**
 * @tc.name: HidumperConfigUtils001
 * @tc.desc: Test GetDumper by index
 * @tc.type: FUNC
 */
HWTEST_F(HidumperConfigUtilsTest, HidumperConfigUtils001, TestSize.Level3)
{
    int index = -1;
    std::vector<std::shared_ptr<DumpCfg>> result;
    ConfigUtils configUtils(nullptr);
    auto args = OptionArgs::Create();
    args->SetPid(PID_EMPTY, UID_EMPTY);
    DumpStatus ret = configUtils.GetDumper(index, result, args);
    EXPECT_NE(ret, DumpStatus::DUMP_OK);

    index = 100000;
    result.clear();
    ret = configUtils.GetDumper(index, result, args);
    EXPECT_NE(ret, DumpStatus::DUMP_OK);

    index = 0;
    result.clear();
    ret = configUtils.GetDumper(index, result, args);
    EXPECT_EQ(ret, DumpStatus::DUMP_OK);

    ASSERT_TRUE(!result.empty());
    ASSERT_TRUE(!result[0]->name_.empty());
    ASSERT_TRUE(result[0]->target_.empty());
    ASSERT_TRUE(result[0]->args_ == args);
}

/**
 * @tc.name: HidumperConfigUtils002
 * @tc.desc: Test GetDumper by index.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperConfigUtilsTest, HidumperConfigUtils002, TestSize.Level3)
{
    int index = 1;
    std::vector<std::shared_ptr<DumpCfg>> result;
    ConfigUtils configUtils(nullptr);
    auto args = OptionArgs::Create();
    args->SetPid(PID_EMPTY, UID_EMPTY);
    DumpStatus ret = configUtils.GetDumper(index, result, args);
    EXPECT_EQ(ret, DumpStatus::DUMP_OK);

    std::size_t allSum = result.size();
    index = 1;
    result.clear();
    ret = configUtils.GetDumper(index, result, args);
    EXPECT_EQ(ret, DumpStatus::DUMP_OK);
    std::size_t highSum = result.size();

    index = 1;
    result.clear();
    ret = configUtils.GetDumper(index, result, args);
    EXPECT_EQ(ret, DumpStatus::DUMP_OK);
    std::size_t middleSum = result.size();

    index = 1;
    result.clear();
    ret = configUtils.GetDumper(index, result, args);
    EXPECT_EQ(ret, DumpStatus::DUMP_OK);
    std::size_t lowSum = result.size();

    index = 1;
    result.clear();
    ret = configUtils.GetDumper(index, result, args);
    EXPECT_EQ(ret, DumpStatus::DUMP_OK);
    std::size_t noneSum = result.size();

    ASSERT_TRUE(highSum <= allSum);
    ASSERT_TRUE(middleSum <= allSum);
    ASSERT_TRUE(lowSum <= allSum);
    ASSERT_TRUE(noneSum <= allSum);
}

/**
 * @tc.name: HidumperConfigUtils003
 * @tc.desc: Test GetDumper by name.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperConfigUtilsTest, HidumperConfigUtils003, TestSize.Level3)
{
    std::string name;
    std::vector<std::shared_ptr<DumpCfg>> result;
    ConfigUtils configUtils(nullptr);
    auto args = OptionArgs::Create();
    args->SetPid(PID_EMPTY, UID_EMPTY);
    DumpStatus ret = configUtils.GetDumper(name, result, args);
    EXPECT_NE(ret, DumpStatus::DUMP_OK);

    name = DUMPER_NAME;
    result.clear();
    ret = configUtils.GetDumper(name, result, args);
    EXPECT_EQ(ret, DumpStatus::DUMP_OK);

    ASSERT_TRUE(!result.empty());
    ASSERT_TRUE(result[0]->name_ == name);
}

/**
 * @tc.name: HidumperConfigUtils004
 * @tc.desc: Test configUtil HandleDumpLog.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperConfigUtilsTest, HidumperConfigUtils004, TestSize.Level3)
{
    std::vector<shared_ptr<DumpCfg>> dumpCfgs;
    auto parameter = std::make_shared<DumperParameter>();
    DumperOpts opts;
    opts.isDumpLog_ = true;
    opts.logArgs_.push_back("hidumper");
    parameter->SetOpts(opts);
    ConfigUtils configUtils(parameter);
    bool ret = configUtils.HandleDumpLog(dumpCfgs);
    ASSERT_TRUE(ret);
}

/**
 * @tc.name: HidumperConfigUtils005
 * @tc.desc: Test configUtil HandleDumpLog.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperConfigUtilsTest, HidumperConfigUtils005, TestSize.Level3)
{
    std::vector<shared_ptr<DumpCfg>> dumpCfgs;
    auto parameter = std::make_shared<DumperParameter>();
    DumperOpts opts;
    opts.isDumpLog_ = true;
    opts.logArgs_.push_back("hidumper");
    parameter->SetOpts(opts);
    ConfigUtils configUtils(parameter);
    bool ret = configUtils.HandleDumpLog(dumpCfgs);
    ASSERT_TRUE(ret);
}

/**
 * @tc.name: HidumperConfigUtils006
 * @tc.desc: Test configUtil isDumpSystemSystem.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperConfigUtilsTest, HidumperConfigUtils006, TestSize.Level3)
{
    std::vector<shared_ptr<DumpCfg>> dumpCfgs;
    auto parameter = std::make_shared<DumperParameter>();
    DumperOpts opts;
    opts.isDumpSystem_ = true;
    opts.isDumpService_ = true;
    opts.isDumpList_ = false;
    opts.isDumpCpuFreq_ = true;
    opts.isDumpCpuUsage_ = true;
    opts.isDumpMem_ = true;
    opts.isShowSmaps_ = true;
    opts.isDumpStorage_ = true;
    opts.isDumpNet_ = true;
    opts.isAppendix_ = true;
    opts.logArgs_.push_back("hidumper");
    parameter->SetOpts(opts);
    ConfigUtils configUtils(parameter);
    configUtils.HandleDumpSystem(dumpCfgs);
    bool ret = true;
    ret = configUtils.HandleDumpCpuFreq(dumpCfgs);
    ASSERT_FALSE(ret);
    ret = configUtils.HandleDumpCpuUsage(dumpCfgs);
    ASSERT_FALSE(ret);
    ret = configUtils.HandleDumpMem(dumpCfgs);
    ASSERT_FALSE(ret);
    ret = configUtils.HandleDumpMemShowMaps(dumpCfgs);
    ASSERT_FALSE(ret);
    ret = configUtils.HandleDumpStorage(dumpCfgs);
    ASSERT_FALSE(ret);
    ret = configUtils.HandleDumpNet(dumpCfgs);
    ASSERT_FALSE(ret);
    ret = configUtils.HandleDumpAppendix(dumpCfgs);
    ASSERT_TRUE(ret);
}

/**
 * @tc.name: HidumperConfigUtils007
 * @tc.desc: Test configUtil HandleDumpService.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperConfigUtilsTest, HidumperConfigUtils007, TestSize.Level3)
{
    std::vector<shared_ptr<DumpCfg>> dumpCfgs;
    auto parameter = std::make_shared<DumperParameter>();
    DumperOpts opts;
    opts.isDumpList_ = false;
    opts.isDumpService_ = true;
    opts.logArgs_.push_back("hidumper");
    parameter->SetOpts(opts);
    ConfigUtils configUtils(parameter);
    bool ret = configUtils.HandleDumpService(dumpCfgs);
    ASSERT_TRUE(ret);
}

/**
 * @tc.name: HidumperConfigUtils008
 * @tc.desc: Test configUtil HandleDumpList.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperConfigUtilsTest, HidumperConfigUtils008, TestSize.Level3)
{
    std::vector<shared_ptr<DumpCfg>> dumpCfgs;
    auto parameter = std::make_shared<DumperParameter>();
    DumperOpts opts;
    opts.isDumpSystemAbility_ = false;
    opts.isDumpList_ = true;
    opts.isDumpService_ = true;
    opts.logArgs_.push_back("hidumper");
    parameter->SetOpts(opts);
    ConfigUtils configUtils(parameter);
    bool ret = configUtils.HandleDumpList(dumpCfgs);
    ASSERT_TRUE(ret);
}

/**
 * @tc.name: HidumperConfigUtils009
 * @tc.desc: Test configUtil HandleDumpJsHeapMem.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperConfigUtilsTest, HidumperConfigUtils009, TestSize.Level3)
{
    std::vector<shared_ptr<DumpCfg>> dumpCfgs;
    auto parameter = std::make_shared<DumperParameter>();
    DumperOpts opts;
    opts.isDumpJsHeapMem_ = true;
    opts.dumpJsHeapMemPid_ = PID_EMPTY;
    parameter->SetOpts(opts);
    ConfigUtils configUtils(parameter);
    bool ret = configUtils.HandleDumpJsHeapMem(dumpCfgs);
    ASSERT_FALSE(ret);
}

/**
 * @tc.name: HidumperConfigUtils010
 * @tc.desc: Test configUtil HandleDumpStorage.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperConfigUtilsTest, HidumperConfigUtils010, TestSize.Level3)
{
    std::vector<shared_ptr<DumpCfg>> dumpCfgs;
    auto parameter = std::make_shared<DumperParameter>();
    DumperOpts opts;
    opts.isDumpStorage_ = true;
    opts.storagePid_ = PID_EMPTY;
    parameter->SetOpts(opts);
    ConfigUtils configUtils(parameter);
    bool ret = configUtils.HandleDumpStorage(dumpCfgs);
    ASSERT_TRUE(ret);
}

/**
 * @tc.name: HidumperConfigUtils011
 * @tc.desc: Test configUtil Nest overceed NEST_MAX.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperConfigUtilsTest, HidumperConfigUtils011, TestSize.Level3)
{
    std::string name = "configUtil test";
    std::vector<shared_ptr<DumpCfg>> result;
    std::shared_ptr<OptionArgs> args;
    ConfigUtils configUtils(nullptr);
    DumpStatus ret = configUtils.GetGroup(name, result, args, DumperConstant::LEVEL_HIGH, 11);
    ASSERT_EQ(ret, DumpStatus::DUMP_INVALID_ARG);
    ret = configUtils.GetGroup(0, result, args, DumperConstant::LEVEL_HIGH, 11);
    ASSERT_EQ(ret, DumpStatus::DUMP_INVALID_ARG);
}

/**
 * @tc.name: HidumperConfigUtils012
 * @tc.desc: Test configUtil CopySmaps.
 * @tc.type: FUNC
 */
HWTEST_F(HidumperConfigUtilsTest, HidumperConfigUtils012, TestSize.Level3)
{
    DumperOpts opts;
    auto parameter = std::make_shared<DumperParameter>();
    parameter->SetOpts(opts);
    ConfigUtils configUtils(parameter);
    bool ret = configUtils.CopySmaps();
    ASSERT_TRUE(ret == false);
}

HWTEST_F(HidumperConfigUtilsTest, HidumperZipWriter001, TestSize.Level3)
{
    string testfile = "/data/log/hidumpertest.txt";
    string testzipfile = "/data/log/hidumpertest.zip";
    system("touch /data/log/hidumpertest.txt");
    system("echo hidumpertest > /data/log/hidumpertest.txt");
    auto zipwriter = std::make_shared<ZipWriter>(testzipfile);
    ASSERT_TRUE(zipwriter->Open());
    ASSERT_TRUE(zipwriter->Close());
    auto testzip = zipwriter->OpenForZipping(testfile, APPEND_STATUS_CREATE);
    zipwriter->AddFileEntryToZip(testzip, testzipfile, testzipfile);
    system("rm -rf /data/log/hidumpertest.txt");
    system("rm -rf /data/log/hidumpertest.zip");
}

HWTEST_F(HidumperConfigUtilsTest, HidumperFileUtils001, TestSize.Level3)
{
    auto fileutils = std::make_shared<FileUtils>();
    string testpath = "/data";
    ASSERT_TRUE(fileutils->CreateFolder(testpath));
    testpath = "";
    ASSERT_TRUE(fileutils->CreateFolder(testpath));
    testpath = "test";
    ASSERT_TRUE(!(fileutils->CreateFolder(testpath)));
    testpath = "/data/log/testhidumper";
    ASSERT_TRUE(fileutils->CreateFolder(testpath));
    ASSERT_TRUE(access(testpath.c_str(), F_OK) == 0);
    system("rm -rf /data/log/testhidumper");
}

HWTEST_F(HidumperConfigUtilsTest, HidumpCommonUtils001, TestSize.Level3)
{
    system("mkdir /data/log/hidumpertest/");
    system("touch /data/log/hidumpertest/1");
    system("touch /data/log/hidumpertest/a");
    const std::string path = "/data/log/hidumpertest";
    bool digit = true;
    std::vector<std::string> strs = DUMP_COMMON_UTILS.GetSubNodes(path, digit);
    ASSERT_TRUE(strs.size() == 1);

    digit = false;
    strs = DUMP_COMMON_UTILS.GetSubNodes(path, digit);
    ASSERT_TRUE(strs.size() == 2);
    system("rm -rf /data/log/hidumpertest");
}

HWTEST_F(HidumperConfigUtilsTest, HidumpCommonUtils002, TestSize.Level3)
{
    const std::string pathTest = "/data";
    ASSERT_TRUE(DUMP_COMMON_UTILS.IsDirectory(pathTest));

    system("touch /data/log/hidumpertest.txt");
    const std::string pathTest2 = "/data/log/hidumpertest.txt";
    ASSERT_FALSE(DUMP_COMMON_UTILS.IsDirectory(pathTest2));
    system("rm -rf /data/log/hidumpertest.txt");
}

HWTEST_F(HidumperConfigUtilsTest, HidumpCommonUtils003, TestSize.Level3)
{
    std::vector<int32_t> pids = DUMP_COMMON_UTILS.GetAllPids();
    ASSERT_FALSE(pids.empty()) << "GetAllPids result is empty.";
}

HWTEST_F(HidumperConfigUtilsTest, HidumpCommonUtils004, TestSize.Level3)
{
    std::vector<DumpCommonUtils::CpuInfo> infos;
    ASSERT_TRUE(DUMP_COMMON_UTILS.GetCpuInfos(infos));
}

HWTEST_F(HidumperConfigUtilsTest, HidumpCommonUtils005, TestSize.Level3)
{
    std::vector<DumpCommonUtils::PidInfo> infos;
    ASSERT_TRUE(DUMP_COMMON_UTILS.GetPidInfos(infos));
    ASSERT_FALSE(infos.empty()) << "GetPidInfos result is empty.";

    std::vector<DumpCommonUtils::PidInfo> infosAll;
    ASSERT_TRUE(DUMP_COMMON_UTILS.GetPidInfos(infosAll, true));
}

HWTEST_F(HidumperConfigUtilsTest, HidumpCommonUtils006, TestSize.Level3)
{
    std::vector<int> pids;
    ASSERT_TRUE(DUMP_COMMON_UTILS.GetUserPids(pids));
}
} // namespace HiviewDFX
} // namespace OHOS