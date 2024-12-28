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
#include "native_inputmethod_types.h"
#include "global.h"
using namespace OHOS::MiscServices;

// Mock function for testing
void MockOnTerminatedFunc() {}
void MockOnMessageFunc() {}

HWTEST_F(MessageHandlerProxyTest, Create_Destroy_ValidProxy) {
    InputMethod_MessageHandlerProxy *proxy = OH_MessageHandlerProxy_Create();
    ASSERT_NE(proxy, nullptr);
    OH_MessageHandlerProxy_Destroy(proxy);
}

HWTEST_F(MessageHandlerProxyTest, Destroy_NullProxy) {
    OH_MessageHandlerProxy_Destroy(nullptr);
}

HWTEST_F(MessageHandlerProxyTest, SetOnTerminatedFunc_ValidProxy) {
    InputMethod_MessageHandlerProxy *proxy = OH_MessageHandlerProxy_Create();
    InputMethod_ErrorCode errorCode = OH_MessageHandlerProxy_SetOnTerminatedFunc(proxy, MockOnTerminatedFunc);
    EXPECT_EQ(errorCode, IME_ERR_OK);
    OH_MessageHandlerProxy_Destroy(proxy);
}

HWTEST_F(MessageHandlerProxyTest, SetOnTerminatedFunc_NullProxy) {
    InputMethod_ErrorCode errorCode = OH_MessageHandlerProxy_SetOnTerminatedFunc(nullptr, MockOnTerminatedFunc);
    EXPECT_EQ(errorCode, IME_ERR_NULL_POINTER);
}

HWTEST_F(MessageHandlerProxyTest, SetOnTerminatedFunc_NullFunc) {
    InputMethod_MessageHandlerProxy *proxy = OH_MessageHandlerProxy_Create();
    InputMethod_ErrorCode errorCode = OH_MessageHandlerProxy_SetOnTerminatedFunc(proxy, nullptr);
    EXPECT_EQ(errorCode, IME_ERR_NULL_POINTER);
    OH_MessageHandlerProxy_Destroy(proxy);
}

HWTEST_F(MessageHandlerProxyTest, GetOnTerminatedFunc_ValidProxy) {
    InputMethod_MessageHandlerProxy *proxy = OH_MessageHandlerProxy_Create();
    OH_MessageHandlerProxy_SetOnTerminatedFunc(proxy, MockOnTerminatedFunc);
    OH_MessageHandlerProxy_OnTerminatedFunc onTerminatedFunc = nullptr;
    InputMethod_ErrorCode errorCode = OH_MessageHandlerProxy_GetOnTerminatedFunc(proxy, &onTerminatedFunc);
    EXPECT_EQ(errorCode, IME_ERR_OK);
    EXPECT_EQ(onTerminatedFunc, MockOnTerminatedFunc);
    OH_MessageHandlerProxy_Destroy(proxy);
}

HWTEST_F(MessageHandlerProxyTest, GetOnTerminatedFunc_NullProxy) {
    OH_MessageHandlerProxy_OnTerminatedFunc onTerminatedFunc = nullptr;
    InputMethod_ErrorCode errorCode = OH_MessageHandlerProxy_GetOnTerminatedFunc(nullptr, &onTerminatedFunc);
    EXPECT_EQ(errorCode, IME_ERR_NULL_POINTER);
}

HWTEST_F(MessageHandlerProxyTest, GetOnTerminatedFunc_NullFunc) {
    InputMethod_MessageHandlerProxy *proxy = OH_MessageHandlerProxy_Create();
    InputMethod_ErrorCode errorCode = OH_MessageHandlerProxy_GetOnTerminatedFunc(proxy, nullptr);
    EXPECT_EQ(errorCode, IME_ERR_NULL_POINTER);
    OH_MessageHandlerProxy_Destroy(proxy);
}

HWTEST_F(MessageHandlerProxyTest, SetOnMessageFunc_ValidProxy) {
    InputMethod_MessageHandlerProxy *proxy = OH_MessageHandlerProxy_Create();
    InputMethod_ErrorCode errorCode = OH_MessageHandlerProxy_SetOnMessageFunc(proxy, MockOnMessageFunc);
    EXPECT_EQ(errorCode, IME_ERR_OK);
    OH_MessageHandlerProxy_Destroy(proxy);
}

HWTEST_F(MessageHandlerProxyTest, SetOnMessageFunc_NullProxy) {
    InputMethod_ErrorCode errorCode = OH_MessageHandlerProxy_SetOnMessageFunc(nullptr, MockOnMessageFunc);
    EXPECT_EQ(errorCode, IME_ERR_NULL_POINTER);
}

HWTEST_F(MessageHandlerProxyTest, SetOnMessageFunc_NullFunc) {
    InputMethod_MessageHandlerProxy *proxy = OH_MessageHandlerProxy_Create();
    InputMethod_ErrorCode errorCode = OH_MessageHandlerProxy_SetOnMessageFunc(proxy, nullptr);
    EXPECT_EQ(errorCode, IME_ERR_NULL_POINTER);
    OH_MessageHandlerProxy_Destroy(proxy);
}

HWTEST_F(MessageHandlerProxyTest, GetOnMessageFunc_ValidProxy) {
    InputMethod_MessageHandlerProxy *proxy = OH_MessageHandlerProxy_Create();
    OH_MessageHandlerProxy_SetOnMessageFunc(proxy, MockOnMessageFunc);
    OH_MessageHandlerProxy_OnMessageFunc onMessageFunc = nullptr;
    InputMethod_ErrorCode errorCode = OH_MessageHandlerProxy_GetOnMessageFunc(proxy, &onMessageFunc);
    EXPECT_EQ(errorCode, IME_ERR_OK);
    EXPECT_EQ(onMessageFunc, MockOnMessageFunc);
    OH_MessageHandlerProxy_Destroy(proxy);
}

HWTEST_F(MessageHandlerProxyTest, GetOnMessageFunc_NullProxy) {
    OH_MessageHandlerProxy_OnMessageFunc onMessageFunc = nullptr;
    InputMethod_ErrorCode errorCode = OH_MessageHandlerProxy_GetOnMessageFunc(nullptr, &onMessageFunc);
    EXPECT_EQ(errorCode, IME_ERR_NULL_POINTER);
}

HWTEST_F(MessageHandlerProxyTest, GetOnMessageFunc_NullFunc) {
    InputMethod_MessageHandlerProxy *proxy = OH_MessageHandlerProxy_Create();
    InputMethod_ErrorCode errorCode = OH_MessageHandlerProxy_GetOnMessageFunc(proxy, nullptr);
    EXPECT_EQ(errorCode, IME_ERR_NULL_POINTER);
    OH_MessageHandlerProxy_Destroy(proxy);
}