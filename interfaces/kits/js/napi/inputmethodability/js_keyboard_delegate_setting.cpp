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

#include "js_keyboard_delegate_setting.h"
#include "js_keyboard_controller_engine.h"
#include "js_text_input_client_engine.h"
#include "input_method_ability.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace MiscServices {
const std::string JsKeyboardDelegateSetting::KDS_CLASS_NAME = "KeyboardDelegate";
thread_local napi_ref JsKeyboardDelegateSetting::KDSRef_ = nullptr;
napi_value JsKeyboardDelegateSetting::Init(napi_env env, napi_value exports) {    
    napi_property_descriptor descriptor[] = {
        DECLARE_NAPI_FUNCTION("createKeyboardDelegate", CreateKeyboardDelegate),
    };
    NAPI_CALL(
        env, napi_define_properties(env, exports, sizeof(descriptor) / sizeof(napi_property_descriptor), descriptor));

    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("on", Subscribe),
        DECLARE_NAPI_FUNCTION("off", UnSubscribe),
    };
    napi_value cons = nullptr;
    NAPI_CALL(env, napi_define_class(env, KDS_CLASS_NAME.c_str(), KDS_CLASS_NAME.size(),
        JsConstructor, nullptr, sizeof(properties) / sizeof(napi_property_descriptor), properties, &cons));
    NAPI_CALL(env, napi_create_reference(env, cons, 1, &KDSRef_));
    NAPI_CALL(env, napi_set_named_property(env, exports, KDS_CLASS_NAME.c_str(), cons));
    return exports;
};

napi_value JsKeyboardDelegateSetting::JsConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr));

    std::shared_ptr<JsKeyboardDelegateSetting> KDSobject = std::make_shared<JsKeyboardDelegateSetting>();
    InputMethodAbility::GetInstance()->setKdListener(KDSobject);
    if (KDSobject == nullptr) {
        IMSA_HILOGE("KDSobject == nullptr");
        napi_value result = nullptr;
        napi_get_null(env, &result);
        return result;
    }
    napi_wrap(env, thisVar, KDSobject.get(), [](napi_env env, void *data, void *hint) {
    }, nullptr, nullptr);
    napi_get_uv_event_loop(env, &KDSobject->loop_);
    return thisVar;
};

napi_value JsKeyboardDelegateSetting::CreateKeyboardDelegate(napi_env env, napi_callback_info info) {
    napi_value instance = nullptr;
    napi_value cons = nullptr;
    if (napi_get_reference_value(env, KDSRef_, &cons) != napi_ok) {
        IMSA_HILOGE("napi_get_reference_value(env, KDSRef_, &cons) != napi_ok");
        return nullptr;
    }
    IMSA_HILOGE("Get a reference to the global variable appAccountRef_ complete");
    if (napi_new_instance(env, cons, 0, nullptr, &instance) != napi_ok) {
        IMSA_HILOGE("napi_new_instance(env, cons, 0, nullptr, &instance) != napi_ok");
        return nullptr;
    }
    return instance;
}

std::string JsKeyboardDelegateSetting::GetStringProperty(napi_env env, napi_value jsString)
{
    char propValue[MAX_VALUE_LEN] = {0};
    size_t propLen;
    if (napi_get_value_string_utf8(env, jsString, propValue, MAX_VALUE_LEN, &propLen) != napi_ok) {
        IMSA_HILOGE("GetStringProperty error");
    }
    return std::string(propValue);
}

void JsKeyboardDelegateSetting::RegisterListener(napi_value callback, std::string type, 
    std::shared_ptr<CallbackObj> callbackObj)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        IMSA_HILOGE("methodName %{public}s not registertd!", type.c_str());
    }

    for (auto &item : jsCbMap_[type]) {
        if (Equals(item->env_, callback, item->callback_)) {
            IMSA_HILOGE("JsKeyboardDelegateSetting::RegisterListener callback already registered!");
            return;
        }
    }

    jsCbMap_[type].push_back(std::move(callbackObj));
}

void JsKeyboardDelegateSetting::UnRegisterListener(napi_value callback, std::string type)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        IMSA_HILOGE("methodName %{public}s not unRegisterted!", type.c_str());
        return;
    }
    
    if (callback == nullptr) {
        jsCbMap_.erase(type);
        IMSA_HILOGE("callback is nullptr");
        return;
    }

    for (auto item = jsCbMap_[type].begin(); item != jsCbMap_[type].end();) {
        if ((callback != nullptr) && (Equals((*item)->env_, callback, (*item)->callback_))) {
            jsCbMap_[type].erase(item);
            break;
        }
    }
    if (jsCbMap_[type].empty()) {
        jsCbMap_.erase(type);
    }
}

JsKeyboardDelegateSetting *JsKeyboardDelegateSetting::GetNative(napi_env env, napi_callback_info info)
{
    size_t argc = AsyncCall::ARGC_MAX;
    void *native = nullptr;
    napi_value self = nullptr;
    napi_value argv[AsyncCall::ARGC_MAX] = { nullptr };
    napi_status status = napi_invalid_arg;
    status = napi_get_cb_info(env, info, &argc, argv, &self, nullptr);
    if (self == nullptr && argc >= AsyncCall::ARGC_MAX) {
        IMSA_HILOGE("napi_get_cb_info failed");
        return nullptr;
    }

    status = napi_unwrap(env, self, &native);
    NAPI_ASSERT(env, (status == napi_ok && native != nullptr), "napi_unwrap failed!");
    return reinterpret_cast<JsKeyboardDelegateSetting*>(native);
}

napi_value JsKeyboardDelegateSetting::Subscribe(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[2] = {nullptr};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, argc == 2, "Wrong number of arguments, requires 2");
    
    napi_valuetype valuetype;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_string, "type is not a string");
    std::string type = GetStringProperty(env, argv[0]);
    IMSA_HILOGE("event type is: %{public}s", type.c_str());

    valuetype = napi_undefined;
    napi_typeof(env, argv[1], &valuetype);
    NAPI_ASSERT(env, valuetype == napi_function, "callback is not a function");
    
    auto engine = GetNative(env, info);
    if (engine == nullptr){
        return nullptr;
    }
    std::shared_ptr<CallbackObj> callbackObj = std::make_shared<CallbackObj>(env, argv[1]);
    engine->RegisterListener(argv[1], type, callbackObj);

    napi_value result = nullptr;
    napi_get_null(env, &result);
    return result;
}

napi_value JsKeyboardDelegateSetting::UnSubscribe(napi_env env, napi_callback_info info)
{    
    size_t argc = 2;
    napi_value argv[2] = {nullptr};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, argc == 1 || argc == 2, "Wrong number of arguments, requires 1 or 2");
    
    napi_valuetype valuetype;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_string, "type is not a string");
    std::string type = GetStringProperty(env, argv[0]);

    auto engine = GetNative(env, info);
    if (engine == nullptr){
        return nullptr;
    }

    if (argc == 2) {
        valuetype = napi_undefined;
        napi_typeof(env, argv[1], &valuetype);
        NAPI_ASSERT(env, valuetype == napi_function, "callback is not a function");
    }

    auto callbackObj = std::make_shared<CallbackObj>(env, argv[1]);
    engine->UnRegisterListener(argv[1], type);

    napi_value result = nullptr;
    napi_get_null(env, &result);
    return result;
}

bool JsKeyboardDelegateSetting::Equals(napi_env env, napi_value value, napi_ref copy)
{
    if (copy == nullptr) {
        return (value == nullptr);
    }

    napi_value copyValue = nullptr;
    napi_get_reference_value(env, copy, &copyValue);

    bool isEquals = false;
    napi_strict_equals(env, value, copyValue, &isEquals);
    IMSA_HILOGE("run in Equals::isEquals is %{public}d", isEquals);
    return isEquals;
}

napi_value JsKeyboardDelegateSetting::GetResultOnKeyEvent(napi_env env, int32_t keyCode, int32_t keyStatus)
{
    napi_value KeyboardDelegate = nullptr;
    napi_create_object(env, &KeyboardDelegate);

    napi_value jsKeyCode = nullptr;
    napi_create_int32(env, keyCode, &jsKeyCode);
    napi_set_named_property(env, KeyboardDelegate, "keyCode", jsKeyCode);

    napi_value jsKeyAction = nullptr;
    napi_create_int32(env, keyStatus, &jsKeyAction);
    napi_set_named_property(env, KeyboardDelegate, "keyAction", jsKeyAction);

    return KeyboardDelegate;
}

bool JsKeyboardDelegateSetting::OnKeyEvent(int32_t keyCode, int32_t keyStatus)
{
    UvEntry *entry = nullptr;
    bool isOnKeyEvent = false;
    bool isResult = false;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        std::string type = keyStatus == 2 ? "keyDown" : "keyUp";
        if (jsCbMap_[type].empty()) {
            IMSA_HILOGE("OnKeyEvent cb-vector is empty");
            return isOnKeyEvent;
        }
        entry = new (std::nothrow) UvEntry(jsCbMap_[type], type);
        if (entry == nullptr) {
            IMSA_HILOGE("entry ptr is nullptr!");
            return isOnKeyEvent;
        }
    }
    for (auto item : entry->vecCopy) {
        napi_value jsObject = GetResultOnKeyEvent(item->env_, keyCode, keyStatus);
        if (jsObject == nullptr) {
            IMSA_HILOGE("get GetResultOnKeyEvent failed: %{punlic}p", jsObject);
        }

        napi_value callback = nullptr;
        napi_value args[1] = {jsObject};
        napi_get_reference_value(item->env_, item->callback_, &callback);
        if (callback == nullptr) {
            IMSA_HILOGE("callback is nullptr");
            continue;
        }
        napi_value global = nullptr;
        napi_get_global(item->env_, &global);
        napi_value result;
        napi_status callStatus = napi_call_function(item->env_, global, callback, 1, args,
            &result);
        napi_get_value_bool(item->env_, result, &isResult);
        if (isResult) {
            isOnKeyEvent = true;
        }
        if (callStatus != napi_ok) {
            IMSA_HILOGE("notify data change failed callStatus:%{public}d callback:%{public}p", callStatus,
                callback);
        }
    }
    return isOnKeyEvent;
}

void JsKeyboardDelegateSetting::OnCursorUpdate(int32_t positionX, int32_t positionY, int height)
{
    struct CursorUvEntry : public UvEntry {
        CursorUvEntry(std::vector<std::shared_ptr<CallbackObj>> cbVec,
            std::string type) : UvEntry(cbVec, type) {};
        int32_t positionX_;
        int32_t positionY_;
        int height_;
    };
    CursorUvEntry *entry = nullptr;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        std::string type = "cursorContextChange";
        if (jsCbMap_[type].empty()) {
            IMSA_HILOGE("OnCursorUpdate cb-vector is empty");
            return;
        }
        entry = new (std::nothrow) CursorUvEntry(jsCbMap_[type], type);
        if (entry == nullptr) {
            IMSA_HILOGE("entry ptr is nullptr!");
            return;
        }
    }
    entry->positionX_ = positionX;
    entry->positionY_ = positionY;
    entry->height_ = height;

    uv_work_t *work = new uv_work_t;
    if (work == nullptr) {
        IMSA_HILOGE("entry ptr is nullptr!");
        return;
    }
    work->data = entry;
    uv_queue_work(loop_, work,
        [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            std::shared_ptr<CursorUvEntry> entry(static_cast<CursorUvEntry *>(work->data), [work](CursorUvEntry *data) {
                delete data;
                delete work;
            });

            for (auto item : entry->vecCopy) {
                
                napi_value args[3] = {nullptr};
                napi_create_int32(item->env_, entry->positionX_, &args[0]);
                napi_create_int32(item->env_, entry->positionY_, &args[1]);
                napi_create_int32(item->env_, entry->height_, &args[2]);
                
                napi_value callback = nullptr;
                napi_get_reference_value(item->env_, item->callback_, &callback);
                if (callback == nullptr) {
                    IMSA_HILOGE("callback is nullptr");
                    continue;
                }
                napi_value global = nullptr;
                napi_get_global(item->env_, &global);
                napi_value result;
                napi_status callStatus = napi_call_function(item->env_, global, callback, 3, args,
                    &result);
                if (callStatus != napi_ok) {
                    IMSA_HILOGE("notify data change failed callStatus:%{public}d callback:%{public}p", callStatus,
                        callback);
                }
            }
        });
}

void JsKeyboardDelegateSetting::OnSelectionChange(int32_t oldBegin, int32_t oldEnd, int32_t newBegin, int32_t newEnd)
{
    struct SelectionUvEntry : public UvEntry {
        SelectionUvEntry(std::vector<std::shared_ptr<CallbackObj>> cbVec,
            std::string type) : UvEntry(cbVec, type) {};
        int32_t oldBegin_;
        int32_t oldEnd_;
        int32_t newBegin_;
        int32_t newEnd_;
    };
    SelectionUvEntry *entry = nullptr;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        std::string type = "selectionChange";
        if (jsCbMap_[type].empty()) {
            IMSA_HILOGE("OnSelectionChange cb-vector is empty");
            return;
        }
        entry = new (std::nothrow) SelectionUvEntry(jsCbMap_[type], type);
        if (entry == nullptr) {
            IMSA_HILOGE("entry ptr is nullptr!");
            return;
        }
    }
    entry->oldBegin_ = oldBegin;
    entry->oldEnd_ = oldEnd;
    entry->newBegin_ = newBegin;
    entry->newEnd_ = newEnd;

    uv_work_t *work = new uv_work_t;
    if (work == nullptr) {
        IMSA_HILOGE("entry ptr is nullptr!");
        return;
    }
    work->data = entry;
    uv_queue_work(loop_, work,
        [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            std::shared_ptr<SelectionUvEntry> entry(static_cast<SelectionUvEntry *>(work->data), [work](SelectionUvEntry *data) {
                delete data;
                delete work;
            });

            for (auto item : entry->vecCopy) {

                napi_value args[4] = {nullptr};
                napi_create_int32(item->env_, entry->oldBegin_, &args[0]);
                napi_create_int32(item->env_, entry->oldEnd_, &args[1]);
                napi_create_int32(item->env_, entry->newBegin_, &args[2]);
                napi_create_int32(item->env_, entry->newEnd_, &args[3]);

                napi_value callback = nullptr;
                napi_get_reference_value(item->env_, item->callback_, &callback);
                if (callback == nullptr) {
                    IMSA_HILOGE("callback is nullptr");
                    continue;
                }
                napi_value global = nullptr;
                napi_get_global(item->env_, &global);
                napi_value result;
                napi_status callStatus = napi_call_function(item->env_, global, callback, 4, args,
                    &result);
                if (callStatus != napi_ok) {
                    IMSA_HILOGE("notify data change failed callStatus:%{public}d callback:%{public}p", callStatus,
                        callback);
                }
            }
        });
}

void JsKeyboardDelegateSetting::OnTextChange(std::string text)
{
    struct TextUvEntry : public UvEntry {
        TextUvEntry(std::vector<std::shared_ptr<CallbackObj>> cbVec,
            std::string type) : UvEntry(cbVec, type) {};
        std::string text_;
    };
    TextUvEntry *entry = nullptr;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        std::string type = "textChange";
        if (jsCbMap_[type].empty()) {
            IMSA_HILOGE("OnTextChange cb-vector is empty");
            return;
        }
        entry = new (std::nothrow) TextUvEntry(jsCbMap_[type], type);
        if (entry == nullptr) {
            IMSA_HILOGE("entry ptr is nullptr!");
            return;
        }
    }
    entry->text_ = text;

    uv_work_t *work = new uv_work_t;
    work->data = entry;
    uv_queue_work(loop_, work,
        [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            std::shared_ptr<TextUvEntry> entry(static_cast<TextUvEntry *>(work->data), [work](TextUvEntry *data) {
                delete data;
                delete work;
            });

            for (auto item : entry->vecCopy) {
                
                napi_value args[1] = {nullptr};
                napi_create_string_utf8(item->env_, entry->text_.c_str(), NAPI_AUTO_LENGTH, &args[0]);
                
                napi_value callback = nullptr;
                napi_get_reference_value(item->env_, item->callback_, &callback);
                if (callback == nullptr) {
                    IMSA_HILOGE("callback is nullptr");
                    continue;
                }
                napi_value global = nullptr;
                napi_get_global(item->env_, &global);
                napi_value result;
                napi_status callStatus = napi_call_function(item->env_, global, callback, 1, args,
                    &result);
                if (callStatus != napi_ok) {
                    IMSA_HILOGE("notify data change failed callStatus:%{public}d callback:%{public}p", callStatus,
                        callback);
                }
            }
        });
}
}
}