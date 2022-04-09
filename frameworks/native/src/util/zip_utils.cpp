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
#include "util/zip_utils.h"
#include "directory_ex.h"
#include "dump_utils.h"
#include "util/zip/zip_writer.h"
#include "hilog_wrapper.h"
namespace OHOS {
namespace HiviewDFX {
bool ZipUtils::ZipFolder(const std::string &srcPath, const std::string &dstFile, const ZipTickNotify notify)
{
    DUMPER_HILOGD(MODULE_COMMON, "enter|srcPath=[%{public}s], dstFile=[%{public}s]",
        srcPath.c_str(), dstFile.c_str());

    std::string srcFolder = IncludeTrailingPathDelimiter(srcPath);

    DUMPER_HILOGD(MODULE_COMMON, "debug|srcFolder=[%{public}s]", srcFolder.c_str());

    if (!DumpUtils::DirectoryExists(srcFolder)) {
        DUMPER_HILOGD(MODULE_COMMON, "leave|ret=false, srcFolder=[%{public}s]", srcFolder.c_str());
        return false;
    }

    DUMPER_HILOGD(MODULE_COMMON, "debug|GetDirFiles, srcFolder=[%{public}s]", srcFolder.c_str());

    std::vector<std::string> allFiles;
    GetDirFiles(srcFolder, allFiles);

    if ((notify != nullptr) && (notify(UNSET_PROGRESS, UNSET_PROGRESS))) {
        DUMPER_HILOGD(MODULE_COMMON, "leave|notify");
        return false;
    }

    for (auto str : allFiles) {
        DUMPER_HILOGD(MODULE_COMMON, "debug|str=[%{public}s]", str.c_str());
    }

    size_t zipRootLen = srcFolder.length();
    std::vector<std::pair<std::string, std::string>> zipItems;
    std::transform(allFiles.begin(), allFiles.end(), std::back_inserter(zipItems),
        [zipRootLen](const std::string &str) {
            return std::make_pair(str, str.substr(zipRootLen)); // first:absolutePath, second:relativePath
        });
    allFiles.clear();

    for (auto zipItem : zipItems) {
        DUMPER_HILOGD(MODULE_COMMON, "debug|zipItems, absPath=[%{public}s], relPath=[%{public}s]",
            zipItem.first.c_str(), zipItem.second.c_str());
    }

    DUMPER_HILOGD(MODULE_COMMON, "debug|Create, dstFile=[%{public}s]", dstFile.c_str());

    ZipWriter zipWriter(dstFile);
    zipWriter.Open();
    bool ret = zipWriter.Write(zipItems, notify);

    DUMPER_HILOGD(MODULE_COMMON, "leave|ret=%{public}d", ret);
    return ret;
}
} // namespace HiviewDFX
} // namespace OHOS
