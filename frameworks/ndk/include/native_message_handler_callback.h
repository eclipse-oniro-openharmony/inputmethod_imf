/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#ifndef NATIVE_MESSAGE_HANDLER_CALLBACK_H
#define NATIVE_MESSAGE_HANDLER_CALLBACK_H
#include "input_method_controller.h"
#include "msg_handler_callback_interface.h"
#include "native_inputmethod_types.h"
namespace OHOS {
namespace MiscServices {
class NativeMessageHandlerCallback : public OHOS::MiscServices::MsgHandlerCallbackInterface {
public:
    explicit NativeMessageHandlerCallback(InputMethod_MessageHandlerProxy *messageHandler) : messageHandler_(messageHandler) {};
    virtual ~NativeMessageHandlerCallback() {};
    virtual int32_t OnTerminated() override;
    virtual int32_t OnMessage(const ArrayBuffer &arrayBuffer) override;

private:
    InputMethod_MessageHandlerProxy *messageHandler_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // NATIVE_MESSAGE_HANDLER_CALLBACK_H