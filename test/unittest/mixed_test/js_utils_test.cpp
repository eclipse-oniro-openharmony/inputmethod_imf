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

#include "js_utils.h"

#include "gtest/gtest.h"
#include "mock_napi.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"


using namespace OHOS::MiscServices;

class JsUtilsTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        // 初始化模拟环境
        MockNapi::Init();
    }

    static void TearDownTestCase()
    {
        // 清理模拟环境
        MockNapi::CleanUp();
    }

    void SetUp() override
    {
        // 为每个测试设置模拟环境
        MockNapi::Reset();
    }

    void TearDown() override
    {
        // 清理每个测试的模拟环境
        MockNapi::Reset();
    }
};

TEST_F(JsUtilsTest, GetValue_ValidString_ReturnsOk)
{
    napi_env env = MockNapi::CreateEnv();
    napi_value value = MockNapi::CreateStringUtf8("testString");
    std::string out;

    napi_status status = JsUtils::GetValue(env, value, out);

    EXPECT_EQ(status, napi_ok);
    EXPECT_EQ(out, "testString");
}

TEST_F(JsUtilsTest, GetValue_InvalidType_ReturnsFailure)
{
    napi_env env = MockNapi::CreateEnv();
    napi_value value = MockNapi::CreateInt32(123); // 非字符串类型
    std::string out;

    napi_status status = JsUtils::GetValue(env, value, out);

    EXPECT_EQ(status, napi_generic_failure);
}

TEST_F(JsUtilsTest, GetValue_StringRetrievalFailure_ReturnsFailure)
{
    napi_env env = MockNapi::CreateEnv();
    napi_value value = MockNapi::CreateStringUtf8("testString");
    std::string out;

    // 模拟字符串检索失败
    MockNapi::SetStringUtf8Failure(true);

    napi_status status = JsUtils::GetValue(env, value, out);

    EXPECT_EQ(status, napi_generic_failure);
}