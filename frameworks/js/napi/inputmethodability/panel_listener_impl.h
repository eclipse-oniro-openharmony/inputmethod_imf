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

#ifndef INPUTMETHOD_IMF_PANEL_LISTENER_IMPL_H
#define INPUTMETHOD_IMF_PANEL_LISTENER_IMPL_H

#include <mutex>
#include <thread>
#include <uv.h>

#include "concurrent_map.h"
#include "js_callback_object.h"
#include "panel_status_listener.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace MiscServices {
class PanelListenerImpl : public PanelStatusListener {
public:
    static std::shared_ptr<PanelListenerImpl> GetInstance();
    ~PanelListenerImpl();
    void OnPanelStatus(uint32_t windowId, bool isShow) override;
    void SaveInfo(napi_env env, const std::string &type, napi_value callback, uint32_t windowId);
    void RemoveInfo(const std::string &type, uint32_t windowId);

    struct UvEntry {
        std::shared_ptr<JSCallbackObject> cbCopy;
        explicit UvEntry(const std::shared_ptr<JSCallbackObject> &cb) : cbCopy(cb) {}
    };
    ConcurrentMap<uint32_t, ConcurrentMap<std::string, std::shared_ptr<JSCallbackObject>>> callbacks_;
    static std::mutex listenerMutex_;
    static std::shared_ptr<PanelListenerImpl> instance_;
};
} // namespace MiscServices
} // namespace OHOS

#endif //INPUTMETHOD_IMF_PANEL_LISTENER_IMPL_H
