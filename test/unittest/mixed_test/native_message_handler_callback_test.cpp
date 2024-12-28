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
#include "native_message_handler_callback.h"
#include "string_ex.h"

using namespace OHOS::MiscServices;

class MockMessageHandler {
public:
    int32_t (*onTerminatedFunc)(void*) = nullptr;
    int32_t (*onMessageFunc)(void*, const char16_t*, size_t, const uint8_t*, size_t) = nullptr;
};

class NativeMessageHandlerCallbackTest : public testing::Test {
protected:
    NativeMessageHandlerCallback callback;
    MockMessageHandler mockHandler;

    void SetUp() override
    {
        callback.messageHandler_ = &mockHandler;
    }

    void TearDown() override
    {
        callback.messageHandler_ = nullptr;
    }
};

HWTEST_F(NativeMessageHandlerCallbackTest, OnTerminated_MessageHandlerNull_ReturnsError) {
    callback.messageHandler_ = nullptr;
    EXPECT_EQ(callback.OnTerminated(), ErrorCode::ERROR_CLIENT_NULL_POINTER);
}

HWTEST_F(NativeMessageHandlerCallbackTest, OnTerminated_OnTerminatedFuncNull_ReturnsError) {
    mockHandler.onTerminatedFunc = nullptr;
    EXPECT_EQ(callback.OnTerminated(), ErrorCode::ERROR_CLIENT_NULL_POINTER);
}

HWTEST_F(NativeMessageHandlerCallbackTest, OnTerminated_OnTerminatedFuncReturnsError_ReturnsError) {
    mockHandler.onTerminatedFunc = [](void*) { return ErrorCode::ERROR_MESSAGE_HANDLER; };
    EXPECT_EQ(callback.OnTerminated(), ErrorCode::ERROR_MESSAGE_HANDLER);
}

HWTEST_F(NativeMessageHandlerCallbackTest, OnTerminated_Success_ReturnsNoError) {
    mockHandler.onTerminatedFunc = [](void*) { return ErrorCode::NO_ERROR; };
    EXPECT_EQ(callback.OnTerminated(), ErrorCode::NO_ERROR);
}

HWTEST_F(NativeMessageHandlerCallbackTest, OnMessage_MessageHandlerNull_ReturnsError) {
    callback.messageHandler_ = nullptr;
    ArrayBuffer arrayBuffer;
    EXPECT_EQ(callback.OnMessage(arrayBuffer), ErrorCode::ERROR_CLIENT_NULL_POINTER);
}

HWTEST_F(NativeMessageHandlerCallbackTest, OnMessage_OnMessageFuncNull_ReturnsError) {
    mockHandler.onMessageFunc = nullptr;
    ArrayBuffer arrayBuffer;
    EXPECT_EQ(callback.OnMessage(arrayBuffer), ErrorCode::ERROR_CLIENT_NULL_POINTER);
}

HWTEST_F(NativeMessageHandlerCallbackTest, OnMessage_OnMessageFuncReturnsError_ReturnsError) {
    mockHandler.onMessageFunc = [](void*, const char16_t*, size_t, const uint8_t*, size_t) {
        return ErrorCode::ERROR_MESSAGE_HANDLER;
    };
    ArrayBuffer arrayBuffer;
    EXPECT_EQ(callback.OnMessage(arrayBuffer), ErrorCode::ERROR_MESSAGE_HANDLER);
}

HWTEST_F(NativeMessageHandlerCallbackTest, OnMessage_Success_ReturnsNoError) {
    mockHandler.onMessageFunc = [](void*, const char16_t*, size_t, const uint8_t*, size_t) {
        return ErrorCode::NO_ERROR;
    };
    ArrayBuffer arrayBuffer;
    EXPECT_EQ(callback.OnMessage(arrayBuffer), ErrorCode::NO_ERROR);
}