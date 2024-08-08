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

#ifndef INPUTMETHOD_IMF_JSPANEL_H
#define INPUTMETHOD_IMF_JSPANEL_H

#include <mutex>
#include <uv.h>

#include "async_call.h"
#include "ffrt_block_queue.h"
#include "input_method_panel.h"
#include "js_callback_object.h"
#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"
#include "native_engine/native_engine.h"
#include "native_engine/native_value.h"

namespace OHOS {
namespace MiscServices {

enum class JsEvent : uint32_t {
    RESIZE = 0,
    MOVE_TO,
    CHANGE_FLAG,
    ADJUST_PANEL_RECT,
    EVENT_END,
};

struct JsEventInfo {
    std::chrono::system_clock::time_point timestamp{};
    JsEvent event{ JsEvent::EVENT_END };
    bool operator==(const JsEventInfo &info) const
    {
        return (timestamp == info.timestamp && event == info.event);
    }
};

struct JsPanelRect {
    static napi_value Write(napi_env env, const LayoutParams &layoutParams);
    static bool Read(napi_env env, napi_value object, LayoutParams &layoutParams);
};

class JsPanel {
public:
    JsPanel() = default;
    ~JsPanel();
    static napi_value Init(napi_env env);
    static napi_value SetUiContent(napi_env env, napi_callback_info info);
    static napi_value Resize(napi_env env, napi_callback_info info);
    static napi_value MoveTo(napi_env env, napi_callback_info info);
    static napi_value Show(napi_env env, napi_callback_info info);
    static napi_value Hide(napi_env env, napi_callback_info info);
    static napi_value ChangeFlag(napi_env env, napi_callback_info info);
    static napi_value SetPrivacyMode(napi_env env, napi_callback_info info);
    static napi_value Subscribe(napi_env env, napi_callback_info info);
    static napi_value UnSubscribe(napi_env env, napi_callback_info info);
    static napi_value AdjustPanelRect(napi_env env, napi_callback_info info);
    void SetNative(const std::shared_ptr<InputMethodPanel> &panel);
    std::shared_ptr<InputMethodPanel> GetNative();

private:
    struct PanelContentContext : public AsyncCall::Context {
        LayoutParams layoutParams = { { 0, 0, 0, 0 }, { 0, 0, 0, 0 } };
        PanelFlag panelFlag = PanelFlag::FLG_FIXED;
        std::string path = "";
        uint32_t width = 0;
        uint32_t height = 0;
        int32_t x = 0;
        int32_t y = 0;
        std::shared_ptr<InputMethodPanel> inputMethodPanel = nullptr;
        std::shared_ptr<NativeReference> contentStorage = nullptr;
        JsEventInfo info;
        PanelContentContext(napi_env env, napi_callback_info info) : Context(nullptr, nullptr)
        {
            napi_value self = nullptr;
            napi_status status = napi_get_cb_info(env, info, 0, nullptr, &self, nullptr);
            CHECK_RETURN_VOID((status == napi_ok) && (self != nullptr), "get callback info failed.");
            void *native = nullptr;
            status = napi_unwrap(env, self, &native);
            CHECK_RETURN_VOID((status == napi_ok) && (native != nullptr), "get jsPanel failed.");
            inputMethodPanel = reinterpret_cast<JsPanel *>(native)->GetNative();
        };
        PanelContentContext(InputAction input, OutputAction output) : Context(std::move(input), std::move(output)){};
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
    static napi_value JsNew(napi_env env, napi_callback_info info);
    static std::shared_ptr<InputMethodPanel> UnwrapPanel(napi_env env, napi_value thisVar);
    static void PrintEditorQueueInfoIfTimeout(int64_t start, const JsEventInfo &currentInfo);
    static napi_status CheckParam(napi_env env, size_t argc, napi_value *argv,
        std::shared_ptr<PanelContentContext> ctxt);
    static const std::string CLASS_NAME;
    static constexpr size_t ARGC_MAX = 6;
    std::shared_ptr<InputMethodPanel> inputMethodPanel_ = nullptr;

    static std::mutex panelConstructorMutex_;
    static thread_local napi_ref panelConstructorRef_;

    static FFRTBlockQueue<JsEventInfo> jsQueue_;
};
} // namespace MiscServices
} // namespace OHOS

#endif //INPUTMETHOD_IMF_JSPANEL_H
