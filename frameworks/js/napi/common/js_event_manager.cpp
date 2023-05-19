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

#include "js_event_manager.h"

#include "js_util.h"

namespace OHOS {
namespace MiscServices {
const std::unordered_map<EventSubscribeModule, std::unordered_set<std::string>> JsEventManager::EVENT_TYPES{
    { EventSubscribeModule::INPUT_METHOD_CONTROLLER,
        { "insertText", "deleteLeft", "deleteRight", "sendKeyboardStatus", "sendFunctionKey", "moveCursor",
            "handleExtendAction", "selectByRange", "selectByMovement" } },
    { EventSubscribeModule::INPUT_METHOD_SETTING, { "imeChange", "imeShow", "imeHide" } },
    { EventSubscribeModule::INPUT_METHOD_ABILITY,
        { "inputStart", "inputStop", "keyboardShow", "keyboardHide", "setCallingWindow", "setSubtype" } },
    { EventSubscribeModule::KEYBOARD_DELEGATE,
        { "keyDown", "keyUp", "cursorContextChange", "selectionChange", "textChange" } },
    { EventSubscribeModule::PANEL, { "show", "hide" } }
};
bool JsEventManager::GetEventType(EventSubscribeModule module, napi_env env, napi_value in, std::string &out)
{
    if (!JsUtil::GetValue(env, in, out)) {
        return false;
    }
    auto it = EVENT_TYPES.find(module);
    if (it == EVENT_TYPES.end()) {
        return false;
    }
    return it->second.find(out) != it->second.end();
}
} // namespace MiscServices
} // namespace OHOS