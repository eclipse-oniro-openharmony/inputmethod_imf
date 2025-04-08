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

std::chrono::steady_clock::time_point InputMethodSysEvent::lastReportTime_ = std::chrono::steady_clock::now();
const std::unordered_map<int32_t, std::string> InputMethodSysEvent::operateInfo_ = {
    { static_cast<int32_t>(OperateIMEInfoCode::IME_SHOW_ATTACH), "Attach: attach, bind and show soft keyboard." },
    { static_cast<int32_t>(OperateIMEInfoCode::IME_SHOW_ENEDITABLE), "ShowTextInput: enter editable state, show soft "
                                                                     "keyboard." },
    { static_cast<int32_t>(OperateIMEInfoCode::IME_SHOW_NORMAL), "ShowSoftKeyboard: show soft keyboard." },
    { static_cast<int32_t>(OperateIMEInfoCode::IME_UNBIND), "Close: unbind." },
    { static_cast<int32_t>(OperateIMEInfoCode::IME_HIDE_UNBIND), "Close: hide soft keyboard, and unbind." },
    { static_cast<int32_t>(OperateIMEInfoCode::IME_HIDE_UNEDITABLE), "HideTextInput: hide soft keyboard, quit "
                                                                     "editable state." },
    { static_cast<int32_t>(OperateIMEInfoCode::IME_HIDE_NORMAL), "HideSoftKeyboard, hide soft keyboard." },
    { static_cast<int32_t>(OperateIMEInfoCode::IME_HIDE_UNFOCUSED), "OnUnfocused: unfocused, hide soft keyboard." },
    { static_cast<int32_t>(OperateIMEInfoCode::IME_HIDE_SELF), "HideKeyboardSelf: hide soft keyboard self." }
};

std::map<int32_t, int32_t> InputMethodSysEvent::inputmethodBehaviour_ = {
    { static_cast<int32_t>(IMEBehaviour::START_IME), 0 }, { static_cast<int32_t>(IMEBehaviour::CHANGE_IME), 0 }
};

InputMethodSysEvent::~InputMethodSysEvent()
{
    StopTimer();
}

InputMethodSysEvent &InputMethodSysEvent::GetInstance()
{
    static InputMethodSysEvent instance;
    return instance;
}

void InputMethodSysEvent::ServiceFaultReporter(const std::string &componentName, int32_t errCode)
{
    IMSA_HILOGD("start.");
    int32_t ret = HiSysEventWrite(HiSysEventNameSpace::Domain::INPUTMETHOD, "SERVICE_INIT_FAILED",
        HiSysEventNameSpace::EventType::FAULT, "USER_ID", userId_, "COMPONENT_ID", componentName, "ERROR_CODE",
        errCode);
    if (ret != HiviewDFX::SUCCESS) {
        IMSA_HILOGE("hisysevent ServiceFaultReporter failed! ret: %{public}d, errCode: %{public}d", ret, errCode);
    }
}

void InputMethodSysEvent::InputmethodFaultReporter(int32_t errCode, const std::string &name, const std::string &info)
{
    IMSA_HILOGD("start.");
    int32_t ret = HiSysEventWrite(HiSysEventNameSpace::Domain::INPUTMETHOD, "UNAVAILABLE_INPUTMETHOD",
        HiSysEventNameSpace::EventType::FAULT, "USER_ID", userId_, "APP_NAME", name, "ERROR_CODE", errCode, "INFO",
        info);
    if (ret != HiviewDFX::SUCCESS) {
        IMSA_HILOGE("hisysevent InputmethodFaultReporter failed! ret: %{public}d,errCode %{public}d", ret, errCode);
    }
}

void InputMethodSysEvent::ImeUsageBehaviourReporter()
{
    IMSA_HILOGD("start.");
    int ret = HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::INPUTMETHOD, "IME_USAGE",
        HiSysEventNameSpace::EventType::STATISTIC, "IME_START",
        inputmethodBehaviour_[static_cast<int32_t>(IMEBehaviour::START_IME)], "IME_CHANGE",
        inputmethodBehaviour_[static_cast<int32_t>(IMEBehaviour::CHANGE_IME)]);
    if (ret != HiviewDFX::SUCCESS) {
        IMSA_HILOGE("hisysevent BehaviourReporter failed! ret: %{public}d", ret);
    }
    {
        std::lock_guard<std::mutex> lock(behaviourMutex_);
        inputmethodBehaviour_[static_cast<int32_t>(IMEBehaviour::START_IME)] = 0;
        inputmethodBehaviour_[static_cast<int32_t>(IMEBehaviour::CHANGE_IME)] = 0;
    }
    StartTimerForReport();
}

void InputMethodSysEvent::RecordEvent(IMEBehaviour behaviour)
{
    IMSA_HILOGD("run in.");
    std::lock_guard<std::mutex> lock(behaviourMutex_);
    if (behaviour == IMEBehaviour::START_IME) {
        ++inputmethodBehaviour_[static_cast<int32_t>(IMEBehaviour::START_IME)];
    } else if (behaviour == IMEBehaviour::CHANGE_IME) {
        ++inputmethodBehaviour_[static_cast<int32_t>(IMEBehaviour::CHANGE_IME)];
    }
}

void InputMethodSysEvent::OperateSoftkeyboardBehaviour(OperateIMEInfoCode infoCode)
{
    IMSA_HILOGD("run in.");
    auto currentTime = std::chrono::steady_clock::now();
    if (static_cast<uint32_t>(
            std::chrono::duration_cast<std::chrono::minutes>(currentTime - lastReportTime_).count()) <
        REPORT_INTERVAL) {
        IMSA_HILOGD("Event triggered within 10 minutes, skipping report.");
        return;
    }
    int32_t ret = HiSysEventWrite(HiSysEventNameSpace::Domain::INPUTMETHOD, "OPERATE_SOFTKEYBOARD",
        HiSysEventNameSpace::EventType::BEHAVIOR, "OPERATING", GetOperateAction(static_cast<int32_t>(infoCode)),
        "OPERATE_INFO", GetOperateInfo(static_cast<int32_t>(infoCode)));
    if (ret != HiviewDFX::SUCCESS) {
        IMSA_HILOGE("Hisysevent: operate soft keyboard report failed! ret: %{public}d", ret);
    }
    lastReportTime_ = currentTime;
    IMSA_HILOGD("Event triggered before 10 minutes, report successfully.");
}

void InputMethodSysEvent::ReportImeState(ImeState state, pid_t pid, const std::string &bundleName)
{
    IMSA_HILOGD("run in.");
    auto currentTime = std::chrono::steady_clock::now();
    if (static_cast<uint32_t>(
            std::chrono::duration_cast<std::chrono::minutes>(currentTime - lastReportTime_).count()) <
        REPORT_INTERVAL) {
        IMSA_HILOGD("Event triggered within 10 minutes, skipping report.");
        return;
    }
     
    int32_t ret = HiSysEventWrite(HiSysEventNameSpace::Domain::INPUTMETHOD, "IME_STATE_CHANGED",
        HiSysEventNameSpace::EventType::BEHAVIOR, "STATE", static_cast<int32_t>(state), "PID", pid, "BUNDLE_NAME",
        bundleName);
    if (ret != HiviewDFX::SUCCESS) {
        IMSA_HILOGE("ime: %{public}s state: %{public}d report failed! ret: %{public}d", bundleName.c_str(),
            static_cast<int32_t>(state), ret);
    }
    lastReportTime_ = currentTime;
    IMSA_HILOGD("Event triggered before 10 minutes, report successfully.");
}

const std::string InputMethodSysEvent::GetOperateInfo(int32_t infoCode)
{
    auto iter = operateInfo_.find(static_cast<int32_t>(infoCode));
    if (iter != operateInfo_.end()) {
        return iter->second;
    }
    return "unknow operating.";
}

std::string InputMethodSysEvent::GetOperateAction(int32_t infoCode)
{
    switch (infoCode) {
        case static_cast<int32_t>(OperateIMEInfoCode::IME_SHOW_ATTACH):
        case static_cast<int32_t>(OperateIMEInfoCode::IME_SHOW_ENEDITABLE):
        case static_cast<int32_t>(OperateIMEInfoCode::IME_SHOW_NORMAL):
            return "show";
        case static_cast<int32_t>(OperateIMEInfoCode::IME_UNBIND):
            return "unbind";
        case static_cast<int32_t>(OperateIMEInfoCode::IME_HIDE_UNBIND):
            return "hide and unbind";
        case static_cast<int32_t>(OperateIMEInfoCode::IME_HIDE_UNEDITABLE):
        case static_cast<int32_t>(OperateIMEInfoCode::IME_HIDE_NORMAL):
        case static_cast<int32_t>(OperateIMEInfoCode::IME_HIDE_UNFOCUSED):
        case static_cast<int32_t>(OperateIMEInfoCode::IME_HIDE_SELF):
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

void InputMethodSysEvent::StopTimer()
{
    IMSA_HILOGD("start.");
    std::lock_guard<std::mutex> lock(timerLock_);
    if (timer_ == nullptr) {
        IMSA_HILOGE("timer_ is nullptr.");
        return;
    }
    timer_->Unregister(timerId_);
    timer_->Shutdown();
}

bool InputMethodSysEvent::StartTimer(const TimerCallback &callback, uint32_t interval)
{
    IMSA_HILOGD("start.");
    if (timer_ == nullptr) {
        timer_ = std::make_shared<Utils::Timer>("OS_imfTimer");
        uint32_t ret = timer_->Setup();
        if (ret != Utils::TIMER_ERR_OK) {
            IMSA_HILOGE("create Timer error.");
            return false;
        }
        timerId_ = timer_->Register(callback, interval, true);
    } else {
        IMSA_HILOGD("timer_ is not nullptr, Update timer.");
        timer_->Unregister(timerId_);
        timerId_ = timer_->Register(callback, interval, false);
    }
    return true;
}

bool InputMethodSysEvent::StartTimerForReport()
{
    IMSA_HILOGD("start.");
    auto reportCallback = [this]() { ImeUsageBehaviourReporter(); };
    std::lock_guard<std::mutex> lock(timerLock_);
    return StartTimer(reportCallback, ONE_DAY_IN_HOURS * ONE_HOUR_IN_SECONDS * SECONDS_TO_MILLISECONDS);
}

void InputMethodSysEvent::ReportSystemShortCut(const std::string &shortcutName)
{
    IMSA_HILOGD("run in.");
    int32_t ret = HiSysEventWrite(HiSysEventNameSpace::Domain::INPUTMETHOD_UE, "SYSTEM_SHORTCUT",
        HiSysEventNameSpace::EventType::BEHAVIOR, "PROCESS_NAME", "inputmethod_service", "SHORTCUT_NAME", shortcutName);
    if (ret != HiviewDFX::SUCCESS) {
        IMSA_HILOGE("system shortcut: %{public}s report failed! ret: %{public}d", shortcutName.c_str(), ret);
    }
}
} // namespace MiscServices
} // namespace OHOS