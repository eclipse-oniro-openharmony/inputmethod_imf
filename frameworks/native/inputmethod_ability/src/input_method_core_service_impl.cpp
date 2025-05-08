/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#include "input_method_core_service_impl.h"

#include <cstdint>
#include <string_ex.h>

#include "iinput_data_channel.h"
#include "input_control_channel_proxy.h"
#include "input_method_ability.h"
#include "ipc_skeleton.h"
#include "itypes_util.h"
#include "message_parcel.h"
#include "system_cmd_channel_proxy.h"
#include "task_manager.h"
#include "tasks/task_imsa.h"
#include "input_method_tools.h"

namespace OHOS {
namespace MiscServices {
using namespace MessageID;
InputMethodCoreServiceImpl::InputMethodCoreServiceImpl() {}

InputMethodCoreServiceImpl::~InputMethodCoreServiceImpl() {}

ErrCode InputMethodCoreServiceImpl::InitInputControlChannel(const sptr<IInputControlChannel> &inputControlChannel)
{
    auto task = std::make_shared<TaskImsaInitInputCtrlChannel>(inputControlChannel->AsObject());
    TaskManager::GetInstance().PostTask(task);
    return ERR_OK;
}

ErrCode InputMethodCoreServiceImpl::ShowKeyboard(int32_t requestKeyboardReason)
{
    auto task = std::make_shared<TaskImsaShowKeyboard>(requestKeyboardReason);
    TaskManager::GetInstance().PostTask(task);
    return ERR_OK;
}

ErrCode InputMethodCoreServiceImpl::HideKeyboard()
{
    auto task = std::make_shared<TaskImsaHideKeyboard>();
    TaskManager::GetInstance().PostTask(task);
    return ERR_OK;
}

ErrCode InputMethodCoreServiceImpl::StopInputService(bool isTerminateIme)
{
    auto task = std::make_shared<TaskImsaStopInputService>(isTerminateIme);
    TaskManager::GetInstance().PostTask(task);
    return ERR_OK;
}

ErrCode InputMethodCoreServiceImpl::OnConnectSystemCmd(
    const sptr<IRemoteObject> &channel, sptr<IRemoteObject> &agent)
{
    return InputMethodAbility::GetInstance().OnConnectSystemCmd(channel, agent);
}

ErrCode InputMethodCoreServiceImpl::StartInput(const InputClientInfoInner &clientInfoInner, bool isBindFromClient)
{
    InputClientInfo clientInfo =
        InputMethodTools::GetInstance().InnerToInputClientInfo(clientInfoInner);
    auto task = std::make_shared<TaskImsaStartInput>(clientInfo, isBindFromClient);
    TaskManager::GetInstance().PostTask(task);
    return ERR_OK;
}

ErrCode InputMethodCoreServiceImpl::SetSubtype(const SubProperty &property)
{
    auto task = std::make_shared<TaskImsaOnSetSubProperty>(property);
    TaskManager::GetInstance().PostTask(task);
    return ERR_OK;
}

ErrCode InputMethodCoreServiceImpl::OnSecurityChange(int32_t security)
{
    return InputMethodAbility::GetInstance().OnSecurityChange(security);
}

ErrCode InputMethodCoreServiceImpl::OnSetInputType(int32_t inputType)
{
    InputMethodAbility::GetInstance().OnSetInputType(static_cast<InputType>(inputType));
    return ERR_OK;
}

ErrCode InputMethodCoreServiceImpl::StopInput(const sptr<IRemoteObject> &channel, uint32_t sessionId)
{
    auto task = std::make_shared<TaskImsaStopInput>(channel, sessionId);
    TaskManager::GetInstance().PostTask(task);
    return ERR_OK;
}

ErrCode InputMethodCoreServiceImpl::IsEnable(bool &resultValue)
{
    resultValue = InputMethodAbility::GetInstance().IsEnable();
    return ERR_OK;
}

ErrCode InputMethodCoreServiceImpl::IsPanelShown(const PanelInfo &panelInfo, bool &isShown)
{
    return InputMethodAbility::GetInstance().IsPanelShown(panelInfo, isShown);
}

ErrCode InputMethodCoreServiceImpl::OnClientInactive(const sptr<IRemoteObject> &channel)
{
    auto task = std::make_shared<TaskImsaOnClientInactive>(channel);
    TaskManager::GetInstance().PostTask(task);
    return ERR_OK;
}

ErrCode InputMethodCoreServiceImpl::OnCallingDisplayIdChanged(uint64_t displayId)
{
    return InputMethodAbility::GetInstance().OnCallingDisplayIdChanged(displayId);
}

ErrCode InputMethodCoreServiceImpl::OnSendPrivateData(const Value &Value)
{
    std::unordered_map<std::string, PrivateDataValue> privateCommand;
    privateCommand = Value.valueMap;
    return InputMethodAbility::GetInstance().OnSendPrivateData(privateCommand);
}

} // namespace MiscServices
} // namespace OHOS