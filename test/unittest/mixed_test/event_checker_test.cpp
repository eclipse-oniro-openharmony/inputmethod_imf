/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>

#include <cstdint>
#include <string>
#include <unordered_set>

enum class EventSubscribeModule {
    MODULE_BEGIN = 0,
    INPUT_METHOD_CONTROLLER,
    INPUT_METHOD_SETTING,
    INPUT_METHOD_ABILITY,
    KEYBOARD_DELEGATE,
    KEYBOARD_PANEL_MANAGER,
    PANEL,
    MODULE_END
};

const std::unordered_set<std::string> EVENT_TYPES[static_cast<uint32_t>(EventSubscribeModule::MODULE_END)] = {
    [static_cast<uint32_t>(EventSubscribeModule::INPUT_METHOD_CONTROLLER)] = { "insertText", "deleteLeft",
        "deleteRight", "sendKeyboardStatus", "sendFunctionKey", "moveCursor", "handleExtendAction", "selectByRange",
        "selectByMovement", "getLeftTextOfCursor", "getRightTextOfCursor", "getTextIndexAtCursor" },
    [static_cast<uint32_t>(EventSubscribeModule::INPUT_METHOD_SETTING)] = { "imeChange", "imeShow", "imeHide" },
    [static_cast<uint32_t>(EventSubscribeModule::INPUT_METHOD_ABILITY)] = { "inputStart", "inputStop", "keyboardShow",
        "keyboardHide", "setCallingWindow", "setSubtype", "securityModeChange", "privateCommand" },
    [static_cast<uint32_t>(EventSubscribeModule::KEYBOARD_DELEGATE)] = { "editorAttributeChanged", "keyDown", "keyUp",
        "keyEvent", "cursorContextChange", "selectionChange", "textChange" },
    [static_cast<uint32_t>(EventSubscribeModule::KEYBOARD_PANEL_MANAGER)] = { "panelPrivateCommand", "isPanelShow" },
    [static_cast<uint32_t>(EventSubscribeModule::PANEL)] = { "show", "hide", "sizeChange" }
};

class EventChecker {
public:
    static bool IsValidEventType(EventSubscribeModule module, const std::string &type)
    {
        if (module < EventSubscribeModule::MODULE_BEGIN || module >= EventSubscribeModule::MODULE_END) {
            return false;
        }
        return EVENT_TYPES[static_cast<uint32_t>(module)].find(type)
               != EVENT_TYPES[static_cast<uint32_t>(module)].end();
    }
};

TEST(EventCheckerTest, IsValidEventType_InvalidModule_ReturnsFalse)
{
    EXPECT_FALSE(EventChecker::IsValidEventType(static_cast<EventSubscribeModule>(-1), "insertText"));
    EXPECT_FALSE(EventChecker::IsValidEventType(EventSubscribeModule::MODULE_END, "insertText"));
}

TEST(EventCheckerTest, IsValidEventType_ValidModuleTypeExists_ReturnsTrue)
{
    EXPECT_TRUE(EventChecker::IsValidEventType(EventSubscribeModule::INPUT_METHOD_CONTROLLER, "insertText"));
    EXPECT_TRUE(EventChecker::IsValidEventType(EventSubscribeModule::INPUT_METHOD_SETTING, "imeChange"));
    EXPECT_TRUE(EventChecker::IsValidEventType(EventSubscribeModule::INPUT_METHOD_ABILITY, "inputStart"));
    EXPECT_TRUE(EventChecker::IsValidEventType(EventSubscribeModule::KEYBOARD_DELEGATE, "editorAttributeChanged"));
    EXPECT_TRUE(EventChecker::IsValidEventType(EventSubscribeModule::KEYBOARD_PANEL_MANAGER, "panelPrivateCommand"));
    EXPECT_TRUE(EventChecker::IsValidEventType(EventSubscribeModule::PANEL, "show"));
}

TEST(EventCheckerTest, IsValidEventType_ValidModuleTypeDoesNotExist_ReturnsFalse)
{
    EXPECT_FALSE(EventChecker::IsValidEventType(EventSubscribeModule::INPUT_METHOD_CONTROLLER, "unknownEvent"));
    EXPECT_FALSE(EventChecker::IsValidEventType(EventSubscribeModule::INPUT_METHOD_SETTING, "unknownEvent"));
    EXPECT_FALSE(EventChecker::IsValidEventType(EventSubscribeModule::INPUT_METHOD_ABILITY, "unknownEvent"));
    EXPECT_FALSE(EventChecker::IsValidEventType(EventSubscribeModule::KEYBOARD_DELEGATE, "unknownEvent"));
    EXPECT_FALSE(EventChecker::IsValidEventType(EventSubscribeModule::KEYBOARD_PANEL_MANAGER, "unknownEvent"));
    EXPECT_FALSE(EventChecker::IsValidEventType(EventSubscribeModule::PANEL, "unknownEvent"));
}