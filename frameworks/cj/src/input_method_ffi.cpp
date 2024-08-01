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
#include "input_method_controller.h"
#include "input_method_property.h"
#include "setting_listeners.h"
#include "global.h"
#include "utils.h"

namespace OHOS::MiscServices
{
    extern "C"
    {
        int32_t CJ_GetDefaultInputMethod(CInputMethodProperty &props)
        {
            std::shared_ptr<Property> property;
            int32_t ret = InputMethodController::GetInstance()->GetDefaultInputMethod(property);
            if (property == nullptr)
            {
                IMSA_HILOGE("default input method is nullptr!");
                return ret;
            }
            Utils::InputMethodProperty2C(props, *property);
            return ret;
        }

        int32_t CJ_GetCurrentInputMethod(CInputMethodProperty &props)
        {
            std::shared_ptr<Property> property = InputMethodController::GetInstance()->GetCurrentInputMethod();
            if (property == nullptr)
            {
                IMSA_HILOGE("current input method is nullptr!");
                return 0;
            }
            Utils::InputMethodProperty2C(props, *property);
            return 0;
        }

        int32_t CJ_SwitchInputMethod(bool &result, CInputMethodProperty props)
        {
            InputMethodSyncTrace tracer("CJInputMethod_SwitchInputMethod");
            int32_t errCode =
                InputMethodController::GetInstance()->SwitchInputMethod(SwitchTrigger::CURRENT_IME, std::string(props.name), std::string(props.id));
            if (errCode == ErrorCode::NO_ERROR)
            {
                result = true;
            }
            return errCode;
        }

        int32_t CJ_SwitchCurrentInputMethodSubtype(bool &result, CInputMethodSubtype target)
        {
            InputMethodSyncTrace tracer("CJInputMethod_SwitchCurrentInputMethodSubtype");
            int32_t errCode = InputMethodController::GetInstance()->SwitchInputMethod(
                SwitchTrigger::CURRENT_IME, std::string(target.name), std::string(target.id));
            if (errCode == ErrorCode::NO_ERROR) {
                result = true;
            }
            return errCode;
        }

        int32_t CJ_GetCurrentInputMethodSubtype(CInputMethodSubtype &props)
        {
            std::shared_ptr<SubProperty> subProperty = InputMethodController::GetInstance()->GetCurrentInputMethodSubtype();
            if (subProperty == nullptr) {
                IMSA_HILOGE("current input method subtype is nullptr!");
                return -1;
            }
            Utils::InputMethodSubProperty2C(props, *subProperty);
            return 0;
        }

        int32_t CJ_SwitchCurrentInputMethodAndSubtype(bool &result, CInputMethodProperty target, CInputMethodSubtype subtype)
        {
            InputMethodSyncTrace tracer("CJInputMethod_SwitchCurrentInputMethodAndSubtype");
            int32_t errCode =
                InputMethodController::GetInstance()->SwitchInputMethod(SwitchTrigger::CURRENT_IME, std::string(subtype.name), std::string(subtype.id));
            if (errCode == ErrorCode::NO_ERROR) {
                result = true;
            }
            return errCode;
        }

        int32_t CJ_GetSystemInputMethodConfigAbility(CElementName &elem)
        {
            OHOS::AppExecFwk::ElementName inputMethodConfig;
            int32_t ret = InputMethodController::GetInstance()->GetInputMethodConfig(inputMethodConfig);
            if (ret == ErrorCode::NO_ERROR) {
                elem.deviceId = Utils::MallocCString(inputMethodConfig.GetDeviceID());
                elem.bundleName = Utils::MallocCString(inputMethodConfig.GetBundleName());
                elem.abilityName = Utils::MallocCString(inputMethodConfig.GetAbilityName());
                elem.moduleName = Utils::MallocCString(inputMethodConfig.GetModuleName());
            }
            return ret;
        }

        RetInputMethodSubtype CJ_ListInputMethodSubtype(CInputMethodProperty props)
        {
            IMSA_HILOGD("run in ListInputMethodSubtype");
            RetInputMethodSubtype ret{};
            Property property = Utils::C2InputMethodProperty(props);
            std::vector<SubProperty> subProps;
            int32_t errCode =
                InputMethodController::GetInstance()->ListInputMethodSubtype(property, subProps);
            ret.code = errCode;
            if (errCode != ErrorCode::NO_ERROR) {
                return ret;
            }
            IMSA_HILOGI("exec ListInputMethodSubtype success");
            ret.size = subProps.size();
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

        RetInputMethodSubtype CJ_ListCurrentInputMethodSubtype()
        {
            IMSA_HILOGD("run in ListCurrentInputMethodSubtype");
            RetInputMethodSubtype ret{};
            std::vector<SubProperty> subProps;
            int32_t errCode = InputMethodController::GetInstance()->ListCurrentInputMethodSubtype(subProps);
            ret.code = errCode;
            if (errCode != ErrorCode::NO_ERROR) {
                return ret;
            }
            IMSA_HILOGI("exec ListCurrentInputMethodSubtype success.");
            ret.size = subProps.size();
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

        RetInputMethodProperty CJ_GetInputMethods(bool enable)
        {
            IMSA_HILOGD("run in");
            RetInputMethodProperty ret{};
            std::vector<Property> properties;
            int32_t errCode = InputMethodController::GetInstance()->ListInputMethod(enable, properties);
            ret.code = errCode;
            if (errCode != ErrorCode::NO_ERROR) {
                return ret;
            }
            ret.size = properties.size();
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

        RetInputMethodProperty CJ_GetAllInputMethods()
        {
            IMSA_HILOGD("run in");
            RetInputMethodProperty ret{};
            std::vector<Property> properties;
            int32_t errCode = InputMethodController::GetInstance()->ListInputMethod(properties);
            ret.code = errCode;
            if (errCode != ErrorCode::NO_ERROR) {
                return ret;
            }
            ret.size = properties.size();
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

        int32_t CJ_InputMethodSettingOn(uint32_t type, void (*func)(CInputMethodProperty, CInputMethodSubtype))
        {
            return CJGetInputMethodSetting::GetIMSInstance()->Subscribe(type, func);
        }

        int32_t CJ_InputMethodSettingOff(uint32_t type)
        {
            return CJGetInputMethodSetting::GetIMSInstance()->UnSubscribe(type);
        }

        int32_t CJ_ShowOptionalInputMethods(bool& result)
        {
            IMSA_HILOGD("start JsGetInputMethodSetting.");
            int32_t errCode = InputMethodController::GetInstance()->DisplayOptionalInputMethod();
            if (errCode == ErrorCode::NO_ERROR) {
                IMSA_HILOGI("exec DisplayOptionalInputMethod success");
                result = true;
            }
            return errCode;
        }

        int32_t CJ_InputMethodControllerOn(char* type, int64_t id)
        {
            return 0;
        }

        int32_t CJ_InputMethodControllerOff(char* type)
        {
            return 0;
        }

        int32_t OHOSFfiAttach(bool showKeyboard, CTextConfig txtCfg)
        {
            return CjInputMethodController::Attach(showKeyboard, txtCfg);
        }

        int32_t OHOSFfiDetach()
        {
            return CjInputMethodController::Detach()
        }

        int32_t OHOSFfiShowTextInput()
        {
            return CjInputMethodController::ShowTextInput();
        }

        int32_t OHOSFfiHideTextInput()
        {
            return CjInputMethodController::HideTextInput();
        }

        int32_t OHOSFfiSetCallingWindow(uint32_t windowId)
        {
            return CjInputMethodController::SetCallingWindow(windowId);
        }

        int32_t OHOSFfiUpdateCursor(CCursorInfo cursor)
        {
            return CjInputMethodController::UpdateCursor(cursor);
        }

        int32_t OHOSFfiChangeSelection(char *text, int32_t start, int32_t end)
        {
            return CjInputMethodController::ChangeSelection(std::string(text), start, end);
        }

        int32_t OHOSFfiUpdateAttribute(CInputAttribute inputAttribute)
        {
            return CjInputMethodController::UpdateAttribute(inputAttribute);
        }

        int32_t OHOSFfiShowSoftKeyboard()
        {
            return CjInputMethodController::ShowSoftKeyboard();
        }

        int32_t OHOSFfiHideSoftKeyboard()
        {
            return CjInputMethodController::HideSoftKeyboard();
        }

        int32_t OHOSFfiStopInputSession()
        {
            return CjInputMethodController::StopInputSession();
        }
    }
}