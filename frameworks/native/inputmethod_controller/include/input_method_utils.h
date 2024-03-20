/*
 * Copyright (C) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_INPUT_METHOD_UTILS_H
#define FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_INPUT_METHOD_UTILS_H

#include <stdint.h>

#include <variant>

#include "global.h"
#include "input_attribute.h"
#include "panel_info.h"

namespace OHOS {
namespace MiscServices {
constexpr uint32_t INIT_WINDOW_ID = 0;
constexpr uint32_t INVALID_WINDOW_ID = INIT_WINDOW_ID - 1;
constexpr int32_t INVALID_VALUE = -1;
const constexpr size_t MAX_PRIVATE_COMMAND_SIZE = 32767;
const constexpr size_t MAX_PRIVATE_COMMAND_COUNT = 5;
enum class EnterKeyType {
    UNSPECIFIED = 0,
    NONE,
    GO,
    SEARCH,
    SEND,
    NEXT,
    DONE,
    PREVIOUS,
    NEW_LINE,
};

enum class TextInputType {
    NONE = -1,
    TEXT = 0,
    MULTILINE,
    NUMBER,
    PHONE,
    DATETIME,
    EMAIL_ADDRESS,
    URL,
    VISIBLE_PASSWORD,
    NUMBER_PASSWORD,
    SCREEN_LOCK_PASSWORD,
    USER_NAME,
    NEW_PASSWORD,
    NUMBER_DECIMAL,
};

enum class Direction {
    NONE = 0,
    UP = 1,
    DOWN,
    LEFT,
    RIGHT,
};

enum class SecurityMode {
    BASIC = 0,
    FULL = 1,
};

enum class ExtendAction {
    SELECT_ALL = 0,
    CUT = 3,
    COPY,
    PASTE,
};

class Configuration {
public:
    EnterKeyType GetEnterKeyType() const
    {
        return enterKeyType;
    }

    void SetEnterKeyType(EnterKeyType keyType)
    {
        enterKeyType = keyType;
    }

    TextInputType GetTextInputType() const
    {
        return textInputType;
    }

    void SetTextInputType(TextInputType textType)
    {
        textInputType = textType;
    }

private:
    EnterKeyType enterKeyType = EnterKeyType::UNSPECIFIED;
    TextInputType textInputType = TextInputType::TEXT;
};

struct CursorInfo {
    double left = -1.0;
    double top = -1.0;
    double width = -1.0;
    double height = -1.0;
    bool operator==(const CursorInfo &info) const
    {
        return (left == info.left && top == info.top && width == info.width && height == info.height);
    }
};

class KeyEvent {};

enum class KeyboardStatus : int32_t { NONE = 0, HIDE, SHOW }; // soft keyboard

enum Trigger : int32_t { IME_APP, IMF, END };
struct PanelStatusInfo {
    PanelInfo panelInfo;
    bool visible{ false };
    Trigger trigger{ END };
    bool operator==(const PanelStatusInfo &info) const
    {
        return info.panelInfo.panelFlag == panelInfo.panelFlag && info.panelInfo.panelType == panelInfo.panelType
               && info.visible == visible && info.trigger == trigger;
    }
};

class FunctionKey {
public:
    EnterKeyType GetEnterKeyType() const
    {
        return enterKeyType;
    }

    void SetEnterKeyType(EnterKeyType keyType)
    {
        enterKeyType = keyType;
    }

private:
    EnterKeyType enterKeyType = EnterKeyType::UNSPECIFIED;
};

struct SelectionRange {
    int32_t start = INVALID_VALUE;
    int32_t end = INVALID_VALUE;
};

struct TextSelection {
    int32_t oldBegin = INVALID_VALUE;
    int32_t oldEnd = INVALID_VALUE;
    int32_t newBegin = INVALID_VALUE;
    int32_t newEnd = INVALID_VALUE;
};

enum PrivateDataValueType : int32_t { VALUE_STRING = 0, VALUE_BOOL, VALUE_NUMBER };
using PrivateDataValue = std::variant<std::string, bool, int32_t>;

struct TextTotalConfig {
public:
    InputAttribute inputAttribute = {};
    CursorInfo cursorInfo = {};
    TextSelection textSelection = {};
    uint32_t windowId = INVALID_WINDOW_ID;
    double positionY = 0;
    double height = 0;
    std::unordered_map<std::string, PrivateDataValue> privateCommand = {};

    static bool IsPrivateCommandValid(const std::unordered_map<std::string, PrivateDataValue> &privateCommand)
    {
        size_t privateCommandSize = privateCommand.size();
        if (privateCommandSize == 0 || privateCommandSize > MAX_PRIVATE_COMMAND_COUNT) {
            return false;
        }
        size_t totalSize = 0;
        for (auto iter : privateCommand) {
            size_t keySize = sizeof(iter.first);
            size_t idx = iter.second.index();
            size_t valueSize = 0;

            if (idx == static_cast<size_t>(PrivateDataValueType::VALUE_STRING)) {
                valueSize = std::get<0>(iter.second).size();
            } else if (idx == static_cast<size_t>(PrivateDataValueType::VALUE_BOOL)) {
                valueSize = sizeof(std::get<1>(iter.second));
            } else if (idx == static_cast<size_t>(PrivateDataValueType::VALUE_NUMBER)) {
                valueSize = sizeof(std::get<2>(iter.second));
            }
            totalSize = totalSize + keySize + valueSize;
        }
        IMSA_HILOGI("totalSize : %{public}zu", totalSize);
        if (totalSize > MAX_PRIVATE_COMMAND_SIZE) {
            return false;
        }
        return true;
    }
};
struct TextConfig {
    InputAttribute inputAttribute = {};
    CursorInfo cursorInfo = {};
    SelectionRange range = {};
    uint32_t windowId = INVALID_WINDOW_ID;
    double positionY = 0;
    double height = 0;
    std::unordered_map<std::string, PrivateDataValue> privateCommand = {};
};

enum class InputType : int32_t { NONE = -1, CAMERA_INPUT = 0, SECURITY_INPUT, END };

enum class SwitchTrigger : uint32_t { CURRENT_IME = 0, SYSTEM_APP, IMSA };
} // namespace MiscServices
} // namespace OHOS
#endif // FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_INPUT_METHOD_UTILS_H
