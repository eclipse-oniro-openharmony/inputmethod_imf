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

#include "param_checker.h"

#include <algorithm>

namespace OHOS {
namespace MiscServices {
const std::set<std::string> ParamChecker::EVENT_TYPE_IMS{ "imeChange", "imeShow", "imeHide" };
const std::set<std::string> ParamChecker::EVENT_TYPE_IMC{ "insertText", "deleteLeft", "deleteRight",
    "sendKeyboardStatus", "sendFunctionKey", "moveCursor", "handleExtendAction", "selectByRange", "selectByMovement" };
const std::set<std::string> ParamChecker::EVENT_TYPE_IMA{ "inputStart", "inputStop", "keyboardShow", "keyboardHide",
    "setCallingWindow", "setSubtype" };
const std::set<std::string> ParamChecker::EVENT_TYPE_KEYBOARD_DELEGATE{ "keyDown", "keyUp", "cursorContextChange",
    "selectionChange", "textChange" };
const std::set<std::string> ParamChecker::EVENT_TYPE_PANEL{ "show", "hide" };

const std::map<EventSubscribeModule, std::set<std::string>> ParamChecker::EVENT_TYPES{
    { EventSubscribeModule::INPUT_METHOD_CONTROLLER, EVENT_TYPE_IMC },
    { EventSubscribeModule::INPUT_METHOD_SETTING, EVENT_TYPE_IMS },
    { EventSubscribeModule::INPUT_METHOD_ABILITY, EVENT_TYPE_IMA },
    { EventSubscribeModule::KEYBOARD_DELEGATE, EVENT_TYPE_KEYBOARD_DELEGATE },
    { EventSubscribeModule::PANEL, EVENT_TYPE_PANEL }
};

bool ParamChecker::IsValidParamCount(size_t count, size_t expectCount)
{
    return count >= expectCount;
}

bool ParamChecker::IsValidParamType(napi_env env, napi_value param, napi_valuetype expectType)
{
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, param, &valueType);
    return expectType == valueType;
}

bool ParamChecker::IsValidEventType(EventSubscribeModule module, const std::string &type)
{
    auto it = std::find_if(
        EVENT_TYPES.begin(), EVENT_TYPES.end(), [module](const auto &evenType) { return module == evenType.first; });
    if (it == EVENT_TYPES.end()) {
        return false;
    }
    return it->second.find(type) != it->second.end();
}
} // namespace MiscServices
} // namespace OHOS