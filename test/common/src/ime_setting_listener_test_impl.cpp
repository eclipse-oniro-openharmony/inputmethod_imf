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

#include "ime_setting_listener_test_impl.h"

#include "global.h"

namespace OHOS {
namespace MiscServices {
constexpr int32_t SWITCH_IME_WAIT_TIME = 3;
InputWindowStatus ImeSettingListenerTestImpl::status_{ InputWindowStatus::NONE };
SubProperty ImeSettingListenerTestImpl::subProperty_{};
Property ImeSettingListenerTestImpl::property_{};
std::mutex ImeSettingListenerTestImpl::imeSettingListenerLock_;
bool ImeSettingListenerTestImpl::isImeChange_{ false };
std::condition_variable ImeSettingListenerTestImpl::imeSettingListenerCv_;
void ImeSettingListenerTestImpl::ResetParam()
{
    status_ = InputWindowStatus::NONE;
    subProperty_ = {};
    property_ = {};
    isImeChange_ = false;
}
bool ImeSettingListenerTestImpl::WaitPanelHide()
{
    std::unique_lock<std::mutex> lock(imeSettingListenerLock_);
    imeSettingListenerCv_.wait_for(lock, std::chrono::seconds(1), []() { return status_ == InputWindowStatus::HIDE; });
    return status_ == InputWindowStatus::HIDE;
}
bool ImeSettingListenerTestImpl::WaitPanelShow()
{
    std::unique_lock<std::mutex> lock(imeSettingListenerLock_);
    imeSettingListenerCv_.wait_for(lock, std::chrono::seconds(1), []() { return status_ == InputWindowStatus::SHOW; });
    return status_ == InputWindowStatus::SHOW;
}

bool ImeSettingListenerTestImpl::WaitImeChange()
{
    std::unique_lock<std::mutex> lock(imeSettingListenerLock_);
    imeSettingListenerCv_.wait_for(lock, std::chrono::seconds(SWITCH_IME_WAIT_TIME), []() { return isImeChange_; });
    return isImeChange_;
}

bool ImeSettingListenerTestImpl::WaitTargetImeChange(const std::string &bundleName)
{
    std::unique_lock<std::mutex> lock(imeSettingListenerLock_);
    imeSettingListenerCv_.wait_for(
        lock, std::chrono::seconds(SWITCH_IME_WAIT_TIME), [&bundleName]() { return bundleName == property_.name; });
    return isImeChange_ && bundleName == property_.name;
}

bool ImeSettingListenerTestImpl::WaitImeChange(const SubProperty &subProperty)
{
    std::unique_lock<std::mutex> lock(imeSettingListenerLock_);
    imeSettingListenerCv_.wait_for(lock, std::chrono::seconds(SWITCH_IME_WAIT_TIME),
        [&subProperty]() { return subProperty_.id == subProperty.id && subProperty_.name == subProperty.name; });
    return subProperty_.id == subProperty.id && subProperty_.name == subProperty.name;
}
void ImeSettingListenerTestImpl::OnImeChange(const Property &property, const SubProperty &subProperty)
{
    std::unique_lock<std::mutex> lock(imeSettingListenerLock_);
    IMSA_HILOGI("ImeSettingListenerTestImpl, property name: %{public}s, property id: %{public}s, subProp id: "
                "%{public}s",
        property.name.c_str(), property.id.c_str(), subProperty.id.c_str());
    isImeChange_ = true;
    subProperty_ = subProperty;
    property_ = property;
    imeSettingListenerCv_.notify_one();
}
void ImeSettingListenerTestImpl::OnImeShow(const ImeWindowInfo &info)
{
    std::unique_lock<std::mutex> lock(imeSettingListenerLock_);
    IMSA_HILOGI("ImeSettingListenerTestImpl");
    status_ = InputWindowStatus::SHOW;
    imeSettingListenerCv_.notify_one();
}
void ImeSettingListenerTestImpl::OnImeHide(const ImeWindowInfo &info)
{
    std::unique_lock<std::mutex> lock(imeSettingListenerLock_);
    IMSA_HILOGI("ImeSettingListenerTestImpl");
    status_ = InputWindowStatus::HIDE;
    imeSettingListenerCv_.notify_one();
}
} // namespace MiscServices
} // namespace OHOS