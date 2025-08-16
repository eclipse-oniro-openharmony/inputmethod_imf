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
using namespace OHOS;
using namespace OHOS::MiscServices;
class InputMethodEngineListenerImpl : public InputMethodEngineListener {
public:
    InputMethodEngineListenerImpl() = default;
    ~InputMethodEngineListenerImpl() = default;
    void OnKeyboardStatus(bool isShow) override { };
    void OnInputStart() override
    {
        inputStartCount_++;
        IMSA_HILOGI("[ImeMirrorLog] OnInputStart count, inputStartCount_:%{public}u", inputStartCount_.load());
    }
    int32_t OnInputStop() override
    {
        inputStopCount_++;
        IMSA_HILOGI("[ImeMirrorLog] OnInputStop, inputStopCount_:%{public}u", inputStopCount_.load());
        return 0;
    }
    void OnInputFinish() override
    {
        inputFinishCount_++;
        IMSA_HILOGI("[ImeMirrorLog] OnInputFinish inputFinishCount_:%{public}u", inputFinishCount_.load());
        }
    void OnSetCallingWindow(uint32_t windowId) override { }
    void OnSetSubtype(const SubProperty &property) override { }
    void ReceivePrivateCommand(const std::unordered_map<std::string, PrivateDataValue> &privateCommand) override { }
    void PrintCount()
    {
        IMSA_HILOGI(
            "[ImeMirrorLog] inputStartCount_:%{public}u, inputFinishCount_:%{public}u, inputStopCount_:%{public}u",
            inputStartCount_.load(), inputFinishCount_.load(), inputStopCount_.load());
    };

    void ResetCount()
    {
        inputStartCount_ = 0;
        inputFinishCount_ = 0;
        inputStopCount_ = 0;
    }

private:
    std::atomic<uint32_t> inputStartCount_ = 0;
    std::atomic<uint32_t> inputFinishCount_ = 0;
    std::atomic<uint32_t> inputStopCount_ = 0;
};
class KeyboardListenerImpl : public KeyboardListener {
public:
    KeyboardListenerImpl() = default;
    ~KeyboardListenerImpl() = default;
    bool OnDealKeyEvent(const std::shared_ptr<MMI::KeyEvent> &keyEvent, uint64_t cbId,
        const sptr<IRemoteObject> &channelObject) override
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
        cursorUpdateCount_++;
        IMSA_HILOGI("[ImeMirrorLog] OnCursorUpdate positionX:%{public}d,positionY:%{public}d,height:%{public}d, "
                    "cursorUpdateCount_:%{public}u",
            positionX, positionY, height, cursorUpdateCount_.load());
        }
    void OnSelectionChange(int32_t oldBegin, int32_t oldEnd, int32_t newBegin, int32_t newEnd) override
    {
        selectionChangeCount_++;
        IMSA_HILOGI("[ImeMirrorLog] OnSelectionChange "
                    "oldBegin:%{public}d,oldEnd:%{public}d,newBegin:%{public}d,newEnd:%{public}d, "
                    "selectionChangeCount_:%{public}u",
            oldBegin, oldEnd, newBegin, newEnd, selectionChangeCount_.load());
    }
    void OnTextChange(const std::string &text) override
    {
        textChangeCount_++;
        IMSA_HILOGI("[ImeMirrorLog] OnTextChange text:%{public}s, textChangeCount_:%{public}u", text.c_str(),
            textChangeCount_.load());
    }
    void OnEditorAttributeChange(const InputAttribute &inputAttribute) override
    {
        IMSA_HILOGI(
            "[ImeMirrorLog] OnEditorAttributeChange inputAttribute:%{public}s", inputAttribute.ToString().c_str());
    }

    void OnFunctionKey(int32_t funcKey) override
    {
        if (funcKey < 0 || funcKey > static_cast<int32_t>(EnterKeyType::NEW_LINE)) {
            IMSA_HILOGE("[ImeMirrorLog] invalid funcKey:%{public}d", funcKey);
            return;
        }
        functionKeyCount_[funcKey]++;
        IMSA_HILOGI("[ImeMirrorLog] OnFunctionKey funcKey:%{public}d, functionKeyCount_:%{public}u", funcKey,
            functionKeyCount_[funcKey].load());
    }

    void PrintCount()
    {
        IMSA_HILOGI("[ImeMirrorLog] selectionChangeCount_:%{public}u, textChangeCount_:%{public}u, "
                    "cursorUpdateCount_:%{public}u",
            selectionChangeCount_.load(), textChangeCount_.load(), cursorUpdateCount_.load());
        for (size_t index = 0; index <= static_cast<size_t>(EnterKeyType::NEW_LINE); index++) {
            IMSA_HILOGI("[ImeMirrorLog] funcKey:%{public}zu, count:%{public}u", index, functionKeyCount_[index].load());
        }
    }

    void ResetCount()
    {
        selectionChangeCount_ = 0;
        textChangeCount_ = 0;
        cursorUpdateCount_ = 0;
        for (size_t index = 0; index <= static_cast<size_t>(EnterKeyType::NEW_LINE); index++) {
            functionKeyCount_[index] = 0;
        }
    }

private:
    std::atomic<uint32_t> selectionChangeCount_ = 0;
    std::atomic<uint32_t> textChangeCount_ = 0;
    std::atomic<uint32_t> functionKeyCount_[static_cast<size_t>(EnterKeyType::NEW_LINE) + 1] = { 0 };
    std::atomic<uint32_t> cursorUpdateCount_ = 0;
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
    std::shared_ptr<InputMethodEngineListenerImpl> imeListener = make_shared<InputMethodEngineListenerImpl>();
    auto instance = InputMethodAbilityInterface::GetInstance();
    instance.SetImeListener(imeListener);
    std::shared_ptr<KeyboardListenerImpl> kdListener = make_shared<KeyboardListenerImpl>();
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
                ret = instance.UnbindImeMirror();
                IMSA_HILOGI("[ImeMirrorLog] UnbindImeMirror finish ret = %{public}d", ret);
                break;
            case 'i':
                ret = instance.InsertText("ime mirror demo");
                IMSA_HILOGI("[ImeMirrorLog] InsertText finish ret = %{public}d", ret);
                break;
            case 'c':
                imeListener->PrintCount();
                kdListener->PrintCount();
                break;
            case 'r':
                imeListener->ResetCount();
                kdListener->ResetCount();
                IMSA_HILOGI("[ImeMirrorLog] retset count success");
                break;
            default:
                IMSA_HILOGE("[ImeMirrorLog] input error");
        }
    }
    IMSA_HILOGI("[ImeMirrorLog] quit...");
    return 0;
}