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
#include <csignal>
#include <unistd.h>
#include <thread>
#include "dump_client_main.h"
#include "hilog_wrapper.h"
using OHOS::HiviewDFX::DumpClientMain;

static void SigHandler(int sig)
{
    if (sig == SIGINT) {
        std::thread([&]() mutable {
            char *argv[] = {
                const_cast<char *>("hidumper"),
                const_cast<char *>("--mem"),
                const_cast<char *>("SIGINT"),
            };
            int argc = sizeof(argv) / sizeof(argv[0]);
            int ret = DumpClientMain::GetInstance().Main(argc, argv, STDOUT_FILENO);
            _exit(ret);
        }).detach();
    }
}

int main(int argc, char *argv[])
{
    std::signal(SIGINT, SigHandler);
    int ret = DumpClientMain::GetInstance().Main(argc, argv, STDOUT_FILENO);
    _exit(ret);
}
