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
#ifndef INTERFACE_KITS_JS_NAPI_INPUTMETHODENGINE_INCLUDE_JS_TEXT_INPUT_CLIENT_H
#define INTERFACE_KITS_JS_NAPI_INPUTMETHODENGINE_INCLUDE_JS_TEXT_INPUT_CLIENT_H

#include "async_call.h"
#include "global.h"
#include "native_engine/native_engine.h"
#include "native_engine/native_value.h"

namespace OHOS {
namespace MiscServices {
struct SendKeyFunctionContext : public AsyncCall::Context {
    bool isSendKeyFunction = false;
    int32_t action = 0;
    napi_status status = napi_generic_failure;
    SendKeyFunctionContext() : Context(nullptr, nullptr){};
    SendKeyFunctionContext(InputAction input, OutputAction output) : Context(std::move(input), std::move(output)){};

    napi_status operator()(napi_env env, size_t argc, napi_value *argv, napi_value self) override
    {
        NAPI_ASSERT_BASE(env, self != nullptr, "self is nullptr", napi_invalid_arg);
        return Context::operator()(env, argc, argv, self);
    }

    napi_status operator()(napi_env env, napi_value *result) override
    {
        if (status != napi_ok) {
            output_ = nullptr;
            return status;
        }
        return Context::operator()(env, result);
    }
};

struct MoveCursorContext : public AsyncCall::Context {
    int32_t num = 0;
    napi_status status = napi_generic_failure;
    MoveCursorContext() : Context(nullptr, nullptr){};
    MoveCursorContext(InputAction input, OutputAction output) : Context(std::move(input), std::move(output)){};

    napi_status operator()(napi_env env, size_t argc, napi_value *argv, napi_value self) override
    {
        NAPI_ASSERT_BASE(env, self != nullptr, "self is nullptr", napi_invalid_arg);
        return Context::operator()(env, argc, argv, self);
    }
    napi_status operator()(napi_env env, napi_value *result) override
    {
        if (status != napi_ok) {
            output_ = nullptr;
            return status;
        }
        return Context::operator()(env, result);
    }
};

struct DeleteForwardContext : public AsyncCall::Context {
    bool isDeleteForward = false;
    int32_t length = 0;
    napi_status status = napi_generic_failure;
    DeleteForwardContext() : Context(nullptr, nullptr){};
    DeleteForwardContext(InputAction input, OutputAction output) : Context(std::move(input), std::move(output)){};

    napi_status operator()(napi_env env, size_t argc, napi_value *argv, napi_value self) override
    {
        NAPI_ASSERT_BASE(env, self != nullptr, "self is nullptr", napi_invalid_arg);
        return Context::operator()(env, argc, argv, self);
    }
    napi_status operator()(napi_env env, napi_value *result) override
    {
        if (status != napi_ok) {
            output_ = nullptr;
            return status;
        }
        return Context::operator()(env, result);
    }
};

struct DeleteBackwardContext : public AsyncCall::Context {
    bool isDeleteBackward = false;
    int32_t length = 0;
    napi_status status = napi_generic_failure;
    DeleteBackwardContext() : Context(nullptr, nullptr){};
    DeleteBackwardContext(InputAction input, OutputAction output) : Context(std::move(input), std::move(output)){};

    napi_status operator()(napi_env env, size_t argc, napi_value *argv, napi_value self) override
    {
        NAPI_ASSERT_BASE(env, self != nullptr, "self is nullptr", napi_invalid_arg);
        return Context::operator()(env, argc, argv, self);
    }
    napi_status operator()(napi_env env, napi_value *result) override
    {
        if (status != napi_ok) {
            output_ = nullptr;
            return status;
        }
        return Context::operator()(env, result);
    }
};

struct InsertTextContext : public AsyncCall::Context {
    bool isInsertText = false;
    std::string text;
    napi_status status = napi_generic_failure;
    InsertTextContext() : Context(nullptr, nullptr){};
    InsertTextContext(InputAction input, OutputAction output) : Context(std::move(input), std::move(output)){};

    napi_status operator()(napi_env env, size_t argc, napi_value *argv, napi_value self) override
    {
        NAPI_ASSERT_BASE(env, self != nullptr, "self is nullptr", napi_invalid_arg);
        return Context::operator()(env, argc, argv, self);
    }
    napi_status operator()(napi_env env, napi_value *result) override
    {
        if (status != napi_ok) {
            output_ = nullptr;
            return status;
        }
        return Context::operator()(env, result);
    }
};

struct GetForwardContext : public AsyncCall::Context {
    int32_t length = 0;
    std::string text;
    napi_status status = napi_generic_failure;
    GetForwardContext() : Context(nullptr, nullptr){};
    GetForwardContext(InputAction input, OutputAction output) : Context(std::move(input), std::move(output)){};

    napi_status operator()(napi_env env, size_t argc, napi_value *argv, napi_value self) override
    {
        NAPI_ASSERT_BASE(env, self != nullptr, "self is nullptr", napi_invalid_arg);
        return Context::operator()(env, argc, argv, self);
    }
    napi_status operator()(napi_env env, napi_value *result) override
    {
        if (status != napi_ok) {
            output_ = nullptr;
            return status;
        }
        return Context::operator()(env, result);
    }
};

struct GetBackwardContext : public AsyncCall::Context {
    int32_t length = 0;
    std::string text;
    napi_status status = napi_generic_failure;
    GetBackwardContext() : Context(nullptr, nullptr){};
    GetBackwardContext(InputAction input, OutputAction output) : Context(std::move(input), std::move(output)){};

    napi_status operator()(napi_env env, size_t argc, napi_value *argv, napi_value self) override
    {
        NAPI_ASSERT_BASE(env, self != nullptr, "self is nullptr", napi_invalid_arg);
        return Context::operator()(env, argc, argv, self);
    }
    napi_status operator()(napi_env env, napi_value *result) override
    {
        if (status != napi_ok) {
            output_ = nullptr;
            return status;
        }
        return Context::operator()(env, result);
    }
};

struct GetEditorAttributeContext : public AsyncCall::Context {
    int32_t inputPattern = 0;
    int32_t enterKeyType = 0;
    napi_status status = napi_generic_failure;
    GetEditorAttributeContext() : Context(nullptr, nullptr){};
    GetEditorAttributeContext(InputAction input, OutputAction output) : Context(std::move(input), std::move(output)){};

    napi_status operator()(napi_env env, size_t argc, napi_value *argv, napi_value self) override
    {
        NAPI_ASSERT_BASE(env, self != nullptr, "self is nullptr", napi_invalid_arg);
        return Context::operator()(env, argc, argv, self);
    }
    napi_status operator()(napi_env env, napi_value *result) override
    {
        if (status != napi_ok) {
            output_ = nullptr;
            return status;
        }
        return Context::operator()(env, result);
    }
};

struct GetTextIndexAtCursorContext : public AsyncCall::Context {
    int32_t index = 0;
    napi_status status = napi_generic_failure;
    GetTextIndexAtCursorContext() : Context(nullptr, nullptr){};
    GetTextIndexAtCursorContext(InputAction input, OutputAction output)
        : Context(std::move(input), std::move(output)){};

    napi_status operator()(napi_env env, size_t argc, napi_value *argv, napi_value self) override
    {
        NAPI_ASSERT_BASE(env, self != nullptr, "self is nullptr", napi_invalid_arg);
        return Context::operator()(env, argc, argv, self);
    }
    napi_status operator()(napi_env env, napi_value *result) override
    {
        if (status != napi_ok) {
            output_ = nullptr;
            return status;
        }
        return Context::operator()(env, result);
    }
};

class JsTextInputClientEngine {
public:
    JsTextInputClientEngine() = default;
    ~JsTextInputClientEngine() = default;
    static napi_value Init(napi_env env, napi_value info);
    static napi_value SendKeyFunction(napi_env env, napi_callback_info info);
    static napi_value DeleteForward(napi_env env, napi_callback_info info);
    static napi_value DeleteBackward(napi_env env, napi_callback_info info);
    static napi_value InsertText(napi_env env, napi_callback_info info);
    static napi_value GetForward(napi_env env, napi_callback_info info);
    static napi_value GetBackward(napi_env env, napi_callback_info info);
    static napi_value MoveCursor(napi_env env, napi_callback_info info);
    static napi_value GetEditorAttribute(napi_env env, napi_callback_info info);
    static napi_value GetTextIndexAtCursor(napi_env env, napi_callback_info info);
    static napi_value GetTextInputClientInstance(napi_env env);

private:
    static napi_status GetAction(napi_env env, napi_value argv, std::shared_ptr<SendKeyFunctionContext> ctxt);
    static napi_status GetDeleteForwardLength(
        napi_env env, napi_value argv, std::shared_ptr<DeleteForwardContext> ctxt);
    static napi_status GetDeleteBackwardLength(
        napi_env env, napi_value argv, std::shared_ptr<DeleteBackwardContext> ctxt);
    static napi_status GetMoveCursorParam(napi_env env, napi_value argv, std::shared_ptr<MoveCursorContext> ctxt);
    static napi_status GetInsertText(napi_env env, napi_value argv, std::shared_ptr<InsertTextContext> ctxt);
    static napi_status GetForwardLength(napi_env env, napi_value argv, std::shared_ptr<GetForwardContext> ctxt);
    static napi_status GetBackwardLength(napi_env env, napi_value argv, std::shared_ptr<GetBackwardContext> ctxt);

    static napi_value JsConstructor(napi_env env, napi_callback_info cbinfo);
    static int32_t GetNumberProperty(napi_env env, napi_value obj);
    static std::string GetStringProperty(napi_env env, napi_value obj);
    static napi_value GetResult(napi_env env, std::string &text);
    static napi_value GetResultEditorAttribute(
        napi_env env, std::shared_ptr<GetEditorAttributeContext> getEditorAttribute);

    static const std::string TIC_CLASS_NAME;
    static thread_local napi_ref TICRef_;
    static constexpr std::int32_t MAX_VALUE_LEN = 4096;
};
} // namespace MiscServices
} // namespace OHOS
#endif // INTERFACE_KITS_JS_NAPI_INPUTMETHODENGINE_INCLUDE_JS_TEXT_INPUT_CLIENT_H