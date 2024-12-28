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

#include "inputmethod_extension_ability_napi.h"
#include "native_engine/native_engine.h"

// 假设二进制数据在测试环境中可用
extern const char _binary_inputmethod_extension_ability_js_start[];
extern const char _binary_inputmethod_extension_ability_js_end[];
extern const char _binary_inputmethod_extension_ability_abc_start[];
extern const char _binary_inputmethod_extension_ability_abc_end[];

TEST(InputMethodExtensionAbilityTest, GetJSCode_ValidPointers_ShouldSetCorrectValues)
{
    const char *buf = nullptr;
    int bufLen = 0;

    NAPI_InputMethodExtensionAbility_GetJSCode(&buf, &bufLen);

    EXPECT_EQ(buf, _binary_inputmethod_extension_ability_js_start);
    EXPECT_EQ(bufLen, _binary_inputmethod_extension_ability_js_end - _binary_inputmethod_extension_ability_js_start);
}

TEST(InputMethodExtensionAbilityTest, GetABCCode_ValidPointers_ShouldSetCorrectValues)
{
    const char *buf = nullptr;
    int bufLen = 0;

    NAPI_InputMethodExtensionAbility_GetABCCode(&buf, &bufLen);

    EXPECT_EQ(buf, _binary_inputmethod_extension_ability_abc_start);
    EXPECT_EQ(bufLen, _binary_inputmethod_extension_ability_abc_end - _binary_inputmethod_extension_ability_abc_start);
}

TEST(InputMethodExtensionAbilityTest, GetJSCode_NullPointers_ShouldNotModify)
{
    const char *buf = nullptr;
    int bufLen = 0;

    NAPI_InputMethodExtensionAbility_GetJSCode(nullptr, nullptr);

    EXPECT_EQ(buf, nullptr);
    EXPECT_EQ(bufLen, 0);
}

TEST(InputMethodExtensionAbilityTest, GetABCCode_NullPointers_ShouldNotModify)
{
    const char *buf = nullptr;
    int bufLen = 0;

    NAPI_InputMethodExtensionAbility_GetABCCode(nullptr, nullptr);

    EXPECT_EQ(buf, nullptr);
    EXPECT_EQ(bufLen, 0);
}

TEST(InputMethodExtensionAbilityTest, GetJSCode_NullBuf_ShouldSetBufLen)
{
    int bufLen = 0;

    NAPI_InputMethodExtensionAbility_GetJSCode(nullptr, &bufLen);

    EXPECT_EQ(bufLen, _binary_inputmethod_extension_ability_js_end - _binary_inputmethod_extension_ability_js_start);
}

TEST(InputMethodExtensionAbilityTest, GetABCCode_NullBuf_ShouldSetBufLen)
{
    int bufLen = 0;

    NAPI_InputMethodExtensionAbility_GetABCCode(nullptr, &bufLen);

    EXPECT_EQ(bufLen, _binary_inputmethod_extension_ability_abc_end - _binary_inputmethod_extension_ability_abc_start);
}

TEST(InputMethodExtensionAbilityTest, GetJSCode_NullBufLen_ShouldSetBuf)
{
    const char *buf = nullptr;

    NAPI_InputMethodExtensionAbility_GetJSCode(&buf, nullptr);

    EXPECT_EQ(buf, _binary_inputmethod_extension_ability_js_start);
}

TEST(InputMethodExtensionAbilityTest, GetABCCode_NullBufLen_ShouldSetBuf)
{
    const char *buf = nullptr;

    NAPI_InputMethodExtensionAbility_GetABCCode(&buf, nullptr);

    EXPECT_EQ(buf, _binary_inputmethod_extension_ability_abc_start);
}