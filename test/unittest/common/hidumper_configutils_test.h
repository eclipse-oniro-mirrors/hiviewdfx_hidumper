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
#ifndef HIDUMPER_CONFIGUTILS_TEST_H
#define HIDUMPER_CONFIGUTILS_TEST_H
#include <gtest/gtest.h>
#include "util/config_utils.h"
#include "util/file_utils.h"
#include "dump_utils.h"
#include "util/zip/zip_writer.h"
namespace OHOS {
namespace HiviewDFX {
class HidumperConfigUtilsTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
protected:
    static const int PID_EMPTY;
    static const int UID_EMPTY;
    static const int PID_TEST;
    static const std::string DUMPER_NAME;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIDUMPER_CONFIGUTILS_TEST_H
