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
#ifndef INTERFACE_JS_CONTEXT_H
#define INTERFACE_JS_CONTEXT_H
#include "input_method_property.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace MiscServices {
using NapiCbInfoParser = std::function<void(size_t argc, napi_value* argv)>;
struct ContextBase {
    virtual ~ContextBase();
    void ParseContext(napi_env env, napi_callback_info Info, NapiCbInfoParser prase = NapiCbInfoParser());
    napi_status GetNative(napi_env env, napi_callback_info info);
    napi_value GetErrorCodeValue(napi_env env, int errCode);
    napi_env env = nullptr;
    napi_value self = nullptr;
    napi_ref selfRef = nullptr;
    napi_async_work work = nullptr;
    napi_deferred deferred = nullptr;
    napi_ref callbackRef = nullptr;
    napi_value outData = nullptr;
    napi_callback_info info;
    void* native = nullptr;
    int32_t errCode;
private:
    static constexpr size_t ARGC_MAX = 6;
};
}
}
#endif // INTERFACE_JS_CONTEXT_H