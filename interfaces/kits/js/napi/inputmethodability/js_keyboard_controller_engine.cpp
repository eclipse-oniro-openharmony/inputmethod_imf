/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "js_keyboard_controller_engine.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "input_method_ability.h"

namespace OHOS {
namespace MiscServices {
thread_local napi_ref JsKeyboardControllerEngine::KCERef_ = nullptr;
const std::string JsKeyboardControllerEngine::KCE_CLASS_NAME = "KeyboardController";
napi_value JsKeyboardControllerEngine::Init(napi_env env, napi_value info)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("hideKeyboard", HideKeyboard),
    };
    napi_value cons = nullptr;
    NAPI_CALL(env, napi_define_class(env, KCE_CLASS_NAME.c_str(), KCE_CLASS_NAME.size(),
        JsConstructor, nullptr, sizeof(properties) / sizeof(napi_property_descriptor), properties, &cons));
    NAPI_CALL(env, napi_create_reference(env, cons, 1, &KCERef_));
    NAPI_CALL(env, napi_set_named_property(env, info, KCE_CLASS_NAME.c_str(), cons));

    return info;
}

napi_value JsKeyboardControllerEngine::JsConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr));

    JsKeyboardControllerEngine *IMSobject = new (std::nothrow) JsKeyboardControllerEngine();
    if (IMSobject == nullptr) {
        IMSA_HILOGE("IMSobject is nullptr");
        napi_value result = nullptr;
        napi_get_null(env, &result);
        return result;
    }
    napi_wrap(env, thisVar, IMSobject, [](napi_env env, void *data, void *hint) {
        auto* objInfo = reinterpret_cast<JsKeyboardControllerEngine*>(data);
        if (objInfo != nullptr) {
            delete objInfo;
        }
    }, nullptr, nullptr);

    return thisVar;
}

napi_value JsKeyboardControllerEngine::GetKeyboardControllerInstance(napi_env env)
{
    napi_value instance = nullptr;
    napi_value cons = nullptr;
    if (napi_get_reference_value(env, KCERef_, &cons) != napi_ok) {
        IMSA_HILOGE("GetKeyboardController::napi_get_reference_value not ok");
        return nullptr;
    }
    if (napi_new_instance(env, cons, 0, nullptr, &instance) != napi_ok) {
        IMSA_HILOGE("GetKeyboardController::napi_new_instance not ok");
        return nullptr;
    }
    IMSA_HILOGE("New the JsKeyboardControllerEngine instance complete");
    return instance;
}

napi_value JsKeyboardControllerEngine::HideKeyboard(napi_env env, napi_callback_info info)
{   
    auto ctxt = std::make_shared<HideKeyboardContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        NAPI_ASSERT_BASE(env, argc == 0 || argc == 1, " should null or 1 parameters!", napi_invalid_arg);
        return napi_ok;
    };
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        InputMethodAbility::GetInstance()->HideKeyboardSelf();
        ctxt->status = napi_ok;
    };
    ctxt->SetAction(std::move(input));
    AsyncCall asyncCall(env, info, std::dynamic_pointer_cast<AsyncCall::Context>(ctxt), 0);
    return asyncCall.Call(env, exec);
}
}
}