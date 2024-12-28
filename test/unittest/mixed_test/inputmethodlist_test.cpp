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

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "native_engine/native_engine.h"


// 模拟外部符号
extern "C" {
const char _binary_inputmethodlist_abc_start[] = "abc";
const char _binary_inputmethodlist_abc_end[] = "abc" + 3;
}

// 测试用例 1：buf 不为 nullptr
TEST(NAPI_inputMethodList_GetABCCode, BufNotNullptr)
{
    const char *buf = nullptr;
    int buflen = 0;

    NAPI_inputMethodList_GetABCCode(&buf, &buflen);

    EXPECT_EQ(buf, _binary_inputmethodlist_abc_start);
    EXPECT_EQ(buflen, _binary_inputmethodlist_abc_end - _binary_inputmethodlist_abc_start);
}

// 测试用例 2：buf 为 nullptr
TEST(NAPI_inputMethodList_GetABCCode, BufNullptr)
{
    const char *buf = nullptr;
    int buflen = 0;

    NAPI_inputMethodList_GetABCCode(nullptr, &buflen);

    EXPECT_EQ(buf, nullptr);
    EXPECT_EQ(buflen, 0);
}

// 测试用例 3：buflen 为 nullptr
TEST(NAPI_inputMethodList_GetABCCode, BufLenNullptr)
{
    const char *buf = nullptr;
    int buflen = 0;

    NAPI_inputMethodList_GetABCCode(&buf, nullptr);

    EXPECT_EQ(buf, _binary_inputmethodlist_abc_start);
    EXPECT_EQ(buflen, 0);
}