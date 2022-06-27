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
#include "executor/zip_output.h"
#include <unistd.h>
#include "securec.h"
#include "dump_utils.h"
#include "util/file_utils.h"
#include "common/dumper_constant.h"
namespace OHOS {
namespace HiviewDFX {
ZipOutput::ZipOutput() : fd_(-1)
{
}

ZipOutput::~ZipOutput()
{
    if (fd_ >= 0) {
        close(fd_);
    }
}

DumpStatus ZipOutput::PreExecute(const std::shared_ptr<DumperParameter>& parameter,
    StringMatrix dumpDatas)
{
    mDumpDatas_ = dumpDatas;
    if (mDumpDatas_.get() == nullptr) {
        return DumpStatus::DUMP_FAIL;
    }

    // init myself once
    if (mFilePath_.empty()) {
        mFilePath_ =  parameter->GetOpts().path_;
        srcBuffer_ = std::make_shared<CompressBuffer>();
        destBuffer_ = std::make_shared<CompressBuffer>();
        ClearBuffer();
        fd_= DumpUtils::FdToWrite(mFilePath_);
        if (fd_ < 0) {
            return DumpStatus::DUMP_FAIL;
        }
    }

    return DumpStatus::DUMP_OK;
}

DumpStatus ZipOutput::Execute()
{
    if (!mDumpDatas_.get() || (fd_ < 0)) {
        return DumpStatus::DUMP_FAIL;
    }
    CompressBuffer* srcBuffer = srcBuffer_.get();
    CompressBuffer* destBuffer = destBuffer_.get();
    if (!srcBuffer || !destBuffer) {
        return DumpStatus::DUMP_FAIL;
    }
    DumpCompressor compressor;
    for (auto v_line : *(mDumpDatas_.get())) {
        // whole line
        std::string line;
        for (auto v_content : v_line)  {
            line.append(v_content);
        }
        const char* content = line.c_str();
        uint16_t content_size = strlen(content);
        uint16_t size = strlen(content) + 1; // include \n
        // buffer is full
        if ((srcBuffer->offset + size) > MAX_COMPRESS_BUFFER_SIZE) {
            // compress and write to file
            CompressAndWriteToFd();
        }
        // Process big line.
        if (size > MAX_COMPRESS_BUFFER_SIZE) {
            CompressAndWriteToFd(); // clear buffer
            CompressBigLine(content, size);
            continue;
        }
        // Add to buffer end with \n
        if (memcpy_s(srcBuffer->content + srcBuffer->offset,
            MAX_COMPRESS_BUFFER_SIZE - srcBuffer->offset, content, content_size) != EOK) {
            LOG_DEBUG("ZipOutput::Execute() memcpy_s failed!\n");
            return DumpStatus::DUMP_FAIL;
        }
        srcBuffer->offset = srcBuffer->offset + content_size;
        srcBuffer->content[srcBuffer->offset] = '\n';
        srcBuffer->offset = srcBuffer->offset + 1;
    }
    // compress and write to file
    CompressAndWriteToFd();
    // clear dump data.
    mDumpDatas_->clear();
    return DumpStatus::DUMP_OK;
}

DumpStatus ZipOutput::CompressAndWriteToFd()
{
    CompressBuffer* srcBuffer = srcBuffer_.get();
    CompressBuffer* destBuffer = destBuffer_.get();
    if (!srcBuffer || !destBuffer) {
        return DumpStatus::DUMP_FAIL;
    }
    DumpStatus ret = DumpStatus::DUMP_OK;
    if (srcBuffer->offset > 0) {
        ret = compressor_.Compress(srcBuffer, destBuffer);
        if (ret == DumpStatus::DUMP_OK) {
            if (write(fd_, destBuffer->content, destBuffer->offset) < 0) {
                ret = DumpStatus::DUMP_FAIL;
                LOG_DEBUG("ZipOutput::CompressAndWriteToFd() Write to FD failed!\n");
            }
        } else {
            ret = DumpStatus::DUMP_FAIL;
            LOG_DEBUG("ZipOutput::CompressAndWriteToFd() Compress failed!\n");
        }
        ClearBuffer();
    }

    return ret;
}

DumpStatus ZipOutput::CompressBigLine(const char* content, uint16_t size)
{
    if (!content) {
        return DumpStatus::DUMP_FAIL;
    }
    CompressBuffer* srcBuffer = srcBuffer_.get();
    CompressBuffer* destBuffer = destBuffer_.get();
    if (!srcBuffer || !destBuffer) {
        return DumpStatus::DUMP_FAIL;
    }
    DumpStatus ret = DumpStatus::DUMP_OK;
    uint16_t remain_size = size - 1; // size include \n
    uint16_t complete_size = 0;
    while (remain_size > 0) {
        uint16_t process_size = remain_size;
        if ((srcBuffer->offset + remain_size) > MAX_COMPRESS_BUFFER_SIZE) {
            process_size = MAX_COMPRESS_BUFFER_SIZE - srcBuffer->offset;
        }
        if (memcpy_s(srcBuffer->content + srcBuffer->offset,
            MAX_COMPRESS_BUFFER_SIZE - srcBuffer->offset,
            content + complete_size, process_size) != EOK) {
            LOG_DEBUG("ZipOutput::CompressBigLine() memcpy_s failed!\n");
            return DumpStatus::DUMP_FAIL;
        }
        srcBuffer->offset = srcBuffer->offset + process_size;
        complete_size += process_size;
        remain_size -= process_size;
        CompressAndWriteToFd();
    }
    // process \n
    srcBuffer->content[srcBuffer->offset] = '\n';
    srcBuffer->offset = srcBuffer->offset + 1;
    CompressAndWriteToFd();
    return ret;
}


DumpStatus ZipOutput::AfterExecute()
{
    return DumpStatus::DUMP_OK;
}

void ZipOutput::ClearBuffer()
{
    if (srcBuffer_.get()) {
        (void)memset_s(srcBuffer_.get(), sizeof(CompressBuffer), 0, sizeof(CompressBuffer));
    }
    if (destBuffer_.get()) {
        (void)memset_s(destBuffer_.get(), sizeof(CompressBuffer), 0, sizeof(CompressBuffer));
    }
}
} // namespace HiviewDFX
} // namespace OHOS
