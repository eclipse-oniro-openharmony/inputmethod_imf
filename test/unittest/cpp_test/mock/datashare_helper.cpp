/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#include "datashare_helper.h"

namespace OHOS {
namespace DataShare {
std::shared_ptr<DataShareHelper> DataShareHelper::instance_;
DataSharePredicates *DataSharePredicates::EqualTo(const std::string &field, const std::string &value)
{
    return this;
}

int32_t DataShareResultSet::GetRowCount(int32_t &count)
{
    count = 1;
    return 0;
}

int32_t DataShareResultSet::Close()
{
    return 0;
}

int32_t DataShareResultSet::GoToFirstRow()
{
    return 0;
}

int32_t DataShareResultSet::GetColumnIndex(const std::string &columnName, int32_t &columnIndex)
{
    columnIndex = 1;
    return 0;
}

int32_t DataShareResultSet::GetString(int columnIndex, std::string &value)
{
    value = strValue_;
    return 0;
}

std::shared_ptr<DataShareHelper> DataShareHelper::Creator(
    const sptr<IRemoteObject> &token, const std::string &strUri, const std::string &extUri)
{
    if (instance_ != nullptr) {
        return instance_;
    }
    instance_ = std::make_shared<DataShareHelper>();
    return instance_;
}

std::shared_ptr<DataShareResultSet> DataShareHelper::Query(Uri &uri, const DataSharePredicates &predicates,
    std::vector<std::string> &columns, DatashareBusinessError *businessError)
{
    if (resultSet_ != nullptr) {
        return resultSet_;
    }
    resultSet_ = std::make_shared<DataShareResultSet>();
    return resultSet_;
}

void DataShareHelper::RegisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
{
    return;
}

void DataShareHelper::UnregisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
{
    return;
}

bool DataShareHelper::Release()
{
    return true;
}
} // namespace DataShare
} // namespace OHOS
