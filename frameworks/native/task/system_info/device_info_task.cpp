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


#include "task/system_info/device_info_task.h"

#include "data_inventory.h"
#include "hilog_wrapper.h"
#include "parameter.h"
#include "task/base/task_register.h"
#include "writer_utils.h"

namespace OHOS {
namespace HiviewDFX {

DumpStatus DeviceInfoTask::TaskEntry(DataInventory& dataInventory, const DumpContext& dumpContext)
{
    bool ret = false;
    ret = GetDeviceInfoByParam(dataInventory);
    dataInventory.InjectString("/proc/version", DataId::PROC_VERSION_INFO, true);
    dataInventory.InjectString("/proc/cmdline", DataId::PROC_CMDLINE_INFO, true);
    dataInventory.InjectString("uptime -p", DataId::UPTIME_INFO, false);
    if (!ret) {
        return DUMP_FAIL;
    }
    return DUMP_OK;
}

bool DeviceInfoTask::GetDeviceInfoByParam(DataInventory& dataInventory)
{
    std::vector<std::string> deviceInfo = {};
    AddApiRetIntoResult(GetDisplayVersion(), "BuildId", deviceInfo);
    AddApiRetIntoResult(GetOsReleaseType(), "RleaseType", deviceInfo);
    AddApiRetIntoResult(GetVersionId(), "OsVersion", deviceInfo);
    AddApiRetIntoResult(GetDeviceType(), "DeviceType", deviceInfo);
    AddApiRetIntoResult(GetManufacture(), "Manufacture", deviceInfo);
    AddApiRetIntoResult(GetBrand(), "Brand", deviceInfo);
    AddApiRetIntoResult(GetMarketName(), "MarketName", deviceInfo);
    AddApiRetIntoResult(GetProductSeries(), "ProductSeries", deviceInfo);
    AddApiRetIntoResult(GetProductModel(), "ProductModel", deviceInfo);
    AddApiRetIntoResult(GetSoftwareModel(), "SoftwareModel", deviceInfo);
    AddApiRetIntoResult(GetHardwareModel(), "HardwareModel", deviceInfo);
    AddApiRetIntoResult(GetHardwareProfile(), "HardwareProfile", deviceInfo);
    AddApiRetIntoResult(GetBootloaderVersion(), "BootLoaderVersion", deviceInfo);
    AddApiRetIntoResult(GetAbiList(), "ABIList", deviceInfo);
    AddApiRetIntoResult(GetSecurityPatchTag(), "SecurityPatch", deviceInfo);
    AddApiRetIntoResult(GetIncrementalVersion(), "IncrementalVersion", deviceInfo);
    AddApiRetIntoResult(GetOSFullName(), "OSFullName", deviceInfo);
    AddApiRetIntoResult(GetSdkApiVersion(), "SDKAPIVersion", deviceInfo);
    AddApiRetIntoResult(GetFirstApiVersion(), "FirstAPIVersion", deviceInfo);
    AddApiRetIntoResult(GetBuildRootHash(), "BuildHash", deviceInfo);
    AddApiRetIntoResult(GetBuildType(), "BuildType", deviceInfo);
    AddApiRetIntoResult(GetBuildUser(), "BuildUser", deviceInfo);
    AddApiRetIntoResult(GetBuildHost(), "BuildHost", deviceInfo);
    AddApiRetIntoResult(GetBuildTime(), "BuildTime", deviceInfo);
    dataInventory.Inject(DataId::DEVICE_INFO, std::make_shared<std::vector<std::string>>(deviceInfo));
    return true;
}

REGISTER_TASK(TaskId::DUMP_DEVICE_INFO, DeviceInfoTask, false);
} // namespace HiviewDFX
} // namespace OHOS