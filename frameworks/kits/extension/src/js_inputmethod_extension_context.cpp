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

#include "js_inputmethod_extension_context.h"

#include <cstdint>

#include "global.h"
#include "js_data_struct_converter.h"
#include "js_error_utils.h"
#include "js_extension_context.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "js_util.h"
#include "js_utils.h"
#include "napi/native_api.h"
#include "napi_common_start_options.h"
#include "napi_common_util.h"
#include "napi_common_want.h"
#include "napi_remote_object.h"
#include "start_options.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::MiscServices;
namespace {
constexpr int32_t INDEX_ZERO = 0;
constexpr int32_t INDEX_ONE = 1;
constexpr int32_t INDEX_TWO = 2;
constexpr int32_t ERROR_CODE_ONE = 1;
constexpr int32_t ERROR_CODE_TWO = 2;
constexpr size_t ARGC_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr size_t ARGC_THREE = 3;
constexpr size_t ARGC_FOUR = 4;

class JsInputMethodExtensionContext final {
public:
    explicit JsInputMethodExtensionContext(const std::shared_ptr<InputMethodExtensionContext> &context)
        : context_(context)
    {
    }
    ~JsInputMethodExtensionContext() = default;
    JsInputMethodExtensionContext() = default;

    static void Finalizer(napi_env env, void *data, void *hint)
    {
        IMSA_HILOGI("JsInputMethodExtensionContext::Finalizer is called.");
        std::unique_ptr<JsInputMethodExtensionContext>(static_cast<JsInputMethodExtensionContext *>(data));
    }

    static napi_value StartAbility(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsInputMethodExtensionContext, OnStartAbility);
    }

    static napi_value StartAbilityWithAccount(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsInputMethodExtensionContext, OnStartAbilityWithAccount);
    }

    static napi_value ConnectAbilityWithAccount(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsInputMethodExtensionContext, OnConnectAbilityWithAccount);
    }

    static napi_value TerminateAbility(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsInputMethodExtensionContext, OnTerminateAbility);
    }

    static napi_value ConnectAbility(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsInputMethodExtensionContext, OnConnectAbility);
    }

    static napi_value DisconnectAbility(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsInputMethodExtensionContext, OnDisconnectAbility);
    }

private:
    std::weak_ptr<InputMethodExtensionContext> context_;

    napi_value OnStartAbility(napi_env env, size_t argc, napi_value *argv)
    {
        IMSA_HILOGI("InputMethodExtensionContext OnStartAbility.");
        // only support one or two or three params
        PARAM_CHECK_RETURN(env, argc == ARGC_ONE || argc == ARGC_TWO || argc == ARGC_THREE,
            "number of param should in [1,3]", TYPE_NONE, CreateJsUndefined(env));
        PARAM_CHECK_RETURN(env, JsUtil::GetType(env, argv[0]) == napi_object, "param want type must be Want",
            TYPE_NONE, JsUtil::Const::Null(env));
        decltype(argc) unwrapArgc = 0;
        AAFwk::Want want;
        OHOS::AppExecFwk::UnwrapWant(env, argv[INDEX_ZERO], want);
        IMSA_HILOGI("%{public}s bundleName: %{public}s abilityName: %{public}s.", __func__, want.GetBundle().c_str(),
            want.GetElement().GetAbilityName().c_str());
        unwrapArgc++;
        AAFwk::StartOptions startOptions;
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[INDEX_ONE], &valueType);
        if (argc > ARGC_ONE && valueType == napi_object) {
            IMSA_HILOGI("OnStartAbility start options is used.");
            AppExecFwk::UnwrapStartOptions(env, argv[INDEX_ONE], startOptions);
            unwrapArgc++;
        }
        napi_value lastParam = argc > unwrapArgc ? argv[unwrapArgc] : nullptr;
        napi_value result = nullptr;
        std::unique_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
        auto asyncTask = [weak = context_, want, startOptions, unwrapArgc, env, task = napiAsyncTask.get()]() {
            IMSA_HILOGI("startAbility start.");
            auto context = weak.lock();
            if (context == nullptr) {
                IMSA_HILOGW("context is released.");
                task->Reject(env, CreateJsError(env, ERROR_CODE_ONE, "Context is released"));
                delete task;
                return;
            }
            ErrCode errcode = ERR_OK;
            (unwrapArgc == 1) ? errcode = context->StartAbility(want)
                              : errcode = context->StartAbility(want, startOptions);
            if (errcode == 0) {
                task->Resolve(env, CreateJsUndefined(env));
            } else {
                task->Reject(env, CreateJsErrorByNativeErr(env, errcode));
            }
            delete task;
        };
        if (napi_send_event(env, asyncTask, napi_eprio_high) != napi_status::napi_ok) {
            napiAsyncTask->Reject(env, CreateJsError(env, ERROR_CODE_ONE, "send event failed"));
        } else {
            napiAsyncTask.release();
        }
        return result;
    }

    napi_value OnStartAbilityWithAccount(napi_env env, size_t argc, napi_value *argv)
    {
        // only support two or three or four params
        CHECK_RETURN(argc == ARGC_TWO || argc == ARGC_THREE || argc == ARGC_FOUR,
            "not enough params!", CreateJsUndefined(env));
        decltype(argc) unwrapArgc = 0;
        AAFwk::Want want;
        OHOS::AppExecFwk::UnwrapWant(env, argv[INDEX_ZERO], want);
        unwrapArgc++;
        int32_t accountId = 0;
        if (!OHOS::AppExecFwk::UnwrapInt32FromJS2(env, argv[INDEX_ONE], accountId)) {
            IMSA_HILOGI("%{public}s called, the second parameter is invalid.", __func__);
            return CreateJsUndefined(env);
        }
        IMSA_HILOGI("bundleName: %{public}s abilityName: %{public}s, accountId: %{public}d", want.GetBundle().c_str(),
            want.GetElement().GetAbilityName().c_str(), accountId);
        unwrapArgc++;
        AAFwk::StartOptions startOptions;
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[INDEX_ONE], &valueType);
        if (argc > ARGC_TWO && valueType == napi_object) {
            AppExecFwk::UnwrapStartOptions(env, argv[INDEX_TWO], startOptions);
            unwrapArgc++;
        }
        napi_value lastParam = argc == unwrapArgc ? nullptr : argv[unwrapArgc];
        napi_value result = nullptr;
        std::unique_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
        auto asyncTask = [weak = context_, want, accountId, startOptions, unwrapArgc, env,
            task = napiAsyncTask.get()]() {
            IMSA_HILOGI("startAbility start");
            auto context = weak.lock();
            if (context == nullptr) {
                task->Reject(env, CreateJsError(env, ERROR_CODE_ONE, "Context is released"));
                delete task;
                return;
            }
            ErrCode errcode = (unwrapArgc == ARGC_TWO)
                                  ? context->StartAbilityWithAccount(want, accountId)
                                  : context->StartAbilityWithAccount(want, accountId, startOptions);
            if (errcode == 0) {
                task->Resolve(env, CreateJsUndefined(env));
            }
            task->Reject(env, CreateJsError(env, errcode, "Start Ability failed."));
            delete task;
        };
        if (napi_send_event(env, asyncTask, napi_eprio_high) != napi_status::napi_ok) {
            napiAsyncTask->Reject(env, CreateJsError(env, ERROR_CODE_ONE, "send event failed"));
        } else {
            napiAsyncTask.release();
        }
        return result;
    }

    napi_value OnTerminateAbility(napi_env env, size_t argc, napi_value *argv)
    {
        IMSA_HILOGI("OnTerminateAbility is called.");
        // only support one or zero params
        if (argc != ARGC_ZERO && argc != ARGC_ONE) {
            IMSA_HILOGE("not enough params!");
            return CreateJsUndefined(env);
        }
        napi_value lastParam = argc == ARGC_ZERO ? nullptr : argv[INDEX_ZERO];
        napi_value result = nullptr;
        std::unique_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
        auto asyncTask = [weak = context_, env, task = napiAsyncTask.get()]() {
            IMSA_HILOGI("TerminateAbility start.");
            auto context = weak.lock();
            if (context == nullptr) {
                IMSA_HILOGW("context is released.");
                task->Reject(env, CreateJsError(env, ERROR_CODE_ONE, "Context is released"));
                delete task;
                return;
            }

            auto errcode = context->TerminateAbility();
            if (errcode == 0) {
                task->Resolve(env, CreateJsUndefined(env));
            } else {
                task->Reject(env, CreateJsError(env, errcode, "Terminate Ability failed."));
            }
            delete task;
        };
        if (napi_send_event(env, asyncTask, napi_eprio_high) != napi_status::napi_ok) {
            napiAsyncTask->Reject(env, CreateJsError(env, ERROR_CODE_ONE, "send event failed"));
        } else {
            napiAsyncTask.release();
        }
        return result;
    }

    napi_value OnConnectAbility(napi_env env, size_t argc, napi_value *argv)
    {
        IMSA_HILOGI("OnConnectAbility start.");
        // only support two params
        CHECK_RETURN(argc == ARGC_TWO, "not enough params!", CreateJsUndefined(env));
        AAFwk::Want want;
        OHOS::AppExecFwk::UnwrapWant(env, argv[INDEX_ZERO], want);
        IMSA_HILOGI("%{public}s bundleName: %{public}s abilityName: %{public}s", __func__, want.GetBundle().c_str(),
            want.GetElement().GetAbilityName().c_str());
        sptr<JSInputMethodExtensionConnection> connection = new JSInputMethodExtensionConnection(env);
        connection->SetJsConnectionObject(argv[1]);
        int64_t connectId = serialNumber_;
        ConnectionKey key;
        key.id = serialNumber_;
        key.want = want;
        {
            std::lock_guard<std::mutex> lock(g_connectMapMtx);
            connects_.emplace(key, connection);
        }
        if (serialNumber_ < INT64_MAX) {
            serialNumber_++;
        } else {
            serialNumber_ = 0;
        }
        napi_value result = nullptr;
        napi_value lastParam = nullptr;
        napi_value connectResult = nullptr;
        std::unique_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
        auto asyncTask = [weak = context_, want, connection, connectId, env, task = napiAsyncTask.get()]() {
            IMSA_HILOGI("OnConnectAbility start.");
            auto context = weak.lock();
            if (context == nullptr) {
                IMSA_HILOGW("context is released.");
                task->Reject(env, CreateJsError(env, ERROR_CODE_ONE, "Context is released"));
                delete task;
                return;
            }
            IMSA_HILOGI("context->ConnectAbility connection: %{public}d.", static_cast<int32_t>(connectId));
            if (!context->ConnectAbility(want, connection)) {
                connection->CallJsFailed(ERROR_CODE_ONE);
            }
            task->Resolve(env, CreateJsUndefined(env));
            delete task;
        };
        if (napi_send_event(env, asyncTask, napi_eprio_high) != napi_status::napi_ok) {
            napiAsyncTask->Reject(env, CreateJsError(env, ERROR_CODE_ONE, "send event failed"));
        } else {
            napiAsyncTask.release();
        }
        napi_create_int64(env, connectId, &connectResult);
        return connectResult;
    }

    void CheckSerialNumber(int64_t &serialNumber)
    {
        if (serialNumber < INT64_MAX) {
            serialNumber++;
        } else {
            serialNumber = 0;
        }
    }
    napi_value OnConnectAbilityWithAccount(napi_env env, size_t argc, napi_value *argv)
    {
        CHECK_RETURN(argc == ARGC_THREE, "not enough params!", CreateJsUndefined(env));
        AAFwk::Want want;
        OHOS::AppExecFwk::UnwrapWant(env, argv[INDEX_ZERO], want);
        IMSA_HILOGI("%{public}s bundleName: %{public}s, abilityName: %{public}s", __func__, want.GetBundle().c_str(),
            want.GetElement().GetAbilityName().c_str());
        int32_t accountId = 0;
        if (!OHOS::AppExecFwk::UnwrapInt32FromJS2(env, argv[INDEX_ONE], accountId)) {
            IMSA_HILOGI("%{public}s called, the second parameter is invalid.", __func__);
            return CreateJsUndefined(env);
        }
        sptr<JSInputMethodExtensionConnection> connection = new JSInputMethodExtensionConnection(env);
        connection->SetJsConnectionObject(argv[1]);
        int64_t connectId = serialNumber_;
        ConnectionKey key;
        key.id = serialNumber_;
        key.want = want;
        {
            std::lock_guard<std::mutex> lock(g_connectMapMtx);
            connects_.emplace(key, connection);
        }
        CheckSerialNumber(serialNumber_);
        napi_value result = nullptr;
        napi_value lastParam = nullptr;
        napi_value connectResult = nullptr;

        std::unique_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
        auto asyncTask = [weak = context_, want, accountId, connection, connectId, env, task = napiAsyncTask.get()]() {
            auto context = weak.lock();
            if (context == nullptr) {
                IMSA_HILOGW("context is released.");
                task->Reject(env, CreateJsError(env, ERROR_CODE_ONE, "Context is released"));
                delete task;
                return;
            }
            IMSA_HILOGI("context->ConnectAbilityWithAccount connection:%{public}d.", static_cast<int32_t>(connectId));
            if (!context->ConnectAbilityWithAccount(want, accountId, connection)) {
                connection->CallJsFailed(ERROR_CODE_ONE);
            }
            task->Resolve(env, CreateJsUndefined(env));
            delete task;
        };
        if (napi_send_event(env, asyncTask, napi_eprio_high) != napi_status::napi_ok) {
            napiAsyncTask->Reject(env, CreateJsError(env, ERROR_CODE_ONE, "send event failed"));
        } else {
            napiAsyncTask.release();
        }
        napi_create_int64(env, connectId, &connectResult);
        return connectResult;
    }

    napi_value OnDisconnectAbility(napi_env env, size_t argc, napi_value *argv)
    {
        IMSA_HILOGI("OnDisconnectAbility is called.");
        // only support one or two params
        CHECK_RETURN(argc == ARGC_ONE || argc == ARGC_TWO, "not enough params!", CreateJsUndefined(env));
        AAFwk::Want want;
        int64_t connectId = -1;
        sptr<JSInputMethodExtensionConnection> connection = nullptr;
        napi_get_value_int64(env, argv[INDEX_ZERO], &connectId);
        IMSA_HILOGI("OnDisconnectAbility connection: %{public}d.", static_cast<int32_t>(connectId));
        {
            std::lock_guard<std::mutex> lock(g_connectMapMtx);
            auto item = std::find_if(connects_.begin(), connects_.end(),
                [&connectId](const std::map<ConnectionKey, sptr<JSInputMethodExtensionConnection>>::value_type &obj) {
                    return connectId == obj.first.id;
                });
            if (item != connects_.end()) {
                // match id
                want = item->first.want;
                connection = item->second;
            }
        }
        // begin disconnect
        napi_value lastParam = argc == ARGC_ONE ? nullptr : argv[INDEX_ONE];
        napi_value result = nullptr;
        std::unique_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
        auto asyncTask = [weak = context_, want, connection, env, task = napiAsyncTask.get()]() {
            IMSA_HILOGI("OnDisconnectAbility start.");
            auto context = weak.lock();
            if (context == nullptr) {
                IMSA_HILOGW("context is released.");
                task->Reject(env, CreateJsError(env, ERROR_CODE_ONE, "Context is released"));
                delete task;
                return;
            }
            if (connection == nullptr) {
                IMSA_HILOGW("connection is nullptr.");
                task->Reject(env, CreateJsError(env, ERROR_CODE_TWO, "not found connection"));
                delete task;
                return;
            }
            IMSA_HILOGI("context->DisconnectAbility.");
            auto errcode = context->DisconnectAbility(want, connection);
            errcode == 0 ? task->Resolve(env, CreateJsUndefined(env))
                         : task->Reject(env, CreateJsError(env, errcode, "Disconnect Ability failed."));
            delete task;
        };
        if (napi_send_event(env, asyncTask, napi_eprio_high) != napi_status::napi_ok) {
            napiAsyncTask->Reject(env, CreateJsError(env, ERROR_CODE_ONE, "send event failed"));
        } else {
            napiAsyncTask.release();
        }
        return result;
    }
};
} // namespace

napi_value CreateJsMetadata(napi_env env, const AppExecFwk::Metadata &info)
{
    IMSA_HILOGI("CreateJsMetadata start.");

    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);

    napi_set_named_property(env, objValue, "name", CreateJsValue(env, info.name));
    napi_set_named_property(env, objValue, "value", CreateJsValue(env, info.value));
    napi_set_named_property(env, objValue, "resource", CreateJsValue(env, info.resource));
    return objValue;
}

napi_value CreateJsMetadataArray(napi_env env, const std::vector<AppExecFwk::Metadata> &info)
{
    IMSA_HILOGI("CreateJsMetadataArray start.");
    napi_value arrayValue = nullptr;
    napi_create_array_with_length(env, info.size(), &arrayValue);
    uint32_t index = 0;
    for (const auto &item : info) {
        napi_set_element(env, arrayValue, index++, CreateJsMetadata(env, item));
    }
    return arrayValue;
}

napi_value CreateJsExtensionAbilityInfo(napi_env env, const AppExecFwk::ExtensionAbilityInfo &info)
{
    IMSA_HILOGI("CreateJsExtensionAbilityInfo start.");
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);

    napi_set_named_property(env, objValue, "bundleName", CreateJsValue(env, info.bundleName));
    napi_set_named_property(env, objValue, "moduleName", CreateJsValue(env, info.moduleName));
    napi_set_named_property(env, objValue, "name", CreateJsValue(env, info.name));
    napi_set_named_property(env, objValue, "labelId", CreateJsValue(env, info.labelId));
    napi_set_named_property(env, objValue, "descriptionId", CreateJsValue(env, info.descriptionId));
    napi_set_named_property(env, objValue, "iconId", CreateJsValue(env, info.iconId));
    napi_set_named_property(env, objValue, "isVisible", CreateJsValue(env, info.visible));
    napi_set_named_property(env, objValue, "extensionAbilityType", CreateJsValue(env, info.type));

    napi_value permissionArray = nullptr;
    napi_create_array_with_length(env, info.permissions.size(), &permissionArray);

    if (permissionArray != nullptr) {
        int index = 0;
        for (auto permission : info.permissions) {
            napi_set_element(env, permissionArray, index++, CreateJsValue(env, permission));
        }
    }
    napi_set_named_property(env, objValue, "permissions", permissionArray);
    napi_set_named_property(env, objValue, "applicationInfo", CreateJsApplicationInfo(env, info.applicationInfo));
    napi_set_named_property(env, objValue, "metadata", CreateJsMetadataArray(env, info.metadata));
    napi_set_named_property(env, objValue, "enabled", CreateJsValue(env, info.enabled));
    napi_set_named_property(env, objValue, "readPermission", CreateJsValue(env, info.readPermission));
    napi_set_named_property(env, objValue, "writePermission", CreateJsValue(env, info.writePermission));
    return objValue;
}

napi_value CreateJsInputMethodExtensionContext(napi_env env, std::shared_ptr<InputMethodExtensionContext> context)
{
    IMSA_HILOGI("CreateJsInputMethodExtensionContext start.");
    if (context != nullptr) {
        auto abilityInfo = context->GetAbilityInfo();
    }

    napi_value objValue = CreateJsExtensionContext(env, context);
    std::unique_ptr<JsInputMethodExtensionContext> jsContext = std::make_unique<JsInputMethodExtensionContext>(context);
    napi_wrap(env, objValue, jsContext.release(), JsInputMethodExtensionContext::Finalizer, nullptr, nullptr);

    const char *moduleName = "JsInputMethodExtensionContext";
    BindNativeFunction(env, objValue, "startAbility", moduleName, JsInputMethodExtensionContext::StartAbility);
    BindNativeFunction(env, objValue, "terminateSelf", moduleName, JsInputMethodExtensionContext::TerminateAbility);
    BindNativeFunction(env, objValue, "destroy", moduleName, JsInputMethodExtensionContext::TerminateAbility);
    BindNativeFunction(env, objValue, "connectAbility", moduleName, JsInputMethodExtensionContext::ConnectAbility);
    BindNativeFunction(env, objValue, "disconnectAbility", moduleName,
        JsInputMethodExtensionContext::DisconnectAbility);
    BindNativeFunction(env, objValue, "startAbilityWithAccount", moduleName,
        JsInputMethodExtensionContext::StartAbilityWithAccount);
    BindNativeFunction(env, objValue, "connectAbilityWithAccount", moduleName,
        JsInputMethodExtensionContext::ConnectAbilityWithAccount);
    return objValue;
}

JSInputMethodExtensionConnection::JSInputMethodExtensionConnection(napi_env env) : env_(env),
    handler_(std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::GetMainEventRunner()))
{
}

JSInputMethodExtensionConnection::~JSInputMethodExtensionConnection() = default;

void JSInputMethodExtensionConnection::OnAbilityConnectDone(const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    IMSA_HILOGI("OnAbilityConnectDone start, resultCode: %{public}d.", resultCode);
    if (handler_ == nullptr) {
        IMSA_HILOGI("handler_ is nullptr.");
        return;
    }
    wptr<JSInputMethodExtensionConnection> connection = this;
    auto task = [connection, element, remoteObject, resultCode]() {
        sptr<JSInputMethodExtensionConnection> connectionSptr = connection.promote();
        if (connectionSptr == nullptr) {
            IMSA_HILOGE("connectionSptr is nullptr.");
            return;
        }
        connectionSptr->HandleOnAbilityConnectDone(element, remoteObject, resultCode);
    };
    handler_->PostTask(task, "OnAbilityConnectDone", 0, AppExecFwk::EventQueue::Priority::VIP);
}

void JSInputMethodExtensionConnection::HandleOnAbilityConnectDone(const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    IMSA_HILOGI("HandleOnAbilityConnectDone start, resultCode:%{public}d.", resultCode);
    // wrap ElementName
    napi_value napiElementName = OHOS::AppExecFwk::WrapElementName(env_, element);

    // wrap RemoteObject
    IMSA_HILOGI("OnAbilityConnectDone start NAPI_ohos_rpc_CreateJsRemoteObject.");
    napi_value napiRemoteObject = NAPI_ohos_rpc_CreateJsRemoteObject(env_, remoteObject);
    napi_value argv[] = { napiElementName, napiRemoteObject };

    if (jsConnectionObject_ == nullptr) {
        IMSA_HILOGE("jsConnectionObject_ is nullptr!");
        return;
    }

    napi_value obj = nullptr;
    if (napi_get_reference_value(env_, jsConnectionObject_, &obj) != napi_ok) {
        IMSA_HILOGE("failed to get jsConnectionObject_!");
        return;
    }
    if (obj == nullptr) {
        IMSA_HILOGE("failed to get object!");
        return;
    }
    napi_value methodOnConnect = nullptr;
    napi_get_named_property(env_, obj, "onConnect", &methodOnConnect);
    if (methodOnConnect == nullptr) {
        IMSA_HILOGE("failed to get onConnect from object!");
        return;
    }
    IMSA_HILOGI("JSInputMethodExtensionConnection::CallFunction onConnect, success.");
    napi_value callResult = nullptr;
    napi_call_function(env_, obj, methodOnConnect, ARGC_TWO, argv, &callResult);
    IMSA_HILOGI("OnAbilityConnectDone end.");
}

void JSInputMethodExtensionConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode)
{
    IMSA_HILOGI("OnAbilityDisconnectDone start, resultCode: %{public}d.", resultCode);
    if (handler_ == nullptr) {
        IMSA_HILOGI("handler_ is nullptr.");
        return;
    }
    wptr<JSInputMethodExtensionConnection> connection = this;
    auto task = [connection, element, resultCode]() {
        sptr<JSInputMethodExtensionConnection> connectionSptr = connection.promote();
        if (!connectionSptr) {
            IMSA_HILOGE("connectionSptr is nullptr.");
            return;
        }
        connectionSptr->HandleOnAbilityDisconnectDone(element, resultCode);
    };
    handler_->PostTask(task, "OnAbilityDisconnectDone", 0, AppExecFwk::EventQueue::Priority::VIP);
}

void JSInputMethodExtensionConnection::HandleOnAbilityDisconnectDone(const AppExecFwk::ElementName &element,
    int resultCode)
{
    IMSA_HILOGI("HandleOnAbilityDisconnectDone start, resultCode:%{public}d.", resultCode);
    napi_value napiElementName = OHOS::AppExecFwk::WrapElementName(env_, element);
    napi_value argv[] = { napiElementName };
    if (jsConnectionObject_ == nullptr) {
        IMSA_HILOGE("jsConnectionObject_ is nullptr!");
        return;
    }
    napi_value obj = nullptr;
    if (napi_get_reference_value(env_, jsConnectionObject_, &obj) != napi_ok) {
        IMSA_HILOGE("failed to get jsConnectionObject_!");
        return;
    }
    if (obj == nullptr) {
        IMSA_HILOGE("failed to get object!");
        return;
    }
    napi_value method = nullptr;
    napi_get_named_property(env_, obj, "onDisconnect", &method);
    if (method == nullptr) {
        IMSA_HILOGE("failed to get onDisconnect from object!");
        return;
    }
    // release connect
    std::string bundleName = element.GetBundleName();
    std::string abilityName = element.GetAbilityName();
    {
        std::lock_guard<std::mutex> lock(g_connectMapMtx);
        IMSA_HILOGI("OnAbilityDisconnectDone connects_.size: %{public}zu.", connects_.size());
        auto item = std::find_if(connects_.begin(), connects_.end(),
            [bundleName, abilityName](
                const std::map<ConnectionKey, sptr<JSInputMethodExtensionConnection>>::value_type &obj) {
                return (bundleName == obj.first.want.GetBundle()) &&
                       (abilityName == obj.first.want.GetElement().GetAbilityName());
            });
        if (item != connects_.end()) {
            // match bundleName && abilityName
            if (item->second != nullptr) {
                item->second->ReleaseConnection();
            }
            connects_.erase(item);
            IMSA_HILOGI("OnAbilityDisconnectDone erase connects_.size: %{public}zu.", connects_.size());
        }
    }
    IMSA_HILOGI("OnAbilityDisconnectDone CallFunction success.");
    napi_value callResult = nullptr;
    napi_call_function(env_, obj, method, ARGC_ONE, argv, &callResult);
}

void JSInputMethodExtensionConnection::SetJsConnectionObject(napi_value jsConnectionObject)
{
    napi_create_reference(env_, jsConnectionObject, 1, &jsConnectionObject_);
}

void JSInputMethodExtensionConnection::CallJsFailed(int32_t errorCode)
{
    IMSA_HILOGI("CallJsFailed start");
    if (jsConnectionObject_ == nullptr) {
        IMSA_HILOGE("jsConnectionObject_ is nullptr!");
        return;
    }
    napi_value obj = nullptr;
    if (napi_get_reference_value(env_, jsConnectionObject_, &obj) != napi_ok) {
        IMSA_HILOGE("failed to get jsConnectionObject_!");
        return;
    }
    if (obj == nullptr) {
        IMSA_HILOGE("failed to get object.");
        return;
    }

    napi_value method = nullptr;
    napi_get_named_property(env_, obj, "onFailed", &method);
    if (method == nullptr) {
        IMSA_HILOGE("failed to get onFailed from object!");
        return;
    }
    napi_value result = nullptr;
    napi_create_int32(env_, errorCode, &result);
    napi_value argv[] = { result };
    IMSA_HILOGI("CallJsFailed CallFunction success.");
    napi_value callResult = nullptr;
    napi_call_function(env_, obj, method, ARGC_ONE, argv, &callResult);
    IMSA_HILOGI("CallJsFailed end.");
}

void JSInputMethodExtensionConnection::ReleaseConnection()
{
    IMSA_HILOGD("ReleaseConnection");
    if (jsConnectionObject_ != nullptr) {
        napi_delete_reference(env_, jsConnectionObject_);
        env_ = nullptr;
        jsConnectionObject_ = nullptr;
    }
}
} // namespace AbilityRuntime
} // namespace OHOS
