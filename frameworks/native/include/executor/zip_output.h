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
#ifndef ZIP_OUTPUT_H
#define ZIP_OUTPUT_H


#include "hidumper_executor.h"
#include "util/dump_compressor.h"

namespace OHOS {
namespace HiviewDFX {
class ZipOutput : public HidumperExecutor {
public:
    ZipOutput();
    ~ZipOutput();
    DumpStatus PreExecute(const std::shared_ptr<DumperParameter>& parameter,
        StringMatrix dumpDatas) override;
    DumpStatus Execute() override;
    DumpStatus AfterExecute() override;

private:
    DumpStatus CompressAndWriteToFd();
    DumpStatus CompressBigLine(const char* content, uint16_t size);
    void ClearBuffer();

private:
    std::string mFilePath_;
    StringMatrix mDumpDatas_;
    int fd_;

    DumpCompressor compressor_;
    std::shared_ptr<CompressBuffer> srcBuffer_;
    std::shared_ptr<CompressBuffer> destBuffer_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // ZIP_OUTPUT_H
