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

#ifndef FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_INPUT_CLIENT_IMPL_H
#define FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_INPUT_CLIENT_IMPL_H

#include "iinput_client.h"
#include "input_client_stub.h"
#include "iremote_object.h"

namespace OHOS {
namespace MiscServices {
class InputClientServiceImpl final : public InputClientStub,
    public std::enable_shared_from_this<InputClientServiceImpl> {
    DISALLOW_COPY_AND_MOVE(InputClientServiceImpl);

public:
    InputClientServiceImpl();
    ~InputClientServiceImpl();
    ErrCode OnInputReady(const sptr<IRemoteObject>& agent, const int64_t pid, const std::string& bundleName) override;
    ErrCode OnInputStop(bool isStopInactiveClient) override;
    ErrCode OnInputStopAsync(bool isStopInactiveClient) override;
    ErrCode OnSwitchInput(const Property& property, const SubProperty& subProperty) override;
    ErrCode OnPanelStatusChange(const uint32_t status, const ImeWindowInfo& info) override;
    ErrCode NotifyInputStart(uint32_t callingWndId, int32_t requestKeyboardReason) override;
    ErrCode NotifyInputStop() override;
    ErrCode DeactivateClient() override;
};
}  // namespace MiscServices
}  // namespace OHOS
#endif // FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_INPUT_CLIENT_IMPL_H