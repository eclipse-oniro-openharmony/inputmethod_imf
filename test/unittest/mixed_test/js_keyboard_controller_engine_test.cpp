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

#include "js_keyboard_controller_engine.h"

#include "gtest/gtest.h"
#include "input_method_ability.h"
#include "mock_async_call.h"
#include "mock_napi.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"


using namespace OHOS::MiscServices;

class JsKeyboardControllerEngineTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        napi_env env = MockNapi::CreateMockEnv();
        napi_value info = nullptr;
        napi_get_undefined(env, &info);
        JsKeyboardControllerEngine::Init(env, info);
        MockNapi::DestroyMockEnv(env);
    }

    static void TearDownTestCase()
    {
        // 如果需要，可以在这里添加任何清理代码
    }

    void SetUp() override
    {
        env = MockNapi::CreateMockEnv();
        napi_value info = nullptr;
        napi_get_undefined(env, &info);
        instance = JsKeyboardControllerEngine::GetKeyboardControllerInstance(env);
    }

    void TearDown() override
    {
        MockNapi::DestroyMockEnv(env);
    }

    napi_env env;
    napi_value instance;
};

TEST_F(JsKeyboardControllerEngineTest, Hide_Success)
{
    MockInputMethodAbility mockInputMethodAbility;
    mockInputMethodAbility.SetHideKeyboardSelfReturnCode(ErrorCode::NO_ERROR);

    napi_value result = JsKeyboardControllerEngine::Hide(env, instance);
    EXPECT_EQ(napi_ok, napi_get_undefined(env, &result));
}

TEST_F(JsKeyboardControllerEngineTest, Hide_Failure)
{
    MockInputMethodAbility mockInputMethodAbility;
    mockInputMethodAbility.SetHideKeyboardSelfReturnCode(ErrorCode::ERROR);

    napi_value result = JsKeyboardControllerEngine::Hide(env, instance);
    EXPECT_EQ(napi_ok, napi_get_undefined(env, &result));
}

TEST_F(JsKeyboardControllerEngineTest, HideKeyboard_Success)
{
    MockInputMethodAbility mockInputMethodAbility;

    napi_value result = JsKeyboardControllerEngine::HideKeyboard(env, instance);
    EXPECT_EQ(napi_ok, napi_get_undefined(env, &result));
}

TEST_F(JsKeyboardControllerEngineTest, ExitCurrentInputType_Success)
{
    MockInputMethodAbility mockInputMethodAbility;
    mockInputMethodAbility.SetExitCurrentInputTypeReturnCode(ErrorCode::NO_ERROR);

    napi_value result = JsKeyboardControllerEngine::ExitCurrentInputType(env, instance);
    EXPECT_EQ(napi_ok, napi_get_undefined(env, &result));
}

TEST_F(JsKeyboardControllerEngineTest, ExitCurrentInputType_Failure)
{
    MockInputMethodAbility mockInputMethodAbility;
    mockInputMethodAbility.SetExitCurrentInputTypeReturnCode(ErrorCode::ERROR);

    napi_value result = JsKeyboardControllerEngine::ExitCurrentInputType(env, instance);
    EXPECT_EQ(napi_ok, napi_get_undefined(env, &result));
}