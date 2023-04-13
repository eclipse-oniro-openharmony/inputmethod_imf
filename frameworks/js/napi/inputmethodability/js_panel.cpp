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
#include "js_runtime_utils.h"
#include "js_utils.h"
#include "napi/native_common.h"
#include "panel_listener_impl.h"

namespace OHOS {
namespace MiscServices {
constexpr size_t ARGC_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
const std::string JsPanel::CLASS_NAME = "Panel";

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
        delete jsPanel;
    };
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr));
    napi_status status = napi_wrap(env, thisVar, panel, finalize, nullptr, nullptr);
    if (status != napi_ok) {
        IMSA_HILOGE("JsPanel napi_wrap failed: %{public}d", status);
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

// setUiContent(path: string): Promise<void>
// setUiContent(path: string, callback: AsyncCallback<void>): void
// setUiContent(path: string, storage: LocalStorage, callback: AsyncCallback<void>): void
// setUiContent(path: string, storage: LocalStorage): Promise<void>
napi_value JsPanel::SetUiContent(napi_env env, napi_callback_info info)
{
    IMSA_HILOGI("JsPanel in.");
    size_t argc = ARGC_MAX;
    napi_value argv[ARGC_MAX] = { nullptr };
    napi_value self = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &self, nullptr));
    PARAM_CHECK_RETURN(env, argc > 0, " should 1 or 2 parameter! ", TYPE_NONE, nullptr);
    std::string contextUrl;
    napi_status status = JsUtils::GetValue(env, argv[0], contextUrl);
    PARAM_CHECK_RETURN(env, status == napi_ok, " get contextUrl error! ", TYPE_NONE, nullptr);
    NativeValue *callBack = nullptr;
    std::shared_ptr<NativeReference> contentStorage = nullptr;
    NativeCallbackInfo *callbackInfo = reinterpret_cast<NativeCallbackInfo *>(info);
    if (callbackInfo->argc == 2) {
        if (callbackInfo->argv[1]->TypeOf() == NATIVE_OBJECT) {
            NativeValue *storage = callbackInfo->argv[1];
            contentStorage = (storage == nullptr)
                                 ? nullptr
                                 : std::shared_ptr<NativeReference>(
                                       reinterpret_cast<NativeEngine *>(env)->CreateReference(storage, 1));
        } else if (callbackInfo->argv[1]->TypeOf() == NATIVE_FUNCTION) {
            callBack = callbackInfo->argv[1];
        }
    } else if (callbackInfo->argc > 2) {
        callBack = callbackInfo->argv[2];
    }
    void *native = nullptr;
    status = napi_unwrap(env, self, &native);
    if (status != napi_ok) {
        IMSA_HILOGI("napi_unwrap error, status = %{public}d.", status);
        return status;
    }
    auto &inputMethodPanel = reinterpret_cast<JsPanel *>(native)->GetNative();
    AbilityRuntime::AsyncTask::CompleteCallback complete = [contentStorage, contextUrl, &inputMethodPanel](
                                                               NativeEngine &engine, AbilityRuntime::AsyncTask &task,
                                                               int32_t status) {
        CHECK_RETURN_VOID(inputMethodPanel != nullptr, "inputMethodPanel_ is nullptr.");
        NativeValue *nativeStorage = (contentStorage == nullptr) ? nullptr : contentStorage->Get();
        auto code = inputMethodPanel->SetUiContent(contextUrl, engine, *nativeStorage);
        if (code != ErrorCode::NO_ERROR) {
            IMSA_HILOGE("SetUiContent err: %{public}d", code);
            task.Reject(engine, engine.CreateUndefined());
            return;
        }
        task.ResolveWithNoError(engine, engine.CreateUndefined());
    };
    NativeValue *result = nullptr;
    NativeEngine *nativeEngine = reinterpret_cast<NativeEngine *>(env);
    AbilityRuntime::AsyncTask::Schedule("JsPanel::SetUiContent", *nativeEngine,
        CreateAsyncTaskWithLastParam(*nativeEngine, callBack, nullptr, std::move(complete), &result));
    return reinterpret_cast<napi_value>(result);
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
        status = JsUtils::GetValue(env, argv[ARGC_ZERO], ctxt->width);
        NAPI_ASSERT_BASE(env, status == napi_ok, "get width failed!", status);
        status = JsUtils::GetValue(env, argv[ARGC_ONE], ctxt->height);
        NAPI_ASSERT_BASE(env, status == napi_ok, "get height failed!", status);
        return napi_ok;
    };

    auto exec = [ctxt](AsyncCall::Context *ctx) {
        auto &inputMethodPanel = reinterpret_cast<JsPanel *>(ctxt->native)->GetNative();
        CHECK_RETURN_VOID(inputMethodPanel != nullptr, "inputMethodPanel_ is nullptr.");
        auto code = inputMethodPanel->MoveTo(ctxt->width, ctxt->height);
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
    inputMethodPanel->SetPanelStatusListener(observer);
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