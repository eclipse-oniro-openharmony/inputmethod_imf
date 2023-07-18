/*
 * Copyright (c) 2022-2022 Huawei Device Co., Ltd.
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

#ifndef INPUTMETHOD_SYSEVENT_H
#define INPUTMETHOD_SYSEVENT_H

#include <map>
#include <mutex>
#include <string>
#include <unordered_map>

#include "global.h"
#include "timer.h"

namespace OHOS {
namespace MiscServices {
enum class OperateIMEInfoCode : int32_t {
    IME_SHOW_ATTACH = 0,
    IME_SHOW_ENEDITABLE,
    IME_SHOW_NORMAL,
    IME_UNBIND,
    IME_HIDE_UNBIND,
    IME_HIDE_UNEDITABLE,
    IME_HIDE_NORMAL,
    IME_HIDE_UNFOCUSED,
    IME_HIDE_SELF,
};

enum class IMEBehaviour : int32_t {
    START_IME = 0,
    CHANGE_IME,
};

class InputMethodSysEvent {
public:
    static InputMethodSysEvent &GetInstance();
    void ServiceFaultReporter(const std::string &componentName, int32_t errCode);
    void InputmethodFaultReporter(int32_t errCode, const std::string &name, const std::string &info);
    void RecordEvent(IMEBehaviour behaviour);
    void OperateSoftkeyboardBehaviour(OperateIMEInfoCode infoCode);
    bool StartTimerForReport();
    void SetUserId(int32_t userId);

private:
    using TimerCallback = std::function<void()>;
    void ImeUsageBehaviourReporter();
    const std::string GetOperateInfo(int32_t infoCode);
    std::string GetOperateAction(int32_t infoCode);
    bool StartTimer(const TimerCallback &callback, uint32_t interval);

private:
    static const std::unordered_map<int32_t, std::string> operateInfo_;
    static std::map<int32_t, int32_t> inputmethodBehaviour_;
    std::mutex behaviourMutex_;

    std::shared_ptr<Utils::Timer> timer_ = nullptr;
    int32_t userId_ = 0;
    uint32_t timerId_ = 0;
    std::mutex timerLock_;
    static inline constexpr int32_t ONE_DAY_IN_HOURS = 24;
    static inline constexpr int32_t ONE_HOUR_IN_SECONDS = 1 * 60 * 60; // 1 hour
    static inline constexpr int32_t SECONDS_TO_MILLISECONDS = 1000;
};
} // namespace MiscServices
} // namespace OHOS
#endif // INPUTMETHOD_SYSEVENT_H