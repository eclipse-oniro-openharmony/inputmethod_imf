/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "callback_Processor.h"

namespace OHOS {
namespace MiscServices {
constexpr size_t MAX_ARGV_COUNT = 10;
void CallbackProcessor::TraverseCallback(const CallbackInput &input)
{
    auto argc = input.argc;
    auto argvProvider = input.argvProvider;
    for (const auto &object : input.vecCopy) {
        napi_value result = nullptr;
        DisposeCallback(object, argc, argvProvider, result);
    }
}

void CallbackProcessor::DisposeCallback(
    const std::shared_ptr<JSCallbackObject> &callbackObject, size_t argc, ArgvProvider argvProvider, napi_value result)
{
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(callbackObject->env_, &scope);
    if (callbackObject->threadId_ != std::this_thread::get_id()) {
        napi_close_handle_scope(callbackObject->env_, scope);
        return;
    }
    napi_value argv[MAX_ARGV_COUNT];
    if (argvProvider != nullptr && !argvProvider(argv, MAX_ARGV_COUNT, callbackObject)) {
        napi_close_handle_scope(callbackObject->env_, scope);
        return;
    }
    napi_value callback = nullptr;
    napi_value global = nullptr;
    napi_get_reference_value(callbackObject->env_, callbackObject->callback_, &callback);
    if (callback != nullptr) {
        napi_get_global(callbackObject->env_, &global);
        auto status = napi_call_function(callbackObject->env_, global, callback, argc, argv, &result);
        if (status != napi_ok) {
            result = nullptr;
        }
    }
    napi_close_handle_scope(callbackObject->env_, scope);
}
} // namespace MiscServices
} // namespace OHOS
