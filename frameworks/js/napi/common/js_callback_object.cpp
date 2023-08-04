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

#include <uv.h>

#include "js_callback_object.h"

namespace OHOS {
namespace MiscServices {
constexpr int32_t MAX_TIMEOUT = 2000;
JSCallbackObject::JSCallbackObject(napi_env env, napi_value callback, std::thread::id threadId)
    : env_(env), threadId_(threadId)
{
    napi_create_reference(env, callback, 1, &callback_);
}

JSCallbackObject::~JSCallbackObject()
{
    if (callback_ != nullptr) {
        if (threadId_ == std::this_thread::get_id()) {
            napi_delete_reference(env_, callback_);
            env_ = nullptr;
            return;
        }
        uv_work_t *work = new (std::nothrow) uv_work_t;
        if (work == nullptr) {
            return;
        }
        isDone_ = std::make_shared<BlockData<bool>>(MAX_TIMEOUT, false);
        work->data = this;
        uv_loop_s *loop = nullptr;
        napi_delete_reference(env_, callback_);
        napi_get_uv_event_loop(env_, &loop);
        uv_queue_work_with_qos(
            loop, work, [](uv_work_t *work) {},
            [](uv_work_t *work, int status) {
                JSCallbackObject *jsObject = static_cast<JSCallbackObject *>(work->data);
                napi_delete_reference(jsObject->env_, jsObject->callback_);
                bool isFinish = true;
                jsObject->isDone_->SetValue(isFinish);
            },
            uv_qos_user_initiated);
    }
    isDone_->GetValue();
    env_ = nullptr;
}
} // namespace MiscServices
} // namespace OHOS