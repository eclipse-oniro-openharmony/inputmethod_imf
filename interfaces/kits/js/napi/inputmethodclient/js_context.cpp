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

#include "js_context.h"
#include "global.h"
namespace OHOS {
namespace MiscServices {

ContextBase::~ContextBase()
{
    // ZLOGD("no memory leak after callback or promise[resolved/rejected]");
    // if (env != nullptr) { //检查env
    //     if (work != nullptr) {
    //         napi_delete_async_work(env, work);
    //     }
    //     if (callbackRef != nullptr) {
    //         napi_delete_reference(env, callbackRef);
    //     }
    //     napi_delete_reference(env, selfRef);
    //     env = nullptr;
    // }
}

napi_status ContextBase::GetNative(napi_env envi, napi_callback_info info)
{
    env = envi;
    size_t argc = ARGC_MAX;
    napi_value argv[ARGC_MAX] = { nullptr };
    napi_status status = napi_invalid_arg;
    status = napi_get_cb_info(env, info, &argc, argv, &self, nullptr);
    if (self == nullptr && argc >= ARGC_MAX) {
        //LOG
        return status;
    }
    napi_create_reference(env, self, 1, &selfRef);
    status = napi_unwrap(env, self, &native);
    return status;
}

napi_value ContextBase::GetErrorCodeValue(napi_env env, int errCode)
{
    napi_value jsObject = nullptr;
    napi_value jsValue = nullptr;
    NAPI_CALL(env, napi_create_int32(env, errCode, &jsValue));
    NAPI_CALL(env, napi_create_object(env, &jsObject));
    NAPI_CALL(env, napi_set_named_property(env, jsObject, "code", jsValue));
    return jsObject;
}

void ContextBase::ParseContext(napi_env envi, napi_callback_info info, NapiCbInfoParser parse)//函数增加返回值
{
    IMSA_HILOGE("run in ParseContext");
    env = envi;
    size_t argc = ARGC_MAX;
    napi_value argv[ARGC_MAX] = { nullptr };
    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    IMSA_HILOGE("ListInputMethod::napi_get_cb_info status is: %{public}d and argc is %{public}zu: ",status,argc);
    if (status != napi_ok || argc >= ARGC_MAX) {
        //LOG 
        return;
    }
    
    // CHECK_ARGS_RETURN_VOID(this, self != nullptr, "no JavaScript this argument!");
    // napi_create_reference(env, self, 1, &selfRef);
    // status = napi_unwrap(env, self, &native);
    // CHECK_STATUS_RETURN_VOID(this, "self unwrap failed!");
    IMSA_HILOGE("ParseContext argc is %{public}zu", argc);
    if (argc > 0) {
        // get the last arguments :: <callback>
        size_t index = argc - 1;
        napi_valuetype type = napi_undefined;
        napi_status tyst = napi_typeof(env, argv[index], &type);
        IMSA_HILOGE("ListInputMethod::ParseContext tyst is: %{public}d and type is %{public}d: ",tyst,type);
        if ((tyst == napi_ok) && (type == napi_function)) {
            IMSA_HILOGE("ListInputMethod::ParseContext callabck");
            status = napi_create_reference(env, argv[index], 1, &callbackRef);
            if (status != napi_ok) {
                //LOG
                return;
            }
            // CHECK_STATUS_RETURN_VOID(this, "ref callback failed!");
            argc = index;
            // ZLOGD("async callback, no promise");
        } else {
            // ZLOGD("no callback, async pormose");
        }
    }

    if (parse) {
        parse(argc, argv);
    } else {
        //LOG
        return;
        // CHECK_ARGS_RETURN_VOID(this, argc == 0, "required no arguments!");
    }
}
}
}