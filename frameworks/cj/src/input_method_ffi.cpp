/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "input_method_ffi.h"

#include "inputmethod_trace.h"
#include "cj_input_method_controller.h"
#include "input_method_property.h"
#include "setting_listeners.h"
#include "global.h"
#include "utils.h"

namespace OHOS::MiscServices {
extern "C" {
int32_t FfiInputMethodGetDefaultInputMethod(CInputMethodProperty &props)
{
    auto ctrl = InputMethodController::GetInstance();
    if (ctrl == nullptr) {
        return ERR_NO_MEMORY;
    }
    std::shared_ptr<Property> property;
    int32_t ret = ctrl->GetDefaultInputMethod(property);
    if (property == nullptr) {
        IMSA_HILOGE("default input method is nullptr!");
        return ret;
    }
    Utils::InputMethodProperty2C(props, *property);
    return ret;
}

int32_t FfiInputMethodGetCurrentInputMethod(CInputMethodProperty &props)
{
    auto ctrl = InputMethodController::GetInstance();
    if (ctrl == nullptr) {
        return ERR_NO_MEMORY;
    }
    std::shared_ptr<Property> property = ctrl->GetCurrentInputMethod();
    if (property == nullptr) {
        IMSA_HILOGE("current input method is nullptr!");
        return ERR_NO_MEMORY;
    }
    Utils::InputMethodProperty2C(props, *property);
    return 0;
}

int32_t FfiInputMethodSwitchInputMethod(bool &result, CInputMethodProperty props)
{
    InputMethodSyncTrace tracer("CJInputMethod_SwitchInputMethod");
    auto ctrl = InputMethodController::GetInstance();
    if (ctrl == nullptr) {
        return ERR_NO_MEMORY;
    }
    int32_t errCode =
        ctrl->SwitchInputMethod(SwitchTrigger::CURRENT_IME, std::string(props.name), std::string(props.id));
    if (errCode == ErrorCode::NO_ERROR) {
        result = true;
    }
    return errCode;
}

int32_t FfiInputMethodSwitchCurrentInputMethodSubtype(bool &result, CInputMethodSubtype target)
{
    InputMethodSyncTrace tracer("CJInputMethod_SwitchCurrentInputMethodSubtype");
    auto ctrl = InputMethodController::GetInstance();
    if (ctrl == nullptr) {
        return ERR_NO_MEMORY;
    }
    int32_t errCode =
        ctrl->SwitchInputMethod(SwitchTrigger::CURRENT_IME, std::string(target.name), std::string(target.id));
    if (errCode == ErrorCode::NO_ERROR) {
        result = true;
    }
    return errCode;
}

int32_t FfiInputMethodGetCurrentInputMethodSubtype(CInputMethodSubtype &props)
{
    auto ctrl = InputMethodController::GetInstance();
    if (ctrl == nullptr) {
        return ERR_NO_MEMORY;
    }
    std::shared_ptr<SubProperty> subProperty = ctrl->GetCurrentInputMethodSubtype();
    if (subProperty == nullptr) {
        IMSA_HILOGE("current input method subtype is nullptr!");
        return ERR_NO_MEMORY;
    }
    Utils::InputMethodSubProperty2C(props, *subProperty);
    return 0;
}

int32_t FfiInputMethodSwitchCurrentInputMethodAndSubtype(bool &result,
    CInputMethodProperty target, CInputMethodSubtype subtype)
{
    InputMethodSyncTrace tracer("CJInputMethod_SwitchCurrentInputMethodAndSubtype");
    auto ctrl = InputMethodController::GetInstance();
    if (ctrl == nullptr) {
        return ERR_NO_MEMORY;
    }
    int32_t errCode = ctrl->SwitchInputMethod(SwitchTrigger::CURRENT_IME,
        std::string(subtype.name), std::string(subtype.id));
    if (errCode == ErrorCode::NO_ERROR) {
        result = true;
    }
    return errCode;
}

int32_t FfiInputMethodGetSystemInputMethodConfigAbility(CElementName &elem)
{
    OHOS::AppExecFwk::ElementName inputMethodConfig;
    auto ctrl = InputMethodController::GetInstance();
    if (ctrl == nullptr) {
        return ERR_NO_MEMORY;
    }
    int32_t ret = ctrl->GetInputMethodConfig(inputMethodConfig);
    if (ret == ErrorCode::NO_ERROR) {
        elem.deviceId = Utils::MallocCString(inputMethodConfig.GetDeviceID());
        elem.bundleName = Utils::MallocCString(inputMethodConfig.GetBundleName());
        elem.abilityName = Utils::MallocCString(inputMethodConfig.GetAbilityName());
        elem.moduleName = Utils::MallocCString(inputMethodConfig.GetModuleName());
    }
    return ret;
}

RetInputMethodSubtype FfiInputMethodSettingListInputMethodSubtype(CInputMethodProperty props)
{
    IMSA_HILOGD("run in ListInputMethodSubtype");
    RetInputMethodSubtype ret{};
    Property property = Utils::C2InputMethodProperty(props);
    std::vector<SubProperty> subProps;
    auto ctrl = InputMethodController::GetInstance();
    if (ctrl == nullptr) {
        ret.code = ERR_NO_MEMORY;
        return ret;
    }
    int32_t errCode = ctrl->ListInputMethodSubtype(property, subProps);
    ret.code = errCode;
    if (errCode != ErrorCode::NO_ERROR) {
        return ret;
    }
    IMSA_HILOGI("exec ListInputMethodSubtype success");
    ret.size = static_cast<int64_t>(subProps.size());
    if (ret.size > 0) {
        ret.head = static_cast<CInputMethodSubtype *>(malloc(sizeof(CInputMethodSubtype) * ret.size));
    }
    if (ret.head == nullptr) {
        ret.size = 0;
        return ret;
    }
    for (unsigned int i = 0; i < ret.size; i++) {
        CInputMethodSubtype props;
        Utils::InputMethodSubProperty2C(props, subProps[i]);
        ret.head[i] = props;
    }
    return ret;
}

RetInputMethodSubtype FfiInputMethodSettingListCurrentInputMethodSubtype()
{
    IMSA_HILOGD("run in ListCurrentInputMethodSubtype");
    RetInputMethodSubtype ret{};
    std::vector<SubProperty> subProps;
    auto ctrl = InputMethodController::GetInstance();
    if (ctrl == nullptr) {
        ret.code = ERR_NO_MEMORY;
        return ret;
    }
    int32_t errCode = ctrl->ListCurrentInputMethodSubtype(subProps);
    ret.code = errCode;
    if (errCode != ErrorCode::NO_ERROR) {
        return ret;
    }
    IMSA_HILOGI("exec ListCurrentInputMethodSubtype success.");
    ret.size = static_cast<int64_t>(subProps.size());
    if (ret.size > 0) {
        ret.head = static_cast<CInputMethodSubtype *>(malloc(sizeof(CInputMethodSubtype) * ret.size));
    }
    if (ret.head == nullptr) {
        ret.size = 0;
        return ret;
    }
    for (unsigned int i = 0; i < ret.size; i++) {
        CInputMethodSubtype props;
        Utils::InputMethodSubProperty2C(props, subProps[i]);
        ret.head[i] = props;
    }
    return ret;
}

RetInputMethodProperty FfiInputMethodSettingGetInputMethods(bool enable)
{
    IMSA_HILOGD("run in");
    RetInputMethodProperty ret{};
    std::vector<Property> properties;
    auto ctrl = InputMethodController::GetInstance();
    if (ctrl == nullptr) {
        ret.code = ERR_NO_MEMORY;
        return ret;
    }
    int32_t errCode = ctrl->ListInputMethod(enable, properties);
    ret.code = errCode;
    if (errCode != ErrorCode::NO_ERROR) {
        return ret;
    }
    ret.size = static_cast<int64_t>(properties.size());
    if (ret.size > 0) {
        ret.head = static_cast<CInputMethodProperty *>(malloc(sizeof(CInputMethodProperty) * ret.size));
    }
    if (ret.head == nullptr) {
        ret.size = 0;
        return ret;
    }
    for (unsigned int i = 0; i < ret.size; i++) {
        CInputMethodProperty props;
        Utils::InputMethodProperty2C(props, properties[i]);
        ret.head[i] = props;
    }
    return ret;
}

RetInputMethodProperty FfiInputMethodSettingGetAllInputMethods()
{
    IMSA_HILOGD("run in");
    RetInputMethodProperty ret{};
    std::vector<Property> properties;
    auto ctrl = InputMethodController::GetInstance();
    if (ctrl == nullptr) {
        ret.code = ERR_NO_MEMORY;
        return ret;
    }
    int32_t errCode = ctrl->ListInputMethod(properties);
    ret.code = errCode;
    if (errCode != ErrorCode::NO_ERROR) {
        return ret;
    }
    ret.size = static_cast<int64_t>(properties.size());
    if (ret.size > 0) {
        ret.head = static_cast<CInputMethodProperty *>(malloc(sizeof(CInputMethodProperty) * ret.size));
    }
    if (ret.head == nullptr) {
        ret.size = 0;
        return ret;
    }
    for (unsigned int i = 0; i < ret.size; i++) {
        CInputMethodProperty props;
        Utils::InputMethodProperty2C(props, properties[i]);
        ret.head[i] = props;
    }
    return ret;
}

int32_t FfiInputMethodSettingOn(uint32_t type, void (*func)(CInputMethodProperty, CInputMethodSubtype))
{
    auto setting = CJGetInputMethodSetting::GetIMSInstance();
    if (setting == nullptr) {
        return ERR_NO_MEMORY;
    }
    return setting->Subscribe(type, func);
}

int32_t FfiInputMethodSettingOff(uint32_t type)
{
    auto setting = CJGetInputMethodSetting::GetIMSInstance();
    if (setting == nullptr) {
        return ERR_NO_MEMORY;
    }
    return setting->UnSubscribe(type);
}

int32_t FfiInputMethodSettingShowOptionalInputMethods(bool& result)
{
    IMSA_HILOGD("start JsGetInputMethodSetting.");
    auto ctrl = InputMethodController::GetInstance();
    if (ctrl == nullptr) {
        return ERR_NO_MEMORY;
    }
    int32_t errCode = ctrl->DisplayOptionalInputMethod();
    if (errCode == ErrorCode::NO_ERROR) {
        IMSA_HILOGI("exec DisplayOptionalInputMethod success");
        result = true;
    }
    return errCode;
}

int32_t FfiInputMethodControllerOn(int8_t type, int64_t id)
{
    return CjInputMethodController::Subscribe(type, id);
}

int32_t FfiInputMethodControllerOff(int8_t type)
{
    return CjInputMethodController::Unsubscribe(type);
}

int32_t FfiInputMethodControllerAttach(bool showKeyboard, CTextConfig txtCfg)
{
    return CjInputMethodController::Attach(txtCfg, showKeyboard);
}

int32_t FfiInputMethodControllerDetach()
{
    return CjInputMethodController::Detach();
}

int32_t FfiInputMethodControllerShowTextInput()
{
    return CjInputMethodController::ShowTextInput();
}

int32_t FfiInputMethodControllerHideTextInput()
{
    return CjInputMethodController::HideTextInput();
}

int32_t FfiInputMethodControllerSetCallingWindow(uint32_t windowId)
{
    return CjInputMethodController::SetCallingWindow(windowId);
}

int32_t FfiInputMethodControllerUpdateCursor(CCursorInfo cursor)
{
    return CjInputMethodController::UpdateCursor(cursor);
}

int32_t FfiInputMethodControllerChangeSelection(char *text, int32_t start, int32_t end)
{
    return CjInputMethodController::ChangeSelection(std::string(text), start, end);
}

int32_t FfiInputMethodControllerUpdateAttribute(CInputAttribute inputAttribute)
{
    return CjInputMethodController::UpdateAttribute(inputAttribute);
}

int32_t FfiInputMethodControllerShowSoftKeyboard()
{
    return CjInputMethodController::ShowSoftKeyboard();
}

int32_t FfiInputMethodControllerHideSoftKeyboard()
{
    return CjInputMethodController::HideSoftKeyboard();
}

int32_t FfiInputMethodControllerStopInputSession()
{
    return CjInputMethodController::StopInputSession();
}
}
}