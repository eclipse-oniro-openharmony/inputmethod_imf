/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef SERVICES_INCLUDE_IME_INFO_ENQUIRER_H
#define SERVICES_INCLUDE_IME_INFO_ENQUIRER_H

#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "input_method_property.h"
#include "input_method_status.h"

namespace OHOS {
namespace MiscServices {
class ImeInfoInquirer {
public:
    static ImeInfoInquirer &GetInstance();
    std::shared_ptr<ImeInfo> GetDefaultImeInfo(int32_t userId);
    std::shared_ptr<Property> GetCurrentInputMethod(int32_t userId);
    std::shared_ptr<Property> GetDefaultImeCfgProp();
    static bool GetImeAppId(int32_t userId, const std::string &bundleName, std::string &appId);
    static bool GetImeVersionCode(int32_t userId, const std::string &bundleName, uint32_t &versionCode);

private:
    static std::shared_ptr<ImeInfo> defaultIme_;
    static std::shared_ptr<Property> currentIme_;
    static std::shared_ptr<Property> defaultImeProperty_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // SERVICES_INCLUDE_IME_INFO_ENQUIRER_H
