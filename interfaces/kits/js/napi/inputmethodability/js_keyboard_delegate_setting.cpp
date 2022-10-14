/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "js_utils.h"

namespace OHOS {
namespace MiscServices {
constexpr size_t ARGC_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr size_t ARGC_THREE = 3;
constexpr size_t ARGC_FOUR = 4;
constexpr int32_t V9_FLAG = 1;
constexpr int32_t ORIGINAL_FLAG = 2;
const std::string JsKeyboardDelegateSetting::KDS_CLASS_NAME = "KeyboardDelegate";
thread_local napi_ref JsKeyboardDelegateSetting::KDSRef_ = nullptr;

std::mutex JsKeyboardDelegateSetting::keyboardMutex_;
std::shared_ptr<JsKeyboardDelegateSetting> JsKeyboardDelegateSetting::keyboardDelegate_ { nullptr };

napi_value JsKeyboardDelegateSetting::Init(napi_env env, napi_value exports)
{
    napi_property_descriptor descriptor[] = {
        
        DECLARE_NAPI_PROPERTY("OPTION_ASCII", GetJsConstProperty(env, static_cast<uint32_t>(20))),
        DECLARE_NAPI_PROPERTY("OPTION_NONE", GetJsConstProperty(env, static_cast<uint32_t>(0))),
        DECLARE_NAPI_PROPERTY("OPTION_AUTO_CAP_CHARACTERS", GetJsConstProperty(env, static_cast<uint32_t>(2))),
        DECLARE_NAPI_PROPERTY("OPTION_AUTO_CAP_SENTENCES", GetJsConstProperty(env, static_cast<uint32_t>(8))),
        DECLARE_NAPI_PROPERTY("OPTION_AUTO_WORDS", GetJsConstProperty(env, static_cast<uint32_t>(4))),
        DECLARE_NAPI_PROPERTY("OPTION_MULTI_LINE", GetJsConstProperty(env, static_cast<uint32_t>(1))),
        DECLARE_NAPI_PROPERTY("OPTION_NO_FULLSCREEN", GetJsConstProperty(env, static_cast<uint32_t>(10))),
        DECLARE_NAPI_PROPERTY("CURSOR_UP", GetJsConstProperty(env, static_cast<uint32_t>(1))),
        DECLARE_NAPI_PROPERTY("CURSOR_DOWN", GetJsConstProperty(env, static_cast<uint32_t>(2))),
        DECLARE_NAPI_PROPERTY("CURSOR_LEFT", GetJsConstProperty(env, static_cast<uint32_t>(3))),
        DECLARE_NAPI_PROPERTY("CURSOR_RIGHT", GetJsConstProperty(env, static_cast<uint32_t>(4))),

        DECLARE_NAPI_PROPERTY("FLAG_SELECTING", GetJsConstProperty(env, static_cast<uint32_t>(2))),
        DECLARE_NAPI_PROPERTY("FLAG_SINGLE_LINE", GetJsConstProperty(env, static_cast<uint32_t>(1))),

        DECLARE_NAPI_PROPERTY("DISPLAY_MODE_PART", GetJsConstProperty(env, static_cast<uint32_t>(0))),
        DECLARE_NAPI_PROPERTY("DISPLAY_MODE_FULL", GetJsConstProperty(env, static_cast<uint32_t>(1))),
        DECLARE_NAPI_PROPERTY("WINDOW_TYPE_INPUT_METHOD_FLOAT", GetJsConstProperty(env, static_cast<uint32_t>(2105))),

        DECLARE_NAPI_FUNCTION("createKeyboardDelegate", CreateKeyboardDelegate),
        DECLARE_NAPI_FUNCTION("getKeyboardDelegate", GetKeyboardDelegate),
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

napi_value JsKeyboardDelegateSetting::GetJsConstProperty(napi_env env, uint32_t num)
{
    napi_value jsNumber = nullptr;
    napi_create_int32(env, num, &jsNumber);
    return jsNumber;
};

std::shared_ptr<JsKeyboardDelegateSetting> JsKeyboardDelegateSetting::GetKeyboardDelegateSetting()
{
    if (keyboardDelegate_ == nullptr) {
        std::lock_guard<std::mutex> lock(keyboardMutex_);
        if (keyboardDelegate_ == nullptr) {
            auto delegate = std::make_shared<JsKeyboardDelegateSetting>();
            if (delegate == nullptr) {
                IMSA_HILOGE("keyboard delegate nullptr");
                return nullptr;
            }
            keyboardDelegate_ = delegate;
            InputMethodAbility::GetInstance()->setKdListener(keyboardDelegate_);
        }
    }
    return keyboardDelegate_;
}

napi_value JsKeyboardDelegateSetting::JsConstructor(napi_env env, napi_callback_info info)
{
    IMSA_HILOGI("run in JsConstructor");
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr));
    auto delegate = GetKeyboardDelegateSetting();
    if (delegate == nullptr) {
        IMSA_HILOGE("get delegate nullptr");
        napi_value result = nullptr;
        napi_get_null(env, &result);
        return result;
    }
    napi_wrap(env, thisVar, delegate.get(), [](napi_env env, void *nativeObject, void *hint) {
        IMSA_HILOGE("delete JsKeyboardDelegateSetting");
    }, nullptr, nullptr);
    if (delegate->loop_ == nullptr) {
        napi_get_uv_event_loop(env, &delegate->loop_);
    }
    return thisVar;
};

napi_value JsKeyboardDelegateSetting::CreateKeyboardDelegate(napi_env env, napi_callback_info info)
{
    return GetKDInstance(env, info, ORIGINAL_FLAG);
}

napi_value JsKeyboardDelegateSetting::GetKeyboardDelegate(napi_env env, napi_callback_info info)
{
    return GetKDInstance(env, info, V9_FLAG);
}

napi_value JsKeyboardDelegateSetting::GetKDInstance(napi_env env, napi_callback_info info, int flag)
{
    napi_value instance = nullptr;
    napi_value cons = nullptr;
    if (flag == V9_FLAG) {
        size_t argc = AsyncCall::ARGC_MAX;
        napi_value argv[AsyncCall::ARGC_MAX] = { nullptr };
    
        NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
        if (argc != ARGC_ZERO) {
            JsUtils::ThrowException(env, IMFErrorCode::EXCEPTION_PARAMCHECK, "Wrong number of arguments, requires 0", TypeCode::TYPE_NONE);
        }
    }

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
    std::shared_ptr<JSCallbackObject> callbackObj)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        IMSA_HILOGE("methodName %{public}s not registered!", type.c_str());
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
    size_t argc = ARGC_TWO;
    napi_value argv[ARGC_TWO] = {nullptr};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    if (argc != ARGC_TWO) {
        JsUtils::ThrowException(env, IMFErrorCode::EXCEPTION_PARAMCHECK, "Wrong number of arguments, requires 2", TypeCode::TYPE_NONE);
    }
    
    napi_valuetype valuetype;
    NAPI_CALL(env, napi_typeof(env, argv[ARGC_ZERO], &valuetype));
    if (valuetype != napi_string) {
        JsUtils::ThrowException(env, IMFErrorCode::EXCEPTION_PARAMCHECK, "'type'", TypeCode::TYPE_STRING);
        return nullptr;
    }
    std::string type = GetStringProperty(env, argv[ARGC_ZERO]);
    IMSA_HILOGE("event type is: %{public}s", type.c_str());

    valuetype = napi_undefined;
    napi_typeof(env, argv[ARGC_ONE], &valuetype);
    if (valuetype != napi_function) {
        JsUtils::ThrowException(env, IMFErrorCode::EXCEPTION_PARAMCHECK, "'callback'", TypeCode::TYPE_FUNCTION);
        return nullptr;
    }
    
    auto engine = GetNative(env, info);
    if (engine == nullptr) {
        return nullptr;
    }
    std::shared_ptr<JSCallbackObject> callback = std::make_shared<JSCallbackObject>(env, argv[1]);
    engine->RegisterListener(argv[ARGC_ONE], type, callback);

    napi_value result = nullptr;
    napi_get_null(env, &result);
    return result;
}

napi_value JsKeyboardDelegateSetting::UnSubscribe(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_TWO;
    napi_value argv[ARGC_TWO] = {nullptr};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    if (argc != ARGC_ONE && argc != ARGC_TWO) {
        JsUtils::ThrowException(env, IMFErrorCode::EXCEPTION_PARAMCHECK, "Wrong number of arguments, requires 1 or 2", TypeCode::TYPE_NONE);
        return nullptr;
    }
    
    napi_valuetype valuetype;
    NAPI_CALL(env, napi_typeof(env, argv[ARGC_ZERO], &valuetype));
    if (valuetype != napi_string) {
        JsUtils::ThrowException(env, IMFErrorCode::EXCEPTION_PARAMCHECK, "'type'", TypeCode::TYPE_STRING);
        return nullptr;
    }
    std::string type = GetStringProperty(env, argv[ARGC_ZERO]);

    auto delegate = GetNative(env, info);
    if (delegate == nullptr) {
        return nullptr;
    }

    if (argc == ARGC_TWO) {
        valuetype = napi_undefined;
        napi_typeof(env, argv[ARGC_ONE], &valuetype);
        if (valuetype != napi_function) {
            JsUtils::ThrowException(env, IMFErrorCode::EXCEPTION_PARAMCHECK, "'callback'", TypeCode::TYPE_FUNCTION);
            return nullptr;
        }
    }
    delegate->UnRegisterListener(argv[ARGC_ONE], type);
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
    IMSA_HILOGI("run in OnKeyEvent");
    KeyEventPara para { keyCode, keyStatus, false };
    std::string type = (keyStatus == ARGC_TWO ? "keyDown" : "keyUp");
    auto isDone = std::make_shared<BlockData<bool>>(MAX_TIMEOUT, false);
    uv_work_t *work = GetKeyEventUVwork(type, para, isDone);
    if (work == nullptr) {
        IMSA_HILOGE("GetKeyEventUVwork nullptr");
        return false;
    }
    uv_queue_work(
        loop_, work, [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            bool isResult = false;
            std::shared_ptr<UvEntry> entry(static_cast<UvEntry *>(work->data), [work](UvEntry *data) {
                delete data;
                delete work;
            });
            bool isOnKeyEvent = false;
            for (auto item : entry->vecCopy) {
                napi_value jsObject =
                    GetResultOnKeyEvent(item->env_, entry->keyEventPara.keyCode, entry->keyEventPara.keyStatus);
                if (jsObject == nullptr) {
                    IMSA_HILOGE("get GetResultOnKeyEvent failed: %{punlic}p", jsObject);
                    continue;
                }
                napi_value callback = nullptr;
                napi_value args[] = { jsObject };
                napi_get_reference_value(item->env_, item->callback_, &callback);
                if (callback == nullptr) {
                    IMSA_HILOGE("callback is nullptr");
                    continue;
                }
                napi_value global = nullptr;
                napi_get_global(item->env_, &global);
                napi_value result = nullptr;
                napi_status callStatus = napi_call_function(item->env_, global, callback, 1, args, &result);
                if (result != nullptr) {
                    napi_get_value_bool(item->env_, result, &isResult);
                }
                if (isResult) {
                    isOnKeyEvent = true;
                }
                if (callStatus != napi_ok) {
                    IMSA_HILOGE(
                        "notify data change failed callStatus:%{public}d callback:%{public}p", callStatus, callback);
                }
            }
            entry->isDone->SetValue(isOnKeyEvent);
        });
    return isDone->GetValue();
}

uv_work_t *JsKeyboardDelegateSetting::GetKeyEventUVwork(
    std::string type, KeyEventPara para, std::shared_ptr<BlockData<bool>> &isDone)
{
    UvEntry *entry = nullptr;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);

        if (jsCbMap_[type].empty()) {
            IMSA_HILOGE("cb-vector is empty");
            return nullptr;
        }
        entry = new (std::nothrow) UvEntry(jsCbMap_[type], type);
        if (entry == nullptr) {
            IMSA_HILOGE("entry ptr is nullptr!");
            return nullptr;
        }
        entry->keyEventPara = { para.keyCode, para.keyStatus, para.isOnKeyEvent };
        entry->isDone = isDone;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        IMSA_HILOGE("entry ptr is nullptr!");
        delete entry;
        return nullptr;
    }
    work->data = entry;
    return work;
}

uv_work_t *JsKeyboardDelegateSetting::GetCursorUVwork(std::string type, CursorPara para)
{
    UvEntry *entry = nullptr;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);

        if (jsCbMap_[type].empty()) {
            IMSA_HILOGE("OnInputStart cb-vector is empty");
            return nullptr;
        }
        entry = new (std::nothrow) UvEntry(jsCbMap_[type], type);
        if (entry == nullptr) {
            IMSA_HILOGE("entry ptr is nullptr!");
            return nullptr;
        }
        entry->curPara.positionX = para.positionX;
        entry->curPara.positionY = para.positionY;
        entry->curPara.height = para.height;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        IMSA_HILOGE("entry ptr is nullptr!");
        return nullptr;
    }
    work->data = entry;
    return work;
}

uv_work_t *JsKeyboardDelegateSetting::GetSelectionUVwork(std::string type, SelectionPara para)
{
    UvEntry *entry = nullptr;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);

        if (jsCbMap_[type].empty()) {
            IMSA_HILOGE("OnInputStart cb-vector is empty");
            return nullptr;
        }
        entry = new (std::nothrow) UvEntry(jsCbMap_[type], type);
        if (entry == nullptr) {
            IMSA_HILOGE("entry ptr is nullptr!");
            return nullptr;
        }
        entry->selPara.oldBegin = para.oldBegin;
        entry->selPara.oldEnd = para.oldEnd;
        entry->selPara.newBegin = para.newBegin;
        entry->selPara.newEnd = para.newEnd;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        IMSA_HILOGE("entry ptr is nullptr!");
        return nullptr;
    }
    work->data = entry;
    return work;
}

uv_work_t *JsKeyboardDelegateSetting::GetTextUVwork(std::string type, std::string text)
{
    UvEntry *entry = nullptr;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);

        if (jsCbMap_[type].empty()) {
            IMSA_HILOGE("OnInputStart cb-vector is empty");
            return nullptr;
        }
        entry = new (std::nothrow) UvEntry(jsCbMap_[type], type);
        if (entry == nullptr) {
            IMSA_HILOGE("entry ptr is nullptr!");
            return nullptr;
        }
        entry->text = text;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        IMSA_HILOGE("entry ptr is nullptr!");
        return nullptr;
    }
    work->data = entry;
    return work;
}

void JsKeyboardDelegateSetting::OnCursorUpdate(int32_t positionX, int32_t positionY, int32_t height)
{
    CursorPara para {positionX, positionY, height};
    std::string type = "cursorContextChange";
    uv_work_t *work = GetCursorUVwork(type, para);
    if (work == nullptr) {
        return;
    }
    uv_queue_work(loop_, work,
        [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            std::shared_ptr<UvEntry> entry(static_cast<UvEntry *>(work->data), [work](UvEntry *data) {
                delete data;
                delete work;
            });

            for (auto item : entry->vecCopy) {
                napi_value args[ARGC_THREE] = {nullptr};
                napi_create_int32(item->env_, entry->curPara.positionX, &args[ARGC_ZERO]);
                napi_create_int32(item->env_, entry->curPara.positionY, &args[ARGC_ONE]);
                napi_create_int32(item->env_, entry->curPara.height, &args[ARGC_TWO]);
                
                napi_value callback = nullptr;
                napi_get_reference_value(item->env_, item->callback_, &callback);
                if (callback == nullptr) {
                    IMSA_HILOGE("callback is nullptr");
                    continue;
                }
                napi_value global = nullptr;
                napi_get_global(item->env_, &global);
                napi_value result;
                napi_status callStatus = napi_call_function(item->env_, global, callback, ARGC_THREE, args,
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
    SelectionPara para {oldBegin, oldEnd, newBegin, newEnd};
    std::string type = "selectionChange";
    uv_work_t *work = GetSelectionUVwork(type, para);
    if (work == nullptr) {
        return;
    }
    uv_queue_work(loop_, work,
        [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            std::shared_ptr<UvEntry> entry(static_cast<UvEntry *>(work->data),
                [work](UvEntry *data) {
                    delete data;
                    delete work;
            });

            for (auto item : entry->vecCopy) {
                napi_value args[ARGC_FOUR] = {nullptr};
                napi_create_int32(item->env_, entry->selPara.oldBegin, &args[ARGC_ZERO]);
                napi_create_int32(item->env_, entry->selPara.oldEnd, &args[ARGC_ONE]);
                napi_create_int32(item->env_, entry->selPara.newBegin, &args[ARGC_TWO]);
                napi_create_int32(item->env_, entry->selPara.newEnd, &args[ARGC_THREE]);

                napi_value callback = nullptr;
                napi_get_reference_value(item->env_, item->callback_, &callback);
                if (callback == nullptr) {
                    IMSA_HILOGE("callback is nullptr");
                    continue;
                }
                napi_value global = nullptr;
                napi_get_global(item->env_, &global);
                napi_value result;
                napi_status callStatus = napi_call_function(item->env_, global, callback, ARGC_FOUR, args,
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
    std::string type = "cursorContextChange";
    uv_work_t *work = GetTextUVwork(type, text);
    if (work == nullptr) {
        return;
    }
    uv_queue_work(loop_, work,
        [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            std::shared_ptr<UvEntry> entry(static_cast<UvEntry *>(work->data), [work](UvEntry *data) {
                delete data;
                delete work;
            });

            for (auto item : entry->vecCopy) {
                napi_value args[ARGC_ONE] = {nullptr};
                napi_create_string_utf8(item->env_, entry->text.c_str(), NAPI_AUTO_LENGTH, &args[ARGC_ZERO]);
                
                napi_value callback = nullptr;
                napi_get_reference_value(item->env_, item->callback_, &callback);
                if (callback == nullptr) {
                    IMSA_HILOGE("callback is nullptr");
                    continue;
                }
                napi_value global = nullptr;
                napi_get_global(item->env_, &global);
                napi_value result;
                napi_status callStatus = napi_call_function(item->env_, global, callback, ARGC_ONE, args,
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