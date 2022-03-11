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
#ifndef HIDUMPER_COMMON_H
#define HIDUMPER_COMMON_H
#include <ctime>
#include <sys/time.h>
namespace OHOS {
namespace HiviewDFX {
enum DumpStatus {
    DUMP_REQUEST_MAX = -5,
    DUMP_TIMEOUT = -4,
    DUMP_INVALID_ARG = -3,
    DUMP_NOPERMISSION = -2,
    DUMP_FAIL = -1,
    DUMP_OK = 0,
    DUMP_HELP,
    DUMP_CHECK_OK,
    DUMP_MORE_DATA,
};

#define LOG_TIME() \
    do { \
        struct timeval tp; \
        gettimeofday(&tp, nullptr); \
        struct tm curTime = {0}; \
        if (localtime_r(&tp.tv_sec, &curTime) != nullptr) { \
            fprintf(stdout, "[%04d%02d%02d %02d:%02d:%02d]", curTime.tm_year + 1900, 1 + curTime.tm_mon, \
                    curTime.tm_mday, curTime.tm_hour, curTime.tm_min, curTime.tm_sec); \
        } \
    } while (0)

#define LOG_ERR(format, ...) \
    do { \
        LOG_TIME(); \
        fprintf(stdout, "Error:" format "", ##__VA_ARGS__); \
    } while (0)

#define HIDUMPER_DEBUG
#ifdef HIDUMPER_DEBUG
#define LOG_DEBUG(format, ...) \
    do { \
        LOG_TIME(); \
        fprintf(stdout, "Debug:" format "(%s %d)\n", ##__VA_ARGS__, __FILE__, __LINE__); \
    } while (0)
#else
#define LOG_DEBUG(format, ...)
#endif
} // namespace HiviewDFX
} // namespace OHOS
#endif
