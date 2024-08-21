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
#ifndef NATIVE_TEXT_CHANGED_LISTENER_H
#define NATIVE_TEXT_CHANGED_LISTENER_H
#include "input_method_controller.h"
#include "native_inputmethod_types.h"
namespace OHOS {
namespace MiscServices {
class NativeTextChangedListener : public OHOS::MiscServices::OnTextChangedListener {
public:
    explicit NativeTextChangedListener(InputMethod_TextEditorProxy *textEditor) : textEditor_(textEditor) {};
    ~NativeTextChangedListener() {};
    void InsertText(const std::u16string &text) override;
    void DeleteForward(int32_t length) override;
    void DeleteBackward(int32_t length) override;
    void SendKeyboardStatus(const OHOS::MiscServices::KeyboardStatus &status) override;
    void SendFunctionKey(const OHOS::MiscServices::FunctionKey &functionKey) override;
    void MoveCursor(const OHOS::MiscServices::Direction direction) override;
    void HandleSetSelection(int32_t start, int32_t end) override;
    void HandleExtendAction(int32_t action) override;
    std::u16string GetLeftTextOfCursor(int32_t number) override;
    std::u16string GetRightTextOfCursor(int32_t number) override;
    int32_t GetTextIndexAtCursor() override;
    int32_t ReceivePrivateCommand(const std::unordered_map<std::string, PrivateDataValue> &privateCommand) override;
    int32_t SetPreviewText(const std::u16string &text, const OHOS::MiscServices::Range &range) override;
    void FinishTextPreview() override;

    // empty impl
    void SendKeyEventFromInputMethod(const KeyEvent &event) override {};
    void SetKeyboardStatus(bool status) override {};
    void HandleSelect(int32_t keyCode, int32_t cursorMoveSkip) override {};

private:
    InputMethod_KeyboardStatus ConvertToCKeyboardStatus(OHOS::MiscServices::KeyboardStatus status);
    InputMethod_EnterKeyType ConvertToCEnterKeyType(OHOS::MiscServices::EnterKeyType enterKeyType);
    InputMethod_Direction ConvertToCDirection(OHOS::MiscServices::Direction direction);
    InputMethod_ExtendAction ConvertToCExtendAction(int32_t action);

    constexpr static int32_t MAX_TEXT_LENGTH = 8 * 1024;

    InputMethod_TextEditorProxy *textEditor_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // NATIVE_TEXT_CHANGED_LISTENE_H