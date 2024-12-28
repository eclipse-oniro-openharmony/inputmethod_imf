/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#include "input_method_controller.h"
#include "input_method_message_handler_proxy.h"
#include "input_method_proxy.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

// 假设这些是错误码的定义
constexpr int32_t IME_ERR_NULL_POINTER = -1;
constexpr int32_t IME_ERR_OK = 0;

using namespace testing;

// 模拟 InputMethodController
class MockInputMethodController : public InputMethodController {
public:
    MOCK_METHOD(int, ShowCurrentInput, (), (override));
    MOCK_METHOD3(OnSelectionChange, int(const std::u16string &text, int start, int end));
    MOCK_METHOD(int, OnCursorUpdate, (const CursorInfo &cursorInfo), (override));
    MOCK_METHOD1(SendPrivateCommand, int(const std::unordered_map<std::string, PrivateDataValue> &));
    MOCK_METHOD1(SendMessage, InputMethod_ErrorCode(const ArrayBuffer &arrayBuffer));
};

class InputMethodProxyTest : public Test {
protected:
    void SetUp() override
    {
        inputMethodProxy = std::make_shared<MockInputMethodProxy>();
        messageHandler = std::make_shared<MockMessageHandlerProxy>();
        inputMethodController = std::make_shared<MockInputMethodController>();
    }

    std::shared_ptr<MockInputMethodProxy> inputMethodProxy;
    std::shared_ptr<MockMessageHandlerProxy> messageHandler;
    std::shared_ptr<MockInputMethodController> inputMethodController;
};

// 测试类
class OH_InputMethodProxy_SendPrivateCommandTest : public Test {
protected:
    MockInputMethodController *mockController;
    InputMethod_InputMethodProxy inputMethodProxy;

    void SetUp() override
    {
        mockController = new MockInputMethodController();
        ON_CALL(*mockController, SendPrivateCommand(_)).WillByDefault(Return(IME_ERR_OK));
        InputMethodController::SetInstance(mockController);
    }

    void TearDown() override
    {
        InputMethodController::SetInstance(nullptr);
        delete mockController;
        mockController = nullptr;
    }
};

// 测试用例：messageHandler 为 nullptr
TEST(IsValidMessageHandlerProxyTest, MessageHandlerIsNull_ReturnsOk)
{
    InputMethod_MessageHandlerProxy *messageHandler = nullptr;
    int32_t result = IsValidMessageHandlerProxy(messageHandler);
    EXPECT_EQ(result, IME_ERR_OK);
}

// 测试用例：onTerminatedFunc 为 nullptr
TEST(IsValidMessageHandlerProxyTest, OnTerminatedFuncIsNull_ReturnsNullPointer)
{
    InputMethod_MessageHandlerProxy messageHandler = { nullptr, nullptr };
    int32_t result = IsValidMessageHandlerProxy(&messageHandler);
    EXPECT_EQ(result, IME_ERR_NULL_POINTER);
}

// 测试用例：onMessageFunc 为 nullptr
TEST(IsValidMessageHandlerProxyTest, OnMessageFuncIsNull_ReturnsNullPointer)
{
    InputMethod_MessageHandlerProxy messageHandler = { [](void *) {}, nullptr };
    int32_t result = IsValidMessageHandlerProxy(&messageHandler);
    EXPECT_EQ(result, IME_ERR_NULL_POINTER);
}

// 测试用例：messageHandler 有效
TEST(IsValidMessageHandlerProxyTest, ValidMessageHandler_ReturnsOk)
{
    InputMethod_MessageHandlerProxy messageHandler = { [](void *) {}, [](void *) {} };
    int32_t result = IsValidMessageHandlerProxy(&messageHandler);
    EXPECT_EQ(result, IME_ERR_OK);
}

TEST(OH_InputMethodProxy_ShowKeyboard, InvalidInputMethodProxy_ReturnsError)
{
    InputMethod_InputMethodProxy inputMethodProxy;
    MockInputMethodController mockController;
    MockLogger mockLogger;

    // 模拟 IsValidInputMethodProxy 返回错误代码
    EXPECT_CALL(mockLogger, LogError(Contains("invalid state")));

    // 模拟 InputMethodController 不应被调用
    EXPECT_CALL(mockController, ShowCurrentInput()).Times(0);

    // 替换实际函数和对象
    auto originalIsValidInputMethodProxy = IsValidInputMethodProxy;
    IsValidInputMethodProxy = MockIsValidInputMethodProxy;
    auto originalInputMethodController = InputMethodController::GetInstance();
    InputMethodController::SetInstance(&mockController);

    int result = OH_InputMethodProxy_ShowKeyboard(&inputMethodProxy);

    // 恢复原始函数和对象
    IsValidInputMethodProxy = originalIsValidInputMethodProxy;
    InputMethodController::SetInstance(originalInputMethodController);

    EXPECT_EQ(result, IME_ERR_INVALID_STATE);
}

TEST(OH_InputMethodProxy_ShowKeyboard, ValidInputMethodProxy_CallsShowCurrentInput)
{
    InputMethod_InputMethodProxy inputMethodProxy;
    MockInputMethodController mockController;
    MockLogger mockLogger;

    // 模拟 IsValidInputMethodProxy 返回 OK
    auto mockIsValidInputMethodProxy = [](InputMethod_InputMethodProxy *inputMethodProxy) {
        return IME_ERR_OK;
    };

    // 模拟 InputMethodController 的 ShowCurrentInput 返回成功
    EXPECT_CALL(mockController, ShowCurrentInput()).WillOnce(Return(IME_ERR_OK));

    // 替换实际函数和对象
    auto originalIsValidInputMethodProxy = IsValidInputMethodProxy;
    IsValidInputMethodProxy = mockIsValidInputMethodProxy;
    auto originalInputMethodController = InputMethodController::GetInstance();
    InputMethodController::SetInstance(&mockController);

    int result = OH_InputMethodProxy_ShowKeyboard(&inputMethodProxy);

    // 恢复原始函数和对象
    IsValidInputMethodProxy = originalIsValidInputMethodProxy;
    InputMethodController::SetInstance(originalInputMethodController);

    EXPECT_EQ(result, IME_ERR_OK);
}

// 测试无效的输入方法代理
TEST(OH_InputMethodProxy_HideKeyboard, InvalidInputMethodProxy_ReturnsError)
{
    InputMethod_InputMethodProxy inputMethodProxy;
    EXPECT_CALL(InputMethodController::GetInstance(), HideCurrentInput()).Times(0);

    // 模拟无效的输入方法代理
    EXPECT_CALL(InputMethodController::GetInstance(), IsValidInputMethodProxy(&inputMethodProxy))
        .WillOnce(Return(IME_ERR_INVALID_STATE));

    int result = OH_InputMethodProxy_HideKeyboard(&inputMethodProxy);
    EXPECT_EQ(result, IME_ERR_INVALID_STATE);
}

// 测试有效的输入方法代理
TEST(OH_InputMethodProxy_HideKeyboard, ValidInputMethodProxy_HidesKeyboard)
{
    InputMethod_InputMethodProxy inputMethodProxy;
    EXPECT_CALL(InputMethodController::GetInstance(), HideCurrentInput()).WillOnce(Return(IME_ERR_OK));

    // 模拟有效的输入方法代理
    EXPECT_CALL(InputMethodController::GetInstance(), IsValidInputMethodProxy(&inputMethodProxy))
        .WillOnce(Return(IME_ERR_OK));

    int result = OH_InputMethodProxy_HideKeyboard(&inputMethodProxy);
    EXPECT_EQ(result, IME_ERR_OK);
}

TEST(OH_InputMethodProxy_NotifySelectionChange, InvalidInputMethodProxy_ReturnsError)
{
    InputMethod_InputMethodProxy inputMethodProxy;
    char16_t text[] = u"test";
    size_t length = 4;
    int start = 0;
    int end = 2;

    // 模拟无效的输入法代理
    EXPECT_CALL(MockIsValidInputMethodProxy(&inputMethodProxy)).WillOnce(Return(IME_ERR_INVALID_STATE));

    int result = OH_InputMethodProxy_NotifySelectionChange(&inputMethodProxy, text, length, start, end);
    EXPECT_EQ(result, IME_ERR_INVALID_STATE);
}

TEST(OH_InputMethodProxy_NotifySelectionChange, NullText_ReturnsNullPointerError)
{
    InputMethod_InputMethodProxy inputMethodProxy;
    size_t length = 4;
    int start = 0;
    int end = 2;

    int result = OH_InputMethodProxy_NotifySelectionChange(&inputMethodProxy, nullptr, length, start, end);
    EXPECT_EQ(result, IME_ERR_NULL_POINTER);
}

TEST(OH_InputMethodProxy_NotifySelectionChange, TextLengthExceedsMax_ReturnsParamCheckError)
{
    InputMethod_InputMethodProxy inputMethodProxy;
    char16_t text[] = u"this_is_a_very_long_text_exceeding_max_length";
    size_t length = sizeof(text) / sizeof(text[0]);
    int start = 0;
    int end = 2;

    // 假设 MAX_TEXT_LENGTH 是一个已知的常量
    const size_t MAX_TEXT_LENGTH = 10;

    int result = OH_InputMethodProxy_NotifySelectionChange(&inputMethodProxy, text, length, start, end);
    EXPECT_EQ(result, IME_ERR_PARAMCHECK);
}

TEST(OH_InputMethodProxy_NotifySelectionChange, ValidInput_CallsOnSelectionChange)
{
    InputMethod_InputMethodProxy inputMethodProxy;
    char16_t text[] = u"test";
    size_t length = 4;
    int start = 0;
    int end = 2;

    // 模拟 InputMethodController
    MockInputMethodController *mockController = new MockInputMethodController();
    EXPECT_CALL(*mockController, OnSelectionChange(std::u16string(text, length), start, end))
        .WillOnce(Return(IME_ERR_OK));

    int result = OH_InputMethodProxy_NotifySelectionChange(&inputMethodProxy, text, length, start, end);
    EXPECT_EQ(result, IME_ERR_OK);

    delete mockController;
    mockController = nullptr;
}

TEST(OH_InputMethodProxy_NotifyConfigurationChange, InvalidProxy_ReturnsError)
{
    InputMethod_InputMethodProxy proxy;
    EXPECT_CALL(proxy, IsValidInputMethodProxy()).WillOnce(Return(IME_ERR_INVALID_STATE));

    int result = OH_InputMethodProxy_NotifyConfigurationChange(&proxy, ENTER_KEY_DEFAULT, TEXT_INPUT_TYPE_TEXT);
    EXPECT_EQ(result, IME_ERR_INVALID_STATE);
}

TEST(OH_InputMethodProxy_NotifyConfigurationChange, ValidProxy_CallsOnConfigurationChange)
{
    InputMethod_InputMethodProxy proxy;
    MockInputMethodController mockController;
    EXPECT_CALL(proxy, IsValidInputMethodProxy()).WillOnce(Return(IME_ERR_OK));
    EXPECT_CALL(mockController, OnConfigurationChange(_)).WillOnce(Return(IME_ERR_OK));

    int result = OH_InputMethodProxy_NotifyConfigurationChange(&proxy, ENTER_KEY_DEFAULT, TEXT_INPUT_TYPE_TEXT);
    EXPECT_EQ(result, IME_ERR_OK);
}

// 测试用例
TEST(OH_InputMethodProxy_NotifyCursorUpdate, InvalidInputMethodProxy_ReturnsError)
{
    InputMethod_InputMethodProxy *proxy = nullptr;
    InputMethod_CursorInfo cursorInfo = { 0, 0, 10, 10 };

    EXPECT_EQ(OH_InputMethodProxy_NotifyCursorUpdate(proxy, &cursorInfo), IME_ERR_INVALID_STATE);
}

TEST(OH_InputMethodProxy_NotifyCursorUpdate, NullCursorInfo_ReturnsNullPointerError)
{
    InputMethod_InputMethodProxy proxy;
    InputMethod_CursorInfo *cursorInfo = nullptr;

    EXPECT_EQ(OH_InputMethodProxy_NotifyCursorUpdate(&proxy, cursorInfo), IME_ERR_NULL_POINTER);
}

TEST(OH_InputMethodProxy_NotifyCursorUpdate, ValidProxyAndCursorInfo_ReturnsSuccess)
{
    InputMethod_InputMethodProxy proxy;
    InputMethod_CursorInfo cursorInfo = { 0, 0, 10, 10 };

    // 模拟 InputMethodController
    MockInputMethodController *mockController = new MockInputMethodController();
    EXPECT_CALL(*mockController, OnCursorUpdate(_)).WillOnce(Return(IME_ERR_OK));

    // 替换实际的控制器实例
    InputMethodController::SetInstance(mockController);

    EXPECT_EQ(OH_InputMethodProxy_NotifyCursorUpdate(&proxy, &cursorInfo), IME_ERR_OK);

    // 清理
    InputMethodController::SetInstance(nullptr);
    delete mockController;
    mockController = nullptr;
}

// 测试用例
TEST_F(OH_InputMethodProxy_SendPrivateCommandTest, InvalidInputMethodProxy_ReturnsError)
{
    // 模拟无效的输入方法代理
    EXPECT_CALL(*mockController, SendPrivateCommand(_)).Times(0);

    InputMethod_PrivateCommand *privateCommand = nullptr;
    size_t size = 0;
    int result = OH_InputMethodProxy_SendPrivateCommand(&inputMethodProxy, &privateCommand, size);

    EXPECT_EQ(result, IME_ERR_INVALID_STATE);
}

TEST_F(OH_InputMethodProxy_SendPrivateCommandTest, PrivateCommandIsNull_ReturnsNullPointerError)
{
    // 模拟私有命令为 nullptr
    EXPECT_CALL(*mockController, SendPrivateCommand(_)).Times(0);

    InputMethod_PrivateCommand *privateCommand = nullptr;
    size_t size = 1;
    int result = OH_InputMethodProxy_SendPrivateCommand(&inputMethodProxy, &privateCommand, size);

    EXPECT_EQ(result, IME_ERR_NULL_POINTER);
}

TEST_F(OH_InputMethodProxy_SendPrivateCommandTest, PrivateCommandArrayContainsNull_ReturnsNullPointerError)
{
    // 模拟私有命令数组包含 nullptr
    EXPECT_CALL(*mockController, SendPrivateCommand(_)).Times(0);

    InputMethod_PrivateCommand *privateCommand[1] = { nullptr };
    size_t size = 1;
    int result = OH_InputMethodProxy_SendPrivateCommand(&inputMethodProxy, privateCommand, size);

    EXPECT_EQ(result, IME_ERR_NULL_POINTER);
}

TEST_F(OH_InputMethodProxy_SendPrivateCommandTest, ValidInput_SendPrivateCommandCalled)
{
    // 模拟有效的输入
    EXPECT_CALL(*mockController, SendPrivateCommand(_)).WillOnce(Return(IME_ERR_OK));

    InputMethod_PrivateCommand privateCommand1 = { "key1", { "value1", PRIVATE_DATA_TYPE_STRING } };
    InputMethod_PrivateCommand *privateCommand[1] = { &privateCommand1 };
    size_t size = 1;
    int result = OH_InputMethodProxy_SendPrivateCommand(&inputMethodProxy, privateCommand, size);

    EXPECT_EQ(result, IME_ERR_OK);
}

// Unit Test Code:
TEST(OH_InputMethodProxy_SendMessage, InvalidInputMethodProxy_ReturnsError)
{
    InputMethod_InputMethodProxy *inputMethodProxy = nullptr;
    const char16_t *msgId = u"test";
    size_t msgIdLength = 4;
    const uint8_t *msgParam = reinterpret_cast<const uint8_t *>("test");
    size_t msgParamLength = 4;

    EXPECT_EQ(OH_InputMethodProxy_SendMessage(inputMethodProxy, msgId, msgIdLength, msgParam, msgParamLength),
        IME_ERR_INVALID_STATE);
}

TEST(OH_InputMethodProxy_SendMessage, NullMsgId_ReturnsNullPointerError)
{
    InputMethod_InputMethodProxy inputMethodProxy;
    const char16_t *msgId = nullptr;
    size_t msgIdLength = 4;
    const uint8_t *msgParam = reinterpret_cast<const uint8_t *>("test");
    size_t msgParamLength = 4;

    EXPECT_EQ(OH_InputMethodProxy_SendMessage(&inputMethodProxy, msgId, msgIdLength, msgParam, msgParamLength),
        IME_ERR_NULL_POINTER);
}

TEST(OH_InputMethodProxy_SendMessage, NullMsgParam_ReturnsNullPointerError)
{
    InputMethod_InputMethodProxy inputMethodProxy;
    const char16_t *msgId = u"test";
    size_t msgIdLength = 4;
    const uint8_t *msgParam = nullptr;
    size_t msgParamLength = 4;

    EXPECT_EQ(OH_InputMethodProxy_SendMessage(&inputMethodProxy, msgId, msgIdLength, msgParam, msgParamLength),
        IME_ERR_NULL_POINTER);
}

TEST(OH_InputMethodProxy_SendMessage, ExceedingMsgIdLength_ReturnsParamCheckError)
{
    InputMethod_InputMethodProxy inputMethodProxy;
    const char16_t *msgId = u"test";
    size_t msgIdLength = INVALID_MSG_ID_SIZE + 1;
    const uint8_t *msgParam = reinterpret_cast<const uint8_t *>("test");
    size_t msgParamLength = 4;

    EXPECT_EQ(OH_InputMethodProxy_SendMessage(&inputMethodProxy, msgId, msgIdLength, msgParam, msgParamLength),
        IME_ERR_PARAMCHECK);
}

TEST(OH_InputMethodProxy_SendMessage, ExceedingMsgParamLength_ReturnsParamCheckError)
{
    InputMethod_InputMethodProxy inputMethodProxy;
    const char16_t *msgId = u"test";
    size_t msgIdLength = 4;
    const uint8_t *msgParam = reinterpret_cast<const uint8_t *>("test");
    size_t msgParamLength = INVALID_MSG_PARAM_SIZE + 1;

    EXPECT_EQ(OH_InputMethodProxy_SendMessage(&inputMethodProxy, msgId, msgIdLength, msgParam, msgParamLength),
        IME_ERR_PARAMCHECK);
}

TEST(OH_InputMethodProxy_SendMessage, ValidInputs_ReturnsSuccess)
{
    InputMethod_InputMethodProxy inputMethodProxy;
    const char16_t *msgId = u"test";
    size_t msgIdLength = 4;
    const uint8_t *msgParam = reinterpret_cast<const uint8_t *>("test");
    size_t msgParamLength = 4;

    MockInputMethodController *mockController =
        static_cast<MockInputMethodController *>(InputMethodController::GetInstance());
    EXPECT_CALL(*mockController, SendMessage(_)).WillOnce(Return(IME_ERR_OK));

    EXPECT_EQ(
        OH_InputMethodProxy_SendMessage(&inputMethodProxy, msgId, msgIdLength, msgParam, msgParamLength), IME_ERR_OK);
}

TEST_F(InputMethodProxyTest, OH_InputMethodProxy_RecvMessage_InvalidInputMethodProxy_ErrorReturned)
{
    EXPECT_CALL(*inputMethodProxy, IsValid()).WillOnce(Return(false));
    EXPECT_CALL(*messageHandler, IsValid()).WillOnce(Return(true));

    int result = OH_InputMethodProxy_RecvMessage(inputMethodProxy.get(), messageHandler.get());
    EXPECT_EQ(result, IME_ERR_INVALID_STATE);
}

TEST_F(InputMethodProxyTest, OH_InputMethodProxy_RecvMessage_InvalidMessageHandler_ErrorReturned)
{
    EXPECT_CALL(*inputMethodProxy, IsValid()).WillOnce(Return(true));
    EXPECT_CALL(*messageHandler, IsValid()).WillOnce(Return(false));

    int result = OH_InputMethodProxy_RecvMessage(inputMethodProxy.get(), messageHandler.get());
    EXPECT_EQ(result, IME_ERR_NULL_POINTER);
}

TEST_F(InputMethodProxyTest, OH_InputMethodProxy_RecvMessage_NullMessageHandler_UnregisterCalled)
{
    EXPECT_CALL(*inputMethodProxy, IsValid()).WillOnce(Return(true));
    EXPECT_CALL(*messageHandler, IsValid()).WillOnce(Return(true));

    EXPECT_CALL(*inputMethodController, RegisterMsgHandler(nullptr)).WillOnce(Return(IME_ERR_OK));

    int result = OH_InputMethodProxy_RecvMessage(inputMethodProxy.get(), nullptr);
    EXPECT_EQ(result, IME_ERR_OK);
}

TEST_F(InputMethodProxyTest, OH_InputMethodProxy_RecvMessage_ValidMessageHandler_RegisterCalled)
{
    EXPECT_CALL(*inputMethodProxy, IsValid()).WillOnce(Return(true));
    EXPECT_CALL(*messageHandler, IsValid()).WillOnce(Return(true));

    std::shared_ptr<MsgHandlerCallbackInterface> msgHandler =
        std::make_shared<NativeMessageHandlerCallback>(messageHandler.get());
    EXPECT_CALL(*inputMethodController, RegisterMsgHandler(msgHandler)).WillOnce(Return(IME_ERR_OK));

    int result = OH_InputMethodProxy_RecvMessage(inputMethodProxy.get(), messageHandler.get());
    EXPECT_EQ(result, IME_ERR_OK);
}
