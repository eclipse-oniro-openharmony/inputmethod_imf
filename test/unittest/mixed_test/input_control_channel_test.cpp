/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
#include "input_control_channel.h"

#include <gtest/gtest.h>

using namespace OHOS::MiscServices;

class InputControlChannelTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        // Set up the test environment
    }

    void TearDown() override
    {
        // Clean up the test environment
    }
};

/**
 * @tc.name: SendKeyEvent_ValidKeyEvent_ReturnsTrue
 * @tc.desc: Verify that the InputControlChannel::SendKeyEvent method returns true when a valid key event is sent
 * @tc.type: FUNC
 */
TEST_F(InputControlChannelTest, SendKeyEvent_ValidKeyEvent_ReturnsTrue)
{
    KeyEvent keyEvent;
    keyEvent.keyCode = 100;
    keyEvent.action = KeyEvent::KEY_ACTION_DOWN;

    bool result = InputControlChannel::GetInstance().SendKeyEvent(keyEvent);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: SendKeyEvent_InvalidKeyEvent_ReturnsFalse
 * @tc.desc: Verify that the InputControlChannel::SendKeyEvent method
 * returns false when an invalid key event is sent
 * @tc.type: FUNC
 */
TEST_F(InputControlChannelTest, SendKeyEvent_InvalidKeyEvent_ReturnsFalse)
{
    KeyEvent keyEvent;
    keyEvent.keyCode = -1; // Invalid key code
    keyEvent.action = KeyEvent::KEY_ACTION_DOWN;

    bool result = InputControlChannel::GetInstance().SendKeyEvent(keyEvent);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: SendMouseEvent_ValidMouseEvent_ReturnsTrue
 * @tc.desc: Verify that the InputControlChannel::SendMouseEvent method
 * returns true when a valid mouse event is sent
 * @tc.type: FUNC
 */
TEST_F(InputControlChannelTest, SendMouseEvent_ValidMouseEvent_ReturnsTrue)
{
    MouseEvent mouseEvent;
    mouseEvent.x = 100;
    mouseEvent.y = 200;
    mouseEvent.action = MouseEvent::MOUSE_ACTION_DOWN;

    bool result = InputControlChannel::GetInstance().SendMouseEvent(mouseEvent);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: SendMouseEvent_InvalidMouseEvent_ReturnsFalse
 * @tc.desc: Verify that the InputControlChannel::SendMouseEvent method
 * returns false when an invalid mouse event is sent
 * @tc.type: FUNC
 */
TEST_F(InputControlChannelTest, SendMouseEvent_InvalidMouseEvent_ReturnsFalse)
{
    MouseEvent mouseEvent;
    mouseEvent.x = -1; // Invalid x coordinate
    mouseEvent.y = -1; // Invalid y coordinate
    mouseEvent.action = MouseEvent::MOUSE_ACTION_DOWN;

    bool result = InputControlChannel::GetInstance().SendMouseEvent(mouseEvent);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: RegisterInputListener_ValidListener_ReturnsTrue
 * @tc.desc: Verify that the InputControlChannel::RegisterInputListener method
 * returns true when a valid listener is registered
 * @tc.type: FUNC
 */
TEST_F(InputControlChannelTest, RegisterInputListener_ValidListener_ReturnsTrue)
{
    std::shared_ptr<InputListener> listener = std::make_shared<InputListener>();

    bool result = InputControlChannel::GetInstance().RegisterInputListener(listener);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: RegisterInputListener_InvalidListener_ReturnsFalse
 * @tc.desc: Verify that the InputControlChannel::RegisterInputListener method
 * returns false when an invalid listener is registered
 * @tc.type: FUNC
 */
TEST_F(InputControlChannelTest, RegisterInputListener_InvalidListener_ReturnsFalse)
{
    std::shared_ptr<InputListener> listener = nullptr;

    bool result = InputControlChannel::GetInstance().RegisterInputListener(listener);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: UnregisterInputListener_ValidListener_ReturnsTrue
 * @tc.desc: Verify that the InputControlChannel::UnregisterInputListener method
 * returns true when a valid listener is unregistered
 * @tc.type: FUNC
 */
TEST_F(InputControlChannelTest, UnregisterInputListener_ValidListener_ReturnsTrue)
{
    std::shared_ptr<InputListener> listener = std::make_shared<InputListener>();
    InputControlChannel::GetInstance().RegisterInputListener(listener);

    bool result = InputControlChannel::GetInstance().UnregisterInputListener(listener);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: UnregisterInputListener_InvalidListener_ReturnsFalse
 * @tc.desc: Verify that the InputControlChannel::UnregisterInputListener method
 * returns false when an invalid listener is unregistered
 * @tc.type: FUNC
 */
TEST_F(InputControlChannelTest, UnregisterInputListener_InvalidListener_ReturnsFalse)
{
    std::shared_ptr<InputListener> listener = nullptr;

    bool result = InputControlChannel::GetInstance().UnregisterInputListener(listener);
    EXPECT_FALSE(result);
}