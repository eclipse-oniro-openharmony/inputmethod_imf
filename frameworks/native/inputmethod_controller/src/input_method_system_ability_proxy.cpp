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

#include "input_method_system_ability_proxy.h"

#include "element_name.h"
#include "global.h"
#include "inputmethod_service_ipc_interface_code.h"
#include "itypes_util.h"
#include "message_option.h"

namespace OHOS {
namespace MiscServices {
using namespace ErrorCode;

InputMethodSystemAbilityProxy::InputMethodSystemAbilityProxy(const sptr<IRemoteObject> &object)
    : IRemoteProxy<IInputMethodSystemAbility>(object)
{
}

int32_t InputMethodSystemAbilityProxy::StartInput(InputClientInfo &inputClientInfo, sptr<IRemoteObject> &agent)
{
    return SendRequest(
        static_cast<uint32_t>(InputMethodInterfaceCode::START_INPUT),
        [&inputClientInfo](MessageParcel &data) {
            return ITypesUtil::Marshal(
                data, inputClientInfo, inputClientInfo.client->AsObject(), inputClientInfo.channel);
        },
        [&agent](MessageParcel &reply) {
            agent = reply.ReadRemoteObject();
            return true;
        });
}

int32_t InputMethodSystemAbilityProxy::ConnectSystemCmd(const sptr<IRemoteObject> &channel, sptr<IRemoteObject> &agent)
{
    return SendRequest(
        static_cast<uint32_t>(InputMethodInterfaceCode::CONNECT_SYSTEM_CMD),
        [channel](MessageParcel &data) { return data.WriteRemoteObject(channel); },
        [&agent](MessageParcel &reply) { return ITypesUtil::Unmarshal(reply, agent); });
}

int32_t InputMethodSystemAbilityProxy::ShowCurrentInput()
{
    return SendRequest(static_cast<uint32_t>(InputMethodInterfaceCode::SHOW_CURRENT_INPUT));
}

int32_t InputMethodSystemAbilityProxy::HideCurrentInput()
{
    return SendRequest(static_cast<uint32_t>(InputMethodInterfaceCode::HIDE_CURRENT_INPUT));
}

int32_t InputMethodSystemAbilityProxy::StopInputSession()
{
    return SendRequest(static_cast<uint32_t>(InputMethodInterfaceCode::STOP_INPUT_SESSION));
}

int32_t InputMethodSystemAbilityProxy::ShowInput(sptr<IInputClient> client)
{
    return SendRequest(static_cast<uint32_t>(InputMethodInterfaceCode::SHOW_INPUT),
        [client](MessageParcel &data) { return data.WriteRemoteObject(client->AsObject()); });
}

int32_t InputMethodSystemAbilityProxy::HideInput(sptr<IInputClient> client)
{
    return SendRequest(static_cast<uint32_t>(InputMethodInterfaceCode::HIDE_INPUT),
        [client](MessageParcel &data) { return data.WriteRemoteObject(client->AsObject()); });
}

int32_t InputMethodSystemAbilityProxy::ReleaseInput(sptr<IInputClient> client)
{
    return SendRequest(static_cast<uint32_t>(InputMethodInterfaceCode::RELEASE_INPUT),
        [client](MessageParcel &data) { return data.WriteRemoteObject(client->AsObject()); });
}

int32_t InputMethodSystemAbilityProxy::RequestShowInput()
{
    return SendRequest(static_cast<uint32_t>(InputMethodInterfaceCode::REQUEST_SHOW_INPUT));
}

int32_t InputMethodSystemAbilityProxy::RequestHideInput()
{
    return SendRequest(static_cast<uint32_t>(InputMethodInterfaceCode::REQUEST_HIDE_INPUT));
}

int32_t InputMethodSystemAbilityProxy::InitConnect()
{
    return SendRequest(static_cast<uint32_t>(InputMethodInterfaceCode::INIT_CONNECT));
}

int32_t InputMethodSystemAbilityProxy::DisplayOptionalInputMethod()
{
    return SendRequest(static_cast<uint32_t>(InputMethodInterfaceCode::DISPLAY_OPTIONAL_INPUT_METHOD));
}

int32_t InputMethodSystemAbilityProxy::SetCoreAndAgent(
    const sptr<IInputMethodCore> &core, const sptr<IRemoteObject> &agent)
{
    return SendRequest(
        static_cast<uint32_t>(InputMethodInterfaceCode::SET_CORE_AND_AGENT), [core, agent](MessageParcel &data) {
            return data.WriteRemoteObject(core->AsObject()) && data.WriteRemoteObject(agent);
        });
}

int32_t InputMethodSystemAbilityProxy::GetDefaultInputMethod(std::shared_ptr<Property> &property, bool isBrief)
{
    return SendRequest(
        static_cast<uint32_t>(InputMethodInterfaceCode::GET_DEFAULT_INPUT_METHOD),
        [isBrief](
            MessageParcel &data) { return ITypesUtil::Marshal(data, isBrief); },
        [&property](MessageParcel &reply) {
            property = std::make_shared<Property>();
            return ITypesUtil::Unmarshal(reply, *property);
        });
}

int32_t InputMethodSystemAbilityProxy::GetInputMethodConfig(OHOS::AppExecFwk::ElementName &inputMethodConfig)
{
    return SendRequest(static_cast<uint32_t>(InputMethodInterfaceCode::GET_INPUT_METHOD_SETTINGS), nullptr,
        [&inputMethodConfig](MessageParcel& reply) {
            return ITypesUtil::Unmarshal(reply, inputMethodConfig);
        });
}

int32_t InputMethodSystemAbilityProxy::GetSecurityMode(int32_t &security)
{
    return SendRequest(static_cast<uint32_t>(InputMethodInterfaceCode::GET_SECURITY_MODE), nullptr,
        [&security](MessageParcel& reply) {
            return ITypesUtil::Unmarshal(reply, security);
        });
}

int32_t InputMethodSystemAbilityProxy::UnRegisteredProxyIme(UnRegisteredType type, const sptr<IInputMethodCore> &core)
{
    return SendRequest(
        static_cast<uint32_t>(InputMethodInterfaceCode::UNREGISTERED_PROXY_IME), [&type, &core](MessageParcel &data) {
            return ITypesUtil::Marshal(data, static_cast<int32_t>(type), core->AsObject());
        });
}

std::shared_ptr<Property> InputMethodSystemAbilityProxy::GetCurrentInputMethod()
{
    std::shared_ptr<Property> property = nullptr;
    int32_t ret = SendRequest(static_cast<uint32_t>(InputMethodInterfaceCode::GET_CURRENT_INPUT_METHOD), nullptr,
        [&property](MessageParcel &reply) {
            property = std::make_shared<Property>();
            return ITypesUtil::Unmarshal(reply, *property);
        });
    return ret != ErrorCode::NO_ERROR ? nullptr : property;
}

std::shared_ptr<SubProperty> InputMethodSystemAbilityProxy::GetCurrentInputMethodSubtype()
{
    std::shared_ptr<SubProperty> property = nullptr;
    int32_t ret = SendRequest(static_cast<uint32_t>(InputMethodInterfaceCode::GET_CURRENT_INPUT_METHOD_SUBTYPE),
        nullptr, [&property](MessageParcel &reply) {
            property = std::make_shared<SubProperty>();
            return ITypesUtil::Unmarshal(reply, *property);
        });
    return ret != ErrorCode::NO_ERROR ? nullptr : property;
}

bool InputMethodSystemAbilityProxy::IsDefaultImeSet()
{
    bool isDefaultImeSet = false;
    IMSA_HILOGI("InputMethodSystemAbilityProxy::IsDefaultImeSet enter.");
    SendRequest(static_cast<uint32_t>(InputMethodInterfaceCode::IS_DEFAULT_IME_SET), nullptr,
        [&isDefaultImeSet](MessageParcel &reply) { return ITypesUtil::Unmarshal(reply, isDefaultImeSet); });
    return isDefaultImeSet;
}
 
bool InputMethodSystemAbilityProxy::EnableIme(const std::string &bundleName)
{
    bool enableIme = false;
    IMSA_HILOGI("InputMethodSystemAbilityProxy::EnableIme enter.");
    SendRequest(static_cast<uint32_t>(InputMethodInterfaceCode::ENABLE_IME),
        [&bundleName](MessageParcel &data) { return ITypesUtil::Marshal(data, bundleName); },
        [&enableIme](MessageParcel &reply) { return ITypesUtil::Unmarshal(reply, enableIme); });
    return enableIme;
}

int32_t InputMethodSystemAbilityProxy::ListInputMethod(InputMethodStatus status, std::vector<Property> &props)
{
    return SendRequest(
        static_cast<uint32_t>(InputMethodInterfaceCode::LIST_INPUT_METHOD),
        [status](MessageParcel &data) { return ITypesUtil::Marshal(data, uint32_t(status)); },
        [&props](MessageParcel &reply) { return ITypesUtil::Unmarshal(reply, props); });
}

int32_t InputMethodSystemAbilityProxy::ShowCurrentInputDeprecated()
{
    return SendRequest(static_cast<uint32_t>(InputMethodInterfaceCode::SHOW_CURRENT_INPUT_DEPRECATED));
}

int32_t InputMethodSystemAbilityProxy::HideCurrentInputDeprecated()
{
    return SendRequest(static_cast<uint32_t>(InputMethodInterfaceCode::HIDE_CURRENT_INPUT_DEPRECATED));
}

int32_t InputMethodSystemAbilityProxy::ListInputMethodSubtype(
    const std::string &name, std::vector<SubProperty> &subProps)
{
    return SendRequest(
        static_cast<uint32_t>(InputMethodInterfaceCode::LIST_INPUT_METHOD_SUBTYPE),
        [&name](MessageParcel &data) { return ITypesUtil::Marshal(data, name); },
        [&subProps](MessageParcel &reply) { return ITypesUtil::Unmarshal(reply, subProps); });
}

int32_t InputMethodSystemAbilityProxy::ListCurrentInputMethodSubtype(std::vector<SubProperty> &subProps)
{
    return SendRequest(static_cast<uint32_t>(InputMethodInterfaceCode::LIST_CURRENT_INPUT_METHOD_SUBTYPE), nullptr,
        [&subProps](MessageParcel &reply) { return ITypesUtil::Unmarshal(reply, subProps); });
}

int32_t InputMethodSystemAbilityProxy::SwitchInputMethod(
    const std::string &name, const std::string &subName, SwitchTrigger trigger)
{
    return SendRequest(static_cast<uint32_t>(InputMethodInterfaceCode::SWITCH_INPUT_METHOD),
        [&name, &subName, trigger](MessageParcel &data) { return ITypesUtil::Marshal(data, name, subName, trigger); });
}

int32_t InputMethodSystemAbilityProxy::PanelStatusChange(const InputWindowStatus &status, const ImeWindowInfo &info)
{
    return SendRequest(static_cast<uint32_t>(InputMethodInterfaceCode::PANEL_STATUS_CHANGE),
        [status, &info](MessageParcel &data) {
            return ITypesUtil::Marshal(data, static_cast<uint32_t>(status), info);
        });
}

int32_t InputMethodSystemAbilityProxy::UpdateListenEventFlag(InputClientInfo &clientInfo, uint32_t eventFlag)
{
    return SendRequest(static_cast<uint32_t>(InputMethodInterfaceCode::UPDATE_LISTEN_EVENT_FLAG),
        [&clientInfo, eventFlag](MessageParcel &data) {
            return ITypesUtil::Marshal(data, clientInfo, clientInfo.client->AsObject(), clientInfo.channel, eventFlag);
        });
}

bool InputMethodSystemAbilityProxy::IsCurrentIme()
{
    bool isCurrentIme = false;
    SendRequest(static_cast<uint32_t>(InputMethodInterfaceCode::IS_CURRENT_IME), nullptr,
        [&isCurrentIme](MessageParcel &reply) { return ITypesUtil::Unmarshal(reply, isCurrentIme); });
    return isCurrentIme;
}

bool InputMethodSystemAbilityProxy::IsCurrentImeByPid(int32_t pid)
{
    bool isCurrentIme = false;
    SendRequest(static_cast<uint32_t>(InputMethodInterfaceCode::IS_CURRENT_IME_BY_PID),
        [&pid](MessageParcel &data) { return ITypesUtil::Marshal(data, pid); },
        [&isCurrentIme](MessageParcel &reply) { return ITypesUtil::Unmarshal(reply, isCurrentIme); });
    return isCurrentIme;
}

bool InputMethodSystemAbilityProxy::IsInputTypeSupported(InputType type)
{
    bool isSupported = false;
    SendRequest(
        static_cast<uint32_t>(InputMethodInterfaceCode::IS_INPUT_TYPE_SUPPORTED),
        [type](MessageParcel &data) { return ITypesUtil::Marshal(data, type); },
        [&isSupported](MessageParcel &reply) { return ITypesUtil::Unmarshal(reply, isSupported); });
    return isSupported;
}

int32_t InputMethodSystemAbilityProxy::StartInputType(InputType type)
{
    return SendRequest(static_cast<uint32_t>(InputMethodInterfaceCode::START_INPUT_TYPE),
        [&type](MessageParcel &data) { return ITypesUtil::Marshal(data, type); });
}

int32_t InputMethodSystemAbilityProxy::ExitCurrentInputType()
{
    return SendRequest(static_cast<uint32_t>(InputMethodInterfaceCode::EXIT_CURRENT_INPUT_TYPE));
}

int32_t InputMethodSystemAbilityProxy::IsPanelShown(const PanelInfo &panelInfo, bool &isShown)
{
    return SendRequest(
        static_cast<uint32_t>(InputMethodInterfaceCode::IS_PANEL_SHOWN),
        [&panelInfo](MessageParcel &data) { return ITypesUtil::Marshal(data, panelInfo); },
        [&isShown](MessageParcel &reply) { return ITypesUtil::Unmarshal(reply, isShown); });
}

int32_t InputMethodSystemAbilityProxy::IsDefaultIme()
{
    return SendRequest(static_cast<uint32_t>(InputMethodInterfaceCode::IS_DEFAULT_IME));
}

int32_t InputMethodSystemAbilityProxy::GetInputMethodState(EnabledStatus &status)
{
    int32_t statusTmp = 0;
    auto ret = SendRequest(static_cast<uint32_t>(InputMethodInterfaceCode::GET_IME_STATE), nullptr,
        [&statusTmp](MessageParcel &reply) { return ITypesUtil::Unmarshal(reply, statusTmp); });
    if (ret != ErrorCode::NO_ERROR) {
        return ret;
    }
    status = static_cast<EnabledStatus>(statusTmp);
    return ErrorCode::NO_ERROR;
}

void InputMethodSystemAbilityProxy::GetMessageOption(int32_t code, MessageOption &option)
{
    switch (code) {
        case static_cast<int32_t>(InputMethodInterfaceCode::PANEL_STATUS_CHANGE): {
            IMSA_HILOGD("Async IPC.");
            option.SetFlags(MessageOption::TF_ASYNC);
            break;
        }
        default:
            option.SetFlags(MessageOption::TF_SYNC);
            break;
    }
}

int32_t InputMethodSystemAbilityProxy::SendRequest(int code, ParcelHandler input, ParcelHandler output)
{
    IMSA_HILOGD("IMSAProxy, code = %{public}d.", code);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    GetMessageOption(code, option);

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        IMSA_HILOGE("write interface token failed!");
        return ErrorCode::ERROR_EX_ILLEGAL_ARGUMENT;
    }
    if (input != nullptr && (!input(data))) {
        IMSA_HILOGE("write data failed!");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    auto remote = Remote();
    if (remote == nullptr) {
        IMSA_HILOGE("remote is nullptr!");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    auto ret = remote->SendRequest(code, data, reply, option);
    if (ret != NO_ERROR) {
        IMSA_HILOGE("failed to send request, code: %{public}d, ret %{public}d!", code, ret);
        return ret;
    }
    if (option.GetFlags() == MessageOption::TF_ASYNC) {
        return ErrorCode::NO_ERROR;
    }
    ret = reply.ReadInt32();
    if (ret != NO_ERROR) {
        IMSA_HILOGE("dispose failed in service, code: %{public}d, ret: %{public}d!", code, ret);
        return ret;
    }
    if (output != nullptr && (!output(reply))) {
        IMSA_HILOGE("reply parcel error!");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    return ErrorCode::NO_ERROR;
}
} // namespace MiscServices
} // namespace OHOS