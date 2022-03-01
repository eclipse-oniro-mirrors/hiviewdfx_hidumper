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

#include "executor/api_dumper.h"
#include <sstream>
#include "parameter.h"

namespace OHOS {
namespace HiviewDFX {
APIDumper::APIDumper()
{
}

APIDumper::~APIDumper()
{
}

DumpStatus APIDumper::PreExecute(const std::shared_ptr<DumperParameter>& parameter,
    StringMatrix dumpDatas)
{
    if (dumpDatas.get()) {
        result_ = dumpDatas;
        return DumpStatus::DUMP_OK;
    }
    return DumpStatus::DUMP_FAIL;
}

DumpStatus APIDumper::Execute()
{
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
    return DumpStatus::DUMP_OK;
}

void APIDumper::AddApiRetIntoResult(const char* content, const std::string& title)
{
    if (content) {
        std::ostringstream s;
        s << title <<": "<< content;
        std::vector<std::string> line_vector;
        line_vector.push_back(s.str());
        result_->push_back(line_vector);
    }
}

void APIDumper::AddApiRetIntoResult(int content, const std::string& title)
{
    std::ostringstream s;
    s << title <<": "<< content;
    std::vector<std::string> line_vector;
    line_vector.push_back(s.str());
    result_->push_back(line_vector);
}

DumpStatus APIDumper::AfterExecute()
{
    return DumpStatus::DUMP_OK;
}
} // namespace HiviewDFX
} // namespace OHOS
