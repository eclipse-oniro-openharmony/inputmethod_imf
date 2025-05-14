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
#ifndef TAIHE_INPUT_METHOD_SETTING_IMPL_H
#define TAIHE_INPUT_METHOD_SETTING_IMPL_H
#include <map>
#include <mutex>
#include <vector>

#include "ani_common.h"
#include "ime_event_listener.h"
#include "input_window_info.h"
#include "ohos.inputMethod.impl.hpp"
#include "ohos.inputMethod.proj.hpp"
namespace OHOS {
namespace MiscServices {
class InputMethodSettingImpl {
public:
    static InputMethodSettingImpl &GetInstance();
    taihe::array<InputMethodProperty_t> GetInputMethodsSync(bool enable);
    taihe::array<InputMethodSubtype_t> ListCurrentInputMethodSubtypeSync();
    taihe::array<InputMethodSubtype_t> ListInputMethodSubtypeSync(InputMethodProperty_t const &inputMethodProperty);
    bool IsPanelShown(PanelInfo_t const &panelInfo);
    taihe::array<InputMethodProperty_t> GetAllInputMethodsSync();
    void OnImeChangeCallback(const Property &property, const SubProperty &subProperty);
    void OnImeShowCallback(const ImeWindowInfo &info);
    void OnImeHideCallback(const ImeWindowInfo &info);
    void RegisterImeEvent(std::string const &eventName, int32_t eventMask, callbackType &&f, uintptr_t opq);
    void UnregisterImeEvent(std::string const &eventName, int32_t eventMask, taihe::optional_view<uintptr_t> opq);

private:
    PanelFlag softKbShowingFlag_{ FLG_CANDIDATE_COLUMN };
    PanelFlag GetSoftKbShowingFlag();
    void SetSoftKbShowingFlag(PanelFlag flag);
    void OnPanelStatusChange(std::string const &type, const InputWindowInfo &info);
    void RegisterListener(std::string const &type, callbackType &&cb, uintptr_t opq);
    void UnregisterListener(std::string const &type, taihe::optional_view<uintptr_t> opq, bool &isUpdateFlag);
    void HandleRegistrationError(std::string const &eventName, int32_t errorCode);
    std::mutex mutex_;
    std::map<std::string, std::vector<std::unique_ptr<CallbackObject>>> jsCbMap_;
};

class IMFSettingImpl {
public:
    IMFSettingImpl()
    {
    }
    taihe::array<InputMethodProperty_t> GetInputMethodsSync(bool enable)
    {
        return InputMethodSettingImpl::GetInstance().GetInputMethodsSync(enable);
    }
    taihe::array<InputMethodSubtype_t> ListCurrentInputMethodSubtypeSync()
    {
        return InputMethodSettingImpl::GetInstance().ListCurrentInputMethodSubtypeSync();
    }
    taihe::array<InputMethodSubtype_t> ListInputMethodSubtypeSync(InputMethodProperty_t const &inputMethodProperty)
    {
        return InputMethodSettingImpl::GetInstance().ListInputMethodSubtypeSync(inputMethodProperty);
    }
    bool IsPanelShown(PanelInfo_t const &panelInfo)
    {
        return InputMethodSettingImpl::GetInstance().IsPanelShown(panelInfo);
    }
    taihe::array<InputMethodProperty_t> GetAllInputMethodsSync()
    {
        return InputMethodSettingImpl::GetInstance().GetAllInputMethodsSync();
    }
    void OnImeHide(taihe::callback_view<void(taihe::array_view<InputWindowInfo_t>)> f, uintptr_t opq)
    {
        InputMethodSettingImpl::GetInstance().RegisterImeEvent("imeHide", EVENT_IME_HIDE_MASK, f, opq);
    }
    void OffImeHide(taihe::optional_view<uintptr_t> opq)
    {
        InputMethodSettingImpl::GetInstance().UnregisterImeEvent("imeHide", EVENT_IME_HIDE_MASK, opq);
    }
    void OnImeShow(taihe::callback_view<void(taihe::array_view<InputWindowInfo_t>)> f, uintptr_t opq)
    {
        InputMethodSettingImpl::GetInstance().RegisterImeEvent("imeShow", EVENT_IME_SHOW_MASK, f, opq);
    }
    void OffImeShow(taihe::optional_view<uintptr_t> opq)
    {
        InputMethodSettingImpl::GetInstance().UnregisterImeEvent("imeShow", EVENT_IME_SHOW_MASK, opq);
    }
    void OnImeChange(taihe::callback_view<void(InputMethodProperty_t const &, InputMethodSubtype_t const &)> f,
        uintptr_t opq)
    {
        InputMethodSettingImpl::GetInstance().RegisterImeEvent("imeChange", EVENT_IME_CHANGE_MASK, f, opq);
    }
    void OffImeChange(taihe::optional_view<uintptr_t> opq)
    {
        InputMethodSettingImpl::GetInstance().UnregisterImeEvent("imeChange", EVENT_IME_CHANGE_MASK, opq);
    }
};
} // namespace MiscServices
} // namespace OHOS
#endif