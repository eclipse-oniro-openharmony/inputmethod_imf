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

#include "gtest/gtest.h"
#include "js_input_method_engine_setting.h"
#include "mock_input_method_ability.h"
#include "mock_js_utils.h"
#include "mock_napi.h"
#include "mock_event_handler.h"

using namespace OHOS;
using namespace MiscServices;

class JsInputMethodEngineSettingTest : public testing::Test {
protected:
    void SetUp() override
    {
        // Initialize mocks and set up test environment
        MockInputMethodAbility::Init();
        MockJsUtils::Init();
        MockNapi::Init();
        MockEventHandler::Init();
    }

    void TearDown() override
    {
        // Clean up mocks and test environment
        MockInputMethodAbility::Reset();
        MockJsUtils::Reset();
        MockNapi::Reset();
        MockEventHandler::Reset();
    }
};

HWTEST_F(JsInputMethodEngineSettingTest, Init_ShouldDefineProperties) {
    napi_env env = MockNapi::CreateEnv();
    napi_value exports = MockNapi::CreateObject(env);
    napi_value result = JsInputMethodEngineSetting::Init(env, exports);

    // Verify that properties are defined on the exports object
    EXPECT_TRUE(MockNapi::HasProperty(env, exports, "ENTER_KEY_TYPE_UNSPECIFIED"));
    EXPECT_TRUE(MockNapi::HasProperty(env, exports, "getInputMethodEngine"));
    // Add more property checks as needed
}

HWTEST_F(JsInputMethodEngineSettingTest, Subscribe_InvalidType_ShouldReturnNull) {
    napi_env env = MockNapi::CreateEnv();
    napi_value thisVar = MockNapi::CreateObject(env);
    napi_value callback = MockNapi::CreateFunction(env, "callback");

    napi_value args[] = { MockNapi::CreateStringUtf8(env, "invalidType"), callback };
    napi_callback_info info = MockNapi::CreateCallbackInfo(env, thisVar, args, 2);

    napi_value result = JsInputMethodEngineSetting::Subscribe(env, info);

    // Verify that the result is null for invalid type
    EXPECT_TRUE(MockNapi::IsNull(env, result));
}

HWTEST_F(JsInputMethodEngineSettingTest, UnSubscribe_ValidType_ShouldUnregisterListener) {
    napi_env env = MockNapi::CreateEnv();
    napi_value thisVar = MockNapi::CreateObject(env);
    napi_value callback = MockNapi::CreateFunction(env, "callback");

    napi_value args[] = { MockNapi::CreateStringUtf8(env, "validType"), callback };
    napi_callback_info info = MockNapi::CreateCallbackInfo(env, thisVar, args, 2);

    napi_value result = JsInputMethodEngineSetting::UnSubscribe(env, info);

    // Verify that the result is null after successful unsubscription
    EXPECT_TRUE(MockNapi::IsNull(env, result));
}

HWTEST_F(JsInputMethodEngineSettingTest, CreatePanel_ValidContext_ShouldCreatePanel) {
    napi_env env = MockNapi::CreateEnv();
    napi_value thisVar = MockNapi::CreateObject(env);
    napi_value context = MockNapi::CreateObject(env);
    napi_value panelInfo = MockNapi::CreateObject(env);

    napi_value args[] = { context, panelInfo };
    napi_callback_info info = MockNapi::CreateCallbackInfo(env, thisVar, args, 2);

    napi_value result = JsInputMethodEngineSetting::CreatePanel(env, info);

    // Verify that the result is a valid panel object
    EXPECT_TRUE(MockNapi::IsObject(env, result));
}

HWTEST_F(JsInputMethodEngineSettingTest, DestroyPanel_ValidPanel_ShouldDestroyPanel) {
    napi_env env = MockNapi::CreateEnv();
    napi_value thisVar = MockNapi::CreateObject(env);
    napi_value panel = MockNapi::CreateObject(env);

    napi_value args[] = { panel };
    napi_callback_info info = MockNapi::CreateCallbackInfo(env, thisVar, args, 1);

    napi_value result = JsInputMethodEngineSetting::DestroyPanel(env, info);

    // Verify that the result is null after successful destruction
    EXPECT_TRUE(MockNapi::IsNull(env, result));
}

HWTEST_F(JsInputMethodEngineSettingTest, GetSecurityMode_ShouldReturnSecurityMode) {
    napi_env env = MockNapi::CreateEnv();
    napi_value thisVar = MockNapi::CreateObject(env);

    napi_callback_info info = MockNapi::CreateCallbackInfo(env, thisVar, nullptr, 0);

    napi_value result = JsInputMethodEngineSetting::GetSecurityMode(env, info);

    // Verify that the result is an integer representing the security mode
    EXPECT_TRUE(MockNapi::IsInt32(env, result));
}