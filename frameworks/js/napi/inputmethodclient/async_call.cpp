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

#include "async_call.h"

#include <algorithm>

#include "global.h"
#include "js_utils.h"

namespace OHOS {
namespace MiscServices {
using namespace std::chrono;
constexpr size_t ARGC_MAX = 6;
constexpr int32_t MAX_WAIT_TIME = 500; // ms
static inline uint64_t GetTimeStamp()
{
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}
AsyncCall::AsyncCall(napi_env env, napi_callback_info info, std::shared_ptr<Context> context, size_t maxParamCount)
    : env_(env)
{
    context_ = new AsyncContext();
    NAPI_ASSERT_RETURN_VOID(env, context_ != nullptr, "context_ != nullptr");
    size_t argc = ARGC_MAX;
    napi_value self = nullptr;
    napi_value argv[ARGC_MAX] = { nullptr };
    NAPI_CALL_RETURN_VOID(env, napi_get_cb_info(env, info, &argc, argv, &self, nullptr));
    napi_valuetype valueType = napi_undefined;
    argc = std::min(argc, maxParamCount);
    if (argc > 0) {
        napi_typeof(env, argv[argc - 1], &valueType);
        if (valueType == napi_function) {
            napi_create_reference(env, argv[argc - 1], 1, &context_->callback);
            argc = argc - 1;
        }
    }
    NAPI_CALL_RETURN_VOID(env, (*context)(env, argc, argv, self));
    context_->ctx = std::move(context);
    napi_create_reference(env, self, 1, &context_->self);
}

AsyncCall::~AsyncCall()
{
    if (context_ == nullptr) {
        return;
    }

    DeleteContext(env_, context_);
}

napi_value AsyncCall::Call(napi_env env, Context::ExecAction exec, const std::string &resourceName)
{
    if (context_ == nullptr) {
        IMSA_HILOGE("context_ is nullptr!");
        return nullptr;
    }
    if (context_->ctx == nullptr) {
        IMSA_HILOGE("context_->ctx is nullptr!");
        return nullptr;
    }
    context_->ctx->exec_ = std::move(exec);
    napi_value promise = nullptr;
    if (context_->callback == nullptr) {
        napi_create_promise(env, &context_->defer, &promise);
    } else {
        napi_get_undefined(env, &promise);
    }
    napi_async_work work = context_->work;
    napi_value resource = nullptr;
    std::string name = "IMF_" + resourceName;
    napi_create_string_utf8(env, name.c_str(), NAPI_AUTO_LENGTH, &resource);
    napi_create_async_work(env, nullptr, resource, AsyncCall::OnExecute, AsyncCall::OnComplete, context_, &work);
    context_->work = work;
    context_ = nullptr;
    napi_queue_async_work_with_qos(env, work, napi_qos_user_initiated);
    return promise;
}

napi_value AsyncCall::Post(napi_env env, Context::ExecAction exec, std::shared_ptr<TaskQueue> queue, const char *func)
{
    if (context_ == nullptr || context_->ctx == nullptr || queue == nullptr) {
        IMSA_HILOGE("context is nullptr!");
        return nullptr;
    }
    context_->ctx->exec_ = std::move(exec);
    napi_value promise = nullptr;
    if (context_->callback == nullptr) {
        napi_create_promise(env, &context_->defer, &promise);
    } else {
        napi_get_undefined(env, &promise);
    }
    napi_async_work work = context_->work;
    napi_value resource = nullptr;
    napi_create_string_utf8(env, func, NAPI_AUTO_LENGTH, &resource);
    napi_create_async_work(env, nullptr, resource, AsyncCall::OnExecuteSeq, AsyncCall::OnComplete, context_, &work);
    context_->work = work;
    context_->queue = queue;
    std::unique_lock<ffrt::mutex> lock(queue->queuesMutex_);
    queue->taskQueue_.emplace(env, work, func);
    if (!queue->isRunning) {
        auto status = napi_queue_async_work_with_qos(env, work, napi_qos_user_initiated);
        queue->isRunning = status == napi_ok;
        if (status != napi_ok) {
            IMSA_HILOGE("async work failed.status:%{public}d, func:%{public}s!", status, func);
        }
    }
    context_ = nullptr;
    return promise;
}

napi_value AsyncCall::SyncCall(napi_env env, AsyncCall::Context::ExecAction exec)
{
    if ((context_ == nullptr) || (context_->ctx == nullptr)) {
        IMSA_HILOGE("context_ or context_->ctx is nullptr!");
        return nullptr;
    }
    context_->ctx->exec_ = std::move(exec);
    napi_value promise = nullptr;
    if (context_->callback == nullptr) {
        napi_create_promise(env, &context_->defer, &promise);
    } else {
        napi_get_undefined(env, &promise);
    }
    AsyncCall::OnExecute(env, context_);
    AsyncCall::OnComplete(env, context_->ctx->status_, context_);
    return promise;
}

void AsyncCall::OnExecute(napi_env env, void *data)
{
    AsyncContext *context = reinterpret_cast<AsyncContext *>(data);
    if (context == nullptr || context->ctx == nullptr) {
        IMSA_HILOGE("context or context->ctx is nullptr!");
        return;
    }
    context->ctx->Exec();
}

void AsyncCall::OnExecuteSeq(napi_env env, void *data)
{
    OnExecute(env, data);
    AsyncContext *context = reinterpret_cast<AsyncContext *>(data);
    if (context == nullptr || context->queue == nullptr) {
        IMSA_HILOGE("context or context->queue is nullptr!");
        return;
    }
    auto queue = context->queue;
    std::unique_lock<ffrt::mutex> lock(queue->queuesMutex_);
    if (!queue->taskQueue_.empty()) {
        queue->taskQueue_.pop();
    }
    queue->isRunning = !queue->taskQueue_.empty() &&
                       napi_queue_async_work_with_qos(queue->taskQueue_.front().env,
                           queue->taskQueue_.front().work, napi_qos_user_initiated) == napi_ok;
}

void AsyncCall::OnComplete(napi_env env, napi_status status, void *data)
{
    AsyncContext *context = reinterpret_cast<AsyncContext *>(data);
    napi_value output = nullptr;
    if (context == nullptr || context->ctx == nullptr) {
        IMSA_HILOGE("context or context->ctx is nullptr!");
        return;
    }
    napi_status runStatus = (*context->ctx)(env, &output);
    napi_value result[ARG_BUTT] = { 0 };
    if (status == napi_ok && runStatus == napi_ok) {
        napi_get_undefined(env, &result[ARG_ERROR]);
        if (output != nullptr) {
            IMSA_HILOGD("output != nullptr!");
            result[ARG_DATA] = output;
        } else {
            IMSA_HILOGD("output == nullptr!");
            napi_get_undefined(env, &result[ARG_DATA]);
        }
    } else {
        IMSA_HILOGE("runStatus: [%{public}d]", runStatus);
        result[ARG_ERROR] = JsUtils::ToError(env, context->ctx->errorCode_, context->ctx->errMessage_);
        napi_get_undefined(env, &result[ARG_DATA]);
    }
    if (context->defer != nullptr) {
        if (status == napi_ok && runStatus == napi_ok) {
            napi_resolve_deferred(env, context->defer, result[ARG_DATA]);
        } else {
            napi_reject_deferred(env, context->defer, result[ARG_ERROR]);
        }
    } else {
        napi_value callback = nullptr;
        napi_get_reference_value(env, context->callback, &callback);
        napi_value returnValue;
        napi_call_function(env, nullptr, callback, ARG_BUTT, result, &returnValue);
    }
    DeleteContext(env, context);
}

void AsyncCall::DeleteContext(napi_env env, AsyncContext *context)
{
    if (env != nullptr) {
        napi_delete_reference(env, context->callback);
        napi_delete_reference(env, context->self);
        napi_delete_async_work(env, context->work);
    }
    delete context;
}

AsyncCall::InnerTask::InnerTask(napi_env env, napi_async_work work, const char *name)
    : env(env), work(work), name(name), startTime(GetTimeStamp())
{
}

AsyncCall::InnerTask::~InnerTask()
{
    auto endTime = GetTimeStamp();
    if (endTime - startTime > MAX_WAIT_TIME) {
        IMSA_HILOGW("async work timeout! func:%{public}s, startTime:%{public}" PRIu64 ", endTime:%{public}" PRIu64
                    ", cost:%{public}" PRIu64 "ms",
            name, startTime, endTime, endTime - startTime);
    } else {
        IMSA_HILOGD("async work finished! func:%{public}s, startTime:%{public}" PRIu64 ", endTime:%{public}" PRIu64
                    ", cost:%{public}" PRIu64 "ms",
            name, startTime, endTime, endTime - startTime);
    }
}
} // namespace MiscServices
} // namespace OHOS
