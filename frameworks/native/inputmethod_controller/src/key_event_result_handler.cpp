/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "key_event_result_handler.h"

#include <cinttypes>

#include "global.h"
namespace OHOS {
namespace MiscServices {
uint64_t KeyEventResultHandler::AddKeyEventCbInfo(const KeyEventCbInfo &cbInfo)
{
    std::lock_guard<std::mutex> lock(keyEventCbHandlersMutex_);
    auto cbId = GenerateKeyEventCbId();
    IMSA_HILOGD("%{public}" PRIu64 "add.", cbId);
    keyEventCbHandlers_.insert_or_assign(cbId, cbInfo);
    return cbId;
}

void KeyEventResultHandler::HandleKeyEventResult(uint64_t cbId, bool consumeResult)
{
    IMSA_HILOGD("result:%{public}" PRIu64 "/%{public}d.", cbId, consumeResult);
    KeyEventCbInfo info;
    auto ret = GetKeyEventCbInfo(cbId, info);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("%{public}" PRIu64 "not be found.", cbId);
        return;
    }
    if (info.callback == nullptr) {
        IMSA_HILOGE("%{public}" PRIu64 "callback is nullptr.", cbId);
    } else {
        info.callback(info.keyEvent, consumeResult);
    }
    RemoveKeyEventCbInfo(cbId);
}

void KeyEventResultHandler::ClearKeyEventCbInfo()
{
    std::lock_guard<std::mutex> lock(keyEventCbHandlersMutex_);
    keyEventCbHandlers_.clear();
}

void KeyEventResultHandler::RemoveKeyEventCbInfo(uint64_t cbId)
{
    std::lock_guard<std::mutex> lock(keyEventCbHandlersMutex_);
    auto iter = keyEventCbHandlers_.find(cbId);
    if (iter == keyEventCbHandlers_.end()) {
        return;
    }
    keyEventCbHandlers_.erase(cbId);
}

int32_t KeyEventResultHandler::GetKeyEventCbInfo(uint64_t cbId, KeyEventCbInfo &info)
{
    std::lock_guard<std::mutex> lock(keyEventCbHandlersMutex_);
    auto iter = keyEventCbHandlers_.find(cbId);
    if (iter != keyEventCbHandlers_.end()) {
        info = iter->second;
        return ErrorCode::NO_ERROR;
    }
    return ErrorCode::ERROR_BAD_PARAMETERS;
}

uint64_t KeyEventResultHandler::GenerateKeyEventCbId()
{
    return maxCbId_.fetch_add(1);
}
} // namespace MiscServices
} // namespace OHOS