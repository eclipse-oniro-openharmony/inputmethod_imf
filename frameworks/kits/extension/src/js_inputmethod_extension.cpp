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

#include "js_inputmethod_extension.h"

#include "ability_handler.h"
#include "ability_info.h"
#include "configuration_utils.h"
#include "global.h"
#include "input_method_ability.h"
#include "inputmethod_extension_ability_stub.h"
#include "inputmethod_trace.h"
#include "iservice_registry.h"
#include "js_extension_context.h"
#include "js_inputmethod_extension_context.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_common_util.h"
#include "napi_common_want.h"
#include "napi_remote_object.h"
#include "system_ability_definition.h"
#include "tasks/task_ams.h"
#include "tasks/task_imsa.h"
#include "task_manager.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
} // namespace
JsInputMethodExtension *JsInputMethodExtension::jsInputMethodExtension = nullptr;
using namespace OHOS::AppExecFwk;
using namespace OHOS::MiscServices;

napi_value AttachInputMethodExtensionContext(napi_env env, void *value, void *)
{
    IMSA_HILOGI("AttachInputMethodExtensionContext start.");
    if (value == nullptr) {
        IMSA_HILOGW("parameter is invalid.");
        return nullptr;
    }
    auto ptr = reinterpret_cast<std::weak_ptr<InputMethodExtensionContext> *>(value)->lock();
    if (ptr == nullptr) {
        IMSA_HILOGW("context is invalid.");
        return nullptr;
    }
    napi_value object = CreateJsInputMethodExtensionContext(env, ptr);
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(env, "InputMethodExtensionContext", &object, 1);
    if (systemModule == nullptr) {
        IMSA_HILOGE("failed to load system module by engine!");
        return nullptr;
    }
    auto contextObj = systemModule ->GetNapiValue();
    napi_coerce_to_native_binding_object(env, contextObj, DetachCallbackFunc, AttachInputMethodExtensionContext, value,
        nullptr);
    auto workContext = new (std::nothrow) std::weak_ptr<InputMethodExtensionContext>(ptr);
    if (workContext == nullptr) {
        IMSA_HILOGE("workContext is nullptr!");
        return nullptr;
    }
    napi_status status = napi_wrap(
        env, contextObj, workContext,
        [](napi_env, void *data, void *) {
            IMSA_HILOGI("finalizer for weak_ptr input method extension context is called.");
            delete static_cast<std::weak_ptr<InputMethodExtensionContext> *>(data);
        },
        nullptr, nullptr);
    if (status != napi_ok) {
        IMSA_HILOGE("InputMethodExtensionContext wrap failed: %{public}d!", status);
        delete workContext;
        return nullptr;
    }
    return object;
}

JsInputMethodExtension *JsInputMethodExtension::Create(const std::unique_ptr<Runtime> &runtime)
{
    IMSA_HILOGI("JsInputMethodExtension Create.");
    jsInputMethodExtension = new JsInputMethodExtension(static_cast<JsRuntime &>(*runtime));
    return jsInputMethodExtension;
}

JsInputMethodExtension::JsInputMethodExtension(JsRuntime &jsRuntime) : jsRuntime_(jsRuntime)
{
}

JsInputMethodExtension::~JsInputMethodExtension()
{
    jsRuntime_.FreeNativeReference(std::move(jsObj_));
}

void JsInputMethodExtension::Init(const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    IMSA_HILOGI("JsInputMethodExtension Init.");
    InputMethodExtension::Init(record, application, handler, token);
    std::string srcPath;
    GetSrcPath(srcPath);
    if (srcPath.empty()) {
        IMSA_HILOGE("failed to get srcPath!");
        return;
    }

    std::string moduleName(Extension::abilityInfo_->moduleName);
    moduleName.append("::").append(abilityInfo_->name);
    IMSA_HILOGI("JsInputMethodExtension, module: %{public}s, srcPath:%{public}s.", moduleName.c_str(), srcPath.c_str());
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    jsObj_ = jsRuntime_.LoadModule(moduleName, srcPath, abilityInfo_->hapPath,
        abilityInfo_->compileMode == CompileMode::ES_MODULE);
    if (jsObj_ == nullptr) {
        IMSA_HILOGE("failed to get jsObj_!");
        return;
    }
    IMSA_HILOGI("JsInputMethodExtension::Init GetNapiValue.");
    napi_value obj = jsObj_->GetNapiValue();
    if (obj == nullptr) {
        IMSA_HILOGE("failed to get JsInputMethodExtension object!");
        return;
    }
    BindContext(env, obj);
    handler_ = handler;
    ListenWindowManager();
    IMSA_HILOGI("JsInputMethodExtension end.");
}

void JsInputMethodExtension::ListenWindowManager()
{
    IMSA_HILOGD("register window manager service listener.");
    auto abilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (abilityManager == nullptr) {
        IMSA_HILOGE("failed to get SaMgr!");
        return;
    }

    auto jsInputMethodExtension = std::static_pointer_cast<JsInputMethodExtension>(shared_from_this());
    displayListener_ = sptr<JsInputMethodExtensionDisplayListener>::MakeSptr(jsInputMethodExtension);
    if (displayListener_ == nullptr) {
        IMSA_HILOGE("failed to create display listener!");
        return;
    }

    auto listener = sptr<SystemAbilityStatusChangeListener>::MakeSptr(displayListener_);
    if (listener == nullptr) {
        IMSA_HILOGE("failed to create status change listener!");
        return;
    }

    auto ret = abilityManager->SubscribeSystemAbility(WINDOW_MANAGER_SERVICE_ID, listener);
    if (ret != 0) {
        IMSA_HILOGE("failed to subscribe system ability, ret: %{public}d!", ret);
    }
}

void JsInputMethodExtension::OnConfigurationUpdated(const AppExecFwk::Configuration &config)
{
    InputMethodExtension::OnConfigurationUpdated(config);
    IMSA_HILOGD("called.");
    auto context = GetContext();
    if (context == nullptr) {
        IMSA_HILOGE("context is invalid!");
        return;
    }

    auto contextConfig = context->GetConfiguration();
    if (contextConfig != nullptr) {
        std::vector<std::string> changeKeyValue;
        contextConfig->CompareDifferent(changeKeyValue, config);
        if (!changeKeyValue.empty()) {
            contextConfig->Merge(changeKeyValue, config);
        }
        IMSA_HILOGD("config dump merge: %{public}s.", contextConfig->GetName().c_str());
    }
    ConfigurationUpdated();
}

void JsInputMethodExtension::ConfigurationUpdated()
{
    IMSA_HILOGD("called.");
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();

    // Notify extension context
    auto context = GetContext();
    if (context == nullptr) {
        IMSA_HILOGE("context is nullptr!");
        return;
    }
    auto fullConfig = context->GetConfiguration();
    if (fullConfig == nullptr) {
        IMSA_HILOGE("configuration is nullptr!");
        return;
    }

    JsExtensionContext::ConfigurationUpdated(env, shellContextRef_, fullConfig);
}

void JsInputMethodExtension::SystemAbilityStatusChangeListener::OnAddSystemAbility(int32_t systemAbilityId,
    const std::string &deviceId)
{
    IMSA_HILOGD("add systemAbilityId: %{public}d.", systemAbilityId);
    if (systemAbilityId == WINDOW_MANAGER_SERVICE_ID) {
        Rosen::DisplayManager::GetInstance().RegisterDisplayListener(listener_);
    }
}

void JsInputMethodExtension::BindContext(napi_env env, napi_value obj)
{
    IMSA_HILOGI("JsInputMethodExtension::BindContext");
    auto context = GetContext();
    if (context == nullptr) {
        IMSA_HILOGE("failed to get context!");
        return;
    }
    IMSA_HILOGD("JsInputMethodExtension::Init CreateJsInputMethodExtensionContext.");
    napi_value contextObj = CreateJsInputMethodExtensionContext(env, context);
    auto shellContextRef = jsRuntime_.LoadSystemModule("InputMethodExtensionContext", &contextObj, ARGC_ONE);
    if (shellContextRef == nullptr) {
        IMSA_HILOGE("shellContextRef is nullptr!");
        return;
    }
    contextObj = shellContextRef->GetNapiValue();
    if (contextObj == nullptr) {
        IMSA_HILOGE("failed to get input method extension native object!");
        return;
    }
    auto workContext = new (std::nothrow) std::weak_ptr<InputMethodExtensionContext>(context);
    if (workContext == nullptr) {
        IMSA_HILOGE("workContext is nullptr!");
        return;
    }
    napi_coerce_to_native_binding_object(env, contextObj, DetachCallbackFunc, AttachInputMethodExtensionContext,
        workContext, nullptr);
    IMSA_HILOGD("JsInputMethodExtension::Init Bind.");
    context->Bind(jsRuntime_, shellContextRef.release());
    IMSA_HILOGD("JsInputMethodExtension::SetProperty.");
    napi_set_named_property(env, obj, "context", contextObj);
    napi_status status = napi_wrap(
        env, contextObj, workContext,
        [](napi_env, void *data, void *) {
            IMSA_HILOGI("Finalizer for weak_ptr input method extension context is called.");
            delete static_cast<std::weak_ptr<InputMethodExtensionContext> *>(data);
        },
        nullptr, nullptr);
    if (status != napi_ok) {
        IMSA_HILOGE("InputMethodExtensionContext wrap failed: %{public}d", status);
        delete workContext;
    }
}

void JsInputMethodExtension::OnStart(const AAFwk::Want &want)
{
    auto task = std::make_shared<TaskAmsInit>();
    TaskManager::GetInstance().PostTask(task);
    auto inputMethodAbility = InputMethodAbility::GetInstance();
    if (inputMethodAbility != nullptr) {
        inputMethodAbility->InitConnect();
    }
    StartAsync("OnStart", static_cast<int32_t>(TraceTaskId::ONSTART_EXTENSION));
    StartAsync("Extension::OnStart", static_cast<int32_t>(TraceTaskId::ONSTART_MIDDLE_EXTENSION));
    Extension::OnStart(want);
    FinishAsync("Extension::OnStart", static_cast<int32_t>(TraceTaskId::ONSTART_MIDDLE_EXTENSION));
    IMSA_HILOGI("JsInputMethodExtension OnStart begin.");
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    napi_value argv[] = { napiWant };
    StartAsync("onCreate", static_cast<int32_t>(TraceTaskId::ONCREATE_EXTENSION));
    CallObjectMethod("onCreate", argv, ARGC_ONE);
    FinishAsync("onCreate", static_cast<int32_t>(TraceTaskId::ONCREATE_EXTENSION));
    TaskManager::GetInstance().PostTask(std::make_shared<TaskImsaSetCoreAndAgent>());
    IMSA_HILOGI("ime bind imf");
    FinishAsync("OnStart", static_cast<int32_t>(TraceTaskId::ONSTART_EXTENSION));

    TaskManager::GetInstance().Complete(task->GetSeqId());
}

void JsInputMethodExtension::OnStop()
{
    InputMethodExtension::OnStop();
    IMSA_HILOGI("JsInputMethodExtension OnStop start.");
    CallObjectMethod("onDestroy");
    bool ret = ConnectionManager::GetInstance().DisconnectCaller(GetContext()->GetToken());
    if (ret) {
        IMSA_HILOGI("the input method extension connection is not disconnected.");
    }
    IMSA_HILOGI("JsInputMethodExtension %{public}s end.", __func__);
}

sptr<IRemoteObject> JsInputMethodExtension::OnConnect(const AAFwk::Want &want)
{
    IMSA_HILOGI("JsInputMethodExtension OnConnect start.");
    Extension::OnConnect(want);
    auto remoteObj = new (std::nothrow) InputMethodExtensionAbilityStub();
    if (remoteObj == nullptr) {
        IMSA_HILOGE("failed to create InputMethodExtensionAbilityStub!");
        return nullptr;
    }
    return remoteObj;
}

void JsInputMethodExtension::OnDisconnect(const AAFwk::Want &want)
{
    IMSA_HILOGI("JsInputMethodExtension OnDisconnect start.");
    Extension::OnDisconnect(want);
    IMSA_HILOGI("%{public}s start.", __func__);
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    napi_value argv[] = { napiWant };
    if (jsObj_ == nullptr) {
        IMSA_HILOGE("not found InputMethodExtension.js!");
        return;
    }

    napi_value obj = jsObj_->GetNapiValue();
    if (obj == nullptr) {
        IMSA_HILOGE("failed to get InputMethodExtension object!");
        return;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, "onDisconnect", &method);
    if (method == nullptr) {
        IMSA_HILOGE("failed to get onDisconnect from InputMethodExtension object!");
        return;
    }
    napi_value remoteNapi = nullptr;
    napi_call_function(env, obj, method, ARGC_ONE, argv, &remoteNapi);
    IMSA_HILOGI("%{public}s end.", __func__);
}

void JsInputMethodExtension::OnCommand(const AAFwk::Want &want, bool restart, int startId)
{
    IMSA_HILOGI("JsInputMethodExtension OnCommand start.");
    Extension::OnCommand(want, restart, startId);
    IMSA_HILOGI("%{public}s start restart=%{public}s,startId=%{public}d.", __func__, restart ? "true" : "false",
        startId);
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    napi_value napiStartId = nullptr;
    napi_create_int32(env, startId, &napiStartId);
    napi_value argv[] = { napiWant, napiStartId };
    CallObjectMethod("onRequest", argv, ARGC_TWO);
    IMSA_HILOGI("%{public}s end.", __func__);
}

napi_value JsInputMethodExtension::CallObjectMethod(const char *name, const napi_value *argv, size_t argc)
{
    IMSA_HILOGI("JsInputMethodExtension::CallObjectMethod(%{public}s), start.", name);

    if (jsObj_ == nullptr) {
        IMSA_HILOGW("not found InputMethodExtension.js.");
        return nullptr;
    }

    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value obj = jsObj_->GetNapiValue();
    if (obj == nullptr) {
        IMSA_HILOGE("failed to get InputMethodExtension object!");
        return nullptr;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, name, &method);
    if (method == nullptr) {
        IMSA_HILOGE("failed to get '%{public}s' from InputMethodExtension object!", name);
        return nullptr;
    }
    IMSA_HILOGI("JsInputMethodExtension::CallFunction(%{public}s), success.", name);
    napi_value remoteNapi = nullptr;
    napi_status status = napi_call_function(env, obj, method, argc, argv, &remoteNapi);
    if (status != napi_ok) {
        return nullptr;
    }
    return remoteNapi;
}

void JsInputMethodExtension::GetSrcPath(std::string &srcPath)
{
    IMSA_HILOGD("JsInputMethodExtension GetSrcPath start.");
    if (!Extension::abilityInfo_->isModuleJson) {
        /* temporary compatibility api8 + config.json */
        srcPath.append(Extension::abilityInfo_->package);
        srcPath.append("/assets/js/");
        if (!Extension::abilityInfo_->srcPath.empty()) {
            srcPath.append(Extension::abilityInfo_->srcPath);
        }
        srcPath.append("/").append(Extension::abilityInfo_->name).append(".abc");
        return;
    }

    if (!Extension::abilityInfo_->srcEntrance.empty()) {
        srcPath.append(Extension::abilityInfo_->moduleName + "/");
        srcPath.append(Extension::abilityInfo_->srcEntrance);
        srcPath.erase(srcPath.rfind('.'));
        srcPath.append(".abc");
    }
}

void JsInputMethodExtension::OnCreate(Rosen::DisplayId displayId)
{
    IMSA_HILOGD("enter");
}

void JsInputMethodExtension::OnDestroy(Rosen::DisplayId displayId)
{
    IMSA_HILOGD("exit");
}

void JsInputMethodExtension::CheckNeedAdjustKeyboard(Rosen::DisplayId displayId)
{
    if (displayId != Rosen::DisplayManager::GetInstance().GetDefaultDisplayId()) {
        return;
    }
    auto displayPtr = Rosen::DisplayManager::GetInstance().GetPrimaryDisplaySync();
    if (displayPtr == nullptr) {
        return;
    }
    IMSA_HILOGD("display width: %{public}d, height: %{public}d, rotation: %{public}d",
        displayPtr->GetWidth(),
        displayPtr->GetHeight(),
        displayPtr->GetRotation());
    if (cacheDisplay_.IsEmpty()) {
        cacheDisplay_.SetCacheDisplay(displayPtr->GetWidth(), displayPtr->GetHeight(), displayPtr->GetRotation());
        return;
    }
    if ((cacheDisplay_.displayWidth != displayPtr->GetWidth() ||
        cacheDisplay_.displayHeight != displayPtr->GetHeight()) &&
        cacheDisplay_.displayRotation == displayPtr->GetRotation()) {
        TaskManager::GetInstance().PostTask(std::make_shared<TaskImsaAdjustKeyboard>());
    }
    cacheDisplay_.SetCacheDisplay(displayPtr->GetWidth(), displayPtr->GetHeight(), displayPtr->GetRotation());
}

void JsInputMethodExtension::OnChange(Rosen::DisplayId displayId)
{
    IMSA_HILOGD("displayId: %{public}" PRIu64 "", displayId);
    auto context = GetContext();
    if (context == nullptr) {
        IMSA_HILOGE("context is invalid!");
        return;
    }

    auto contextConfig = context->GetConfiguration();
    if (contextConfig == nullptr) {
        IMSA_HILOGE("configuration is invalid!");
        return;
    }
    bool isConfigChanged = false;
    auto configUtils = std::make_shared<ConfigurationUtils>();
    configUtils->UpdateDisplayConfig(displayId, contextConfig, context->GetResourceManager(), isConfigChanged);
    IMSA_HILOGD("OnChange, isConfigChanged: %{public}d, Config after update: %{public}s.", isConfigChanged,
        contextConfig->GetName().c_str());

    if (isConfigChanged) {
        // Only notify the display device changes related to the window created through the IMA
        bool needNotice = false;
        auto callingDisplayId = InputMethodAbility::GetInstance()->GetCallingWindowDisplayId();
        if (callingDisplayId == displayId) {
            needNotice = true;
            IMSA_HILOGE("check displayId diff.callingDisplayId:%{public}" PRIu64"", callingDisplayId);
            return;
        }
        if (!needNotice) {
            IMSA_HILOGD("OnChange, CheckHasPanelDisplayId.need:%{public}d, Config after update: %{public}s.",
                needNotice, contextConfig->GetName().c_str());
            return;
        }
        auto inputMethodExtension = std::static_pointer_cast<JsInputMethodExtension>(shared_from_this());
        auto task = [inputMethodExtension]() {
            if (inputMethodExtension) {
                inputMethodExtension->ConfigurationUpdated();
            }
        };
        if (handler_ != nullptr) {
            handler_->PostTask(task, "JsInputMethodExtension:OnChange", 0, AppExecFwk::EventQueue::Priority::VIP);
        }
    }
}
} // namespace AbilityRuntime
} // namespace OHOS
