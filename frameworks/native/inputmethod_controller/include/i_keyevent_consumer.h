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

#ifndef FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_I_KEY_EVENT_CONSUMER_H
#define FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_I_KEY_EVENT_CONSUMER_H
#include <errors.h>

#include "global.h"
#include "input_method_utils.h"
#include "iremote_broker.h"

/**
 * brief Definition of interface IKeyEventConsumer
 * It defines the remote calls from input method service to input client
 */
namespace OHOS {
namespace MiscServices {
class IKeyEventConsumer : public IRemoteBroker {
public:
    enum {
        KEY_EVENT_CONSUMER_BEGIN,
        KEY_EVENT_RESULT = KEY_EVENT_CONSUMER_BEGIN,
        KEY_EVENT_CONSUMER_CMD_END
    };
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.miscservices.inputmethod.IKeyEventConsumer");
    virtual int32_t OnKeyEventResult(bool isConsumed) = 0;
};
} // namespace MiscServices
} // namespace OHOS
#endif // FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_I_KEY_EVENT_CONSUMER_H
