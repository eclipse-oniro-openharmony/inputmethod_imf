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
#include "input_method_controller.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "event_handler.h"
#include "event_runner.h"
#include "string_ex.h"
#include "js_input_method.h"

namespace OHOS {
namespace MiscServices {
napi_value JsInputMethod::Init(napi_env env, napi_value exports)
{
    napi_property_descriptor descriptor[] = {
        DECLARE_NAPI_FUNCTION("switchInputMethod", SwitchInputMethod),
        DECLARE_NAPI_FUNCTION("getCurrentInputMethod", GetCurrentInputMethod),
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

napi_status JsInputMethod::GetInputMethodProperty(napi_env env, napi_value argv,
    std::shared_ptr<SwitchInputMethodContext> ctxt)
{
    napi_valuetype valueType = napi_undefined;
    napi_status status = napi_generic_failure;
    status = napi_typeof(env, argv, &valueType);
    if (valueType == napi_object) {
        napi_value result = nullptr;
        status = napi_get_named_property(env, argv, "packageName", &result);
        ctxt->packageName = GetStringProperty(env, result);
        result = nullptr;
        status = napi_get_named_property(env, argv, "methodId", &result);
        ctxt->methodId = GetStringProperty(env, result);
        IMSA_HILOGI("methodId:%{public}s and packageName:%{public}s",
            ctxt->methodId.c_str(), ctxt->packageName.c_str());
    }
    return status;
}

napi_value JsInputMethod::GetJsInputMethodProperty(napi_env env, const Property &property)
{
    napi_value prop = nullptr;
    napi_create_object(env, &prop);

    napi_value packageName = nullptr;
    napi_create_string_utf8(env, property.packageName.c_str(), NAPI_AUTO_LENGTH, &packageName);
    napi_set_named_property(env, prop, "packageName", packageName);

    napi_value methodId = nullptr;
    napi_create_string_utf8(env, property.abilityName.c_str(), NAPI_AUTO_LENGTH, &methodId);
    napi_set_named_property(env, prop, "methodId", methodId);

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
        NAPI_ASSERT_BASE(env, argc == 1 || argc == 2, " should 1 or 2 parameters!", napi_invalid_arg);
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
            IMSA_HILOGI("exec  SwitchInputMethod success");
            ctxt->status = napi_ok;
            ctxt->isSwitchInput = true;
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
    return GetJsInputMethodProperty(env, { property->packageName, property->abilityName });
}
} // namespace MiscServices
} // namespace OHOS