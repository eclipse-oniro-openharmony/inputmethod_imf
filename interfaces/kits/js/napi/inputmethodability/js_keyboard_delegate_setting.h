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

#include <map>
#include <uv.h>
#include <mutex>
#include "napi/native_api.h"
#include "global.h"
#include "async_call.h"
#include "keyboard_listener.h"
#include "js_callback_object.h"

namespace OHOS {
namespace MiscServices {
class JsKeyboardDelegateSetting : public KeyboardListener {
public:
    JsKeyboardDelegateSetting() = default;
    ~JsKeyboardDelegateSetting() override = default;
    static napi_value Init(napi_env env, napi_value info);
    static napi_value CreateKeyboardDelegate(napi_env env, napi_callback_info info);
    static napi_value Subscribe(napi_env env, napi_callback_info info);
    static napi_value UnSubscribe(napi_env env, napi_callback_info info);
    bool OnKeyEvent(int32_t keyCode, int32_t keyStatus) override;
    void OnCursorUpdate(int32_t positionX, int32_t positionY, int32_t height) override;
    void OnSelectionChange(int32_t oldBegin, int32_t oldEnd, int32_t newBegin, int32_t newEnd) override;
    void OnTextChange(std::string text) override;
private:
    static napi_value GetResultOnKeyEvent(napi_env env, int32_t keyCode, int32_t keyStatus);
    static napi_value GetJsConstProperty(napi_env env, uint32_t num);
    static napi_value JsConstructor(napi_env env, napi_callback_info cbinfo);
    static JsKeyboardDelegateSetting *GetNative(napi_env env, napi_callback_info info);
    static bool Equals(napi_env env, napi_value value, napi_ref copy);
    void RegisterListener(napi_value callback, std::string type,
        std::shared_ptr<JSCallbackObject> callbackObj);
    void UnRegisterListener(napi_value callback, std::string type);

    static std::string GetStringProperty(napi_env env, napi_value obj);
    static constexpr int32_t MAX_VALUE_LEN = 1024;
    static const std::string KDS_CLASS_NAME;
    static thread_local napi_ref KDSRef_;
    struct CursorPara {
        int32_t positionX;
        int32_t positionY;
        int height;
    };
    struct SelectionPara {
        int32_t oldBegin;
        int32_t oldEnd;
        int32_t newBegin;
        int32_t newEnd;
    };
    struct UvEntry {
        std::vector<std::shared_ptr<JSCallbackObject>> vecCopy;
        std::string type;
        CursorPara curPara;
        SelectionPara selPara;
        std::string text;
        UvEntry(std::vector<std::shared_ptr<JSCallbackObject>> cbVec, std::string type)
            : vecCopy(cbVec), type(type) {}
    };
    uv_work_t *GetCursorUVwork(std::string type, CursorPara para);
    uv_work_t *GetSelectionUVwork(std::string type, SelectionPara para);
    uv_work_t *GetTextUVwork(std::string type, std::string text);
    uv_loop_s *loop_ = nullptr;
    std::recursive_mutex mutex_;
    std::map<std::string, std::vector<std::shared_ptr<JSCallbackObject>>> jsCbMap_;
};
}
}
#endif // INTERFACE_KITS_JS_KEYBOARD_DELEGATE_SETTING_H