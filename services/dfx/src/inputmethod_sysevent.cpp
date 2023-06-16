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

#include "hisysevent.h"

#include <unistd.h>

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

void InputMethodSysEvent::FaultReporter(int32_t userId, const std::string &bundleName, int32_t errCode)
{
    int32_t ret = HiSysEventWrite(HiSysEventNameSpace::Domain::INPUTMETHOD, "SERVICE_INIT_FAILED",
        HiSysEventNameSpace::EventType::FAULT, "USER_ID", userId, "COMPONENT_ID", bundleName, "ERROR_CODE", errCode);
    if (ret != HiviewDFX::SUCCESS) {
        IMSA_HILOGE("hisysevent FaultReporter failed! ret %{public}d,errCode %{public}d", ret, errCode);
    }
}

void InputMethodSysEvent::CreateComponentFailed(int32_t userId, int32_t errCode)
{
    int32_t ret = HiSysEventWrite(HiSysEventNameSpace::Domain::INPUTMETHOD, "CREATE_COMPONENT_FAILED",
        HiSysEventNameSpace::EventType::FAULT, "USER_ID", userId, "ERROR_CODE", errCode);
    if (ret != HiviewDFX::SUCCESS) {
        IMSA_HILOGE("hisysevent CreateComponentFailed failed! ret %{public}d,errCode %{public}d", ret, errCode);
    }
}

void InputMethodSysEvent::BehaviourReporter(const std::string &activeName, const std::string &inputMethodName)
{
    int32_t ret = HiSysEventWrite(HiSysEventNameSpace::Domain::INPUTMETHOD, "INPUTMETHOD_USING",
        HiSysEventNameSpace::EventType::BEHAVIOR, "ACTIVE_NAME", activeName, "INPUTMETHOD_NAME", inputMethodName);
    if (ret != HiviewDFX::SUCCESS) {
        IMSA_HILOGE("hisysevent BehaviourReporter failed! ret %{public}d", ret);
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

std::string InputMethodSysEvent::GetOperateInfo(OperateIMEInfoCode infoCode)
{
    std::string info;
    auto iter = operateInfo_.find(infoCode);
    if (iter != operateInfo_.end()) {
        info = iter->second;
        return info;
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
} // namespace MiscServices
} // namespace OHOS