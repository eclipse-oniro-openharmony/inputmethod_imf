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

#ifndef IMF_SYSTEM_LANGUAGE_OBSERVER_H
#define IMF_SYSTEM_LANGUAGE_OBSERVER_H

#include <functional>
#include <utility>

namespace OHOS {
namespace MiscServices {
class SystemLanguageObserver {
public:
    static SystemLanguageObserver &GetInstance();
    void Watch();

private:
    static constexpr const char *SYSTEM_LANGUAGE_KEY = "persist.global.language";
    SystemLanguageObserver() = default;
    static void OnChange(const char *key, const char *value, void *context);
};
} // namespace MiscServices
} // namespace OHOS

#endif // IMF_SYSTEM_LANGUAGE_OBSERVER_H
