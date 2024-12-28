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
#include <vector>
#include <mutex>
#include <thread>
#include "js_keyboard_delegate_setting.h"
#include "mock_input_method_ability.h"
#include "mock_event_handler.h"
#include "mock_key_event_consumer_proxy.h"
#include "mock_napi_env.h"
#include "mock_napi_value.h"

using namespace OHOS::MiscServices;
using namespace testing;

class JsKeyboardDelegateSettingTest : public Test {
protected:
    void SetUp() override
    {
        env_ = std::make_shared<MockNapiEnv>();
        inputMethodAbility_ = std::make_shared<MockInputMethodAbility>();
        eventHandler_ = std::make_shared<MockEventHandler>();
        keyEventConsumerProxy_ = std::make_shared<MockKeyEventConsumerProxy>();
    }

    void TearDown() override
    {
        // Clean up resources if needed
    }

    std::shared_ptr<MockNapiEnv> env_;
    std::shared_ptr<MockInputMethodAbility> inputMethodAbility_;
    std::shared_ptr<MockEventHandler> eventHandler_;
    std::shared_ptr<MockKeyEventConsumerProxy> keyEventConsumerProxy_;
};

HWTEST_F(JsKeyboardDelegateSettingTest, Init_WithValidEnv_ShouldDefinePropertiesAndMethods) {
    napi_value exports = nullptr;
    napi_create_object(env_.get(), &exports);
    napi_value result = JsKeyboardDelegateSetting::Init(env_.get(), exports);
    EXPECT_NE(result, nullptr);
}

HWTEST_F(JsKeyboardDelegateSettingTest, GetKeyboardDelegateSetting_ShouldReturnSingletonInstance) {
    auto delegate1 = JsKeyboardDelegateSetting::GetKeyboardDelegateSetting();
    auto delegate2 = JsKeyboardDelegateSetting::GetKeyboardDelegateSetting();
    EXPECT_EQ(delegate1, delegate2);
}

HWTEST_F(JsKeyboardDelegateSettingTest, InitKeyboardDelegate_WithActiveIme_ShouldReturnTrue) {
    EXPECT_CALL(*inputMethodAbility_, IsCurrentIme()).WillOnce(Return(true));
    EXPECT_CALL(*inputMethodAbility_, SetKdListener(_)).WillOnce(Return());
    EXPECT_CALL(*eventHandler_, Current()).WillOnce(Return(eventHandler_));
    bool result = JsKeyboardDelegateSetting::InitKeyboardDelegate();
    EXPECT_TRUE(result);
}

HWTEST_F(JsKeyboardDelegateSettingTest, InitKeyboardDelegate_WithInactiveIme_ShouldReturnFalse) {
    EXPECT_CALL(*inputMethodAbility_, IsCurrentIme()).WillOnce(Return(false));
    bool result = JsKeyboardDelegateSetting::InitKeyboardDelegate();
    EXPECT_FALSE(result);
}

HWTEST_F(JsKeyboardDelegateSettingTest, Subscribe_WithValidEventType_ShouldRegisterCallback) {
    napi_value callback = nullptr;
    napi_create_function(env_.get(), nullptr, 0, [](napi_env, napi_callback_info) { return nullptr; },
        nullptr, &callback);
    napi_value thisVar = nullptr;
    napi_create_object(env_.get(), &thisVar);
    napi_callback_info info = { env_.get(), callback, thisVar, 2, { callback, callback }, nullptr };
    napi_value result = JsKeyboardDelegateSetting::Subscribe(env_.get(), info);
    EXPECT_NE(result, nullptr);
}

HWTEST_F(JsKeyboardDelegateSettingTest, UnSubscribe_WithValidEventType_ShouldUnregisterCallback) {
    napi_value callback = nullptr;
    napi_create_function(env_.get(), nullptr, 0, [](napi_env, napi_callback_info) { return nullptr; },
        nullptr, &callback);
    napi_value thisVar = nullptr;
    napi_create_object(env_.get(), &thisVar);
    napi_callback_info info = { env_.get(), callback, thisVar, 2, { callback, callback }, nullptr };
    napi_value result = JsKeyboardDelegateSetting::UnSubscribe(env_.get(), info);
    EXPECT_NE(result, nullptr);
}

HWTEST_F(JsKeyboardDelegateSettingTest, OnDealKeyEvent_WithValidKeyEvent_ShouldPostTask) {
    auto keyEvent = std::make_shared<MMI::KeyEvent>();
    EXPECT_CALL(*eventHandler_, PostTask(_, _, _, _)).WillOnce(Return());
    bool result = JsKeyboardDelegateSetting::OnDealKeyEvent(keyEvent, keyEventConsumerProxy_);
    EXPECT_TRUE(result);
}

HWTEST_F(JsKeyboardDelegateSettingTest, OnCursorUpdate_ShouldPostTask) {
    EXPECT_CALL(*eventHandler_, PostTask(_, _, _, _)).WillOnce(Return());
    JsKeyboardDelegateSetting::OnCursorUpdate(10, 20, 30);
}

HWTEST_F(JsKeyboardDelegateSettingTest, OnSelectionChange_ShouldPostTask) {
    EXPECT_CALL(*eventHandler_, PostTask(_, _, _, _)).WillOnce(Return());
    JsKeyboardDelegateSetting::OnSelectionChange(1, 2, 3, 4);
}

HWTEST_F(JsKeyboardDelegateSettingTest, OnTextChange_ShouldPostTask) {
    EXPECT_CALL(*eventHandler_, PostTask(_, _, _, _)).WillOnce(Return());
    JsKeyboardDelegateSetting::OnTextChange("test");
}

HWTEST_F(JsKeyboardDelegateSettingTest, OnEditorAttributeChange_ShouldPostTask) {
    InputAttribute inputAttribute;
    EXPECT_CALL(*eventHandler_, PostTask(_, _, _, _)).WillOnce(Return());
    JsKeyboardDelegateSetting::OnEditorAttributeChange(inputAttribute);
}