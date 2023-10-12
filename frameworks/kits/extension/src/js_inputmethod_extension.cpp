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

#include "ability_info.h"
#include "global.h"
#include "input_method_ability.h"
#include "inputmethod_trace.h"
#include "js_inputmethod_extension_context.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_common_util.h"
#include "napi_common_want.h"
#include "napi_remote_object.h"

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
    IMSA_HILOGI("AttachInputMethodExtensionContext");
    if (value == nullptr) {
        IMSA_HILOGW("invalid parameter.");
        return nullptr;
    }
    auto ptr = reinterpret_cast<std::weak_ptr<InputMethodExtensionContext> *>(value)->lock();
    if (ptr == nullptr) {
        IMSA_HILOGW("invalid context.");
        return nullptr;
    }
    napi_value object  = CreateJsInputMethodExtensionContext(env, ptr);
    auto contextObj = JsRuntime::LoadSystemModuleByEngine(
        env, "InputMethodExtensionContext", &object, 1)->GetNapiValue();
    napi_coerce_to_native_binding_object(
        env, contextObj, DetachCallbackFunc, AttachInputMethodExtensionContext, value, nullptr);
    auto workContext = new (std::nothrow) std::weak_ptr<InputMethodExtensionContext>(ptr);
    napi_wrap(env, contextObj, workContext,
        [](napi_env, void *data, void *) {
            IMSA_HILOGI("Finalizer for weak_ptr input method extension context is called");
            delete static_cast<std::weak_ptr<InputMethodExtensionContext> *>(data);
        }, nullptr, nullptr);
    return object;
}

JsInputMethodExtension *JsInputMethodExtension::Create(const std::unique_ptr<Runtime> &runtime)
{
    IMSA_HILOGI("JsInputMethodExtension Create");
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
    IMSA_HILOGI("JsInputMethodExtension Init");
    InputMethodExtension::Init(record, application, handler, token);
    std::string srcPath;
    GetSrcPath(srcPath);
    if (srcPath.empty()) {
        IMSA_HILOGE("Failed to get srcPath");
        return;
    }

    std::string moduleName(Extension::abilityInfo_->moduleName);
    moduleName.append("::").append(abilityInfo_->name);
    IMSA_HILOGI(
        "JsInputMethodExtension::Init module:%{public}s,srcPath:%{public}s.", moduleName.c_str(), srcPath.c_str());
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    jsObj_ = jsRuntime_.LoadModule(
        moduleName, srcPath, abilityInfo_->hapPath, abilityInfo_->compileMode == CompileMode::ES_MODULE);
    if (jsObj_ == nullptr) {
        IMSA_HILOGE("Failed to get jsObj_");
        return;
    }
    IMSA_HILOGI("JsInputMethodExtension::Init GetNapiValue.");
    napi_value obj = jsObj_->GetNapiValue();
    if (obj == nullptr) {
        IMSA_HILOGE("Failed to get JsInputMethodExtension object");
        return;
    }
    BindContext(env, obj);
    IMSA_HILOGI("JsInputMethodExtension::Init end.");
}

void JsInputMethodExtension::BindContext(napi_env env, napi_value obj)
{
    IMSA_HILOGI("JsInputMethodExtension::BindContext");
    auto context = GetContext();
    if (context == nullptr) {
        IMSA_HILOGE("Failed to get context");
        return;
    }
    IMSA_HILOGI("JsInputMethodExtension::Init CreateJsInputMethodExtensionContext.");
    napi_value contextObj = CreateJsInputMethodExtensionContext(env, context);
    auto shellContextRef = jsRuntime_.LoadSystemModule("InputMethodExtensionContext", &contextObj, ARGC_ONE);
    contextObj = shellContextRef->GetNapiValue();
    if (contextObj == nullptr) {
        IMSA_HILOGE("Failed to get input method extension native object");
        return;
    }
    auto workContext = new (std::nothrow) std::weak_ptr<InputMethodExtensionContext>(context);
    napi_coerce_to_native_binding_object(
        env, contextObj, DetachCallbackFunc, AttachInputMethodExtensionContext, workContext, nullptr);
    IMSA_HILOGI("JsInputMethodExtension::Init Bind.");
    context->Bind(jsRuntime_, shellContextRef.release());
    IMSA_HILOGI("JsInputMethodExtension::SetProperty.");
    napi_set_named_property(env, obj, "context", contextObj);
    napi_wrap(env, contextObj, workContext,
        [](napi_env, void *data, void *) {
            IMSA_HILOGI("Finalizer for weak_ptr input method extension context is called");
            delete static_cast<std::weak_ptr<InputMethodExtensionContext> *>(data);
        }, nullptr, nullptr);
}

void JsInputMethodExtension::OnStart(const AAFwk::Want &want)
{
    StartAsync("OnStart", static_cast<int32_t>(TraceTaskId::ONSTART_EXTENSION));
    StartAsync("Extension::OnStart", static_cast<int32_t>(TraceTaskId::ONSTART_MIDDLE_EXTENSION));
    Extension::OnStart(want);
    FinishAsync("Extension::OnStart", static_cast<int32_t>(TraceTaskId::ONSTART_MIDDLE_EXTENSION));
    IMSA_HILOGI("JsInputMethodExtension OnStart begin..");
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    napi_value argv[] = { napiWant };
    StartAsync("onCreate", static_cast<int32_t>(TraceTaskId::ONCREATE_EXTENSION));
    CallObjectMethod("onCreate", argv, ARGC_ONE);
    FinishAsync("onCreate", static_cast<int32_t>(TraceTaskId::ONCREATE_EXTENSION));
    auto ret = InputMethodAbility::GetInstance()->SetCoreAndAgent();
    IMSA_HILOGI("ime bind imf ret: %{public}d", ret);
    FinishAsync("OnStart", static_cast<int32_t>(TraceTaskId::ONSTART_EXTENSION));
}

void JsInputMethodExtension::OnStop()
{
    InputMethodExtension::OnStop();
    IMSA_HILOGI("JsInputMethodExtension OnStop begin.");
    CallObjectMethod("onDestroy");
    bool ret = ConnectionManager::GetInstance().DisconnectCaller(GetContext()->GetToken());
    if (ret) {
        IMSA_HILOGI("The input method extension connection is not disconnected.");
    }
    IMSA_HILOGI("JsInputMethodExtension %{public}s end.", __func__);
}

sptr<IRemoteObject> JsInputMethodExtension::OnConnect(const AAFwk::Want &want)
{
    IMSA_HILOGI("JsInputMethodExtension OnConnect begin.");
    StartAsync("OnConnect", static_cast<int32_t>(TraceTaskId::ONCONNECT_EXTENSION));
    StartAsync("Extension::OnConnect", static_cast<int32_t>(TraceTaskId::ONCONNECT_MIDDLE_EXTENSION));
    Extension::OnConnect(want);
    FinishAsync("Extension::OnConnect", static_cast<int32_t>(TraceTaskId::ONCONNECT_MIDDLE_EXTENSION));
    IMSA_HILOGI("%{public}s begin.", __func__);
    HandleScope handleScope(jsRuntime_);
    napi_env env = (napi_env)jsRuntime_.GetNativeEnginePointer();
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    napi_value argv[] = { napiWant };
    if (jsObj_ == nullptr) {
        IMSA_HILOGE("Not found InputMethodExtension.js");
        return nullptr;
    }

    napi_value obj = jsObj_->GetNapiValue();
    if (obj == nullptr) {
        IMSA_HILOGE("Failed to get InputMethodExtension object");
        return nullptr;
    }
    napi_value method = nullptr;
    napi_get_named_property(env, obj, "onConnect", &method);
    if (method == nullptr) {
        IMSA_HILOGE("Failed to get onConnect from InputMethodExtension object");
        return nullptr;
    }
    IMSA_HILOGI("JsInputMethodExtension::CallFunction onConnect, success");
    napi_value remoteNapi = nullptr;
    napi_call_function(env, obj, method, ARGC_ONE, argv, &remoteNapi);
    if (remoteNapi == nullptr) {
        IMSA_HILOGE("remoteNative nullptr.");
    }
    auto remoteObj = NAPI_ohos_rpc_getNativeRemoteObject(env, remoteNapi);
    if (remoteObj == nullptr) {
        IMSA_HILOGE("remoteObj nullptr.");
    }
    FinishAsync("OnConnect", static_cast<int32_t>(TraceTaskId::ONCONNECT_EXTENSION));
    return remoteObj;
}

void JsInputMethodExtension::OnDisconnect(const AAFwk::Want &want)
{
    IMSA_HILOGI("JsInputMethodExtension OnDisconnect begin.");
    Extension::OnDisconnect(want);
    IMSA_HILOGI("%{public}s begin.", __func__);
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    napi_value argv[] = { napiWant };
    if (jsObj_ == nullptr) {
        IMSA_HILOGE("Not found InputMethodExtension.js");
        return;
    }

    napi_value obj = jsObj_->GetNapiValue();
    if (obj == nullptr) {
        IMSA_HILOGE("Failed to get InputMethodExtension object");
        return;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, "onDisconnect", &method);
    if (method == nullptr) {
        IMSA_HILOGE("Failed to get onDisconnect from InputMethodExtension object");
        return;
    }
    napi_value remoteNapi = nullptr;
    napi_call_function(env, obj, method, ARGC_ONE, argv, &remoteNapi);
    IMSA_HILOGI("%{public}s end.", __func__);
}

void JsInputMethodExtension::OnCommand(const AAFwk::Want &want, bool restart, int startId)
{
    IMSA_HILOGI("JsInputMethodExtension OnCommand begin.");
    Extension::OnCommand(want, restart, startId);
    IMSA_HILOGI(
        "%{public}s begin restart=%{public}s,startId=%{public}d.", __func__, restart ? "true" : "false", startId);
    HandleScope handleScope(jsRuntime_);
    napi_env env =  jsRuntime_.GetNapiEnv();
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    napi_value napiStartId = nullptr;
    napi_create_int32(env, startId, &napiStartId);
    napi_value argv[] = { napiWant, napiStartId };
    CallObjectMethod("onRequest", argv, ARGC_TWO);
    IMSA_HILOGI("%{public}s end.", __func__);
}

napi_value JsInputMethodExtension::CallObjectMethod(const char *name, const napi_value *argv, size_t argc)
{
    IMSA_HILOGI("JsInputMethodExtension::CallObjectMethod(%{public}s), begin", name);

    if (jsObj_ == nullptr) {
        IMSA_HILOGW("Not found InputMethodExtension.js");
        return nullptr;
    }

    HandleScope handleScope(jsRuntime_);
    napi_env env =  jsRuntime_.GetNapiEnv();
    napi_value obj = jsObj_->GetNapiValue();
    if (obj == nullptr) {
        IMSA_HILOGE("Failed to get InputMethodExtension object");
        return nullptr;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, name, &method);
    if (method == nullptr) {
        IMSA_HILOGE("Failed to get '%{public}s' from InputMethodExtension object", name);
        return nullptr;
    }
    IMSA_HILOGI("JsInputMethodExtension::CallFunction(%{public}s), success", name);
    napi_value remoteNapi = nullptr;
    napi_status status = napi_call_function(env, obj, method, argc, argv, &remoteNapi);
    if (status != napi_ok) {
        return nullptr;
    }
    return remoteNapi;
}

void JsInputMethodExtension::GetSrcPath(std::string &srcPath)
{
    IMSA_HILOGI("JsInputMethodExtension GetSrcPath begin.");
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
} // namespace AbilityRuntime
} // namespace OHOS
