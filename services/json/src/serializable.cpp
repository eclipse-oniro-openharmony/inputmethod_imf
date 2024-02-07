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
cJSON *Serializable::ToJson(const std::string &jsonStr)
{
    auto root = cJSON_Parse(jsonStr.c_str());
    if (root == nullptr) {
        IMSA_HILOGE("%{public}s: parse failed", jsonStr.c_str());
    }
    return root;
}

std::string Serializable::ToStr(cJSON *node)
{
    return cJSON_PrintUnformatted(node);
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

bool Serializable::GetValue(
    cJSON *node, const std::vector<std::string> &names, std::vector<std::string> &values, uint32_t maxNum)
{
    auto subNode = node;
    for (const auto &name : names) {
        auto subNodeTemp = GetSubNode(subNode, name);
        subNode = subNodeTemp;
    }
    if (cJSON_IsArray(subNode)) {
        return false;
    }
    auto size = cJSON_GetArraySize(subNode);
    if (size <= 0) {
        return false;
    }
    size = maxNum != 0 && size > maxNum ? maxNum : size;
    values.resize(size);
    for (int32_t i = 0; i < size; ++i) {
        auto item = cJSON_GetArrayItem(subNode, i);
        values[i] = item->valuestring;
    }
    return true;
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

bool Serializable::SetValue(cJSON *node, const std::string &name, const bool &value)
{
    cJSON_AddBoolToObject(node, name.c_str(), value);
    return true;
}

cJSON *Serializable::GetSubNode(cJSON *node, const std::string &name)
{
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
