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

#include "inputmethod_sysevent.h"

#include <unistd.h>

#include "common_timer_errors.h"
#include "hisysevent.h"

namespace OHOS {
namespace MiscServices {
namespace {
using HiSysEventNameSpace = OHOS::HiviewDFX::HiSysEvent;
} // namespace

const std::unordered_map<int32_t, std::string> InputMethodSysEvent::operateInfo_ = {
    {IME_SHOW_ATTACH, "Attach: attach, bind and show soft keyboard."},
    {IME_SHOW_ENEDITABLE, "ShowTextInput: enter editable state, show soft keyboard."},
    {IME_SHOW_NORMAL, "ShowSoftKeyboard: show soft keyboard."},
    {IME_UNBIND, "Close: unbind."},
    {IME_HIDE_UNBIND, "Close: hide soft keyboard, and unbind."},
    {IME_HIDE_UNEDITABLE, "HideTextInput: hide soft keyboard, quit editable state."},
    {IME_HIDE_NORMAL, "HideSoftKeyboard, hide soft keyboard."},
    {IME_HIDE_UNFOCUSED, "OnUnfocused: unfocused, hide soft keyboard."},
    {IME_HIDE_SELF, "HideKeyboardSelf: hide soft keyboard self."}
};

std::map<int32_t, int32_t> InputMethodSysEvent::inputmethodBehaviour_ = {
    {START_IME, 0},
    {CHANGE_IME, 0}
};

Utils::Timer InputMethodSysEvent::timer_("imfTimer");
uint32_t InputMethodSysEvent::timerId_(0);
std::mutex InputMethodSysEvent::behaviourMutex_;
std::mutex InputMethodSysEvent::timerLock_;
bool InputMethodSysEvent::isTimerStart_ = false;
int32_t InputMethodSysEvent::userId_ = 0;

void InputMethodSysEvent::ServiceFaultReporter(const std::string &bundleName, int32_t errCode)
{
    int32_t ret = HiSysEventWrite(HiSysEventNameSpace::Domain::INPUTMETHOD, "SERVICE_INIT_FAILED",
        HiSysEventNameSpace::EventType::FAULT, "USER_ID", userId_, "COMPONENT_ID", bundleName, "ERROR_CODE", errCode);
    if (ret != HiviewDFX::SUCCESS) {
        IMSA_HILOGE("hisysevent ServiceFaultReporter failed! ret %{public}d,errCode %{public}d", ret, errCode);
    }
}

void InputMethodSysEvent::InputmethodFaultReporter(int32_t errCode, const std::string &name, const std::string &info)
{
    int32_t ret = HiSysEventWrite(HiSysEventNameSpace::Domain::INPUTMETHOD, "INPUTMETHOD_UNAVAILABLE",
        HiSysEventNameSpace::EventType::FAULT, "USER_ID", userId_, "APP_NAME", name, "ERROR_CODE", errCode, "INFO",
        info);
    if (ret != HiviewDFX::SUCCESS) {
        IMSA_HILOGE("hisysevent InputmethodFaultReporter failed! ret %{public}d,errCode %{public}d", ret, errCode);
    }
}

void InputMethodSysEvent::ImeUsageBehaviourReporter()
{
    IMSA_HILOGE("msy ImeUsageBehaviourReporter");
    std::lock_guard<std::mutex> lock(behaviourMutex_);
    int ret = HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::INPUTMETHOD, "IME_USAGE",
        HiSysEventNameSpace::EventType::STATISTIC, "IME_START", inputmethodBehaviour_[START_IME], "IME_CHANGE",
        inputmethodBehaviour_[CHANGE_IME]);
    if (ret != HiviewDFX::SUCCESS) {
        IMSA_HILOGE("hisysevent BehaviourReporter failed! ret %{public}d", ret);
    }
    inputmethodBehaviour_[START_IME] = 0;
    inputmethodBehaviour_[CHANGE_IME] = 0;
    StartTimerForReport();
}

void InputMethodSysEvent::EventRecorder(IMEBehaviour behaviour)
{
    std::lock_guard<std::mutex> lock(behaviourMutex_);
    if (behaviour == IMEBehaviour::START_IME) {
        inputmethodBehaviour_[START_IME]++;
    } else if (behaviour == IMEBehaviour::CHANGE_IME) {
        inputmethodBehaviour_[CHANGE_IME]++;
    }
}

void InputMethodSysEvent::OperateSoftkeyboardBehaviour(OperateIMEInfoCode infoCode)
{
    int32_t ret = HiSysEventWrite(HiSysEventNameSpace::Domain::INPUTMETHOD, "OPERATE_SOFTKEYBOARD",
        HiSysEventNameSpace::EventType::BEHAVIOR, "OPERATING", GetOperateAction(infoCode), "OPERATE_INFO",
        GetOperateInfo(infoCode));
    if (ret != HiviewDFX::SUCCESS) {
        IMSA_HILOGE("Hisysevent: operate soft keyboard report failed! ret %{public}d", ret);
    }
}

const std::string InputMethodSysEvent::GetOperateInfo(OperateIMEInfoCode infoCode)
{
    auto iter = operateInfo_.find(infoCode);
    if (iter != operateInfo_.end()) {
        return iter->second;
    }
    return "unknow operating.";
}

std::string InputMethodSysEvent::GetOperateAction(OperateIMEInfoCode infoCode)
{
    switch (infoCode) {
        case IME_SHOW_ATTACH:
        case IME_SHOW_ENEDITABLE:
        case IME_SHOW_NORMAL:
            return "show";
        case IME_UNBIND:
            return "unbind";
        case IME_HIDE_UNBIND:
            return "hide and unbind";
        case IME_HIDE_UNEDITABLE:
        case IME_HIDE_NORMAL:
        case IME_HIDE_UNFOCUSED:
        case IME_HIDE_SELF:
            return "hide";
        default:
            break;
    }
    return "unknow action.";
}

void InputMethodSysEvent::SetUserId(int32_t userId)
{
    userId_ = userId;
}

void InputMethodSysEvent::StartTimer(const TimerCallback &callback, uint32_t interval)
{
    IMSA_HILOGD("run in");
    isTimerStart_ = true;
    uint32_t ret = timer_.Setup();
    if (ret != Utils::TIMER_ERR_OK) {
        IMSA_HILOGE("Create Timer error");
        return;
    }
    timerId_ = timer_.Register(callback, interval, true);
}

void InputMethodSysEvent::StopTimer()
{
    IMSA_HILOGD("run in");
    timer_.Unregister(timerId_);
    timer_.Shutdown();
    isTimerStart_ = false;
}

void InputMethodSysEvent::StartTimerForReport()
{
    IMSA_HILOGD("run in");
    auto reportCallback = []() { ImeUsageBehaviourReporter(); };
    std::lock_guard<std::mutex> lock(timerLock_);
    if (isTimerStart_) {
        IMSA_HILOGD("isTimerStart_ is true. Update timer.");
        timer_.Unregister(timerId_);
        timerId_ =
            timer_.Register(reportCallback, ONE_DAY_IN_HOURS * ONE_HOUR_IN_SECONDS * SECONDS_TO_MILLISECONDS, false);
    } else {
        int32_t interval = GetReportTime();
        if (interval >= 0) {
            StartTimer(reportCallback, interval);
        }
    }
}

int32_t InputMethodSysEvent::GetReportTime()
{
    IMSA_HILOGD("GetReportTime run in.");
    time_t current = time(nullptr);
    if (current == -1) {
        IMSA_HILOGE("Get current time failed!");
        return -1;
    }
    tm localTime = { 0 };
    tm *result = localtime_r(&current, &localTime);
    if (result == nullptr) {
        IMSA_HILOGE("Get local time failed!");
        return -1;
    }
    int32_t currentHour = localTime.tm_hour;
    int32_t currentMin = localTime.tm_min;
    IMSA_HILOGD("get");
    if ((EXEC_MIN_TIME - currentMin) != EXEC_MIN_TIME) {
        int32_t nHours = EXEC_HOUR_TIME - currentHour;
        int32_t nMin = EXEC_MIN_TIME - currentMin;
        int32_t nTime = (nMin)*ONE_MINUTE_IN_SECONDS + (nHours)*ONE_HOUR_IN_SECONDS;
        IMSA_HILOGD(
            " StartTimerThread if needHours=%{public}d,needMin=%{public}d,needTime=%{public}d", nHours, nMin, nTime);
        return nTime * SECONDS_TO_MILLISECONDS;
    } else {
        return ONE_HOUR_IN_SECONDS * (ONE_DAY_IN_HOURS - currentHour) * SECONDS_TO_MILLISECONDS;
    }
}
} // namespace MiscServices
} // namespace OHOS