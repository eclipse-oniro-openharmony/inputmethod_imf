/*
 * Copyright (c) 2022-2022 Huawei Device Co., Ltd.
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

#include "input_method_ability.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace MiscServices {
thread_local napi_ref JsKeyboardControllerEngine::KCERef_ = nullptr;
const std::string JsKeyboardControllerEngine::KCE_CLASS_NAME = "KeyboardController";
napi_value JsKeyboardControllerEngine::Init(napi_env env, napi_value info)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("hideKeyboard", HideKeyboard),
        DECLARE_NAPI_FUNCTION("hide", Hide),
        DECLARE_NAPI_FUNCTION("exitCurrentInputType", ExitCurrentInputType),
    };
    napi_value cons = nullptr;
    NAPI_CALL(env, napi_define_class(env, KCE_CLASS_NAME.c_str(), KCE_CLASS_NAME.size(), JsConstructor, nullptr,
                       sizeof(properties) / sizeof(napi_property_descriptor), properties, &cons));
    NAPI_CALL(env, napi_create_reference(env, cons, 1, &KCERef_));
    NAPI_CALL(env, napi_set_named_property(env, info, KCE_CLASS_NAME.c_str(), cons));

    return info;
}

napi_value JsKeyboardControllerEngine::JsConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr));

    JsKeyboardControllerEngine *controllerObject = new (std::nothrow) JsKeyboardControllerEngine();
    if (controllerObject == nullptr) {
        IMSA_HILOGE("controllerObject is nullptr!");
        napi_value result = nullptr;
        napi_get_null(env, &result);
        return result;
    }
    auto finalize = [](napi_env env, void *data, void *hint) {
        IMSA_HILOGD("JsKBCEngine finalize.");
        auto *objInfo = reinterpret_cast<JsKeyboardControllerEngine *>(data);
        if (objInfo != nullptr) {
            delete objInfo;
        }
    };
    napi_status status = napi_wrap(env, thisVar, controllerObject, finalize, nullptr, nullptr);
    if (status != napi_ok) {
        IMSA_HILOGE("JsKeyboardControllerEngine wrap failed: %{public}d!", status);
        delete controllerObject;
        return nullptr;
    }

    return thisVar;
}

napi_value JsKeyboardControllerEngine::GetKeyboardControllerInstance(napi_env env)
{
    napi_value instance = nullptr;
    napi_value cons = nullptr;
    if (napi_get_reference_value(env, KCERef_, &cons) != napi_ok) {
        IMSA_HILOGE("failed to get reference value!");
        return nullptr;
    }
    if (napi_new_instance(env, cons, 0, nullptr, &instance) != napi_ok) {
        IMSA_HILOGE("failed to new instance value!");
        return nullptr;
    }
    IMSA_HILOGD("success");
    return instance;
}

napi_value JsKeyboardControllerEngine::Hide(napi_env env, napi_callback_info info)
{
    auto ctxt = std::make_shared<HideContext>();
    auto input = [ctxt](
                     napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status { return napi_ok; };
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        int32_t code = InputMethodAbility::GetInstance()->HideKeyboardSelf();
        if (code == ErrorCode::NO_ERROR) {
            ctxt->status = napi_ok;
            ctxt->SetState(ctxt->status);
        } else {
            ctxt->SetErrorCode(code);
        }
    };
    ctxt->SetAction(std::move(input));
    // 1 means JsAPI:hide has 1 params at most.
    AsyncCall asyncCall(env, info, ctxt, 1);
    return asyncCall.Call(env, exec, "keyboard.hide");
}

napi_value JsKeyboardControllerEngine::HideKeyboard(napi_env env, napi_callback_info info)
{
    auto ctxt = std::make_shared<HideKeyboardContext>();
    auto input = [ctxt](
                     napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status { return napi_ok; };
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        InputMethodAbility::GetInstance()->HideKeyboardSelf();
        ctxt->status = napi_ok;
    };
    ctxt->SetAction(std::move(input));
    // 1 means JsAPI:hideKeyboard has 1 params at most.
    AsyncCall asyncCall(env, info, ctxt, 1);
    return asyncCall.Call(env, exec, "hideKeyboard");
}

napi_value JsKeyboardControllerEngine::ExitCurrentInputType(napi_env env, napi_callback_info info)
{
    auto ctxt = std::make_shared<ExitContext>();
    auto input = [ctxt](
                     napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status { return napi_ok; };
    auto output = [ctxt](napi_env env, napi_value *result) -> napi_status { return napi_ok; };
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        int32_t errorCode = InputMethodAbility::GetInstance()->ExitCurrentInputType();
        if (errorCode == ErrorCode::NO_ERROR) {
            ctxt->status = napi_ok;
            ctxt->SetState(napi_ok);
        } else {
            ctxt->SetErrorCode(errorCode);
        }
    };
    ctxt->SetAction(std::move(input), std::move(output));
    AsyncCall asyncCall(env, info, ctxt, 1);
    return asyncCall.Call(env, exec, "exitCurrentInputType");
}
} // namespace MiscServices
} // namespace OHOS