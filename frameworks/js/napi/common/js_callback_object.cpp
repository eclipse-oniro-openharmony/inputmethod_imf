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
#include "global.h"

#include <uv.h>

namespace OHOS {
namespace MiscServices {
constexpr int32_t MAX_TIMEOUT = 2000;
JSCallbackObject::JSCallbackObject(napi_env env, napi_value callback, std::thread::id threadId,
    std::shared_ptr<AppExecFwk::EventHandler> jsHandler)
    : env_(env), threadId_(threadId), jsHandler_(jsHandler)
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
        isDone_ = std::make_shared<BlockData<bool>>(MAX_TIMEOUT, false);
        std::string type = "~JSCallbackObject";
        auto eventHandler = jsHandler_;
        if (eventHandler == nullptr) {
            IMSA_HILOGE("eventHandler is nullptr!");
            return;
        }
        auto task = [env = env_, callback = callback_, isDone = isDone_]() {
            napi_delete_reference(env, callback);
            bool isFinish = true;
            isDone->SetValue(isFinish);
        };
        eventHandler->PostTask(task, type);
        isDone_->GetValue();
    }
    env_ = nullptr;
}


JSMsgHandlerCallbackObject::JSMsgHandlerCallbackObject(napi_env env, napi_value onTerminated, napi_value onMessage)
    : env_(env), handler_(AppExecFwk::EventHandler::Current()), threadId_(std::this_thread::get_id())
{
    napi_create_reference(env, onTerminated, 1, &onTerminatedCallback_);
    napi_create_reference(env, onMessage, 1, &onMessageCallback_);
}

JSMsgHandlerCallbackObject::~JSMsgHandlerCallbackObject()
{
    if (threadId_ == std::this_thread::get_id()) {
        if (onTerminatedCallback_ != nullptr) {
            napi_delete_reference(env_, onTerminatedCallback_);
        }
        if (onMessageCallback_ != nullptr) {
            napi_delete_reference(env_, onMessageCallback_);
        }
        env_ = nullptr;
        return;
    }
    IMSA_HILOGW("Thread id is not same, abstract destructor is run in muti-thread!");
    env_ = nullptr;
}

std::shared_ptr<AppExecFwk::EventHandler> JSMsgHandlerCallbackObject::GetEventHandler()
{
    std::lock_guard<std::mutex> lock(eventHandlerMutex_);
    return handler_;
}
} // namespace MiscServices
} // namespace OHOS
