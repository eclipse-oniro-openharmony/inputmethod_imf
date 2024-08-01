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

namespace OHOS {
namespace MiscServices {
const std::unordered_set<std::string> EventChecker::EVENT_TYPES[static_cast<uint32_t>(EventSubscribeModule::MODULE_END)] = {
    { { "insertText", "deleteLeft", "deleteRight", "sendKeyboardStatus", "sendFunctionKey", "moveCursor",
        "handleExtendAction", "selectByRange", "selectByMovement", "getLeftTextOfCursor", "getRightTextOfCursor",
        "getTextIndexAtCursor" } },
    { { "imeChange", "imeShow", "imeHide" } },
    { { "inputStart", "inputStop", "keyboardShow", "keyboardHide", "setCallingWindow", "setSubtype",
        "securityModeChange", "privateCommand" } },
    { { "editorAttributeChanged", "keyDown", "keyUp", "keyEvent", "cursorContextChange", "selectionChange",
        "textChange" } },
    { { "show", "hide", "sizeChange" } }, { { "panelPrivateCommand", "isPanelShow" } }
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