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
#include "message.h"
#include "message_handler.h"
#include "parameter.h"
namespace OHOS {
namespace MiscServices {
SystemLanguageObserver &SystemLanguageObserver::GetInstance()
{
    static SystemLanguageObserver observer;
    return observer;
}

void SystemLanguageObserver::Watch()
{
    auto errNo = WatchParameter(SYSTEM_LANGUAGE_KEY, OnChange, nullptr);
    IMSA_HILOGD("ret: %{public}d", errNo);
}

void SystemLanguageObserver::OnChange(const char *key, const char *value, void *context)
{
    if (strncmp(key, SYSTEM_LANGUAGE_KEY, strlen(SYSTEM_LANGUAGE_KEY)) != 0) {
        IMSA_HILOGE("key: %{public}s is error", key);
        return;
    }
    IMSA_HILOGD("value: %{public}s.", value);
    Message *msg = new (std::nothrow) Message(MessageID::MSG_ID_SYS_LANGUAGE_CHANGED, nullptr);
    if (msg == nullptr) {
        return;
    }
    MessageHandler::Instance()->SendMessage(msg);
}
} // namespace MiscServices
} // namespace OHOS