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

#include "system_language_observer.h"

#include "global.h"
#include "parameter.h"
namespace OHOS {
namespace MiscServices {
constexpr int32_t HANDLE_OK = 0;
SystemLanguageObserver::ChangeHandler SystemLanguageObserver::handler_;
SystemLanguageObserver &SystemLanguageObserver::GetInstance()
{
    static SystemLanguageObserver observer;
    return observer;
}

void SystemLanguageObserver::Watch(ChangeHandler handler)
{
    IMSA_HILOGI("run in");
    handler_ = std::move(handler);
    auto errNo = WatchParameter(SYSTEM_LANGUAGE_KEY, OnChange, nullptr);
    if (errNo != HANDLE_OK) {
        IMSA_HILOGE("watch failed: %{public}d", errNo);
    }
}

void SystemLanguageObserver::OnChange(const char *key, const char *value, void *context)
{
    if (strncmp(key, SYSTEM_LANGUAGE_KEY, strlen(SYSTEM_LANGUAGE_KEY)) != 0) {
        IMSA_HILOGE("key: %{public}s is error", key);
        return;
    }
    IMSA_HILOGD("value: %{public}s", value);
    if (handler_ != nullptr) {
        handler_();
    }
}
} // namespace MiscServices
} // namespace OHOS