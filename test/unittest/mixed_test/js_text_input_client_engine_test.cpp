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

#include <gtest/gtest.h>
#include <napi/native_api.h>
#include <napi/native_node_api.h>
#include <memory>
#include <string>
#include "input_method_ability.h"
#include "js_text_input_client_engine.h"
#include "mock_input_method_ability.h"
#include "mock_napi.h"
#include "mock_async_call.h"
#include "mock_block_queue.h"

using namespace OHOS::MiscServices;
using namespace testing;

class JsTextInputClientEngineTest : public Test {
public:
    static void SetUpTestCase()
    {
        // Initialize NAPI environment
        napi_env env = nullptr;
        napi_create_env(nullptr, nullptr, &env);
        napi_value info = nullptr;
        napi_create_object(env, &info);
        JsTextInputClientEngine::Init(env, info);
    }

    static void TearDownTestCase()
    {
        // Clean up NAPI environment
        napi_env env = nullptr;
        napi_destroy_env(env);
    }

    void SetUp()
    {
        // Mock InputMethodAbility
        mockInputMethodAbility = std::make_shared<MockInputMethodAbility>();
        InputMethodAbility::SetInstance(mockInputMethodAbility);
    }

    void TearDown()
    {
        // Reset InputMethodAbility instance
        InputMethodAbility::SetInstance(nullptr);
    }

    std::shared_ptr<MockInputMethodAbility> mockInputMethodAbility;
};

HWTEST_F(JsTextInputClientEngineTest, MoveCursor_ValidDirection_Success) {
    napi_env env = nullptr;
    napi_value result = nullptr;
    napi_create_env(nullptr, nullptr, &env);
    napi_value args[1];
    napi_create_int32(env, 1, &args[0]); // Valid direction

    EXPECT_CALL(*mockInputMethodAbility, MoveCursor(1)).WillOnce(Return(ErrorCode::NO_ERROR));

    napi_value instance = JsTextInputClientEngine::GetTextInputClientInstance(env);
    napi_call_function(env, instance, instance, JsTextInputClientEngine::MoveCursor, 1, args, &result);

    napi_destroy_env(env);
}

HWTEST_F(JsTextInputClientEngineTest, MoveCursor_InvalidDirection_ThrowsException) {
    napi_env env = nullptr;
    napi_value result = nullptr;
    napi_create_env(nullptr, nullptr, &env);
    napi_value args[1];
    napi_create_int32(env, -1, &args[0]); // Invalid direction

    EXPECT_CALL(*mockInputMethodAbility, MoveCursor(-1)).WillOnce(Return(ErrorCode::ERROR_INVALID_PARAMETER));

    napi_value instance = JsTextInputClientEngine::GetTextInputClientInstance(env);
    napi_call_function(env, instance, instance, JsTextInputClientEngine::MoveCursor, 1, args, &result);

    napi_destroy_env(env);
}

HWTEST_F(JsTextInputClientEngineTest, DeleteForward_ValidLength_Success) {
    napi_env env = nullptr;
    napi_value result = nullptr;
    napi_create_env(nullptr, nullptr, &env);
    napi_value args[1];
    napi_create_int32(env, 2, &args[0]); // Valid length

    EXPECT_CALL(*mockInputMethodAbility, DeleteForward(2)).WillOnce(Return(ErrorCode::NO_ERROR));

    napi_value instance = JsTextInputClientEngine::GetTextInputClientInstance(env);
    napi_call_function(env, instance, instance, JsTextInputClientEngine::DeleteForward, 1, args, &result);

    napi_destroy_env(env);
}

HWTEST_F(JsTextInputClientEngineTest, DeleteForward_InvalidLength_ThrowsException) {
    napi_env env = nullptr;
    napi_value result = nullptr;
    napi_create_env(nullptr, nullptr, &env);
    napi_value args[1];
    napi_create_int32(env, -1, &args[0]); // Invalid length

    EXPECT_CALL(*mockInputMethodAbility, DeleteForward(-1)).WillOnce(Return(ErrorCode::ERROR_INVALID_PARAMETER));

    napi_value instance = JsTextInputClientEngine::GetTextInputClientInstance(env);
    napi_call_function(env, instance, instance, JsTextInputClientEngine::DeleteForward, 1, args, &result);

    napi_destroy_env(env);
}

HWTEST_F(JsTextInputClientEngineTest, SendMessage_ValidParameters_MessageSent) {
    napi_value msgId = env_->CreateString("validMsgId");
    napi_value msgParam = env_->CreateArrayBuffer(10);
    napi_value args[] = {msgId, msgParam};
    napi_callback_info info = env_->CreateCallbackInfo(2, args);

    MockAsyncCall asyncCall(env_.get(), info, std::make_shared<AsyncCall::Context>(), 2);
    EXPECT_CALL(asyncCall, Call(_, _, _)).WillOnce(Return(env_->CreateUndefined()));

    EXPECT_CALL(*MockInputMethodAbility::GetInstance(), SendMessage(_)).WillOnce(Return(ErrorCode::NO_ERROR));

    napi_value result = inputClient_->SendMessage(env_.get(), info);
    EXPECT_EQ(result, env_->CreateUndefined());
}

HWTEST_F(JsTextInputClientEngineTest, RecvMessage_ValidHandler_HandlerRegistered) {
    napi_value onMessage = env_->CreateFunction("onMessage", nullptr);
    napi_value onTerminated = env_->CreateFunction("onTerminated", nullptr);
    napi_value msgHandler = env_->CreateObject();
    env_->SetNamedProperty(msgHandler, "onMessage", onMessage);
    env_->SetNamedProperty(msgHandler, "onTerminated", onTerminated);

    napi_value args[] = {msgHandler};
    napi_callback_info info = env_->CreateCallbackInfo(1, args);

    EXPECT_CALL(*MockInputMethodAbility::GetInstance(), RegisterMsgHandler(_)).WillOnce(Return());

    napi_value result = inputClient_->RecvMessage(env_.get(), info);
    EXPECT_EQ(result, env_->CreateNull());
}

HWTEST_F(JsTextInputClientEngineTest, OnTerminated_ValidCallback_CallbackInvoked) {
    napi_value onTerminated = env_->CreateFunction("onTerminated", nullptr);
    napi_value msgHandler = env_->CreateObject();
    env_->SetNamedProperty(msgHandler, "onTerminated", onTerminated);

    napi_value args[] = {msgHandler};
    napi_callback_info info = env_->CreateCallbackInfo(1, args);

    EXPECT_CALL(*MockInputMethodAbility::GetInstance(), RegisterMsgHandler(_)).WillOnce(Return());

    napi_value result = inputClient_->RecvMessage(env_.get(), info);
    EXPECT_EQ(result, env_->CreateNull());

    auto jsMessageHandler = std::make_shared<JsTextInputClientEngine::JsMessageHandler>(env_.get(),
        onTerminated, nullptr);
    EXPECT_CALL(*jsMessageHandler, OnTerminated()).WillOnce(Return(ErrorCode::NO_ERROR));

    jsMessageHandler->OnTerminated();
}

HWTEST_F(JsTextInputClientEngineTest, OnMessage_ValidCallback_CallbackInvoked) {
    napi_value onMessage = env_->CreateFunction("onMessage", nullptr);
    napi_value msgHandler = env_->CreateObject();
    env_->SetNamedProperty(msgHandler, "onMessage", onMessage);

    napi_value args[] = {msgHandler};
    napi_callback_info info = env_->CreateCallbackInfo(1, args);

    EXPECT_CALL(*MockInputMethodAbility::GetInstance(), RegisterMsgHandler(_)).WillOnce(Return());

    napi_value result = inputClient_->RecvMessage(env_.get(), info);
    EXPECT_EQ(result, env_->CreateNull());

    auto jsMessageHandler = std::make_shared<JsTextInputClientEngine::JsMessageHandler>(env_.get(), nullptr, onMessage);
    ArrayBuffer arrayBuffer = {"validMsgId", nullptr, 0};
    EXPECT_CALL(*jsMessageHandler, OnMessage(arrayBuffer)).WillOnce(Return(ErrorCode::NO_ERROR));

    jsMessageHandler->OnMessage(arrayBuffer);
}