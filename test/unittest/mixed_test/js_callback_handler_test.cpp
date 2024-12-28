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

#include "js_callback_handler.h"

#include <gtest/gtest.h>

#include <memory>
#include <thread>

#include "global.h"


using namespace OHOS::MiscServices;

class MockNapiEnv {
public:
    napi_env env;
    napi_value global;
    napi_value callback;
    napi_ref callbackRef;

    MockNapiEnv()
    {
        env = reinterpret_cast<napi_env>(this);
        global = reinterpret_cast<napi_value>(this);
        callback = reinterpret_cast<napi_value>(this);
        callbackRef = reinterpret_cast<napi_ref>(this);
    }

    napi_status napi_get_reference_value(napi_ref ref, napi_value *result)
    {
        *result = callback;
        return napi_ok;
    }

    napi_status napi_get_global(napi_value *result)
    {
        *result = global;
        return napi_ok;
    }

    napi_status napi_call_function(
        napi_value receiver, napi_value jsfunction, size_t argc, napi_value *argv, napi_value *result)
    {
        *result = callback;
        return napi_ok;
    }
};

class JsCallbackHandlerTest : public testing::Test {
protected:
    void SetUp() override
    {
        mockEnv = std::make_shared<MockNapiEnv>();
        callbackObject = std::make_shared<JSCallbackObject>();
        callbackObject->env_ = mockEnv->env;
        callbackObject->callback_ = mockEnv->callbackRef;
        callbackObject->threadId_ = std::this_thread::get_id();
    }

    std::shared_ptr<MockNapiEnv> mockEnv;
    std::shared_ptr<JSCallbackObject> callbackObject;
};

TEST_F(JsCallbackHandlerTest, Execute_ThreadIdsMatch_CallbackExecuted)
{
    napi_value output = nullptr;
    ArgContainer argContainer = { 0, nullptr };

    JsCallbackHandler::Execute(callbackObject, argContainer, output);

    EXPECT_NE(output, nullptr);
}

TEST_F(JsCallbackHandlerTest, Execute_ThreadIdsDoNotMatch_NoCallbackExecution)
{
    napi_value output = nullptr;
    ArgContainer argContainer = { 0, nullptr };
    callbackObject->threadId_ = std::thread::id();

    JsCallbackHandler::Execute(callbackObject, argContainer, output);

    EXPECT_EQ(output, nullptr);
}

TEST_F(JsCallbackHandlerTest, Execute_ArgvProviderReturnsFalse_NoCallbackExecution)
{
    napi_value output = nullptr;
    ArgContainer argContainer = { 0, [](napi_env, napi_value *, size_t) { return false; } };

    JsCallbackHandler::Execute(callbackObject, argContainer, output);

    EXPECT_EQ(output, nullptr);
}

TEST_F(JsCallbackHandlerTest, Execute_CallbackReferenceIsNull_NoCallbackExecution)
{
    napi_value output = nullptr;
    ArgContainer argContainer = { 0, nullptr };
    callbackObject->callback_ = nullptr;

    JsCallbackHandler::Execute(callbackObject, argContainer, output);

    EXPECT_EQ(output, nullptr);
}

TEST_F(JsCallbackHandlerTest, Execute_CallbackFunctionCallFails_OutputIsNull)
{
    napi_value output = nullptr;
    ArgContainer argContainer = { 0, nullptr };

    EXPECT_CALL(*mockEnv, napi_call_function).WillOnce(testing::Return(napi_generic_failure));

    JsCallbackHandler::Execute(callbackObject, argContainer, output);

    EXPECT_EQ(output, nullptr);
}