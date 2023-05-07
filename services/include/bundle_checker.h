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

#ifndef SERVICES_INCLUDE_BUNDLE_CHECKER_H
#define SERVICES_INCLUDE_BUNDLE_CHECKER_H

#include "accesstoken_kit.h"
#include "event_status_manager.h"

namespace OHOS {
namespace MiscServices {
class BundleChecker {
public:
    static bool IsFocused(uint32_t tokenID);
    static bool IsSystemApp(uint64_t fullTokenID);
    static bool IsCurrentIme(uint32_t tokenID, const std::string &currentIme);
    static bool CheckPermission(uint32_t tokenID, const std::string &permission);
    static bool IsEventListenPermissionCheckSuccess(EventStatus status, uint64_t fullTokenID);

private:
    static std::string GetBundleNameByToken(uint32_t tokenID);
};
} // namespace MiscServices
} // namespace OHOS

#endif // SERVICES_INCLUDE_BUNDLE_CHECKER_H
