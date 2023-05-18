/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_PARAM_CHECK_H
#define OHOS_PARAM_CHECK_H
#include <map>
#include <set>
#include <string>
#include <vector>

#include "napi/native_api.h"
#include "napi/native_node_api.h"
namespace OHOS::MiscServices {
enum class EventSubscribeModule : uint32_t {
    INPUT_METHOD_CONTROLLER = 0,
    INPUT_METHOD_SETTING = 1,
    INPUT_METHOD_ABILITY,
    KEYBOARD_DELEGATE,
    PANEL,
};
class ParamChecker {
public:
    static bool IsValidParamCount(size_t count, size_t expectCount);
    static bool IsValidParamType(napi_env env, napi_value param, napi_valuetype expectType);
    static bool IsValidEventType(EventSubscribeModule module, const std::string &type);

private:
    static const std::set<std::string> EVENT_TYPE_IMS;
    static const std::set<std::string> EVENT_TYPE_IMC;
    static const std::set<std::string> EVENT_TYPE_IMA;
    static const std::set<std::string> EVENT_TYPE_KEYBOARD_DELEGATE;
    static const std::set<std::string> EVENT_TYPE_PANEL;
    static const std::map<EventSubscribeModule, std::set<std::string>> EVENT_TYPES;
};
} // namespace OHOS::MiscServices
#endif // OHOS_PARAM_CHECK_H
