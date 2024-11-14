/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include "util/dump_compressor.h"
#include <securec.h>

using namespace std;
namespace OHOS {
namespace HiviewDFX {
DumpCompressor::DumpCompressor()
{
}

DumpStatus DumpCompressor::Init(CompressBuffer*& srcBuffer)
{
    uint32_t bound = compressBound(srcBuffer->offset);
    if (bound == 0) {
        return DumpStatus::DUMP_FAIL;
    }
    zDataSize_ = (bound < CHUNK) ? CHUNK : bound;
    zData_ = new unsigned char[zDataSize_];
    if (zData_ == nullptr) {
        return DumpStatus::DUMP_FAIL;
    }
    if (memset_s(zData_, zDataSize_, 0, zDataSize_) != EOK) {
        DeleteZData();
        return DumpStatus::DUMP_FAIL;
    };

    zStream_.zalloc = Z_NULL;
    zStream_.zfree = Z_NULL;
    zStream_.opaque = Z_NULL;

    if (deflateInit2(&zStream_, Z_DEFAULT_COMPRESSION, Z_DEFLATED, MAX_WBITS + WINDOWS_BITS,
        MEM_LEVEL, Z_DEFAULT_STRATEGY) != Z_OK) {
        DeleteZData();
        return DumpStatus::DUMP_FAIL;
    }
    return DumpStatus::DUMP_OK;
}

void DumpCompressor::DeleteZData()
{
    if (zData_ != nullptr) {
        delete[] zData_;
    };
    zData_ = nullptr;
}
void DumpCompressor::DeleteZBuff(char* buffIn, char* buffOut)
{
    delete[] buffIn;
    delete[] buffOut;
    buffIn = nullptr;
    buffOut = nullptr;
}
DumpStatus DumpCompressor::Compress(CompressBuffer*& srcBuffer, CompressBuffer*& destBuffer)
{
    if (Init(srcBuffer) != DumpStatus::DUMP_OK) {
        return DumpStatus::DUMP_FAIL;
    }
    size_t const toRead = CHUNK;
    size_t srcPos = 0;
    size_t dstPos = 0;
    size_t srcBufferOffset = srcBuffer->offset;
    int flush = 0;
    char* buffIn = new char[CHUNK];
    char* buffOut = new char[CHUNK];
    if (buffIn == nullptr || buffOut == nullptr) {
        return DumpStatus::DUMP_FAIL;
    }
    if (memset_s(buffIn, CHUNK, 0, CHUNK) != EOK ||
        memset_s(buffOut, CHUNK, 0, CHUNK) != EOK) {
        DeleteZBuff(buffIn, buffOut);
        return DumpStatus::DUMP_FAIL;
    };

    do {
        if (FillBuffer(flush, srcBuffer, buffIn, toRead, srcPos, srcBufferOffset) == DumpStatus::DUMP_FAIL) {
            DeleteZBuff(buffIn, buffOut);
            return DumpStatus::DUMP_FAIL;
        }
        if (DeflateBuffer(flush, buffOut, dstPos) == DumpStatus::DUMP_FAIL) {
            DeleteZBuff(buffIn, buffOut);
            return DumpStatus::DUMP_FAIL;
        }
    } while (flush != Z_FINISH);

    /* clean up and return */
    (void)deflateEnd(&zStream_);

    if (memcpy_s(destBuffer->content + destBuffer->offset,
        MAX_COMPRESS_BUFFER_SIZE - destBuffer->offset, zData_, dstPos) != EOK) {
        DeleteZData();
        return DumpStatus::DUMP_FAIL;
    }
    destBuffer->offset += dstPos;
    DeleteZData();
    DeleteZBuff(buffIn, buffOut);
    return DumpStatus::DUMP_OK;
}

DumpStatus DumpCompressor::FillBuffer(int& flush, CompressBuffer*& srcBuffer, char* buffIn,
    size_t const toRead, size_t& src_pos, size_t srcBufferOffset)
{
    bool flag = (srcBufferOffset - src_pos) < toRead;
    (void)memset_s(buffIn, CHUNK, 0, CHUNK);

    if (flag) {
        if (memmove_s(buffIn, CHUNK, srcBuffer->content + src_pos, srcBufferOffset - src_pos) != EOK) {
            DeleteZData();
            return DumpStatus::DUMP_FAIL;
        }
        zStream_.avail_in = srcBufferOffset - src_pos;
        src_pos += srcBufferOffset - src_pos;
    } else {
        if (memmove_s(buffIn, CHUNK, srcBuffer->content + src_pos, toRead) != EOK) {
            DeleteZData();
            return DumpStatus::DUMP_FAIL;
        };
        zStream_.avail_in = toRead;
        src_pos += toRead;
    }

    flush = flag ? Z_FINISH : Z_NO_FLUSH;
    zStream_.next_in = (Bytef*)buffIn;
    return DumpStatus::DUMP_OK;
}

DumpStatus DumpCompressor::DeflateBuffer(int flush, char* buffOut, size_t& dstPos)
{
    /* run deflate() on input , finish compression until all of source has been read in. */
    do {
        zStream_.avail_out = CHUNK;
        zStream_.next_out = (Bytef*)buffOut;
        if (deflate(&zStream_, flush) == Z_STREAM_ERROR) {
            DeleteZData();
            return DumpStatus::DUMP_FAIL;
        }
        unsigned have = CHUNK - zStream_.avail_out;
        if (memmove_s(zData_ + dstPos, zDataSize_ - dstPos, buffOut, have) != EOK) {
            DeleteZData();
            return DumpStatus::DUMP_FAIL;
        }
        dstPos += have;
    } while (zStream_.avail_out == 0);

    return DumpStatus::DUMP_OK;
}
} // namespace HiviewDFX
} // namespace OHOS
