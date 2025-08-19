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
#ifndef FRAMEWORKS_INPUTMETHOD_KEY_EVENT_RESULT_HANDLER_H
#define FRAMEWORKS_INPUTMETHOD_KEY_EVENT_RESULT_HANDLER_H
#include <map>
#include <mutex>

#include "key_event.h"
namespace OHOS {
namespace MiscServices {
using KeyEventCallback = std::function<void(std::shared_ptr<MMI::KeyEvent> &keyEvent, bool isConsumed)>;
struct KeyEventCbInfo {
    std::shared_ptr<MMI::KeyEvent> keyEvent{ nullptr };
    KeyEventCallback callback{ nullptr };
};
class KeyEventResultHandler {
public:
    uint64_t AddKeyEventCbInfo(const KeyEventCbInfo &cbInfo);
    void RemoveKeyEventCbInfo(uint64_t cbId);
    void HandleKeyEventResult(uint64_t cbId, bool consumeResult);
    void ClearKeyEventCbInfo();

private:
    int32_t GetKeyEventCbInfo(uint64_t cbId, KeyEventCbInfo &info);
    uint64_t GenerateKeyEventCbId();
    std::mutex keyEventCbHandlersMutex_;
    std::map<uint64_t, KeyEventCbInfo> keyEventCbHandlers_;
    std::atomic<uint64_t> maxCbId_{1};
};
} // namespace MiscServices
} // namespace OHOS
#endif // FRAMEWORKS_INPUTMETHOD_KEY_EVENT_RESULT_HANDLER_H