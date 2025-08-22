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
#include <iostream>

#include "accesstoken_kit.h"
#include "global.h"
#include "ime_event_listener.h"
#include "ime_event_monitor_manager.h"
#include "nativetoken_kits.h"
#include "tdd_util.h"
#include "token_setproc.h"

using namespace std;
using namespace OHOS::Security::AccessToken;
using namespace OHOS::MiscServices;
std::vector<std::shared_ptr<ImeEventListener>> listeners_;
class InputStatusChangedListener : public ImeEventListener {
public:
    void OnInputStart(uint32_t callingWndId, int32_t requestKeyboardReason) override
    {
        IMSA_HILOGI("callingWndId:%{public}d.", callingWndId);
    }
    void OnInputStop() override
    {
        IMSA_HILOGI("run in.");
    }
};

void RegisterInputStatusChangedListener()
{
    auto listener = std::make_shared<InputStatusChangedListener>();
    auto ret =
        ImeEventMonitorManager::GetInstance().RegisterImeEventListener(EVENT_INPUT_STATUS_CHANGED_MASK, listener);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGI("Register input status changed listener failed:%{public}d.", ret);
        return;
    }
    listeners_.push_back(listener);
    IMSA_HILOGI(
        "Register input status changed listener succeed, current listener nums:%{public}zu.", listeners_.size());
}

void RegisterSameInputStatusChangedListener()
{
    if (listeners_.empty()) {
        IMSA_HILOGW("has no listener .");
        return;
    }
    auto listener = listeners_.back();
    auto ret =
        ImeEventMonitorManager::GetInstance().RegisterImeEventListener(EVENT_INPUT_STATUS_CHANGED_MASK, listener);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGI("Register same input status changed listener failed:%{public}d.", ret);
        return;
    }
    IMSA_HILOGI("Register same input status changed succeed, current listener nums:%{public}zu.", listeners_.size());
}

void UnRegisterInputStatusChangedListener()
{
    if (listeners_.empty()) {
        IMSA_HILOGW("has no listener .");
        return;
    }
    auto listener = listeners_.back();
    auto ret =
        ImeEventMonitorManager::GetInstance().UnRegisterImeEventListener(EVENT_INPUT_STATUS_CHANGED_MASK, listener);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGI("UnRegister input status changed listener failed:%{public}d.", ret);
        return;
    }
    listeners_.pop_back();
    IMSA_HILOGI("UnRegister input status changed succeed, current listener nums:%{public}zu.", listeners_.size());
}

int main()
{
    TddUtil::GrantNativePermission();
    int32_t input = 1;
    while (true) {
        printf("=====1:RegisterListener  2:RegisterSameListener  3:UnRegisterListener=====\n");
        printf("input: ");
        fflush(stdout);
        scanf("%d", &input);
        getchar();
        printf("input:%d \n", input);
        switch (input) {
            case 1:
                RegisterInputStatusChangedListener();
                break;
            case 2:
                RegisterSameInputStatusChangedListener();
                break;
            case 3:
                UnRegisterInputStatusChangedListener();
                break;
            default:
                printf("input error!\n");
        }
        printf("=======================END=========================");
    }
}