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

#include "js_panel.h"

#include "gtest/gtest.h"
#include "mock_napi.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"

using namespace OHOS::MiscServices;
using namespace testing;

class JsPanelTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        // 在所有测试用例之前设置任何共享资源
    }

    static void TearDownTestCase()
    {
        // 在所有测试用例之后清理任何共享资源
    }

    void SetUp() override
    {
        // 在每个测试之前设置任何特定资源
        MockNapi::Init();
    }

    void TearDown() override
    {
        // 在每个测试之后清理任何特定资源
        MockNapi::Reset();
    }
};

TEST_F(JsPanelTest, Init_AlreadyInitialized_ReturnsExistingConstructor)
{
    napi_env env = MockNapi::CreateEnv();
    napi_value existingConstructor = MockNapi::CreateObject();
    JsPanel::panelConstructorRef_ = MockNapi::CreateReference(existingConstructor, 1);

    napi_value result = JsPanel::Init(env);

    EXPECT_EQ(result, existingConstructor);
    MockNapi::DestroyEnv(env);
}

TEST_F(JsPanelTest, Init_NotInitialized_DefinesNewClass)
{
    napi_env env = MockNapi::CreateEnv();
    napi_value newConstructor = MockNapi::CreateObject();
    MockNapi::SetDefineClassResult(newConstructor);

    napi_value result = JsPanel::Init(env);

    EXPECT_EQ(result, newConstructor);
    MockNapi::DestroyEnv(env);
}

TEST_F(JsPanelTest, Init_DefineClassFails_ReturnsNull)
{
    napi_env env = MockNapi::CreateEnv();
    MockNapi::SetDefineClassResult(nullptr);

    napi_value result = JsPanel::Init(env);

    EXPECT_EQ(result, nullptr);
    MockNapi::DestroyEnv(env);
}

TEST_F(JsPanelTest, Init_CreateReferenceFails_ReturnsNull)
{
    napi_env env = MockNapi::CreateEnv();
    napi_value newConstructor = MockNapi::CreateObject();
    MockNapi::SetDefineClassResult(newConstructor);
    MockNapi::SetCreateReferenceResult(nullptr);

    napi_value result = JsPanel::Init(env);

    EXPECT_EQ(result, nullptr);
    MockNapi::DestroyEnv(env);
}