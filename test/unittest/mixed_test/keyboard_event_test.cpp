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

#include "keyboard_event.h"

#include <gtest/gtest.h>

#include <memory>
#include <set>

#include "input_event_callback.h"
#include "input_manager.h"
#include "inputmethod_sysevent.h"
#include "key_event.h"

using namespace OHOS::MiscServices;
using namespace OHOS::MMI;

class MockInputManager : public InputManager {
public:
    int32_t AddMonitor(MonitorCallback callback) override
    {
        if (addMonitorSuccess) {
            return 1;
        }
        return -1;
    }

    int32_t SubscribeKeyEvent(std::shared_ptr<KeyOption> keyOption, CombinationKeyCallBack callback) override
    {
        if (subscribeSuccess) {
            return 1;
        }
        return -1;
    }

    bool addMonitorSuccess = true;
    bool subscribeSuccess = true;
};

class MockInputMethodSysEvent : public InputMethodSysEvent {
public:
    void ReportSystemShortCut(const std::string &shortcut) override
    {
        reportedShortcuts.insert(shortcut);
    }

    std::set<std::string> reportedShortcuts;
};

class MockInputEventCallback : public InputEventCallback {
public:
    void OnInputEvent(std::shared_ptr<KeyEvent> keyEvent) override
    {
        onInputEventCalled = true;
    }

    void TriggerSwitch() override
    {
        triggerSwitchCalled = true;
    }

    bool onInputEventCalled = false;
    bool triggerSwitchCalled = false;
};

class KeyboardEventTest : public testing::Test {
protected:
    void SetUp() override
    {
        inputManager = std::make_shared<MockInputManager>();
        inputMethodSysEvent = std::make_shared<MockInputMethodSysEvent>();
        inputEventCallback = std::make_shared<MockInputEventCallback>();
    }

    void TearDown() override
    {
        inputManager.reset();
        inputMethodSysEvent.reset();
        inputEventCallback.reset();
    }

    std::shared_ptr<MockInputManager> inputManager;
    std::shared_ptr<MockInputMethodSysEvent> inputMethodSysEvent;
    std::shared_ptr<MockInputEventCallback> inputEventCallback;
};

/**
 * @tc.name: AddKeyEventMonitor_SuccessfulMonitorAddition
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(KeyboardEventTest, AddKeyEventMonitor_SuccessfulMonitorAddition, TestSize.Level0)
{
    inputManager->addMonitorSuccess = true;
    inputManager->subscribeSuccess = true;

    KeyHandle keyHandle = [inputEventCallback](
                              std::shared_ptr<KeyEvent> keyEvent) { inputEventCallback->OnInputEvent(keyEvent); };

    int32_t result = KeyboardEvent::GetInstance().AddKeyEventMonitor(keyHandle);

    EXPECT_EQ(result, ErrorCode::NO_ERROR);
    EXPECT_TRUE(inputMethodSysEvent->reportedShortcuts.find("usual.event.WIN_SPACE")
                != inputMethodSysEvent->reportedShortcuts.end());
    EXPECT_TRUE(inputMethodSysEvent->reportedShortcuts.find("usual.event.CTRL_SHIFT")
                != inputMethodSysEvent->reportedShortcuts.end());
}

/**
 * @tc.name: AddKeyEventMonitor_MonitorAdditionFailure
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(KeyboardEventTest, AddKeyEventMonitor_MonitorAdditionFailure, TestSize.Level0)
{
    inputManager->addMonitorSuccess = false;

    KeyHandle keyHandle = [inputEventCallback](
                              std::shared_ptr<KeyEvent> keyEvent) { inputEventCallback->OnInputEvent(keyEvent); };

    int32_t result = KeyboardEvent::GetInstance().AddKeyEventMonitor(keyHandle);

    EXPECT_EQ(result, ErrorCode::ERROR_SUBSCRIBE_KEYBOARD_EVENT);
}