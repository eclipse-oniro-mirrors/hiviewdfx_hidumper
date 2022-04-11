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
#include "executor/list_dumper.h"

#include "dump_utils.h"
#include "manager/dump_implement.h"
#include "util/config_utils.h"
namespace OHOS {
namespace HiviewDFX {
const std::string ListDumper::ABILITY_HEADER = "System ability list:";
const std::string ListDumper::SYSTEM_HEADER = "System cluster list:";
const int ListDumper::ITEM_SUM_LINE = 3;
const std::string ListDumper::NAME_SPACE = "                                 ";
ListDumper::ListDumper()
{
}

ListDumper::~ListDumper()
{
}

DumpStatus ListDumper::PreExecute(const std::shared_ptr<DumperParameter>& parameter,
    StringMatrix dumpDatas)
{
    target_ = ptrDumpCfg_->target_;
    if (dumpDatas.get()) {
        result_ = dumpDatas;
        return DumpStatus::DUMP_OK;
    }
    return DumpStatus::DUMP_FAIL;
}

DumpStatus ListDumper::Execute()
{
    std::string header;
    std::vector<std::string> list;
    if (target_ == ConfigUtils::STR_ABILITY) {
        header = ABILITY_HEADER;
        auto sma = DumpImplement::GetInstance().GetSystemAbilityManager();
        if (sma == nullptr) {
            return DumpStatus::DUMP_OK;
        }
        std::vector<std::u16string> abilities = sma->ListSystemAbilities();
        std::transform(abilities.begin(), abilities.end(), std::back_inserter(list), Str16ToStr8);
        std::transform(list.begin(), list.end(), list.begin(), DumpUtils::ConvertSaIdToSaName);
    } else if (target_ == ConfigUtils::STR_SYSTEM) {
        header = SYSTEM_HEADER;
        ConfigUtils::GetSectionNames(ConfigUtils::CONFIG_GROUP_SYSTEM_, list);
    }
    if (!header.empty()) {
        std::vector<std::string> line_vector;
        line_vector.push_back(header);
        result_->push_back(line_vector);
        line_vector.clear();
        int sum = 0;
        for (size_t i = 0; i < list.size(); i++) {
            std::string showname = list[i] + AppendBlank(list[i]);
            line_vector.push_back(showname);
            sum = sum + 1;
            if (sum < ITEM_SUM_LINE) {
                continue;
            }
            result_->push_back(line_vector);
            line_vector.clear();
            sum = 0;
        }
        if (!line_vector.empty()) {
            result_->push_back(line_vector);
            line_vector.clear();
        }
        line_vector.clear();
        result_->push_back(line_vector);
    }
    return DumpStatus::DUMP_OK;
}

DumpStatus ListDumper::AfterExecute()
{
    DUMPER_HILOGD(MODULE_COMMON, "enter|");
    for (size_t lineIndex = 0; lineIndex < result_->size(); lineIndex++) {
        std::vector<std::string> line = result_->at(lineIndex);
        for (size_t itemIndex = 0; itemIndex < line.size(); itemIndex++) {
            std::string item = line[itemIndex];
            DUMPER_HILOGD(MODULE_COMMON, "debug|item[%{public}zu, %{public}zu]=[%{public}s]",
                lineIndex, itemIndex, item.c_str());
        }
    }
    DUMPER_HILOGD(MODULE_COMMON, "leave|");
    return DumpStatus::DUMP_OK;
}

const std::string ListDumper::AppendBlank(const std::string& str)
{
    std::string spaces;
    if (str.length() < NAME_SPACE.length()) {
        spaces = NAME_SPACE.substr(str.length());
    }
    return spaces;
}
} // namespace HiviewDFX
} // namespace OHOS
