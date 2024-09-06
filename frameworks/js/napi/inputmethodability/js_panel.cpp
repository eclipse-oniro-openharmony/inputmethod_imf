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
#include "inputmethod_trace.h"
#include "js_text_input_client_engine.h"
#include "js_util.h"
#include "js_utils.h"
#include "napi/native_common.h"
#include "panel_listener_impl.h"

namespace OHOS {
namespace MiscServices {
using namespace std::chrono;
using WMError = OHOS::Rosen::WMError;
const std::string JsPanel::CLASS_NAME = "Panel";
thread_local napi_ref JsPanel::panelConstructorRef_ = nullptr;
std::mutex JsPanel::panelConstructorMutex_;
constexpr int32_t MAX_WAIT_TIME = 10;
FFRTBlockQueue<JsEventInfo> JsPanel::jsQueue_{ MAX_WAIT_TIME };

napi_value JsPanel::Init(napi_env env)
{
    IMSA_HILOGI("JsPanel in.");
    napi_value constructor = nullptr;
    std::lock_guard<std::mutex> lock(panelConstructorMutex_);
    if (panelConstructorRef_ != nullptr) {
        napi_status status = napi_get_reference_value(env, panelConstructorRef_, &constructor);
        CHECK_RETURN(status == napi_ok, "Failed to get jsPanel constructor.", nullptr);
        return constructor;
    }
    const napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("setUiContent", SetUiContent),
        DECLARE_NAPI_FUNCTION("resize", Resize),
        DECLARE_NAPI_FUNCTION("moveTo", MoveTo),
        DECLARE_NAPI_FUNCTION("show", Show),
        DECLARE_NAPI_FUNCTION("hide", Hide),
        DECLARE_NAPI_FUNCTION("changeFlag", ChangeFlag),
        DECLARE_NAPI_FUNCTION("setPrivacyMode", SetPrivacyMode),
        DECLARE_NAPI_FUNCTION("on", Subscribe),
        DECLARE_NAPI_FUNCTION("off", UnSubscribe),
        DECLARE_NAPI_FUNCTION("adjustPanelRect", AdjustPanelRect),
    };
    NAPI_CALL(env, napi_define_class(env, CLASS_NAME.c_str(), CLASS_NAME.size(), JsNew, nullptr,
                       sizeof(properties) / sizeof(napi_property_descriptor), properties, &constructor));
    CHECK_RETURN(constructor != nullptr, "napi_define_class failed!", nullptr);
    NAPI_CALL(env, napi_create_reference(env, constructor, 1, &panelConstructorRef_));
    return constructor;
}

napi_value JsPanel::JsNew(napi_env env, napi_callback_info info)
{
    IMSA_HILOGD("JsPanel, create panel instance in.");
    std::shared_ptr<PanelListenerImpl> panelImpl = PanelListenerImpl::GetInstance();
    if (panelImpl != nullptr) {
        IMSA_HILOGD("Set eventhandler.");
        panelImpl->SetEventHandler(AppExecFwk::EventHandler::Current());
    }
    JsPanel *panel = new (std::nothrow) JsPanel();
    CHECK_RETURN(panel != nullptr, "no memory for JsPanel", nullptr);
    auto finalize = [](napi_env env, void *data, void *hint) {
        IMSA_HILOGD("jsPanel finalize.");
        auto *jsPanel = reinterpret_cast<JsPanel *>(data);
        CHECK_RETURN_VOID(jsPanel != nullptr, "finalize null!");
        jsPanel->GetNative() = nullptr;
        delete jsPanel;
    };
    napi_value thisVar = nullptr;
    napi_status status = napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
    if (status != napi_ok) {
        IMSA_HILOGE("JsPanel napi_get_cb_info failed: %{public}d", status);
        delete panel;
        return nullptr;
    }
    status = napi_wrap(env, thisVar, panel, finalize, nullptr, nullptr);
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
        PARAM_CHECK_RETURN(env, argc >= 1, "at least one paramster is required", TYPE_NONE, status);
        // 0 means the first param path<std::string>
        PARAM_CHECK_RETURN(env, JsUtils::GetValue(env, argv[0], ctxt->path) == napi_ok,
            "js param path covert failed, must be string", TYPE_NONE, status);
        // if type of argv[1] is object, we will get value of 'storage' from it.
        if (argc >= 2) {
            napi_valuetype valueType = napi_undefined;
            status = napi_typeof(env, argv[1], &valueType);
            CHECK_RETURN(status == napi_ok, "get valueType failed!", status);
            if (valueType == napi_object) {
                napi_ref storage = nullptr;
                napi_create_reference(env, argv[1], 1, &storage);
                auto contentStorage = (storage == nullptr) ? nullptr
                                                           : std::shared_ptr<NativeReference>(
                                                                 reinterpret_cast<NativeReference *>(storage));
                ctxt->contentStorage = contentStorage;
            }
        }
        return napi_ok;
    };

    auto exec = [ctxt](AsyncCall::Context *ctx) {
        ctxt->SetState(napi_ok);
    };
    auto output = [ctxt](napi_env env, napi_value *result) -> napi_status {
        CHECK_RETURN(ctxt->inputMethodPanel != nullptr, "inputMethodPanel is nullptr!", napi_generic_failure);
        auto code = ctxt->inputMethodPanel->SetUiContent(ctxt->path, env, ctxt->contentStorage);
        if (code == ErrorCode::ERROR_PARAMETER_CHECK_FAILED) {
            ctxt->SetErrorCode(code);
            ctxt->SetErrorMessage("path should be a path to specific page.");
            return napi_generic_failure;
        }
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
        PARAM_CHECK_RETURN(env, argc > 1, "at least two paramsters is required", TYPE_NONE, status);
        // 0 means the first param width<uint32_t>
        PARAM_CHECK_RETURN(env, JsUtils::GetValue(env, argv[0], ctxt->width) == napi_ok,
            "param width type must be number", TYPE_NONE, status);
        // 1 means the second param height<uint32_t>
        PARAM_CHECK_RETURN(env, JsUtils::GetValue(env, argv[1], ctxt->height) == napi_ok,
            "param height type must be number", TYPE_NONE, status);
        ctxt->info = { std::chrono::system_clock::now(), JsEvent::RESIZE };
        jsQueue_.Push(ctxt->info);
        return napi_ok;
    };

    auto exec = [ctxt](AsyncCall::Context *ctx) {
        jsQueue_.Wait(ctxt->info);
        if (ctxt->inputMethodPanel == nullptr) {
            IMSA_HILOGE("inputMethodPanel_ is nullptr.");
            jsQueue_.Pop();
            return;
        }
        SysPanelStatus sysPanelStatus = { false, ctxt->inputMethodPanel->GetPanelFlag(), ctxt->width, ctxt->height };
        InputMethodAbility::GetInstance()->NotifyPanelStatus(ctxt->inputMethodPanel, sysPanelStatus);
        auto code = ctxt->inputMethodPanel->Resize(ctxt->width, ctxt->height);
        jsQueue_.Pop();
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
        PARAM_CHECK_RETURN(env, argc > 1, "at least two paramsters is required ", TYPE_NONE, status);
        // 0 means the first param x<int32_t>
        PARAM_CHECK_RETURN(env, JsUtils::GetValue(env, argv[0], ctxt->x) == napi_ok, "param x type must be number",
            TYPE_NONE, status);
        // 1 means the second param y<int32_t>
        PARAM_CHECK_RETURN(env, JsUtils::GetValue(env, argv[1], ctxt->y) == napi_ok, "param y type must be number",
            TYPE_NONE, status);
        ctxt->info = { std::chrono::system_clock::now(), JsEvent::MOVE_TO };
        jsQueue_.Push(ctxt->info);
        return napi_ok;
    };

    auto exec = [ctxt](AsyncCall::Context *ctx) {
        int64_t start = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
        jsQueue_.Wait(ctxt->info);
        PrintEditorQueueInfoIfTimeout(start, ctxt->info);
        if (ctxt->inputMethodPanel == nullptr) {
            IMSA_HILOGE("inputMethodPanel_ is nullptr.");
            jsQueue_.Pop();
            return;
        }
        auto code = ctxt->inputMethodPanel->MoveTo(ctxt->x, ctxt->y);
        jsQueue_.Pop();
        if (code == ErrorCode::ERROR_PARAMETER_CHECK_FAILED) {
            ctxt->SetErrorCode(code);
            return;
        }
        ctxt->SetState(napi_ok);
    };
    ctxt->SetAction(std::move(input));
    // 3 means JsAPI:moveTo has 3 params at most.
    AsyncCall asyncCall(env, info, ctxt, 3);
    return asyncCall.Call(env, exec, "moveTo");
}

void JsPanel::PrintEditorQueueInfoIfTimeout(int64_t start, const JsEventInfo &currentInfo)
{
    int64_t end = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    if (end - start >= MAX_WAIT_TIME) {
        JsEventInfo frontInfo;
        auto ret = jsQueue_.GetFront(frontInfo);
        int64_t frontTime = duration_cast<microseconds>(frontInfo.timestamp.time_since_epoch()).count();
        int64_t currentTime = duration_cast<microseconds>(currentInfo.timestamp.time_since_epoch()).count();
        IMSA_HILOGI("ret:%{public}d,front[%{public}" PRId64 ",%{public}d],current[%{public}" PRId64 ",%{public}d]", ret,
            frontTime, static_cast<int32_t>(frontInfo.event), currentTime, static_cast<int32_t>(currentInfo.event));
    }
}

napi_value JsPanel::Show(napi_env env, napi_callback_info info)
{
    InputMethodSyncTrace tracer("JsPanel_Show");
    auto ctxt = std::make_shared<PanelContentContext>(env, info);
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        CHECK_RETURN_VOID(ctxt->inputMethodPanel != nullptr, "inputMethodPanel_ is nullptr.");
        auto code = InputMethodAbility::GetInstance()->ShowPanel(ctxt->inputMethodPanel);
        if (code == ErrorCode::NO_ERROR) {
            ctxt->SetState(napi_ok);
            return;
        }
        ctxt->SetErrorCode(code);
    };
    // 1 means JsAPI:show has 1 param at most.
    AsyncCall asyncCall(env, info, ctxt, 1);
    return asyncCall.Call(env, exec, "show");
}

napi_value JsPanel::Hide(napi_env env, napi_callback_info info)
{
    InputMethodSyncTrace tracer("JsPanel_Hide");
    auto ctxt = std::make_shared<PanelContentContext>(env, info);
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        CHECK_RETURN_VOID(ctxt->inputMethodPanel != nullptr, "inputMethodPanel_ is nullptr.");
        auto code = InputMethodAbility::GetInstance()->HidePanel(ctxt->inputMethodPanel);
        if (code == ErrorCode::NO_ERROR) {
            ctxt->SetState(napi_ok);
            return;
        }
        ctxt->SetErrorCode(code);
    };
    // 1 means JsAPI:hide has 1 param at most.
    AsyncCall asyncCall(env, info, ctxt, 1);
    return asyncCall.Call(env, exec, "panel.hide");
}

napi_value JsPanel::ChangeFlag(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_MAX;
    napi_value argv[ARGC_MAX] = { nullptr };
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    PARAM_CHECK_RETURN(env, argc > 0, "at least one paramster is required", TYPE_NONE, nullptr);
    int32_t panelFlag = 0;
    // 0 means the first param flag<PanelFlag>
    napi_status status = JsUtils::GetValue(env, argv[0], panelFlag);
    PARAM_CHECK_RETURN(env, status == napi_ok, "param flag type must be PanelFlag", TYPE_NONE, nullptr);
    auto inputMethodPanel = UnwrapPanel(env, thisVar);
    if (inputMethodPanel == nullptr) {
        IMSA_HILOGE("inputMethodPanel is nullptr");
        return nullptr;
    }
    PARAM_CHECK_RETURN(env,
        (panelFlag == PanelFlag::FLG_FIXED || panelFlag == PanelFlag::FLG_FLOATING ||
            panelFlag == PanelFlag::FLG_CANDIDATE_COLUMN),
        "param flag type must be one of PanelFlag", TYPE_NONE, nullptr);
    JsEventInfo eventInfo = { std::chrono::system_clock::now(), JsEvent::CHANGE_FLAG };
    jsQueue_.Push(eventInfo);
    jsQueue_.Wait(eventInfo);
    auto ret = inputMethodPanel->ChangePanelFlag(PanelFlag(panelFlag));
    jsQueue_.Pop();
    CHECK_RETURN(ret == ErrorCode::NO_ERROR, "ChangePanelFlag failed!", nullptr);
    return nullptr;
}

napi_value JsPanel::SetPrivacyMode(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_MAX;
    napi_value argv[ARGC_MAX] = { nullptr };
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    PARAM_CHECK_RETURN(env, argc > 0, "at least one paramster is required", TYPE_NONE, nullptr);
    bool isPrivacyMode = false;
    // 0 means the first param isPrivacyMode<boolean>
    napi_status status = JsUtils::GetValue(env, argv[0], isPrivacyMode);
    PARAM_CHECK_RETURN(env, status == napi_ok, "param isPrivacyMode type must be boolean", TYPE_NONE, nullptr);
    CHECK_RETURN(status == napi_ok, "get isPrivacyMode failed!", nullptr);
    auto inputMethodPanel = UnwrapPanel(env, thisVar);
    if (inputMethodPanel == nullptr) {
        IMSA_HILOGE("inputMethodPanel is nullptr");
        return nullptr;
    }
    auto ret = inputMethodPanel->SetPrivacyMode(isPrivacyMode);
    if (ret == static_cast<int32_t>(WMError::WM_ERROR_INVALID_PERMISSION)) {
        JsUtils::ThrowException(env, JsUtils::Convert(ErrorCode::ERROR_STATUS_PERMISSION_DENIED),
            " ohos.permission.PRIVACY_WINDOW permission denied", TYPE_NONE);
    }
    CHECK_RETURN(ret == ErrorCode::NO_ERROR, "SetPrivacyMode failed!", nullptr);
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
    if (argc < 2 || !JsUtil::GetValue(env, argv[0], type) ||
        !EventChecker::IsValidEventType(EventSubscribeModule::PANEL, type) ||
        JsUtil::GetType(env, argv[1]) != napi_function) {
        IMSA_HILOGE("Subscribe failed, type:%{public}s", type.c_str());
        return nullptr;
    }
    IMSA_HILOGD("Subscribe type:%{public}s", type.c_str());
    std::shared_ptr<PanelListenerImpl> observer = PanelListenerImpl::GetInstance();
    auto inputMethodPanel = UnwrapPanel(env, thisVar);
    // 1 means the second param callback.
    observer->SaveInfo(env, type, argv[1], inputMethodPanel->windowId_);
    bool ret = inputMethodPanel->SetPanelStatusListener(observer, type);
    if (!ret) {
        IMSA_HILOGE("failed to subscribe %{public}s", type.c_str());
        observer->RemoveInfo(type, inputMethodPanel->windowId_);
    }
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
    PARAM_CHECK_RETURN(env, argc >= 1, "at least one paramster is required", TYPE_NONE, nullptr);
    PARAM_CHECK_RETURN(
        env, JsUtil::GetValue(env, argv[0], type), "param type must be string", TYPE_NONE, nullptr);
    PARAM_CHECK_RETURN(env, EventChecker::IsValidEventType(EventSubscribeModule::PANEL, type),
        "type shoule be show/hide/sizeChange", TYPE_NONE, nullptr);
    // if the second param is not napi_function/napi_null/napi_undefined, return
    auto paramType = JsUtil::GetType(env, argv[1]);
    PARAM_CHECK_RETURN(env, (paramType == napi_function || paramType == napi_null || paramType == napi_undefined),
        "callback should be function or null or undefined", TYPE_NONE, nullptr);
    // if the second param is napi_function, delete it, else delete all
    argv[1] = paramType == napi_function ? argv[1] : nullptr;

    IMSA_HILOGD("UnSubscribe type:%{public}s", type.c_str());
    std::shared_ptr<PanelListenerImpl> observer = PanelListenerImpl::GetInstance();
    auto inputMethodPanel = UnwrapPanel(env, thisVar);
    if (inputMethodPanel == nullptr) {
        IMSA_HILOGE("inputMethodPanel is nullptr");
        return nullptr;
    }
    observer->RemoveInfo(type, inputMethodPanel->windowId_);
    inputMethodPanel->ClearPanelListener(type);
    napi_value result = nullptr;
    napi_get_null(env, &result);
    return result;
}

napi_status JsPanel::CheckParam(napi_env env, size_t argc, napi_value *argv,
    std::shared_ptr<PanelContentContext> ctxt)
{
    napi_status status = napi_generic_failure;
    PARAM_CHECK_RETURN(env, argc > 1, "at least two paramsters is required", TYPE_NONE, status);
    // 0 means the first param flag
    PARAM_CHECK_RETURN(env, JsUtil::GetType(env, argv[0]) == napi_number, "flag",
        TYPE_NUMBER, napi_generic_failure);
    int32_t panelFlag = 0;
    CHECK_RETURN(JsUtils::GetValue(env, argv[0], panelFlag) == napi_ok,
        "js param flag covert failed", napi_generic_failure);
    ctxt->panelFlag = PanelFlag(panelFlag);
    if (InputMethodAbility::GetInstance()->IsDefaultIme()) {
        PARAM_CHECK_RETURN(env, ctxt->panelFlag == 0 || ctxt->panelFlag == 1 ||
            ctxt->panelFlag == FLG_CANDIDATE_COLUMN,
            "param flag type shoule be FLG_FIXED or FLG_FLOATING or FLG_CANDIDATE_COLUMN", TYPE_NONE,
            napi_generic_failure);
    } else {
        PARAM_CHECK_RETURN(env, ctxt->panelFlag == 0 || ctxt->panelFlag == 1,
            "param flag type shoule be FLG_FIXED or FLG_FLOATING ", TYPE_NONE, napi_generic_failure);
    }
    // 1 means the second param rect
    PARAM_CHECK_RETURN(env, JsUtil::GetType(env, argv[1]) == napi_object, "param rect type must be PanelRect",
        TYPE_NONE, napi_generic_failure);
    PARAM_CHECK_RETURN(env, JsPanelRect::Read(env, argv[1], ctxt->layoutParams), "js param rect covert failed",
        TYPE_NONE, napi_generic_failure);
    return napi_ok;
}

napi_value JsPanel::AdjustPanelRect(napi_env env, napi_callback_info info)
{
    auto ctxt = std::make_shared<PanelContentContext>(env, info);
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        if (CheckParam(env, argc, argv, ctxt) != napi_ok) {
            return napi_generic_failure;
        }
        ctxt->info = { std::chrono::system_clock::now(), JsEvent::ADJUST_PANEL_RECT };
        jsQueue_.Push(ctxt->info);
        return napi_ok;
    };

    auto exec = [ctxt](AsyncCall::Context *ctx) {
        jsQueue_.Wait(ctxt->info);
        if (ctxt->inputMethodPanel == nullptr) {
            IMSA_HILOGE("inputMethodPanel_ is nullptr.");
            jsQueue_.Pop();
            return;
        }
        SysPanelStatus sysPanelStatus;
        if (ctxt->inputMethodPanel->IsDisplayPortrait()) {
            sysPanelStatus = { false, ctxt->panelFlag, ctxt->layoutParams.portraitRect.width_,
                ctxt->layoutParams.portraitRect.height_ };
        } else {
            sysPanelStatus = { false, ctxt->panelFlag, ctxt->layoutParams.landscapeRect.width_,
                ctxt->layoutParams.landscapeRect.height_ };
        }
        InputMethodAbility::GetInstance()->NotifyPanelStatus(ctxt->inputMethodPanel, sysPanelStatus);
        auto code = ctxt->inputMethodPanel->AdjustPanelRect(ctxt->panelFlag, ctxt->layoutParams);
        jsQueue_.Pop();
        if (code == ErrorCode::NO_ERROR) {
            ctxt->SetState(napi_ok);
            return;
        } else if (code == ErrorCode::ERROR_PARAMETER_CHECK_FAILED) {
            ctxt->SetErrorMessage("width limit:[0, displayWidth], height limit:[0, 70 percent of displayHeight]");
        }
        ctxt->SetErrorCode(code);
    };
    ctxt->SetAction(std::move(input));
    // 2 means JsAPI:adjustPanelRect has 2 params at most
    AsyncCall asyncCall(env, info, ctxt, 2);
    return asyncCall.Call(env, exec, "adjustPanelRect");
}

std::shared_ptr<InputMethodPanel> JsPanel::UnwrapPanel(napi_env env, napi_value thisVar)
{
    void *native = nullptr;
    napi_status status = napi_unwrap(env, thisVar, &native);
    CHECK_RETURN((status == napi_ok && native != nullptr), "napi_unwrap failed!", nullptr);
    auto jsPanel = reinterpret_cast<JsPanel *>(native);
    if (jsPanel == nullptr) {
        return nullptr;
    }
    auto inputMethodPanel = jsPanel->GetNative();
    CHECK_RETURN(inputMethodPanel != nullptr, "inputMethodPanel is nullptr", nullptr);
    return inputMethodPanel;
}

napi_value JsPanelRect::Write(napi_env env, const LayoutParams &layoutParams)
{
    napi_value jsObject = nullptr;
    napi_create_object(env, &jsObject);
    bool ret =
        JsUtil::Object::WriteProperty(env, jsObject, "landscapeRect", JsRect::Write(env, layoutParams.landscapeRect));
    ret = ret &&
          JsUtil::Object::WriteProperty(env, jsObject, "portraitRect", JsRect::Write(env, layoutParams.portraitRect));
    return ret ? jsObject : JsUtil::Const::Null(env);
}
bool JsPanelRect::Read(napi_env env, napi_value object, LayoutParams &layoutParams)
{
    napi_value rectObject = nullptr;
    napi_get_named_property(env, object, "landscapeRect", &rectObject);
    bool ret = JsRect::Read(env, rectObject, layoutParams.landscapeRect);
    napi_get_named_property(env, object, "portraitRect", &rectObject);
    ret = ret && JsRect::Read(env, rectObject, layoutParams.portraitRect);
    return ret;
}
} // namespace MiscServices
} // namespace OHOS