/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include "input_method_system_ability_stub.h"

#include <chrono>
#include <cinttypes>
#include <memory>

#include "element_name.h"
#include "input_client_proxy.h"
#include "input_method_core_proxy.h"
#include "ipc_skeleton.h"
#include "itypes_util.h"
#include "xcollie/xcollie.h"
#include "xcollie/xcollie_define.h"
namespace OHOS {
namespace MiscServices {
using namespace std::chrono;
using namespace HiviewDFX;
constexpr uint32_t FATAL_TIMEOUT = 30;    // 30s
constexpr int64_t WARNING_TIMEOUT = 5000; // 5s
int32_t InputMethodSystemAbilityStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    if (code != static_cast<uint32_t>(InputMethodInterfaceCode::RELEASE_INPUT)) {
        IMSA_HILOGI("IMSA, code = %{public}u, callingPid/Uid/timestamp: %{public}d/%{public}d/%{public}lld", code,
            IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid(),
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
                .count());
    }
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (remoteDescriptor != IInputMethodSystemAbility::GetDescriptor()) {
        IMSA_HILOGE("%{public}s descriptor failed!", __func__);
        return ErrorCode::ERROR_STATUS_UNKNOWN_TRANSACTION;
    }
    if (code >= static_cast<uint32_t>(InputMethodInterfaceCode::IMS_CMD_BEGIN) &&
        code < static_cast<uint32_t>(InputMethodInterfaceCode::IMS_CMD_END)) {
        // service reboot when timeout 30s
        auto id = XCollie::GetInstance().SetTimer("IMSA_API[" + std::to_string(code) + "]", FATAL_TIMEOUT, nullptr,
            nullptr, XCOLLIE_FLAG_DEFAULT);
        int64_t startPoint = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
        auto ret = (this->*HANDLERS[code])(data, reply);
        int64_t costTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count() - startPoint;
        // log warning when timeout 5s
        if (costTime > WARNING_TIMEOUT) {
            IMSA_HILOGW("code: %{public}d, pid: %{public}d, uid: %{public}d, cost: %{public}" PRId64 "", code,
                IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid(), costTime);
        }
        XCollie::GetInstance().CancelTimer(id);
        return ret;
    } else {
        IMSA_HILOGE("code error, code = %{public}u, callingPid: %{public}d, callingUid: %{public}d.", code,
            IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid());
        return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
}

int32_t InputMethodSystemAbilityStub::StartInputOnRemote(MessageParcel &data, MessageParcel &reply)
{
    InputClientInfo clientInfo;
    sptr<IRemoteObject> client = nullptr;
    if (!ITypesUtil::Unmarshal(data, clientInfo, client, clientInfo.channel)) {
        IMSA_HILOGE("read clientInfo failed!");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    clientInfo.client = iface_cast<IInputClient>(client);
    sptr<IRemoteObject> agent = nullptr;
    int32_t ret = StartInput(clientInfo, agent);
    return reply.WriteInt32(ret) && reply.WriteRemoteObject(agent) ? ErrorCode::NO_ERROR
                                                                   : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodSystemAbilityStub::ShowCurrentInputOnRemote(MessageParcel &data, MessageParcel &reply)
{
    int32_t ret = ShowCurrentInput();
    return reply.WriteInt32(ret) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodSystemAbilityStub::HideCurrentInputOnRemote(MessageParcel &data, MessageParcel &reply)
{
    int32_t ret = HideCurrentInput();
    return reply.WriteInt32(ret) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodSystemAbilityStub::StopInputSessionOnRemote(MessageParcel &data, MessageParcel &reply)
{
    int32_t ret = StopInputSession();
    return reply.WriteInt32(ret) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodSystemAbilityStub::ShowInputOnRemote(MessageParcel &data, MessageParcel &reply)
{
    auto clientObject = data.ReadRemoteObject();
    if (clientObject == nullptr) {
        IMSA_HILOGE("clientObject is nullptr!");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    int32_t ret = ShowInput(iface_cast<IInputClient>(clientObject));
    return reply.WriteInt32(ret) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodSystemAbilityStub::HideInputOnRemote(MessageParcel &data, MessageParcel &reply)
{
    auto clientObject = data.ReadRemoteObject();
    if (clientObject == nullptr) {
        IMSA_HILOGE("clientObject is nullptr!");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    int32_t ret = HideInput(iface_cast<IInputClient>(clientObject));
    return reply.WriteInt32(ret) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodSystemAbilityStub::ReleaseInputOnRemote(MessageParcel &data, MessageParcel &reply)
{
    auto clientObject = data.ReadRemoteObject();
    if (clientObject == nullptr) {
        IMSA_HILOGE("clientObject is nullptr!");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    int32_t ret = ReleaseInput(iface_cast<IInputClient>(clientObject));
    return reply.WriteInt32(ret) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodSystemAbilityStub::RequestShowInputOnRemote(MessageParcel &data, MessageParcel &reply)
{
    return reply.WriteInt32(RequestShowInput()) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodSystemAbilityStub::RequestHideInputOnRemote(MessageParcel &data, MessageParcel &reply)
{
    return reply.WriteInt32(RequestHideInput()) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodSystemAbilityStub::DisplayOptionalInputMethodOnRemote(MessageParcel &data, MessageParcel &reply)
{
    int32_t ret = DisplayOptionalInputMethod();
    return reply.WriteInt32(ret) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodSystemAbilityStub::SetCoreAndAgentOnRemote(MessageParcel &data, MessageParcel &reply)
{
    auto coreObject = data.ReadRemoteObject();
    if (coreObject == nullptr) {
        IMSA_HILOGE("coreObject is nullptr!");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    auto agentObject = data.ReadRemoteObject();
    if (agentObject == nullptr) {
        IMSA_HILOGE("agentObject is nullptr!");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    int32_t ret = SetCoreAndAgent(iface_cast<IInputMethodCore>(coreObject), agentObject);
    return reply.WriteInt32(ret) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodSystemAbilityStub::GetDefaultInputMethodOnRemote(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Property> prop = std::make_shared<Property>();
    bool isBrief = false;
    auto ret = data.ReadBool(isBrief);
    if (!ret) {
        IMSA_HILOGE("read isBrief failed!");
    }
    ret = GetDefaultInputMethod(prop, isBrief);
    if (prop == nullptr) {
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    return ITypesUtil::Marshal(reply, ret, *prop) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodSystemAbilityStub::IsDefaultImeSetOnRemote(MessageParcel &data, MessageParcel &reply)
{
    return ITypesUtil::Marshal(reply, ErrorCode::NO_ERROR, IsDefaultImeSet()) ? ErrorCode::NO_ERROR
                                                                           : ErrorCode::ERROR_EX_PARCELABLE;
}
 
int32_t InputMethodSystemAbilityStub::EnableImeOnRemote(MessageParcel &data, MessageParcel &reply)
{
    std::string bundleName;
    if (!ITypesUtil::Unmarshal(data, bundleName)) {
        IMSA_HILOGE("unmarshal failed!");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    return ITypesUtil::Marshal(reply, ErrorCode::NO_ERROR, EnableIme(bundleName)) ? ErrorCode::NO_ERROR
                                                                        : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodSystemAbilityStub::GetInputMethodConfigOnRemote(MessageParcel &data, MessageParcel &reply)
{
    OHOS::AppExecFwk::ElementName inputMethodConfig;
    auto ret = GetInputMethodConfig(inputMethodConfig);
    IMSA_HILOGD("GetInputMethodConfigOnRemote inputMethodConfig is %{public}s, %{public}s",
        inputMethodConfig.GetBundleName().c_str(), inputMethodConfig.GetAbilityName().c_str());
    return ITypesUtil::Marshal(reply, ret, inputMethodConfig) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodSystemAbilityStub::GetSecurityModeOnRemote(MessageParcel &data, MessageParcel &reply)
{
    IMSA_HILOGD("GetSecurityModeOnRemote start.");
    int32_t security;
    auto ret = GetSecurityMode(security);
    IMSA_HILOGD("GetSecurityModeOnRemote, security: %{public}d", security);
    return ITypesUtil::Marshal(reply, ret, security) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodSystemAbilityStub::GetCurrentInputMethodOnRemote(MessageParcel &data, MessageParcel &reply)
{
    auto property = GetCurrentInputMethod();
    if (property == nullptr) {
        IMSA_HILOGE("property is nullptr!");
        return reply.WriteInt32(ErrorCode::ERROR_EX_NULL_POINTER) ? ErrorCode::NO_ERROR
                                                                  : ErrorCode::ERROR_EX_PARCELABLE;
    }
    if (!ITypesUtil::Marshal(reply, ErrorCode::NO_ERROR, *property)) {
        IMSA_HILOGE("marshal failed!");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodSystemAbilityStub::GetCurrentInputMethodSubtypeOnRemote(MessageParcel &data, MessageParcel &reply)
{
    auto property = GetCurrentInputMethodSubtype();
    if (property == nullptr) {
        IMSA_HILOGE("property is nullptr!");
        return reply.WriteInt32(ErrorCode::ERROR_EX_NULL_POINTER) ? ErrorCode::NO_ERROR
                                                                  : ErrorCode::ERROR_EX_PARCELABLE;
    }
    if (!ITypesUtil::Marshal(reply, ErrorCode::NO_ERROR, *property)) {
        IMSA_HILOGE("marshal failed!");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodSystemAbilityStub::ListInputMethodOnRemote(MessageParcel &data, MessageParcel &reply)
{
    uint32_t status;
    if (!ITypesUtil::Unmarshal(data, status)) {
        IMSA_HILOGE("read status failed!");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    std::vector<Property> properties = {};
    auto ret = ListInputMethod(InputMethodStatus(status), properties);
    if (!ITypesUtil::Marshal(reply, ret, properties)) {
        IMSA_HILOGE("marshal failed!");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodSystemAbilityStub::ListInputMethodSubtypeOnRemote(MessageParcel &data, MessageParcel &reply)
{
    std::string bundleName;
    if (!ITypesUtil::Unmarshal(data, bundleName)) {
        IMSA_HILOGE("read bundleName failed!");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    std::vector<SubProperty> subProps = {};
    auto ret = ListInputMethodSubtype(bundleName, subProps);
    if (!ITypesUtil::Marshal(reply, ret, subProps)) {
        IMSA_HILOGE("marshal failed!");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodSystemAbilityStub::ListCurrentInputMethodSubtypeOnRemote(MessageParcel &data, MessageParcel &reply)
{
    std::vector<SubProperty> subProps = {};
    auto ret = ListCurrentInputMethodSubtype(subProps);
    if (!ITypesUtil::Marshal(reply, ret, subProps)) {
        IMSA_HILOGE("marshal failed!");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodSystemAbilityStub::SwitchInputMethodOnRemote(MessageParcel &data, MessageParcel &reply)
{
    std::string name;
    std::string subName;
    SwitchTrigger trigger;
    if (!ITypesUtil::Unmarshal(data, name, subName, trigger)) {
        IMSA_HILOGE("unmarshal failed!");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    return reply.WriteInt32(SwitchInputMethod(name, subName, trigger)) ? ErrorCode::NO_ERROR
                                                                       : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodSystemAbilityStub::PanelStatusChangeOnRemote(MessageParcel &data, MessageParcel &reply)
{
    uint32_t status = 0;
    ImeWindowInfo info;
    if (!ITypesUtil::Unmarshal(data, status, info)) {
        IMSA_HILOGE("unmarshal failed!");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    int32_t ret = PanelStatusChange(static_cast<InputWindowStatus>(status), info);
    return reply.WriteInt32(ret) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodSystemAbilityStub::UpdateListenEventFlagOnRemote(MessageParcel &data, MessageParcel &reply)
{
    InputClientInfo clientInfo;
    sptr<IRemoteObject> client = nullptr;
    uint32_t eventFlag = 0;
    if (!ITypesUtil::Unmarshal(data, clientInfo, client, clientInfo.channel, eventFlag)) {
        IMSA_HILOGE("unmarshal failed!");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    clientInfo.client = iface_cast<IInputClient>(client);
    int32_t ret = UpdateListenEventFlag(clientInfo, eventFlag);
    return reply.WriteInt32(ret) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodSystemAbilityStub::SetCallingWindowOnRemote(MessageParcel &data, MessageParcel &reply)
{
    auto clientObject = data.ReadRemoteObject();
    if (clientObject == nullptr) {
        IMSA_HILOGE("clientObject is nullptr!");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    uint32_t windowId = 0;
    if (!ITypesUtil::Unmarshal(data, windowId)) {
        IMSA_HILOGE("unmarshal failed!");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    return ITypesUtil::Marshal(reply, SetCallingWindow(windowId, iface_cast<IInputClient>(clientObject))) ?
        ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodSystemAbilityStub::GetInputStartInfoOnRemote(MessageParcel &data, MessageParcel &reply)
{
    bool isInputStart = false;
    uint32_t callingWndId = 0;
    auto ret = GetInputStartInfo(isInputStart, callingWndId);
    return ITypesUtil::Marshal(reply, ret, isInputStart, callingWndId) ?
        ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodSystemAbilityStub::ShowCurrentInputOnRemoteDeprecated(MessageParcel &data, MessageParcel &reply)
{
    int32_t ret = ShowCurrentInputDeprecated();
    return reply.WriteInt32(ret) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodSystemAbilityStub::HideCurrentInputOnRemoteDeprecated(MessageParcel &data, MessageParcel &reply)
{
    int32_t ret = HideCurrentInputDeprecated();
    return reply.WriteInt32(ret) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodSystemAbilityStub::IsCurrentImeOnRemote(MessageParcel &data, MessageParcel &reply)
{
    return ITypesUtil::Marshal(reply, ErrorCode::NO_ERROR, IsCurrentIme()) ? ErrorCode::NO_ERROR
                                                                           : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodSystemAbilityStub::UnRegisteredProxyImeOnRemote(MessageParcel &data, MessageParcel &reply)
{
    int32_t type = -1;
    sptr<IRemoteObject> coreObject = nullptr;
    if (!ITypesUtil::Unmarshal(data, type, coreObject) || coreObject == nullptr) {
        IMSA_HILOGE("coreObject is nullptr!");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    int32_t ret = UnRegisteredProxyIme(static_cast<UnRegisteredType>(type), iface_cast<IInputMethodCore>(coreObject));
    return reply.WriteInt32(ret) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodSystemAbilityStub::IsInputTypeSupportedOnRemote(MessageParcel &data, MessageParcel &reply)
{
    InputType type;
    if (!ITypesUtil::Unmarshal(data, type)) {
        IMSA_HILOGE("unmarshal failed!");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    return ITypesUtil::Marshal(reply, ErrorCode::NO_ERROR, IsInputTypeSupported(type)) ? ErrorCode::NO_ERROR
                                                                                       : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodSystemAbilityStub::StartInputTypeOnRemote(MessageParcel &data, MessageParcel &reply)
{
    InputType type;
    if (!ITypesUtil::Unmarshal(data, type)) {
        IMSA_HILOGE("unmarshal failed!");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    return ITypesUtil::Marshal(reply, StartInputType(type)) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodSystemAbilityStub::ExitCurrentInputTypeOnRemote(MessageParcel &data, MessageParcel &reply)
{
    return ITypesUtil::Marshal(reply, ExitCurrentInputType()) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodSystemAbilityStub::IsPanelShownOnRemote(MessageParcel &data, MessageParcel &reply)
{
    PanelInfo info;
    if (!ITypesUtil::Unmarshal(data, info)) {
        IMSA_HILOGE("unmarshal failed!");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    bool isShown = false;
    int32_t ret = IsPanelShown(info, isShown);
    return ITypesUtil::Marshal(reply, ret, isShown) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodSystemAbilityStub::IsDefaultImeOnRemote(MessageParcel &data, MessageParcel &reply)
{
    return ITypesUtil::Marshal(reply, IsDefaultIme()) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodSystemAbilityStub::ConnectSystemCmdOnRemote(MessageParcel &data, MessageParcel &reply)
{
    auto systemCmdStub = data.ReadRemoteObject();
    if (systemCmdStub == nullptr) {
        IMSA_HILOGE("systemCmdStub is nullptr!");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    sptr<IRemoteObject> agent = nullptr;
    int32_t ret = ConnectSystemCmd(systemCmdStub, agent);
    return reply.WriteInt32(ret) && reply.WriteRemoteObject(agent) ? ErrorCode::NO_ERROR
                                                                   : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodSystemAbilityStub::IsCurrentImeByPidOnRemote(MessageParcel &data, MessageParcel &reply)
{
    int32_t pid = -1;
    if (!ITypesUtil::Unmarshal(data, pid)) {
        IMSA_HILOGE("unmarshal failed!");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    return ITypesUtil::Marshal(reply, ErrorCode::NO_ERROR, IsCurrentImeByPid(pid)) ? ErrorCode::NO_ERROR
                                                                           : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodSystemAbilityStub::InitConnectOnRemote(MessageParcel &data, MessageParcel &reply)
{
    return reply.WriteInt32(InitConnect()) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodSystemAbilityStub::GetInputMethodStateOnRemote(MessageParcel &data, MessageParcel &reply)
{
    EnabledStatus status = EnabledStatus::DISABLED;
    int32_t ret = GetInputMethodState(status);
    return ITypesUtil::Marshal(reply, ret, static_cast<int32_t>(status)) ? ErrorCode::NO_ERROR
                                                                         : ErrorCode::ERROR_EX_PARCELABLE;
}
} // namespace MiscServices
} // namespace OHOS