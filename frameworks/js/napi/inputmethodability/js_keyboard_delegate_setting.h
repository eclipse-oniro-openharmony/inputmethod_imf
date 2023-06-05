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

#ifndef INTERFACE_KITS_JS_KEYBOARD_DELEGATE_SETTING_H
#define INTERFACE_KITS_JS_KEYBOARD_DELEGATE_SETTING_H

#include <uv.h>

#include <map>
#include <memory>
#include <mutex>

#include "async_call.h"
#include "block_data.h"
#include "global.h"
#include "js_callback_object.h"
#include "keyboard_listener.h"
#include "napi/native_api.h"

namespace OHOS {
namespace MiscServices {
class JsKeyboardDelegateSetting : public KeyboardListener {
public:
    JsKeyboardDelegateSetting() = default;
    ~JsKeyboardDelegateSetting() override = default;
    static napi_value Init(napi_env env, napi_value exports);
    static napi_value CreateKeyboardDelegate(napi_env env, napi_callback_info info);
    static napi_value GetKeyboardDelegate(napi_env env, napi_callback_info info);
    static napi_value Subscribe(napi_env env, napi_callback_info info);
    static napi_value UnSubscribe(napi_env env, napi_callback_info info);
    bool OnKeyEvent(int32_t keyCode, int32_t keyStatus) override;
    void OnCursorUpdate(int32_t positionX, int32_t positionY, int32_t height) override;
    void OnSelectionChange(int32_t oldBegin, int32_t oldEnd, int32_t newBegin, int32_t newEnd) override;
    void OnTextChange(const std::string &text) override;

private:
    static napi_value GetResultOnKeyEvent(napi_env env, int32_t keyCode, int32_t keyStatus);
    static napi_value GetJsConstProperty(napi_env env, uint32_t num);
    static napi_value GetKDInstance(napi_env env, napi_callback_info info);
    static std::shared_ptr<JsKeyboardDelegateSetting> GetKeyboardDelegateSetting();
    static bool InitKeyboardDelegate();
    static napi_value JsConstructor(napi_env env, napi_callback_info cbinfo);
    void RegisterListener(napi_value callback, std::string type, std::shared_ptr<JSCallbackObject> callbackObj);
    void UnRegisterListener(napi_value callback, std::string type);
    static constexpr int32_t MAX_TIMEOUT = 100;
    static const std::string KDS_CLASS_NAME;
    static thread_local napi_ref KDSRef_;
    struct CursorPara {
        int32_t positionX = 0;
        int32_t positionY = 0;
        int height = 0;
    };
    struct SelectionPara {
        int32_t oldBegin = 0;
        int32_t oldEnd = 0;
        int32_t newBegin = 0;
        int32_t newEnd = 0;
    };
    struct KeyEventPara {
        int32_t keyCode = 0;
        int32_t keyStatus = 0;
        bool isOnKeyEvent = false;
    };
    struct UvEntry {
        std::vector<std::shared_ptr<JSCallbackObject>> vecCopy;
        std::string type;
        CursorPara curPara;
        SelectionPara selPara;
        KeyEventPara keyEventPara;
        std::shared_ptr<BlockData<bool>> isDone;
        std::string text;
        UvEntry(const std::vector<std::shared_ptr<JSCallbackObject>> &cbVec, const std::string &type)
            : vecCopy(cbVec), type(type)
        {
        }
    };
    using EntrySetter = std::function<void(UvEntry &)>;
    uv_work_t *GetUVwork(const std::string &type, EntrySetter entrySetter = nullptr);
    uv_loop_s *loop_ = nullptr;
    std::recursive_mutex mutex_;
    std::map<std::string, std::vector<std::shared_ptr<JSCallbackObject>>> jsCbMap_;
    static std::mutex keyboardMutex_;
    static std::shared_ptr<JsKeyboardDelegateSetting> keyboardDelegate_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // INTERFACE_KITS_JS_KEYBOARD_DELEGATE_SETTING_H
