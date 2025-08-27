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

#include "global.h"
#include "input_method_ability_interface.h"
#include "input_method_engine_listener.h"
#include "sys_cfg_parser.h"

using namespace std;
using namespace OHOS::MiscServices;
const uint32_t INSERT_TEXT_MAX_NUM = 1000;
const uint32_t REGISTER_MAX_NUM = 10;
void InsertText()
{
    for (uint32_t i = 0; i < INSERT_TEXT_MAX_NUM; i++) {
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
        printf("=====OnInputStart.=====\n");
        std::thread t(InsertText);
        t.detach();
    }
    int32_t OnInputStop() override
    {
        printf("=====OnInputStop.=====\n");
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
        printf("=====IsEnable.=====\n");
        return true;
    }
    void NotifyPreemption() override
    {
        printf("=====NotifyPreemption.=====\n");
    }
};

void RegisterProxy()
{
    InputMethodAbilityInterface::GetInstance().SetImeListener(std::make_shared<InputMethodEngineListenerImpl>());
    auto ret = InputMethodAbilityInterface::GetInstance().RegisteredProxy();
    if (ret != ErrorCode::NO_ERROR) {
        printf("=====Register proxy ime failed:%d.=====\n", ret);
        return;
    }
    printf("=====Register proxy ime succeed.=====\n");
}

void RegisterProxyLoop()
{
    for (uint32_t i = 0; i < REGISTER_MAX_NUM; i++) {
        InputMethodAbilityInterface::GetInstance().SetImeListener(std::make_shared<InputMethodEngineListenerImpl>());
        auto ret = InputMethodAbilityInterface::GetInstance().RegisteredProxy();
        if (ret != ErrorCode::NO_ERROR) {
            printf("=====RegisterProxyLoop::Register proxy ime failed:%d.=====\n", ret);
            return;
        }
        printf("=====RegisterProxyLoop::Register proxy ime succeed.=====\n");
    }
}

void UnRegisterProxy()
{
    auto ret = InputMethodAbilityInterface::GetInstance().UnRegisteredProxy(UnRegisteredType::REMOVE_PROXY_IME);
    if (ret != ErrorCode::NO_ERROR) {
        printf("=====UnRegister proxy ime failed:%d.=====\n", ret);
        return;
    }
    printf("=====UnRegister proxy ime succeed.=====\n");
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
    int32_t input = 0;
    // 4: input 4
    while (input != 4) {
        printf("=====1:RegisterProxy  2:UnRegisterProxy  3:RegisterProxyLoop  4:exit=====\n");
        printf("input: ");
        cin >> input;
        switch (input) {
            // 1: input 1
            case 1:
                RegisterProxy();
                break;
            // 2: input 2
            case 2:
                UnRegisterProxy();
                break;
            // 3: input 3
            case 3:
                RegisterProxyLoop();
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