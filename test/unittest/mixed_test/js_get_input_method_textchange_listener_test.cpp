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

#include "js_get_input_method_textchange_listener.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <mutex>

#include "js_get_input_method_controller.h"


using namespace OHOS::MiscServices;
using namespace testing;

class MockJsGetInputMethodController : public JsGetInputMethodController {
public:
    MOCK_METHOD1(InsertText, void(const std::u16string &text));
    MOCK_METHOD1(DeleteRight, void(int32_t length));
    MOCK_METHOD1(DeleteLeft, void(int32_t length));
    MOCK_METHOD1(SendKeyboardStatus, void(const KeyboardStatus &status));
    MOCK_METHOD1(SendFunctionKey, void(const FunctionKey &functionKey));
    MOCK_METHOD1(MoveCursor, void(const Direction direction));
    MOCK_METHOD1(HandleExtendAction, void(int32_t action));
    MOCK_METHOD2(GetText, std::u16string(const std::string &method, int32_t number));
    MOCK_METHOD0(GetTextIndexAtCursor, int32_t());
};

class JsGetInputMethodTextChangedListenerTest : public Test {
protected:
    void SetUp() override
    {
        // 为测试设置模拟对象
        mockController_ = std::make_shared<MockJsGetInputMethodController>();
        ON_CALL(*mockController_, InsertText(_)).WillByDefault(Invoke([](const std::u16string &text) {}));
        ON_CALL(*mockController_, DeleteRight(_)).WillByDefault(Invoke([](int32_t length) {}));
        ON_CALL(*mockController_, DeleteLeft(_)).WillByDefault(Invoke([](int32_t length) {}));
        ON_CALL(*mockController_, SendKeyboardStatus(_)).WillByDefault(Invoke([](const KeyboardStatus &status) {}));
        ON_CALL(*mockController_, SendFunctionKey(_)).WillByDefault(Invoke([](const FunctionKey &functionKey) {}));
        ON_CALL(*mockController_, MoveCursor(_)).WillByDefault(Invoke([](const Direction direction) {}));
        ON_CALL(*mockController_, HandleExtendAction(_)).WillByDefault(Invoke([](int32_t action) {}));
        ON_CALL(*mockController_, GetText(_, _)).WillByDefault(Invoke([](const std::string &method, int32_t number) {
            return std::u16string();
        }));
        ON_CALL(*mockController_, GetTextIndexAtCursor()).WillByDefault(Invoke([]() { return 0; }));

        // 将模拟对象注入到单例中
        JsGetInputMethodTextChangedListener::GetInstance()->SetController(mockController_);
    }

    std::shared_ptr<MockJsGetInputMethodController> mockController_;
};

TEST_F(JsGetInputMethodTextChangedListenerTest, GetInstance_ShouldReturnSameInstance)
{
    auto instance1 = JsGetInputMethodTextChangedListener::GetInstance();
    auto instance2 = JsGetInputMethodTextChangedListener::GetInstance();
    EXPECT_EQ(instance1, instance2);
}

TEST_F(JsGetInputMethodTextChangedListenerTest, InsertText_ShouldDelegateToController)
{
    std::u16string text = u"test";
    JsGetInputMethodTextChangedListener::GetInstance()->InsertText(text);
    EXPECT_CALL(*mockController_, InsertText(text)).Times(1);
}

TEST_F(JsGetInputMethodTextChangedListenerTest, DeleteForward_ShouldDelegateToController)
{
    int32_t length = 5;
    JsGetInputMethodTextChangedListener::GetInstance()->DeleteForward(length);
    EXPECT_CALL(*mockController_, DeleteRight(length)).Times(1);
}

TEST_F(JsGetInputMethodTextChangedListenerTest, DeleteBackward_ShouldDelegateToController)
{
    int32_t length = 5;
    JsGetInputMethodTextChangedListener::GetInstance()->DeleteBackward(length);
    EXPECT_CALL(*mockController_, DeleteLeft(length)).Times(1);
}

TEST_F(JsGetInputMethodTextChangedListenerTest, SendKeyboardStatus_ShouldDelegateToController)
{
    KeyboardStatus status;
    JsGetInputMethodTextChangedListener::GetInstance()->SendKeyboardStatus(status);
    EXPECT_CALL(*mockController_, SendKeyboardStatus(status)).Times(1);
}

TEST_F(JsGetInputMethodTextChangedListenerTest, SendFunctionKey_ShouldDelegateToController)
{
    FunctionKey functionKey;
    JsGetInputMethodTextChangedListener::GetInstance()->SendFunctionKey(functionKey);
    EXPECT_CALL(*mockController_, SendFunctionKey(functionKey)).Times(1);
}

TEST_F(JsGetInputMethodTextChangedListenerTest, MoveCursor_ShouldDelegateToController)
{
    Direction direction = Direction::FORWARD;
    JsGetInputMethodTextChangedListener::GetInstance()->MoveCursor(direction);
    EXPECT_CALL(*mockController_, MoveCursor(direction)).Times(1);
}

TEST_F(JsGetInputMethodTextChangedListenerTest, HandleExtendAction_ShouldDelegateToController)
{
    int32_t action = 10;
    JsGetInputMethodTextChangedListener::GetInstance()->HandleExtendAction(action);
    EXPECT_CALL(*mockController_, HandleExtendAction(action)).Times(1);
}

TEST_F(JsGetInputMethodTextChangedListenerTest, GetLeftTextOfCursor_ShouldDelegateToController)
{
    int32_t number = 3;
    JsGetInputMethodTextChangedListener::GetInstance()->GetLeftTextOfCursor(number);
    EXPECT_CALL(*mockController_, GetText("getLeftTextOfCursor", number)).Times(1);
}

TEST_F(JsGetInputMethodTextChangedListenerTest, GetRightTextOfCursor_ShouldDelegateToController)
{
    int32_t number = 3;
    JsGetInputMethodTextChangedListener::GetInstance()->GetRightTextOfCursor(number);
    EXPECT_CALL(*mockController_, GetText("getRightTextOfCursor", number)).Times(1);
}

TEST_F(JsGetInputMethodTextChangedListenerTest, GetTextIndexAtCursor_ShouldDelegateToController)
{
    JsGetInputMethodTextChangedListener::GetInstance()->GetTextIndexAtCursor();
    EXPECT_CALL(*mockController_, GetTextIndexAtCursor()).Times(1);
}

TEST_F(JsGetInputMethodTextChangedListenerTest, ReceivePrivateCommand_ShouldReturnNoError)
{
    std::unordered_map<std::string, PrivateDataValue> privateCommand;
    int32_t result = JsGetInputMethodTextChangedListener::GetInstance()->ReceivePrivateCommand(privateCommand);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(JsGetInputMethodTextChangedListenerTest, IsFromTs_ShouldReturnTrue)
{
    bool result = JsGetInputMethodTextChangedListener::GetInstance()->IsFromTs();
    EXPECT_TRUE(result);
}