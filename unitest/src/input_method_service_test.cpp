/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use  file except in compliance with the License.
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

#include <gtest/gtest.h>
#include <sys/time.h>

#include <cstdint>
#include <functional>
#include <string>
#include <thread>
#include <vector>

#include "global.h"
#include "im_common_event_manager.h"
#include "input_manager.h"
#include "input_method_controller.h"
#include "key_event.h"
#include "pointer_event.h"

namespace OHOS {
namespace MiscServices {
using namespace testing::ext;
using namespace MMI;
namespace {
constexpr int32_t TIME_WAIT_FOR_STATUS_OK = 2;
constexpr int32_t SEC_TO_NANOSEC = 1000000000;
constexpr int32_t NANOSECOND_TO_MILLISECOND = 1000000;
} // namespace

class InputMethodServiceTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    static std::shared_ptr<MMI::KeyEvent> SetKeyEvent(int32_t keyCode);
    static std::shared_ptr<MMI::KeyEvent> SetCombineKeyEvent(int32_t preKey, int32_t finalKey);
};

class TextListener : public OnTextChangedListener {
public:
    TextListener()
    {
    }
    ~TextListener()
    {
    }
    void InsertText(const std::u16string &text)
    {
        IMSA_HILOGI("IMC TEST TextListener InsertText");
    }

    void DeleteBackward(int32_t length)
    {
        IMSA_HILOGI("IMC TEST TextListener DeleteBackward length: %{public}d", length);
    }

    void SetKeyboardStatus(bool status)
    {
        IMSA_HILOGI("IMC TEST TextListener SetKeyboardStatus %{public}d", status);
    }
    void DeleteForward(int32_t length)
    {
        IMSA_HILOGI("IMC TEST TextListener DeleteForward length: %{public}d", length);
    }
    void SendKeyEventFromInputMethod(const KeyEvent &event)
    {
        IMSA_HILOGI("IMC TEST TextListener sendKeyEventFromInputMethod");
    }
    void SendKeyboardInfo(const KeyboardInfo &status)
    {
        IMSA_HILOGI("IMC TEST TextListener SendKeyboardInfo");
    }
    void MoveCursor(const Direction direction)
    {
        IMSA_HILOGI("IMC TEST TextListener MoveCursor");
    }
};

int64_t GetNanoTime()
{
    struct timespec time = { 0 };
    clock_gettime(CLOCK_MONOTONIC, &time);
    return static_cast<int64_t>(time.tv_sec) * SEC_TO_NANOSEC + time.tv_nsec;
}

void InputMethodServiceTest::SetUpTestCase(void)
{
    IMSA_HILOGI("InputMethodServiceTest::SetUpTestCase");
}

void InputMethodServiceTest::TearDownTestCase(void)
{
    IMSA_HILOGI("InputMethodServiceTest::TearDownTestCase");
}

void InputMethodServiceTest::SetUp(void)
{
    IMSA_HILOGI("InputMethodServiceTest::SetUp");
}

void InputMethodServiceTest::TearDown(void)
{
    IMSA_HILOGI("InputMethodServiceTest::TearDown");
}

std::shared_ptr<MMI::KeyEvent> InputMethodServiceTest::SetKeyEvent(int32_t keyCode)
{
    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
    int64_t downTime = GetNanoTime() / NANOSECOND_TO_MILLISECOND;
    MMI::KeyEvent::KeyItem keyItem;
    keyItem.SetKeyCode(keyCode);
    keyItem.SetPressed(true);
    keyItem.SetDownTime(downTime);
    keyEvent->SetKeyCode(keyCode);
    keyEvent->SetKeyAction(MMI::KeyEvent::KEY_ACTION_DOWN);
    keyEvent->AddPressedKeyItems(keyItem);

    return keyEvent;
}

std::shared_ptr<MMI::KeyEvent> InputMethodServiceTest::SetCombineKeyEvent(int32_t preKey, int32_t finalKey)
{
    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
    int64_t downTime = GetNanoTime() / NANOSECOND_TO_MILLISECOND;
    MMI::KeyEvent::KeyItem keyItem1;
    MMI::KeyEvent::KeyItem keyItem2;
    keyItem1.SetKeyCode(preKey);
    keyItem1.SetPressed(true);
    keyItem1.SetDownTime(downTime);
    keyItem2.SetKeyCode(finalKey);
    keyItem2.SetPressed(true);
    keyItem2.SetDownTime(downTime);

    keyEvent->SetKeyCode(finalKey);
    keyEvent->SetKeyCode(MMI::KeyEvent::KEY_ACTION_DOWN);
    keyEvent->AddPressedKeyItems(keyItem1);
    keyEvent->AddPressedKeyItems(keyItem2);

    return keyEvent;
}

/**
 * @tc.name: testtestKeyboardEventCallback001
 * @tc.desc: test KeyboardEvent Callback.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodServiceTest, testKeyboardEventCallback001, TestSize.Level0)
{
    IMSA_HILOGI("SubscribeKeyboardEvent001 TEST START");
    sptr<OnTextChangedListener> textListener = new TextListener();
    InputMethodController::GetInstance()->Attach(textListener);
    auto keyEvent = InputMethodServiceTest::SetKeyEvent(MMI::KeyEvent::KEYCODE_CAPS_LOCK);
    EXPECT_TRUE(keyEvent != nullptr);
    InputManager::GetInstance()->SimulateInputEvent(keyEvent);
    sleep(TIME_WAIT_FOR_STATUS_OK);
}

/**
 * @tc.name: testtestKeyboardEventCallback002
 * @tc.desc: test KeyboardEvent Callback.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodServiceTest, testKeyboardEventCallback002, TestSize.Level0)
{
    IMSA_HILOGI("SubscribeKeyboardEvent002 TEST START");
    sptr<OnTextChangedListener> textListener = new TextListener();
    InputMethodController::GetInstance()->Attach(textListener);
    auto keyEvent = InputMethodServiceTest::SetKeyEvent(MMI::KeyEvent::KEYCODE_SHIFT_LEFT);
    EXPECT_TRUE(keyEvent != nullptr);
    InputManager::GetInstance()->SimulateInputEvent(keyEvent);
    sleep(TIME_WAIT_FOR_STATUS_OK);
}

/**
 * @tc.name: testtestKeyboardEventCallback003
 * @tc.desc: test KeyboardEvent Callback.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodServiceTest, testKeyboardEventCallback003, TestSize.Level0)
{
    IMSA_HILOGI("SubscribeKeyboardEvent003 TEST START");
    sptr<OnTextChangedListener> textListener = new TextListener();
    InputMethodController::GetInstance()->Attach(textListener);
    auto keyEvent = InputMethodServiceTest::SetKeyEvent(MMI::KeyEvent::KEYCODE_SHIFT_RIGHT);
    EXPECT_TRUE(keyEvent != nullptr);
    InputManager::GetInstance()->SimulateInputEvent(keyEvent);
    sleep(TIME_WAIT_FOR_STATUS_OK);
}

/**
 * @tc.name: testtestKeyboardEventCallback004
 * @tc.desc: test KeyboardEvent Callback.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodServiceTest, testKeyboardEventCallback004, TestSize.Level0)
{
    IMSA_HILOGI("SubscribeKeyboardEvent004 TEST START");
    sptr<OnTextChangedListener> textListener = new TextListener();
    InputMethodController::GetInstance()->Attach(textListener);
    auto keyEvent = InputMethodServiceTest::SetCombineKeyEvent(
        MMI::KeyEvent::KEYCODE_CTRL_LEFT, MMI::KeyEvent::KEYCODE_SHIFT_LEFT);
    EXPECT_TRUE(keyEvent != nullptr);
    InputManager::GetInstance()->SimulateInputEvent(keyEvent);
    sleep(TIME_WAIT_FOR_STATUS_OK);
}

/**
 * @tc.name: testtestKeyboardEventCallback005
 * @tc.desc: test KeyboardEvent Callback.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodServiceTest, testKeyboardEventCallback005, TestSize.Level0)
{
    IMSA_HILOGI("SubscribeKeyboardEvent005 TEST START");
    sptr<OnTextChangedListener> textListener = new TextListener();
    InputMethodController::GetInstance()->Attach(textListener);
    auto keyEvent = InputMethodServiceTest::SetCombineKeyEvent(
        MMI::KeyEvent::KEYCODE_CTRL_LEFT, MMI::KeyEvent::KEYCODE_SHIFT_RIGHT);
    EXPECT_TRUE(keyEvent != nullptr);
    InputManager::GetInstance()->SimulateInputEvent(keyEvent);
    sleep(TIME_WAIT_FOR_STATUS_OK);
}

/**
 * @tc.name: testtestKeyboardEventCallback006
 * @tc.desc: test KeyboardEvent Callback.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodServiceTest, testKeyboardEventCallback006, TestSize.Level0)
{
    IMSA_HILOGI("SubscribeKeyboardEvent006 TEST START");
    sptr<OnTextChangedListener> textListener = new TextListener();
    InputMethodController::GetInstance()->Attach(textListener);
    auto keyEvent = InputMethodServiceTest::SetCombineKeyEvent(
        MMI::KeyEvent::KEYCODE_CTRL_RIGHT, MMI::KeyEvent::KEYCODE_SHIFT_LEFT);
    EXPECT_TRUE(keyEvent != nullptr);
    InputManager::GetInstance()->SimulateInputEvent(keyEvent);
    sleep(TIME_WAIT_FOR_STATUS_OK);
}

/**
 * @tc.name: testtestKeyboardEventCallback007
 * @tc.desc: test KeyboardEvent Callback.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodServiceTest, testKeyboardEventCallback007, TestSize.Level0)
{
    IMSA_HILOGI("SubscribeKeyboardEvent007 TEST START");
    sptr<OnTextChangedListener> textListener = new TextListener();
    InputMethodController::GetInstance()->Attach(textListener);
    auto keyEvent = InputMethodServiceTest::SetCombineKeyEvent(
        MMI::KeyEvent::KEYCODE_CTRL_RIGHT, MMI::KeyEvent::KEYCODE_SHIFT_RIGHT);
    EXPECT_TRUE(keyEvent != nullptr);
    InputManager::GetInstance()->SimulateInputEvent(keyEvent);
    sleep(TIME_WAIT_FOR_STATUS_OK);
}
} // namespace MiscServices
} // namespace OHOS