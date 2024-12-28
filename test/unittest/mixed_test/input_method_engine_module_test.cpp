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
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "js_input_method_engine_setting.h"
#include "js_keyboard_controller_engine.h"
#include "js_keyboard_delegate_setting.h"
#include "js_text_input_client_engine.h"

using namespace testing;

namespace OHOS {
namespace MiscServices {
namespace {
class MockJsInputMethodEngineSetting {
public:
    static bool initCalled;
    static napi_value Init(napi_env env, napi_value exports) {
        initCalled = true;
        return exports;
    }
};

bool MockJsInputMethodEngineSetting::initCalled = false;

class MockJsKeyboardControllerEngine {
public:
    static bool initCalled;
    static napi_value Init(napi_env env, napi_value exports) {
        initCalled = true;
        return exports;
    }
};

bool MockJsKeyboardControllerEngine::initCalled = false;

class MockJsTextInputClientEngine {
public:
    static bool initCalled;
    static napi_value Init(napi_env env, napi_value exports) {
        initCalled = true;
        return exports;
    }
};

bool MockJsTextInputClientEngine::initCalled = false;


class MockJsKeyboardDelegateSetting {
public:
    static bool initCalled;
    static napi_value Init(napi_env env, napi_value exports)
    {
        initCalled = true;
        return exports;
    }
};

HWTEST_F(InitTest, Init_CallsAllInitMethods_ReturnsExports) {
    napi_env env = nullptr;
    napi_value exports = nullptr;

    // Mock the Init methods of the classes
    ON_CALL(MockJsInputMethodEngineSetting, Init(env, exports)).WillByDefault(Return(exports));
    ON_CALL(MockJsKeyboardControllerEngine, Init(env, exports)).WillByDefault(Return(exports));
    ON_CALL(MockJsTextInputClientEngine, Init(env, exports)).WillByDefault(Return(exports));
    ON_CALL(MockJsKeyboardDelegateSetting, Init(env, exports)).WillByDefault(Return(exports));

    napi_value result = Init(env, exports);

    EXPECT_TRUE(MockJsInputMethodEngineSetting::initCalled);
    EXPECT_TRUE(MockJsKeyboardControllerEngine::initCalled);
    EXPECT_TRUE(MockJsTextInputClientEngine::initCalled);
    EXPECT_TRUE(MockJsKeyboardDelegateSetting::initCalled);
    EXPECT_EQ(result, exports);
}
} // namespace MiscServices
} // namespace OHOS