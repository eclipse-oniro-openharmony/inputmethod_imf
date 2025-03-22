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

#include "event_checker.h"
#include <unordered_set>

namespace OHOS {
namespace MiscServices {
const std::unordered_set<std::string> EVENT_TYPES[static_cast<uint32_t>(EventSubscribeModule::MODULE_END)] = {
    [static_cast<uint32_t>(EventSubscribeModule::INPUT_METHOD_CONTROLLER)] = { "insertText", "deleteLeft",
        "deleteRight", "sendKeyboardStatus", "sendFunctionKey", "moveCursor", "handleExtendAction", "selectByRange",
        "selectByMovement", "getLeftTextOfCursor", "getRightTextOfCursor", "getTextIndexAtCursor", "setPreviewText",
        "finishTextPreview" },
    [static_cast<uint32_t>(EventSubscribeModule::INPUT_METHOD_SETTING)] = { "imeChange", "imeShow", "imeHide" },
    [static_cast<uint32_t>(EventSubscribeModule::INPUT_METHOD_ABILITY)] = { "inputStart", "inputStop", "keyboardShow",
        "keyboardHide", "setCallingWindow", "setSubtype", "securityModeChange", "privateCommand",
        "callingDisplayDidChange" },
    [static_cast<uint32_t>(EventSubscribeModule::KEYBOARD_DELEGATE)] = { "editorAttributeChanged", "keyDown", "keyUp",
        "keyEvent", "cursorContextChange", "selectionChange", "textChange" },
    [static_cast<uint32_t>(EventSubscribeModule::KEYBOARD_PANEL_MANAGER)] = { "panelPrivateCommand", "isPanelShow" },
    [static_cast<uint32_t>(EventSubscribeModule::PANEL)] = { "show", "hide", "sizeChange", "sizeUpdate" }
};

bool EventChecker::IsValidEventType(EventSubscribeModule module, const std::string &type)
{
    if (module < EventSubscribeModule::MODULE_BEGIN || module >= EventSubscribeModule::MODULE_END) {
        return false;
    }
    return EVENT_TYPES[static_cast<uint32_t>(module)].find(type) != EVENT_TYPES[static_cast<uint32_t>(module)].end();
}
} // namespace MiscServices
} // namespace OHOS