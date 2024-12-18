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
#ifndef JS_CALLBACK_OBJECT_H
#define JS_CALLBACK_OBJECT_H

#include <mutex>
#include <thread>

#include "block_data.h"
#include "event_handler.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace MiscServices {
class JSCallbackObject {
public:
    JSCallbackObject(napi_env env, napi_value callback, std::thread::id threadId);
    ~JSCallbackObject();
    napi_ref callback_ = nullptr;
    napi_env env_{};
    std::thread::id threadId_;
    std::shared_ptr<BlockData<bool>> isDone_;
};

// Ensure this object abstract in constract thread.
class JSMsgHandlerCallbackObject {
public:
    JSMsgHandlerCallbackObject(napi_env env, napi_value onTerminated, napi_value onMessage);
    ~JSMsgHandlerCallbackObject();
    napi_env env_{};
    napi_ref onTerminatedCallback_ = nullptr;
    napi_ref onMessageCallback_ = nullptr;
    std::shared_ptr<AppExecFwk::EventHandler> GetEventHandler();

private:
    std::mutex eventHandlerMutex_;
    std::shared_ptr<AppExecFwk::EventHandler> handler_ = nullptr;
    std::thread::id threadId_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // JS_CALLBACK_OBJECT_H
