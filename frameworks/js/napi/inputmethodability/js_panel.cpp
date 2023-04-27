/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "js_panel.h"

#include "input_method_ability.h"
#include "js_utils.h"
#include "napi/native_common.h"
#include "panel_listener_impl.h"

namespace OHOS {
namespace MiscServices {
constexpr size_t ARGC_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
const std::string JsPanel::CLASS_NAME = "Panel";
thread_local napi_ref JsPanel::panelConstructorRef_ = nullptr;

napi_value JsPanel::Constructor(napi_env env)
{
    IMSA_HILOGI("JsPanel in.");
    const napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("setUiContent", SetUiContent),
        DECLARE_NAPI_FUNCTION("resize", Resize),
        DECLARE_NAPI_FUNCTION("moveTo", MoveTo),
        DECLARE_NAPI_FUNCTION("show", Show),
        DECLARE_NAPI_FUNCTION("hide", Hide),
        DECLARE_NAPI_FUNCTION("changeFlag", ChangeFlag),
        DECLARE_NAPI_FUNCTION("on", Subscribe),
        DECLARE_NAPI_FUNCTION("off", UnSubscribe),
    };
    napi_value constructor = nullptr;
    NAPI_CALL(env, napi_define_class(env, CLASS_NAME.c_str(), CLASS_NAME.size(), JsNew, nullptr,
                       sizeof(properties) / sizeof(napi_property_descriptor), properties, &constructor));
    NAPI_ASSERT(env, constructor != nullptr, "napi_define_class failed!");
    NAPI_CALL(env, napi_create_reference(env, constructor, 1, &panelConstructorRef_));
    return constructor;
}

napi_value JsPanel::JsNew(napi_env env, napi_callback_info info)
{
    IMSA_HILOGD("JsPanel, create panel instance in.");
    JsPanel *panel = new (std::nothrow) JsPanel();
    NAPI_ASSERT(env, panel != nullptr, "no memory for JsPanel");
    auto finalize = [](napi_env env, void *data, void *hint) {
        IMSA_HILOGI("jsPanel finalize.");
        auto *jsPanel = reinterpret_cast<JsPanel *>(data);
        NAPI_ASSERT_RETURN_VOID(env, jsPanel != nullptr, "finalize null!");
        jsPanel->GetNative() = nullptr;
        delete jsPanel;
    };
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr));
    napi_status status = napi_wrap(env, thisVar, panel, finalize, nullptr, nullptr);
    if (status != napi_ok) {
        IMSA_HILOGE("JsPanel napi_wrap failed: %{public}d", status);
        delete panel;
        return nullptr;
    }
    return thisVar;
}

JsPanel::~JsPanel()
{
    inputMethodPanel_ = nullptr;
}

void JsPanel::SetNative(const std::shared_ptr<InputMethodPanel> &panel)
{
    inputMethodPanel_ = panel;
}

std::shared_ptr<InputMethodPanel> &JsPanel::GetNative()
{
    return inputMethodPanel_;
}

napi_value JsPanel::SetUiContent(napi_env env, napi_callback_info info)
{
    IMSA_HILOGI("JsPanel in.");
    auto ctxt = std::make_shared<PanelContentContext>(env, info);
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        napi_status status = napi_generic_failure;
        PARAM_CHECK_RETURN(env, argc >= 1, "should 1 or 2 parameters!", TYPE_NONE, status);
        status = JsUtils::GetValue(env, argv[ARGC_ZERO], ctxt->path);
        NAPI_ASSERT_BASE(env, status == napi_ok, "get path failed!", status);
        // if type of argv[1] is object, we will get value of 'storage' from it.
        if (argc >= 2) {
            napi_valuetype valueType = napi_undefined;
            napi_status status = napi_typeof(env, argv[1], &valueType);
            NAPI_ASSERT_BASE(env, status == napi_ok, "get valueType failed!", status);
            if (valueType == napi_object) {
                NativeValue *storage = nullptr;
                storage = reinterpret_cast<NativeValue *>(argv[1]);
                auto contentStorage = (storage == nullptr)
                                          ? nullptr
                                          : std::shared_ptr<NativeReference>(
                                                reinterpret_cast<NativeEngine *>(env)->CreateReference(storage, 1));
                ctxt->contentStorage = contentStorage;
            }
        }
        return napi_ok;
    };

    auto exec = [ctxt](AsyncCall::Context *ctx) {
        ctxt->SetState(napi_ok);
    };
    auto output = [ctxt](napi_env env, napi_value *result) -> napi_status {
        auto &inputMethodPanel = reinterpret_cast<JsPanel *>(ctxt->native)->GetNative();
        NAPI_ASSERT_BASE(env, inputMethodPanel != nullptr, "inputMethodPanel is nullptr!", napi_generic_failure);
        auto code = inputMethodPanel->SetUiContent(ctxt->path, *(reinterpret_cast<NativeEngine *>(env)),
                                                   ctxt->contentStorage);
        NAPI_ASSERT_BASE(env, code == ErrorCode::NO_ERROR, "SetUiContent failed!", napi_generic_failure);
        return napi_ok;
    };
    ctxt->SetAction(std::move(input), std::move(output));
    AsyncCall asyncCall(env, info, ctxt, 1);
    return asyncCall.Call(env, exec);
}

napi_value JsPanel::Resize(napi_env env, napi_callback_info info)
{
    auto ctxt = std::make_shared<PanelContentContext>(env, info);
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        napi_status status = napi_generic_failure;
        PARAM_CHECK_RETURN(env, argc > 1, "should 2 or 3 parameters!", TYPE_NONE, status);
        status = JsUtils::GetValue(env, argv[ARGC_ZERO], ctxt->width);
        NAPI_ASSERT_BASE(env, status == napi_ok, "get width failed!", status);
        status = JsUtils::GetValue(env, argv[ARGC_ONE], ctxt->height);
        NAPI_ASSERT_BASE(env, status == napi_ok, "get height failed!", status);
        return napi_ok;
    };

    auto exec = [ctxt](AsyncCall::Context *ctx) {
        auto &inputMethodPanel = reinterpret_cast<JsPanel *>(ctxt->native)->GetNative();
        CHECK_RETURN_VOID(inputMethodPanel != nullptr, "inputMethodPanel_ is nullptr.");
        auto code = inputMethodPanel->Resize(ctxt->width, ctxt->height);
        if (code == ErrorCode::NO_ERROR) {
            ctxt->SetState(napi_ok);
            return;
        }
        ctxt->SetErrorCode(code);
    };
    ctxt->SetAction(std::move(input));
    AsyncCall asyncCall(env, info, ctxt, ARGC_TWO);
    return asyncCall.Call(env, exec);
}

napi_value JsPanel::MoveTo(napi_env env, napi_callback_info info)
{
    auto ctxt = std::make_shared<PanelContentContext>(env, info);
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        napi_status status = napi_generic_failure;
        PARAM_CHECK_RETURN(env, argc > 1, " should 2 or 3 parameters! ", TYPE_NONE, status);
        status = JsUtils::GetValue(env, argv[ARGC_ZERO], ctxt->x);
        NAPI_ASSERT_BASE(env, status == napi_ok, "get x failed!", status);
        status = JsUtils::GetValue(env, argv[ARGC_ONE], ctxt->y);
        NAPI_ASSERT_BASE(env, status == napi_ok, "get y failed!", status);
        return napi_ok;
    };

    auto exec = [ctxt](AsyncCall::Context *ctx) {
        auto &inputMethodPanel = reinterpret_cast<JsPanel *>(ctxt->native)->GetNative();
        CHECK_RETURN_VOID(inputMethodPanel != nullptr, "inputMethodPanel_ is nullptr.");
        auto code = inputMethodPanel->MoveTo(ctxt->x, ctxt->y);
        if (code == ErrorCode::NO_ERROR) {
            ctxt->SetState(napi_ok);
            return;
        }
        ctxt->SetErrorCode(code);
    };
    ctxt->SetAction(std::move(input));
    AsyncCall asyncCall(env, info, ctxt, ARGC_TWO);
    return asyncCall.Call(env, exec);
}

napi_value JsPanel::Show(napi_env env, napi_callback_info info)
{
    auto ctxt = std::make_shared<PanelContentContext>(env, info);
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        auto &inputMethodPanel = reinterpret_cast<JsPanel *>(ctxt->native)->GetNative();
        CHECK_RETURN_VOID(inputMethodPanel != nullptr, "inputMethodPanel_ is nullptr.");
        auto code = inputMethodPanel->ShowPanel();
        if (code == ErrorCode::NO_ERROR) {
            ctxt->SetState(napi_ok);
            return;
        }
        ctxt->SetErrorCode(code);
    };
    AsyncCall asyncCall(env, info, ctxt, 0);
    return asyncCall.Call(env, exec);
}

napi_value JsPanel::Hide(napi_env env, napi_callback_info info)
{
    auto ctxt = std::make_shared<PanelContentContext>(env, info);
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        auto &inputMethodPanel = reinterpret_cast<JsPanel *>(ctxt->native)->GetNative();
        CHECK_RETURN_VOID(inputMethodPanel != nullptr, "inputMethodPanel_ is nullptr.");
        auto code = inputMethodPanel->HidePanel();
        if (code == ErrorCode::NO_ERROR) {
            ctxt->SetState(napi_ok);
            return;
        }
        ctxt->SetErrorCode(code);
    };
    AsyncCall asyncCall(env, info, ctxt, 0);
    return asyncCall.Call(env, exec);
}

napi_value JsPanel::ChangeFlag(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_MAX;
    napi_value argv[ARGC_MAX] = { nullptr };
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    PARAM_CHECK_RETURN(env, argc > 0, " should 1 parameter! ", TYPE_NONE, nullptr);
    int32_t panelFlag = 0;
    napi_status status = JsUtils::GetValue(env, argv[ARGC_ZERO], panelFlag);
    NAPI_ASSERT(env, status == napi_ok, "get panelFlag failed!");
    auto inputMethodPanel = UnwrapPanel(env, thisVar);
    auto ret = inputMethodPanel->ChangePanelFlag(PanelFlag(panelFlag));
    NAPI_ASSERT(env, ret == ErrorCode::NO_ERROR, "ChangePanelFlag failed!");
    return nullptr;
}

napi_value JsPanel::Subscribe(napi_env env, napi_callback_info info)
{
    IMSA_HILOGD("JsPanel in");
    size_t argc = ARGC_MAX;
    napi_value argv[ARGC_MAX] = { nullptr };
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_ASSERT(env, (argc >= ARGC_ONE) && (argc <= ARGC_MAX), "err number of argument!");
    std::string type = "";
    JsUtils::GetValue(env, argv[ARGC_ZERO], type);
    IMSA_HILOGD("on event type is: %{public}s", type.c_str());

    napi_valuetype valuetype = napi_undefined;
    napi_typeof(env, argv[1], &valuetype);
    NAPI_ASSERT(env, valuetype == napi_function, "callback is not a function");
    std::shared_ptr<PanelListenerImpl> observer = PanelListenerImpl::GetInstance();
    auto inputMethodPanel = UnwrapPanel(env, thisVar);
    observer->SaveInfo(env, type, argv[ARGC_ONE], inputMethodPanel->windowId_);
    inputMethodPanel->SetPanelStatusListener(observer, type);
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

napi_value JsPanel::UnSubscribe(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_MAX;
    napi_value argv[ARGC_MAX] = { nullptr };
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments, requires 1 or 2");
    std::string type = "";
    JsUtils::GetValue(env, argv[ARGC_ZERO], type);
    IMSA_HILOGI("event type is: %{public}s", type.c_str());
    std::shared_ptr<PanelListenerImpl> observer = PanelListenerImpl::GetInstance();
    auto inputMethodPanel = UnwrapPanel(env, thisVar);
    observer->RemoveInfo(type, inputMethodPanel->windowId_);
    inputMethodPanel->RemovePanelListener(type);
    napi_value result = nullptr;
    napi_get_null(env, &result);
    return result;
}

std::shared_ptr<InputMethodPanel> JsPanel::UnwrapPanel(napi_env env, napi_value thisVar)
{
    void *native = nullptr;
    napi_status status = napi_unwrap(env, thisVar, &native);
    NAPI_ASSERT_BASE(env, (status == napi_ok && native != nullptr), "napi_unwrap failed!", nullptr);
    auto jsPanel = reinterpret_cast<JsPanel *>(native);
    if (jsPanel == nullptr) {
        return nullptr;
    }
    auto &inputMethodPanel = jsPanel->GetNative();
    NAPI_ASSERT_BASE(env, inputMethodPanel != nullptr, "inputMethodPanel is nullptr", nullptr);
    return inputMethodPanel;
}
} // namespace MiscServices
} // namespace OHOS