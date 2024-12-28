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

#include "gtest/gtest.h"
#include "native_inputmethod_types.h"
#include "inputmethod_texteditorproxy.h"

// Mock function for testing
void MockFunction() {}

// Test case for creating and destroying a proxy
TEST(TextEditorProxyTest, CreateAndDestroyProxy) {
    InputMethod_TextEditorProxy *proxy = OH_TextEditorProxy_Create();
    EXPECT_NE(proxy, nullptr);
    OH_TextEditorProxy_Destroy(proxy);
}

// Test case for setting a valid function pointer
TEST(TextEditorProxyTest, SetValidFunctionPointer) {
    InputMethod_TextEditorProxy *proxy = OH_TextEditorProxy_Create();
    InputMethod_ErrorCode errorCode = OH_TextEditorProxy_SetInsertTextFunc(proxy, MockFunction);
    EXPECT_EQ(errorCode, IME_ERR_OK);
    OH_TextEditorProxy_Destroy(proxy);
}

// Test case for setting a nullptr function pointer
TEST(TextEditorProxyTest, SetNullFunctionPointer) {
    InputMethod_TextEditorProxy *proxy = OH_TextEditorProxy_Create();
    InputMethod_ErrorCode errorCode = OH_TextEditorProxy_SetInsertTextFunc(proxy, nullptr);
    EXPECT_EQ(errorCode, IME_ERR_NULL_POINTER);
    OH_TextEditorProxy_Destroy(proxy);
}

// Test case for setting a function pointer with a nullptr proxy
TEST(TextEditorProxyTest, SetFunctionPointerWithNullProxy) {
    InputMethod_ErrorCode errorCode = OH_TextEditorProxy_SetInsertTextFunc(nullptr, MockFunction);
    EXPECT_EQ(errorCode, IME_ERR_NULL_POINTER);
}

// Test case for getting a valid function pointer
TEST(TextEditorProxyTest, GetValidFunctionPointer) {
    InputMethod_TextEditorProxy *proxy = OH_TextEditorProxy_Create();
    OH_TextEditorProxy_SetInsertTextFunc(proxy, MockFunction);
    OH_TextEditorProxy_InsertTextFunc func;
    InputMethod_ErrorCode errorCode = OH_TextEditorProxy_GetInsertTextFunc(proxy, &func);
    EXPECT_EQ(errorCode, IME_ERR_OK);
    EXPECT_EQ(func, MockFunction);
    OH_TextEditorProxy_Destroy(proxy);
}

// Test case for getting a function pointer with a nullptr proxy
TEST(TextEditorProxyTest, GetFunctionPointerWithNullProxy) {
    OH_TextEditorProxy_InsertTextFunc func;
    InputMethod_ErrorCode errorCode = OH_TextEditorProxy_GetInsertTextFunc(nullptr, &func);
    EXPECT_EQ(errorCode, IME_ERR_NULL_POINTER);
}

// Test case for getting a function pointer with a nullptr function pointer
TEST(TextEditorProxyTest, GetFunctionPointerWithNullFunctionPointer) {
    InputMethod_TextEditorProxy *proxy = OH_TextEditorProxy_Create();
    InputMethod_ErrorCode errorCode = OH_TextEditorProxy_GetInsertTextFunc(proxy, nullptr);
    EXPECT_EQ(errorCode, IME_ERR_NULL_POINTER);
    OH_TextEditorProxy_Destroy(proxy);
}