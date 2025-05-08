/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#ifndef TAIHE_INPUT_METHOD_CONTROLLER_IMPL_H
#define TAIHE_INPUT_METHOD_CONTROLLER_IMPL_H
#include <cstdint>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "ani_common.h"
#include "controller_listener.h"
#include "input_method_utils.h"
#include "ohos.inputMethod.impl.hpp"
#include "ohos.inputMethod.proj.hpp"
namespace OHOS {
namespace MiscServices {
class InputMethodControllerImpl : public ControllerListener {
public:
    static std::shared_ptr<InputMethodControllerImpl> GetInstance();
    void HideSoftKeyboardSync();
    void ShowTextInputHasParam(RequestKeyboardReason_t requestKeyboardReason);
    void ShowTextInputSync();
    void HideTextInputSync();
    void AttachSync(bool showKeyboard, TextConfig_t const &textConfig);
    void AttachWithReason(bool showKeyboard, TextConfig_t const &textConfig,
        RequestKeyboardReason_t requestKeyboardReason);
    void DetachSync();
    void RegisterListener(std::string const &type, callbackType &&cb, uintptr_t opq);
    void UnRegisterListener(std::string const &type, taihe::optional_view<uintptr_t> opq);

    void InsertTextCallback(const std::u16string &text);
    void DeleteLeftCallback(int32_t length);
    void DeleteRightCallback(int32_t length);
    void SendKeyboardStatusCallback(const KeyboardStatus &status);
    void SendFunctionKeyCallback(const FunctionKey &key);
    void MoveCursorCallback(const Direction &direction);
    void HandleExtendActionCallback(int32_t action);
    std::u16string GetLeftTextOfCursorCallback(int32_t number);
    std::u16string GetRightTextOfCursorCallback(int32_t number);
    int32_t GetTextIndexAtCursorCallback();

    void OnSelectByRange(int32_t start, int32_t end) override;
    void OnSelectByMovement(int32_t direction) override;

private:
    std::mutex mutex_;
    std::map<std::string, std::vector<std::unique_ptr<CallbackObject>>> jsCbMap_;
    static std::mutex controllerMutex_;
    static std::shared_ptr<InputMethodControllerImpl> controller_;
};

class IMFControllerImpl {
public:
    void HideSoftKeyboardSync()
    {
        InputMethodControllerImpl::GetInstance()->HideSoftKeyboardSync();
    }
    void ShowTextInputHasParam(RequestKeyboardReason_t requestKeyboardReason)
    {
        InputMethodControllerImpl::GetInstance()->ShowTextInputHasParam(requestKeyboardReason);
    }
    void ShowTextInputSync()
    {
        InputMethodControllerImpl::GetInstance()->ShowTextInputSync();
    }
    void HideTextInputSync()
    {
        InputMethodControllerImpl::GetInstance()->HideTextInputSync();
    }
    void AttachSync(bool showKeyboard, TextConfig_t const &textConfig)
    {
        InputMethodControllerImpl::GetInstance()->AttachSync(showKeyboard, textConfig);
    }
    void AttachWithReason(bool showKeyboard, TextConfig_t const &textConfig,
        RequestKeyboardReason_t requestKeyboardReason)
    {
        InputMethodControllerImpl::GetInstance()->AttachWithReason(showKeyboard, textConfig, requestKeyboardReason);
    }
    void DetachSync()
    {
        InputMethodControllerImpl::GetInstance()->DetachSync();
    }
    void OnSelectByRange(taihe::callback_view<void(Range_t const &)> f, uintptr_t opq)
    {
        InputMethodControllerImpl::GetInstance()->RegisterListener("selectByRange", f, opq);
    }
    void OffSelectByRange(taihe::optional_view<uintptr_t> opq)
    {
        InputMethodControllerImpl::GetInstance()->UnRegisterListener("selectByRange", opq);
    }
    void OnSelectByMovement(taihe::callback_view<void(Movement_t const &)> f, uintptr_t opq)
    {
        InputMethodControllerImpl::GetInstance()->RegisterListener("selectByMovement", f, opq);
    }
    void OffSelectByMovement(taihe::optional_view<uintptr_t> opq)
    {
        InputMethodControllerImpl::GetInstance()->UnRegisterListener("selectByMovement", opq);
    }
    void OnInsertText(taihe::callback_view<void(taihe::string_view)> f, uintptr_t opq)
    {
        InputMethodControllerImpl::GetInstance()->RegisterListener("insertText", f, opq);
    }
    void OffInsertText(taihe::optional_view<uintptr_t> opq)
    {
        InputMethodControllerImpl::GetInstance()->UnRegisterListener("insertText", opq);
    }
    void OnDeleteLeft(taihe::callback_view<void(int32_t)> f, uintptr_t opq)
    {
        InputMethodControllerImpl::GetInstance()->RegisterListener("deleteLeft", f, opq);
    }
    void OffDeleteLeft(taihe::optional_view<uintptr_t> opq)
    {
        InputMethodControllerImpl::GetInstance()->UnRegisterListener("deleteLeft", opq);
    }
    void OnDeleteRight(taihe::callback_view<void(int32_t)> f, uintptr_t opq)
    {
        InputMethodControllerImpl::GetInstance()->RegisterListener("deleteRight", f, opq);
    }
    void OffDeleteRight(taihe::optional_view<uintptr_t> opq)
    {
        InputMethodControllerImpl::GetInstance()->UnRegisterListener("deleteRight", opq);
    }
    void OnSendKeyboardStatus(taihe::callback_view<void(KeyboardStatus_t)> f, uintptr_t opq)
    {
        InputMethodControllerImpl::GetInstance()->RegisterListener("sendKeyboardStatus", f, opq);
    }
    void OffSendKeyboardStatus(taihe::optional_view<uintptr_t> opq)
    {
        InputMethodControllerImpl::GetInstance()->UnRegisterListener("sendKeyboardStatus", opq);
    }
    void OnSendFunctionKey(taihe::callback_view<void(FunctionKey_t const &)> f, uintptr_t opq)
    {
        InputMethodControllerImpl::GetInstance()->RegisterListener("sendFunctionKey", f, opq);
    }
    void OffSendFunctionKey(taihe::optional_view<uintptr_t> opq)
    {
        InputMethodControllerImpl::GetInstance()->UnRegisterListener("sendFunctionKey", opq);
    }
    void OnMoveCursor(taihe::callback_view<void(Direction_t)> f, uintptr_t opq)
    {
        InputMethodControllerImpl::GetInstance()->RegisterListener("moveCursor", f, opq);
    }
    void OffMoveCursor(taihe::optional_view<uintptr_t> opq)
    {
        InputMethodControllerImpl::GetInstance()->UnRegisterListener("moveCursor", opq);
    }
    void OnHandleExtendAction(taihe::callback_view<void(ExtendAction_t)> f, uintptr_t opq)
    {
        InputMethodControllerImpl::GetInstance()->RegisterListener("handleExtendAction", f, opq);
    }
    void OffHandleExtendAction(taihe::optional_view<uintptr_t> opq)
    {
        InputMethodControllerImpl::GetInstance()->UnRegisterListener("handleExtendAction", opq);
    }
    void OnGetLeftTextOfCursor(taihe::callback_view<taihe::string(int32_t)> f, uintptr_t opq)
    {
        InputMethodControllerImpl::GetInstance()->RegisterListener("getLeftTextOfCursor", f, opq);
    }
    void OffGetLeftTextOfCursor(taihe::optional_view<uintptr_t> opq)
    {
        InputMethodControllerImpl::GetInstance()->UnRegisterListener("getLeftTextOfCursor", opq);
    }
    void OnGetRightTextOfCursor(taihe::callback_view<taihe::string(int32_t)> f, uintptr_t opq)
    {
        InputMethodControllerImpl::GetInstance()->RegisterListener("getRightTextOfCursor", f, opq);
    }
    void OffGetRightTextOfCursor(taihe::optional_view<uintptr_t> opq)
    {
        InputMethodControllerImpl::GetInstance()->UnRegisterListener("getRightTextOfCursor", opq);
    }
    void OnGetTextIndexAtCursor(taihe::callback_view<int32_t()> f, uintptr_t opq)
    {
        InputMethodControllerImpl::GetInstance()->RegisterListener("getTextIndexAtCursor", f, opq);
    }
    void OffGetTextIndexAtCursor(taihe::optional_view<uintptr_t> opq)
    {
        InputMethodControllerImpl::GetInstance()->UnRegisterListener("getTextIndexAtCursor", opq);
    }
};
} // namespace MiscServices
} // namespace OHOS
#endif