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
#ifndef INTERFACE_KITS_JS_GET_INPUT_METHOD_TEXTCHANGE_LISTENER_H
#define INTERFACE_KITS_JS_GET_INPUT_METHOD_TEXTCHANGE_LISTENER_H

#include "input_method_controller.h"
namespace OHOS {
namespace MiscServices {
class JsGetInputMethodController;
class JsGetInputMethodTextChangedListener : public OnTextChangedListener {
public:
    JsGetInputMethodTextChangedListener() = default;
    ~JsGetInputMethodTextChangedListener() = default;
    static sptr<JsGetInputMethodTextChangedListener> GetTextListener(bool newEditBox = false);

    void InsertText(const std::u16string &text) override;
    void DeleteForward(int32_t length) override;
    void DeleteBackward(int32_t length) override;
    void SendKeyEventFromInputMethod(const KeyEvent &event) override
    {
    }
    void SendKeyboardStatus(const KeyboardStatus &status) override;
    void SendFunctionKey(const FunctionKey &functionKey) override;
    void SetKeyboardStatus(bool status) override
    {
    }
    void MoveCursor(const Direction direction) override;
    void HandleSetSelection(int32_t start, int32_t end) override
    {
    }
    void HandleExtendAction(int32_t action) override;
    void HandleSelect(int32_t keyCode, int32_t cursorMoveSkip) override
    {
    }
    std::u16string GetLeftTextOfCursor(int32_t number) override;
    std::u16string GetRightTextOfCursor(int32_t number) override;
    int32_t GetTextIndexAtCursor() override;
    int32_t ReceivePrivateCommand(const std::unordered_map<std::string, PrivateDataValue> &privateCommand) override;
    bool IsFromTs() override;
    int32_t SetPreviewText(const std::u16string &text, const Range &range) override;
    void FinishTextPreview() override;

private:
    static std::mutex listenerMutex_;
    static sptr<JsGetInputMethodTextChangedListener> inputMethodListener_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // INTERFACE_KITS_JS_GET_INPUT_METHOD_TEXTCHANGE_LISTENER_H