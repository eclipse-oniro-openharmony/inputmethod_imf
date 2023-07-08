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

#include "js_callback_object.h"

namespace OHOS {
namespace MiscServices {
JSCallbackObject::JSCallbackObject(napi_env env, napi_value callback, std::thread::id threadId)
    : env_(env), threadId_(threadId)
{
    napi_create_reference(env, callback, 1, &callback_);
}

JSCallbackObject::~JSCallbackObject()
{
    if (callback_ != nullptr) {
        napi_delete_reference(env_, callback_);
    }
    env_ = nullptr;
}
} // namespace MiscServices
} // namespace OHOS