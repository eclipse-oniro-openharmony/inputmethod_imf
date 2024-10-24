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

#ifndef DATASHARE_HELPER_H
#define DATASHARE_HELPER_H

#include <cstdint>
#include <string>

#include "data_ability_observer_interface.h"
#include "datashare_value_object.h"
#include "datashare_values_bucket.h"
#include "uri.h"

namespace OHOS {
namespace DataShare {
constexpr int32_t E_OK = 0;
class DataSharePredicates {
public:
    DataSharePredicates *EqualTo(const std::string &field, const std::string &value);
};
class DatashareBusinessError {};
class DataShareResultSet {
public:
    DataShareResultSet() = default;
    ~DataShareResultSet() = default;
    static int32_t GetRowCount(int32_t &count);
    static int32_t Close();
    static int32_t GoToFirstRow();
    static int32_t GetColumnIndex(const std::string &columnName, int32_t &columnIndex);
    int32_t GetString(int columnIndex, std::string &value) const;

private:
    std::string strValue_;
};

class DataShareHelper {
public:
    DataShareHelper() = default;
    ~DataShareHelper() = default;
    static std::shared_ptr<DataShareHelper> Creator(
        const sptr<IRemoteObject> &token, const std::string &strUri, const std::string &extUri);
    std::shared_ptr<DataShareResultSet> Query(Uri &uri, const DataSharePredicates &predicates,
        std::vector<std::string> &columns, DatashareBusinessError *businessError = nullptr);
    static void RegisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver);
    static void UnregisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver);
    static bool Release();
    static int Update(Uri &uri, const DataSharePredicates &predicates, const DataShareValuesBucket &value);
    static int Insert(Uri &uri, const DataShareValuesBucket &value);

private:
    static std::shared_ptr<DataShareHelper> instance_;
    std::shared_ptr<DataShareResultSet> resultSet_;
};
} // namespace DataShare
} // namespace OHOS
#endif // DATASHARE_HELPER_H
