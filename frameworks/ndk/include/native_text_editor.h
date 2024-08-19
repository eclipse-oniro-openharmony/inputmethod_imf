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
#ifndef NATIVE_TEXT_EDITOR_H
#define NATIVE_TEXT_EDITOR_H
#include "input_method_controller.h"
#include "inputmethod_controller_capi.h"

struct InputMethod_PrivateCommand {
    std::string key;
    std::variant<std::string, bool, int32_t> value;
};

struct InputMethod_CursorInfo {
    double left = -1.0;
    double top = -1.0;
    double width = -1.0;
    double height = -1.0;
};

struct InputMethod_TextAvoidInfo {
    double positionY;
    double height;
};
struct InputMethod_TextConfig {
    InputMethod_TextInputType inputType;
    InputMethod_EnterKeyType enterKeyType;
    bool previewTextSupported;
    InputMethod_CursorInfo cursorInfo;
    InputMethod_TextAvoidInfo avoidInfo;
    int32_t selectionStart;
    int32_t selectionEnd;
    int32_t windowId;
};

struct InputMethod_TextEditorProxy {
    OH_TextEditorProxy_GetTextConfigFunc getTextConfigFunc;
    OH_TextEditorProxy_InsertTextFunc insertTextFunc;
    OH_TextEditorProxy_DeleteForwardFunc deleteForwardFunc;
    OH_TextEditorProxy_DeleteBackwardFunc deleteBackwardFunc;
    OH_TextEditorProxy_SendKeyboardStatusFunc sendKeyboardStatusFunc;
    OH_TextEditorProxy_SendEnterKeyFunc sendEnterKeyFunc;
    OH_TextEditorProxy_MoveCursorFunc moveCursorFunc;
    OH_TextEditorProxy_HandleSetSelectionFunc handleSetSelectionFunc;
    OH_TextEditorProxy_HandleExtendActionFunc handleExtendActionFunc;
    OH_TextEditorProxy_GetLeftTextOfCursorFunc getLeftTextOfCursorFunc;
    OH_TextEditorProxy_GetRightTextOfCursorFunc getRightTextOfCursorFunc;
    OH_TextEditorProxy_GetTextIndexAtCursorFunc getTextIndexAtCursorFunc;
    OH_TextEditorProxy_ReceivePrivateCommandFunc receivePrivateCommandFunc;
    OH_TextEditorProxy_SetPreviewTextFunc setPreviewTextFunc;
    OH_TextEditorProxy_FinishTextPreview finishTextPreviewFunc;
};

InputMethod_ErrorCode ErrorCodeConvert(int32_t code);
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
#endif // NATIVE_TEXT_EDITOR_H
