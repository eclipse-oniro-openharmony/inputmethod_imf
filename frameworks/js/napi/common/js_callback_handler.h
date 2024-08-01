/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#ifndef OHOS_INPUT_CALLBACK_HANDLER_H
#define OHOS_INPUT_CALLBACK_HANDLER_H

#include <functional>
#include "inputmethod_trace.h"
#include "js_callback_object.h"
#include "js_util.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace MiscServices {
class JsCallbackHandler {
public:
    using ArgvProvider = std::function<bool(napi_env, napi_value *, size_t)>;
    struct ArgContainer {
        size_t argc{ 0 };
        ArgvProvider argvProvider{ nullptr };
    };
    // 0 means the callback has no param.
    static void Traverse(const std::vector<std::shared_ptr<JSCallbackObject>> &objects,
        const ArgContainer &argContainer = { 0, nullptr })
    {
        InputMethodSyncTrace tracer("Traverse callback");
        for (const auto &object : objects) {
            if (object == nullptr) {
                continue;
            }
            JsUtil::ScopeGuard scopeGuard(object->env_);
            napi_value jsOutput = nullptr;
            Execute(object, argContainer, jsOutput);
        }
    }
    template<typename T>
    static void Traverse(const std::vector<std::shared_ptr<JSCallbackObject>> &objects,
        const ArgContainer &argContainer, T &output)
    {
        InputMethodSyncTrace tracer("Traverse callback with output");
        for (const auto &object : objects) {
            JsUtil::ScopeGuard scopeGuard(object->env_);
            napi_value jsOutput = nullptr;
            Execute(object, argContainer, jsOutput);
            if (jsOutput != nullptr && JsUtil::GetValue(object->env_, jsOutput, output)) {
                break;
            }
        }
    }

private:
    static void Execute(const std::shared_ptr<JSCallbackObject> &object, const ArgContainer &argContainer,
        napi_value &output);
};
} // namespace MiscServices
} // namespace OHOS
#endif // OHOS_INPUT_CALLBACK_HANDLER_H
