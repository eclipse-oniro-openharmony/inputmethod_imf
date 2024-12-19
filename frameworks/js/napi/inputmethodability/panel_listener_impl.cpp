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

PanelListenerImpl::~PanelListenerImpl() {}

void PanelListenerImpl::Subscribe(uint32_t windowId, const std::string &type,
    std::shared_ptr<JSCallbackObject> cbObject)
{
    callbacks_.Compute(windowId,
        [cbObject, &type](auto windowId, std::map<std::string, std::shared_ptr<JSCallbackObject>> &cbs) {
            auto [it, insert] = cbs.try_emplace(type, cbObject);
            if (insert) {
                IMSA_HILOGI("start to subscribe type: %{public}s of windowId: %{public}u.", type.c_str(), windowId);
            } else {
                IMSA_HILOGD("type: %{public}s of windowId: %{public}u already subscribed.", type.c_str(), windowId);
            }
            return !cbs.empty();
        });
}

void PanelListenerImpl::RemoveInfo(const std::string &type, uint32_t windowId)
{
    callbacks_.ComputeIfPresent(windowId,
        [&type](auto windowId, std::map<std::string, std::shared_ptr<JSCallbackObject>> &cbs) {
            cbs.erase(type);
            return !cbs.empty();
        });
}

void PanelListenerImpl::OnPanelStatus(uint32_t windowId, bool isShow)
{
    std::string type = isShow ? "show" : "hide";
    auto eventHandler = GetEventHandler();
    if (eventHandler == nullptr) {
        IMSA_HILOGE("eventHandler is nullptr!");
        return;
    }
    std::shared_ptr<JSCallbackObject> callBack = GetCallback(windowId, type);
    if (callBack == nullptr) {
        IMSA_HILOGE("callBack is nullptr!");
        return;
    }
    auto entry = std::make_shared<UvEntry>(callBack);
    IMSA_HILOGI("windowId = %{public}u, type = %{public}s", windowId, type.c_str());
    auto task = [entry]() {
        JsCallbackHandler::Traverse({ entry->cbCopy });
    };
    eventHandler->PostTask(task, type, 0, AppExecFwk::EventQueue::Priority::VIP);
}

void PanelListenerImpl::OnSizeChange(uint32_t windowId, const WindowSize &size)
{
    std::string type = "sizeChange";
    auto eventHandler = GetEventHandler();
    if (eventHandler == nullptr) {
        IMSA_HILOGE("eventHandler is nullptr!");
        return;
    }
    std::shared_ptr<JSCallbackObject> callBack = GetCallback(windowId, type);
    if (callBack == nullptr) {
        return;
    }
    auto entry = std::make_shared<UvEntry>(callBack);
    entry->size = size;
    IMSA_HILOGI("OnSizeChange start. windowId:%{public}u, w:%{public}u, h:%{public}u", windowId, size.width,
        size.height);
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
    eventHandler->PostTask(task, type, 0, AppExecFwk::EventQueue::Priority::VIP);
}

void PanelListenerImpl::SetEventHandler(std::shared_ptr<AppExecFwk::EventHandler> handler)
{
    std::unique_lock<decltype(eventHandlerMutex_)> lock(eventHandlerMutex_);
    handler_ = handler;
}

std::shared_ptr<AppExecFwk::EventHandler> PanelListenerImpl::GetEventHandler()
{
    std::shared_lock<decltype(eventHandlerMutex_)> lock(eventHandlerMutex_);
    return handler_;
}

std::shared_ptr<JSCallbackObject> PanelListenerImpl::GetCallback(uint32_t windowId, const std::string &type)
{
    std::shared_ptr<JSCallbackObject> callBack = nullptr;
    callbacks_.ComputeIfPresent(windowId, [&type, &callBack](uint32_t id, auto callbacks) {
        auto it = callbacks.find(type);
        if (it == callbacks.end()) {
            return !callbacks.empty();
        }
        callBack = it->second;
        return !callbacks.empty();
    });
    return callBack;
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
