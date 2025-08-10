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

#include "input_client_service_impl.h"

#include "input_client_stub.h"
#include "ime_event_monitor_manager_impl.h"
#include "input_method_controller.h"

namespace OHOS {
namespace MiscServices {

InputClientServiceImpl::InputClientServiceImpl() {}

InputClientServiceImpl::~InputClientServiceImpl() {}

ErrCode InputClientServiceImpl::OnInputReady(const sptr<IRemoteObject> &agent, const BindImeInfo &imeInfo)
{
    IMSA_HILOGI("ClientStub start.");
    auto instance = InputMethodController::GetInstance();
    if (instance != nullptr) {
        instance->OnInputReady(agent, imeInfo);
    } else {
        IMSA_HILOGW("failed to get InputMethodController instance!");
    }
    return ERR_OK;
}

ErrCode InputClientServiceImpl::OnInputStop(bool isStopInactiveClient, const sptr<IRemoteObject> &object)
{
    auto instance = InputMethodController::GetInstance();
    if (instance != nullptr) {
        instance->OnInputStop(isStopInactiveClient, object);
    } else {
        IMSA_HILOGW("failed to get InputMethodController instance!");
    }
    return ERR_OK;
}

ErrCode InputClientServiceImpl::OnInputStopAsync(bool isStopInactiveClient)
{
    auto instance = InputMethodController::GetInstance();
    if (instance != nullptr) {
        instance->OnInputStop(isStopInactiveClient, nullptr);
    } else {
        IMSA_HILOGW("failed to get InputMethodController instance!");
    }
    return ERR_OK;
}

ErrCode InputClientServiceImpl::OnImeMirrorStop(const sptr<IRemoteObject> &object)
{
    auto instance = InputMethodController::GetInstance();
    if (instance != nullptr) {
        instance->OnImeMirrorStop(object);
    } else {
        IMSA_HILOGW("[ImeMirrorTag]failed to get InputMethodController instance!");
    }
    return ERR_OK;
}

ErrCode InputClientServiceImpl::OnSwitchInput(const Property &property, const SubProperty &subProperty)
{
    return ImeEventMonitorManagerImpl::GetInstance().OnImeChange(property, subProperty);
}

ErrCode InputClientServiceImpl::OnPanelStatusChange(const uint32_t status, const ImeWindowInfo& info)
{
    return ImeEventMonitorManagerImpl::GetInstance().OnPanelStatusChange(static_cast<InputWindowStatus>(status), info);
}

ErrCode InputClientServiceImpl::NotifyInputStart(uint32_t callingWndId, int32_t requestKeyboardReason)
{
    return ImeEventMonitorManagerImpl::GetInstance().OnInputStart(callingWndId, requestKeyboardReason);
}

ErrCode InputClientServiceImpl::NotifyInputStop()
{
    return ImeEventMonitorManagerImpl::GetInstance().OnInputStop();
}

ErrCode InputClientServiceImpl::DeactivateClient()
{
    auto instance = InputMethodController::GetInstance();
    if (instance != nullptr) {
        instance->DeactivateClient();
    } else {
        IMSA_HILOGW("failed to get InputMethodController instance!");
    }
    return ERR_OK;
}
}  // namespace MiscServices
}  // namespace OHOS