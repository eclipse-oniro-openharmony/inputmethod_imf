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

#include "system_param_adapter.h"

#include "inputmethod_message_handler.h"
#include "parameter.h"
#include "parameters.h"
namespace OHOS {
namespace MiscServices {
const std::unordered_map<std::string, SystemParamAdapter::CbHandler> SystemParamAdapter::PARAM_CB_HANDLERS = {
    { SystemParamAdapter::SYSTEM_LANGUAGE_KEY, SystemParamAdapter::OnLanguageChange },
    { SystemParamAdapter::MEMORY_WATERMARK_KEY, SystemParamAdapter::OnMemoryChange },
};
SystemParamAdapter &SystemParamAdapter::GetInstance()
{
    static SystemParamAdapter adapter;
    return adapter;
}

int32_t SystemParamAdapter::WatchParam(const std::string &key)
{
    auto it = PARAM_CB_HANDLERS.find(key);
    if (it == PARAM_CB_HANDLERS.end() || it->second == nullptr) {
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    auto errNo = WatchParameter(key.c_str(), it->second, nullptr);
    IMSA_HILOGD("key/ret: %{public}s/%{public}d.", key.c_str(), errNo);
    return errNo;
}

bool SystemParamAdapter::GetBoolParam(const std::string &key)
{
    return system::GetBoolParameter(key, false);
}

void SystemParamAdapter::OnLanguageChange(const char *key, const char *value, void *context)
{
    HandleSysParamChanged(key, value, SYSTEM_LANGUAGE_KEY, MessageID::MSG_ID_SYS_LANGUAGE_CHANGED);
}

void SystemParamAdapter::OnMemoryChange(const char *key, const char *value, void *context)
{
    HandleSysParamChanged(key, value, MEMORY_WATERMARK_KEY, MessageID::MSG_ID_SYS_MEMORY_CHANGED);
}

void SystemParamAdapter::HandleSysParamChanged(
    const char *key, const char *value, const char *expectedKey, int32_t messageId)
{
    if (strcmp(key, expectedKey) != 0) {
        IMSA_HILOGE("key: %{public}s is error!", key);
        return;
    }
    IMSA_HILOGI("value: %{public}s.", value);
    Message *msg = new (std::nothrow) Message(messageId, nullptr);
    if (msg == nullptr) {
        return;
    }
    auto mesHandler = MessageHandler::Instance();
    if (mesHandler == nullptr) {
        delete msg;
        return;
    }
    mesHandler->SendMessage(msg);
}
} // namespace MiscServices
} // namespace OHOS