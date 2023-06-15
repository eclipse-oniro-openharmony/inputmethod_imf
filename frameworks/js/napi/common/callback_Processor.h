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
#ifndef OHOS_INPUT_CALLBACK_PROCESSOR_H
#define OHOS_INPUT_CALLBACK_PROCESSOR_H

#include "js_callback_object.h"
#include "js_util.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace MiscServices {

class CallbackProcessor {
public:
    using ArgvProvider = std::function<bool(napi_env, napi_value *, size_t)>;
    struct CallbackInput {
        std::vector<std::shared_ptr<JSCallbackObject>> vecCopy;
        size_t argc{ 0 };
        ArgvProvider argvProvider{ nullptr };
    };
    static void TraverseCallback(const CallbackInput &input);
    template<class T> static void TraverseCallback(const CallbackInput &input, T &out)
    {
        auto argc = input.argc;
        auto argvProvider = input.argvProvider;
        for (const auto &object : input.vecCopy) {
            napi_handle_scope scope = nullptr;
            napi_open_handle_scope(object->env_, &scope);
            napi_value result = nullptr;
            DisposeCallback(object, argc, argvProvider, result);
            if (GenerateOutput(object->env_, result, out)) {
                napi_close_handle_scope(object->env_, scope);
                break; //是否一个处理完成就返回？？？？
            }
            napi_close_handle_scope(object->env_, scope);
        }
    }

private:
    static void DisposeCallback(const std::shared_ptr<JSCallbackObject> &callbackObject, size_t argc,
        ArgvProvider argvProvider, napi_value &result);
    template<class T> static bool GenerateOutput(napi_env env, napi_value result, T &out)
    {
        if (result == nullptr) {
            return false;
        }
        if (!JsUtil::GetValue(env, result, out)) {
            return false;
        }
        return true;
    }
}; // namespace MiscServices
} // namespace MiscServices
} // namespace OHOS
#endif // OHOS_INPUT_CALLBACK_PROCESSOR_H
