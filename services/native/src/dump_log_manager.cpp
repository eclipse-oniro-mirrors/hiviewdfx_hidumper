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
namespace OHOS {
namespace HiviewDFX {
namespace {
static const std::string TMP_FOLDER = "/data/log/hidumper/tmp/";
static const int LOGFILE_MAX = 10;
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

    std::vector<std::string> allFiles;
    GetDirFiles(ZIP_FOLDER, allFiles);

    std::vector<std::string> zipFiles;
    for (std::string str : allFiles) {
        std::string filePath = IncludeTrailingPathDelimiter(ExtractFilePath(str));
        std::string fileName = ExtractFileName(str);
        std::string fileExt = ExtractFileExt(fileName);
        DUMPER_HILOGD(MODULE_COMMON, "EraseLogs debug|"
            "str=[%{public}s], filePath=[%{public}s], fileName=[%{public}s], fileExt=[%{public}s]",
            str.c_str(), filePath.c_str(), fileName.c_str(), fileExt.c_str());

        if ((filePath != ZIP_FOLDER) || (fileExt != ZIP_FILEEXT)) {
            DUMPER_HILOGD(MODULE_COMMON, "EraseLogs debug|skip, str=[%{public}s]", str.c_str());
            continue;
        }

        DUMPER_HILOGD(MODULE_COMMON, "EraseLogs debug|add, str=[%{public}s]", str.c_str());
        zipFiles.push_back(str);
    }
    allFiles.clear();

    std::sort(zipFiles.begin(), zipFiles.end(),
        [] (const std::string &left, const std::string &right) {
        return (right.compare(left) > 0);
    });

    int sum = static_cast<int>(zipFiles.size()) - LOGFILE_MAX;
    DUMPER_HILOGD(MODULE_COMMON, "EraseLogs debug|sum=%{public}d", sum);
    for (int i = 0; i < sum; i++) {
        std::string &str = zipFiles[i];
        DUMPER_HILOGD(MODULE_COMMON, "EraseLogs debug|del, str=[%{public}s]", str.c_str());
        RemoveFile(str);
    }

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
