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
#include "input_method_ability_interface.h"
#include "input_method_engine_listener.h"
#include "nativetoken_kits.h"
#include "sys_cfg_parser.h"
#include "token_setproc.h"

using namespace std;
using namespace OHOS::Security::AccessToken;
using namespace OHOS::MiscServices;
const uint32_t INSERT_TEXT_MAX_NUM = 1000;
void InsertText()
{
    for (int32_t i = 0; i < INSERT_TEXT_MAX_NUM; i++) {
        InputMethodAbilityInterface::GetInstance().InsertText("A");
    }
}
class InputMethodEngineListenerImpl : public InputMethodEngineListener {
public:
    void OnKeyboardStatus(bool isShow) override
    {
    }
    void OnInputStart() override
    {
        std::thread t(InsertText);
        t.detach();
    }
    int32_t OnInputStop() override
    {
        return 0;
    }
    void OnSetCallingWindow(uint32_t windowId) override
    {
    }
    void OnSetSubtype(const SubProperty &property) override
    {
    }
    void ReceivePrivateCommand(const std::unordered_map<std::string, PrivateDataValue> &privateCommand) override
    {
    }
    void OnInputFinish() override
    {
    }
    bool IsEnable() override
    {
        return true;
    }
    void NotifyPreemption() override
    {
    }
};

void RegisterProxy()
{
    InputMethodAbilityInterface::GetInstance().SetImeListener(std::make_shared<InputMethodEngineListenerImpl>());
    auto ret = InputMethodAbilityInterface::GetInstance().RegisteredProxy();
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGI("Register proxy ime failed:%{public}d.", ret);
        return;
    }
    IMSA_HILOGI("Register proxy ime succeed.");
}

void UnRegisterProxy()
{
    auto ret = InputMethodAbilityInterface::GetInstance().UnRegisteredProxy(UnRegisteredType::REMOVE_PROXY_IME);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGI("UnRegister proxy ime failed:%{public}d.", ret);
        return;
    }
    IMSA_HILOGI("UnRegister proxy ime succeed.");
}

int32_t GetProxyImeUid()
{
    SystemConfig systemConfig;
    SysCfgParser::ParseSystemConfig(systemConfig);
    if (systemConfig.proxyImeUidList.empty()) {
        return -1;
    }
    return *systemConfig.proxyImeUidList.begin();
}

int main()
{
    setuid(GetProxyImeUid());
    int32_t input = 1;
    while (true) {
        printf("=====1:RegisterProxy  2:UnRegisterProxy=====\n");
        printf("input: ");
        fflush(stdout);
        scanf("%d", &input);
        getchar();
        printf("input:%d \n", input);
        switch (input) {
            case 1:
                RegisterProxy();
                break;
            case 2:
                UnRegisterProxy();
                break;
            default:
                printf("input error!\n");
        }
        printf("=======================END=========================");
    }
}