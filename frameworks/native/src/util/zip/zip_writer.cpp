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
#include "util/zip/zip_writer.h"
#include <chrono>
#include "directory_ex.h"
#include "dump_utils.h"
#include "hilog_wrapper.h"
namespace OHOS {
namespace HiviewDFX {
namespace {
static const int PROCENT100 = 100;
static const int kZipBufSize = 8192;
static const int kBaseYear = 1900;
static const uLong LANGUAGE_ENCODING_FLAG = 0x1 << 11;
} // namespace

ZipWriter::ZipWriter(const std::string &zipFilePath) : zipFilePath_(zipFilePath), zipFile_(nullptr)
{
    DUMPER_HILOGD(MODULE_COMMON, "create|zipFilePath=[%{public}s]", zipFilePath_.c_str());
}

ZipWriter::~ZipWriter()
{
    DUMPER_HILOGD(MODULE_COMMON, "release|");
    zipItems_.clear();
    Close();
}

bool ZipWriter::Open()
{
    DUMPER_HILOGD(MODULE_COMMON, "Open enter|zipFilePath=[%{public}s]", zipFilePath_.c_str());

    if (zipFilePath_.empty()) {
        DUMPER_HILOGD(MODULE_COMMON, "Open leave|false, path is empty");
        return false;
    }

    DUMPER_HILOGD(MODULE_COMMON, "Open debug|");
    zipFile_ = OpenForZipping(zipFilePath_, APPEND_STATUS_CREATE);
    if (zipFile_ == nullptr) {
        DUMPER_HILOGD(MODULE_COMMON, "Open leave|false, couldn't create ZIP file");
        return false;
    }

    DUMPER_HILOGD(MODULE_COMMON, "Open leave|true, create ZIP file");
    return true;
}

bool ZipWriter::Close()
{
    DUMPER_HILOGD(MODULE_COMMON, "Close enter|");

    int res = ZIP_OK;

    if (zipFile_ != nullptr) {
        res = zipClose(zipFile_, nullptr);
    }
    zipFile_ = nullptr;

    bool ret = (res == ZIP_OK);

    DUMPER_HILOGD(MODULE_COMMON, "Close leave|ret=%{public}d, res=%{public}d", ret, res);
    return ret;
}

bool ZipWriter::Write(const std::vector<std::pair<std::string, std::string>> &zipItems, const ZipTickNotify notify)
{
    DUMPER_HILOGD(MODULE_COMMON, "Write enter|");

    if (zipFile_ == nullptr) {
        return false;
    }

    zipItems_.insert(zipItems_.end(), zipItems.begin(), zipItems.end());

    bool ret = FlushItems(notify);
    DUMPER_HILOGD(MODULE_COMMON, "Write debug|FlushItems, ret=%{public}d", ret);

    if (ret) {
        ret = Close();
    }

    DUMPER_HILOGD(MODULE_COMMON, "Write leave|ret=%{public}d", ret);
    return ret;
}

bool ZipWriter::FlushItems(const ZipTickNotify notify)
{
    DUMPER_HILOGD(MODULE_COMMON, "FlushItems enter|");

    std::vector<std::pair<std::string, std::string>> zipItems;
    zipItems.assign(zipItems_.begin(), zipItems_.end());
    zipItems_.clear();

    bool ret = true;
    for (size_t i = 0; i < zipItems.size(); i++) {
        if ((notify != nullptr) && (notify(((PROCENT100 * i) / zipItems.size()), UNSET_PROGRESS))) {
            DUMPER_HILOGE(MODULE_COMMON, "FlushItems error|notify");
            ret = false;
            break;
        }

        auto item = zipItems[i];
        std::string absolutePath = item.first; // first:absolutePath
        std::string relativePath = item.second; // second:relativePath
        DUMPER_HILOGD(MODULE_COMMON, "FlushItems debug|relativePath=[%{public}s], absolutePath=[%{public}s]",
            relativePath.c_str(), absolutePath.c_str());

        if (!AddFileEntryToZip(zipFile_, relativePath, absolutePath)) {
            DUMPER_HILOGE(MODULE_COMMON, "FlushItems error|false, failed to write file");
            ret = false;
            break;
        }
    }

    DUMPER_HILOGD(MODULE_COMMON, "FlushItems leave|ret=%{public}d", ret);
    return ret;
}

bool ZipWriter::SetTimeToZipFileInfo(zip_fileinfo &zipInfo)
{
    auto nowTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm localTime = {0};
    if (localtime_r(&nowTime, &localTime) == nullptr) {
        DUMPER_HILOGE(MODULE_COMMON, "SetTimeToZipFileInfo error|nullptr");
        return false;
    }

    zipInfo.tmz_date.tm_year = static_cast<uInt>(localTime.tm_year + kBaseYear);
    zipInfo.tmz_date.tm_mon = static_cast<uInt>(localTime.tm_mon);
    zipInfo.tmz_date.tm_mday = static_cast<uInt>(localTime.tm_mday);
    zipInfo.tmz_date.tm_hour = static_cast<uInt>(localTime.tm_hour);
    zipInfo.tmz_date.tm_min = static_cast<uInt>(localTime.tm_min);
    zipInfo.tmz_date.tm_sec = static_cast<uInt>(localTime.tm_sec);
    return true;
}

zipFile ZipWriter::OpenForZipping(const std::string &fileName, int append)
{
    return zipOpen2(fileName.c_str(), append, nullptr, nullptr);
}

bool ZipWriter::ZipOpenNewFileInZip(zipFile zip_file, const std::string &strPath)
{
    DUMPER_HILOGD(MODULE_COMMON, "ZipOpenNewFileInZip enter|strPath=[%{public}s]", strPath.c_str());

    zip_fileinfo fileInfo = {};
    SetTimeToZipFileInfo(fileInfo);

    int res = zipOpenNewFileInZip4(zip_file, strPath.c_str(), &fileInfo,
        nullptr, 0u, nullptr, 0u, nullptr, Z_DEFLATED, Z_DEFAULT_COMPRESSION,
        0, -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, nullptr, 0, 0, LANGUAGE_ENCODING_FLAG);

    bool ret = (res == ZIP_OK);

    DUMPER_HILOGD(MODULE_COMMON, "ZipOpenNewFileInZip leave|ret=%{public}d, res=%{public}d", ret, res);
    return ret;
}

bool ZipWriter::AddFileContentToZip(zipFile zip_file, std::string &file_path)
{
    DUMPER_HILOGD(MODULE_COMMON, "AddFileContentToZip enter|file_path=[%{public}s]", file_path.c_str());

    if (!DumpUtils::PathIsValid(file_path)) {
        DUMPER_HILOGE(MODULE_COMMON, "AddFileContentToZip leave|false, PathIsValid");
        return false;
    }

    auto fp = fopen(file_path.c_str(), "rb");
    if (fp == nullptr) {
        DUMPER_HILOGE(MODULE_COMMON, "AddFileContentToZip leave|false, fopen");
        return false;
    }

    bool ret = true;
    char buf[kZipBufSize];
    while (!feof(fp)) {
        size_t readSum = fread(buf, 1, kZipBufSize, fp);
        if (readSum < 1) {
            continue;
        }

        if (zipWriteInFileInZip(zip_file, buf, readSum) != ZIP_OK) {
            DUMPER_HILOGE(MODULE_COMMON, "AddFileContentToZip error|could not write data to zip");
            ret = false;
            break;
        }
    }

    (void)fclose(fp);
    fp = nullptr;

    DUMPER_HILOGD(MODULE_COMMON, "AddFileContentToZip leave|ret=%{public}d", ret);
    return ret;
}

bool ZipWriter::OpenNewFileEntry(zipFile zip_file, std::string &path)
{
    DUMPER_HILOGD(MODULE_COMMON, "OpenNewFileEntry enter|path=[%{public}s]", path.c_str());

    bool ret = ZipOpenNewFileInZip(zip_file, path);

    DUMPER_HILOGD(MODULE_COMMON, "OpenNewFileEntry leave|ret=%{public}d", ret);
    return ret;
}

bool ZipWriter::CloseNewFileEntry(zipFile zip_file)
{
    DUMPER_HILOGD(MODULE_COMMON, "CloseNewFileEntry enter|");

    int res = zipCloseFileInZip(zip_file);
    bool ret = (res == ZIP_OK);

    DUMPER_HILOGD(MODULE_COMMON, "CloseNewFileEntry leave|ret=%{public}d, res=%{public}d", ret, res);
    return ret;
}

bool ZipWriter::AddFileEntryToZip(zipFile zip_file, std::string &relativePath, std::string &absolutePath)
{
    DUMPER_HILOGD(MODULE_COMMON, "AddFileEntryToZip enter|relativePath=[%{public}s], absolutePath=[%{public}s]",
        relativePath.c_str(), absolutePath.c_str());

    if (!OpenNewFileEntry(zip_file, relativePath)) {
        DUMPER_HILOGD(MODULE_COMMON, "AddFileEntryToZip leave|false, open");
        return false;
    }

    bool ret = AddFileContentToZip(zip_file, absolutePath);

    if (!CloseNewFileEntry(zip_file)) {
        DUMPER_HILOGD(MODULE_COMMON, "AddFileEntryToZip leave|false, close");
        return false;
    }

    DUMPER_HILOGD(MODULE_COMMON, "AddFileEntryToZip leave|ret=%{public}d", ret);
    return ret;
}
} // namespace HiviewDFX
} // namespace OHOS
