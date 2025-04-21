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
#include "data_id.h"
#include "hilog_wrapper.h"
#include "parameter.h"
#include "task/base/task_register.h"
#include "task/base/task_id.h"

namespace OHOS {
namespace HiviewDFX {

DumpStatus DeviceInfoTask::TaskEntry(DataInventory& dataInventory, const std::shared_ptr<DumperParameter>& parameter)
{
    DUMPER_HILOGI(MODULE_COMMON, "info|APIDumper Execute");
    AddApiRetIntoResult(GetDisplayVersion(), "BuildId");
    AddApiRetIntoResult(GetOsReleaseType(), "RleaseType");
    AddApiRetIntoResult(GetVersionId(), "OsVersion");
    AddApiRetIntoResult(GetDeviceType(), "DeviceType");
    AddApiRetIntoResult(GetManufacture(), "Manufacture");
    AddApiRetIntoResult(GetBrand(), "Brand");
    AddApiRetIntoResult(GetMarketName(), "MarketName");
    AddApiRetIntoResult(GetProductSeries(), "ProductSeries");
    AddApiRetIntoResult(GetProductModel(), "ProductModel");
    AddApiRetIntoResult(GetSoftwareModel(), "SoftwareModel");
    AddApiRetIntoResult(GetHardwareModel(), "HardwareModel");
    AddApiRetIntoResult(GetHardwareProfile(), "HardwareProfile");
    AddApiRetIntoResult(GetBootloaderVersion(), "BootLoaderVersion");
    AddApiRetIntoResult(GetAbiList(), "ABIList");
    AddApiRetIntoResult(GetSecurityPatchTag(), "SecurityPatch");
    AddApiRetIntoResult(GetIncrementalVersion(), "IncrementalVersion");
    AddApiRetIntoResult(GetOSFullName(), "OSFullName");
    AddApiRetIntoResult(GetSdkApiVersion(), "SDKAPIVersion");
    AddApiRetIntoResult(GetFirstApiVersion(), "FirstAPIVersion");
    AddApiRetIntoResult(GetBuildRootHash(), "BuildHash");
    AddApiRetIntoResult(GetBuildType(), "BuildType");
    AddApiRetIntoResult(GetBuildUser(), "BuildUser");
    AddApiRetIntoResult(GetBuildHost(), "BuildHost");
    AddApiRetIntoResult(GetBuildTime(), "BuildTime");

    dataInventory.Inject(DataId::DEVICE_INFO, std::make_shared<std::vector<std::string>>(result_));
    return DUMP_OK;
}

REGISTER_TASK(DUMP_DEVICE_INFO, DeviceInfoTask, true);
} // namespace HiviewDFX
} // namespace OHOS