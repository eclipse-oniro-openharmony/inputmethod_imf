/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef FRAMEWORKS_JS_NAPI_JS_KEYBOARD_PANEL_MANAGER_H
#define FRAMEWORKS_JS_NAPI_JS_KEYBOARD_PANEL_MANAGER_H

#include "async_call.h"
#include "block_queue.h"
#include "event_handler.h"
#include "js_callback_object.h"
#include "ime_system_channel.h"

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

struct SendPrivateCommandContext : public AsyncCall::Context {
    std::unordered_map<std::string, PrivateDataValue> privateCommand;
    PrivateCommandInfo info;
    SendPrivateCommandContext() : Context(nullptr, nullptr){};

    napi_status operator()(napi_env env, napi_value *result) override
    {
        if (status_ != napi_ok) {
            output_ = nullptr;
            return status_;
        }
        return Context::operator()(env, result);
    }
};

struct SmartMenuContext : public AsyncCall::Context {
    std::string smartMenu;
    SmartMenuContext() : Context(nullptr, nullptr){};

    napi_status operator()(napi_env env, napi_value *result) override
    {
        if (status_ != napi_ok) {
            output_ = nullptr;
            return status_;
        }
        return Context::operator()(env, result);
    }
};

class JsKeyboardPanelManager : public OnSystemCmdListener {
public:
    JsKeyboardPanelManager();
    ~JsKeyboardPanelManager() = default;

    static napi_value Init(napi_env env, napi_value info);
    static sptr<JsKeyboardPanelManager> GetInstance();
    static napi_value SendPrivateCommand(napi_env env, napi_callback_info info);
    static napi_value GetSmartMenuCfg(napi_env env, napi_callback_info info);
    static napi_value Subscribe(napi_env env, napi_callback_info info);
    static napi_value UnSubscribe(napi_env env, napi_callback_info info);

    void ReceivePrivateCommand(const std::unordered_map<std::string, PrivateDataValue> &privateCommand) override;
    void NotifyIsShowSysPanel(bool shouldSysPanelShow) override;

private:
    void RegisterListener(napi_value callback, std::string type, std::shared_ptr<JSCallbackObject> callbackObj);
    void UnRegisterListener(napi_value callback, std::string type);

    struct UvEntry {
        std::vector<std::shared_ptr<JSCallbackObject>> vecCopy;
        std::string type;
        bool shouldSysPanelShow = false;
        std::string smartMenu;
        std::unordered_map<std::string, PrivateDataValue> privateCommand;
        explicit UvEntry(const std::vector<std::shared_ptr<JSCallbackObject>> &cbVec, const std::string &type)
            : vecCopy(cbVec), type(type), shouldSysPanelShow(false), smartMenu(""), privateCommand({})
        {
        }
    };
    using EntrySetter = std::function<void(UvEntry &)>;
    std::shared_ptr<AppExecFwk::EventHandler> GetEventHandler();
    std::shared_ptr<UvEntry> GetEntry(const std::string &type, EntrySetter entrySetter = nullptr);

    std::recursive_mutex mutex_;
    std::map<std::string, std::vector<std::shared_ptr<JSCallbackObject>>> jsCbMap_;
    std::mutex eventHandlerMutex_;
    std::shared_ptr<AppExecFwk::EventHandler> handler_;
    static std::mutex managerMutex_;
    static sptr<JsKeyboardPanelManager> keyboardPanelManager_;
    static BlockQueue<PrivateCommandInfo> privateCommandQueue_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // FRAMEWORKS_JS_NAPI_JS_KEYBOARD_PANEL_MANAGER_H