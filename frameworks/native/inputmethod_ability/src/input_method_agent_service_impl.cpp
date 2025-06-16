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

#include "input_method_agent_service_impl.h"

#include "global.h"
#include "input_method_ability.h"
#include "ipc_skeleton.h"
#include "itypes_util.h"
#include "task_manager.h"
#include "tasks/task_imsa.h"
#include "input_method_tools.h"

namespace OHOS {
namespace MiscServices {
using namespace MessageID;
InputMethodAgentServiceImpl::InputMethodAgentServiceImpl() {}

InputMethodAgentServiceImpl::~InputMethodAgentServiceImpl() {}

ErrCode InputMethodAgentServiceImpl::DispatchKeyEvent(
    const MiscServices::KeyEventValue &keyEvent, const sptr<IKeyEventConsumer> &consumer)
{
    sptr<KeyEventConsumerProxy> proxyConsumer = new (std::nothrow) KeyEventConsumerProxy(consumer->AsObject());
    return InputMethodAbility::GetInstance().DispatchKeyEvent(keyEvent.event, proxyConsumer);
}

ErrCode InputMethodAgentServiceImpl::SetCallingWindow(uint32_t windowId)
{
    InputMethodAbility::GetInstance().SetCallingWindow(windowId);
    return ERR_OK;
}

ErrCode InputMethodAgentServiceImpl::OnCursorUpdate(int32_t positionX, int32_t positionY, int height)
{
    auto task = std::make_shared<TaskImsaOnCursorUpdate>(positionX, positionY, height);
    TaskManager::GetInstance().PostTask(task);
    return ERR_OK;
}

ErrCode InputMethodAgentServiceImpl::OnSelectionChange(
    const std::string& text, int32_t oldBegin, int32_t oldEnd, int32_t newBegin, int32_t newEnd)
{
    std::u16string u16text = Str8ToStr16(text);
    auto task = std::make_shared<TaskImsaOnSelectionChange>(u16text, oldBegin, oldEnd, newBegin, newEnd);
    TaskManager::GetInstance().PostTask(task);
    return ERR_OK;
}

ErrCode InputMethodAgentServiceImpl::SendPrivateCommand(
    const Value &value)
{
    if (!InputMethodAbility::GetInstance().IsDefaultIme()) {
        IMSA_HILOGE("current is not default ime!");
        return ErrorCode::ERROR_NOT_DEFAULT_IME;
    }
    std::unordered_map<std::string, PrivateDataValue> privateCommand;
    privateCommand = value.valueMap;
    auto task = std::make_shared<TaskImsaSendPrivateCommand>(privateCommand);
    TaskManager::GetInstance().PostTask(task);
    return ERR_OK;
}

ErrCode InputMethodAgentServiceImpl::OnAttributeChange(const InputAttributeInner &attributeInner)
{
    InputAttribute attribute = InputMethodTools::GetInstance().InnerToAttribute(attributeInner);
    auto task = std::make_shared<TaskImsaAttributeChange>(attribute);
    TaskManager::GetInstance().PostTask(task);
    return ERR_OK;
}

ErrCode InputMethodAgentServiceImpl::SendMessage(const ArrayBuffer &arraybuffer)
{
    return InputMethodAbility::GetInstance().RecvMessage(arraybuffer);
}


ErrCode InputMethodAgentServiceImpl::DiscardTypingText()
{
    IMSA_HILOGD("DiscardTypingText run");
    std::string type = "discardTypingText";
    auto ret = InputMethodAbility::GetInstance().IsCallbackRegistered(type);
    if (!ret) {
        IMSA_HILOGE("callback not registered");
        return ErrorCode::ERROR_MSG_HANDLER_NOT_REGIST;
    }
    auto task = std::make_shared<TaskImsaDiscardTypingText>();
    TaskManager::GetInstance().PostTask(task);
    return ErrorCode::NO_ERROR;
}

ErrCode InputMethodAgentServiceImpl::ResponseDataChannel(uint64_t msgId, int code, const ResponseDataInner &msg)
{
    return InputMethodAbility::GetInstance().OnResponse(msgId, code, msg.rspData);
}
} // namespace MiscServices
} // namespace OHOS