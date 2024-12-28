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

#include "input_event_callback.h"

#include <gtest/gtest.h>

#include <memory>

#include "mock/mock_key_event.h"
#include "mock/mock_key_handle.h"


using namespace testing;
using namespace OHOS::MiscServices;

class InputEventCallbackTest : public Test {
public:
    void SetUp() override
    {
        callback_ = std::make_shared<InputEventCallback>();
        keyEvent_ = std::make_shared<MMI::MockKeyEvent>();
        keyHandler_ = std::make_shared<MockKeyHandle>();
    }

    void TearDown() override
    {
        callback_ = nullptr;
        keyEvent_ = nullptr;
        keyHandler_ = nullptr;
    }

protected:
    std::shared_ptr<InputEventCallback> callback_;
    std::shared_ptr<MMI::MockKeyEvent> keyEvent_;
    std::shared_ptr<MockKeyHandle> keyHandler_;
};

/**
 * @tc.name: OnInputEvent_UnknownKeyCode_ResetKeyState
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputEventCallbackTest, OnInputEvent_UnknownKeyCode_ResetKeyState, TestSize.Level0)
{
    EXPECT_CALL(*keyEvent_, GetKeyCode()).WillOnce(Return(MMI::KeyEvent::KEYCODE_UNKNOWN));
    EXPECT_CALL(*keyEvent_, GetKeyAction()).WillOnce(Return(MMI::KeyEvent::KEY_ACTION_DOWN));

    callback_->OnInputEvent(keyEvent_);

    EXPECT_EQ(InputEventCallback::keyState_, 0);
}

/**
 * @tc.name: OnInputEvent_CapsLockDown_CallKeyHandler
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputEventCallbackTest, OnInputEvent_CapsLockDown_CallKeyHandler, TestSize.Level0)
{
    EXPECT_CALL(*keyEvent_, GetKeyCode()).WillOnce(Return(MMI::KeyEvent::KEYCODE_CAPS_LOCK));
    EXPECT_CALL(*keyEvent_, GetKeyAction()).WillOnce(Return(MMI::KeyEvent::KEY_ACTION_DOWN));
    EXPECT_CALL(*keyHandler_, operator()(_)).WillOnce(Return(0));

    callback_->SetKeyHandle(keyHandler_);
    callback_->OnInputEvent(keyEvent_);

    EXPECT_EQ(InputEventCallback::keyState_, KeyboardEvent::CAPS_MASK);
}

/**
 * @tc.name: OnInputEvent_KeyUpWithoutHandler_NoAction
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputEventCallbackTest, OnInputEvent_KeyUpWithoutHandler_NoAction, TestSize.Level0)
{
    EXPECT_CALL(*keyEvent_, GetKeyCode()).WillOnce(Return(MMI::KeyEvent::KEYCODE_CAPS_LOCK));
    EXPECT_CALL(*keyEvent_, GetKeyAction()).WillOnce(Return(MMI::KeyEvent::KEY_ACTION_UP));

    callback_->OnInputEvent(keyEvent_);

    EXPECT_EQ(InputEventCallback::keyState_, 0);
}