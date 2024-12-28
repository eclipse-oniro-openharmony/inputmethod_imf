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

#include <gtest/gtest.h>
#include "native_inputmethod_types.h"
#include "global.h"

HWTEST_F(InputMethod_PrivateCommandTest, Create_ValidKey_ShouldCreateObject)
{
    char key[] = "testKey";
    InputMethod_PrivateCommand *command = OH_PrivateCommand_Create(key, sizeof(key) - 1);
    EXPECT_NE(command, nullptr);
    OH_PrivateCommand_Destroy(command);
}

HWTEST_F(InputMethod_PrivateCommandTest, Create_EmptyKey_ShouldCreateObject)
{
    char key[] = "";
    InputMethod_PrivateCommand *command = OH_PrivateCommand_Create(key, 0);
    EXPECT_NE(command, nullptr);
    OH_PrivateCommand_Destroy(command);
}

HWTEST_F(InputMethod_PrivateCommandTest, Destroy_ValidCommand_ShouldNotCrash)
{
    char key[] = "testKey";
    InputMethod_PrivateCommand *command = OH_PrivateCommand_Create(key, sizeof(key) - 1);
    EXPECT_NE(command, nullptr);
    OH_PrivateCommand_Destroy(command);
}

HWTEST_F(InputMethod_PrivateCommandTest, Destroy_NullCommand_ShouldLogError)
{
    OH_PrivateCommand_Destroy(nullptr);
}

HWTEST_F(InputMethod_PrivateCommandTest, SetKey_ValidCommandAndKey_ShouldSetKey)
{
    char key[] = "testKey";
    InputMethod_PrivateCommand *command = OH_PrivateCommand_Create(key, sizeof(key) - 1);
    EXPECT_NE(command, nullptr);

    char newKey[] = "newKey";
    InputMethod_ErrorCode result = OH_PrivateCommand_SetKey(command, newKey, sizeof(newKey) - 1);
    EXPECT_EQ(result, IME_ERR_OK);

    const char *actualKey;
    size_t keyLength;
    result = OH_PrivateCommand_GetKey(command, &actualKey, &keyLength);
    EXPECT_EQ(result, IME_ERR_OK);
    EXPECT_STREQ(actualKey, newKey);
    EXPECT_EQ(keyLength, sizeof(newKey) - 1);

    OH_PrivateCommand_Destroy(command);
}

HWTEST_F(InputMethod_PrivateCommandTest, SetKey_NullCommand_ShouldReturnError)
{
    char key[] = "testKey";
    InputMethod_ErrorCode result = OH_PrivateCommand_SetKey(nullptr, key, sizeof(key) - 1);
    EXPECT_EQ(result, IME_ERR_NULL_POINTER);
}

HWTEST_F(InputMethod_PrivateCommandTest, SetKey_NullKey_ShouldReturnError)
{
    char key[] = "testKey";
    InputMethod_PrivateCommand *command = OH_PrivateCommand_Create(key, sizeof(key) - 1);
    EXPECT_NE(command, nullptr);

    InputMethod_ErrorCode result = OH_PrivateCommand_SetKey(command, nullptr, sizeof(key) - 1);
    EXPECT_EQ(result, IME_ERR_NULL_POINTER);

    OH_PrivateCommand_Destroy(command);
}

HWTEST_F(InputMethod_PrivateCommandTest, SetBoolValue_ValidCommand_ShouldSetValue)
{
    char key[] = "testKey";
    InputMethod_PrivateCommand *command = OH_PrivateCommand_Create(key, sizeof(key) - 1);
    EXPECT_NE(command, nullptr);

    InputMethod_ErrorCode result = OH_PrivateCommand_SetBoolValue(command, true);
    EXPECT_EQ(result, IME_ERR_OK);

    bool value;
    result = OH_PrivateCommand_GetBoolValue(command, &value);
    EXPECT_EQ(result, IME_ERR_OK);
    EXPECT_TRUE(value);

    OH_PrivateCommand_Destroy(command);
}

HWTEST_F(InputMethod_PrivateCommandTest, SetBoolValue_NullCommand_ShouldReturnError)
{
    InputMethod_ErrorCode result = OH_PrivateCommand_SetBoolValue(nullptr, true);
    EXPECT_EQ(result, IME_ERR_NULL_POINTER);
}

HWTEST_F(InputMethod_PrivateCommandTest, SetIntValue_ValidCommand_ShouldSetValue)
{
    char key[] = "testKey";
    InputMethod_PrivateCommand *command = OH_PrivateCommand_Create(key, sizeof(key) - 1);
    EXPECT_NE(command, nullptr);

    InputMethod_ErrorCode result = OH_PrivateCommand_SetIntValue(command, 42);
    EXPECT_EQ(result, IME_ERR_OK);

    int32_t value;
    result = OH_PrivateCommand_GetIntValue(command, &value);
    EXPECT_EQ(result, IME_ERR_OK);
    EXPECT_EQ(value, 42);

    OH_PrivateCommand_Destroy(command);
}

HWTEST_F(InputMethod_PrivateCommandTest, SetIntValue_NullCommand_ShouldReturnError)
{
    InputMethod_ErrorCode result = OH_PrivateCommand_SetIntValue(nullptr, 42);
    EXPECT_EQ(result, IME_ERR_NULL_POINTER);
}

HWTEST_F(InputMethod_PrivateCommandTest, SetStrValue_ValidCommandAndValue_ShouldSetValue)
{
    char key[] = "testKey";
    InputMethod_PrivateCommand *command = OH_PrivateCommand_Create(key, sizeof(key) - 1);
    EXPECT_NE(command, nullptr);

    char value[] = "testValue";
    InputMethod_ErrorCode result = OH_PrivateCommand_SetStrValue(command, value, sizeof(value) - 1);
    EXPECT_EQ(result, IME_ERR_OK);

    const char *actualValue;
    size_t valueLength;
    result = OH_PrivateCommand_GetStrValue(command, &actualValue, &valueLength);
    EXPECT_EQ(result, IME_ERR_OK);
    EXPECT_STREQ(actualValue, value);
    EXPECT_EQ(valueLength, sizeof(value) - 1);

    OH_PrivateCommand_Destroy(command);
}

HWTEST_F(InputMethod_PrivateCommandTest, SetStrValue_NullCommand_ShouldReturnError)
{
    char value[] = "testValue";
    InputMethod_ErrorCode result = OH_PrivateCommand_SetStrValue(nullptr, value, sizeof(value) - 1);
    EXPECT_EQ(result, IME_ERR_NULL_POINTER);
}

HWTEST_F(InputMethod_PrivateCommandTest, SetStrValue_NullValue_ShouldReturnError)
{
    char key[] = "testKey";
    InputMethod_PrivateCommand *command = OH_PrivateCommand_Create(key, sizeof(key) - 1);
    EXPECT_NE(command, nullptr);

    InputMethod_ErrorCode result = OH_PrivateCommand_SetStrValue(command, nullptr, 0);
    EXPECT_EQ(result, IME_ERR_NULL_POINTER);

    OH_PrivateCommand_Destroy(command);
}

HWTEST_F(InputMethod_PrivateCommandTest, GetKey_ValidCommand_ShouldReturnKey)
{
    char key[] = "testKey";
    InputMethod_PrivateCommand *command = OH_PrivateCommand_Create(key, sizeof(key) - 1);
    EXPECT_NE(command, nullptr);

    const char *actualKey;
    size_t keyLength;
    InputMethod_ErrorCode result = OH_PrivateCommand_GetKey(command, &actualKey, &keyLength);
    EXPECT_EQ(result, IME_ERR_OK);
    EXPECT_STREQ(actualKey, key);
    EXPECT_EQ(keyLength, sizeof(key) - 1);

    OH_PrivateCommand_Destroy(command);
}

HWTEST_F(InputMethod_PrivateCommandTest, GetKey_NullCommand_ShouldReturnError)
{
    const char *key;
    size_t keyLength;
    InputMethod_ErrorCode result = OH_PrivateCommand_GetKey(nullptr, &key, &keyLength);
    EXPECT_EQ(result, IME_ERR_NULL_POINTER);
}

HWTEST_F(InputMethod_PrivateCommandTest, GetKey_NullKey_ShouldReturnError)
{
    char key[] = "testKey";
    InputMethod_PrivateCommand *command = OH_PrivateCommand_Create(key, sizeof(key) - 1);
    EXPECT_NE(command, nullptr);

    size_t keyLength;
    InputMethod_ErrorCode result = OH_PrivateCommand_GetKey(command, nullptr, &keyLength);
    EXPECT_EQ(result, IME_ERR_NULL_POINTER);

    OH_PrivateCommand_Destroy(command);
}

HWTEST_F(InputMethod_PrivateCommandTest, GetKey_NullKeyLength_ShouldReturnError)
{
    char key[] = "testKey";
    InputMethod_PrivateCommand *command = OH_PrivateCommand_Create(key, sizeof(key) - 1);
    EXPECT_NE(command, nullptr);

    const char *actualKey;
    InputMethod_ErrorCode result = OH_PrivateCommand_GetKey(command, &actualKey, nullptr);
    EXPECT_EQ(result, IME_ERR_NULL_POINTER);

    OH_PrivateCommand_Destroy(command);
}

HWTEST_F(InputMethod_PrivateCommandTest, GetValueType_ValidCommand_ShouldReturnCorrectType)
{
    char key[] = "testKey";
    InputMethod_PrivateCommand *command = OH_PrivateCommand_Create(key, sizeof(key) - 1);
    EXPECT_NE(command, nullptr);

    InputMethod_ErrorCode result = OH_PrivateCommand_SetBoolValue(command, true);
    EXPECT_EQ(result, IME_ERR_OK);

    InputMethod_CommandValueType type;
    result = OH_PrivateCommand_GetValueType(command, &type);
    EXPECT_EQ(result, IME_ERR_OK);
    EXPECT_EQ(type, IME_COMMAND_VALUE_TYPE_BOOL);

    OH_PrivateCommand_Destroy(command);
}

HWTEST_F(InputMethod_PrivateCommandTest, GetValueType_NullCommand_ShouldReturnError)
{
    InputMethod_CommandValueType type;
    InputMethod_ErrorCode result = OH_PrivateCommand_GetValueType(nullptr, &type);
    EXPECT_EQ(result, IME_ERR_NULL_POINTER);
}

HWTEST_F(InputMethod_PrivateCommandTest, GetValueType_NullType_ShouldReturnError)
{
    char key[] = "testKey";
    InputMethod_PrivateCommand *command = OH_PrivateCommand_Create(key, sizeof(key) - 1);
    EXPECT_NE(command, nullptr);

    InputMethod_ErrorCode result = OH_PrivateCommand_GetValueType(command, nullptr);
    EXPECT_EQ(result, IME_ERR_NULL_POINTER);

    OH_PrivateCommand_Destroy(command);
}

HWTEST_F(InputMethod_PrivateCommandTest, GetBoolValue_ValidCommand_ShouldReturnBool)
{
    char key[] = "testKey";
    InputMethod_PrivateCommand *command = OH_PrivateCommand_Create(key, sizeof(key) - 1);
    EXPECT_NE(command, nullptr);

    InputMethod_ErrorCode result = OH_PrivateCommand_SetBoolValue(command, true);
    EXPECT_EQ(result, IME_ERR_OK);

    bool value;
    result = OH_PrivateCommand_GetBoolValue(command, &value);
    EXPECT_EQ(result, IME_ERR_OK);
    EXPECT_TRUE(value);

    OH_PrivateCommand_Destroy(command);
}

HWTEST_F(InputMethod_PrivateCommandTest, GetBoolValue_NullCommand_ShouldReturnError)
{
    bool value;
    InputMethod_ErrorCode result = OH_PrivateCommand_GetBoolValue(nullptr, &value);
    EXPECT_EQ(result, IME_ERR_NULL_POINTER);
}

HWTEST_F(InputMethod_PrivateCommandTest, GetBoolValue_NullValue_ShouldReturnError)
{
    char key[] = "testKey";
    InputMethod_PrivateCommand *command = OH_PrivateCommand_Create(key, sizeof(key) - 1);
    EXPECT_NE(command, nullptr);

    InputMethod_ErrorCode result = OH_PrivateCommand_GetBoolValue(command, nullptr);
    EXPECT_EQ(result, IME_ERR_NULL_POINTER);

    OH_PrivateCommand_Destroy(command);
}

HWTEST_F(InputMethod_PrivateCommandTest, GetBoolValue_NonBoolValue_ShouldReturnError)
{
    char key[] = "testKey";
    InputMethod_PrivateCommand *command = OH_PrivateCommand_Create(key, sizeof(key) - 1);
    EXPECT_NE(command, nullptr);

    InputMethod_ErrorCode result = OH_PrivateCommand_SetIntValue(command, 42);
    EXPECT_EQ(result, IME_ERR_OK);

    bool value;
    result = OH_PrivateCommand_GetBoolValue(command, &value);
    EXPECT_EQ(result, IME_ERR_QUERY_FAILED);

    OH_PrivateCommand_Destroy(command);
}

HWTEST_F(InputMethod_PrivateCommandTest, GetIntValue_ValidCommand_ShouldReturnInt)
{
    char key[] = "testKey";
    InputMethod_PrivateCommand *command = OH_PrivateCommand_Create(key, sizeof(key) - 1);
    EXPECT_NE(command, nullptr);

    InputMethod_ErrorCode result = OH_PrivateCommand_SetIntValue(command, 42);
    EXPECT_EQ(result, IME_ERR_OK);

    int32_t value;
    result = OH_PrivateCommand_GetIntValue(command, &value);
    EXPECT_EQ(result, IME_ERR_OK);
    EXPECT_EQ(value, 42);

    OH_PrivateCommand_Destroy(command);
}

HWTEST_F(InputMethod_PrivateCommandTest, GetIntValue_NullCommand_ShouldReturnError)
{
    int32_t value;
    InputMethod_ErrorCode result = OH_PrivateCommand_GetIntValue(nullptr, &value);
    EXPECT_EQ(result, IME_ERR_NULL_POINTER);
}

HWTEST_F(InputMethod_PrivateCommandTest, GetIntValue_NullValue_ShouldReturnError)
{
    char key[] = "testKey";
    InputMethod_PrivateCommand *command = OH_PrivateCommand_Create(key, sizeof(key) - 1);
    EXPECT_NE(command, nullptr);

    InputMethod_ErrorCode result = OH_PrivateCommand_GetIntValue(command, nullptr);
    EXPECT_EQ(result, IME_ERR_NULL_POINTER);

    OH_PrivateCommand_Destroy(command);
}

HWTEST_F(InputMethod_PrivateCommandTest, GetIntValue_NonIntValue_ShouldReturnError)
{
    char key[] = "testKey";
    InputMethod_PrivateCommand *command = OH_PrivateCommand_Create(key, sizeof(key) - 1);
    EXPECT_NE(command, nullptr);

    InputMethod_ErrorCode result = OH_PrivateCommand_SetBoolValue(command, true);
    EXPECT_EQ(result, IME_ERR_OK);

    int32_t value;
    result = OH_PrivateCommand_GetIntValue(command, &value);
    EXPECT_EQ(result, IME_ERR_QUERY_FAILED);

    OH_PrivateCommand_Destroy(command);
}

HWTEST_F(InputMethod_PrivateCommandTest, GetStrValue_ValidCommand_ShouldReturnString)
{
    char key[] = "testKey";
    InputMethod_PrivateCommand *command = OH_PrivateCommand_Create(key, sizeof(key) - 1);
    EXPECT_NE(command, nullptr);

    char value[] = "testValue";
    InputMethod_ErrorCode result = OH_PrivateCommand_SetStrValue(command, value, sizeof(value) - 1);
    EXPECT_EQ(result, IME_ERR_OK);

    const char *actualValue;
    size_t valueLength;
    result = OH_PrivateCommand_GetStrValue(command, &actualValue, &valueLength);
    EXPECT_EQ(result, IME_ERR_OK);
    EXPECT_STREQ(actualValue, value);
    EXPECT_EQ(valueLength, sizeof(value) - 1);

    OH_PrivateCommand_Destroy(command);
}

HWTEST_F(InputMethod_PrivateCommandTest, GetStrValue_NullCommand_ShouldReturnError)
{
    const char *value;
    size_t valueLength;
    InputMethod_ErrorCode result = OH_PrivateCommand_GetStrValue(nullptr, &value, &valueLength);
    EXPECT_EQ(result, IME_ERR_NULL_POINTER);
}

HWTEST_F(InputMethod_PrivateCommandTest, GetStrValue_NullValue_ShouldReturnError)
{
    char key[] = "testKey";
    InputMethod_PrivateCommand *command = OH_PrivateCommand_Create(key, sizeof(key) - 1);
    EXPECT_NE(command, nullptr);

    size_t valueLength;
    InputMethod_ErrorCode result = OH_PrivateCommand_GetStrValue(command, nullptr, &valueLength);
    EXPECT_EQ(result, IME_ERR_NULL_POINTER);

    OH_PrivateCommand_Destroy(command);
}

HWTEST_F(InputMethod_PrivateCommandTest, GetStrValue_NullValueLength_ShouldReturnError)
{
    char key[] = "testKey";
    InputMethod_PrivateCommand *command = OH_PrivateCommand_Create(key, sizeof(key) - 1);
    EXPECT_NE(command, nullptr);

    const char *value;
    InputMethod_ErrorCode result = OH_PrivateCommand_GetStrValue(command, &value, nullptr);
    EXPECT_EQ(result, IME_ERR_NULL_POINTER);

    OH_PrivateCommand_Destroy(command);
}

HWTEST_F(InputMethod_PrivateCommandTest, GetStrValue_NonStringValue_ShouldReturnError)
{
    char key[] = "testKey";
    InputMethod_PrivateCommand *command = OH_PrivateCommand_Create(key, sizeof(key) - 1);
    EXPECT_NE(command, nullptr);

    InputMethod_ErrorCode result = OH_PrivateCommand_SetIntValue(command, 42);
    EXPECT_EQ(result, IME_ERR_OK);

    const char *value;
    size_t valueLength;
    result = OH_PrivateCommand_GetStrValue(command, &value, &valueLength);
    EXPECT_EQ(result, IME_ERR_QUERY_FAILED);

    OH_PrivateCommand_Destroy(command);
}