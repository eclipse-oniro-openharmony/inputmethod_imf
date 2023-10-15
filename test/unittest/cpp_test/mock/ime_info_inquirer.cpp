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

#include "ime_info_inquirer.h"

namespace OHOS {
namespace MiscServices {
std::shared_ptr<ImeInfo> ImeInfoInquirer::defaultIme_ = std::make_shared<ImeInfo>();
std::shared_ptr<Property> ImeInfoInquirer::currentIme_ = std::make_shared<Property>();
ImeInfoInquirer &ImeInfoInquirer::GetInstance()
{
    static ImeInfoInquirer instance;
    return instance;
}

std::shared_ptr<ImeInfo> ImeInfoInquirer::GetDefaultImeInfo(int32_t userId)
{
    if (defaultIme_ != nullptr) {
        return defaultIme_;
    }
    defaultIme_ = std::make_shared<ImeInfo>();
    return defaultIme_;
}

std::shared_ptr<Property> ImeInfoInquirer::GetCurrentInputMethod(int32_t userId)
{
    if (currentIme_ != nullptr) {
        return currentIme_;
    }
    currentIme_ = std::make_shared<Property>();
    return currentIme_;
}
} // namespace MiscServices
} // namespace OHOS