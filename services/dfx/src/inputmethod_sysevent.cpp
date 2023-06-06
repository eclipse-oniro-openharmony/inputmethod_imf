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
const std::string DOMAIN_STR = std::string(HiSysEventNameSpace::Domain::INPUTMETHOD);
} // namespace

std::map<int32_t, int32_t> InputmethodSysevent::inputmethodBehaviour_ = {
    {START_IME, 0},
    {CHANGE_IME, 0}
};
const std::map<int32_t, std::string> InputmethodSysevent::oprateInfo_ = {
    {IME_SHOW_ATTACH, "attach, bind and show soft keyboard."},
    {IME_SHOW_ENEDITABLE, "enter editable state, show soft keyboard."},
    {IME_SHOW_NORMAL, "show soft keyboard."},
    {IME_UNBIND, "unbind."},
    {IME_HIDE_UNBIND, "hide soft keyboard, and unbind."},
    {IME_HIDE_UNEDITABLE, "hide soft keyboard, quit editable state."},
    {IME_HIDE_NORMAL, "hide soft keyboard."},
    {IME_HIDE_UNFOCUSED, "unfocused, hide soft keyboard."},
    {IME_HIDE_SELF, "hide soft keyboard self."}
};

void InputmethodSysevent::FaultReporter(int32_t userId, std::string bundleName, int32_t errCode)
{
    int32_t ret = HiSysEventWrite(DOMAIN_STR, "SERVICE_INIT_FAILED", HiSysEventNameSpace::EventType::FAULT, "USER_ID",
        userId, "COMPONENT_ID", bundleName, "ERROR_CODE", errCode);
    if (ret != 0) {
        IMSA_HILOGE("hisysevent FaultReporter failed! ret %{public}d,errCode %{public}d", ret, errCode);
    }
}

void InputmethodSysevent::CreateComponentFailed(int32_t userId, int32_t errCode)
{
    int32_t ret = HiSysEventWrite(DOMAIN_STR, "CREATE_COMPONENT_FAILED", HiSysEventNameSpace::EventType::FAULT,
        "USER_ID", userId, "ERROR_CODE", errCode);
    if (ret != 0) {
        IMSA_HILOGE("hisysevent CreateComponentFailed failed! ret %{public}d,errCode %{public}d", ret, errCode);
    }
}

void InputmethodSysevent::BehaviourReporter(IMEBehaviour ActiveName)
{
    if (ActiveName == IMEBehaviour::START_IME) {
        inputmethodBehaviour_[START_IME]++;
    } else if (ActiveName == IMEBehaviour::CHANGE_IME) {
        inputmethodBehaviour_[CHANGE_IME]++;
    }
}

void InputmethodSysevent::InvokeInputmethodStatistic()
{
    int32_t ret = HiSysEventWrite(DOMAIN_STR, "IME_USAGE", HiSysEventNameSpace::EventType::STATISTIC, "IME_START",
        inputmethodBehaviour_[START_IME], "IME_CHANGE", inputmethodBehaviour_[CHANGE_IME]);
    if (ret != HiviewDFX::SUCCESS) {
        IMSA_HILOGE("hisysevent BehaviourReporter failed! ret %{public}d", ret);
    }
    inputmethodBehaviour_[START_IME] = 0;
    inputmethodBehaviour_[CHANGE_IME] = 0;
}

void InputmethodSysevent::OperateSoftkeyboardBehaviour(OperateIMEInfoCode infoCode)
{
    int32_t ret = HiSysEventWrite(DOMAIN_STR, "OPERATE_SOFTKEYBOARD", HiSysEventNameSpace::EventType::BEHAVIOR,
        "OPERATING", GetOperateAction(infoCode), "OPERATE_INFO", GetOperateInfo(infoCode));
    if (ret != HiviewDFX::SUCCESS) {
        IMSA_HILOGE("hisysevent BehaviourReporter failed! ret %{public}d", ret);
    }
}

std::string InputmethodSysevent::GetOperateInfo(OperateIMEInfoCode infoCode)
{
    std::string info;
    auto iter = oprateInfo_.find(infoCode);
    if (iter != oprateInfo_.end()) {
        info = iter->second;
        return info;
    }
    return "unknow operating.";
}

std::string InputmethodSysevent::GetOperateAction(OperateIMEInfoCode infoCode)
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
    }
}
} // namespace MiscServices
} // namespace OHOS