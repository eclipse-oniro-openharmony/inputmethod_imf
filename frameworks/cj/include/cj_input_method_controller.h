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
#ifndef INPUT_METHOD_CONTROLLER_H
#define INPUT_METHOD_CONTROLLER_H

#include <set>
#include <map>
#include "controller_listener.h"
#include "input_method_ffi_structs.h"
#include "event_handler.h"
#include "input_method_utils.h"

namespace OHOS {
namespace MiscServices {
class CjInputMethodController : public ControllerListener {
public:
    CjInputMethodController() = default;
    ~CjInputMethodController() = default;
    static std::shared_ptr<CjInputMethodController> GetInstance();
    static int32_t Attach(const CTextConfig &txtCfg, const AttachOptions &attachOptions);
    static int32_t Detach();
    static int32_t ShowTextInput(const AttachOptions &attachOptions);
    static int32_t HideTextInput();
    static int32_t SetCallingWindow(uint32_t windowId);
    static int32_t UpdateCursor(const CCursorInfo &cursor);
    static int32_t ChangeSelection(const std::string &text, int32_t start, int32_t end);
    static int32_t UpdateAttribute(const CInputAttribute &inputAttribute);
    static int32_t ShowSoftKeyboard();
    static int32_t HideSoftKeyboard();
    static int32_t StopInputSession();
    static int32_t Subscribe(int8_t type, int64_t id);
    static int32_t Unsubscribe(int8_t type);
    void OnSelectByRange(int32_t start, int32_t end) override;
    void OnSelectByMovement(int32_t direction) override;
    void InsertText(const std::u16string &text);
    void DeleteRight(int32_t length);
    void DeleteLeft(int32_t length);
    void SendKeyboardStatus(const KeyboardStatus &status);
    void SendFunctionKey(const FunctionKey &functionKey);
    void MoveCursor(const Direction direction);
    void HandleExtendAction(int32_t action);
    std::u16string GetLeftText(int32_t number);
    std::u16string GetRightText(int32_t number);
    int32_t GetTextIndexAtCursor();

private:
    static std::shared_ptr<AppExecFwk::EventHandler> GetEventHandler();
    static constexpr int32_t MAX_TIMEOUT = 2500;
    void RegisterListener(int8_t type, int64_t id);
    void UnRegisterListener(int8_t type);
    void InitInsertText(int64_t id);
    void InitDeleteRight(int64_t id);
    void InitDeleteLeft(int64_t id);
    void InitSendKeyboardStatus(int64_t id);
    void InitSendFunctionKey(int64_t id);
    void InitMoveCursor(int64_t id);
    void InitHandleExtendAction(int64_t id);
    void InitGetLeftText(int64_t id);
    void InitGetRightText(int64_t id);
    void InitSelectByRange(int64_t id);
    void InitSelectByMovement(int64_t id);
    void InitGetTextIndexAtCursor(int64_t id);
    std::recursive_mutex mutex_;
    std::function<void(Range)> onSelectByRange;
    std::function<void(int32_t)> onSelectByMovement;
    std::function<void(const char* text)> insertText;
    std::function<void(int32_t length)> deleteRight;
    std::function<void(int32_t length)> deleteLeft;
    std::function<void(int32_t status)> sendKeyboardStatus;
    std::function<void(int32_t functionKey)> sendFunctionKey;
    std::function<void(int32_t direction)> moveCursor;
    std::function<void(int32_t action)> handleExtendAction;
    std::function<char*(int32_t number)> getLeftText;
    std::function<char*(int32_t number)> getRightText;
    std::function<int32_t(void)> getTextIndexAtCursor;
    static std::mutex controllerMutex_;
    static std::shared_ptr<CjInputMethodController> controller_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // INPUT_METHOD_CONTROLLER_H
