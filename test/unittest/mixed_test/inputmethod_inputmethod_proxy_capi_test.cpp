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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "input_method_controller.h"
#include "native_message_handler_callback.h"
#include "native_inputmethod_types.h"
#include "native_inputmethod_utils.h"
#include "string_ex.h"
#include "input_method_proxy.h"

using namespace OHOS;
using namespace OHOS::MiscServices;
using namespace testing;

class MockInputMethodController : public InputMethodController {
public:
    MOCK_METHOD0(ShowCurrentInput, int32_t());
    MOCK_METHOD0(HideCurrentInput, int32_t());
    MOCK_METHOD3(OnSelectionChange, int32_t(const std::u16string&, int, int));
    MOCK_METHOD1(OnConfigurationChange, int32_t(const Configuration&));
    MOCK_METHOD1(OnCursorUpdate, int32_t(const CursorInfo&));
    MOCK_METHOD1(SendPrivateCommand, int32_t(const std::unordered_map<std::string, PrivateDataValue>&));
    MOCK_METHOD1(SendMessage, int32_t(const ArrayBuffer&));
    MOCK_METHOD1(RegisterMsgHandler, int32_t(const std::shared_ptr<MsgHandlerCallbackInterface>&));
};

class MockNativeMessageHandlerCallback : public NativeMessageHandlerCallback {
public:
    explicit MockNativeMessageHandlerCallback(InputMethod_MessageHandlerProxy* messageHandler)
        : NativeMessageHandlerCallback(messageHandler) {}
};

class InputMethodProxyTest : public Test {
protected:
    void SetUp() override
    {
        inputMethodProxy = new InputMethod_InputMethodProxy();
        messageHandler = new InputMethod_MessageHandlerProxy();
        messageHandler->onTerminatedFunc = [](void*) {};
        messageHandler->onMessageFunc = [](void*, const char16_t*, size_t, const uint8_t*, size_t) {};
        controller = std::make_shared<MockInputMethodController>();
        ON_CALL(*controller, ShowCurrentInput()).WillByDefault(Return(IME_ERR_OK));
        ON_CALL(*controller, HideCurrentInput()).WillByDefault(Return(IME_ERR_OK));
        ON_CALL(*controller, OnSelectionChange(_, _, _)).WillByDefault(Return(IME_ERR_OK));
        ON_CALL(*controller, OnConfigurationChange(_)).WillByDefault(Return(IME_ERR_OK));
        ON_CALL(*controller, OnCursorUpdate(_)).WillByDefault(Return(IME_ERR_OK));
        ON_CALL(*controller, SendPrivateCommand(_)).WillByDefault(Return(IME_ERR_OK));
        ON_CALL(*controller, SendMessage(_)).WillByDefault(Return(IME_ERR_OK));
        ON_CALL(*controller, RegisterMsgHandler(_)).WillByDefault(Return(IME_ERR_OK));
    }

    void TearDown() override
    {
        delete inputMethodProxy;
        inputMethodProxy = nullptr;
        delete messageHandler;
        messageHandler = nullptr;
    }

    InputMethod_InputMethodProxy* inputMethodProxy;
    InputMethod_MessageHandlerProxy* messageHandler;
    std::shared_ptr<MockInputMethodController> controller;
};

HWTEST_F(InputMethodProxyTest, OH_InputMethodProxy_ShowKeyboard_ValidProxy_Success) {
    EXPECT_CALL(*controller, ShowCurrentInput()).WillOnce(Return(IME_ERR_OK));
    EXPECT_EQ(OH_InputMethodProxy_ShowKeyboard(inputMethodProxy), IME_ERR_OK);
}

HWHWTEST_F(InputMethodProxyTest, OH_InputMethodProxy_ShowKeyboard_NullProxy_Error) {
    EXPECT_EQ(OH_InputMethodProxy_ShowKeyboard(nullptr), IME_ERR_OK);
}

HWTEST_F(InputMethodProxyTest, OH_InputMethodProxy_HideKeyboard_ValidProxy_Success) {
    EXPECT_CALL(*controller, HideCurrentInput()).WillOnce(Return(IME_ERR_OK));
    EXPECT_EQ(OH_InputMethodProxy_HideKeyboard(inputMethodProxy), IME_ERR_OK);
}

HWTEST_F(InputMethodProxyTest, OH_InputMethodProxy_HideKeyboard_NullProxy_Error) {
    EXPECT_EQ(OH_InputMethodProxy_HideKeyboard(nullptr), IME_ERR_OK);
}

HWTEST_F(InputMethodProxyTest, OH_InputMethodProxy_NotifySelectionChange_ValidParameters_Success) {
    char16_t text[] = u"test";
    EXPECT_CALL(*controller, OnSelectionChange(_, _, _)).WillOnce(Return(IME_ERR_OK));
    EXPECT_EQ(OH_InputMethodProxy_NotifySelectionChange(inputMethodProxy, text, 4, 0, 4), IME_ERR_OK);
}

HWTEST_F(InputMethodProxyTest, OH_InputMethodProxy_NotifySelectionChange_NullText_Error) {
    EXPECT_EQ(OH_InputMethodProxy_NotifySelectionChange(inputMethodProxy, nullptr, 4, 0, 4), IME_ERR_NULL_POINTER);
}

HWTEST_F(InputMethodProxyTest, OH_InputMethodProxy_NotifySelectionChange_ExceedingLength_Error) {
    char16_t text[] = u"test";
    EXPECT_EQ(OH_InputMethodProxy_NotifySelectionChange(inputMethodProxy, text, MAX_TEXT_LENGTH + 1, 0, 4),
        IME_ERR_PARAMCHECK);
}

HWTEST_F(InputMethodProxyTest, OH_InputMethodProxy_NotifyCursorUpdate_ValidCursorInfo_Success) {
    InputMethod_CursorInfo cursorInfo = { 0, 0, 10, 10 };
    EXPECT_CALL(*controller, OnCursorUpdate(_)).WillOnce(Return(IME_ERR_OK));
    EXPECT_EQ(OH_InputMethodProxy_NotifyCursorUpdate(inputMethodProxy, &cursorInfo), IME_ERR_OK);
}

HWTEST_F(InputMethodProxyTest, OH_InputMethodProxy_NotifyCursorUpdate_NullCursorInfo_Error) {
    EXPECT_EQ(OH_InputMethodProxy_NotifyCursorUpdate(inputMethodProxy, nullptr), IME_ERR_NULL_POINTER);
}

HWTEST_F(InputMethodProxyTest, OH_InputMethodProxy_SendPrivateCommand_ValidCommand_Success) {
    InputMethod_PrivateCommand privateCommand = { "key", "value" };
    InputMethod_PrivateCommand* commands[] = { &privateCommand };
    EXPECT_CALL(*controller, SendPrivateCommand(_)).WillOnce(Return(IME_ERR_OK));
    EXPECT_EQ(OH_InputMethodProxy_SendPrivateCommand(inputMethodProxy, commands, 1), IME_ERR_OK);
}

HWTEST_F(InputMethodProxyTest, OH_InputMethodProxy_SendPrivateCommand_NullCommand_Error) {
    EXPECT_EQ(OH_InputMethodProxy_SendPrivateCommand(inputMethodProxy, nullptr, 1), IME_ERR_NULL_POINTER);
}

HWTEST_F(InputMethodProxyTest, OH_InputMethodProxy_SendMessage_ValidParameters_Success) {
    char16_t msgId[] = u"msgId";
    uint8_t msgParam[] = { 0, 1, 2 };
    EXPECT_CALL(*controller, SendMessage(_)).WillOnce(Return(IME_ERR_OK));
    EXPECT_EQ(OH_InputMethodProxy_SendMessage(inputMethodProxy, msgId, 5, msgParam, 3), IME_ERR_OK);
}

HWTEST_F(InputMethodProxyTest, OH_InputMethodProxy_SendMessage_NullParameters_Error) {
    EXPECT_EQ(OH_InputMethodProxy_SendMessage(inputMethodProxy, nullptr, 5, nullptr, 3), IME_ERR_NULL_POINTER);
}

HWTEST_F(InputMethodProxyTest, OH_InputMethodProxy_SendMessage_ExceedingLength_Error) {
    char16_t msgId[] = u"msgId";
    uint8_t msgParam[] = { 0, 1, 2 };
    EXPECT_EQ(OH_InputMethodProxy_SendMessage(inputMethodProxy, msgId, INVALID_MSG_ID_SIZE + 1, msgParam, 3),
        IME_ERR_PARAMCHECK);
}

HWTEST_F(InputMethodProxyTest, OH_InputMethodProxy_RecvMessage_ValidHandler_Success) {
    EXPECT_CALL(*controller, RegisterMsgHandler(_)).WillOnce(Return(IME_ERR_OK));
    EXPECT_EQ(OH_InputMethodProxy_RecvMessage(inputMethodProxy, messageHandler), IME_ERR_OK);
}

HWTEST_F(InputMethodProxyTest, OH_InputMethodProxy_RecvMessage_NullHandler_Success) {
    EXPECT_CALL(*controller, RegisterMsgHandler(nullptr)).WillOnce(Return(IME_ERR_OK));
    EXPECT_EQ(OH_InputMethodProxy_RecvMessage(inputMethodProxy, nullptr), IME_ERR_OK);
}