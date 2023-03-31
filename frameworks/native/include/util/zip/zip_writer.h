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
#ifndef HIDUMPER_UTIL_ZIP_WRITER_H
#define HIDUMPER_UTIL_ZIP_WRITER_H
#include <string>
#include <vector>
#include "contrib/minizip/zip.h"
#include "util/zip/zip_common_type.h"
#ifdef DUMP_TEST_MODE // for mock test
#include "gtest/gtest_prod.h"
#endif // for mock test
namespace OHOS {
namespace HiviewDFX {
class ZipWriter {
public:
    ZipWriter(const std::string &zipFilePath);
    ~ZipWriter();
public:
    bool Open();
    bool Close();
    // zipItems: first:absolutePath, second:relativePath
    bool Write(const std::vector<std::pair<std::string, std::string>> &zipItems,
        const ZipTickNotify notify = nullptr);
private:
    bool FlushItems(const ZipTickNotify notify = nullptr);
    static bool SetTimeToZipFileInfo(zip_fileinfo &zipInfo);
    static zipFile OpenForZipping(const std::string &fileName, int append);
    static bool ZipOpenNewFileInZip(zipFile zip_file, const std::string &strPath);
    static bool AddFileContentToZip(zipFile zip_file, std::string &file_path);
    static bool OpenNewFileEntry(zipFile zip_file, std::string &path);
    static bool CloseNewFileEntry(zipFile zip_file);
    static bool AddFileEntryToZip(zipFile zip_file, std::string &relativePath, std::string &absolutePath);
private:
    std::vector<std::pair<std::string, std::string>> zipItems_;
    std::string zipFilePath_;
    zipFile zipFile_;
#ifdef DUMP_TEST_MODE // for mock test
    FRIEND_TEST(HidumperConfigUtilsTest, HidumperZipWriter001);
#endif // for mock test
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIDUMPER_UTIL_ZIP_WRITER_H
