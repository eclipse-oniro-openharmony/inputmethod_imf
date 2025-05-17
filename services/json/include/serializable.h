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

#ifndef INPUT_METHOD_SERIALIZABLE_H
#define INPUT_METHOD_SERIALIZABLE_H
#include <string>
#include <unordered_set>

#include "cJSON.h"
#include "global.h"
namespace OHOS {
namespace MiscServices {
#ifndef GET_NAME
#define GET_NAME(value) #value
#endif
struct Serializable {
public:
    virtual ~Serializable(){};
    bool Unmarshall(const std::string &content);
    bool Marshall(std::string &content) const;
    virtual bool Unmarshal(cJSON *node)
    {
        return false;
    }
    virtual bool Marshal(cJSON *node) const
    {
        return false;
    }
    static bool GetValue(cJSON *node, const std::string &name, std::string &value);
    static bool GetValue(cJSON *node, const std::string &name, int32_t &value);
    static bool GetValue(cJSON *node, const std::string &name, uint32_t &value);
    static bool GetValue(cJSON *node, const std::string &name, bool &value);
    static bool GetValue(cJSON *node, const std::string &name, Serializable &value);
    static bool GetValue(cJSON *node, const std::string &name, std::vector<std::vector<std::string>> &values);
    template<typename T>
    static bool GetKeys(cJSON *nodes, std::vector<std::string> &keys, const std::function<bool(T)> &filter = nullptr)
    {
        if (nodes == nullptr) {
            IMSA_HILOGE("nodes nullptr");
            return false;
        }
        cJSON *child = nodes->child;
        bool result = true;
        while (child != nullptr) {
            std::string nodeKey = child->string;
            T value;
            bool ret = GetValue(child, "", value);
            if (ret && (filter == nullptr || filter(value))) {
                keys.push_back(nodeKey);
            }
            result = result && ret;
            child = child->next;
        }
        return result;
    }
    template<typename T>
    static bool GetValues(cJSON *nodes, std::vector<T> &values, const std::function<bool(T)> &filter = nullptr)
    {
        if (nodes == nullptr) {
            IMSA_HILOGE("nodes nullptr");
            return false;
        }
        cJSON *child = nodes->child;
        bool result = true;
        while (child != nullptr) {
            std::string nodeKey = child->string;
            T value;
            bool ret = GetValue(child, "", value);
            if (ret && (filter == nullptr || filter(value))) {
                values.push_back(value);
            }
            result = result && ret;
            child = child->next;
        }
        return result;
    }
    template<typename T>
    static bool GetValue(cJSON *node, const std::string &name, std::vector<T> &values, int32_t maxNum = 0)
    {
        auto subNode = GetSubNode(node, name);
        if (!cJSON_IsArray(subNode)) {
            IMSA_HILOGE("not array");
            return false;
        }
        auto size = cJSON_GetArraySize(subNode);
        IMSA_HILOGD("size:%{public}d, maxNum:%{public}d", size, maxNum);
        if (maxNum > 0 && size > maxNum) {
            size = maxNum;
        }
        values.resize(size);
        bool ret = true;
        for (int32_t i = 0; i < size; i++) {
            auto item = cJSON_GetArrayItem(subNode, i);
            if (item == NULL) {
                return false;
            }
            ret = GetValue(item, "", values[i]) && ret;
        }
        return ret;
    }
    template<typename T>
    static bool GetValue(cJSON *node, const std::string &name, std::unordered_set<T> &values, int32_t maxNum = 0)
    {
        auto subNode = GetSubNode(node, name);
        if (!cJSON_IsArray(subNode)) {
            IMSA_HILOGE("not array");
            return false;
        }
        auto size = cJSON_GetArraySize(subNode);
        IMSA_HILOGD("size:%{public}d, maxNum:%{public}d", size, maxNum);
        if (maxNum > 0 && size > maxNum) {
            size = maxNum;
        }
        bool result = true;
        for (int32_t i = 0; i < size; i++) {
            auto item = cJSON_GetArrayItem(subNode, i);
            if (item == NULL) {
                return false;
            }
            T value;
            bool ret = GetValue(item, "", value);
            if (ret && !values.count(value)) {
                values.insert(value);
            }
            result = ret && result;
        }
        return result;
    }
    static bool SetValue(cJSON *node, const std::string &name, const std::string &value);
    static bool SetValue(cJSON *node, const std::string &name, const int32_t &value);
    static bool SetValue(cJSON *node, const std::string &name, const bool &value);
    static bool SetValue(cJSON *node, const std::string &name, const Serializable &value);
    static bool SetValue(cJSON *node, const std::string &name, const std::vector<std::vector<std::string>> &values);
    static bool SetValue(cJSON *node, const std::string &name, const std::vector<std::string> &values);
    template<typename T> static bool SetValue(cJSON *node, const std::string &name, const std::vector<T> &values)
    {
        auto array = cJSON_CreateArray();
        for (const auto &value : values) {
            auto *item = cJSON_CreateObject();
            auto ret = value.Marshal(item);
            if (!ret || !cJSON_AddItemToArray(array, item)) {
                cJSON_Delete(item);
            }
        }
        auto ret = cJSON_AddItemToObject(node, name.c_str(), array);
        if (!ret) {
            cJSON_Delete(array);
        }
        return ret;
    }
    static cJSON *GetSubNode(cJSON *node, const std::string &name);
};
} // namespace MiscServices
} // namespace OHOS
#endif // INPUT_METHOD_SERIALIZABLE_H
