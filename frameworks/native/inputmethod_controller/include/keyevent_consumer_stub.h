/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#ifndef FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_KEY_EVENT_PROXY_CONSUMER_STUB_H
#define FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_KEY_EVENT_PROXY_CONSUMER_STUB_H

#include <cstdint>
#include <string>

#include "i_keyevent_consumer.h"
#include "iremote_stub.h"
#include "key_event.h"
#include "message_handler.h"
#include "message_option.h"
#include "message_parcel.h"
#include "nocopyable.h"
#include "refbase.h"

namespace OHOS {
namespace MiscServices {
class KeyEventConsumerStub : public IRemoteStub<IKeyEventConsumer> {
public:
    DISALLOW_COPY_AND_MOVE(KeyEventConsumerStub);
    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
    using KeyEventCallback = std::function<void(std::shared_ptr<MMI::KeyEvent> &keyEvent, bool isConsumed)>;
    KeyEventConsumerStub(KeyEventCallback callback, std::shared_ptr<MMI::KeyEvent> keyEvent);
    ~KeyEventConsumerStub();
    int32_t OnKeyEventResult(bool isConsumed) override;

private:
    int32_t OnKeyEventResultOnRemote(MessageParcel &data, MessageParcel &reply);
    using RequestHandler = int32_t (KeyEventConsumerStub::*)(MessageParcel &, MessageParcel &);
    static constexpr RequestHandler HANDLERS[KEY_EVENT_CONSUMER_CMD_END] = {
        [KEY_EVENT_RESULT] = &KeyEventConsumerStub::OnKeyEventResultOnRemote,
    };
    KeyEventCallback callback_;
    std::shared_ptr<MMI::KeyEvent> keyEvent_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_KEY_EVENT_PROXY_CONSUMER_STUB_H
