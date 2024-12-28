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

#include "serializable.h"

#include <gtest/gtest.h>

#include <string>

#include "cJSON.h"

using namespace OHOS::MiscServices;

class TestSerializable : public Serializable {
public:
    bool Unmarshal(cJSON *root) override
    {
        return true;
    }

    bool Marshal(cJSON *root) const override
    {
        return true;
    }
};

/**
 * @tc.name: Unmarshall_ValidJson_ReturnsTrue
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SerializableTest, Unmarshall_ValidJson_ReturnsTrue, TestSize.Level0)
{
    TestSerializable obj;
    std::string json = "{\"key\":\"value\"}";
    EXPECT_TRUE(obj.Unmarshall(json));
}

/**
 * @tc.name: Unmarshall_InvalidJson_ReturnsFalse
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SerializableTest, Unmarshall_InvalidJson_ReturnsFalse, TestSize.Level0)
{
    TestSerializable obj;
    std::string json = "invalid json";
    EXPECT_FALSE(obj.Unmarshall(json));
}

/**
 * @tc.name: Marshall_ValidObject_ReturnsTrue
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SerializableTest, Marshall_ValidObject_ReturnsTrue, TestSize.Level0)
{
    TestSerializable obj;
    std::string json;
    EXPECT_TRUE(obj.Marshall(json));
}

/**
 * @tc.name: GetValue_ValidString_ReturnsTrue
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SerializableTest, GetValue_ValidString_ReturnsTrue, TestSize.Level0)
{
    TestSerializable obj;
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "key", "value");
    std::string value;
    EXPECT_TRUE(obj.GetValue(root, "key", value));
    EXPECT_EQ(value, "value");
    cJSON_Delete(root);
}

/**
 * @tc.name: GetValue_InvalidType_ReturnsFalse
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SerializableTest, GetValue_InvalidType_ReturnsFalse, TestSize.Level0)
{
    TestSerializable obj;
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "key", 123);
    std::string value;
    EXPECT_FALSE(obj.GetValue(root, "key", value));
    cJSON_Delete(root);
}

/**
 * @tc.name: SetValue_ValidString_ReturnsTrue
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SerializableTest, SetValue_ValidString_ReturnsTrue, TestSize.Level0)
{
    TestSerializable obj;
    cJSON *root = cJSON_CreateObject();
    EXPECT_TRUE(obj.SetValue(root, "key", "value"));
    cJSON_Delete(root);
}

/**
 * @tc.name: GetSubNode_ValidNode_ReturnsNode
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SerializableTest, GetSubNode_ValidNode_ReturnsNode, TestSize.Level0)
{
    TestSerializable obj;
    cJSON *root = cJSON_CreateObject();
    cJSON *subNode = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "key", subNode);
    EXPECT_EQ(subNode, obj.GetSubNode(root, "key"));
    cJSON_Delete(root);
}

/**
 * @tc.name: GetSubNode_InvalidParent_ReturnsNull
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SerializableTest, GetSubNode_InvalidParent_ReturnsNull, TestSize.Level0)
{
    TestSerializable obj;
    cJSON *root = cJSON_CreateArray();
    EXPECT_EQ(nullptr, obj.GetSubNode(root, "key"));
    cJSON_Delete(root);
}