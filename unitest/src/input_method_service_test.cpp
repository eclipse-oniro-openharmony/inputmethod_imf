/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
constexpr int32_t TIME_WAIT_FOR_STATUS_OK = 50;
constexpr int32_t TIME_WAIT_FOR_HANDLE_KEY_EVENT = 10000;
constexpr int32_t SEC_TO_NANOSEC = 1000000000;
constexpr int32_t NANOSECOND_TO_MILLISECOND = 1000000;
} // namespace

class InputMethodServiceTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    static std::shared_ptr<MMI::KeyEvent> SetKeyEvent(int32_t keyCode, int32_t keyAction);
    static bool SimulateKeyEvent(int32_t keyCode);
    static bool SimulateCombinationKeyEvent(int32_t preKey, int32_t finalKey);
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
        IMSA_HILOGI("SERVICE TEST TextListener InsertText");
    }

    void DeleteBackward(int32_t length)
    {
        IMSA_HILOGI("SERVICE TEST TextListener DeleteBackward length: %{public}d", length);
    }

    void SetKeyboardStatus(bool status)
    {
        IMSA_HILOGI("SERVICE TEST TextListener SetKeyboardStatus %{public}d", status);
    }
    void DeleteForward(int32_t length)
    {
        IMSA_HILOGI("SERVICE TEST TextListener DeleteForward length: %{public}d", length);
    }
    void SendKeyEventFromInputMethod(const KeyEvent &event)
    {
        IMSA_HILOGI("SERVICE TEST TextListener sendKeyEventFromInputMethod");
    }
    void SendKeyboardInfo(const KeyboardInfo &status)
    {
        IMSA_HILOGI("SERVICE TEST TextListener SendKeyboardInfo");
    }
    void MoveCursor(const Direction direction)
    {
        IMSA_HILOGI("SERVICE TEST TextListener MoveCursor");
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

std::shared_ptr<MMI::KeyEvent> InputMethodServiceTest::SetKeyEvent(int32_t keyCode, int32_t keyAction)
{
    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
    int64_t downTime = GetNanoTime() / NANOSECOND_TO_MILLISECOND;
    MMI::KeyEvent::KeyItem keyItem;
    keyItem.SetKeyCode(keyCode);
    keyItem.SetPressed(true);
    keyItem.SetDownTime(downTime);
    if (keyEvent != nullptr) {
        keyEvent->SetKeyCode(keyCode);
        keyEvent->SetKeyAction(keyAction);
        keyEvent->AddPressedKeyItems(keyItem);
    }
    return keyEvent;
}

bool InputMethodServiceTest::SimulateKeyEvent(int32_t keyCode)
{
    auto keyDown = InputMethodServiceTest::SetKeyEvent(keyCode, MMI::KeyEvent::KEY_ACTION_DOWN);
    auto keyUp = InputMethodServiceTest::SetKeyEvent(keyCode, MMI::KeyEvent::KEY_ACTION_UP);
    bool result = keyDown != nullptr && keyUp != nullptr;
    InputManager::GetInstance()->SimulateInputEvent(keyDown);
    usleep(TIME_WAIT_FOR_STATUS_OK);
    InputManager::GetInstance()->SimulateInputEvent(keyUp);
    return result;
}

bool InputMethodServiceTest::SimulateCombinationKeyEvent(int32_t preKey, int32_t finalKey)
{
    auto preDown = InputMethodServiceTest::SetKeyEvent(preKey, MMI::KeyEvent::KEY_ACTION_DOWN);
    auto finalDown = InputMethodServiceTest::SetKeyEvent(finalKey, MMI::KeyEvent::KEY_ACTION_DOWN);
    auto preUp = InputMethodServiceTest::SetKeyEvent(finalKey, MMI::KeyEvent::KEY_ACTION_UP);
    auto finalUp = InputMethodServiceTest::SetKeyEvent(preKey, MMI::KeyEvent::KEY_ACTION_UP);
    InputManager::GetInstance()->SimulateInputEvent(preDown);
    usleep(TIME_WAIT_FOR_STATUS_OK);
    InputManager::GetInstance()->SimulateInputEvent(finalDown);
    usleep(TIME_WAIT_FOR_STATUS_OK);
    InputManager::GetInstance()->SimulateInputEvent(preUp);
    usleep(TIME_WAIT_FOR_STATUS_OK);
    InputManager::GetInstance()->SimulateInputEvent(finalUp);
    bool result = preDown != nullptr && finalDown != nullptr && preUp != nullptr && finalUp != nullptr;
    return result;
}

/**
 * @tc.name: test_KeyEvent_UNKNOWN_001
 * @tc.desc: test KeyEvent Callback.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodServiceTest, test_KeyEvent_UNKNOWN_001, TestSize.Level0)
{
    IMSA_HILOGI("test_KeyEvent_UNKNOWN_001 TEST START");
    bool result = InputMethodServiceTest::SimulateKeyEvent(MMI::KeyEvent::KEYCODE_0);
    EXPECT_TRUE(result);
    usleep(TIME_WAIT_FOR_HANDLE_KEY_EVENT);
}

/**
 * @tc.name: test_KeyEvent_UNKNOWN_002
 * @tc.desc: test KeyEvent Callback.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodServiceTest, test_KeyEvent_UNKNOWN_002, TestSize.Level0)
{
    IMSA_HILOGI("test_KeyEvent_UNKNOWN_002 TEST START");
    bool result =
        InputMethodServiceTest::SimulateCombinationKeyEvent(MMI::KeyEvent::KEYCODE_0, MMI::KeyEvent::KEYCODE_1);
    EXPECT_TRUE(result);
    usleep(TIME_WAIT_FOR_HANDLE_KEY_EVENT);
}

/**
 * @tc.name: test_KeyEvent_CAPS_001
 * @tc.desc: test KeyEvent Callback.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodServiceTest, test_KeyEvent_CAPS_001, TestSize.Level0)
{
    IMSA_HILOGI("test_KeyEvent_CAPS_001 TEST START");
    bool result = InputMethodServiceTest::SimulateKeyEvent(MMI::KeyEvent::KEYCODE_CAPS_LOCK);
    EXPECT_TRUE(result);
    usleep(TIME_WAIT_FOR_HANDLE_KEY_EVENT);
}

/**
 * @tc.name: test_KeyEvent_CTRL_001
 * @tc.desc: test KeyEvent Callback.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodServiceTest, test_KeyEvent_CTRL_001, TestSize.Level0)
{
    IMSA_HILOGI("test_KeyEvent_CTRL_001 TEST START");
    bool result = InputMethodServiceTest::SimulateKeyEvent(MMI::KeyEvent::KEYCODE_CTRL_LEFT);
    EXPECT_TRUE(result);
    usleep(TIME_WAIT_FOR_HANDLE_KEY_EVENT);
}

/**
 * @tc.name: test_KeyEvent_CTRL_002
 * @tc.desc: test KeyEvent Callback.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodServiceTest, test_KeyEvent_CTRL_002, TestSize.Level0)
{
    IMSA_HILOGI("test_KeyEvent_CTRL_002 TEST START");
    bool result = InputMethodServiceTest::SimulateKeyEvent(MMI::KeyEvent::KEYCODE_CTRL_RIGHT);
    EXPECT_TRUE(result);
    usleep(TIME_WAIT_FOR_HANDLE_KEY_EVENT);
}

/**
 * @tc.name: test_KeyEvent_SHIFT_001
 * @tc.desc: test KeyEvent Callback.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodServiceTest, test_KeyEvent_SHIFT_001, TestSize.Level0)
{
    IMSA_HILOGI("test_KeyEvent_SHIFT_001 TEST START");
    bool result = InputMethodServiceTest::SimulateKeyEvent(MMI::KeyEvent::KEYCODE_SHIFT_LEFT);
    EXPECT_TRUE(result);
    usleep(TIME_WAIT_FOR_HANDLE_KEY_EVENT);
}

/**
 * @tc.name: test_KeyEvent_SHIFT_002
 * @tc.desc: test KeyEvent Callback.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodServiceTest, test_KeyEvent_SHIFT_002, TestSize.Level0)
{
    IMSA_HILOGI("test_KeyEvent_SHIFT_002 TEST START");
    bool result = InputMethodServiceTest::SimulateKeyEvent(MMI::KeyEvent::KEYCODE_SHIFT_RIGHT);
    EXPECT_TRUE(result);
    usleep(TIME_WAIT_FOR_HANDLE_KEY_EVENT);
}

/**
 * @tc.name: test_KeyEvent_CTRL_SHIFT_001
 * @tc.desc: test KeyEvent Callback.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodServiceTest, test_KeyEvent_CTRL_SHIFT_001, TestSize.Level0)
{
    IMSA_HILOGI("test_KeyEvent_CTRL_SHIFT_001 TEST START");
    bool result = InputMethodServiceTest::SimulateCombinationKeyEvent(
        MMI::KeyEvent::KEYCODE_CTRL_LEFT, MMI::KeyEvent::KEYCODE_SHIFT_LEFT);
    EXPECT_TRUE(result);
    usleep(TIME_WAIT_FOR_HANDLE_KEY_EVENT);
}

/**
 * @tc.name: test_KeyEvent_CTRL_SHIFT_002
 * @tc.desc: test KeyEvent Callback.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodServiceTest, test_KeyEvent_CTRL_SHIFT_002, TestSize.Level0)
{
    IMSA_HILOGI("test_KeyEvent_CTRL_SHIFT_002 TEST START");
    bool result = InputMethodServiceTest::SimulateCombinationKeyEvent(
        MMI::KeyEvent::KEYCODE_CTRL_LEFT, MMI::KeyEvent::KEYCODE_SHIFT_RIGHT);
    EXPECT_TRUE(result);
    usleep(TIME_WAIT_FOR_HANDLE_KEY_EVENT);
}

/**
 * @tc.name: test_KeyEvent_CTRL_SHIFT_003
 * @tc.desc: test KeyEvent Callback.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodServiceTest, test_KeyEvent_CTRL_SHIFT_003, TestSize.Level0)
{
    IMSA_HILOGI("test_KeyEvent_CTRL_SHIFT_003 TEST START");
    bool result = InputMethodServiceTest::SimulateCombinationKeyEvent(
        MMI::KeyEvent::KEYCODE_CTRL_RIGHT, MMI::KeyEvent::KEYCODE_SHIFT_LEFT);
    EXPECT_TRUE(result);
    usleep(TIME_WAIT_FOR_HANDLE_KEY_EVENT);
}

/**
 * @tc.name: test_KeyEvent_CTRL_SHIFT_004
 * @tc.desc: test KeyEvent Callback.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodServiceTest, test_KeyEvent_CTRL_SHIFT_004, TestSize.Level0)
{
    IMSA_HILOGI("SubscribeKeyboardEvent007 TEST START");
    bool result = InputMethodServiceTest::SimulateCombinationKeyEvent(
        MMI::KeyEvent::KEYCODE_CTRL_RIGHT, MMI::KeyEvent::KEYCODE_SHIFT_RIGHT);
    EXPECT_TRUE(result);
}
} // namespace MiscServices
} // namespace OHOS