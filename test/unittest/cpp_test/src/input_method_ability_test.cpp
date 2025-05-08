/*
 * Copyright (C) 2021-2023 Huawei Device Co., Ltd.
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
#define private public
#define protected public
#include "input_method_ability.h"

#include "input_method_controller.h"
#include "input_method_system_ability.h"
#include "task_manager.h"
#undef private

#include <gtest/gtest.h>
#include <string_ex.h>
#include <unistd.h>

#include <cstdint>
#include <functional>
#include <string>
#include <thread>
#include <vector>

#include "global.h"
#include "iinput_data_channel.h"
#include "identity_checker_mock.h"
#include "input_attribute.h"
#include "input_control_channel_stub.h"
#include "input_data_channel_proxy.h"
#include "input_data_channel_service_impl.h"
#include "input_method_core_proxy.h"
#include "input_method_ability_interface.h"
#include "input_method_agent_service_impl.h"
#include "input_method_core_service_impl.h"
#include "input_method_core_stub.h"
#include "input_method_panel.h"
#include "inputmethod_message_handler.h"
#include "scope_utils.h"
#include "input_method_system_ability_proxy.h"
#include "tdd_util.h"
#include "text_listener.h"

using namespace testing::ext;
namespace OHOS {
namespace MiscServices {
constexpr uint32_t DEALY_TIME = 1;
class InputMethodAbilityTest : public testing::Test {
public:
    static std::mutex imeListenerCallbackLock_;
    static std::condition_variable imeListenerCv_;
    static bool showKeyboard_;
    static constexpr int CURSOR_DIRECTION_BASE_VALUE = 2011;
    static sptr<InputMethodController> imc_;
    static sptr<OnTextChangedListener> textListener_;
    static InputMethodAbility &inputMethodAbility_;
    static uint32_t windowId_;
    static int32_t security_;
    static uint64_t currentImeTokenId_;
    static int32_t currentImeUid_;
    static sptr<InputMethodSystemAbility> imsa_;
    static sptr<InputMethodSystemAbilityProxy> imsaProxy_;

    class InputMethodEngineListenerImpl : public InputMethodEngineListener {
    public:
        InputMethodEngineListenerImpl() = default;
        ~InputMethodEngineListenerImpl() = default;

        void OnKeyboardStatus(bool isShow)
        {
            showKeyboard_ = isShow;
            InputMethodAbilityTest::imeListenerCv_.notify_one();
            IMSA_HILOGI("InputMethodEngineListenerImpl OnKeyboardStatus");
        }

        void OnInputStart()
        {
            IMSA_HILOGI("InputMethodEngineListenerImpl OnInputStart");
        }

        int32_t OnInputStop()
        {
            IMSA_HILOGI("InputMethodEngineListenerImpl OnInputStop");
            return ErrorCode::NO_ERROR;
        }

        void OnSetCallingWindow(uint32_t windowId)
        {
            windowId_ = windowId;
            IMSA_HILOGI("InputMethodEngineListenerImpl OnSetCallingWindow");
        }

        void OnSetSubtype(const SubProperty &property)
        {
            IMSA_HILOGI("InputMethodEngineListenerImpl OnSetSubtype");
        }

        void OnSecurityChange(int32_t security)
        {
            security_ = security;
            IMSA_HILOGI("InputMethodEngineListenerImpl OnSecurityChange");
        }

        void ReceivePrivateCommand(const std::unordered_map<std::string, PrivateDataValue> &privateCommand)
        {
            IMSA_HILOGI("InputMethodEngineListenerImpl ReceivePrivateCommand");
        }

        void OnCallingDisplayIdChanged(uint64_t callingDisplayId)
        {
            IMSA_HILOGI("InputMethodEngineListenerImpl OnCallingDisplayIdChanged displayId:%{public}" PRIu64"",
                callingDisplayId);
        }
    };

    static void SetUpTestCase(void)
    {
        IdentityCheckerMock::ResetParam();
        // Set the tokenID to the tokenID of the current ime
        TddUtil::StorageSelfTokenID();
        imsa_ = new (std::nothrow) InputMethodSystemAbility();
        if (imsa_ == nullptr) {
            return;
        }
        imsa_->OnStart();
        imsa_->userId_ = TddUtil::GetCurrentUserId();
        imsa_->identityChecker_ = std::make_shared<IdentityCheckerMock>();
        sptr<InputMethodSystemAbilityStub> serviceStub = imsa_;
        imsaProxy_ = new (std::nothrow) InputMethodSystemAbilityProxy(serviceStub->AsObject());
        if (imsaProxy_ == nullptr) {
            return;
        }
        IdentityCheckerMock::SetFocused(true);

        std::shared_ptr<Property> property = InputMethodController::GetInstance()->GetCurrentInputMethod();
        auto currentIme = property != nullptr ? property->name : "default.inputmethod.unittest";
        currentImeTokenId_ = TddUtil::GetTestTokenID(currentIme);
        currentImeUid_ = TddUtil::GetUid(currentIme);

        inputMethodAbility_.abilityManager_ = imsaProxy_;
        TddUtil::InitCurrentImePermissionInfo();
        IdentityCheckerMock::SetBundleName(TddUtil::currentBundleNameMock_);
        inputMethodAbility_.SetCoreAndAgent();
        TaskManager::GetInstance().SetInited(true);

        TextListener::ResetParam();
        imc_ = InputMethodController::GetInstance();
        imc_->abilityManager_ = imsaProxy_;
        textListener_ = new TextListener();
    }
    static void TearDownTestCase(void)
    {
        IMSA_HILOGI("InputMethodAbilityTest::TearDownTestCase");
        imc_->Close();
        TextListener::ResetParam();
        TddUtil::RestoreSelfTokenID();
        IdentityCheckerMock::ResetParam();
        imsa_->OnStop();
    }
    static void GetIMCAttachIMA()
    {
        imc_->SetTextListener(textListener_);
        imc_->clientInfo_.state = ClientState::ACTIVE;
        imc_->isBound_.store(true);
        imc_->isEditable_.store(true);
        auto agent = inputMethodAbility_.agentStub_->AsObject();
        imc_->SetAgent(agent);

        sptr<IInputDataChannel> channel = iface_cast<IInputDataChannel>(imc_->clientInfo_.channel);
        inputMethodAbility_.SetInputDataChannel(channel->AsObject());
        IMSA_HILOGI("end");
    }
    static void GetIMCDetachIMA()
    {
        imc_->OnInputStop();
        inputMethodAbility_.ClearDataChannel(inputMethodAbility_.dataChannelObject_);
        IMSA_HILOGI("end");
    }
    void SetUp()
    {
        IMSA_HILOGI("InputMethodAbilityTest::SetUp");
        TaskManager::GetInstance().Reset();
    }
    void TearDown()
    {
        IMSA_HILOGI("InputMethodAbilityTest::TearDown");
    }
    void CheckPanelStatusInfo(const std::shared_ptr<InputMethodPanel> &panel, const PanelStatusInfo &info)
    {
        TextListener::ResetParam();
        info.visible ? CheckPanelInfoInShow(panel, info) : CheckPanelInfoInHide(panel, info);
    }
    void CheckPanelInfoInShow(const std::shared_ptr<InputMethodPanel> &panel, const PanelStatusInfo &info)
    {
        auto ret = inputMethodAbility_.ShowPanel(panel);
        EXPECT_EQ(ret, ErrorCode::NO_ERROR);
        if (info.panelInfo.panelFlag != FLG_CANDIDATE_COLUMN) {
            if (info.panelInfo.panelType == SOFT_KEYBOARD) {
                EXPECT_TRUE(TextListener::WaitSendKeyboardStatusCallback(KeyboardStatus::SHOW));
            } else {
                EXPECT_FALSE(TextListener::WaitSendKeyboardStatusCallback(KeyboardStatus::SHOW));
            }
            PanelStatusInfo statusInfo;
            statusInfo.panelInfo.panelType = info.panelInfo.panelType;
            statusInfo.panelInfo.panelFlag = info.panelInfo.panelFlag;
            statusInfo.visible = info.visible;
            statusInfo.trigger = info.trigger;
            
            EXPECT_TRUE(TextListener::WaitNotifyPanelStatusInfoCallback(statusInfo));
            return;
        }
        EXPECT_FALSE(TextListener::WaitSendKeyboardStatusCallback(KeyboardStatus::SHOW));
        PanelStatusInfo statusInfo1;
        statusInfo1.panelInfo.panelType = info.panelInfo.panelType;
        statusInfo1.panelInfo.panelFlag = info.panelInfo.panelFlag;
        statusInfo1.visible = info.visible;
        statusInfo1.trigger = info.trigger;
        
        EXPECT_FALSE(TextListener::WaitNotifyPanelStatusInfoCallback(statusInfo1));
    }
    void CheckPanelInfoInHide(const std::shared_ptr<InputMethodPanel> &panel, const PanelStatusInfo &info)
    {
        AccessScope scope(currentImeTokenId_, currentImeUid_);
        auto ret = inputMethodAbility_.HidePanel(panel);
        EXPECT_EQ(ret, ErrorCode::NO_ERROR);
        if (info.panelInfo.panelFlag != FLG_CANDIDATE_COLUMN) {
            if (info.panelInfo.panelType == SOFT_KEYBOARD) {
                EXPECT_TRUE(TextListener::WaitSendKeyboardStatusCallback(KeyboardStatus::HIDE));
            } else {
                EXPECT_FALSE(TextListener::WaitSendKeyboardStatusCallback(KeyboardStatus::HIDE));
            };
            PanelStatusInfo statusInfo1;
            statusInfo1.panelInfo.panelType = info.panelInfo.panelType;
            statusInfo1.panelInfo.panelFlag = info.panelInfo.panelFlag;
            statusInfo1.visible = info.visible;
            statusInfo1.trigger = info.trigger;
            
            EXPECT_TRUE(TextListener::WaitNotifyPanelStatusInfoCallback(statusInfo1));
            return;
        }
        EXPECT_FALSE(TextListener::WaitSendKeyboardStatusCallback(KeyboardStatus::HIDE));
        PanelStatusInfo statusInfo;
        statusInfo.panelInfo.panelType = info.panelInfo.panelType;
        statusInfo.panelInfo.panelFlag = info.panelInfo.panelFlag;
        statusInfo.visible = info.visible;
        statusInfo.trigger = info.trigger;
        
        EXPECT_FALSE(TextListener::WaitNotifyPanelStatusInfoCallback(statusInfo));
    }
};

std::mutex InputMethodAbilityTest::imeListenerCallbackLock_;
std::condition_variable InputMethodAbilityTest::imeListenerCv_;
bool InputMethodAbilityTest::showKeyboard_ = true;
sptr<InputMethodController> InputMethodAbilityTest::imc_;
sptr<OnTextChangedListener> InputMethodAbilityTest::textListener_;
InputMethodAbility &InputMethodAbilityTest::inputMethodAbility_ = InputMethodAbility::GetInstance();
uint32_t InputMethodAbilityTest::windowId_ = 0;
int32_t InputMethodAbilityTest::security_ = -1;
uint64_t InputMethodAbilityTest::currentImeTokenId_ = 0;
int32_t InputMethodAbilityTest::currentImeUid_ = 0;
sptr<InputMethodSystemAbility> InputMethodAbilityTest::imsa_;
sptr<InputMethodSystemAbilityProxy> InputMethodAbilityTest::imsaProxy_;

/**
 * @tc.name: testSerializedInputAttribute
 * @tc.desc: Checkout the serialization of InputAttribute.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodAbilityTest, testSerializedInputAttribute, TestSize.Level0)
{
    InputAttributeInner inAttribute;
    inAttribute.inputPattern = InputAttributeInner::PATTERN_PASSWORD;
    MessageParcel data;
    EXPECT_TRUE(inAttribute.Marshalling(data));
    InputAttributeInner outAttribute;
    auto attribute = InputAttributeInner::Unmarshalling(data);
    EXPECT_TRUE(attribute->GetSecurityFlag());
}

/**
 * @tc.name: testSerializedInputAttribute
 * @tc.desc: Checkout the serialization of InputAttribute.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodAbilityTest, testSerializedInputAttribute_WithSpecificBundleName, TestSize.Level0)
{
    InputAttributeInner inAttribute;
    inAttribute.bundleName = "com.example.inputmethod";
    MessageParcel data;
    EXPECT_TRUE(inAttribute.Marshalling(data));
    auto ret = InputAttributeInner::Unmarshalling(data);
    EXPECT_NE(ret, nullptr);
    EXPECT_EQ(inAttribute.bundleName, ret->bundleName);
}

/**
 * @tc.name: testShowKeyboardInputMethodCoreProxy
 * @tc.desc: Test InputMethodCoreProxy ShowKeyboard
 * @tc.type: FUNC
 * @tc.require: issueI5NXHK
 */
HWTEST_F(InputMethodAbilityTest, testShowKeyboardInputMethodCoreProxy, TestSize.Level0)
{
    IMSA_HILOGI("testShowKeyboardInputMethodCoreProxy start.");
    sptr<InputMethodCoreStub> coreStub = new InputMethodCoreServiceImpl();
    sptr<IInputMethodCore> core = coreStub;
    sptr<InputDataChannelStub> channelStub = new InputDataChannelServiceImpl();
    inputMethodAbility_.SetImeListener(std::make_shared<InputMethodEngineListenerImpl>());

    MessageParcel data;
    data.WriteRemoteObject(core->AsObject());
    data.WriteRemoteObject(channelStub->AsObject());
    sptr<IRemoteObject> coreObject = data.ReadRemoteObject();
    sptr<IRemoteObject> channelObject = data.ReadRemoteObject();

    sptr<InputMethodCoreProxy> coreProxy = new InputMethodCoreProxy(coreObject);
    sptr<InputDataChannelProxy> channelProxy = new InputDataChannelProxy(channelObject);
    auto ret = coreProxy->ShowKeyboard(static_cast<int32_t>(RequestKeyboardReason::NONE));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ret = coreProxy->InitInputControlChannel(nullptr);
    EXPECT_EQ(ERR_INVALID_DATA, ret);

    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_EQ(showKeyboard_, true);
}

/**
 * @tc.name: testShowKeyboardWithoutImeListener
 * @tc.desc: InputMethodAbility ShowKeyboard without imeListener
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodAbilityTest, testShowKeyboardWithoutImeListener, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbilityTest testShowKeyboardWithoutImeListener start.");
    auto ret = inputMethodAbility_.ShowKeyboard(static_cast<int32_t>(RequestKeyboardReason::NONE));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: testHideKeyboardWithoutImeListener
 * @tc.desc: InputMethodAbility HideKeyboard without imeListener
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodAbilityTest, testHideKeyboardWithoutImeListener, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbilityTest testHideKeyboardWithoutImeListener start.");
    auto ret = inputMethodAbility_.HideKeyboard();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: testStartInputWithoutPanel
 * @tc.desc: InputMethodAbility StartInput Without Panel
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodAbilityTest, testStartInputWithoutPanel, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbilityTest testStartInputWithoutAttach start.");
    inputMethodAbility_.SetImeListener(std::make_shared<InputMethodEngineListenerImpl>());
    sptr<InputDataChannelStub> channelStub = new InputDataChannelServiceImpl();
    InputClientInfo clientInfo;
    clientInfo.channel = channelStub;
    auto ret = inputMethodAbility_.StartInput(clientInfo, false);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    clientInfo.isShowKeyboard = true;
    ret = inputMethodAbility_.StartInput(clientInfo, false);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: testStartInputBeforeCreatePanel
 * @tc.desc: InputMethodAbility StartInput before create panel
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodAbilityTest, testStartInputBeforeCreatePanel, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbilityTest testStartInputBeforeCreatePanel start.");
    inputMethodAbility_.panels_.Clear();
    inputMethodAbility_.SetImeListener(std::make_shared<InputMethodEngineListenerImpl>());
    auto ret = imc_->Attach(textListener_);
    EXPECT_EQ(ErrorCode::NO_ERROR, ret);
    {
        std::unique_lock<std::mutex> lock(InputMethodAbilityTest::imeListenerCallbackLock_);
        InputMethodAbilityTest::imeListenerCv_.wait_for(lock, std::chrono::seconds(DEALY_TIME), [] {
            return InputMethodAbilityTest::showKeyboard_;
        });
    }
    InputMethodAbilityTest::showKeyboard_ = false;
    std::shared_ptr<InputMethodPanel> softKeyboardPanel = nullptr;
    {
        AccessScope scope(currentImeTokenId_, currentImeUid_);
        PanelInfo panelInfo = {};
        panelInfo.panelType = SOFT_KEYBOARD;
        panelInfo.panelFlag = FLG_FIXED;
        ret = inputMethodAbility_.CreatePanel(nullptr, panelInfo, softKeyboardPanel);
        EXPECT_EQ(ErrorCode::NO_ERROR, ret);
    }
    {
        std::unique_lock<std::mutex> lock(InputMethodAbilityTest::imeListenerCallbackLock_);
        InputMethodAbilityTest::imeListenerCv_.wait_for(lock, std::chrono::seconds(DEALY_TIME), [] {
            return InputMethodAbilityTest::showKeyboard_;
        });
        EXPECT_TRUE(InputMethodAbilityTest::showKeyboard_);
    }
    imc_->Close();
    inputMethodAbility_.DestroyPanel(softKeyboardPanel);
}

/**
 * @tc.name: testHideKeyboardSelf
 * @tc.desc: InputMethodAbility HideKeyboardSelf
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: Hollokin
 */
HWTEST_F(InputMethodAbilityTest, testHideKeyboardSelf, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility testHideKeyboardSelf START");
    imc_->Attach(textListener_);
    std::unique_lock<std::mutex> lock(InputMethodAbilityTest::imeListenerCallbackLock_);
    InputMethodAbilityTest::showKeyboard_ = true;
    inputMethodAbility_.SetImeListener(std::make_shared<InputMethodEngineListenerImpl>());
    auto ret = inputMethodAbility_.HideKeyboardSelf();
    InputMethodAbilityTest::imeListenerCv_.wait_for(lock, std::chrono::seconds(DEALY_TIME), [] {
        return InputMethodAbilityTest::showKeyboard_ == false;
    });
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_FALSE(InputMethodAbilityTest::showKeyboard_);
}

/**
 * @tc.name: testMoveCursor
 * @tc.desc: InputMethodAbility MoveCursor
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: Hollokin
 */
HWTEST_F(InputMethodAbilityTest, testMoveCursor, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility MoveCursor Test START");
    constexpr int32_t keyCode = 4;
    auto ret = inputMethodAbility_.MoveCursor(keyCode); // move cursor right });
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(TextListener::WaitMoveCursor(keyCode));

    ret = InputMethodAbilityInterface::GetInstance().MoveCursor(keyCode);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(TextListener::WaitMoveCursor(keyCode));
}

/**
 * @tc.name: testInsertText
 * @tc.desc: InputMethodAbility InsertText
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: Hollokin
 */
HWTEST_F(InputMethodAbilityTest, testInsertText, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility InsertText Test START");
    std::string text = "text";
    std::u16string u16Text = Str8ToStr16(text);
    auto ret = inputMethodAbility_.InsertText(text);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(TextListener::WaitInsertText(u16Text));
}

/**
 * @tc.name: testSendFunctionKey
 * @tc.desc: InputMethodAbility SendFunctionKey
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: Hollokin
 */
HWTEST_F(InputMethodAbilityTest, testSendFunctionKey, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility SendFunctionKey Test START");
    constexpr int32_t funcKey = 1;
    auto ret = inputMethodAbility_.SendFunctionKey(funcKey);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(TextListener::WaitSendFunctionKey(funcKey));

    TextListener::ResetParam();
    ret = InputMethodAbilityInterface::GetInstance().SendFunctionKey(funcKey);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(TextListener::WaitSendFunctionKey(funcKey));
}

/**
 * @tc.name: testSendExtendAction
 * @tc.desc: InputMethodAbility SendExtendAction
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodAbilityTest, testSendExtendAction, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility SendExtendAction Test START");
    constexpr int32_t action = 1;
    auto ret = inputMethodAbility_.SendExtendAction(action);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(TextListener::WaitHandleExtendAction(action));
}

/**
 * @tc.name: testDeleteText
 * @tc.desc: InputMethodAbility DeleteForward & DeleteBackward
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: Hollokin
 */
HWTEST_F(InputMethodAbilityTest, testDeleteText, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility testDelete Test START");
    int32_t deleteForwardLenth = 1;
    auto ret = inputMethodAbility_.DeleteForward(deleteForwardLenth);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(TextListener::WaitDeleteBackward(deleteForwardLenth));

    ret = InputMethodAbilityInterface::GetInstance().DeleteForward(deleteForwardLenth);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(TextListener::WaitDeleteBackward(deleteForwardLenth));

    int32_t deleteBackwardLenth = 2;
    ret = InputMethodAbilityInterface::GetInstance().DeleteBackward(deleteBackwardLenth);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(TextListener::WaitDeleteForward(deleteBackwardLenth));
    ret = inputMethodAbility_.DeleteBackward(deleteBackwardLenth);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(TextListener::WaitDeleteForward(deleteBackwardLenth));
}

/**
 * @tc.name: testGetEnterKeyType
 * @tc.desc: InputMethodAbility GetEnterKeyType & GetInputPattern
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: Hollokin
 */
HWTEST_F(InputMethodAbilityTest, testGetEnterKeyType, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility testGetEnterKeyType START");
    Configuration config;
    EnterKeyType keyType = EnterKeyType::NEXT;
    config.SetEnterKeyType(keyType);
    TextInputType textInputType = TextInputType::DATETIME;
    config.SetTextInputType(textInputType);
    imc_->OnConfigurationChange(config);
    int32_t keyType2;
    auto ret = inputMethodAbility_.GetEnterKeyType(keyType2);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(keyType2, (int)keyType);
    int32_t inputPattern;
    ret = inputMethodAbility_.GetInputPattern(inputPattern);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(inputPattern, (int)textInputType);
}

/**
 * @tc.name: testGetTextConfig
 * @tc.desc: InputMethodAbility GetTextConfig
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: Hollokin
 */
HWTEST_F(InputMethodAbilityTest, testGetTextConfig, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility testGetTextConfig START");
    TextConfig textConfig;
    textConfig.inputAttribute = { .inputPattern = 0, .enterKeyType = 1 };
    auto ret = imc_->Attach(textListener_, false, textConfig);
    TextTotalConfig textTotalConfig;
    ret = inputMethodAbility_.GetTextConfig(textTotalConfig);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(textTotalConfig.inputAttribute.inputPattern, textConfig.inputAttribute.inputPattern);
    EXPECT_EQ(textTotalConfig.inputAttribute.enterKeyType, textConfig.inputAttribute.enterKeyType);

    InputAttribute inputAttribute;
    ret = InputMethodAbilityInterface::GetInstance().GetInputAttribute(inputAttribute);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(inputAttribute, textConfig.inputAttribute);
}

/**
 * @tc.name: testSelectByRange_001
 * @tc.desc: InputMethodAbility SelectByRange
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: Zhaolinglan
 */
HWTEST_F(InputMethodAbilityTest, testSelectByRange_001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility testSelectByRange_001 START");
    constexpr int32_t start = 1;
    constexpr int32_t end = 2;
    auto ret = inputMethodAbility_.SelectByRange(start, end);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(TextListener::WaitHandleSetSelection(start, end));
}

/**
 * @tc.name: testSelectByRange_002
 * @tc.desc: InputMethodAbility SelectByRange
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodAbilityTest, testSelectByRange_002, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility testSelectByRange_002 START");
    int32_t start = -2;
    int32_t end = 2;
    auto ret = inputMethodAbility_.SelectByRange(start, end);
    EXPECT_EQ(ret, ErrorCode::ERROR_PARAMETER_CHECK_FAILED);

    start = 2;
    end = -2;
    ret = inputMethodAbility_.SelectByRange(start, end);
    EXPECT_EQ(ret, ErrorCode::ERROR_PARAMETER_CHECK_FAILED);
}

/**
 * @tc.name: testSelectByMovement
 * @tc.desc: InputMethodAbility SelectByMovement
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: Zhaolinglan
 */
HWTEST_F(InputMethodAbilityTest, testSelectByMovement, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility testSelectByMovement START");
    constexpr int32_t direction = 1;
    auto ret = inputMethodAbility_.SelectByMovement(direction);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(TextListener::WaitHandleSelect(direction + InputMethodAbilityTest::CURSOR_DIRECTION_BASE_VALUE, 0));
}

/**
 * @tc.name: testGetTextAfterCursor
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodAbilityTest, testGetTextAfterCursor, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility testGetTextAfterCursor START");
    int32_t number = 3;
    std::u16string text;
    auto ret = inputMethodAbility_.GetTextAfterCursor(number, text);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(text, Str8ToStr16(TextListener::TEXT_AFTER_CURSOR));
}

/**
 * @tc.name: testGetTextBeforeCursor
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodAbilityTest, testGetTextBeforeCursor, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility testGetTextBeforeCursor START");
    int32_t number = 5;
    std::u16string text;
    auto ret = inputMethodAbility_.GetTextBeforeCursor(number, text);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(text, Str8ToStr16(TextListener::TEXT_BEFORE_CURSOR));
}

/**
 * @tc.name: testGetTextIndexAtCursor
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodAbilityTest, testGetTextIndexAtCursor, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility testGetTextIndexAtCursor START");
    int32_t index;
    auto ret = inputMethodAbility_.GetTextIndexAtCursor(index);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(index, TextListener::TEXT_INDEX);
}

/**
 * @tc.name: testCreatePanel001
 * @tc.desc: It's allowed to create one SOFT_KEYBOARD panel, but two is denied.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodAbilityTest, testCreatePanel001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbilityTest testCreatePanel001 START. You can not create two SOFT_KEYBOARD panel.");
    AccessScope scope(currentImeTokenId_, currentImeUid_);
    std::shared_ptr<InputMethodPanel> softKeyboardPanel1 = nullptr;
    PanelInfo panelInfo = {};
    panelInfo.panelType = SOFT_KEYBOARD;
    panelInfo.panelFlag = FLG_FIXED;
    auto ret = inputMethodAbility_.CreatePanel(nullptr, panelInfo, softKeyboardPanel1);
    EXPECT_TRUE(softKeyboardPanel1 != nullptr);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    std::shared_ptr<InputMethodPanel> softKeyboardPanel2 = nullptr;
    ret = inputMethodAbility_.CreatePanel(nullptr, panelInfo, softKeyboardPanel2);
    EXPECT_TRUE(softKeyboardPanel2 == nullptr);
    EXPECT_EQ(ret, ErrorCode::ERROR_OPERATE_PANEL);

    ret = inputMethodAbility_.DestroyPanel(softKeyboardPanel1);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ret = inputMethodAbility_.DestroyPanel(softKeyboardPanel2);
    EXPECT_EQ(ret, ErrorCode::ERROR_BAD_PARAMETERS);
}

/**
 * @tc.name: testCreatePanel002
 * @tc.desc: It's allowed to create one STATUS_BAR panel, but two is denied.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodAbilityTest, testCreatePanel002, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbilityTest testCreatePanel002 START. You can not create two STATUS_BAR panel.");
    AccessScope scope(currentImeTokenId_, currentImeUid_);
    std::shared_ptr<InputMethodPanel> statusBar1 = nullptr;
    PanelInfo panelInfo = {};
    panelInfo.panelType = STATUS_BAR;
    auto ret = inputMethodAbility_.CreatePanel(nullptr, panelInfo, statusBar1);
    EXPECT_TRUE(statusBar1 != nullptr);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    std::shared_ptr<InputMethodPanel> statusBar2 = nullptr;
    ret = inputMethodAbility_.CreatePanel(nullptr, panelInfo, statusBar2);
    EXPECT_TRUE(statusBar2 == nullptr);
    EXPECT_EQ(ret, ErrorCode::ERROR_OPERATE_PANEL);

    ret = inputMethodAbility_.DestroyPanel(statusBar1);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ret = inputMethodAbility_.DestroyPanel(statusBar2);
    EXPECT_EQ(ret, ErrorCode::ERROR_BAD_PARAMETERS);
}

/**
 * @tc.name: testCreatePanel003
 * @tc.desc: It's allowed to create one STATUS_BAR panel and one SOFT_KEYBOARD panel.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodAbilityTest, testCreatePanel003, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbilityTest testCreatePanel006 START. Allowed to create one SOFT_KEYBOARD panel and "
                "one STATUS_BAR panel.");
    AccessScope scope(currentImeTokenId_, currentImeUid_);
    std::shared_ptr<InputMethodPanel> softKeyboardPanel = nullptr;
    PanelInfo panelInfo1;
    panelInfo1.panelType = SOFT_KEYBOARD;
    panelInfo1.panelFlag = FLG_FIXED;
    auto ret = inputMethodAbility_.CreatePanel(nullptr, panelInfo1, softKeyboardPanel);
    EXPECT_TRUE(softKeyboardPanel != nullptr);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    PanelInfo panelInfo2;
    panelInfo2.panelType = STATUS_BAR;
    std::shared_ptr<InputMethodPanel> statusBar = nullptr;
    ret = inputMethodAbility_.CreatePanel(nullptr, panelInfo2, statusBar);
    EXPECT_TRUE(statusBar != nullptr);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ret = inputMethodAbility_.DestroyPanel(softKeyboardPanel);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ret = inputMethodAbility_.DestroyPanel(statusBar);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: testCreatePanel004
 * @tc.desc: It's allowed to create one STATUS_BAR panel and one SOFT_KEYBOARD panel.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodAbilityTest, testCreatePanel004, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbilityTest testCreatePanel006 START. Allowed to create one SOFT_KEYBOARD panel and "
                "one STATUS_BAR panel.");
    AccessScope scope(currentImeTokenId_, currentImeUid_);
    std::shared_ptr<InputMethodPanel> inputMethodPanel = nullptr;
    PanelInfo panelInfo = {};
    panelInfo.panelType = SOFT_KEYBOARD;
    panelInfo.panelFlag = FLG_FIXED;
    auto ret = inputMethodAbility_.CreatePanel(nullptr, panelInfo, inputMethodPanel);
    EXPECT_TRUE(inputMethodPanel != nullptr);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ret = inputMethodAbility_.DestroyPanel(inputMethodPanel);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    PanelInfo panelInfo1;
    panelInfo1.panelType = STATUS_BAR;
    ret = inputMethodAbility_.CreatePanel(nullptr, panelInfo1, inputMethodPanel);
    EXPECT_TRUE(inputMethodPanel != nullptr);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ret = inputMethodAbility_.DestroyPanel(inputMethodPanel);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    PanelInfo panelInfo2;
    panelInfo2.panelFlag = FLG_FLOATING;
    ret = inputMethodAbility_.CreatePanel(nullptr, panelInfo2, inputMethodPanel);
    EXPECT_TRUE(inputMethodPanel != nullptr);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ret = inputMethodAbility_.DestroyPanel(inputMethodPanel);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: testCreatePanel005
 * @tc.desc: It's allowed to create one SOFT_KEYBOARD panel, but two is denied.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodAbilityTest, testCreatePanel005, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbilityTest testCreatePanel005 START.");
    AccessScope scope(currentImeTokenId_, currentImeUid_);
    std::shared_ptr<InputMethodPanel> softKeyboardPanel1 = nullptr;
    PanelInfo panelInfo = {};
    panelInfo.panelType = SOFT_KEYBOARD;
    panelInfo.panelFlag = FLG_FIXED;
    auto ret = inputMethodAbility_.CreatePanel(nullptr, panelInfo, softKeyboardPanel1);
    EXPECT_TRUE(softKeyboardPanel1 != nullptr);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ret = inputMethodAbility_.DestroyPanel(softKeyboardPanel1);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    std::shared_ptr<InputMethodPanel> softKeyboardPanel2 = nullptr;
    ret = inputMethodAbility_.CreatePanel(nullptr, panelInfo, softKeyboardPanel2);
    EXPECT_TRUE(softKeyboardPanel2 != nullptr);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ret = inputMethodAbility_.DestroyPanel(softKeyboardPanel2);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: testCreatePanel006
 * @tc.desc: It's allowed to create one SOFT_KEYBOARD panel, but two is denied.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodAbilityTest, testCreatePanel006, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbilityTest testCreatePanel006 START.");
    AccessScope scope(currentImeTokenId_, currentImeUid_);
    std::shared_ptr<InputMethodPanel> softKeyboardPanel1 = nullptr;
    PanelInfo panelInfo = {};
    panelInfo.panelType = SOFT_KEYBOARD;
    panelInfo.panelFlag = FLG_FIXED;
    auto ret = inputMethodAbility_.CreatePanel(nullptr, panelInfo, softKeyboardPanel1);
    EXPECT_TRUE(softKeyboardPanel1 != nullptr);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    std::shared_ptr<InputMethodPanel> softKeyboardPanel2 = nullptr;
    ret = inputMethodAbility_.CreatePanel(nullptr, panelInfo, softKeyboardPanel2);
    EXPECT_TRUE(softKeyboardPanel2 == nullptr);
    EXPECT_EQ(ret, ErrorCode::ERROR_OPERATE_PANEL);

    ret = inputMethodAbility_.DestroyPanel(softKeyboardPanel1);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    std::shared_ptr<InputMethodPanel> softKeyboardPanel3 = nullptr;
    ret = inputMethodAbility_.CreatePanel(nullptr, panelInfo, softKeyboardPanel3);
    EXPECT_TRUE(softKeyboardPanel3 != nullptr);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ret = inputMethodAbility_.DestroyPanel(softKeyboardPanel2);
    EXPECT_EQ(ret, ErrorCode::ERROR_BAD_PARAMETERS);

    ret = inputMethodAbility_.DestroyPanel(softKeyboardPanel3);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    std::this_thread::sleep_for(std::chrono::seconds(2));
}

/**
 * @tc.name: testSetCallingWindow001
 * @tc.desc: InputMethodAbility SetCallingWindow
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: Hollokin
 */
HWTEST_F(InputMethodAbilityTest, testSetCallingWindow001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility testSetCallingWindow001 START");
    std::unique_lock<std::mutex> lock(InputMethodAbilityTest::imeListenerCallbackLock_);
    InputMethodAbilityTest::showKeyboard_ = true;
    inputMethodAbility_.SetImeListener(std::make_shared<InputMethodEngineListenerImpl>());
    uint32_t windowId = 10;
    inputMethodAbility_.SetCallingWindow(windowId);
    InputMethodAbilityTest::imeListenerCv_.wait_for(lock, std::chrono::seconds(DEALY_TIME), [windowId] {
        return InputMethodAbilityTest::windowId_ == windowId;
    });
    EXPECT_EQ(InputMethodAbilityTest::windowId_, windowId);
    std::this_thread::sleep_for(std::chrono::seconds(2));
}

/**
 * @tc.name: testNotifyPanelStatusInfo_001
 * @tc.desc: ShowKeyboard HideKeyboard SOFT_KEYBOARD FLG_FIXED
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodAbilityTest, testNotifyPanelStatusInfo_001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility testNotifyPanelStatusInfo_001 START");
    imc_->Attach(textListener_);
    PanelInfo info;
    info.panelType = STATUS_BAR;
    auto panel = std::make_shared<InputMethodPanel>();
    AccessScope scope(currentImeTokenId_, currentImeUid_);
    auto ret = inputMethodAbility_.CreatePanel(nullptr, info, panel);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    auto panel1 = std::make_shared<InputMethodPanel>();
    PanelInfo info1;
    info1.panelType = SOFT_KEYBOARD;
    info1.panelFlag = FLG_FIXED;
    ret = inputMethodAbility_.CreatePanel(nullptr, info1, panel1);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    TextListener::ResetParam();
    ret = inputMethodAbility_.ShowKeyboard(static_cast<int32_t>(RequestKeyboardReason::NONE));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(TextListener::WaitSendKeyboardStatusCallback(KeyboardStatus::SHOW));
    PanelStatusInfo statusInfo;
    statusInfo.panelInfo = info1;
    statusInfo.visible = true;
    statusInfo.trigger = Trigger::IMF;
    EXPECT_TRUE(TextListener::WaitNotifyPanelStatusInfoCallback(statusInfo));

    TextListener::ResetParam();
    ret = inputMethodAbility_.HideKeyboard();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(TextListener::WaitSendKeyboardStatusCallback(KeyboardStatus::HIDE));
    PanelStatusInfo statusInfo1;
    statusInfo1.panelInfo = info1;
    statusInfo1.visible = false;
    statusInfo1.trigger = Trigger::IMF;
    EXPECT_TRUE(TextListener::WaitNotifyPanelStatusInfoCallback(statusInfo1));

    ret = inputMethodAbility_.DestroyPanel(panel);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = inputMethodAbility_.DestroyPanel(panel1);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    std::this_thread::sleep_for(std::chrono::seconds(2));
}

/**
 * @tc.name: testNotifyPanelStatusInfo_002
 * @tc.desc: ShowPanel HidePanel SOFT_KEYBOARD  FLG_FLOATING
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodAbilityTest, testNotifyPanelStatusInfo_002, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility testNotifyPanelStatusInfo_002 START");
    imc_->Attach(textListener_);
    PanelInfo info1;
    info1.panelType = SOFT_KEYBOARD;
    info1.panelFlag = FLG_FLOATING;
    AccessScope scope(currentImeTokenId_, currentImeUid_);
    auto panel = std::make_shared<InputMethodPanel>();
    auto ret = inputMethodAbility_.CreatePanel(nullptr, info1, panel);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    PanelStatusInfo statusInfo;
    statusInfo.panelInfo = info1;
    statusInfo.visible = true;
    statusInfo.trigger = Trigger::IME_APP;
    // ShowPanel
    CheckPanelStatusInfo(panel, statusInfo);
    PanelStatusInfo statusInfo1;
    statusInfo1.panelInfo = info1;
    statusInfo1.visible = false;
    statusInfo1.trigger = Trigger::IME_APP;
    // HidePanel
    CheckPanelStatusInfo(panel, statusInfo1);

    ret = inputMethodAbility_.DestroyPanel(panel);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    std::this_thread::sleep_for(std::chrono::seconds(2));
}

/**
 * @tc.name: testNotifyPanelStatusInfo_003
 * @tc.desc: ShowPanel HidePanel STATUS_BAR
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodAbilityTest, testNotifyPanelStatusInfo_003, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility testNotifyPanelStatusInfo_003 START");
    imc_->Attach(textListener_);
    PanelInfo panelInfo = {};
    panelInfo.panelType = STATUS_BAR;
    auto panel = std::make_shared<InputMethodPanel>();
    AccessScope scope(currentImeTokenId_, currentImeUid_);
    auto ret = inputMethodAbility_.CreatePanel(nullptr, panelInfo, panel);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    PanelStatusInfo statusInfo;
    statusInfo.panelInfo = panelInfo;
    statusInfo.visible = true;
    statusInfo.trigger = Trigger::IME_APP;
    // ShowPanel
    CheckPanelStatusInfo(panel, statusInfo);
    PanelStatusInfo statusInfo1;
    statusInfo1.panelInfo = panelInfo;
    statusInfo1.visible = false;
    statusInfo1.trigger = Trigger::IME_APP;
    // HidePanel
    CheckPanelStatusInfo(panel, statusInfo1);

    ret = inputMethodAbility_.DestroyPanel(panel);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: testNotifyPanelStatusInfo_004
 * @tc.desc: ShowPanel HidePanel SOFT_KEYBOARD  FLG_CANDIDATE_COLUMN
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodAbilityTest, testNotifyPanelStatusInfo_004, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility testNotifyPanelStatusInfo_004 START");
    imc_->Attach(textListener_);
    PanelInfo info;
    info.panelType = SOFT_KEYBOARD;
    info.panelFlag = FLG_CANDIDATE_COLUMN;
    auto panel = std::make_shared<InputMethodPanel>();
    AccessScope scope(currentImeTokenId_, currentImeUid_);
    auto ret = inputMethodAbility_.CreatePanel(nullptr, info, panel);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    PanelStatusInfo statusInfo;
    statusInfo.panelInfo = info;
    statusInfo.visible = true;
    statusInfo.trigger = Trigger::IME_APP;
    // ShowPanel
    CheckPanelStatusInfo(panel, statusInfo);
    PanelStatusInfo statusInfo1;
    statusInfo1.panelInfo = info;
    statusInfo1.visible = false;
    statusInfo1.trigger = Trigger::IME_APP;
    // HidePanel
    CheckPanelStatusInfo(panel, statusInfo1);

    ret = inputMethodAbility_.DestroyPanel(panel);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: testNotifyPanelStatusInfo_005
 * @tc.desc: HideKeyboardSelf
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodAbilityTest, testNotifyPanelStatusInfo_005, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility testNotifyPanelStatusInfo_005 START");
    PanelInfo info;
    info.panelType = SOFT_KEYBOARD;
    info.panelFlag = FLG_FLOATING;
    imc_->Attach(textListener_);

    // has no panel
    TextListener::ResetParam();
    auto ret = inputMethodAbility_.HideKeyboardSelf();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(TextListener::WaitSendKeyboardStatusCallback(KeyboardStatus::HIDE));
    PanelStatusInfo statusInfo;
    statusInfo.panelInfo = info;
    statusInfo.visible = false;
    statusInfo.trigger = Trigger::IME_APP;
    EXPECT_FALSE(TextListener::WaitNotifyPanelStatusInfoCallback(statusInfo));

    AccessScope scope(currentImeTokenId_, currentImeUid_);
    auto panel = std::make_shared<InputMethodPanel>();
    ret = inputMethodAbility_.CreatePanel(nullptr, info, panel);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = inputMethodAbility_.ShowPanel(panel);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    // has panel
    TextListener::ResetParam();
    ret = inputMethodAbility_.HideKeyboardSelf();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(TextListener::WaitSendKeyboardStatusCallback(KeyboardStatus::HIDE));
    EXPECT_TRUE(TextListener::WaitNotifyPanelStatusInfoCallback(statusInfo));

    ret = inputMethodAbility_.DestroyPanel(panel);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: testNotifyKeyboardHeight_001
 * @tc.desc: NotifyKeyboardHeight SOFT_KEYBOARD  FLG_FIXED
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: mashaoyin
 */
HWTEST_F(InputMethodAbilityTest, testNotifyKeyboardHeight_001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility testNotifyKeyboardHeight_001 START");
    imc_->Attach(textListener_);
    AccessScope scope(currentImeTokenId_, currentImeUid_);
    TextListener::ResetParam();
    inputMethodAbility_.NotifyKeyboardHeight(1, FLG_FIXED);
    EXPECT_TRUE(TextListener::WaitNotifyKeyboardHeightCallback(1));
}

/**
 * @tc.name: testNotifyKeyboardHeight_002
 * @tc.desc: NotifyKeyboardHeight SOFT_KEYBOARD  FLG_CANDIDATE_COLUMN
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: mashaoyin
 */
HWTEST_F(InputMethodAbilityTest, testNotifyKeyboardHeight_002, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility testNotifyKeyboardHeight_002 START");
    imc_->Attach(textListener_);
    AccessScope scope(currentImeTokenId_, currentImeUid_);
    TextListener::ResetParam();
    inputMethodAbility_.NotifyKeyboardHeight(1, FLG_CANDIDATE_COLUMN);
    EXPECT_TRUE(TextListener::WaitNotifyKeyboardHeightCallback(0));
}

/**
 * @tc.name: testNotifyKeyboardHeight_003
 * @tc.desc: NotifyKeyboardHeight Attach with hard keyboard
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: mashaoyin
 */
HWTEST_F(InputMethodAbilityTest, testNotifyKeyboardHeight_003, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility testNotifyKeyboardHeight_003 START");
    TextListener::ResetParam();
    AccessScope scope(currentImeTokenId_, currentImeUid_);
    PanelInfo info;
    info.panelType = SOFT_KEYBOARD;
    info.panelFlag = FLG_CANDIDATE_COLUMN;
    auto panel = std::make_shared<InputMethodPanel>();
    auto ret = inputMethodAbility_.CreatePanel(nullptr, info, panel);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    panel->Resize(1, 1);
    imc_->Attach(textListener_);
    EXPECT_TRUE(TextListener::WaitNotifyKeyboardHeightCallback(0));
    ret = inputMethodAbility_.DestroyPanel(panel);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: testAdjustKeyboard_001
 * @tc.desc: adjust keyboard
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: guojin
 */
HWTEST_F(InputMethodAbilityTest, testAdjustKeyboard_001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility testAdjustKeyboard_001 START");
    AccessScope scope(currentImeTokenId_, currentImeUid_);
    PanelInfo panelInfo = {};
    panelInfo.panelType = SOFT_KEYBOARD;
    panelInfo.panelFlag = FLG_FIXED;
    auto panel = std::make_shared<InputMethodPanel>();
    auto ret = inputMethodAbility_.CreatePanel(nullptr, panelInfo, panel);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ret = inputMethodAbility_.AdjustKeyboard();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ret = inputMethodAbility_.DestroyPanel(panel);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: testGetInputType_001
 * @tc.desc: get inputType
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: guojin
 */
HWTEST_F(InputMethodAbilityTest, testGetInputType_001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility testGetInputType_001 START");
    imc_->Attach(textListener_);

    InputType inputType = inputMethodAbility_.GetInputType();
    EXPECT_EQ(inputType, InputType::NONE);
}

/**
 * @tc.name: testOnSecurityChange
 * @tc.desc: OnSecurityChange
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodAbilityTest, testOnSecurityChange, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility testOnSecurityChange START");
    int32_t security = 32;
    inputMethodAbility_.SetImeListener(std::make_shared<InputMethodEngineListenerImpl>());
    auto ret = inputMethodAbility_.OnSecurityChange(security);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(InputMethodAbilityTest::security_, security);
}

/**
 * @tc.name: testSendPrivateCommand_001
 * @tc.desc: IMA SendPrivateCommand current is not default ime.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: mashaoyin
 */
HWTEST_F(InputMethodAbilityTest, testSendPrivateCommand_001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility testSendPrivateCommand_001 Test START");
    IdentityCheckerMock::SetBundleNameValid(false);
    TextListener::ResetParam();
    InputMethodAbilityTest::GetIMCDetachIMA();
    TddUtil::RestoreSelfTokenID();
    std::unordered_map<std::string, PrivateDataValue> privateCommand;
    auto ret = inputMethodAbility_.SendPrivateCommand(privateCommand);
    EXPECT_EQ(ret, ErrorCode::ERROR_NOT_DEFAULT_IME);
    IdentityCheckerMock::SetBundleNameValid(true);
}

/**
 * @tc.name: testSendPrivateCommand_002
 * @tc.desc: IMA SendPrivateCommand current data specification, default ime, not bound.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: mashaoyin
 */
HWTEST_F(InputMethodAbilityTest, testSendPrivateCommand_002, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility testSendPrivateCommand_002 Test START");
    InputMethodAbilityTest::GetIMCDetachIMA();
    IdentityCheckerMock::SetBundleNameValid(true);
    std::unordered_map<std::string, PrivateDataValue> privateCommand;
    PrivateDataValue privateDataValue1 = std::string("stringValue");
    privateCommand.insert({ "value1", privateDataValue1 });
    auto ret = inputMethodAbility_.SendPrivateCommand(privateCommand);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);
    IdentityCheckerMock::SetBundleNameValid(false);
}

/**
 * @tc.name: testSendPrivateCommand_003
 * @tc.desc: IMA SendPrivateCommand with correct data specification and all data type.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: mashaoyin
 */
HWTEST_F(InputMethodAbilityTest, testSendPrivateCommand_003, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility testSendPrivateCommand_003 Test START");
    TextListener::ResetParam();
    InputMethodAbilityTest::GetIMCAttachIMA();
    IdentityCheckerMock::SetBundleNameValid(true);
    std::unordered_map<std::string, PrivateDataValue> privateCommand;
    PrivateDataValue privateDataValue1 = std::string("stringValue");
    PrivateDataValue privateDataValue2 = true;
    PrivateDataValue privateDataValue3 = 100;
    privateCommand.emplace("value1", privateDataValue1);
    privateCommand.emplace("value2", privateDataValue2);
    privateCommand.emplace("value3", privateDataValue3);
    auto ret = inputMethodAbility_.SendPrivateCommand(privateCommand);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(TextListener::WaitSendPrivateCommandCallback(privateCommand));
    InputMethodAbilityTest::GetIMCDetachIMA();
    IdentityCheckerMock::SetBundleNameValid(false);
}

/**
 * @tc.name: testGetCallingWindowInfo_001
 * @tc.desc: GetCallingWindowInfo with IMC not bound
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: zhaolinglan
 */
HWTEST_F(InputMethodAbilityTest, testGetCallingWindowInfo_001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility testGetCallingWindowInfo_001 Test START");
    InputMethodAbilityTest::GetIMCDetachIMA();
    CallingWindowInfo windowInfo;
    int32_t ret = InputMethodAbilityTest::inputMethodAbility_.GetCallingWindowInfo(windowInfo);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_FOUND);
}

/**
 * @tc.name: testGetCallingWindowInfo_002
 * @tc.desc: GetCallingWindowInfo with panel not created
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: zhaolinglan
 */
HWTEST_F(InputMethodAbilityTest, testGetCallingWindowInfo_002, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility testGetCallingWindowInfo_002 Test START");
    AccessScope accessScope(InputMethodAbilityTest::currentImeTokenId_, InputMethodAbilityTest::currentImeUid_);
    // bind IMC
    InputMethodAbilityTest::GetIMCAttachIMA();
    // no panel is created
    InputMethodAbilityTest::inputMethodAbility_.panels_.Clear();
    CallingWindowInfo windowInfo;
    int32_t ret = InputMethodAbilityTest::inputMethodAbility_.GetCallingWindowInfo(windowInfo);
    EXPECT_EQ(ret, ErrorCode::ERROR_PANEL_NOT_FOUND);
    InputMethodAbilityTest::GetIMCDetachIMA();
}

/**
 * @tc.name: testGetCallingWindowInfo_003
 * @tc.desc: GetCallingWindowInfo with only status_bar created
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: zhaolinglan
 */
HWTEST_F(InputMethodAbilityTest, testGetCallingWindowInfo_003, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility testGetCallingWindowInfo_003 Test START");
    AccessScope accessScope(InputMethodAbilityTest::currentImeTokenId_, InputMethodAbilityTest::currentImeUid_);
    // bind IMC
    InputMethodAbilityTest::GetIMCAttachIMA();
    // only STATUS_BAR panel in IMA
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo info;
    info.panelType = STATUS_BAR;
    InputMethodAbilityTest::inputMethodAbility_.CreatePanel(nullptr, info, inputMethodPanel);
    CallingWindowInfo windowInfo;
    int32_t ret = InputMethodAbilityTest::inputMethodAbility_.GetCallingWindowInfo(windowInfo);
    EXPECT_EQ(ret, ErrorCode::ERROR_PANEL_NOT_FOUND);
    InputMethodAbilityTest::inputMethodAbility_.DestroyPanel(inputMethodPanel);
    InputMethodAbilityTest::GetIMCDetachIMA();
}

/**
 * @tc.name: testGetCallingWindowInfo_004
 * @tc.desc: GetCallingWindowInfo with invalid windowid
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: zhaolinglan
 */
HWTEST_F(InputMethodAbilityTest, testGetCallingWindowInfo_004, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility testGetCallingWindowInfo_004 Test START");
    AccessScope accessScope(InputMethodAbilityTest::currentImeTokenId_, InputMethodAbilityTest::currentImeUid_);
    // bind imc
    InputMethodAbilityTest::GetIMCAttachIMA();
    // SOFT_KEYBOARD panel exists
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo info;
    info.panelType = SOFT_KEYBOARD;
    info.panelFlag = FLG_FIXED;
    InputMethodAbilityTest::inputMethodAbility_.CreatePanel(nullptr, info, inputMethodPanel);
    // invalid window id
    InputMethodAbilityTest::imc_->clientInfo_.config.windowId = INVALID_WINDOW_ID;
    CallingWindowInfo windowInfo;
    int32_t ret = InputMethodAbilityTest::inputMethodAbility_.GetCallingWindowInfo(windowInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodAbilityTest::inputMethodAbility_.DestroyPanel(inputMethodPanel);
    InputMethodAbilityTest::GetIMCDetachIMA();
}

/**
 * @tc.name: testGetCallingWindowInfo_005
 * @tc.desc: GetCallingWindowInfo success
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: zhaolinglan
 */
HWTEST_F(InputMethodAbilityTest, testGetCallingWindowInfo_005, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility testGetCallingWindowInfo_005 Test START");
    AccessScope accessScope(InputMethodAbilityTest::currentImeTokenId_, InputMethodAbilityTest::currentImeUid_);
    // SOFT_KEYBOARD window is created
    InputMethodAbilityTest::inputMethodAbility_.panels_.Clear();
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo info;
    info.panelType = SOFT_KEYBOARD;
    info.panelFlag = FLG_FIXED;
    InputMethodAbilityTest::inputMethodAbility_.CreatePanel(nullptr, info, inputMethodPanel);
    // bind IMC
    InputMethodAbilityTest::GetIMCAttachIMA();
    InputMethodAbilityTest::imc_->textConfig_.windowId = TddUtil::WindowManager::currentWindowId_;
    // get window info success
    CallingWindowInfo windowInfo;
    int32_t ret = InputMethodAbilityTest::inputMethodAbility_.GetCallingWindowInfo(windowInfo);
    EXPECT_TRUE(ret == ErrorCode::NO_ERROR || ret == ErrorCode::ERROR_WINDOW_MANAGER);
    InputMethodAbilityTest::GetIMCDetachIMA();
    InputMethodAbilityTest::inputMethodAbility_.DestroyPanel(inputMethodPanel);
}

/**
 * @tc.name: testSetPreviewText_001
 * @tc.desc: IMA
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: zhaolinglan
 */
HWTEST_F(InputMethodAbilityTest, testSetPreviewText_001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbilityTest testSetPreviewText_001 Test START");
    TextListener::ResetParam();
    std::string text = "test";
    Range range = { 1, 2 };
    InputMethodAbilityTest::GetIMCAttachIMA();
    InputMethodAbilityTest::imc_->textConfig_.inputAttribute.isTextPreviewSupported = true;
    auto ret = InputMethodAbilityTest::inputMethodAbility_.SetPreviewText(text, range);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(TextListener::previewText_, text);
    EXPECT_EQ(TextListener::previewRange_, range);
    InputMethodAbilityTest::GetIMCDetachIMA();
}

/**
 * @tc.name: testSetPreviewText_002
 * @tc.desc: IMA
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: zhaolinglan
 */
HWTEST_F(InputMethodAbilityTest, testSetPreviewText_002, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbilityTest testSetPreviewText_002 Test START");
    TextListener::ResetParam();
    std::string text = "test";
    Range range = { 1, 2 };
    InputMethodAbilityTest::inputMethodAbility_.ClearDataChannel(
        InputMethodAbilityTest::inputMethodAbility_.dataChannelObject_);
    auto ret = InputMethodAbilityTest::inputMethodAbility_.SetPreviewText(text, range);
    EXPECT_EQ(ret, ErrorCode::ERROR_IMA_CHANNEL_NULLPTR);
    EXPECT_NE(TextListener::previewText_, text);
    EXPECT_FALSE(TextListener::previewRange_ == range);
}

/**
 * @tc.name: testSetPreviewText_003
 * @tc.desc: IMA
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: zhaolinglan
 */
HWTEST_F(InputMethodAbilityTest, testSetPreviewText_003, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbilityTest testSetPreviewText_003 Test START");
    TextListener::ResetParam();
    std::string text = "test";
    Range range = { 1, 2 };
    InputMethodAbilityTest::GetIMCAttachIMA();
    InputMethodAbilityTest::imc_->textConfig_.inputAttribute.isTextPreviewSupported = false;
    auto ret = InputMethodAbilityTest::inputMethodAbility_.SetPreviewText(text, range);
    EXPECT_EQ(ret, ErrorCode::ERROR_TEXT_PREVIEW_NOT_SUPPORTED);
    EXPECT_NE(TextListener::previewText_, text);
    EXPECT_FALSE(TextListener::previewRange_ == range);
    InputMethodAbilityTest::GetIMCDetachIMA();
}

/**
 * @tc.name: testFinishTextPreview_001
 * @tc.desc: IMA
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: zhaolinglan
 */
HWTEST_F(InputMethodAbilityTest, testFinishTextPreview_001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbilityTest testFinishTextPreview_001 Test START");
    TextListener::ResetParam();
    InputMethodAbilityTest::GetIMCAttachIMA();
    InputMethodAbilityTest::imc_->textConfig_.inputAttribute.isTextPreviewSupported = true;
    auto ret = InputMethodAbilityTest::inputMethodAbility_.FinishTextPreview(false);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(TextListener::isFinishTextPreviewCalled_);
    InputMethodAbilityTest::GetIMCDetachIMA();
}

/**
 * @tc.name: testFinishTextPreview_002
 * @tc.desc: IMA
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: zhaolinglan
 */
HWTEST_F(InputMethodAbilityTest, testFinishTextPreview_002, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbilityTest testFinishTextPreview_002 Test START");
    TextListener::ResetParam();
    InputMethodAbilityTest::inputMethodAbility_.ClearDataChannel(
        InputMethodAbilityTest::inputMethodAbility_.dataChannelObject_);
    auto ret = InputMethodAbilityTest::inputMethodAbility_.FinishTextPreview(false);
    EXPECT_EQ(ret, ErrorCode::ERROR_IMA_CHANNEL_NULLPTR);
    EXPECT_FALSE(TextListener::isFinishTextPreviewCalled_);
}

/**
 * @tc.name: testFinishTextPreview_003
 * @tc.desc: IMA
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: zhaolinglan
 */
HWTEST_F(InputMethodAbilityTest, testFinishTextPreview_003, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbilityTest testFinishTextPreview_003 Test START");
    TextListener::ResetParam();
    InputMethodAbilityTest::GetIMCAttachIMA();
    InputMethodAbilityTest::imc_->textConfig_.inputAttribute.isTextPreviewSupported = false;
    auto ret = InputMethodAbilityTest::inputMethodAbility_.FinishTextPreview(false);
    EXPECT_EQ(ret, ErrorCode::ERROR_TEXT_PREVIEW_NOT_SUPPORTED);
    EXPECT_FALSE(TextListener::isFinishTextPreviewCalled_);
    InputMethodAbilityTest::GetIMCDetachIMA();
}

/**
 *@tc.name: testGetInputMethodState_001
 *@tc.desc: IMA
 *@tc.type: FUNC
 *@tc.require:
 */
HWTEST_F(InputMethodAbilityTest, testGetInputMethodState_001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbilityTest GetInputMethodState_001 Test START");
    int32_t status = 0;
    auto ret = InputMethodAbilityTest::imsa_->GetInputMethodState(status);
    EXPECT_EQ(ret, ErrorCode::ERROR_NOT_IME);
}

/**
 * @tc.name: BranchCoverage001
 * @tc.desc: BranchCoverage
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodAbilityTest, BranchCoverage001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbilityTest BranchCoverage001 TEST START");
    auto ret = InputMethodAbilityTest::inputMethodAbility_.OnStopInputService(false);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    InputMethodAbilityTest::inputMethodAbility_.imeListener_ = nullptr;
    ret = InputMethodAbilityTest::inputMethodAbility_.OnStopInputService(false);
    EXPECT_EQ(ret, ErrorCode::ERROR_IME_NOT_STARTED);

    ret = InputMethodAbilityTest::inputMethodAbility_.OnSecurityChange(INVALID_VALUE);
    EXPECT_EQ(ret, ErrorCode::ERROR_BAD_PARAMETERS);

    ret = InputMethodAbilityTest::inputMethodAbility_.HideKeyboardImplWithoutLock(INVALID_VALUE, 0);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ret = InputMethodAbilityTest::inputMethodAbility_.ShowKeyboardImplWithoutLock(INVALID_VALUE);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ret = InputMethodAbilityTest::inputMethodAbility_.ShowPanel(nullptr);
    EXPECT_EQ(ret, ErrorCode::ERROR_BAD_PARAMETERS);

    PanelFlag flag = PanelFlag::FLG_FIXED;
    Trigger trigger = Trigger::IME_APP;
    ret = InputMethodAbilityTest::inputMethodAbility_.ShowPanel(nullptr, flag, trigger);
    EXPECT_EQ(ret, ErrorCode::ERROR_IMA_NULLPTR);

    ret = InputMethodAbilityTest::inputMethodAbility_.HidePanel(nullptr);
    EXPECT_EQ(ret, ErrorCode::ERROR_BAD_PARAMETERS);

    ret = InputMethodAbilityTest::inputMethodAbility_.HidePanel(nullptr, flag, trigger, 0);
    EXPECT_EQ(ret, ErrorCode::ERROR_BAD_PARAMETERS);

    InputMethodAbilityTest::inputMethodAbility_.isCurrentIme_ = true;
    auto ret2 = InputMethodAbilityTest::inputMethodAbility_.IsCurrentIme();
    EXPECT_TRUE(ret2);

    ret2 = InputMethodAbilityTest::inputMethodAbility_.IsEnable();
    EXPECT_FALSE(ret2);
}

/**
 * @tc.name: BranchCoverage002
 * @tc.desc: BranchCoverage
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodAbilityTest, BranchCoverage002, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbilityTest BranchCoverage002 TEST START");
    int32_t vailidUserId = 100001;
    SwitchInfo switchInfo;
    std::string vailidString = "";
    std::shared_ptr<ImeInfo> info;
    bool needHide = false;
    auto ret = imsa_->OnStartInputType(vailidUserId, switchInfo, true);
    EXPECT_NE(ret, ErrorCode::NO_ERROR);

    ret = imsa_->Switch(vailidUserId, vailidString, info);
    EXPECT_NE(ret, ErrorCode::NO_ERROR);
    ret = imsa_->SwitchExtension(vailidUserId, info);
    EXPECT_EQ(ret, ErrorCode::ERROR_NULL_POINTER);
    ret = imsa_->SwitchSubType(vailidUserId, info);
    imsa_->NeedHideWhenSwitchInputType(vailidUserId, needHide);
    EXPECT_EQ(ret, ErrorCode::ERROR_NULL_POINTER);
    ret = imsa_->SwitchInputType(vailidUserId, switchInfo);
    EXPECT_EQ(ret, ErrorCode::ERROR_IMSA_USER_SESSION_NOT_FOUND);
    ret = imsa_->OnPackageRemoved(vailidUserId, vailidString);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    auto ret2 = imsa_->IsNeedSwitch(vailidUserId, vailidString, vailidString);
    EXPECT_FALSE(ret2);
    ret2 = imsa_->IsStartInputTypePermitted(vailidUserId);
    EXPECT_FALSE(ret2);
}

/**
 * @tc.name: testOnCallingDisplayIdChanged
 * @tc.desc: Test InputMethodCoreProxy OnCallingDisplayIdChanged
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodAbilityTest, testOnCallingDisplayIdChanged, TestSize.Level0)
{
    IMSA_HILOGI("testOnCallingDisplayIdChanged start.");
    sptr<InputMethodCoreStub> coreStub = new InputMethodCoreServiceImpl();
    sptr<IInputMethodCore> core = coreStub;
    inputMethodAbility_.SetImeListener(std::make_shared<InputMethodEngineListenerImpl>());
    MessageParcel data;
    data.WriteRemoteObject(core->AsObject());
    sptr<IRemoteObject> coreObject = data.ReadRemoteObject();
    sptr<InputMethodCoreProxy> coreProxy = new InputMethodCoreProxy(coreObject);
    if (coreProxy == nullptr) {
        IMSA_HILOGI("coreProxy is null");
        return;
    }
    coreProxy->OnCallingDisplayIdChanged(0);
    EXPECT_TRUE(coreProxy != nullptr);
}

/**
 * @tc.name: testOnSendPrivateData_001
 * @tc.desc: IMA InputMethodCoreProxy OnSendPrivateData
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
*/
HWTEST_F(InputMethodAbilityTest, testOnSendPrivateData_001, TestSize.Level0)
{
    IMSA_HILOGI("testOnSendPrivateData_001 start.");
    std::unordered_map<std::string, PrivateDataValue> privateCommand;
    PrivateDataValue privateDataValue = std::string("stringValue");
    privateCommand.insert({ "value", privateDataValue });
    auto ret = InputMethodAbilityTest::inputMethodAbility_.OnSendPrivateData(privateCommand);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: testOnSendPrivateData_002
 * @tc.desc: IMA InputMethodCoreProxy OnSendPrivateData
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
*/
HWTEST_F(InputMethodAbilityTest, testOnSendPrivateData_002, TestSize.Level0)
{
    IMSA_HILOGI("testOnSendPrivateData_002 start.");
    std::unordered_map<std::string, PrivateDataValue> privateCommand;
    PrivateDataValue privateDataValue = std::string("stringValue");
    privateCommand.insert({ "value", privateDataValue });
    InputMethodAbilityTest::inputMethodAbility_.imeListener_ = nullptr;
    auto ret = InputMethodAbilityTest::inputMethodAbility_.OnSendPrivateData(privateCommand);
    EXPECT_NE(ret, ErrorCode::NO_ERROR);
}
} // namespace MiscServices
} // namespace OHOS
