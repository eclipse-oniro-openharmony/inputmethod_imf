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

#ifndef DISTRIBUTED_RDB_SERIALIZABLE_H
#define DISTRIBUTED_RDB_SERIALIZABLE_H
#include <string>
#include <vector>

#include "cJSON.h"
namespace OHOS {
namespace MiscServices {
#ifndef GET_NAME
#define GET_NAME(value) #value
#endif
struct Serializable {
public:
    static cJSON *ToJson(const std::string &jsonStr);
    static std::string ToStr(cJSON *node);
    static bool GetValue(cJSON *node, const std::string &name, std::string &value);
    static bool GetValue(cJSON *node, const std::string &name, int32_t &value);
    static bool GetValue(cJSON *node, const std::string &name, bool &value);
    static bool GetValue(
        cJSON *node, const std::vector<std::string> &names, std::vector<std::string> &values, uint32_t maxNum = 0);
    template<typename T>
    static bool GetValue(cJSON *node, const std::string &name, std::vector<T> &values, uint32_t maxNum = 0)
    {
        auto array = GetSubNode(node, name);
        if (cJSON_IsArray(array)) {
            return false;
        }
        auto size = cJSON_GetArraySize(array);
        if (size <= 0) {
            return false;
        }
        size = maxNum != 0 && size > maxNum ? maxNum : size;
        values.resize(size);
        for (int32_t i = 0; i < size; ++i) {
            auto item = cJSON_GetArrayItem(array, i);
            values[i].GetValue(item);
        }
        return true;
    }
    template<typename T> static bool GetValue(cJSON *node, const std::string &name, T &value)
    {
        auto object = GetSubNode(node, name);
        if (cJSON_IsObject(object)) {
            return false;
        }
        return value.GetValue(object);
    }

    static bool SetValue(cJSON *node, const std::string &name, const std::string &value);
    static bool SetValue(cJSON *node, const std::string &name, const int32_t &value);
    static bool SetValue(cJSON *node, const std::string &name, const bool &value);
    template<typename T> static bool SetValue(cJSON *node, const std::string &name, const std::vector<T> &values)
    {
        auto array = cJSON_AddArrayToObject(node, name.c_str());
        for (auto &value : values) {
            cJSON *item = cJSON_CreateObject(); // todo 释放内存
            cJSON_AddItemToArray(array, item);
            value.SetValue(item);
        }
        return true;
    }

private:
    static cJSON *GetSubNode(cJSON *node, const std::string &name);
};
} // namespace MiscServices
} // namespace OHOS
#endif // DISTRIBUTED_RDB_SERIALIZABLE_H
