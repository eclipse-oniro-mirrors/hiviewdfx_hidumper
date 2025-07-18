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
#include <mutex>
#include <set>
#include <unordered_map>
#include "hilog_wrapper.h"
#include "writer_utils.h"

namespace OHOS {
namespace HiviewDFX {


enum DataId : uint32_t {
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
};

struct InfoConfig {
    std::string title;
    DataId dataId;
};

struct BaseType {
    virtual ~BaseType() = default;
};
using BaseTypePtr = std::shared_ptr<BaseType>;

template <typename T>
struct CustomType : public BaseType {
    std::shared_ptr<T> data;
};

template <typename T>
struct SharedPtrType {
    using Type = std::shared_ptr<T>;
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
    bool LoadAndInject(const std::string& source, DataId dataId, bool isFile)
    {
        std::vector<std::string> result = {};
        auto loader = isFile ? LoadStringFromFile : LoadStringFromCommand;
        if (!loader(source, [&result](const std::string& line) {
            result.emplace_back(line);
            return true;
        })) {
            return false;
        }
        return Inject(dataId, std::make_shared<std::vector<std::string>>(result));
        ;
    }

    bool LoadAndInjectWithFilter(const std::string& source, DataId dataId, bool isFile, const DataFilterHandler& func)
    {
        std::vector<std::string> result = {};
        auto loader = isFile ? LoadStringFromFile : LoadStringFromCommand;
        if (!loader(source, [&result, &func](const std::string& line) {
            std::string formatLine = line;
            func(formatLine);
            result.emplace_back(formatLine);
            return true;
        })) {
            return false;
        }
        return Inject(dataId, std::make_shared<std::vector<std::string>>(result));
    }

    std::set<DataId> RemoveRestData(const std::set<DataId>& keepingDataType);

    std::size_t Size()
    {
        return data_.size();
    }

    DataInventory() = default;
    ~DataInventory() = default;
    DataInventory(const DataInventory&) = delete;
    DataInventory& operator=(const DataInventory&) = delete;
    DataInventory(DataInventory&& dataInventory) : data_(std::move(dataInventory.data_)) {}
    DataInventory& operator=(DataInventory&& dataInventory)
    {
        if (&dataInventory == this) {
            return *this;
        }
        data_.swap(dataInventory.data_);
        dataInventory.data_.clear();
        return *this;
    }
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
    mutable std::mutex mutex_;
};
    
} // namespace name
}

#endif
