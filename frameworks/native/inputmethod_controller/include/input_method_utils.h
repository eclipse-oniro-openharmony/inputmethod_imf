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
constexpr size_t MAX_PRIVATE_COMMAND_SIZE = 32 * 1024; // 32K
constexpr size_t MAX_PRIVATE_COMMAND_COUNT = 5;
const constexpr char* SYSTEM_CMD_KEY = "sys_cmd";
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

struct Range {
    int32_t start = INVALID_VALUE;
    int32_t end = INVALID_VALUE;
    bool operator==(const Range &range) const
    {
        return start == range.start && end == range.end;
    }
};

struct TextSelection {
    int32_t oldBegin = INVALID_VALUE;
    int32_t oldEnd = INVALID_VALUE;
    int32_t newBegin = INVALID_VALUE;
    int32_t newEnd = INVALID_VALUE;
};

enum PrivateDataValueType : int32_t { VALUE_TYPE_STRING = 0, VALUE_TYPE_BOOL, VALUE_TYPE_NUMBER };
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
};
struct TextConfig {
    InputAttribute inputAttribute = {};
    CursorInfo cursorInfo = {};
    Range range = {};
    uint32_t windowId = INVALID_WINDOW_ID;
    double positionY = 0;
    double height = 0;
    std::unordered_map<std::string, PrivateDataValue> privateCommand = {};

    std::string ToString() const
    {
        std::string config;
        config.append("pattern/enterKey/previewSupport: " + std::to_string(inputAttribute.inputPattern) + "/"
                      + std::to_string(inputAttribute.enterKeyType) + "/"
                      + std::to_string(inputAttribute.isTextPreviewSupported));
        config.append(" windowId/y/height: " + std::to_string(windowId) + "/" + std::to_string(positionY) + "/"
                      + std::to_string(height));
        config.append(" range: " + std::to_string(range.start) + "/" + std::to_string(range.end));
        config.append(" cursor: " + std::to_string(cursorInfo.left) + "/" + std::to_string(cursorInfo.top) + "/"
                      + std::to_string(cursorInfo.width) + "/" + std::to_string(cursorInfo.height));
        return config;
    }

    static bool IsPrivateCommandValid(const std::unordered_map<std::string, PrivateDataValue> &privateCommand)
    {
        size_t privateCommandSize = privateCommand.size();
        size_t maxSize = IsSystemPrivateCommand(privateCommand) ? (MAX_PRIVATE_COMMAND_COUNT + 1)
                                                                : MAX_PRIVATE_COMMAND_COUNT;
        if (privateCommandSize == 0 || privateCommandSize > maxSize) {
            IMSA_HILOGE("privateCommand size must more than 0 and less than 5.");
            return false;
        }
        size_t totalSize = 0;
        for (const auto &iter : privateCommand) {
            size_t keySize = iter.first.size();
            size_t idx = iter.second.index();
            size_t valueSize = 0;

            if (idx == static_cast<size_t>(PrivateDataValueType::VALUE_TYPE_STRING)) {
                auto stringValue = std::get_if<std::string>(&iter.second);
                if (stringValue == nullptr) {
                    IMSA_HILOGE("get stringValue failed.");
                    return false;
                }
                valueSize = (*stringValue).size();
            } else if (idx == static_cast<size_t>(PrivateDataValueType::VALUE_TYPE_BOOL)) {
                valueSize = sizeof(bool);
            } else if (idx == static_cast<size_t>(PrivateDataValueType::VALUE_TYPE_NUMBER)) {
                valueSize = sizeof(int32_t);
            }
            totalSize = totalSize + keySize + valueSize;
        }
        if (totalSize > MAX_PRIVATE_COMMAND_SIZE) {
            IMSA_HILOGE("totalSize : %{public}zu", totalSize);
            return false;
        }
        return true;
    }
    static bool IsSystemPrivateCommand(const std::unordered_map<std::string, PrivateDataValue> &privateCommand)
    {
        IMSA_HILOGD("in.");
        size_t privateCommandSize = privateCommand.size();
        if (privateCommandSize == 0 || privateCommandSize > MAX_PRIVATE_COMMAND_COUNT) {
            IMSA_HILOGE("privateCommand size must more than 0 and less than 5.");
            return false;
        }
        auto it = privateCommand.find(SYSTEM_CMD_KEY);
        if (it != privateCommand.end()) {
            // if privateCommand has the key system_cmd and value is 1, it's a system privateCommand.
            if (it->second.index() == static_cast<size_t>(PrivateDataValueType::VALUE_TYPE_NUMBER)) {
                auto numberValue = std::get_if<int32_t>(&it->second);
                if (numberValue == nullptr) {
                    IMSA_HILOGE("get stringValue failed.");
                    return false;
                }
                return *numberValue == 1;
            }
        }
        return false;
    }
};

enum class InputType : int32_t { NONE = -1, CAMERA_INPUT = 0, SECURITY_INPUT, VOICE_INPUT, END };

enum class SwitchTrigger : uint32_t { CURRENT_IME = 0, SYSTEM_APP, IMSA };
} // namespace MiscServices
} // namespace OHOS
#endif // FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_INPUT_METHOD_UTILS_H
