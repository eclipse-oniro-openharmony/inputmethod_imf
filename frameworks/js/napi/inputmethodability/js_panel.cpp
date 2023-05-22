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

#include "event_checker.h"
#include "input_method_ability.h"
#include "js_util.h"
#include "js_utils.h"
#include "napi/native_common.h"
#include "panel_listener_impl.h"

namespace OHOS {
namespace MiscServices {
const std::string JsPanel::CLASS_NAME = "Panel";
thread_local napi_ref JsPanel::panelConstructorRef_ = nullptr;
std::mutex JsPanel::panelConstructorMutex_;

napi_value JsPanel::Init(napi_env env)
{
    IMSA_HILOGI("JsPanel in.");
    napi_value constructor = nullptr;
    std::lock_guard<std::mutex> lock(panelConstructorMutex_);
    if (panelConstructorRef_ != nullptr) {
        napi_status status = napi_get_reference_value(env, panelConstructorRef_, &constructor);
        NAPI_ASSERT_BASE(env, status == napi_ok, "Failed to get jsPanel constructor.", nullptr);
        return constructor;
    }
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

std::shared_ptr<InputMethodPanel> JsPanel::GetNative()
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
        // 0 means the first param path<std::string>
        status = JsUtils::GetValue(env, argv[0], ctxt->path);
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

    auto exec = [ctxt](AsyncCall::Context *ctx) { ctxt->SetState(napi_ok); };
    auto output = [ctxt](napi_env env, napi_value *result) -> napi_status {
        NAPI_ASSERT_BASE(env, ctxt->inputMethodPanel != nullptr, "inputMethodPanel is nullptr!", napi_generic_failure);
        auto code = ctxt->inputMethodPanel->SetUiContent(
            ctxt->path, *(reinterpret_cast<NativeEngine *>(env)), ctxt->contentStorage);
        NAPI_ASSERT_BASE(env, code == ErrorCode::NO_ERROR, "SetUiContent failed!", napi_generic_failure);
        return napi_ok;
    };
    ctxt->SetAction(std::move(input), std::move(output));
    // 3 means JsAPI:setUiContent has 3 params at most.
    AsyncCall asyncCall(env, info, ctxt, 3);
    return asyncCall.Call(env, exec, "setUiContent");
}

napi_value JsPanel::Resize(napi_env env, napi_callback_info info)
{
    auto ctxt = std::make_shared<PanelContentContext>(env, info);
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        napi_status status = napi_generic_failure;
        PARAM_CHECK_RETURN(env, argc > 1, "should 2 or 3 parameters!", TYPE_NONE, status);
        // 0 means the first param width<uint32_t>
        status = JsUtils::GetValue(env, argv[0], ctxt->width);
        NAPI_ASSERT_BASE(env, status == napi_ok, "get width failed!", status);
        // 1 means the second param height<uint32_t>
        status = JsUtils::GetValue(env, argv[1], ctxt->height);
        NAPI_ASSERT_BASE(env, status == napi_ok, "get height failed!", status);
        return napi_ok;
    };

    auto exec = [ctxt](AsyncCall::Context *ctx) {
        CHECK_RETURN_VOID(ctxt->inputMethodPanel != nullptr, "inputMethodPanel_ is nullptr.");
        auto code = ctxt->inputMethodPanel->Resize(ctxt->width, ctxt->height);
        if (code == ErrorCode::NO_ERROR) {
            ctxt->SetState(napi_ok);
            return;
        }
        ctxt->SetErrorCode(code);
    };
    ctxt->SetAction(std::move(input));
    // 3 means JsAPI:resize has 3 params at most.
    AsyncCall asyncCall(env, info, ctxt, 3);
    return asyncCall.Call(env, exec, "resize");
}

napi_value JsPanel::MoveTo(napi_env env, napi_callback_info info)
{
    auto ctxt = std::make_shared<PanelContentContext>(env, info);
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        napi_status status = napi_generic_failure;
        PARAM_CHECK_RETURN(env, argc > 1, " should 2 or 3 parameters! ", TYPE_NONE, status);
        // 0 means the first param x<int32_t>
        status = JsUtils::GetValue(env, argv[0], ctxt->x);
        NAPI_ASSERT_BASE(env, status == napi_ok, "get x failed!", status);
        // 1 means the second param y<int32_t>
        status = JsUtils::GetValue(env, argv[1], ctxt->y);
        NAPI_ASSERT_BASE(env, status == napi_ok, "get y failed!", status);
        return napi_ok;
    };

    auto exec = [ctxt](AsyncCall::Context *ctx) {
        CHECK_RETURN_VOID(ctxt->inputMethodPanel != nullptr, "inputMethodPanel_ is nullptr.");
        auto code = ctxt->inputMethodPanel->MoveTo(ctxt->x, ctxt->y);
        if (code == ErrorCode::NO_ERROR) {
            ctxt->SetState(napi_ok);
            return;
        }
        ctxt->SetErrorCode(code);
    };
    ctxt->SetAction(std::move(input));
    // 3 means JsAPI:moveTo has 3 params at most.
    AsyncCall asyncCall(env, info, ctxt, 3);
    return asyncCall.Call(env, exec, "moveTo");
}

napi_value JsPanel::Show(napi_env env, napi_callback_info info)
{
    auto ctxt = std::make_shared<PanelContentContext>(env, info);
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        CHECK_RETURN_VOID(ctxt->inputMethodPanel != nullptr, "inputMethodPanel_ is nullptr.");
        auto code = ctxt->inputMethodPanel->ShowPanel();
        if (code == ErrorCode::NO_ERROR) {
            ctxt->SetState(napi_ok);
            return;
        }
        ctxt->SetErrorCode(code);
    };
    // 1 means JsAPI:show has 1 params at most.
    AsyncCall asyncCall(env, info, ctxt, 1);
    return asyncCall.Call(env, exec, "show");
}

napi_value JsPanel::Hide(napi_env env, napi_callback_info info)
{
    auto ctxt = std::make_shared<PanelContentContext>(env, info);
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        CHECK_RETURN_VOID(ctxt->inputMethodPanel != nullptr, "inputMethodPanel_ is nullptr.");
        auto code = ctxt->inputMethodPanel->HidePanel();
        if (code == ErrorCode::NO_ERROR) {
            ctxt->SetState(napi_ok);
            return;
        }
        ctxt->SetErrorCode(code);
    };
    // 1 means JsAPI:hide has 1 params at most.
    AsyncCall asyncCall(env, info, ctxt, 1);
    return asyncCall.Call(env, exec, "panel.hide");
}

napi_value JsPanel::ChangeFlag(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_MAX;
    napi_value argv[ARGC_MAX] = { nullptr };
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    PARAM_CHECK_RETURN(env, argc > 0, " should 1 parameter! ", TYPE_NONE, nullptr);
    int32_t panelFlag = 0;
    // 0 means the first param flag<PanelFlag>
    napi_status status = JsUtils::GetValue(env, argv[0], panelFlag);
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
    std::string type;
    // 2 means least param num.
    if (argc < 2 || !JsUtil::GetValue(env, argv[0], type)
        || !EventChecker::IsValidEventType(EventSubscribeModule::PANEL, type)
        || JsUtil::GetType(env, argv[1]) != napi_function) {
        IMSA_HILOGE("Subscribe failed, type:%{public}s", type.c_str());
        return nullptr;
    }
    IMSA_HILOGD("Subscribe type:%{public}s", type.c_str());
    std::shared_ptr<PanelListenerImpl> observer = PanelListenerImpl::GetInstance();
    auto inputMethodPanel = UnwrapPanel(env, thisVar);
    // 1 means the second param callback.
    observer->SaveInfo(env, type, argv[1], inputMethodPanel->windowId_);
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
    std::string type;
    // 1 means least param num.
    if (argc < 1 || !JsUtil::GetValue(env, argv[0], type)
        || !EventChecker::IsValidEventType(EventSubscribeModule::PANEL, type)) {
        IMSA_HILOGE("UnSubscribe failed, type:%{public}s", type.c_str());
        JsUtils::ThrowException(env, IMFErrorCode::EXCEPTION_PARAMCHECK, "please check the params", TYPE_NONE);
        return nullptr;
    }
    // If the type of optional parameter is wrong, make it nullptr
    if (JsUtil::GetType(env, argv[1]) != napi_function) {
        argv[1] = nullptr;
    }
    IMSA_HILOGD("UnSubscribe type:%{public}s", type.c_str());
    std::shared_ptr<PanelListenerImpl> observer = PanelListenerImpl::GetInstance();
    auto inputMethodPanel = UnwrapPanel(env, thisVar);
    observer->RemoveInfo(type, inputMethodPanel->windowId_);
    inputMethodPanel->ClearPanelListener(type);
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
    auto inputMethodPanel = jsPanel->GetNative();
    NAPI_ASSERT_BASE(env, inputMethodPanel != nullptr, "inputMethodPanel is nullptr", nullptr);
    return inputMethodPanel;
}
} // namespace MiscServices
} // namespace OHOS