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

#include "input_method_ability_interface.h"
#include "sys_cfg_parser.h"

using namespace std;
using namespace OHOS::MiscServices;
class InputMethodEngineListenerImpl : public InputMethodEngineListener {
public:
    InputMethodEngineListenerImpl() = default;
    ~InputMethodEngineListenerImpl() = default;
    void OnKeyboardStatus(bool isShow) override { };
    void OnInputStart() override
    {
        IMSA_HILOGI("[ImeMirrorLog] OnInputStart");
    }
    int32_t OnInputStop() override
    {
        IMSA_HILOGI("[ImeMirrorLog] OnInputStop");
        return 0;
    }
    void OnInputFinish() override
    {
        IMSA_HILOGI("[ImeMirrorLog] OnInputFinish");
    }
    void OnSetCallingWindow(uint32_t windowId) override { }
    void OnSetSubtype(const SubProperty &property) override { }
    void ReceivePrivateCommand(const std::unordered_map<std::string, PrivateDataValue> &privateCommand) override { }
};
class KeyboardListenerImpl : public KeyboardListener {
public:
    KeyboardListenerImpl() = default;
    ~KeyboardListenerImpl() = default;
    bool OnDealKeyEvent(
        const std::shared_ptr<OHOS::MMI::KeyEvent> &keyEvent, OHOS::sptr<KeyEventConsumerProxy> &consumer) override
    {
        return false;
    }
    bool OnKeyEvent(int32_t keyCode, int32_t keyStatus, OHOS::sptr<KeyEventConsumerProxy> &consumer) override
    {
        return false;
    }
    bool OnKeyEvent(
        const std::shared_ptr<OHOS::MMI::KeyEvent> &keyEvent, OHOS::sptr<KeyEventConsumerProxy> &consumer) override
    {
        return false;
    }
    void OnCursorUpdate(int32_t positionX, int32_t positionY, int32_t height) override
    {
        IMSA_HILOGI("[ImeMirrorLog] OnCursorUpdate positionX:%{public}d,positionY:%{public}d,height:%{public}d",
            positionX, positionY, height);
    }
    void OnSelectionChange(int32_t oldBegin, int32_t oldEnd, int32_t newBegin, int32_t newEnd) override
    {
        IMSA_HILOGI("[ImeMirrorLog] OnSelectionChange "
                    "oldBegin:%{public}d,oldEnd:%{public}d,newBegin:%{public}d,newEnd:%{public}d",
            oldBegin, oldEnd, newBegin, newEnd);
    }
    void OnTextChange(const std::string &text) override
    {
        IMSA_HILOGI("[ImeMirrorLog] OnTextChange text:%{public}s", text.c_str());
    }
    void OnEditorAttributeChange(const InputAttribute &inputAttribute) override
    {
        IMSA_HILOGI(
            "[ImeMirrorLog] OnEditorAttributeChange inputAttribute:%{public}s", inputAttribute.ToString().c_str());
    }

    void OnFunctionKey(int32_t funcKey) override
    {
        IMSA_HILOGI("[ImeMirrorLog] OnFunctionKey funcKey:%{public}d", funcKey);
    }
};

int32_t GetAgentUid()
{
    SystemConfig systemConfig;
    SysCfgParser::ParseSystemConfig(systemConfig);
    bool isImeMirrorFeatureEnabled =
        systemConfig.supportedCapacityList.find("ime_mirror") != systemConfig.supportedCapacityList.end();
    if (!isImeMirrorFeatureEnabled) {
        return -1;
    }

    if (systemConfig.proxyImeUidList.empty()) {
        return -1;
    }
    return *systemConfig.proxyImeUidList.begin();
}
int main()
{
    std::shared_ptr<InputMethodEngineListener> imeListener = make_shared<InputMethodEngineListenerImpl>();
    auto instance = InputMethodAbilityInterface::GetInstance();
    instance.SetImeListener(imeListener);
    std::shared_ptr<KeyboardListener> kdListener = make_shared<KeyboardListenerImpl>();
    instance.SetKdListener(kdListener);
    char input = '0';
    int32_t ret = 0;
    int32_t uid = GetAgentUid();
    if (uid == -1) {
        IMSA_HILOGE("[ImeMirrorLog] GetAgentUid failed, please check system config");
        return 0;
    }
    setuid(uid);
    while (input != 'q') {
        cin >> input;
        switch (input) {
            case 'b':
                ret = instance.BindImeMirror();
                IMSA_HILOGI("[ImeMirrorLog] BindImeMirror finish ret = %{public}d", ret);
                break;
            case 'u':
                ret = instance.UnBindImeMirror();
                IMSA_HILOGI("[ImeMirrorLog] UnBindImeMirror finish ret = %{public}d", ret);
                break;
            case 'i':
                ret = instance.InsertText("ime mirror demo");
                IMSA_HILOGI("[ImeMirrorLog] InsertText finish ret = %{public}d", ret);
                break;
            default:
                IMSA_HILOGE("[ImeMirrorLog] input error");
        }
    }
    IMSA_HILOGI("[ImeMirrorLog] quit...");
    return 0;
}