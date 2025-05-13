/*
 * Copyright (C) 2024-2024 Huawei Device Co., Ltd.
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

#ifndef FRAMEWORKS_INPUTMETHOD_ABILITY_INCLUDE_TASKS_TASK_IMSA_H
#define FRAMEWORKS_INPUTMETHOD_ABILITY_INCLUDE_TASKS_TASK_IMSA_H

#include "task.h"

#include "input_attribute.h"
#include "input_client_info.h"
#include "input_method_ability.h"
#include "input_method_property.h"
#include "iremote_object.h"

namespace OHOS {
namespace MiscServices {

class TaskImsaStartInput : public Task {
public:
    TaskImsaStartInput(const InputClientInfo &client, bool fromClient) : Task(TASK_TYPE_IMSA_START_INPUT)
    {
        auto func = [client, fromClient]() {
            InputMethodAbility::GetInstance().StartInput(client, fromClient);
        };
        actions_.emplace_back(std::make_unique<Action>(func));
    }
    ~TaskImsaStartInput() = default;
};

class TaskImsaStopInput : public Task {
public:
    explicit TaskImsaStopInput(sptr<IRemoteObject> channel, uint32_t sessionId) : Task(TASK_TYPE_IMSA_STOP_INPUT)
    {
        auto func = [channel, sessionId]() {
            InputMethodAbility::GetInstance().StopInput(channel, sessionId);
        };
        actions_.emplace_back(std::make_unique<Action>(func));
    }
    ~TaskImsaStopInput() = default;
};

class TaskImsaShowKeyboard : public Task {
public:
    TaskImsaShowKeyboard(int32_t requestKeyboardReason = 0) : Task(TASK_TYPE_IMSA_SHOW_KEYBOARD)
    {
        auto func = [requestKeyboardReason]() {
            InputMethodAbility::GetInstance().ShowKeyboard(requestKeyboardReason);
        };
        actions_.emplace_back(std::make_unique<Action>(func));
    }
    ~TaskImsaShowKeyboard() = default;
};

class TaskImsaHideKeyboard : public Task {
public:
    explicit TaskImsaHideKeyboard() : Task(TASK_TYPE_IMSA_HIDE_KEYBOARD)
    {
        auto func = []() {
            InputMethodAbility::GetInstance().HideKeyboard();
        };
        actions_.emplace_back(std::make_unique<Action>(func));
    }
    ~TaskImsaHideKeyboard() = default;
};

class TaskImsaOnClientInactive : public Task {
public:
    explicit TaskImsaOnClientInactive(sptr<IRemoteObject> channel) : Task(TASK_TYPE_IMSA_CLIENT_INACTIVE)
    {
        auto func = [channel]() {
            InputMethodAbility::GetInstance().OnClientInactive(channel);
        };
        actions_.emplace_back(std::make_unique<Action>(func));
    }
    ~TaskImsaOnClientInactive() = default;
};

class TaskImsaInitInputCtrlChannel : public Task {
public:
    explicit TaskImsaInitInputCtrlChannel(sptr<IRemoteObject> channel) : Task(TASK_TYPE_IMSA_INIT_INPUT_CTRL_CHANNEL)
    {
        auto func = [channel]() {
            InputMethodAbility::GetInstance().OnInitInputControlChannel(channel);
        };
        actions_.emplace_back(std::make_unique<Action>(func));
    }
    ~TaskImsaInitInputCtrlChannel() = default;
};

class TaskImsaOnCursorUpdate : public Task {
public:
    TaskImsaOnCursorUpdate(int32_t x, int32_t y, int32_t h) : Task(TASK_TYPE_IMSA_CURSOR_UPDATE)
    {
        auto func = [x, y, h]() {
            InputMethodAbility::GetInstance().OnCursorUpdate(x, y, h);
        };
        actions_.emplace_back(std::make_unique<Action>(func));
    }
    ~TaskImsaOnCursorUpdate() = default;
};

class TaskImsaSendPrivateCommand : public Task {
public:
    TaskImsaSendPrivateCommand(std::unordered_map<std::string, PrivateDataValue> privateCommand)
        : Task(TASK_TYPE_IMSA_SEND_PRIVATE_COMMAND)
    {
        auto func = [privateCommand]() {
            InputMethodAbility::GetInstance().ReceivePrivateCommand(privateCommand);
        };
        actions_.emplace_back(std::make_unique<Action>(func));
    }
    ~TaskImsaSendPrivateCommand() = default;
};

class TaskImsaOnSelectionChange : public Task {
public:
    TaskImsaOnSelectionChange(std::u16string text, int32_t oldBegin, int32_t oldEnd, int32_t newBegin, int32_t newEnd)
        : Task(TASK_TYPE_IMSA_SELECTION_CHANGE)
    {
        auto func = [text, oldBegin, oldEnd, newBegin, newEnd]() {
            InputMethodAbility::GetInstance().OnSelectionChange(text, oldBegin, oldEnd, newBegin, newEnd);
        };
        actions_.emplace_back(std::make_unique<Action>(func));
    }
    ~TaskImsaOnSelectionChange() = default;
};

class TaskImsaAttributeChange : public Task {
public:
    explicit TaskImsaAttributeChange(InputAttribute attr) : Task(TASK_TYPE_IMSA_ATTRIBUTE_CHANGE)
    {
        auto func = [attr]() {
            InputMethodAbility::GetInstance().OnAttributeChange(attr);
        };
        actions_.emplace_back(std::make_unique<Action>(func));
    }
    ~TaskImsaAttributeChange() = default;
};

class TaskImsaStopInputService : public Task {
public:
    explicit TaskImsaStopInputService(bool isTerminateIme) : Task(TASK_TYPE_IMSA_STOP_INPUT_SERVICE)
    {
        auto func = [isTerminateIme]() {
            InputMethodAbility::GetInstance().OnStopInputService(isTerminateIme);
        };
        actions_.emplace_back(std::make_unique<Action>(func));
    }
    ~TaskImsaStopInputService() = default;
};

class TaskImsaOnSetSubProperty : public Task {
public:
    explicit TaskImsaOnSetSubProperty(SubProperty prop) : Task(TASK_TYPE_IMSA_SET_SUBPROPERTY)
    {
        auto func = [prop]() {
            InputMethodAbility::GetInstance().OnSetSubtype(prop);
        };
        actions_.emplace_back(std::make_unique<Action>(func));
    }
    ~TaskImsaOnSetSubProperty() = default;
};

class TaskImsaSetCoreAndAgent : public Task {
public:
    TaskImsaSetCoreAndAgent() : Task(TASK_TYPE_IMSA_SET_CORE_AND_AGENT)
    {
        auto func = []() {
            InputMethodAbility::GetInstance().SetCoreAndAgent();
        };
        actions_.emplace_back(std::make_unique<Action>(func));
    }
    ~TaskImsaSetCoreAndAgent() = default;
};
} // namespace MiscServices
} // namespace OHOS

#endif // FRAMEWORKS_INPUTMETHOD_ABILITY_INCLUDE_TASKS_TASK_IMSA_H