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
        IMSA_HILOGI("start to subscribe, type = %{public}s, windowId = %{public}u", type.c_str(), windowId);
        ConcurrentMap<std::string, std::shared_ptr<JSCallbackObject>> cbs{};
        cbs.Insert(type, cbObject);
        callbacks_.Insert(windowId, cbs);
    } else {
        auto res = result.second.Find(type);
        if (res.first) {
            IMSA_HILOGD("type: %{public}s of windowId: %{public}u already subscribed", type.c_str(), windowId);
            return;
        }
        IMSA_HILOGI("start to subscribe type: %{public}s of windowId: %{public}u", type.c_str(), windowId);
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
    IMSA_HILOGI("windowId = %{public}u, type = %{public}s", windowId, type.c_str());
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        IMSA_HILOGE("uv_work_t is nullptr!");
        return;
    }
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
    work->data = new (std::nothrow) UvEntry(callback.second);
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(callback.second.env_, &loop);
    uv_queue_work_with_qos(
        loop, work, [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            std::shared_ptr<UvEntry> entry(static_cast<UvEntry *>(work->data), [work](UvEntry *data) {
                delete data;
                delete work;
            });
            if (entry == nullptr) {
                IMSA_HILOGE("entry is nullptr");
                return;
            }
            JsCallbackHandler::Traverse({ entry->cbCopy });
        },
        uv_qos_user_initiated);
}
} // namespace MiscServices
} // namespace OHOS