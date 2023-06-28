/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
#include "tdd_util.h"

namespace OHOS {
namespace MiscServices {
using namespace testing::ext;
using namespace MMI;
namespace {
constexpr int32_t TIME_WAIT_FOR_HANDLE_KEY_EVENT = 10000;
} // namespace

class InputMethodServiceTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
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
    void SendKeyboardStatus(const KeyboardStatus &status)
    {
        IMSA_HILOGI("SERVICE TEST TextListener SendKeyboardStatus");
    }
    void SendFunctionKey(const FunctionKey &functionKey)
    {
        IMSA_HILOGI("SERVICE TEST TextListener SendFunctionKey");
    }
    void MoveCursor(const Direction direction)
    {
        IMSA_HILOGI("SERVICE TEST TextListener MoveCursor");
    }
};

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

/**
 * @tc.name: test_KeyEvent_UNKNOWN_001
 * @tc.desc: test KeyEvent Callback.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodServiceTest, test_KeyEvent_UNKNOWN_001, TestSize.Level0)
{
    IMSA_HILOGI("test_KeyEvent_UNKNOWN_001 TEST START");
    bool result = TddUtil::SimulateKeyEvent(MMI::KeyEvent::KEYCODE_0);
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
    bool result = TddUtil::SimulateKeyEvents({ MMI::KeyEvent::KEYCODE_0, MMI::KeyEvent::KEYCODE_1 });
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
    bool result = TddUtil::SimulateKeyEvent(MMI::KeyEvent::KEYCODE_CAPS_LOCK);
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
    bool result = TddUtil::SimulateKeyEvent(MMI::KeyEvent::KEYCODE_CTRL_LEFT);
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
    bool result = TddUtil::SimulateKeyEvent(MMI::KeyEvent::KEYCODE_CTRL_RIGHT);
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
    bool result = TddUtil::SimulateKeyEvent(MMI::KeyEvent::KEYCODE_SHIFT_LEFT);
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
    bool result = TddUtil::SimulateKeyEvent(MMI::KeyEvent::KEYCODE_SHIFT_RIGHT);
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
    bool result = TddUtil::SimulateKeyEvents({ MMI::KeyEvent::KEYCODE_CTRL_LEFT, MMI::KeyEvent::KEYCODE_SHIFT_LEFT });
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
    bool result = TddUtil::SimulateKeyEvents({ MMI::KeyEvent::KEYCODE_CTRL_LEFT, MMI::KeyEvent::KEYCODE_SHIFT_RIGHT });
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
    bool result = TddUtil::SimulateKeyEvents({ MMI::KeyEvent::KEYCODE_CTRL_RIGHT, MMI::KeyEvent::KEYCODE_SHIFT_LEFT });
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
    bool result = TddUtil::SimulateKeyEvents({ MMI::KeyEvent::KEYCODE_CTRL_RIGHT, MMI::KeyEvent::KEYCODE_SHIFT_RIGHT });
    EXPECT_TRUE(result);
}
} // namespace MiscServices
} // namespace OHOS