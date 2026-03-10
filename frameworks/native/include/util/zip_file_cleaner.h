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
#ifndef HIDUMPER_UTIL_ZIP_FILE_CLEANER_H
#define HIDUMPER_UTIL_ZIP_FILE_CLEANER_H

#include <string>
#include <vector>
#include <sys/stat.h>

namespace OHOS {
namespace HiviewDFX {

constexpr int BYTES_PER_KB = 1024;
constexpr int BYTES_PER_MB = 1024 * 1024;
constexpr int ZIP_FILE_EXT_LEN = 4;
constexpr int LOGFILE_MAX = 20;

struct FileInfo {
    std::string path;
    int64_t size;
    std::string name;
};

class ZipFileCleaner {
public:
    static bool CleanOldFiles(const std::string &dirPath);

private:
    static int64_t GetDirTotalSize(const std::string &dirPath, std::vector<FileInfo>& files);
    static bool DeleteOldestFiles(int64_t totalSize, std::vector<FileInfo>& files);
};

} // namespace HiviewDFX
} // namespace OHOS

#endif // HIDUMPER_UTIL_ZIP_FILE_CLEANER_H
