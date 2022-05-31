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
#ifndef HIDUMPER_DUMP_COMPRESSOR_H
#define HIDUMPER_DUMP_COMPRESSOR_H

#include <iostream>
#include <zlib.h>
#include "common.h"


const uint32_t MAX_COMPRESS_BUFFER_SIZE = 32 * 1024;

namespace OHOS {
namespace HiviewDFX {
struct CompressBuffer {
    uint32_t offset;
    char content[MAX_COMPRESS_BUFFER_SIZE] = { 0 };
};

const uint16_t CHUNK = 16384;
const uint16_t WINDOWS_BITS = 16;
const uint16_t MEM_LEVEL = 8;

class DumpCompressor {
public:
    DumpCompressor();
    ~DumpCompressor() = default;

    /**
     * Compress content in source buffer to dest buffer using zlib.
     *
     * @param srcBuffer, source buffer.
     * @param destBuffer, dest buffer.
     * @return DUMP_OK on sucess or DUMP_FAIL on any errors.
     */
    DumpStatus Compress(CompressBuffer*& srcBuffer, CompressBuffer*& destBuffer);

private:
    DumpStatus FillBuffer(int& flush, CompressBuffer*& srcBuffer, char*& buffIn,
        size_t const toRead, size_t& src_pos, size_t srcBufferOffset);
    DumpStatus DeflateBuffer(int flush, char*& buffOut, size_t& dst_pos);
    void DeleteZData();

private:
    z_stream zStream_ = {0};
    unsigned char* zData_ = nullptr;
    uint32_t zDataSize_ = 0;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIDUMPER_COMPRESS_H
