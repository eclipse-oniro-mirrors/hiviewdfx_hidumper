/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#ifndef HIDUMPER_SERVICES_DUMPER_CONSTANT_H
#define HIDUMPER_SERVICES_DUMPER_CONSTANT_H

#include <string>
#include <vector>
#include <iostream>
#include <memory>

namespace OHOS {
namespace HiviewDFX {
enum DumperConstant {
    NONE,
    GROUP,        // group
    DUMPER_BEGIN, // dumper begin
    CPU_DUMPER,
    FILE_DUMPER,
    ENV_PARAM_DUMPER,
    CMD_DUMPER,
    PROPERTIES_DUMPER,
    API_DUMPER,
    LIST_DUMPER,
    VERSION_DUMPER,
    SA_DUMPER,
    MEMORY_DUMPER,
    STACK_DUMPER,
    DUMPER_END,   // dumper end
    FILTER_BEGIN, // filter begin
    COLUMN_ROWS_FILTER,
    FILE_FORMAT_DUMP_FILTER,
    FILTER_END,   // filter end
    OUTPUT_BEGIN, // output begin
    STD_OUTPUT,
    FILE_OUTPUT,
    FD_OUTPUT,
    ZIP_OUTPUT,
    OUTPUT_END,  // output end
    LEVEL_BEGIN, // level begin
    LEVEL_NONE,
    LEVEL_MIDDLE,
    LEVEL_HIGH,
    LEVEL_ALL,
    LEVEL_END,       // level end
    GROUPTYPE_BEGIN, // group type begin
    GROUPTYPE_PID,   // pid of group type
    GROUPTYPE_CPUID, // cpuid of group type
    GROUPTYPE_END,   // group type end
    LOOP,
};

const std::string LOG_DEFAULT = "log.txt";
const std::string ZIP_FILEEXT = "zip";
const std::string ZIP_FOLDER = "/data/log/hidumper/";

const std::string RELEASE_MODE = "Release";
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIDUMPER_SERVICES_DUMPER_CONSTANT_H
