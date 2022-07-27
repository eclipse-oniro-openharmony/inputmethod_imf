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
#include "js_get_input_method_setting.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "string_ex.h"
#include "input_method_controller.h"

namespace OHOS {
namespace MiscServices {
int32_t MAX_TYPE_NUM = 128;
thread_local napi_ref JsGetInputMethodSetting::IMSRef_ = nullptr;
const std::string JsGetInputMethodSetting::IMS_CLASS_NAME = "InputMethodSetting";
napi_value JsGetInputMethodSetting::Init(napi_env env, napi_value exports)
{
        napi_value maxTypeNumber = nullptr;
        napi_create_int32(env, MAX_TYPE_NUM, &maxTypeNumber);
        
        napi_property_descriptor descriptor[] = {
        DECLARE_NAPI_FUNCTION("getInputMethodSetting", GetInputMethodSetting),
        DECLARE_NAPI_PROPERTY("MAX_TYPE_NUM", maxTypeNumber),
    };
    NAPI_CALL(
        env, napi_define_properties(env, exports, sizeof(descriptor) / sizeof(napi_property_descriptor), descriptor));

    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("listInputMethod", ListInputMethod),
        DECLARE_NAPI_FUNCTION("displayOptionalInputMethod", DisplayOptionalInputMethod),
    };
    napi_value cons = nullptr;
    NAPI_CALL(env, napi_define_class(env, IMS_CLASS_NAME.c_str(), IMS_CLASS_NAME.size(),
        JsConstructor, nullptr, sizeof(properties) / sizeof(napi_property_descriptor), properties, &cons));
    NAPI_CALL(env, napi_create_reference(env, cons, 1, &IMSRef_));
    NAPI_CALL(env, napi_set_named_property(env, exports, IMS_CLASS_NAME.c_str(), cons));
    return exports;
}

napi_value JsGetInputMethodSetting::JsConstructor(napi_env env, napi_callback_info cbinfo)
{
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, cbinfo, nullptr, nullptr, &thisVar, nullptr));

    JsGetInputMethodSetting *IMSobject = new (std::nothrow) JsGetInputMethodSetting();
    if (IMSobject == nullptr) {
        IMSA_HILOGE("IMSobject is nullptr");
        napi_value result = nullptr;
        napi_get_null(env, &result);
        return result;
    }
    napi_wrap(env, thisVar, IMSobject, [](napi_env env, void *data, void *hint) {
        auto* objInfo = reinterpret_cast<JsGetInputMethodSetting*>(data);
        if (objInfo != nullptr) {
            IMSA_HILOGE("objInfo is nullptr");
            delete objInfo;
        }
    }, nullptr, nullptr);

    return thisVar;
}

napi_value JsGetInputMethodSetting::GetInputMethodSetting(napi_env env, napi_callback_info info)
{
    napi_value instance = nullptr;
    napi_value cons = nullptr;
    if (napi_get_reference_value(env, IMSRef_, &cons) != napi_ok) {
        IMSA_HILOGE("GetInputMethodSetting::napi_get_reference_value not ok");
        return nullptr;
    }
    if (napi_new_instance(env, cons, 0, nullptr, &instance) != napi_ok) {
        IMSA_HILOGE("GetInputMethodSetting::napi_new_instance not ok");
        return nullptr;
    }
    return instance;
}

napi_value JsGetInputMethodSetting::GetJSInputMethodProperty(napi_env env,
    std::vector<InputMethodProperty*> &properties)
{
    IMSA_HILOGD("run in GetResult %{public}lu", properties.size());
    uint32_t index = 0;
    napi_value result = nullptr;
    napi_create_array(env, &result);
    if (result == nullptr) {
        IMSA_HILOGE("create_array failed");
        return result;
    }
    for (const auto &item : properties) {
        if (item == nullptr) {
            IMSA_HILOGE("GetResult::item is null");
            continue;
        }
        napi_value InputMethodSetting = nullptr;
        napi_create_object(env, &InputMethodSetting);

        std::string packageName = Str16ToStr8(item->mPackageName);
        napi_value jsPackageName = nullptr;
        napi_create_string_utf8(env, packageName.c_str(), NAPI_AUTO_LENGTH, &jsPackageName);
        napi_set_named_property(env, InputMethodSetting, "packageName", jsPackageName);

        std::string methodId = Str16ToStr8(item->mAbilityName);
        napi_value jsMethodId = nullptr;
        napi_create_string_utf8(env, methodId.c_str(), NAPI_AUTO_LENGTH, &jsMethodId);
        napi_set_named_property(env, InputMethodSetting, "methodId", jsMethodId);

        napi_set_element(env, result, index, InputMethodSetting);
        index++;
    }
    return result;
}

napi_value JsGetInputMethodSetting::ListInputMethod(napi_env env, napi_callback_info info)
{
    auto ctxt = std::make_shared<ListInputContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        NAPI_ASSERT_BASE(env, argc == 0 || argc == 1, " should null or 1 parameters!", napi_invalid_arg);
        return napi_ok;
    };
    auto output = [ctxt](napi_env env, napi_value *result) -> napi_status {
        napi_value data = GetJSInputMethodProperty(env, ctxt->properties);
        *result = data;
        return napi_ok;
    };
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        ctxt->properties = InputMethodController::GetInstance()->ListInputMethod();
        if (!ctxt->properties.empty()) {
            IMSA_HILOGE("exec ---- ListInputMethod success");
            ctxt->status = napi_ok;
        }
    };
    ctxt->SetAction(std::move(input), std::move(output));
    AsyncCall asyncCall(env, info, std::dynamic_pointer_cast<AsyncCall::Context>(ctxt), 0);
    return asyncCall.Call(env, exec);
}

napi_value JsGetInputMethodSetting::DisplayOptionalInputMethod(napi_env env, napi_callback_info info)
{
    auto ctxt = std::make_shared<DisplayOptionalInputMethodContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        NAPI_ASSERT_BASE(env, argc == 0 || argc == 1, " should null or 1 parameters!", napi_invalid_arg);
        return napi_ok;
    };
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        int32_t errCode = InputMethodController::GetInstance()->DisplayOptionalInputMethod();
        if (errCode == ErrorCode::NO_ERROR) {
            IMSA_HILOGE("exec ---- DisplayOptionalInputMethod success");
            ctxt->status = napi_ok;
        }
    };
    ctxt->SetAction(std::move(input));
    AsyncCall asyncCall(env, info, std::dynamic_pointer_cast<AsyncCall::Context>(ctxt), 0);
    return asyncCall.Call(env, exec);
}
}
}