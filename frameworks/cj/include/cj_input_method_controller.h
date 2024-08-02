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
    static int32_t Attach(bool showKeyboard, const CTextConfig &txtCfg);
    static int32_t Detach();
    static int32_t ShowTextInput();
    static int32_t HideTextInput();
    static int32_t SetCallingWindow(uint32_t windowId);
    static int32_t UpdateCursor(const CCursorInfo &cursor);
    static int32_t ChangeSelection(const std::string &text, int32_t start, int32_t end);
    static int32_t UpdateAttribute(const CInputAttribute &inputAttribute);
    static int32_t ShowSoftKeyboard();
    static int32_t HideSoftKeyboard();
    static int32_t StopInputSession();
    void OnSelectByRange(int32_t start, int32_t end) override;
    void OnSelectByMovement(int32_t direction) override;
    void InsertText(const std::u16string &text);
    void DeleteRight(int32_t length);
    void DeleteLeft(int32_t length);
    void SendKeyboardStatus(const KeyboardStatus &status);
    void SendFunctionKey(const FunctionKey &functionKey);
    void MoveCursor(const Direction direction);
    void HandleExtendAction(int32_t action);
    std::u16string GetText(const std::string &type, int32_t number);
    int32_t GetTextIndexAtCursor();

private:
    static std::shared_ptr<AppExecFwk::EventHandler> GetEventHandler();
    // static const std::set<std::string> TEXT_EVENT_TYPE;
    static constexpr int32_t MAX_TIMEOUT = 2500;
    static std::mutex controllerMutex_;
    static std::shared_ptr<CjInputMethodController> controller_;
    static const std::string IMC_CLASS_NAME;
    static std::mutex eventHandlerMutex_;
    static std::shared_ptr<AppExecFwk::EventHandler> handler_;
    static constexpr size_t PARAM_POS_ZERO = 0;
    static constexpr size_t PARAM_POS_ONE = 1;
    static constexpr size_t PARAM_POS_TWO = 2;
    static constexpr size_t PARAM_POS_THREE = 3;
};
} // namespace MiscServices
} // namespace OHOS
#endif // INPUT_METHOD_CONTROLLER_H
