/*
* Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef IME_STATE_MANAGER_H
#define IME_STATE_MANAGER_H
#include "event_handler.h"
namespace OHOS {
namespace MiscServices {
enum class RequestType : int32_t {
    NORMAL = 0,
    START_INPUT,
    STOP_INPUT,
    REQUEST_SHOW,
    REQUEST_HIDE
};
class ImeStateManager {
public:
    explicit ImeStateManager(pid_t pid) : pid_(pid) { }
    virtual ~ImeStateManager() = default;
    static void SetEventHandler(const std::shared_ptr<AppExecFwk::EventHandler> &eventHandler);
    bool IsIpcNeeded(RequestType type);
    void BeforeIpc(RequestType type);
    void AfterIpc(RequestType type, bool isSuccess);
    bool IsImeInUse();
    virtual void TemporaryActiveIme() { };

protected:
    std::mutex mutex_;
    static std::shared_ptr<AppExecFwk::EventHandler> eventHandler_;
    pid_t pid_;
    bool isFrozen_ { true };

private:
    virtual void ControlIme(bool shouldApply) = 0;
    bool isImeInUse_ { false };
};
} // namespace MiscServices
} // namespace OHOS
#endif // IME_STATE_MANAGER_H