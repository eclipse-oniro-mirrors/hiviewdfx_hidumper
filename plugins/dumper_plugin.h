/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef DUMPER_PLUGIN_H
#define DUMPER_PLUGIN_H

#include <dlfcn.h>
#include <cstdio>
#include <cstdlib>

namespace OHOS {
namespace HiviewDFX {
#if (defined(__aarch64__) || defined(__x86_64__))
const char PLUGIN_SO_PATH[] = "/system/lib64/libhidumperplugin_impl.z.so";
#else
const char PLUGIN_SO_PATH[] = "/system/lib/libhidumperplugin_impl.z.so";
#endif

// 在插件里，创建插件对象
const char CREATE_PLUGIN[] = "OnCreatePlugin";

const int PLUGIN_ENABLE = 0;
const int PLUGIN_DISABLE = -1;

struct DumpMeminfo {
    int pid = 0;
    int size = 0;
};

struct Plugin {
    uint32_t version;
    const char *pluginName;
    void (*queryMemInfo)(DumpMeminfo[], int&);
};

int32_t LoadPlugin(void);
void UnloadPlugin(void);
bool QueryMemInfo(DumpMeminfo infos[], int& realSize);

} // namespace HiviewDFX
} // namespace OHOS
#endif // DUMPER_PLUGIN_H
