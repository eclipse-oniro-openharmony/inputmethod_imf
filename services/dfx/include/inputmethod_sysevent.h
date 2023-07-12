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
enum OperateIMEInfoCode : int32_t {
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

enum IMEBehaviour : int32_t {
    START_IME = 0,
    CHANGE_IME,
};

class InputMethodSysEvent {
public:
    static void ServiceFaultReporter(const std::string &bundleName, int32_t errCode);
    static void InputmethodFaultReporter(int32_t errCode, const std::string &name, const std::string &info);
    static void EventRecorder(IMEBehaviour behaciour);
    static void OperateSoftkeyboardBehaviour(OperateIMEInfoCode infoCode);
    static void StartTimerForReport();
    static void SetUserId(int32_t userId);

private:
    using TimerCallback = std::function<void()>;
    static void ImeUsageBehaviourReporter();
    static const std::string GetOperateInfo(OperateIMEInfoCode infoCode);
    static std::string GetOperateAction(OperateIMEInfoCode infoCode);
    static void StartTimer(const TimerCallback &callback, uint32_t interval);
    static void StopTimer();
    static int32_t GetReportTime();

private:
    static const std::unordered_map<int32_t, std::string> operateInfo_;
    static std::map<int32_t, int32_t> inputmethodBehaviour_;
    static std::mutex behaviourMutex_;

    static Utils::Timer timer_;
    static int32_t userId_;
    static uint32_t timerId_;
    static std::mutex timerLock_;
    static bool isTimerStart_;
    static inline constexpr int32_t ONE_DAY_IN_HOURS = 24;
    static inline constexpr int32_t EXEC_HOUR_TIME = 23;
    static inline constexpr int32_t EXEC_MIN_TIME = 60;
    static inline constexpr int32_t ONE_MINUTE_IN_SECONDS = 60;
    static inline constexpr int32_t ONE_HOUR_IN_SECONDS = 1 * 60 * 60; // 1 hour
    static inline constexpr int32_t SECONDS_TO_MILLISECONDS = 1000;
};
} // namespace MiscServices
} // namespace OHOS
#endif // INPUTMETHOD_SYSEVENT_H