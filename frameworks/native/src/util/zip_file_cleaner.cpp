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
#include "util/zip_file_cleaner.h"
#include "common/dumper_constant.h"
#include "directory_ex.h"
#include "dump_utils.h"
#include "hilog_wrapper.h"
#include <algorithm>
#include <sys/stat.h>

namespace OHOS {
namespace HiviewDFX {

int64_t ZipFileCleaner::GetDirTotalSize(const std::string &dirPath, std::vector<FileInfo>& files)
{
    if (!DumpUtils::DirectoryExists(dirPath)) {
        DUMPER_HILOGE(MODULE_COMMON, "Directory not exists: %{public}s", dirPath.c_str());
        return 0;
    }

    std::vector<std::string> allFiles;
    GetDirFiles(dirPath, allFiles);

    int64_t totalSize = 0;
    for (const auto& str : allFiles) {
        std::string filePath = IncludeTrailingPathDelimiter(ExtractFilePath(str));
        std::string fileName = ExtractFileName(str);
        std::string fileExt = ExtractFileExt(fileName);
        DUMPER_HILOGD(MODULE_COMMON, "EraseLogs debug|"
            "str=[%{public}s], filePath=[%{public}s], fileName=[%{public}s], fileExt=[%{public}s]",
            str.c_str(), filePath.c_str(), fileName.c_str(), fileExt.c_str());

        if ((filePath != dirPath) || (fileExt != ZIP_FILEEXT)) {
            DUMPER_HILOGD(MODULE_COMMON, "EraseLogs debug|skip, str=[%{public}s]", str.c_str());
            continue;
        }
        DUMPER_HILOGD(MODULE_COMMON, "EraseLogs debug|add, str=[%{public}s]", str.c_str());
        struct stat fileStat;
        if (stat(str.c_str(), &fileStat) != 0) {
            DUMPER_HILOGE(MODULE_COMMON, "Failed to get file stat: %{public}s", str.c_str());
            continue;
        }

        FileInfo info;
        info.path = str;
        info.size = fileStat.st_size;
        info.name = fileName;
        files.push_back(info);
        totalSize += info.size;
    }

    return totalSize;
}

bool ZipFileCleaner::DeleteOldestFiles(int64_t totalSize, std::vector<FileInfo>& files)
{
    if (files.empty()) {
        return true;
    }

    std::sort(files.begin(), files.end(),
        [](const FileInfo& a, const FileInfo& b) {
            return a.name < b.name;
        });

    int64_t currentSize = totalSize;
    int32_t fileCount = static_cast<int32_t>(files.size());
    int deletedCount = 0;
    for (const auto& file : files) {
        if (currentSize <= ZIP_TARGET_SIZE && fileCount <= LOGFILE_MAX) {
            break;
        }
        if (RemoveFile(file.path)) {
            currentSize -= file.size;
            fileCount--;
            deletedCount++;
            DUMPER_HILOGI(MODULE_COMMON, "Deleted old zip file: %{public}s, size: %{public}lld",
                file.path.c_str(), (long long)file.size);
        } else {
            DUMPER_HILOGE(MODULE_COMMON, "Failed to delete file: %{public}s, errno: %{public}d",
                file.path.c_str(), errno);
        }
    }

    DUMPER_HILOGI(MODULE_COMMON, "Cleaned %{public}d files, current size: %{public}lldMB",
        deletedCount, (long long)(currentSize / BYTES_PER_MB));
    return true;
}

bool ZipFileCleaner::CleanOldFiles(const std::string &dirPath)
{
    DUMPER_HILOGI(MODULE_COMMON, "Start cleaning old zip files in: %{public}s", dirPath.c_str());

    std::vector<FileInfo> files;
    int64_t totalSize = GetDirTotalSize(dirPath, files);

    DUMPER_HILOGI(MODULE_COMMON, "Total zip files size: %{public}lldMB, max size: %{public}lldMB",
        (long long)(totalSize / BYTES_PER_MB),
        (long long)(ZIP_MAX_SIZE / BYTES_PER_MB));

    if (totalSize <= ZIP_MAX_SIZE && files.size() <= LOGFILE_MAX) {
        DUMPER_HILOGI(MODULE_COMMON, "No need to clean, total size is under threshold");
        return true;
    }
    return DeleteOldestFiles(totalSize, files);
}

} // namespace HiviewDFX
} // namespace OHOS
