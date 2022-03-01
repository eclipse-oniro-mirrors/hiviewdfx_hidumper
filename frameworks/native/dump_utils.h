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
#ifndef DUMP_UTILS_H
#define DUMP_UTILS_H
#include <string>
#include <vector>
#include <unistd.h>
namespace OHOS {
namespace HiviewDFX {
#ifndef TEMP_FAILURE_RETRY
#define TEMP_FAILURE_RETRY
#endif
#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif
class DumpUtils {
public:
    // system operations
    static void IgnoreOom();
    static void IgnorePipeQuit();
    static void IgnoreStdoutCache();
    static void SetAdj(int adj);
    static void BoostPriority();

    // string operations
    static bool String2Uint32(const std::string &str, uint32_t &val);
    static void RemoveDuplicateString(std::vector<std::string> &vector);

    // file operations
    static bool FileWriteable(const std::string &file);
    static int FdToRead(const std::string &file);
    static int FdToWrite(const std::string &file);
    
    // process operations
    static bool WaitPid(pid_t pid, uint32_t timeoutMs, int *status);
    static bool CheckProcessAlive(uint32_t pid);
    static void LiveWithParent();
    static bool StrToCmdBuf(const std::string &cmd, std::vector<const char *> &argVtr,
                            std::vector<std::string> &cmdVtr);
    
    // ability string to ID
    static int StrToId(const std::string &name); // 0:invalid

    static constexpr int TOP_PRIORITY = -20;
    static constexpr int TOP_OOM_ADJ = -1000;
    static constexpr char FILE_CUR_OOM_ADJ[] = "/proc/self/oom_score_adj";
    static constexpr auto& SPACE = " ";
    static const int INVALID_PID = -1;
private:
    static std::string ErrnoToMsg(const int &error);
};

} // namespace HiviewDFX
} // namespace OHOS
#endif
