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
#include "js_get_input_method_controller.h"

#include <set>

#include "input_method_controller.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "string_ex.h"

namespace OHOS {
namespace MiscServices {
constexpr size_t ARGC_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
const std::set<std::string> EVENT_TYPE{
    "selectByRange",
    "selectByMovement",
};
thread_local napi_ref JsGetInputMethodController::IMCRef_ = nullptr;
const std::string JsGetInputMethodController::IMC_CLASS_NAME = "InputMethodController";
std::mutex JsGetInputMethodController::controllerMutex_;
std::shared_ptr<JsGetInputMethodController> JsGetInputMethodController::controller_{ nullptr };
napi_value JsGetInputMethodController::Init(napi_env env, napi_value info)
{
    napi_property_descriptor descriptor[] = {
        DECLARE_NAPI_FUNCTION("getInputMethodController", GetInputMethodController),
        DECLARE_NAPI_FUNCTION("getController", GetController),
    };
    NAPI_CALL(
        env, napi_define_properties(env, info, sizeof(descriptor) / sizeof(napi_property_descriptor), descriptor));

    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("stopInput", StopInput),
        DECLARE_NAPI_FUNCTION("stopInputSession", StopInputSession),
        DECLARE_NAPI_FUNCTION("hideSoftKeyboard", HideSoftKeyboard),
        DECLARE_NAPI_FUNCTION("showSoftKeyboard", ShowSoftKeyboard),
        DECLARE_NAPI_FUNCTION("on", Subscribe),
        DECLARE_NAPI_FUNCTION("off", UnSubscribe),
    };
    napi_value cons = nullptr;
    NAPI_CALL(env, napi_define_class(env, IMC_CLASS_NAME.c_str(), IMC_CLASS_NAME.size(), JsConstructor, nullptr,
                       sizeof(properties) / sizeof(napi_property_descriptor), properties, &cons));
    NAPI_CALL(env, napi_create_reference(env, cons, 1, &IMCRef_));
    NAPI_CALL(env, napi_set_named_property(env, info, IMC_CLASS_NAME.c_str(), cons));

    return info;
}

napi_value JsGetInputMethodController::JsConstructor(napi_env env, napi_callback_info cbinfo)
{
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, cbinfo, nullptr, nullptr, &thisVar, nullptr));

    auto controllerObject = GetInstance();
    if (controllerObject == nullptr) {
        IMSA_HILOGE("controllerObject is nullptr");
        napi_value result = nullptr;
        napi_get_null(env, &result);
        return result;
    }
    napi_status status = napi_wrap(
        env, thisVar, controllerObject.get(), [](napi_env env, void *data, void *hint) {}, nullptr, nullptr);
    if (status != napi_ok) {
        IMSA_HILOGE("JsGetInputMethodController napi_wrap failed:%{public}d", status);
        return nullptr;
    }

    if (controllerObject->loop_ == nullptr) {
        napi_get_uv_event_loop(env, &controllerObject->loop_);
    }

    return thisVar;
}

napi_value JsGetInputMethodController::GetInputMethodController(napi_env env, napi_callback_info cbInfo)
{
    return GetIMController(env, cbInfo, false);
}

napi_value JsGetInputMethodController::GetController(napi_env env, napi_callback_info cbInfo)
{
    return GetIMController(env, cbInfo, true);
}

napi_value JsGetInputMethodController::GetIMController(napi_env env, napi_callback_info cbInfo, bool needThrowException)
{
    napi_value instance = nullptr;
    napi_value cons = nullptr;
    if (napi_get_reference_value(env, IMCRef_, &cons) != napi_ok) {
        IMSA_HILOGE("GetInputMethodSetting::napi_get_reference_value not ok");
        if (needThrowException) {
            JsUtils::ThrowException(env, IMFErrorCode::EXCEPTION_CONTROLLER, "", TYPE_OBJECT);
        }
        return nullptr;
    }

    if (napi_new_instance(env, cons, 0, nullptr, &instance) != napi_ok) {
        IMSA_HILOGE("GetInputMethodSetting::napi_new_instance not ok");
        if (needThrowException) {
            JsUtils::ThrowException(env, IMFErrorCode::EXCEPTION_CONTROLLER, "", TYPE_OBJECT);
        }
        return nullptr;
    }
    return instance;
}

std::shared_ptr<JsGetInputMethodController> JsGetInputMethodController::GetInstance()
{
    if (controller_ == nullptr) {
        std::lock_guard<std::mutex> lock(controllerMutex_);
        if (controller_ == nullptr) {
            auto controller = std::make_shared<JsGetInputMethodController>();
            controller_ = controller;
            InputMethodController::GetInstance()->SetControllerListener(controller_);
        }
    }
    return controller_;
}

void JsGetInputMethodController::RegisterListener(
    napi_value callback, std::string type, std::shared_ptr<JSCallbackObject> callbackObj)
{
    IMSA_HILOGI("run in, type: %{public}s", type.c_str());
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        IMSA_HILOGE("methodName: %{public}s not registered!", type.c_str());
    }

    auto callbacks = jsCbMap_[type];
    bool ret = std::any_of(callbacks.begin(), callbacks.end(), [&callback](std::shared_ptr<JSCallbackObject> cb) {
        return JsUtils::Equals(cb->env_, callback, cb->callback_, cb->threadId_);
    });
    if (ret) {
        IMSA_HILOGE("JsGetInputMethodController callback already registered!");
        return;
    }

    IMSA_HILOGI("Add %{public}s callbackObj into jsCbMap_", type.c_str());
    jsCbMap_[type].push_back(std::move(callbackObj));
}

void JsGetInputMethodController::UnRegisterListener(std::string type)
{
    IMSA_HILOGI("UnRegisterListener %{public}s", type.c_str());
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        IMSA_HILOGE("methodName: %{public}s already unRegistered!", type.c_str());
        return;
    }
    jsCbMap_.erase(type);
}

napi_value JsGetInputMethodController::Subscribe(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_TWO;
    napi_value argv[ARGC_TWO] = { nullptr };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    PARAM_CHECK_RETURN(env, argc > 1, "should 2 parameters!", TYPE_NONE, nullptr);

    std::string type = "";
    napi_status status = JsUtils::GetValue(env, argv[ARGC_ZERO], type);
    PARAM_CHECK_RETURN(env, status == napi_ok, "callback", TYPE_FUNCTION, nullptr);

    auto engine = reinterpret_cast<JsGetInputMethodController *>(JsUtils::GetNativeSelf(env, info));
    if (engine == nullptr) {
        return nullptr;
    }
    std::shared_ptr<JSCallbackObject> callback =
        std::make_shared<JSCallbackObject>(env, argv[ARGC_ONE], std::this_thread::get_id());
    engine->RegisterListener(argv[ARGC_ONE], type, callback);

    napi_value result = nullptr;
    napi_get_null(env, &result);
    return result;
}

napi_value JsGetInputMethodController::UnSubscribe(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_TWO;
    napi_value argv[ARGC_TWO] = { nullptr };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    PARAM_CHECK_RETURN(env, argc > 0, "should 1 parameters!", TYPE_NONE, nullptr);

    std::string type = "";
    JsUtils::GetValue(env, argv[ARGC_ZERO], type);
    PARAM_CHECK_RETURN(env, EVENT_TYPE.find(type) != EVENT_TYPE.end(), "unkown type", TYPE_NONE, nullptr);
    auto engine = reinterpret_cast<JsGetInputMethodController *>(JsUtils::GetNativeSelf(env, info));
    if (engine == nullptr) {
        return nullptr;
    }
    engine->UnRegisterListener(type);

    napi_value result = nullptr;
    napi_get_null(env, &result);
    return result;
}

napi_value JsGetInputMethodController::CreateSelectRange(napi_env env, int32_t start, int32_t end)
{
    napi_value range = nullptr;
    napi_create_object(env, &range);

    napi_value value = nullptr;
    napi_create_int32(env, start, &value);
    napi_set_named_property(env, range, "start", value);

    napi_create_int32(env, end, &value);
    napi_set_named_property(env, range, "end", value);

    return range;
}

napi_value JsGetInputMethodController::CreateSelectMovement(napi_env env, int32_t direction)
{
    napi_value movement = nullptr;
    napi_create_object(env, &movement);

    napi_value value = nullptr;
    napi_create_int32(env, direction, &value);
    napi_set_named_property(env, movement, "direction", value);

    return movement;
}

napi_value JsGetInputMethodController::HandleSoftKeyboard(
    napi_env env, napi_callback_info info, std::function<int32_t()> callback, bool isOutput, bool needThrowException)
{
    auto ctxt = std::make_shared<HandleContext>();
    auto input = [ctxt](
                     napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status { return napi_ok; };
    auto output = [ctxt, isOutput](napi_env env, napi_value *result) -> napi_status {
        if (!isOutput) {
            return napi_ok;
        }
        napi_status status = napi_get_boolean(env, ctxt->isHandle, result);
        IMSA_HILOGE("output napi_get_boolean != nullptr[%{public}d]", result != nullptr);
        return status;
    };
    auto exec = [ctxt, callback, needThrowException](AsyncCall::Context *ctx) {
        int errCode = callback();
        if (errCode == ErrorCode::NO_ERROR) {
            IMSA_HILOGI("exec success");
            ctxt->status = napi_ok;
            ctxt->isHandle = true;
            ctxt->SetState(ctxt->status);
            return;
        }
        IMSA_HILOGI("exec %{public}d", errCode);
        if (needThrowException) {
            ctxt->SetErrorCode(errCode);
        }
    };
    ctxt->SetAction(std::move(input), std::move(output));
    AsyncCall asyncCall(env, info, std::dynamic_pointer_cast<AsyncCall::Context>(ctxt), 0);
    return asyncCall.Call(env, exec);
}

napi_value JsGetInputMethodController::ShowSoftKeyboard(napi_env env, napi_callback_info info)
{
    return HandleSoftKeyboard(
        env, info, [] { return InputMethodController::GetInstance()->ShowSoftKeyboard(); }, false, true);
}

napi_value JsGetInputMethodController::HideSoftKeyboard(napi_env env, napi_callback_info info)
{
    return HandleSoftKeyboard(
        env, info, [] { return InputMethodController::GetInstance()->HideSoftKeyboard(); }, false, true);
}

napi_value JsGetInputMethodController::StopInputSession(napi_env env, napi_callback_info info)
{
    return HandleSoftKeyboard(
        env, info, [] { return InputMethodController::GetInstance()->StopInputSession(); }, true, true);
}

napi_value JsGetInputMethodController::StopInput(napi_env env, napi_callback_info info)
{
    return HandleSoftKeyboard(
        env, info, [] { return InputMethodController::GetInstance()->HideCurrentInput(); }, true, false);
}

void JsGetInputMethodController::OnSelectByRange(int32_t start, int32_t end)
{
    IMSA_HILOGD("run in, start: %{public}d, end: %{public}d", start, end);
    std::string type = "selectByRange";
    uv_work_t *work = GetUVwork("selectByRange", [start, end](UvEntry &entry) {
        entry.start = start;
        entry.end = end;
    });
    if (work == nullptr) {
        IMSA_HILOGD("failed to get uv entry");
        return;
    }
    uv_queue_work(
        loop_, work, [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            std::shared_ptr<UvEntry> entry(static_cast<UvEntry *>(work->data), [work](UvEntry *data) {
                delete data;
                delete work;
            });
            if (entry == nullptr) {
                IMSA_HILOGE("OnSelectByRange entryptr is null");
                return;
            }
            auto getProperty = [entry](napi_value *args, uint8_t argc, std::shared_ptr<JSCallbackObject> item) -> bool {
                if (argc < ARGC_ONE) {
                    return false;
                }
                napi_value range = CreateSelectRange(item->env_, entry->start, entry->end);
                if (range == nullptr) {
                    IMSA_HILOGE("set select range failed");
                    return false;
                }
                args[ARGC_ZERO] = range;
                return true;
            };
            JsUtils::TraverseCallback(entry->vecCopy, ARGC_ONE, getProperty);
        });
}

void JsGetInputMethodController::OnSelectByMovement(int32_t direction)
{
    IMSA_HILOGD("run in, direction: %{public}d", direction);
    std::string type = "selectByMovement";
    uv_work_t *work = GetUVwork(type, [direction](UvEntry &entry) { entry.direction = direction; });
    if (work == nullptr) {
        IMSA_HILOGD("failed to get uv entry");
        return;
    }
    uv_queue_work(
        loop_, work, [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            std::shared_ptr<UvEntry> entry(static_cast<UvEntry *>(work->data), [work](UvEntry *data) {
                delete data;
                delete work;
            });
            if (entry == nullptr) {
                IMSA_HILOGE("OnSelectByMovement entryptr is null");
                return;
            }
            auto getProperty = [entry](napi_value *args, uint8_t argc, std::shared_ptr<JSCallbackObject> item) -> bool {
                if (argc < 1) {
                    return false;
                }
                napi_value movement = CreateSelectMovement(item->env_, entry->direction);
                if (movement == nullptr) {
                    IMSA_HILOGE("set select movement failed");
                    return false;
                }
                args[ARGC_ZERO] = movement;
                return true;
            };
            JsUtils::TraverseCallback(entry->vecCopy, ARGC_ONE, getProperty);
        });
}

uv_work_t *JsGetInputMethodController::GetUVwork(const std::string &type, EntrySetter entrySetter)
{
    IMSA_HILOGD("run in, type: %{public}s", type.c_str());
    UvEntry *entry = nullptr;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);

        if (jsCbMap_[type].empty()) {
            IMSA_HILOGE("%{public}s cb-vector is empty", type.c_str());
            return nullptr;
        }
        entry = new (std::nothrow) UvEntry(jsCbMap_[type], type);
        if (entry == nullptr) {
            IMSA_HILOGE("entry ptr is nullptr!");
            return nullptr;
        }
        if (entrySetter != nullptr) {
            entrySetter(*entry);
        }
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        IMSA_HILOGE("entry ptr is nullptr!");
        return nullptr;
    }
    work->data = entry;
    return work;
}
} // namespace MiscServices
} // namespace OHOS