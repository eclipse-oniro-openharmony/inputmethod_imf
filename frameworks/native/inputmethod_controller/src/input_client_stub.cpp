/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include "input_client_stub.h"

#include "global.h"
#include "input_method_controller.h"
#include "ipc_object_stub.h"
#include "ipc_skeleton.h"
#include "ipc_types.h"
#include "itypes_util.h"
#include "message.h"

namespace OHOS {
namespace MiscServices {
InputClientStub::InputClientStub()
{
}

InputClientStub::~InputClientStub()
{
}

int32_t InputClientStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    IMSA_HILOGD("InputClientStub code = %{public}u, callingPid: %{public}d, callingUid: %{public}d", code,
        IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid());
    auto descriptorToken = data.ReadInterfaceToken();
    if (descriptorToken != GetDescriptor()) {
        return ErrorCode::ERROR_STATUS_UNKNOWN_TRANSACTION;
    }
    switch (code) {
        case ON_INPUT_READY: {
            OnInputReadyOnRemote(data, reply);
            break;
        }
        case ON_INPUT_STOP: {
            OnInputStopOnRemote(data, reply);
            break;
        }
        case ON_SWITCH_INPUT: {
            return OnSwitchInputOnRemote(data, reply);
        }
        case ON_PANEL_STATUS_CHANGE: {
            return OnPanelStatusChangeOnRemote(data, reply);
        }
        case DEACTIVATE_CLIENT: {
            return DeactivateClientOnRemote(data, reply);
        }
        default:
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return NO_ERROR;
}

void InputClientStub::OnInputReadyOnRemote(MessageParcel &data, MessageParcel &reply)
{
    IMSA_HILOGI("ClientStub, run in");
    auto object = data.ReadRemoteObject();
    InputMethodController::GetInstance()->OnInputReady(object);
}

int32_t InputClientStub::OnInputStopOnRemote(MessageParcel &data, MessageParcel &reply)
{
    return reply.WriteInt32(OnInputStop()) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputClientStub::OnSwitchInputOnRemote(MessageParcel &data, MessageParcel &reply)
{
    Property property;
    SubProperty subProperty;
    if (!ITypesUtil::Unmarshal(data, property, subProperty)) {
        IMSA_HILOGE("read message parcel failed");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    return reply.WriteInt32(OnSwitchInput(property, subProperty)) ? ErrorCode::NO_ERROR
                                                                  : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputClientStub::OnPanelStatusChangeOnRemote(MessageParcel &data, MessageParcel &reply)
{
    uint32_t status = 0;
    ImeWindowInfo info;
    if (!ITypesUtil::Unmarshal(data, status, info)) {
        IMSA_HILOGE("read message parcel failed");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    return reply.WriteInt32(OnPanelStatusChange(static_cast<InputWindowStatus>(status), info))
               ? ErrorCode::NO_ERROR
               : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputClientStub::DeactivateClientOnRemote(MessageParcel &data, MessageParcel &reply)
{
    DeactivateClient();
    return reply.WriteInt32(ErrorCode::NO_ERROR) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputClientStub::OnInputReady(const sptr<IInputMethodAgent> &agent)
{
    return ErrorCode::NO_ERROR;
}

int32_t InputClientStub::OnInputStop()
{
    InputMethodController::GetInstance()->OnInputStop();
    return ErrorCode::NO_ERROR;
}

int32_t InputClientStub::OnSwitchInput(const Property &property, const SubProperty &subProperty)
{
    return ImeEventMonitorManager::GetInstance().OnImeChange(property, subProperty);
}

int32_t InputClientStub::OnPanelStatusChange(const InputWindowStatus &status, const ImeWindowInfo &info)
{
    return ImeEventMonitorManager::GetInstance().OnPanelStatusChange(status, info);
}

void InputClientStub::DeactivateClient()
{
    InputMethodController::GetInstance()->DeactivateClient();
}
} // namespace MiscServices
} // namespace OHOS
