/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include <unordered_map>
#include <mutex>

#include "edit_async_call.h"
#include "block_queue.h"
#include "calling_window_info.h"
#include "global.h"
#include "js_callback_object.h"
#include "js_message_handler_info.h"
#include "js_util.h"
#include "msg_handler_callback_interface.h"
#include "native_engine/native_engine.h"
#include "native_engine/native_value.h"
#include "wm_common.h"
#include "text_input_client_listener.h"
#include "input_method_utils.h"

namespace OHOS {
namespace MiscServices {
struct PrivateCommandInfo {
    std::chrono::system_clock::time_point timestamp{};
    std::unordered_map<std::string, PrivateDataValue> privateCommand;
    bool operator==(const PrivateCommandInfo &info) const
    {
        return (timestamp == info.timestamp && privateCommand == info.privateCommand);
    }
};

struct JsRect {
    static napi_value Write(napi_env env, const Rosen::Rect &nativeObject);
    static bool Read(napi_env env, napi_value jsObject, Rosen::Rect &nativeObject);
};

struct JsCallingWindowInfo {
    static napi_value Write(napi_env env, const CallingWindowInfo &nativeObject);
    static bool Read(napi_env env, napi_value object, CallingWindowInfo &nativeObject);
};

struct JsRange {
    static napi_value Write(napi_env env, const Range &nativeObject);
    static bool Read(napi_env env, napi_value jsObject, Range &nativeObject);
};

struct JsInputAttribute {
    static napi_value Write(napi_env env, const InputAttribute &nativeObject);
    static bool Read(napi_env env, napi_value jsObject, InputAttribute &nativeObject);
};

struct JsAttachOptions {
    static napi_value Write(napi_env env, const AttachOptions &attachOptions);
    static bool Read(napi_env env, napi_value jsObject, AttachOptions &attachOptions);
};

struct SendKeyFunctionContext : public AsyncCall::Context {
    bool isSendKeyFunction = false;
    int32_t action = 0;
    napi_status status = napi_generic_failure;
    SendKeyFunctionContext() : Context(nullptr, nullptr){};
    SendKeyFunctionContext(InputAction input, OutputAction output) : Context(std::move(input), std::move(output)){};

    napi_status operator()(napi_env env, size_t argc, napi_value *argv, napi_value self) override
    {
        CHECK_RETURN(self != nullptr, "self is nullptr", napi_invalid_arg);
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
        CHECK_RETURN(self != nullptr, "self is nullptr", napi_invalid_arg);
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
        CHECK_RETURN(self != nullptr, "self is nullptr", napi_invalid_arg);
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
        CHECK_RETURN(self != nullptr, "self is nullptr", napi_invalid_arg);
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
        CHECK_RETURN(self != nullptr, "self is nullptr", napi_invalid_arg);
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
        CHECK_RETURN(self != nullptr, "self is nullptr", napi_invalid_arg);
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
        CHECK_RETURN(self != nullptr, "self is nullptr", napi_invalid_arg);
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
    InputAttribute inputAttribute;
    GetEditorAttributeContext() : Context(nullptr, nullptr){};
    GetEditorAttributeContext(InputAction input, OutputAction output) : Context(std::move(input), std::move(output)){};
    napi_status operator()(napi_env env, napi_value *result) override
    {
        if (status_ != napi_ok) {
            output_ = nullptr;
            return status_;
        }
        return Context::operator()(env, result);
    }
};

struct SelectContext : public AsyncCall::Context {
    int32_t start = 0;
    int32_t end = 0;
    int32_t direction = 0;
    napi_status status = napi_generic_failure;
    SelectContext() : Context(nullptr, nullptr){};
    SelectContext(InputAction input, OutputAction output) : Context(std::move(input), std::move(output)){};
    napi_status operator()(napi_env env, size_t argc, napi_value *argv, napi_value self) override
    {
        CHECK_RETURN(self != nullptr, "self is nullptr", napi_invalid_arg);
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
        CHECK_RETURN(self != nullptr, "self is nullptr", napi_invalid_arg);
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

struct SendExtendActionContext : public AsyncCall::Context {
    int32_t action = 0;
    SendExtendActionContext() : Context(nullptr, nullptr){};
    SendExtendActionContext(InputAction input, OutputAction output) : Context(std::move(input), std::move(output)){};

    napi_status operator()(napi_env env, size_t argc, napi_value *argv, napi_value self) override
    {
        CHECK_RETURN(self != nullptr, "self is nullptr", napi_invalid_arg);
        return Context::operator()(env, argc, argv, self);
    }
    napi_status operator()(napi_env env, napi_value *result) override
    {
        if (status_ != napi_ok) {
            output_ = nullptr;
            return status_;
        }
        return Context::operator()(env, result);
    }
};

struct SendPrivateCommandContext : public AsyncCall::Context {
    std::unordered_map<std::string, PrivateDataValue> privateCommand;
    PrivateCommandInfo info;
    napi_status status = napi_generic_failure;
    SendPrivateCommandContext() : Context(nullptr, nullptr){};

    napi_status operator()(napi_env env, size_t argc, napi_value *argv, napi_value self) override
    {
        CHECK_RETURN(self != nullptr, "self is nullptr", napi_invalid_arg);
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

struct GetCallingWindowInfoContext : public AsyncCall::Context {
    CallingWindowInfo windowInfo{};
    GetCallingWindowInfoContext() : Context(nullptr, nullptr){};
    napi_status operator()(napi_env env, napi_value *result) override
    {
        if (status_ != napi_ok) {
            output_ = nullptr;
            return status_;
        }
        return Context::operator()(env, result);
    }
};

struct SetPreviewTextContext : public AsyncCall::Context {
    std::string text;
    Range range;
    SetPreviewTextContext() : Context(nullptr, nullptr){};
    napi_status operator()(napi_env env, size_t argc, napi_value *argv, napi_value self) override
    {
        CHECK_RETURN(self != nullptr, "self is nullptr", napi_invalid_arg);
        return Context::operator()(env, argc, argv, self);
    }
    napi_status operator()(napi_env env, napi_value *result) override
    {
        if (status_ != napi_ok) {
            output_ = nullptr;
            return status_;
        }
        return Context::operator()(env, result);
    }
};

struct FinishTextPreviewContext : public AsyncCall::Context {
    FinishTextPreviewContext() : Context(nullptr, nullptr){};
    napi_status operator()(napi_env env, size_t argc, napi_value *argv, napi_value self) override
    {
        CHECK_RETURN(self != nullptr, "self is nullptr", napi_invalid_arg);
        return Context::operator()(env, argc, argv, self);
    }
    napi_status operator()(napi_env env, napi_value *result) override
    {
        if (status_ != napi_ok) {
            output_ = nullptr;
            return status_;
        }
        return Context::operator()(env, result);
    }
};

class JsTextInputClientEngine : public TextInputClientListener {
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
    static napi_value SelectByRange(napi_env env, napi_callback_info info);
    static napi_value SelectByMovement(napi_env env, napi_callback_info info);
    static napi_value SendExtendAction(napi_env env, napi_callback_info info);
    static napi_value InsertTextSync(napi_env env, napi_callback_info info);
    static napi_value MoveCursorSync(napi_env env, napi_callback_info info);
    static napi_value GetEditorAttributeSync(napi_env env, napi_callback_info info);
    static napi_value SelectByRangeSync(napi_env env, napi_callback_info info);
    static napi_value SelectByMovementSync(napi_env env, napi_callback_info info);
    static napi_value GetTextIndexAtCursorSync(napi_env env, napi_callback_info info);
    static napi_value DeleteForwardSync(napi_env env, napi_callback_info info);
    static napi_value DeleteBackwardSync(napi_env env, napi_callback_info info);
    static napi_value GetForwardSync(napi_env env, napi_callback_info info);
    static napi_value GetBackwardSync(napi_env env, napi_callback_info info);
    static napi_value GetCallingWindowInfo(napi_env env, napi_callback_info info);
    static napi_value SendPrivateCommand(napi_env env, napi_callback_info info);
    static napi_value SetPreviewText(napi_env env, napi_callback_info info);
    static napi_value SetPreviewTextSync(napi_env env, napi_callback_info info);
    static napi_value FinishTextPreview(napi_env env, napi_callback_info info);
    static napi_value FinishTextPreviewSync(napi_env env, napi_callback_info info);
    static napi_value SendMessage(napi_env env, napi_callback_info info);
    static napi_value RecvMessage(napi_env env, napi_callback_info info);
    static napi_value GetAttachOptions(napi_env env, napi_callback_info info);
    static napi_value Subscribe(napi_env env, napi_callback_info info);
    static napi_value UnSubscribe(napi_env env, napi_callback_info info);
    void OnAttachOptionsChanged(const AttachOptions &attachOptions) override;
    class JsMessageHandler : public MsgHandlerCallbackInterface {
    public:
        explicit JsMessageHandler(napi_env env, napi_value onTerminated, napi_value onMessage)
            : jsMessageHandler_(std::make_shared<JSMsgHandlerCallbackObject>(env, onTerminated, onMessage)) {};
        virtual ~JsMessageHandler() {};
        int32_t OnTerminated() override;
        int32_t OnMessage(const ArrayBuffer &arrayBuffer) override;

    private:
        std::mutex callbackObjectMutex_;
        std::shared_ptr<JSMsgHandlerCallbackObject> jsMessageHandler_ = nullptr;
    };
private:
    static napi_status GetSelectRange(napi_env env, napi_value argv, std::shared_ptr<SelectContext> ctxt);
    static napi_status GetSelectMovement(napi_env env, napi_value argv, std::shared_ptr<SelectContext> ctxt);

    static napi_value JsConstructor(napi_env env, napi_callback_info info);
    static std::shared_ptr<JsTextInputClientEngine> GetTextInputClientEngine();
    static bool InitTextInputClientEngine();
    static napi_value GetResult(napi_env env, std::string &text);
    static napi_value GetResultEditorAttribute(napi_env env,
        std::shared_ptr<GetEditorAttributeContext> getEditorAttribute);
    static napi_value HandleParamCheckFailure(napi_env env);
    static napi_status GetPreviewTextParam(napi_env env, size_t argc, napi_value *argv, std::string &text,
        Range &range);
    static bool IsTargetDeviceType(int32_t resDeviceType);
    void RegisterListener(napi_value callback, std::string type, std::shared_ptr<JSCallbackObject> callbackObj);
    void UnRegisterListener(napi_value callback, std::string type);

    struct UvEntry {
        std::vector<std::shared_ptr<JSCallbackObject>> vecCopy;
        std::string type;
        AttachOptions attachOptions;
        explicit UvEntry(const std::vector<std::shared_ptr<JSCallbackObject>> &cbVec, const std::string &type)
            : vecCopy(cbVec), type(type)
        {
        }
    };
    using EntrySetter = std::function<void(UvEntry &)>;
    static std::shared_ptr<AppExecFwk::EventHandler> GetEventHandler();
    std::shared_ptr<UvEntry> GetEntry(const std::string &type, EntrySetter entrySetter = nullptr);
    std::recursive_mutex mutex_;
    std::map<std::string, std::vector<std::shared_ptr<JSCallbackObject>>> jsCbMap_;

    static const std::string TIC_CLASS_NAME;
    static thread_local napi_ref TICRef_;
    static std::shared_ptr<AsyncCall::TaskQueue> taskQueue_;
    static BlockQueue<PrivateCommandInfo> privateCommandQueue_;
    static std::string GenerateTraceId()
    {
        auto traceId = traceId_++;
        return std::to_string(traceId);
    }
    static uint32_t traceId_;

    static BlockQueue<MessageHandlerInfo> messageHandlerQueue_;
    static std::mutex engineMutex_;
    static std::shared_ptr<JsTextInputClientEngine> textInputClientEngine_;
    static std::mutex eventHandlerMutex_;
    static std::shared_ptr<AppExecFwk::EventHandler> handler_;
    static int32_t deviceTypeCache_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // INTERFACE_KITS_JS_NAPI_INPUTMETHODENGINE_INCLUDE_JS_TEXT_INPUT_CLIENT_H