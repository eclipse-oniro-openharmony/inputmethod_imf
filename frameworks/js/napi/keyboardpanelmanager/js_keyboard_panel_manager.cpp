/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "js_keyboard_panel_manager.h"

#include "event_checker.h"
#include "input_method_utils.h"
#include "js_callback_handler.h"
#include "js_util.h"
#include "js_utils.h"

namespace OHOS {
namespace MiscServices {
constexpr int32_t MAX_WAIT_TIME_PRIVATE_COMMAND = 2000;
BlockQueue<PrivateCommandInfo> JsKeyboardPanelManager::privateCommandQueue_{ MAX_WAIT_TIME_PRIVATE_COMMAND };
std::mutex JsKeyboardPanelManager::managerMutex_;
sptr<JsKeyboardPanelManager> JsKeyboardPanelManager::keyboardPanelManager_{ nullptr };

JsKeyboardPanelManager::JsKeyboardPanelManager()
{
    std::lock_guard<std::mutex> lock(eventHandlerMutex_);
    handler_ = AppExecFwk::EventHandler::Current();
}

napi_value JsKeyboardPanelManager::Init(napi_env env, napi_value info)
{
    napi_property_descriptor descriptor[] = {
        DECLARE_NAPI_FUNCTION("sendPrivateCommand", SendPrivateCommand),
        DECLARE_NAPI_FUNCTION("getSmartMenuCfg", GetSmartMenuCfg),
        DECLARE_NAPI_FUNCTION("on", Subscribe),
        DECLARE_NAPI_FUNCTION("off", UnSubscribe),
        DECLARE_NAPI_FUNCTION("getDefaultInputMethod", GetDefaultInputMethod),
        DECLARE_NAPI_FUNCTION("connectSystemCmd", ConnectSystemCmd),
    };
    NAPI_CALL(
        env, napi_define_properties(env, info, sizeof(descriptor) / sizeof(napi_property_descriptor), descriptor));
    return info;
}

sptr<JsKeyboardPanelManager> JsKeyboardPanelManager::GetInstance()
{
    if (keyboardPanelManager_ == nullptr) {
        std::lock_guard<std::mutex> lock(managerMutex_);
        if (keyboardPanelManager_ == nullptr) {
            keyboardPanelManager_ = new (std::nothrow) JsKeyboardPanelManager();
        }
    }
    return keyboardPanelManager_;
}

struct PanelManagerContext : public AsyncCall::Context {
    PanelManagerContext() : Context(nullptr, nullptr){};
    napi_status operator()(napi_env env, napi_value *result) override
    {
        if (status_ != napi_ok) {
            output_ = nullptr;
            return status_;
        }
        return Context::operator()(env, result);
    }
};

napi_value JsKeyboardPanelManager::ConnectSystemCmd(napi_env env, napi_callback_info info)
{
    auto ctxt = std::make_shared<PanelManagerContext>();
    auto manager = JsKeyboardPanelManager::GetInstance();
    auto exec = [ctxt, env, manager](AsyncCall::Context *ctx) {
        auto ret = ImeSystemCmdChannel::GetInstance()->ConnectSystemCmd(manager);
        ctxt->SetErrorCode(ret);
        CHECK_RETURN_VOID(ret == ErrorCode::NO_ERROR, "ConnectSystemCmd return error!");
        ctxt->SetState(napi_ok);
    };
    // 0 means JsAPI:ConnectSystemCmd has 0 params at most.
    AsyncCall asyncCall(env, info, ctxt, 0);
    return asyncCall.Call(env, exec, "ConnectSystemCmd");
}

napi_value JsKeyboardPanelManager::Subscribe(napi_env env, napi_callback_info info)
{
    size_t argc = 2; // has 2 param
    napi_value argv[2] = { nullptr };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    std::string type;
    // 2 means least param num.
    if (argc < 2 || !JsUtil::GetValue(env, argv[0], type)
        || !EventChecker::IsValidEventType(EventSubscribeModule::KEYBOARD_PANEL_MANAGER, type)
        || JsUtil::GetType(env, argv[1]) != napi_function) {
        IMSA_HILOGE("Subscribe failed, type:%{public}s", type.c_str());
        return nullptr;
    }
    auto manager = JsKeyboardPanelManager::GetInstance();
    IMSA_HILOGD("Subscribe type:%{public}s.", type.c_str());
    std::shared_ptr<JSCallbackObject> callback =
        std::make_shared<JSCallbackObject>(env, argv[1], std::this_thread::get_id());
    manager->RegisterListener(argv[1], type, callback);
    return JsUtil::Const::Null(env);
}

napi_value JsKeyboardPanelManager::UnSubscribe(napi_env env, napi_callback_info info)
{
    size_t argc = 2; // has 2 param
    napi_value argv[2] = { nullptr };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    std::string type;
    // 1 means least param num.
    if (argc < 1 || !JsUtil::GetValue(env, argv[0], type)
        || !EventChecker::IsValidEventType(EventSubscribeModule::KEYBOARD_PANEL_MANAGER, type)) {
        IMSA_HILOGE("UnSubscribe failed, type:%{public}s", type.c_str());
        return nullptr;
    }
    auto manager = JsKeyboardPanelManager::GetInstance();
    // if the second param is not napi_function/napi_null/napi_undefined, return
    auto paramType = JsUtil::GetType(env, argv[1]);
    if (paramType != napi_function && paramType != napi_null && paramType != napi_undefined) {
        return nullptr;
    }
    // if the second param is napi_function, delete it, else delete all
    argv[1] = paramType == napi_function ? argv[1] : nullptr;

    IMSA_HILOGD("UnSubscribe type:%{public}s.", type.c_str());
    manager->UnRegisterListener(argv[1], type);
    return JsUtil::Const::Null(env);
}

void JsKeyboardPanelManager::RegisterListener(
    napi_value callback, std::string type, std::shared_ptr<JSCallbackObject> callbackObj)
{
    IMSA_HILOGD("RegisterListener %{public}s", type.c_str());
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        IMSA_HILOGD("methodName: %{public}s not registered!", type.c_str());
    }
    auto callbacks = jsCbMap_[type];
    bool ret = std::any_of(callbacks.begin(), callbacks.end(), [&callback](std::shared_ptr<JSCallbackObject> cb) {
        return JsUtils::Equals(cb->env_, callback, cb->callback_, cb->threadId_);
    });
    if (ret) {
        IMSA_HILOGD("JsKeyboardPanelManagerListener callback already registered!");
        return;
    }

    IMSA_HILOGI("Add %{public}s callbackObj into jsCbMap_", type.c_str());
    jsCbMap_[type].push_back(std::move(callbackObj));
}

void JsKeyboardPanelManager::UnRegisterListener(napi_value callback, std::string type)
{
    IMSA_HILOGI("event: %{public}s", type.c_str());
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        IMSA_HILOGE("methodName: %{public}s already unRegistered!", type.c_str());
        return;
    }

    if (callback == nullptr) {
        jsCbMap_.erase(type);
        IMSA_HILOGI("callback is nullptr");
        return;
    }

    for (auto item = jsCbMap_[type].begin(); item != jsCbMap_[type].end(); item++) {
        if (JsUtils::Equals((*item)->env_, callback, (*item)->callback_, (*item)->threadId_)) {
            jsCbMap_[type].erase(item);
            break;
        }
    }

    if (jsCbMap_[type].empty()) {
        jsCbMap_.erase(type);
    }
}

napi_value JsKeyboardPanelManager::GetSmartMenuCfg(napi_env env, napi_callback_info info)
{
    auto ctxt = std::make_shared<SmartMenuContext>();
    auto output = [ctxt](napi_env env, napi_value *result) -> napi_status {
        *result = JsUtil::GetValue(env, ctxt->smartMenu);
        return napi_ok;
    };
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        ctxt->smartMenu = ImeSystemCmdChannel::GetInstance()->GetSmartMenuCfg();
        ctxt->SetState(napi_ok);
    };
    ctxt->SetAction(nullptr, std::move(output));
    // 1 means JsAPI:displayOptionalInputMethod has 1 params at most.
    AsyncCall asyncCall(env, info, ctxt, 1);
    return asyncCall.Call(env, exec, "GetSmartMenuCfg");
}


napi_value JsKeyboardPanelManager::SendPrivateCommand(napi_env env, napi_callback_info info)
{
    auto ctxt = std::make_shared<SendPrivateCommandContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        PARAM_CHECK_RETURN(env, argc > 0, "at least one paramster is required", TYPE_NONE, napi_generic_failure);
        CHECK_RETURN(JsUtils::GetValue(env, argv[0], ctxt->privateCommand) == napi_ok,
            "param commandData covert failed, type must be Record<string, CommandDataType>", napi_generic_failure);
        if (!TextConfig::IsPrivateCommandValid(ctxt->privateCommand)) {
            PARAM_CHECK_RETURN(
                env, false, "commandData size limit 32KB, count limit 5.", TYPE_NONE, napi_generic_failure);
        }
        ctxt->info = { std::chrono::system_clock::now(), ctxt->privateCommand };
        privateCommandQueue_.Push(ctxt->info);
        return napi_ok;
    };
    auto output = [ctxt](napi_env env, napi_value *result) -> napi_status { return napi_ok; };
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        privateCommandQueue_.Wait(ctxt->info);
        int32_t code = ImeSystemCmdChannel::GetInstance()->SendPrivateCommand(ctxt->privateCommand);
        privateCommandQueue_.Pop();
        if (code == ErrorCode::NO_ERROR) {
            ctxt->SetState(napi_ok);
        } else {
            ctxt->SetErrorCode(code);
        }
    };
    ctxt->SetAction(std::move(input), std::move(output));
    // 1 means JsAPI:SendPrivateCommand has 1 params at most.
    AsyncCall asyncCall(env, info, ctxt, 1);
    return asyncCall.Call(env, exec, "SendPrivateCommand");
}

napi_value JsKeyboardPanelManager::GetDefaultInputMethod(napi_env env, napi_callback_info info)
{
    std::shared_ptr<Property> property;
    int32_t ret = ImeSystemCmdChannel::GetInstance()->GetDefaultImeCfg(property);
    if (ret != ErrorCode::NO_ERROR || property == nullptr) {
        IMSA_HILOGE("GetDefaultImeCfg failed or property is nullptr ret: %{public}d", ret);
        return nullptr;
    }
    return GetJsInputMethodProperty(env, *property);
}

napi_value JsKeyboardPanelManager::GetJsInputMethodProperty(napi_env env, const Property &property)
{
    napi_value obj = nullptr;
    napi_create_object(env, &obj);

    auto ret = JsUtil::Object::WriteProperty(env, obj, "packageName", property.name);
    ret = ret && JsUtil::Object::WriteProperty(env, obj, "name", property.name);
    ret = ret && JsUtil::Object::WriteProperty(env, obj, "methodId", property.id);
    ret = ret && JsUtil::Object::WriteProperty(env, obj, "id", property.id);
    ret = ret && JsUtil::Object::WriteProperty(env, obj, "icon", property.icon);
    ret = ret && JsUtil::Object::WriteProperty(env, obj, "iconId", property.iconId);
    ret = ret && JsUtil::Object::WriteProperty(env, obj, "label", property.label);
    ret = ret && JsUtil::Object::WriteProperty(env, obj, "labelId", property.labelId);
    if (!ret) {
        IMSA_HILOGE("init module inputMethod.Panel.PanelType failed, ret: %{public}d", ret);
    }
    return obj;
}

void JsKeyboardPanelManager::ReceivePrivateCommand(
    const std::unordered_map<std::string, PrivateDataValue> &privateCommand)
{
    IMSA_HILOGD("JsKeyboardPanelManager, run in");
    std::string type = "panelPrivateCommand";
    auto entry = GetEntry(type, [&privateCommand](UvEntry &entry) { entry.privateCommand = privateCommand; });
    if (entry == nullptr) {
        return;
    }
    auto eventHandler = GetEventHandler();
    if (eventHandler == nullptr) {
        IMSA_HILOGE("eventHandler is nullptr!");
        return;
    }
    auto task = [entry]() {
        auto paramGetter = [entry](napi_env env, napi_value *args, uint8_t argc) -> bool {
            if (argc < 1) {
                return false;
            }
            napi_value jsObject = JsUtils::GetJsPrivateCommand(env, entry->privateCommand);
            if (jsObject == nullptr) {
                IMSA_HILOGE("GetJsPrivateCommand failed: jsObject is nullptr");
                return false;
            }
            // 0 means the first param of callback.
            args[0] = { jsObject };
            return true;
        };
        // 1 means callback has 1 params.
        JsCallbackHandler::Traverse(entry->vecCopy, { 1, paramGetter });
    };
    eventHandler->PostTask(task, type, 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

void JsKeyboardPanelManager::NotifyPanelStatus(const SysPanelStatus &sysPanelStatus)
{
    IMSA_HILOGD("JsKeyboardPanelManager, run in");
    std::string type = "isPanelShow";
    auto entry =
        GetEntry(type, [sysPanelStatus](UvEntry &entry) { entry.sysPanelStatus = sysPanelStatus; });
    if (entry == nullptr) {
        return;
    }
    auto eventHandler = GetEventHandler();
    if (eventHandler == nullptr) {
        IMSA_HILOGE("eventHandler is nullptr!");
        return;
    }
    auto task = [entry]() {
        auto paramGetter = [entry](napi_env env, napi_value *args, uint8_t argc) -> bool {
            if (argc < 1) {
                return false;
            }
            napi_value jsObject = JsPanelStatus::Write(env, entry->sysPanelStatus);
            if (jsObject == nullptr) {
                IMSA_HILOGE("GetJsSysPanelStatus failed: jsObject is nullptr");
                return false;
            }
            // 0 means the first param of callback.
            args[0] = { jsObject };
            return true;
        };
        // 1 means callback has 1 params.
        JsCallbackHandler::Traverse(entry->vecCopy, { 1, paramGetter });
    };
    eventHandler->PostTask(task, type, 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

std::shared_ptr<AppExecFwk::EventHandler> JsKeyboardPanelManager::GetEventHandler()
{
    if (handler_ == nullptr) {
        std::lock_guard<std::mutex> lock(eventHandlerMutex_);
        if (handler_ == nullptr) {
            handler_ = AppExecFwk::EventHandler::Current();
        }
    }
    return handler_;
}

std::shared_ptr<JsKeyboardPanelManager::UvEntry> JsKeyboardPanelManager::GetEntry(
    const std::string &type, EntrySetter entrySetter)
{
    IMSA_HILOGD("type: %{public}s", type.c_str());
    std::shared_ptr<UvEntry> entry = nullptr;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        if (jsCbMap_[type].empty()) {
            IMSA_HILOGD("%{public}s cb-vector is empty", type.c_str());
            return nullptr;
        }
        entry = std::make_shared<UvEntry>(jsCbMap_[type], type);
    }
    if (entrySetter != nullptr) {
        entrySetter(*entry);
    }
    return entry;
}

napi_value JsPanelStatus::Write(napi_env env, const SysPanelStatus &in)
{
    napi_value jsObject = nullptr;
    napi_create_object(env, &jsObject);
    bool ret = JsUtil::Object::WriteProperty(env, jsObject, "isSecurity", in.isSecurity);
    ret = ret && JsUtil::Object::WriteProperty(env, jsObject, "flag", in.flag);
    ret = ret && JsUtil::Object::WriteProperty(env, jsObject, "width", in.width);
    ret = ret && JsUtil::Object::WriteProperty(env, jsObject, "height", in.height);
    return ret ? jsObject : JsUtil::Const::Null(env);
}
} // namespace MiscServices
} // namespace OHOS
