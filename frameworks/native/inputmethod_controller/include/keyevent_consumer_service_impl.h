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

#ifndef FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_KEYEVENT_CONSUMER_SERVICE_IMPL_H
#define FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_KEYEVENT_CONSUMER_SERVICE_IMPL_H

#include "key_event_consumer_stub.h"
#include "key_event.h"
#include "iremote_object.h"

namespace OHOS {
namespace MiscServices {
class KeyEventConsumerServiceImpl final : public KeyEventConsumerStub,
    public std::enable_shared_from_this<KeyEventConsumerServiceImpl> {
    DISALLOW_COPY_AND_MOVE(KeyEventConsumerServiceImpl);

public:
    using KeyEventCallback = std::function<void(std::shared_ptr<MMI::KeyEvent> &keyEvent, bool isConsumed)>;
    KeyEventConsumerServiceImpl(KeyEventCallback callback, std::shared_ptr<MMI::KeyEvent> keyEvent);
    ~KeyEventConsumerServiceImpl();
    ErrCode OnKeyEventResult(bool isConsumed) override;

private:
    KeyEventCallback callback_;
    std::shared_ptr<MMI::KeyEvent> keyEvent_;
};
}  // namespace MiscServices
}  // namespace OHOS
#endif // FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_INPUT_CLIENT_IMPL_H