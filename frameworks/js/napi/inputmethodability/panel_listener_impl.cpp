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

#include "panel_listener_impl.h"

#include "js_callback_handler.h"
#include "js_utils.h"

namespace OHOS {
namespace MiscServices {
std::shared_ptr<PanelListenerImpl> PanelListenerImpl::instance_{ nullptr };
std::mutex PanelListenerImpl::listenerMutex_;
std::shared_ptr<PanelListenerImpl> PanelListenerImpl::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> lock(listenerMutex_);
        if (instance_ == nullptr) {
            instance_ = std::make_shared<PanelListenerImpl>();
        }
    }
    return instance_;
}

PanelListenerImpl::~PanelListenerImpl()
{
}

void PanelListenerImpl::SaveInfo(napi_env env, const std::string &type, napi_value callback, uint32_t windowId)
{
    std::shared_ptr<JSCallbackObject> cbObject =
        std::make_shared<JSCallbackObject>(env, callback, std::this_thread::get_id());
    auto result = callbacks_.Find(windowId);
    if (!result.first) {
        IMSA_HILOGI("start to subscribe, type: %{public}s, windowId: %{public}u.", type.c_str(), windowId);
        ConcurrentMap<std::string, std::shared_ptr<JSCallbackObject>> cbs{};
        cbs.Insert(type, cbObject);
        callbacks_.Insert(windowId, cbs);
    } else {
        auto res = result.second.Find(type);
        if (res.first) {
            IMSA_HILOGD("type: %{public}s of windowId: %{public}u already subscribed.", type.c_str(), windowId);
            return;
        }
        IMSA_HILOGI("start to subscribe type: %{public}s of windowId: %{public}u.", type.c_str(), windowId);
        result.second.Insert(type, cbObject);
        callbacks_.InsertOrAssign(windowId, result.second);
    }
}

void PanelListenerImpl::RemoveInfo(const std::string &type, uint32_t windowId)
{
    auto result = callbacks_.Find(windowId);
    if (result.first) {
        result.second.Erase(type);
        if (result.second.Empty()) {
            callbacks_.Erase(windowId);
        }
    }
}

void PanelListenerImpl::OnPanelStatus(uint32_t windowId, bool isShow)
{
    std::string type = isShow ? "show" : "hide";
    auto result = callbacks_.Find(windowId);
    if (!result.first) {
        IMSA_HILOGE("no callback of windowId = %{public}d!", windowId);
        return;
    }
    auto callback = result.second.Find(type);
    if (!callback.first) {
        IMSA_HILOGE("no callback in map!");
        return;
    }
    auto entry = std::make_shared<UvEntry>(callback.second);
    if (entry == nullptr) {
        IMSA_HILOGE("entry is nullptr!");
        return;
    }
    auto eventHandler = GetEventHandler();
    if (eventHandler == nullptr) {
        IMSA_HILOGE("eventHandler is nullptr!");
        return;
    }
    IMSA_HILOGI("windowId = %{public}u, type = %{public}s", windowId, type.c_str());
    auto task = [entry]() { JsCallbackHandler::Traverse({ entry->cbCopy }); };
    eventHandler->PostTask(task, type);
}

void PanelListenerImpl::OnSizeChange(uint32_t windowId, const WindowSize &size)
{
    std::string type = "sizeChange";
    auto callback = GetCallback(type, windowId);
    if (callback == nullptr) {
        IMSA_HILOGE("callback not found! type: %{public}s", type.c_str());
        return;
    }
    auto entry = GetEntry(callback, [&size](UvEntry &entry) { entry.size = size; });
    if (entry == nullptr) {
        IMSA_HILOGD("entry is nullptr!");
        return;
    }
    auto eventHandler = GetEventHandler();
    if (eventHandler == nullptr) {
        IMSA_HILOGE("eventHandler is nullptr!");
        return;
    }

    IMSA_HILOGI("start.");
    auto task = [entry]() {
        auto gitWindowSizeParams = [entry](napi_env env, napi_value *args, uint8_t argc) -> bool {
            if (argc == 0) {
                return false;
            }
            napi_value windowSize = JsWindowSize::Write(env, entry->size);
            // 0 means the first param of callback.
            args[0] = { windowSize };
            return true;
        };
        JsCallbackHandler::Traverse({ entry->cbCopy }, { 1, gitWindowSizeParams });
    };
    eventHandler->PostTask(task, type);
}

std::shared_ptr<PanelListenerImpl::UvEntry> PanelListenerImpl::GetEntry(
    const std::shared_ptr<JSCallbackObject> &callback, EntrySetter entrySetter)
{
    std::shared_ptr<UvEntry> entry = std::make_shared<UvEntry>(callback);
    if (entrySetter != nullptr) {
        entrySetter(*entry);
    }
    return entry;
}

std::shared_ptr<JSCallbackObject> PanelListenerImpl::GetCallback(const std::string &type, uint32_t windowId)
{
    IMSA_HILOGD("start, type: %{public}s, windowId: %{public}d", type.c_str(), windowId);
    auto result = callbacks_.Find(windowId);
    if (!result.first) {
        IMSA_HILOGE("no callback of windowId: %{public}d!", windowId);
        return nullptr;
    }
    auto callback = result.second.Find(type);
    if (!callback.first) {
        IMSA_HILOGE("no callback in map, type: %{public}s", type.c_str());
        return nullptr;
    }
    return callback.second;
}

void PanelListenerImpl::SetEventHandler(std::shared_ptr<AppExecFwk::EventHandler> handler)
{
    std::lock_guard<std::mutex> lock(eventHandlerMutex_);
    handler_ = handler;
}

std::shared_ptr<AppExecFwk::EventHandler> PanelListenerImpl::GetEventHandler()
{
    std::lock_guard<std::mutex> lock(eventHandlerMutex_);
    return handler_;
}

napi_value JsWindowSize::Write(napi_env env, const WindowSize &nativeObject)
{
    napi_value jsObject = nullptr;
    napi_create_object(env, &jsObject);
    bool ret = JsUtil::Object::WriteProperty(env, jsObject, "width", nativeObject.width);
    ret = ret && JsUtil::Object::WriteProperty(env, jsObject, "height", nativeObject.height);
    return ret ? jsObject : JsUtil::Const::Null(env);
}

bool JsWindowSize::Read(napi_env env, napi_value jsObject, WindowSize &nativeObject)
{
    auto ret = JsUtil::Object::ReadProperty(env, jsObject, "width", nativeObject.width);
    ret = ret && JsUtil::Object::ReadProperty(env, jsObject, "height", nativeObject.height);
    return ret;
}
} // namespace MiscServices
} // namespace OHOS