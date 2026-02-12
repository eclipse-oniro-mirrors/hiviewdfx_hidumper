/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#ifndef HIVIEWDFX_HIDUMPER_DATA_INVENTORY_H
#define HIVIEWDFX_HIDUMPER_DATA_INVENTORY_H

#include <memory>
#include <shared_mutex>
#include <set>
#include <unordered_map>
#include "hilog_wrapper.h"
#include "writer_utils.h"

namespace OHOS {
namespace HiviewDFX {

enum class DataId : uint32_t {
    DEVICE_INFO = 0,
    SYSTEM_CLUSTER_INFO,
    PROC_VERSION_INFO,
    PROC_CMDLINE_INFO,
    WAKEUP_SOURCES_INFO,
    UPTIME_INFO,
    CPU_FREQ_INFO,
    PROC_SLAB_INFO,
    PROC_ZONE_INFO,
    PROC_VMSTAT_INFO,
    PROC_VMALLOC_INFO,
    PROC_MODULES_INFO,
    PRINTENV_INFO,
    LSMOD_INFO,
    STORAGE_STATE_INFO,
    DF_INFO,
    LSOF_INFO,
    IOTOP_INFO,
    PROC_MOUNTS_INFO,
    PROC_PID_IO_INFO,
    SYSTEM_ABILITY_LIST,
    PROC_DEVHOST_SLAB_INFO,
    SYSTEM_ABILITY_INFO,
};

struct InfoConfig {
    std::string title;
    DataId dataId;
};

struct CpuFreqInfo {
    int cpuId = 0;
    std::string curFreq;
    std::string maxFreq;
};

struct BaseType {
    virtual ~BaseType() = default;
};
using BaseTypePtr = std::shared_ptr<BaseType>;

template <typename T>
struct CustomType : public BaseType {
    std::shared_ptr<T> data;
};

class DataInventory {
public:
    template <typename T>
    bool Inject(DataId dataId, std::shared_ptr<T> ptr)
    {
        if (ptr == nullptr) {
            return false;
        }
        if (GetPtr<T>(dataId) != nullptr) {
            return false;
        }

        std::shared_ptr<CustomType<T>> container = std::make_shared<CustomType<T>>();
        container->data = ptr;
        return InputToData(dataId, container);
    }

    template <typename T>
    std::shared_ptr<T> GetPtr(DataId dataId) const
    {
        return Cast<T>(GetPtr(dataId));
    }

    using DataFilterHandler = std::function<void(std::string& line)>;
    bool InjectString(const std::string& source, DataId dataId, bool isFile);
    bool InjectStringWithFilter(const std::string& source, DataId dataId, bool isFile, const DataFilterHandler& func);

    std::set<DataId> RemoveRestData(const std::set<DataId>& keepingDataType);
    std::size_t Size() const;

    DataInventory() = default;
    ~DataInventory() = default;
private:
    template<typename T>
    std::shared_ptr<T> Cast(const BaseTypePtr& ptr) const
    {
        if (ptr == nullptr) {
            return {};
        }
        auto customPtr = std::static_pointer_cast<CustomType<T>>(ptr);
        return customPtr->data;
    }
    
    bool InputToData(DataId dataId, BaseTypePtr ptr);
    BaseTypePtr GetPtr(DataId dataId) const;

private:
    std::unordered_map<DataId, BaseTypePtr> data_;
    mutable std::shared_mutex rwMutex_;
};
    
} // namespace HiviewDFX
} // namespace OHOS

#endif
