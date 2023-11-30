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

namespace OHOS {
namespace MiscServices {
InputWindowStatus ImeSettingListenerTestImpl::status_{ InputWindowStatus::NONE };
SubProperty ImeSettingListenerTestImpl::subProperty_{};
std::mutex ImeSettingListenerTestImpl::imeSettingListenerLock_;
std::condition_variable ImeSettingListenerTestImpl::imeSettingListenerCv_;
void ImeSettingListenerTestImpl::ResetParam()
{
    status_ = InputWindowStatus::NONE;
    subProperty_ = {};
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
bool ImeSettingListenerTestImpl::WaitImeChange(const SubProperty &subProperty)
{
    std::unique_lock<std::mutex> lock(imeSettingListenerLock_);
    imeSettingListenerCv_.wait_for(lock, std::chrono::seconds(1),
        [&subProperty]() { return subProperty_.id == subProperty.id && subProperty_.name == subProperty.name; });
    return subProperty_.id == subProperty.id && subProperty_.name == subProperty.name;
}
void ImeSettingListenerTestImpl::OnImeChange(const Property &property, const SubProperty &subProperty)
{
    subProperty_ = subProperty;
    imeSettingListenerCv_.notify_one();
}
void ImeSettingListenerTestImpl::OnPanelStatusChange(
    const InputWindowStatus &status, const std::vector<InputWindowInfo> &windowInfo)
{
    status_ = status;
    imeSettingListenerCv_.notify_one();
}
} // namespace MiscServices
} // namespace OHOS