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
#include "dump_log_manager.h"
#include "directory_ex.h"
#include "common/dumper_constant.h"
#include "hilog_wrapper.h"
#include "util/zip_file_cleaner.h"
namespace OHOS {
namespace HiviewDFX {
namespace {
static const std::string TMP_FOLDER = "/data/log/hidumper/tmp/";

} // namespace
DumpLogManager::DumpLogManager()
{
}

DumpLogManager::~DumpLogManager()
{
}

bool DumpLogManager::Init()
{
    DUMPER_HILOGD(MODULE_COMMON, "Init enter|");

    ForceRemoveDirectory(TMP_FOLDER);
    ForceCreateDirectory(TMP_FOLDER);
    ForceCreateDirectory(ZIP_FOLDER);
    EraseLogs();

    DUMPER_HILOGD(MODULE_COMMON, "Init leave|");
    return true;
}

void DumpLogManager::Uninit()
{
    ForceRemoveDirectory(TMP_FOLDER);
    EraseLogs();
}

void DumpLogManager::EraseLogs()
{
    DUMPER_HILOGD(MODULE_COMMON, "EraseLogs enter|");

    ZipFileCleaner::CleanOldFiles(ZIP_FOLDER);
    DUMPER_HILOGD(MODULE_COMMON, "EraseLogs leave|");
}

std::string DumpLogManager::CreateTmpFolder(uint32_t id)
{
    DUMPER_HILOGD(MODULE_COMMON, "CreateTmpFolder enter|id=%{public}d", id);

    std::string ret = IncludeTrailingPathDelimiter(TMP_FOLDER + std::to_string(id));
    bool res = ForceCreateDirectory(ret);

    DUMPER_HILOGD(MODULE_COMMON, "CreateTmpFolder leave|ret=%{public}s, res=[%{public}d]", ret.c_str(), res);
    return ret;
}

bool DumpLogManager::EraseTmpFolder(uint32_t id)
{
    DUMPER_HILOGD(MODULE_COMMON, "EraseTmpFolder enter|id=%{public}d", id);

    std::string folder = IncludeTrailingPathDelimiter(TMP_FOLDER + std::to_string(id));
    bool ret = ForceRemoveDirectory(folder);

    DUMPER_HILOGD(MODULE_COMMON, "EraseTmpFolder leave|ret=[%{public}d]", ret);
    return ret;
}
} // namespace HiviewDFX
} // namespace OHOS
