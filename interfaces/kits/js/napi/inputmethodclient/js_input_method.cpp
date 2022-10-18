/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "js_input_method.h"

#include "event_handler.h"
#include "event_runner.h"
#include "input_method_controller.h"
#include "input_method_property.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "string_ex.h"

namespace OHOS {
namespace MiscServices {
napi_value JsInputMethod::Init(napi_env env, napi_value exports)
{
    napi_property_descriptor descriptor[] = {
        DECLARE_NAPI_FUNCTION("switchInputMethod", SwitchInputMethod),
        DECLARE_NAPI_FUNCTION("getCurrentInputMethod", GetCurrentInputMethod),
        DECLARE_NAPI_FUNCTION("getCurrentInputMethodSubtype", GetCurrentInputMethodSubtype),
        DECLARE_NAPI_FUNCTION("switchCurrentInputMethodSubtype", SwitchCurrentInputMethodSubtype),
        DECLARE_NAPI_FUNCTION("switchCurrentInputMethodAndSubtype", SwitchCurrentInputMethodAndSubtype),
    };
    NAPI_CALL(
        env, napi_define_properties(env, exports, sizeof(descriptor) / sizeof(napi_property_descriptor), descriptor));
    return exports;
};

std::string JsInputMethod::GetStringProperty(napi_env env, napi_value obj)
{
    char propValue[MAX_VALUE_LEN] = {0};
    size_t propLen;
    if (napi_get_value_string_utf8(env, obj, propValue, MAX_VALUE_LEN, &propLen) != napi_ok) {
        IMSA_HILOGE("GetStringProperty error");
    }
    return std::string(propValue);
}

int32_t JsInputMethod::GetNumberProperty(napi_env env, napi_value obj)
{
    int32_t out;
    if (napi_get_value_int32(env, obj, &out) != napi_ok) {
        IMSA_HILOGE("GetInt32Property error");
    }
    return out;
}

napi_status JsInputMethod::GetInputMethodProperty(napi_env env, napi_value argv,
    std::shared_ptr<SwitchInputMethodContext> ctxt)
{
    napi_valuetype valueType = napi_undefined;
    napi_status status = napi_generic_failure;
    status = napi_typeof(env, argv, &valueType);
    if (valueType == napi_object) {
        napi_value result = nullptr;
        // has at least two properties : name, id
        status = napi_get_named_property(env, argv, "packageName", &result);
        if (status != napi_ok) {
            status = napi_get_named_property(env, argv, "name", &result);
        }
        if (status != napi_ok) {
            JsUtils::ThrowException(env, IMFErrorCode::EXCEPTION_PARAMCHECK,
                                    "missing packageName parameter.", TYPE_STRING);
            return status;
        }
        ctxt->packageName = GetStringProperty(env, result);
        result = nullptr;
        status = napi_get_named_property(env, argv, "methodId", &result);
        if (status != napi_ok) {
            status = napi_get_named_property(env, argv, "id", &result);
        }
        if (status != napi_ok) {
            JsUtils::ThrowException(env, IMFErrorCode::EXCEPTION_PARAMCHECK,
                                    "missing methodId parameter.", TYPE_STRING);
            return status;
        }
        ctxt->methodId = GetStringProperty(env, result);
        IMSA_HILOGI("methodId:%{public}s and packageName:%{public}s",
            ctxt->methodId.c_str(), ctxt->packageName.c_str());
    }
    return status;
}

napi_status JsInputMethod::GetInputMethodSubProperty(
    napi_env env, napi_value argv, std::shared_ptr<SwitchInputMethodContext> ctxt)
{
    napi_valuetype valueType = napi_undefined;
    napi_status status = napi_generic_failure;
    status = napi_typeof(env, argv, &valueType);
    if (valueType == napi_object) {
        napi_value result = nullptr;
        status = napi_get_named_property(env, argv, "id", &result);
        if (status != napi_ok) {
            JsUtils::ThrowException(env, IMFErrorCode::EXCEPTION_PARAMCHECK,
                                    "missing id parameter.", TYPE_STRING);
            return status;
        }
        ctxt->id = GetStringProperty(env, result);
        result = nullptr;
        status = napi_get_named_property(env, argv, "label", &result);
        if (status != napi_ok) {
            JsUtils::ThrowException(env, IMFErrorCode::EXCEPTION_PARAMCHECK,
                                    "missing label parameter.", TYPE_STRING);
            return status;
        }
        ctxt->label = GetStringProperty(env, result);
        IMSA_HILOGI("label:%{public}s and id:%{public}s", ctxt->label.c_str(), ctxt->id.c_str());
    }
    return status;
}

napi_value JsInputMethod::GetJsInputMethodProperty(napi_env env, const Property &property)
{
    napi_value prop = nullptr;
    napi_create_object(env, &prop);

    napi_value packageName = nullptr;
    napi_create_string_utf8(env, property.name.c_str(), NAPI_AUTO_LENGTH, &packageName);
    napi_set_named_property(env, prop, "packageName", packageName);
    if (packageName == nullptr) {
        napi_set_named_property(env, prop, "name", packageName);
    }

    napi_value methodId = nullptr;
    napi_create_string_utf8(env, property.id.c_str(), NAPI_AUTO_LENGTH, &methodId);
    napi_set_named_property(env, prop, "methodId", methodId);
    if (methodId == nullptr) {
        napi_set_named_property(env, prop, "id", methodId);
    }

    return prop;
}

napi_value JsInputMethod::GetJsInputMethodSubProperty(napi_env env, const SubProperty &subProperty)
{
    napi_value prop = nullptr;
    napi_create_object(env, &prop);

    napi_value id = nullptr;
    napi_create_string_utf8(env, subProperty.id.c_str(), NAPI_AUTO_LENGTH, &id);
    napi_set_named_property(env, prop, "id", id);

    napi_value label = nullptr;
    napi_create_string_utf8(env, subProperty.label.c_str(), NAPI_AUTO_LENGTH, &label);
    napi_set_named_property(env, prop, "label", label);

    napi_value name = nullptr;
    napi_create_string_utf8(env, subProperty.name.c_str(), NAPI_AUTO_LENGTH, &name);
    napi_set_named_property(env, prop, "name", name);

    napi_value mode = nullptr;
    napi_create_string_utf8(env, subProperty.mode.c_str(), NAPI_AUTO_LENGTH, &mode);
    napi_set_named_property(env, prop, "mode", mode);

    napi_value locale = nullptr;
    napi_create_string_utf8(env, subProperty.locale.c_str(), NAPI_AUTO_LENGTH, &locale);
    napi_set_named_property(env, prop, "locale", locale);

    napi_value language = nullptr;
    napi_create_string_utf8(env, subProperty.language.c_str(), NAPI_AUTO_LENGTH, &language);
    napi_set_named_property(env, prop, "language", language);

    napi_value icon = nullptr;
    napi_create_string_utf8(env, subProperty.icon.c_str(), NAPI_AUTO_LENGTH, &icon);
    napi_set_named_property(env, prop, "icon", icon);

    napi_value iconId = nullptr;
    napi_create_int32(env, subProperty.iconId, &iconId);
    napi_set_named_property(env, prop, "iconId", iconId);
    return prop;
}

napi_value JsInputMethod::GetJSInputMethodSubProperties(napi_env env, const std::vector<SubProperty> &subProperties)
{
    uint32_t index = 0;
    napi_value prop = nullptr;
    napi_create_array(env, &prop);
    if (prop == nullptr) {
        IMSA_HILOGE("create array failed");
        return prop;
    }
    for (const auto &subproperty : subProperties) {
        napi_value pro = GetJsInputMethodSubProperty(env, subproperty);
        napi_set_element(env, prop, index, pro);
        index++;
    }
    return prop;
}

napi_value JsInputMethod::GetJSInputMethodProperties(napi_env env, const std::vector<Property> &properties)
{
    uint32_t index = 0;
    napi_value prop = nullptr;
    napi_create_array(env, &prop);
    if (prop == nullptr) {
        IMSA_HILOGE("create array failed");
        return prop;
    }
    for (const auto &property : properties) {
        napi_value pro = GetJsInputMethodProperty(env, property);
        napi_set_element(env, prop, index, pro);
        index++;
    }
    return prop;
}

napi_value JsInputMethod::SwitchInputMethod(napi_env env, napi_callback_info info)
{
    auto ctxt = std::make_shared<SwitchInputMethodContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        if (argc < 1) {
            JsUtils::ThrowException(env, IMFErrorCode::EXCEPTION_PARAMCHECK,
                                    "should has 1 parameters!", TYPE_NONE);
            return napi_invalid_arg;
        }
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[0], &valueType);
        if (valueType != napi_object) {
            JsUtils::ThrowException(env, IMFErrorCode::EXCEPTION_PARAMCHECK, " target: ", TYPE_OBJECT);
            return napi_ok;
        }
        napi_status status = GetInputMethodProperty(env, argv[0], ctxt);
        return status;
    };
    auto output = [ctxt](napi_env env, napi_value *result) -> napi_status {
        napi_status status = napi_get_boolean(env, ctxt->isSwitchInput, result);
        IMSA_HILOGE("output  napi_get_boolean != nullptr[%{public}d]", result != nullptr);
        return status;
    };
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        int32_t errCode = InputMethodController::GetInstance()->SwitchInputMethod(ctxt->packageName);
        if (errCode == ErrorCode::NO_ERROR) {
            IMSA_HILOGI("exec SwitchInputMethod success");
            ctxt->status = napi_ok;
            ctxt->SetState(ctxt->status);
            ctxt->isSwitchInput = true;
        } else {
            ctxt->SetErrorCode(errCode);
        }
    };
    ctxt->SetAction(std::move(input), std::move(output));
    AsyncCall asyncCall(env, info, std::dynamic_pointer_cast<AsyncCall::Context>(ctxt), 1);
    return asyncCall.Call(env, exec);
}

napi_value JsInputMethod::GetCurrentInputMethod(napi_env env, napi_callback_info info)
{
    std::shared_ptr<Property> property = InputMethodController::GetInstance()->GetCurrentInputMethod();
    if (property == nullptr) {
        IMSA_HILOGE("get current inputmethod is nullptr");
        napi_value result = nullptr;
        napi_get_null(env, &result);
        return result;
    }
    return GetJsInputMethodProperty(env, { property->name, property->id });
}

napi_value JsInputMethod::GetCurrentInputMethodSubtype(napi_env env, napi_callback_info info)
{
    std::shared_ptr<SubProperty> subProperty = InputMethodController::GetInstance()->GetCurrentInputMethodSubtype();
    if (subProperty == nullptr) {
        IMSA_HILOGE("get current inputmethodsubtype is nullptr");
        napi_value result = nullptr;
        napi_get_null(env, &result);
        return result;
    }
    return GetJsInputMethodSubProperty(env, *subProperty);
}

napi_value JsInputMethod::SwitchCurrentInputMethodSubtype(napi_env env, napi_callback_info info)
{
    auto ctxt = std::make_shared<SwitchInputMethodContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        if (argc < 1) {
            JsUtils::ThrowException(env, IMFErrorCode::EXCEPTION_PARAMCHECK,
                                    "should has one parameter.", TYPE_NONE);
            return napi_invalid_arg;
        }
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[0], &valueType);
        if (valueType != napi_object) {
            JsUtils::ThrowException(env, IMFErrorCode::EXCEPTION_PARAMCHECK, " inputMethodSubtype: ", TYPE_OBJECT);
            return napi_object_expected;
        }
        napi_status status = GetInputMethodSubProperty(env, argv[0], ctxt);
        return status;
    };
    auto output = [ctxt](napi_env env, napi_value *result) -> napi_status {
        napi_status status = napi_get_boolean(env, ctxt->isSwitchInput, result);
        IMSA_HILOGE("output napi_get_boolean != nullptr[%{public}d]", result != nullptr);
        return status;
    };
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        int32_t errCode = InputMethodController::GetInstance()->SwitchInputMethod(ctxt->id, ctxt->label);
        if (errCode == ErrorCode::NO_ERROR) {
            IMSA_HILOGI("exec SwitchInputMethod success");
            ctxt->status = napi_ok;
            ctxt->SetState(ctxt->status);
            ctxt->isSwitchInput = true;
        } else {
            ctxt->SetErrorCode(errCode);
        }
    };
    ctxt->SetAction(std::move(input), std::move(output));
    AsyncCall asyncCall(env, info, std::dynamic_pointer_cast<AsyncCall::Context>(ctxt), 1);
    return asyncCall.Call(env, exec);
}

napi_value JsInputMethod::SwitchCurrentInputMethodAndSubtype(napi_env env, napi_callback_info info)
{
    auto ctxt = std::make_shared<SwitchInputMethodContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        if (argc < 2) {
            JsUtils::ThrowException(env, IMFErrorCode::EXCEPTION_PARAMCHECK,
                                    "should has two parameter.", TYPE_NONE);
            return napi_invalid_arg;
        }
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[0], &valueType);
        if (valueType != napi_object) {
            JsUtils::ThrowException(env, IMFErrorCode::EXCEPTION_PARAMCHECK, " inputMethodProperty: ", TYPE_OBJECT);
            return napi_object_expected;
        }
        napi_typeof(env, argv[1], &valueType);
        if (valueType != napi_object) {
            JsUtils::ThrowException(env, IMFErrorCode::EXCEPTION_PARAMCHECK, " inputMethodSubtype: ", TYPE_OBJECT);
            return napi_object_expected;
        }
        napi_status status = GetInputMethodSubProperty(env, argv[1], ctxt);
        return status;
    };
    auto output = [ctxt](napi_env env, napi_value *result) -> napi_status {
        napi_status status = napi_get_boolean(env, ctxt->isSwitchInput, result);
        IMSA_HILOGE("output  napi_get_boolean != nullptr[%{public}d]", result != nullptr);
        return status;
    };
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        int32_t errCode = InputMethodController::GetInstance()->SwitchInputMethod(ctxt->id, ctxt->label);
        if (errCode == ErrorCode::NO_ERROR) {
            IMSA_HILOGI("exec SwitchInputMethod success");
            ctxt->status = napi_ok;
            ctxt->SetState(ctxt->status);
            ctxt->isSwitchInput = true;
        } else {
            ctxt->SetErrorCode(errCode);
        }
    };
    ctxt->SetAction(std::move(input), std::move(output));
    AsyncCall asyncCall(env, info, std::dynamic_pointer_cast<AsyncCall::Context>(ctxt), 2);
    return asyncCall.Call(env, exec);
}
} // namespace MiscServices
} // namespace OHOS