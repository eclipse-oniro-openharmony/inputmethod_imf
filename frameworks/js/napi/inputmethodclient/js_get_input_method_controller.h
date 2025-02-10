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
#ifndef INTERFACE_KITS_JS_GET_INPUT_METHOD_CONTROLLER_H
#define INTERFACE_KITS_JS_GET_INPUT_METHOD_CONTROLLER_H

#include "async_call.h"
#include "controller_listener.h"
#include "event_handler.h"
#include "global.h"
#include "js_callback_object.h"
#include "js_input_method.h"
#include "js_message_handler_info.h"
#include "msg_handler_callback_interface.h"

namespace OHOS {
namespace MiscServices {
struct HandleContext : public AsyncCall::Context {
    bool isHandle = false;
    napi_status status = napi_generic_failure;
    HandleContext() : Context(nullptr, nullptr){};
    HandleContext(InputAction input, OutputAction output) : Context(std::move(input), std::move(output)){};

    napi_status operator()(napi_env env, size_t argc, napi_value *argv, napi_value self) override
    {
        CHECK_RETURN(self != nullptr, "self is nullptr!", napi_invalid_arg);
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

struct AttachContext : public AsyncCall::Context {
    sptr<OnTextChangedListener> textListener;
    InputAttribute attribute;
    bool showKeyboard = false;
    TextConfig textConfig;
    AttachContext() : Context(nullptr, nullptr) {};
    AttachContext(InputAction input, OutputAction output) : Context(std::move(input), std::move(output)) {};

    napi_status operator()(napi_env env, size_t argc, napi_value *argv, napi_value self) override
    {
        CHECK_RETURN(self != nullptr, "self is nullptr!", napi_invalid_arg);
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

struct SetCallingWindowContext : public AsyncCall::Context {
    int32_t windID = 0;
    SetCallingWindowContext() : Context(nullptr, nullptr) {};
    SetCallingWindowContext(InputAction input, OutputAction output) : Context(std::move(input), std::move(output)) {};

    napi_status operator()(napi_env env, size_t argc, napi_value *argv, napi_value self) override
    {
        CHECK_RETURN(self != nullptr, "self is nullptr!", napi_invalid_arg);
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

struct UpdateCursorContext : public AsyncCall::Context {
    CursorInfo cursorInfo;
    UpdateCursorContext() : Context(nullptr, nullptr) {};
    UpdateCursorContext(InputAction input, OutputAction output) : Context(std::move(input), std::move(output)) {};

    napi_status operator()(napi_env env, size_t argc, napi_value *argv, napi_value self) override
    {
        CHECK_RETURN(self != nullptr, "self is nullptr!", napi_invalid_arg);
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

struct ChangeSelectionContext : public AsyncCall::Context {
    std::u16string text;
    int32_t start = 0;
    int32_t end = 0;
    ChangeSelectionContext() : Context(nullptr, nullptr) {};
    ChangeSelectionContext(InputAction input, OutputAction output) : Context(std::move(input), std::move(output)) {};

    napi_status operator()(napi_env env, size_t argc, napi_value *argv, napi_value self) override
    {
        CHECK_RETURN(self != nullptr, "self is nullptr!", napi_invalid_arg);
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

struct UpdateAttributeContext : public AsyncCall::Context {
    InputAttribute attribute;
    Configuration configuration;
    UpdateAttributeContext() : Context(nullptr, nullptr) {};
    UpdateAttributeContext(InputAction input, OutputAction output) : Context(std::move(input), std::move(output)) {};

    napi_status operator()(napi_env env, size_t argc, napi_value *argv, napi_value self) override
    {
        CHECK_RETURN(self != nullptr, "self is nullptr!", napi_invalid_arg);
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

class JsGetInputMethodController : public ControllerListener {
public:
    JsGetInputMethodController() = default;
    ~JsGetInputMethodController() = default;
    static napi_value Init(napi_env env, napi_value info);
    static napi_value GetController(napi_env env, napi_callback_info cbInfo);
    static napi_value GetInputMethodController(napi_env env, napi_callback_info cbInfo);
    static std::shared_ptr<JsGetInputMethodController> GetInstance();
    static napi_value HandleSoftKeyboard(napi_env env, napi_callback_info info, std::function<int32_t()> callback,
        bool isOutput, bool needThrowException);
    static napi_value Attach(napi_env env, napi_callback_info info);
    static napi_value Detach(napi_env env, napi_callback_info info);
    static napi_value ShowTextInput(napi_env env, napi_callback_info info);
    static napi_value HideTextInput(napi_env env, napi_callback_info info);
    static napi_value SetCallingWindow(napi_env env, napi_callback_info info);
    static napi_value UpdateCursor(napi_env env, napi_callback_info info);
    static napi_value ChangeSelection(napi_env env, napi_callback_info info);
    static napi_value UpdateAttribute(napi_env env, napi_callback_info info);
    static napi_value HideSoftKeyboard(napi_env env, napi_callback_info info);
    static napi_value ShowSoftKeyboard(napi_env env, napi_callback_info info);
    static napi_value StopInputSession(napi_env env, napi_callback_info info);
    static napi_value StopInput(napi_env env, napi_callback_info info);
    static napi_value Subscribe(napi_env env, napi_callback_info info);
    static napi_value UnSubscribe(napi_env env, napi_callback_info info);
    static napi_value SendMessage(napi_env env, napi_callback_info info);
    static napi_value RecvMessage(napi_env env, napi_callback_info info);
    void OnSelectByRange(int32_t start, int32_t end) override;
    void OnSelectByMovement(int32_t direction) override;
    void InsertText(const std::u16string &text);
    void DeleteRight(int32_t length);
    void DeleteLeft(int32_t length);
    void SendKeyboardStatus(const KeyboardStatus &status);
    void SendFunctionKey(const FunctionKey &functionKey);
    void MoveCursor(const Direction direction);
    void HandleExtendAction(int32_t action);
    std::u16string GetText(const std::string &type, int32_t number);
    int32_t GetTextIndexAtCursor();

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
    static napi_value JsConstructor(napi_env env, napi_callback_info cbinfo);
    static napi_value GetIMController(napi_env env, napi_callback_info cbInfo, bool needThrowException);
    static napi_value CreateSelectRange(napi_env env, int32_t start, int32_t end);
    static napi_value CreateSelectMovement(napi_env env, int32_t direction);
    static napi_value CreateSendFunctionKey(napi_env env, int32_t functionKey);
    void RegisterListener(napi_value callback, std::string type, std::shared_ptr<JSCallbackObject> callbackObj);
    void UnRegisterListener(napi_value callback, std::string type);
    static bool GetValue(napi_env env, napi_value in, CursorInfo &out);
    static bool GetValue(napi_env env, napi_value in, InputAttribute &out);
    static bool GetValue(napi_env env, napi_value in, TextConfig &out);
    static bool GetValue(napi_env env, napi_value in, Range &out);
    static napi_value GetJsKeyboardStatusProperty(napi_env env);
    static napi_value GetJsEnterKeyTypeProperty(napi_env env);
    static napi_value GetJsTextInputTypeProperty(napi_env env);
    static napi_value GetJsDirectionProperty(napi_env env);
    static napi_value GetJsExtendActionProperty(napi_env env);
    static std::shared_ptr<AppExecFwk::EventHandler> GetEventHandler();
    static const std::set<std::string> TEXT_EVENT_TYPE;
    static constexpr int32_t MAX_TIMEOUT = 2500;
    struct UvEntry {
        std::vector<std::shared_ptr<JSCallbackObject>> vecCopy;
        std::string type;
        std::string text;
        int32_t start = 0;
        int32_t end = 0;
        int32_t direction = 0;
        int32_t length = 0;
        int32_t action = 0;
        int32_t keyboardStatus = 0;
        int32_t enterKeyType = 0;
        int32_t number = 0;
        std::shared_ptr<BlockData<std::string>> textResultHandler;
        std::shared_ptr<BlockData<std::int32_t>> indexResultHandler;
        explicit UvEntry(const std::vector<std::shared_ptr<JSCallbackObject>> &cbVec, const std::string &type)
            : vecCopy(cbVec), type(type)
        {
        }
    };
    using EntrySetter = std::function<void(UvEntry &)>;
    std::shared_ptr<UvEntry> GetEntry(const std::string &type, EntrySetter entrySetter = nullptr);
    uv_loop_s *loop_ = nullptr;
    std::recursive_mutex mutex_;
    std::map<std::string, std::vector<std::shared_ptr<JSCallbackObject>>> jsCbMap_;
    static std::mutex controllerMutex_;
    static std::shared_ptr<JsGetInputMethodController> controller_;
    static const std::string IMC_CLASS_NAME;
    static thread_local napi_ref IMCRef_;
    static std::mutex eventHandlerMutex_;
    static std::shared_ptr<AppExecFwk::EventHandler> handler_;
    static constexpr size_t PARAM_POS_ZERO = 0;
    static constexpr size_t PARAM_POS_ONE = 1;
    static constexpr size_t PARAM_POS_TWO = 2;
    static constexpr size_t PARAM_POS_THREE = 3;
    static BlockQueue<MessageHandlerInfo> messageHandlerQueue_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // INTERFACE_KITS_JS_GET_INPUT_METHOD_CONTROLLER_H
