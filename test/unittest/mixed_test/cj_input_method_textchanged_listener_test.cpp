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

#include "CjInputMethodController.h"
#include "CjInputMethodTextChangedListener.h"
#include "CjInputMethodTextChangedListener.h" // 假设这是包含被测方法的头文件
#include "ErrorCode.h"
#include "PrivateDataValue.h" // 假设这是包含 PrivateDataValue 的头文件
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

using namespace testing;
// 假设 CjInputMethodTextChangedListener 是一个类，且我们有访问其私有成员的权限
// 如果没有访问权限，可能需要通过公共方法或友元类来访问私有成员

// Mock class for CjInputMethodController
class MockCjInputMethodController : public CjInputMethodController {
public:
    MOCK_METHOD1(InsertText, void(const std::u16string &text));
    MOCK_METHOD1(DeleteRight, void(int32_t length));
    MOCK_METHOD1(DeleteLeft, void(int32_t length));
    MOCK_METHOD1(SendKeyboardStatus, void(const KeyboardStatus &status));
    MOCK_METHOD1(SendFunctionKey, void(const FunctionKey &functionKey));
    MOCK_METHOD1(HandleExtendAction, void(int32_t action));
    MOCK_METHOD1(GetLeftText, std::u16string(int32_t));
    MOCK_METHOD1(GetRightText, u16string(int32_t));
    MOCK_METHOD0(GetTextIndexAtCursor, int32_t());
};

// Test fixture
class CjInputMethodTextChangedListenerTest : public Test {
protected:
    void SetUp() override
    {
        // Mock the singleton instance of CjInputMethodController
        ON_CALL(*mockController, InsertText(_)).WillByDefault(Invoke([](const std::u16string &text) {
            // Do nothing, just simulate the method call
        }));
        // Use the mock instance for the singleton
        ON_CALL(*mockController, GetInstance()).WillByDefault(Return(mockController));
        ON_CALL(*mockController, GetRightText(_)).WillByDefault(Return(u""));

        listener = new CjInputMethodTextChangedListener();
        CjInputMethodController::SetInstance(mockController);
    }

    void TearDown() override
    {
        delete listener;
        listener = nullptr;
        CjInputMethodController::SetInstance(nullptr);
    }
    CjInputMethodTextChangedListener *listener;

    std::unique_ptr<MockCjInputMethodController> mockController = std::make_unique<MockCjInputMethodController>();
};

// 测试单例实例的创建
TEST(CjInputMethodTextChangedListenerTest, GetInstance_SingleThread_ReturnsSameInstance)
{
    sptr<CjInputMethodTextChangedListener> instance1 = CjInputMethodTextChangedListener::GetInstance();
    sptr<CjInputMethodTextChangedListener> instance2 = CjInputMethodTextChangedListener::GetInstance();
    EXPECT_EQ(instance1, instance2);
}

// 测试并发环境下的单例实例创建
TEST(CjInputMethodTextChangedListenerTest, GetInstance_MultiThread_ReturnsSameInstance)
{
    std::vector<std::thread> threads;
    std::vector<sptr<CjInputMethodTextChangedListener>> instances;

    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&instances]() {
            instances.push_back(CjInputMethodTextChangedListener::GetInstance());
        });
    }

    for (auto &thread : threads) {
        thread.join();
    }

    for (size_t i = 1; i < instances.size(); ++i) {
        EXPECT_EQ(instances[0], instances[i]);
    }
}

TEST_F(CjInputMethodTextChangedListenerTest, InsertText_ShouldCallInsertTextOnController)
{
    // Arrange
    CjInputMethodTextChangedListener listener;
    std::u16string text = u"Hello, World!";

    // Act
    listener.InsertText(text);

    // Assert
    EXPECT_CALL(*mockController, InsertText(text)).Times(1);
}

TEST_F(CjInputMethodTextChangedListenerTest, DeleteForward_CallsDeleteRightWithCorrectLength)
{
    CjInputMethodTextChangedListener listener;
    int32_t length = 5;

    // 预期：DeleteRight 方法被调用，并且传递了正确的长度
    EXPECT_CALL(mockController, DeleteRight(length));

    // 调用被测试的方法
    listener.DeleteForward(length);
}

TEST_F(CjInputMethodTextChangedListenerTest, DeleteBackward_ValidLength_DeletesCorrectly)
{
    CjInputMethodTextChangedListener listener;
    int32_t length = 5;

    EXPECT_CALL(*mockController, DeleteLeft(length)).Times(1);

    listener.DeleteBackward(length);
}

TEST_F(CjInputMethodTextChangedListenerTest, DeleteBackward_ZeroLength_NoDeletion)
{
    CjInputMethodTextChangedListener listener;
    int32_t length = 0;

    EXPECT_CALL(*mockController, DeleteLeft(length)).Times(1);

    listener.DeleteBackward(length);
}

TEST_F(CjInputMethodTextChangedListenerTest, DeleteBackward_NegativeLength_NoDeletion)
{
    CjInputMethodTextChangedListener listener;
    int32_t length = -3;

    EXPECT_CALL(*mockController, DeleteLeft(length)).Times(1);

    listener.DeleteBackward(length);
}

TEST(CjInputMethodTextChangedListenerTest, SendKeyboardStatus_CallsController)
{
    CjInputMethodTextChangedListener listener;
    KeyboardStatus status;
    status.status = KeyboardStatus::Status::VISIBLE;

    EXPECT_CALL(*mockController, SendKeyboardStatus(status));

    listener.SendKeyboardStatus(status);
}

TEST(CjInputMethodTextChangedListenerTest, SendFunctionKey_ValidFunctionKey_CallsSendFunctionKey)
{
    // Arrange
    MockCjInputMethodController *mockController = new MockCjInputMethodController();
    EXPECT_CALL(*mockController, SendFunctionKey(_)).Times(1);

    // Mock the singleton instance
    CjInputMethodController::SetInstance(mockController);

    CjInputMethodTextChangedListener listener;
    FunctionKey functionKey; // Assume FunctionKey is default constructible

    // Act
    listener.SendFunctionKey(functionKey);

    // Assert
    // Verification is handled by the EXPECT_CALL
}

// Test case for MoveCursor method
TEST_F(CjInputMethodTextChangedListenerTest, MoveCursor_ShouldCallControllerMoveCursor)
{
    CjInputMethodTextChangedListener listener;

    // Test with each direction
    for (int i = static_cast<int>(Direction::Left); i <= static_cast<int>(Direction::Down); ++i) {
        Direction direction = static_cast<Direction>(i);
        EXPECT_CALL(*mockController, MoveCursor(direction)).Times(1);
        listener.MoveCursor(direction);
    }
}

TEST(CjInputMethodTextChangedListenerTest, HandleExtendAction_ValidAction_CallsController)
{
    // 设置
    int action = 123;
    MockCjInputMethodController *mockController = CreateMockCjInputMethodController();
    EXPECT_CALL(*mockController, HandleExtendAction(action)).Times(1);

    // 模拟 CjInputMethodController::GetInstance() 返回 mockController
    CjInputMethodController::SetInstance(mockController);

    CjInputMethodTextChangedListener listener;
    listener.HandleExtendAction(action);
}

TEST(CjInputMethodTextChangedListenerTest, HandleExtendAction_InvalidAction_CallsController)
{
    // 设置
    MockCjInputMethodController *mockController = CreateMockCjInputMethodController();
    EXPECT_CALL(*mockController, HandleExtendAction(-1)).Times(1);

    // 模拟 CjInputMethodController::GetInstance() 返回 mockController
    CjInputMethodController::SetInstance(mockController);

    CjInputMethodTextChangedListener listener;
    listener.HandleExtendAction(-1);
}

TEST_F(CjInputMethodTextChangedListenerTest, GetLeftTextOfCursor_NormalCase)
{
    auto text = u"abc";
    int index = 3;
    EXPECT_CALL(*mockController, GetLeftText(index)).WillOnce(Return(text));
    EXPECT_EQ(listener->GetLeftTextOfCursor(index), text);
}

TEST_F(CjInputMethodTextChangedListenerTest, GetLeftTextOfCursor_BeyondTextLength)
{
    auto text = u"shorttext";
    int index = 10;
    EXPECT_CALL(*mockController, GetLeftText(index)).WillOnce(Return(text));
    EXPECT_EQ(listener->GetLeftTextOfCursor(index), text);
}

TEST_F(CjInputMethodTextChangedListenerTest, GetLeftTextOfCursor_ZeroOrNegativeNumber)
{
    auto text = u"";
    int index = 0;
    EXPECT_CALL(*mockController, GetLeftText(index)).WillOnce(Return(text));
    EXPECT_EQ(listener->GetLeftTextOfCursor(index), text);
    index = -1;
    EXPECT_CALL(*mockController, GetLeftText(index)).WillOnce(Return(text));
    EXPECT_EQ(listener->GetLeftTextOfCursor(index), text);
}

TEST_F(CjInputMethodTextChangedListenerTest, GetLeftTextOfCursor_EmptyText)
{
    auto text = u"";
    int index = 3;
    EXPECT_CALL(*mockController, GetLeftText(index)).WillOnce(Return(text));
    EXPECT_EQ(listener->GetLeftTextOfCursor(index), text);
}

TEST_F(CjInputMethodTextChangedListenerTest, GetLeftTextOfCursor_DifferentCursorPositions)
{
    auto text = u"xy";
    int index = 2;
    EXPECT_CALL(*mockController, GetLeftText(index)).WillOnce(Return(text));
    EXPECT_EQ(listener->GetLeftTextOfCursor(index), text);
}

TEST_F(CjInputMethodTextChangedListenerTest, GetRightTextOfCursor_NormalCase)
{
    auto text = u"abc";
    int index = 3;
    EXPECT_CALL(*mockController, GetRightText(index)).WillOnce(Return(text));
    EXPECT_EQ(listener->GetRightTextOfCursor(index), text);
}

TEST_F(CjInputMethodTextChangedListenerTest, GetRightTextOfCursor_PositiveNumber_ReturnsCorrectText)
{
    auto text = u"rightText";
    int index = 5;
    EXPECT_CALL(*mockController, GetRightText(index)).WillOnce(Return(text));
    CjInputMethodTextChangedListener listener;
    u16string result = listener.GetRightTextOfCursor(index);
    EXPECT_EQ(result, text);
}

TEST_F(CjInputMethodTextChangedListenerTest, GetRightTextOfCursor_Zero_ReturnsEmptyString)
{
    auto text = u"";
    int index = 0;
    EXPECT_CALL(*mockController, GetRightText(index)).WillOnce(Return(text));
    CjInputMethodTextChangedListener listener;
    u16string result = listener.GetRightTextOfCursor(index);
    EXPECT_EQ(result, text);
}

TEST_F(CjInputMethodTextChangedListenerTest, GetRightTextOfCursor_NegativeNumber_ReturnsEmptyString)
{
    auto text = u"";
    int index = -3;
    EXPECT_CALL(*mockController, GetRightText(index)).WillOnce(Return(text));
    CjInputMethodTextChangedListener listener;
    u16string result = listener.GetRightTextOfCursor(index);
    EXPECT_EQ(result, text);
}

TEST_F(CjInputMethodTextChangedListenerTest, GetRightTextOfCursor_NumberGreaterThanTextLength_ReturnsPartialText)
{
    auto text = u"partialText";
    int index = 10;
    EXPECT_CALL(*mockController, GetRightText(index)).WillOnce(Return(text));
    CjInputMethodTextChangedListener listener;
    u16string result = listener.GetRightTextOfCursor(index);
    EXPECT_EQ(result, text);
}

TEST_F(CjInputMethodTextChangedListenerTest, GetTextIndexAtCursor_NormalCase_ReturnsExpectedIndex)
{
    MockCjInputMethodController *mockController =
        static_cast<MockCjInputMethodController *>(CjInputMethodController::GetInstance());
    int cursorIndex = 5;
    EXPECT_CALL(*mockController, GetTextIndexAtCursor()).WillOnce(Return(cursorIndex));

    CjInputMethodTextChangedListener listener;
    int32_t index = listener.GetTextIndexAtCursor();

    EXPECT_EQ(index, cursorIndex);
}

TEST_F(CjInputMethodTextChangedListenerTest, GetTextIndexAtCursor_CursorMoved_ReturnsUpdatedIndex)
{
    MockCjInputMethodController *mockController =
        static_cast<MockCjInputMethodController *>(CjInputMethodController::GetInstance());
    int cursorIndex = 10;
    EXPECT_CALL(*mockController, GetTextIndexAtCursor()).WillOnce(Return(cursorIndex));

    CjInputMethodTextChangedListener listener;
    int32_t index = listener.GetTextIndexAtCursor();

    EXPECT_EQ(index, cursorIndex);
}

TEST_F(CjInputMethodTextChangedListenerTest, GetTextIndexAtCursor_NoCursorSet_ReturnsDefaultIndex)
{
    MockCjInputMethodController *mockController =
        static_cast<MockCjInputMethodController *>(CjInputMethodController::GetInstance());
    EXPECT_CALL(*mockController, GetTextIndexAtCursor()).WillOnce(Return(0));

    CjInputMethodTextChangedListener listener;
    int32_t index = listener.GetTextIndexAtCursor();

    EXPECT_EQ(index, 0);
}

TEST_F(CjInputMethodTextChangedListenerTest, ReceivePrivateCommand_EmptyCommand_ReturnsNoError)
{
    std::unordered_map<std::string, PrivateDataValue> privateCommand;
    int32_t result = listener.ReceivePrivateCommand(privateCommand);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(CjInputMethodTextChangedListenerTest, ReceivePrivateCommand_NonEmptyCommand_ReturnsNoError)
{
    std::unordered_map<std::string, PrivateDataValue> privateCommand;
    privateCommand["key"] = PrivateDataValue("value");
    int32_t result = listener.ReceivePrivateCommand(privateCommand);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}