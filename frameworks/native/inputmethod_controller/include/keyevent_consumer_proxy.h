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

#ifndef FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_KEY_EVENT_PROXY_CONSUMER_PROXY_H
#define FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_KEY_EVENT_PROXY_CONSUMER_PROXY_H

#include <cstdint>
#include <functional>
#include <string>

#include "i_keyevent_consumer.h"
#include "iremote_broker.h"
#include "iremote_object.h"
#include "iremote_proxy.h"
#include "key_event.h"
#include "nocopyable.h"
#include "refbase.h"

namespace OHOS {
namespace MiscServices {

class KeyEventConsumerProxy : public IRemoteProxy<IKeyEventConsumer> {
public:
    DISALLOW_COPY_AND_MOVE(KeyEventConsumerProxy);
    explicit KeyEventConsumerProxy(const sptr<IRemoteObject> &object);
    ~KeyEventConsumerProxy() = default;
    int32_t OnKeyEventResult(bool isConsumed) override;
    void OnKeyEventConsumeResult(bool isConsumed);
    void OnKeyCodeConsumeResult(bool isConsumed);

private:
    static inline BrokerDelegator<KeyEventConsumerProxy> delegator_;
    using ParcelHandler = std::function<bool(MessageParcel &)>;
    int32_t SendRequest(int code, ParcelHandler input = nullptr, ParcelHandler output = nullptr,
        MessageOption option = MessageOption::TF_SYNC);
    bool keyEventConsume_ = false;
    bool keyCodeConsume_ = false;
    bool keyEventResult_ = false;
    bool keyCodeResult_ = false;
    std::shared_ptr<MMI::KeyEvent> keyEvent_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_KEY_EVENT_PROXY_CONSUMER_PROXY_H
