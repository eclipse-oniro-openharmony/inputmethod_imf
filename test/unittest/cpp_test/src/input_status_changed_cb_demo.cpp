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
#include <cstdio>
#include <iostream>

#include "accesstoken_kit.h"
#include "global.h"
#include "ime_event_listener.h"
#include "ime_event_monitor_manager.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"

using namespace std;
using namespace OHOS::Security::AccessToken;
using namespace OHOS::MiscServices;
std::vector<std::shared_ptr<ImeEventListener>> listeners_;
const int32_t PERMISSION_NUM = 1;

void GrantNativePermission()
{
    const char **perms = new const char *[PERMISSION_NUM];
    TokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = PERMISSION_NUM,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = "imf_test",
        .aplStr = "system_core",
    };
    uint64_t tokenId = GetAccessTokenId(&infoInstance);
    int32_t ret = SetSelfTokenID(tokenId);
    if (ret == 0) {
        IMSA_HILOGI("SetSelfTokenID success!");
    } else {
        IMSA_HILOGE("SetSelfTokenID fail!");
    }
    AccessTokenKit::ReloadNativeTokenInfo();
    delete[] perms;
}

class InputStatusChangedListener : public ImeEventListener {
public:
    void OnInputStart(uint32_t callingWndId, int32_t requestKeyboardReason) override
    {
        printf("=====callingWndId:%u.=====\n", callingWndId);
    }
    void OnInputStop() override
    {
        printf("=====run in.=====\n.");
    }
};

void RegisterInputStatusChangedListener()
{
    auto listener = std::make_shared<InputStatusChangedListener>();
    auto ret =
        ImeEventMonitorManager::GetInstance().RegisterImeEventListener(EVENT_INPUT_STATUS_CHANGED_MASK, listener);
    if (ret != ErrorCode::NO_ERROR) {
        printf("=====Register input status changed listener failed:%d.=====\n", ret);
        return;
    }
    listeners_.push_back(listener);
    printf("=====Register input status changed listener succeed, current listener nums:%zu.=====\n", listeners_.size());
}

void RegisterSameInputStatusChangedListener()
{
    if (listeners_.empty()) {
        printf("=====has no listener.=====\n");
        return;
    }
    auto listener = listeners_.back();
    auto ret =
        ImeEventMonitorManager::GetInstance().RegisterImeEventListener(EVENT_INPUT_STATUS_CHANGED_MASK, listener);
    if (ret != ErrorCode::NO_ERROR) {
        printf("=====Register same input status changed listener failed:%d.=====\n", ret);
        return;
    }
    printf("=====Register same input status changed succeed, current listener nums:%zu.=====\n", listeners_.size());
}

void UnRegisterInputStatusChangedListener()
{
    if (listeners_.empty()) {
        printf("=====has no listener.=====\n");
        return;
    }
    auto listener = listeners_.back();
    auto ret =
        ImeEventMonitorManager::GetInstance().UnRegisterImeEventListener(EVENT_INPUT_STATUS_CHANGED_MASK, listener);
    if (ret != ErrorCode::NO_ERROR) {
        printf("=====UnRegister input status changed listener failed:%d.=====\n", ret);
        return;
    }
    listeners_.pop_back();
    printf("=====UnRegister input status changed succeed, current listener nums:%zu.=====\n", listeners_.size());
}

int main()
{
    GrantNativePermission();
    int32_t input = 0;
    // 4: input 4
    while (input != 4) {
        printf("=====1:RegisterListener  2:RegisterSameListener  3:UnRegisterListener  4:exit=====\n");
        printf("input: ");
        cin >> input;
        switch (input) {
            // 1: input 1
            case 1:
                RegisterInputStatusChangedListener();
                break;
            // 2: input 2
            case 2:
                RegisterSameInputStatusChangedListener();
                break;
            // 3: input 3
            case 3:
                UnRegisterInputStatusChangedListener();
                break;
            // 4: input 4
            case 4:
                printf("=====EXIT=====\n");
                break;
            default:
                printf("=====input error!=====\n");
        }
    }
    return 0;
}