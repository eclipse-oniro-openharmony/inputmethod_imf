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

#include "global.h"
namespace OHOS {
namespace MiscServices {
bool Serializable::Unmarshall(const std::string &content)
{
    auto root = cJSON_Parse(content.c_str());
    if (root == nullptr) {
        return false;
    }
    auto ret = Unmarshal(root);
    cJSON_Delete(root);
    return ret;
}

bool Serializable::GetValue(cJSON *node, const std::string &name, std::string &value)
{
    auto subNode = GetSubNode(node, name);
    if (subNode == nullptr) {
        return false;
    }
    if (!cJSON_IsString(subNode)) {
        return false;
    }
    value = subNode->valuestring;
    return true;
}

bool Serializable::GetValue(cJSON *node, const std::string &name, int32_t &value)
{
    auto subNode = GetSubNode(node, name);
    if (subNode == nullptr) {
        return false;
    }
    if (!cJSON_IsNumber(subNode)) {
        return false;
    }
    value = subNode->valueint;
    return true;
}

bool Serializable::GetValue(cJSON *node, const std::string &name, bool &value)
{
    auto subNode = GetSubNode(node, name);
    if (subNode == nullptr) {
        return false;
    }
    if (!cJSON_IsBool(subNode)) {
        return false;
    }
    value = false;
    if (subNode->type == cJSON_True) {
        value = true;
    }
    return true;
}

bool Serializable::GetValue(cJSON *node, const std::string &name, Serializable &value)
{
    auto object = GetSubNode(node, name);
    if (cJSON_IsObject(object)) {
        return false;
    }
    return value.Unmarshal(object);
}

bool Serializable::SetValue(cJSON *node, const std::string &name, const std::string &value)
{
    cJSON_AddStringToObject(node, name.c_str(), value.c_str());
    return true;
}

bool Serializable::SetValue(cJSON *node, const std::string &name, const int32_t &value)
{
    cJSON_AddNumberToObject(node, name.c_str(), value);
    return true;
}

cJSON *Serializable::GetSubNode(cJSON *node, const std::string &name)
{
    if (name.empty()) {
        return node;
    }
    if (!cJSON_IsObject(node)) {
        IMSA_HILOGE("The json is not object");
        return nullptr;
    }
    if (!cJSON_HasObjectItem(node, name.c_str())) {
        IMSA_HILOGE("subNode: %{public}s not contain", name.c_str());
        return nullptr;
    }
    auto *subNode = cJSON_GetObjectItem(node, name.c_str());
    if (subNode == nullptr) {
        IMSA_HILOGE("subNode: %{public}s is error", name.c_str());
    }
    return subNode;
}
} // namespace MiscServices
} // namespace OHOS
