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
#include "input_method_setting_impl.h"
#include "input_method_event_listener.h"
#include "ime_event_monitor_manager_impl.h"
#include "taihe/runtime.hpp"
#include "stdexcept"
#include "input_method_controller.h"
#include "js_utils.h"
#include "ani_common.h"

namespace OHOS {
namespace MiscServices {
using namespace taihe;

InputMethodSettingImpl& InputMethodSettingImpl::GetInstance()
{
    static InputMethodSettingImpl instance;
    return instance;
}

array<InputMethodProperty_t> InputMethodSettingImpl::GetInputMethodSync(bool enable) {
    std::vector<Property> properties;
    int32_t errCode =
        InputMethodController::GetInstance()->ListInputMethod(enable, properties);
    if (errCode != ErrorCode::NO_ERROR) {
        set_business_error(JsUtils::Convert(errCode), "failed to get input method!");
        return array<InputMethodProperty_t>(nullptr, 0);
    }
    std::vector<InputMethodProperty_t> vecProperty;
    for (const auto& property : properties) {
        vecProperty.push_back(PropertyConverter::ConvertProperty(property));
    }
    return array<InputMethodProperty_t>(vecProperty);
}

array<InputMethodSubtype_t> InputMethodSettingImpl::ListCurrentInputMethodSubtypeSync()
{
    std::vector<SubProperty> subProperties;
    int32_t errCode =
        InputMethodController::GetInstance()->ListCurrentInputMethodSubtype(subProperties);
    if (errCode != ErrorCode::NO_ERROR) {
        set_business_error(JsUtils::Convert(errCode), JsUtils::ToMessage(JsUtils::Convert(errCode)));
        return array<InputMethodSubtype_t>(nullptr, 0);
    }
    std::vector<InputMethodSubtype_t> vecSubtype;
    for (const auto& property : subProperties) {
        vecSubtype.push_back(PropertyConverter::ConvertSubProperty(property));
    }
    return array<InputMethodSubtype_t>(vecSubtype);

}

array<InputMethodSubtype_t> InputMethodSettingImpl::ListInputMethodSubtypeSync(InputMethodProperty_t const& inputMethodProperty)
{
    Property property{.name = std::string(inputMethodProperty.name), .id = std::string(inputMethodProperty.id)};
    if (property.name.empty() || property.id.empty()) {
        property.name = inputMethodProperty.packageName;
        property.id = inputMethodProperty.methodId;
    }
    if (property.name.empty() || property.id.empty()) {
        set_business_error(IMFErrorCode::EXCEPTION_PARAMCHECK, "name and id must be string and cannot empty");
        return array<InputMethodSubtype_t>(nullptr, 0);
    }
    std::vector<SubProperty> subProperties;
    int32_t errCode =
        InputMethodController::GetInstance()->ListInputMethodSubtype(property, subProperties);
    if (errCode != ErrorCode::NO_ERROR) {
        set_business_error(JsUtils::Convert(errCode), JsUtils::ToMessage(JsUtils::Convert(errCode)));
        return array<InputMethodSubtype_t>(nullptr, 0);
    }
    std::vector<InputMethodSubtype_t> vecSubtype;
    for (const auto& property : subProperties) {
        vecSubtype.push_back(PropertyConverter::ConvertSubProperty(property));
    }
    return array<InputMethodSubtype_t>(vecSubtype);
}

bool InputMethodSettingImpl::IsPanelShown(PanelInfo_t const& panelInfo)
{
    PanelInfo info;
    if (panelInfo.flag.has_value()) {
        info.panelFlag = static_cast<PanelFlag>(panelInfo.flag.value().get_value());
    }
    info.panelType = static_cast<PanelType>(panelInfo.type.get_value());
    bool isShown = false;
    int32_t errorCode = InputMethodController::GetInstance()->IsPanelShown(info, isShown);
    if (errorCode != ErrorCode::NO_ERROR) {
        set_business_error(JsUtils::Convert(errorCode), "failed to query is panel shown!");
        return false; //napi 接口返回null
    }
    return isShown;
}

array<InputMethodProperty_t> InputMethodSettingImpl::GetAllInputMethodsSync()
{
    TH_THROW(std::runtime_error, "GetAllInputMethodsSync not implemented");
    std::vector<Property> properties;
    int32_t errCode =
        InputMethodController::GetInstance()->ListInputMethod(properties);
    if (errCode != ErrorCode::NO_ERROR) {
        set_business_error(JsUtils::Convert(errCode), "failed to get input method!");
        return array<InputMethodProperty_t>(nullptr, 0);
    }
    std::vector<InputMethodProperty_t> vecProperty;
    for (const auto& property : properties) {
        vecProperty.push_back(PropertyConverter::ConvertProperty(property));
    }
    return array<InputMethodProperty_t>(vecProperty);
}

void InputMethodSettingImpl::RegisterImeEvent(std::string const& eventName, int32_t eventMask, callbackType&& f, uintptr_t opq)
{
    auto ret = ImeEventMonitorManagerImpl::GetInstance().RegisterImeEventListener(
        eventMask, InputMethodEventListener::GetInstance());
    
    if (ret == ErrorCode::NO_ERROR) {
        RegisterListener(eventName, std::forward<callbackType>(f), opq);
    } else {
        HandleRegistrationError(eventName, ret);
    }
}
void InputMethodSettingImpl::UnregisterImeEvent(std::string const& eventName, int32_t eventMask, optional_view<uintptr_t> opq)
{
    bool isUpdateFlag = false;
    UnregisterListener(eventName, opq, isUpdateFlag);
    
    if (isUpdateFlag) {
        auto ret = ImeEventMonitorManagerImpl::GetInstance().UnRegisterImeEventListener(
            eventMask, InputMethodEventListener::GetInstance());
        IMSA_HILOGI("Updated event: %{public}s flag, ret: %{public}d", eventName.c_str(), ret);
    }
}
void InputMethodSettingImpl::HandleRegistrationError(std::string const& eventName, int32_t errorCode)
{
    auto errCode = JsUtils::Convert(errorCode);
    if (errCode == EXCEPTION_SYSTEM_PERMISSION) {
        set_business_error(EXCEPTION_SYSTEM_PERMISSION, 
                        JsUtils::ToMessage(EXCEPTION_SYSTEM_PERMISSION));
    }
    IMSA_HILOGE("Failed to register %{public}s, error: %{public}d", eventName.c_str(), errorCode);
}
void InputMethodSettingImpl::RegisterListener(std::string const& type, callbackType&& cb, uintptr_t opq)
{
    std::lock_guard<std::mutex> lock(mutex_);
    ani_object callbackObj = reinterpret_cast<ani_object>(opq);
    ani_ref callbackRef;
    ani_env *env = taihe::get_env();
    if (env == nullptr || ANI_OK != env->GlobalReference_Create(callbackObj, &callbackRef)) {
        return;
    }
    auto& cbVec = jsCbMap_[type]; 
    bool isDuplicate = std::any_of(cbVec.begin(), cbVec.end(),
        [env, callbackRef](const CallbackObject& obj) {
            ani_boolean isEqual = false;
            return (ANI_OK == env->Reference_StrictEquals(callbackRef, obj.ref, &isEqual)) && isEqual;
        }
    );
    if (isDuplicate) {
        env->GlobalReference_Delete(callbackRef);
        return;
    }
    cbVec.emplace_back(cb, callbackRef);
}
void InputMethodSettingImpl::UnregisterListener(std::string const& type, optional_view<uintptr_t> opq, bool& isUpdateFlag)
{
    std::lock_guard<std::mutex> lock(mutex_);
    const auto iter = jsCbMap_.find(type);
    if (iter == jsCbMap_.end()) {
        return;
    }
    
    if (!opq.has_value()) {
        jsCbMap_.erase(iter);
        isUpdateFlag = true;
        return;
    }
    
    ani_env* env = taihe::get_env();
    if (env == nullptr) {
        return;
    }

    GlobalRefGuard guard(env, reinterpret_cast<ani_object>(opq.value()));
    if (!guard) {
        return;
    }
    
    const auto pred = [env, targetRef = guard.get()](const CallbackObject& obj) {
        ani_boolean is_equal = false;
        return (ANI_OK == env->Reference_StrictEquals(targetRef, obj.ref, &is_equal)) && is_equal;
    };
    auto& callbacks = iter->second;
    const auto it = std::find_if(callbacks.begin(), callbacks.end(), pred);
    if (it != callbacks.end()) {
        callbacks.erase(it);
    }
    if (callbacks.empty()) {
        jsCbMap_.erase(iter);
        isUpdateFlag = true;
    }    
}

void InputMethodSettingImpl::OnImeChangeCallback(const Property &property, const SubProperty &subProperty)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto& cbVec = jsCbMap_["imeChange"]; 
    for(auto &cb : cbVec) {
        auto& func = std::get<taihe::callback<void(InputMethodProperty_t const&, InputMethodSubtype_t const&)>>(cb.callback);
        func(PropertyConverter::ConvertProperty(property), PropertyConverter::ConvertSubProperty(subProperty));
    }
}
void InputMethodSettingImpl::OnImeShowCallback(const ImeWindowInfo &info)
{
    if (info.panelInfo.panelType != PanelType::SOFT_KEYBOARD ||
        (info.panelInfo.panelFlag != FLG_FLOATING && info.panelInfo.panelFlag != FLG_FIXED)) {
        return;
    }
    auto showingFlag = GetSoftKbShowingFlag();
    // FLG_FIXED->FLG_FLOATING in show
    if (info.panelInfo.panelFlag == FLG_FLOATING && showingFlag == FLG_FIXED) {
        InputWindowInfo windowInfo{ info.windowInfo.name, 0, 0, 0, 0 };
        OnPanelStatusChange("imeHide", windowInfo);
    }
    // FLG_FLOATING->FLG_FIXED in show/show FLG_FIXED/ rotating(resize) in FLG_FIXED show
    if ((info.panelInfo.panelFlag == FLG_FIXED && showingFlag == FLG_FLOATING) ||
        (info.panelInfo.panelFlag == FLG_FIXED && showingFlag == FLG_CANDIDATE_COLUMN) ||
        (info.panelInfo.panelFlag == FLG_FIXED && showingFlag == FLG_FIXED)) {
        OnPanelStatusChange("imeShow", info.windowInfo);
    }
    SetSoftKbShowingFlag(info.panelInfo.panelFlag);
}

void InputMethodSettingImpl::OnImeHideCallback(const ImeWindowInfo &info)
{
    SetSoftKbShowingFlag(FLG_CANDIDATE_COLUMN);
    if (info.panelInfo.panelType != PanelType::SOFT_KEYBOARD || info.panelInfo.panelFlag != PanelFlag::FLG_FIXED) {
        return;
    }
    OnPanelStatusChange("imeHide", info.windowInfo);
}

PanelFlag InputMethodSettingImpl::GetSoftKbShowingFlag()
{
    return softKbShowingFlag_;
}
void InputMethodSettingImpl::SetSoftKbShowingFlag(PanelFlag flag)
{
    softKbShowingFlag_ = flag;
}

void InputMethodSettingImpl::OnPanelStatusChange(std::string const& type, const InputWindowInfo &info)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto& cbVec = jsCbMap_[type];
    for(auto &cb : cbVec) {
        InputWindowInfo_t inputWindowInfo{
            .name = info.name,
            .left = info.left,
            .top = info.top,
            .width = info.width,
            .height = info.height
        };
        taihe::array<InputWindowInfo_t> arrInfo{inputWindowInfo};
        auto& func = std::get<taihe::callback<void(taihe::array_view<InputWindowInfo_t>)>>(cb.callback);
        func(arrInfo);
    }
}
}
}