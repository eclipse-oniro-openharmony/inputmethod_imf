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

#include "input_method_ability.h"

#include <gtest/gtest.h>
#include <string_ex.h>
#include <unistd.h>

#include <cstdint>
#include <functional>
#include <string>
#include <thread>
#include <vector>

#include "global.h"
#include "i_input_data_channel.h"
#include "input_attribute.h"
#include "input_control_channel_stub.h"
#include "input_data_channel_proxy.h"
#include "input_data_channel_stub.h"
#include "input_method_agent_stub.h"
#include "input_method_controller.h"
#include "input_method_core_proxy.h"
#include "input_method_core_stub.h"
#include "input_method_panel.h"
#include "message_handler.h"
#include "tdd_util.h"
#include "text_listener.h"

using namespace testing::ext;
namespace OHOS {
namespace MiscServices {
using WindowMgr = TddUtil::WindowManager;
constexpr uint32_t DEALY_TIME = 1;
class InputMethodAbilityTest : public testing::Test {
public:
    static std::string imeIdStopped_;
    static std::mutex imeListenerCallbackLock_;
    static std::condition_variable imeListenerCv_;
    static bool showKeyboard_;
    static constexpr int CURSOR_DIRECTION_BASE_VALUE = 2011;
    static sptr<InputMethodController> imc_;
    static sptr<InputMethodAbility> inputMethodAbility_;
    static uint32_t windowId_;

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

        void OnInputStop(const std::string &imeId)
        {
            imeIdStopped_ = imeId;
            IMSA_HILOGI("InputMethodEngineListenerImpl OnInputStop");
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
    };
    static void SetUpTestCase(void)
    {
        // Set the tokenID to the tokenID of the current ime
        TddUtil::StorageSelfTokenID();
        std::shared_ptr<Property> property = InputMethodController::GetInstance()->GetCurrentInputMethod();
        std::string bundleName = property != nullptr ? property->name : "default.inputmethod.unittest";
        TddUtil::SetTestTokenID(TddUtil::GetTestTokenID(bundleName));
        inputMethodAbility_ = InputMethodAbility::GetInstance();
        inputMethodAbility_->SetCoreAndAgent();
        TddUtil::RestoreSelfTokenID();
        TextListener::ResetParam();
        TddUtil::WindowManager::RegisterFocusChangeListener();
    }
    static void TearDownTestCase(void)
    {
        IMSA_HILOGI("InputMethodAbilityTest::TearDownTestCase");
        imc_->Close();
        TextListener::ResetParam();
        WindowMgr::HideWindow();
        WindowMgr::DestroyWindow();
    }
    void SetUp()
    {
        IMSA_HILOGI("InputMethodAbilityTest::SetUp");
    }
    void TearDown()
    {
        IMSA_HILOGI("InputMethodAbilityTest::TearDown");
    }
};

std::string InputMethodAbilityTest::imeIdStopped_;
std::mutex InputMethodAbilityTest::imeListenerCallbackLock_;
std::condition_variable InputMethodAbilityTest::imeListenerCv_;
bool InputMethodAbilityTest::showKeyboard_ = true;
sptr<InputMethodController> InputMethodAbilityTest::imc_;
sptr<InputMethodAbility> InputMethodAbilityTest::inputMethodAbility_;
uint32_t InputMethodAbilityTest::windowId_ = 0;

/**
* @tc.name: testSerializedInputAttribute
* @tc.desc: Checkout the serialization of InputAttribute.
* @tc.type: FUNC
*/
HWTEST_F(InputMethodAbilityTest, testSerializedInputAttribute, TestSize.Level0)
{
    InputAttribute inAttribute;
    inAttribute.inputPattern = InputAttribute::PATTERN_PASSWORD;
    MessageParcel data;
    EXPECT_TRUE(InputAttribute::Marshalling(inAttribute, data));
    InputAttribute outAttribute;
    EXPECT_TRUE(InputAttribute::Unmarshalling(outAttribute, data));
    EXPECT_TRUE(outAttribute.GetSecurityFlag());
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
    sptr<InputMethodCoreStub> coreStub = new InputMethodCoreStub();
    sptr<IInputMethodCore> core = coreStub;
    auto msgHandler = new (std::nothrow) MessageHandler();
    coreStub->SetMessageHandler(msgHandler);
    sptr<InputDataChannelStub> channelStub = new InputDataChannelStub();

    MessageParcel data;
    data.WriteRemoteObject(core->AsObject());
    data.WriteRemoteObject(channelStub->AsObject());
    sptr<IRemoteObject> coreObject = data.ReadRemoteObject();
    sptr<IRemoteObject> channelObject = data.ReadRemoteObject();

    sptr<InputMethodCoreProxy> coreProxy = new InputMethodCoreProxy(coreObject);
    sptr<InputDataChannelProxy> channelProxy = new InputDataChannelProxy(channelObject);
    auto ret = coreProxy->ShowKeyboard(channelProxy, false, false);
    EXPECT_EQ(ret, ErrorCode::ERROR_IME);
    delete msgHandler;
}

/**
* @tc.name: testShowKeyboardException
* @tc.desc: InputMethodAbility ShowKeyboard without imeListener
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(InputMethodAbilityTest, testShowKeyboardException, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbilityTest testShowKeyboardException start.");
    sptr<InputDataChannelStub> channelStub = new InputDataChannelStub();
    auto ret = inputMethodAbility_->ShowKeyboard(channelStub->AsObject(), false, false);
    EXPECT_EQ(ret, ErrorCode::ERROR_IME);
}

/**
* @tc.name: testHideKeyboard
* @tc.desc: InputMethodAbility HideKeyboard
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(InputMethodAbilityTest, testHideKeyboard, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbilityTest testHideKeyboard start.");
    auto ret = inputMethodAbility_->HideKeyboard();
    EXPECT_EQ(ret, ErrorCode::ERROR_IME);
}

/**
* @tc.name: testHideKeyboardSelfWithoutImeListener
* @tc.desc: InputMethodAbility HideKeyboardSelf Without ImeListener
* @tc.type: FUNC
* @tc.require:
* @tc.author: Hollokin
*/
HWTEST_F(InputMethodAbilityTest, testHideKeyboardSelfWithoutImeListener, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility testHideKeyboardSelfWithoutImeListener START");
    std::unique_lock<std::mutex> lock(InputMethodAbilityTest::imeListenerCallbackLock_);
    InputMethodAbilityTest::showKeyboard_ = true;
    auto ret = inputMethodAbility_->HideKeyboardSelf();
    auto cvStatus = imeListenerCv_.wait_for(lock, std::chrono::seconds(DEALY_TIME));
    EXPECT_EQ(cvStatus, std::cv_status::timeout);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(InputMethodAbilityTest::showKeyboard_);
}

/**
* @tc.name: testShowKeyboard
* @tc.desc: InputMethodAbility ShowKeyboard
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(InputMethodAbilityTest, testShowKeyboard, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbilityTest testShowKeyboard start.");
    inputMethodAbility_->SetImeListener(std::make_shared<InputMethodEngineListenerImpl>());
    sptr<InputDataChannelStub> channelStub = new InputDataChannelStub();
    auto ret = inputMethodAbility_->ShowKeyboard(channelStub->AsObject(), false, false);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = inputMethodAbility_->HideKeyboard();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
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
    WindowMgr::CreateWindow();
    WindowMgr::ShowWindow();
    bool isFocused = FocusChangedListenerTestImpl::isFocused_->GetValue();
    IMSA_HILOGI("testHideKeyboardSelf getFocus end, isFocused = %{public}d", isFocused);
    sptr<OnTextChangedListener> textListener = new TextListener();
    imc_ = InputMethodController::GetInstance();
    imc_->Attach(textListener);
    std::unique_lock<std::mutex> lock(InputMethodAbilityTest::imeListenerCallbackLock_);
    InputMethodAbilityTest::showKeyboard_ = true;
    inputMethodAbility_->SetImeListener(std::make_shared<InputMethodEngineListenerImpl>());
    auto ret = inputMethodAbility_->HideKeyboardSelf();
    InputMethodAbilityTest::imeListenerCv_.wait_for(
        lock, std::chrono::seconds(DEALY_TIME), [] { return InputMethodAbilityTest::showKeyboard_ == false; });
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
    auto ret = inputMethodAbility_->MoveCursor(keyCode); // move cursor right
    std::unique_lock<std::mutex> lock(TextListener::textListenerCallbackLock_);
    TextListener::textListenerCv_.wait_for(
        lock, std::chrono::seconds(DEALY_TIME), [] { return TextListener::direction_ == keyCode; });
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(TextListener::direction_, keyCode);
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
    auto ret = inputMethodAbility_->InsertText(text);
    std::unique_lock<std::mutex> lock(TextListener::textListenerCallbackLock_);
    TextListener::textListenerCv_.wait_for(
        lock, std::chrono::seconds(DEALY_TIME), [u16Text] { return TextListener::insertText_ == u16Text; });
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(TextListener::insertText_, u16Text);
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
    auto ret = inputMethodAbility_->SendFunctionKey(funcKey);
    std::unique_lock<std::mutex> lock(TextListener::textListenerCallbackLock_);
    TextListener::textListenerCv_.wait_for(
        lock, std::chrono::seconds(DEALY_TIME), [] { return TextListener::key_ == funcKey; });
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(TextListener::key_, funcKey);
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
    auto ret = inputMethodAbility_->SendExtendAction(action);
    std::unique_lock<std::mutex> lock(TextListener::textListenerCallbackLock_);
    TextListener::textListenerCv_.wait_for(
        lock, std::chrono::seconds(DEALY_TIME), [] { return TextListener::action_ == action; });
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(TextListener::action_, action);
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
    auto ret = inputMethodAbility_->DeleteForward(deleteForwardLenth);
    std::unique_lock<std::mutex> lock(TextListener::textListenerCallbackLock_);
    TextListener::textListenerCv_.wait_for(lock, std::chrono::seconds(DEALY_TIME),
        [deleteForwardLenth] { return TextListener::deleteBackwardLength_ == deleteForwardLenth; });
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(TextListener::deleteBackwardLength_, deleteForwardLenth);

    int32_t deleteBackwardLenth = 2;
    ret = inputMethodAbility_->DeleteBackward(deleteBackwardLenth);
    TextListener::textListenerCv_.wait_for(lock, std::chrono::seconds(DEALY_TIME),
        [deleteBackwardLenth] { return TextListener::deleteForwardLength_ == deleteBackwardLenth; });
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(TextListener::deleteForwardLength_, deleteBackwardLenth);
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
    auto ret = inputMethodAbility_->GetEnterKeyType(keyType2);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(keyType2, (int)keyType);
    int32_t inputPattern;
    ret = inputMethodAbility_->GetInputPattern(inputPattern);
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
    sptr<OnTextChangedListener> textListener = new TextListener();
    TextConfig textConfig;
    textConfig.inputAttribute = { .inputPattern = 0, .enterKeyType = 1 };
    auto ret = imc_->Attach(textListener, false, textConfig);
    TextTotalConfig textTotalConfig;
    ret = inputMethodAbility_->GetTextConfig(textTotalConfig);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(textTotalConfig.inputAttribute.inputPattern, textConfig.inputAttribute.inputPattern);
    EXPECT_EQ(textTotalConfig.inputAttribute.enterKeyType, textConfig.inputAttribute.enterKeyType);
    textListener = nullptr;
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
    auto ret = inputMethodAbility_->SelectByRange(start, end);
    std::unique_lock<std::mutex> lock(TextListener::textListenerCallbackLock_);
    TextListener::textListenerCv_.wait_for(lock, std::chrono::seconds(DEALY_TIME),
        [] { return TextListener::selectionStart_ == start && TextListener::selectionEnd_ == end; });
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(TextListener::selectionStart_, start);
    EXPECT_EQ(TextListener::selectionEnd_, end);
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
    auto ret = inputMethodAbility_->SelectByRange(start, end);
    EXPECT_EQ(ret, ErrorCode::ERROR_BAD_PARAMETERS);

    start = 2;
    end = -2;
    ret = inputMethodAbility_->SelectByRange(start, end);
    EXPECT_EQ(ret, ErrorCode::ERROR_BAD_PARAMETERS);
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
    auto ret = inputMethodAbility_->SelectByMovement(direction);
    std::unique_lock<std::mutex> lock(TextListener::textListenerCallbackLock_);
    TextListener::textListenerCv_.wait_for(lock, std::chrono::seconds(DEALY_TIME), [] {
        return TextListener::selectionDirection_ == direction + InputMethodAbilityTest::CURSOR_DIRECTION_BASE_VALUE;
    });
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(TextListener::selectionDirection_, direction + InputMethodAbilityTest::CURSOR_DIRECTION_BASE_VALUE);
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
    auto ret = inputMethodAbility_->GetTextAfterCursor(number, text);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(text, Str8ToStr16(TextListener::TEXT_AFTER_CURSOR));
}

/**
* @tc.name: testGetTextAfterCursor_timeout
* @tc.desc:
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(InputMethodAbilityTest, testGetTextAfterCursor_timeout, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility testGetTextAfterCursor_timeout START");
    TextListener::setTimeout(true);
    int32_t number = 3;
    std::u16string text;
    auto ret = inputMethodAbility_->GetTextAfterCursor(number, text);
    EXPECT_EQ(ret, ErrorCode::ERROR_CONTROLLER_INVOKING_FAILED);
    TextListener::setTimeout(false);
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
    auto ret = inputMethodAbility_->GetTextBeforeCursor(number, text);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(text, Str8ToStr16(TextListener::TEXT_BEFORE_CURSOR));
}

/**
* @tc.name: testGetTextBeforeCursor_timeout
* @tc.desc:
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(InputMethodAbilityTest, testGetTextBeforeCursor_timeout, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility testGetTextBeforeCursor_timeout START");
    TextListener::setTimeout(true);
    int32_t number = 5;
    std::u16string text;
    auto ret = inputMethodAbility_->GetTextBeforeCursor(number, text);
    EXPECT_EQ(ret, ErrorCode::ERROR_CONTROLLER_INVOKING_FAILED);
    TextListener::setTimeout(false);
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
    auto ret = inputMethodAbility_->GetTextIndexAtCursor(index);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(index, TextListener::TEXT_INDEX);
}

/**
* @tc.name: testGetTextIndexAtCursor_timeout
* @tc.desc:
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(InputMethodAbilityTest, testGetTextIndexAtCursor_timeout, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility testGetTextIndexAtCursor_timeout START");
    TextListener::setTimeout(true);
    int32_t index;
    auto ret = inputMethodAbility_->GetTextIndexAtCursor(index);
    EXPECT_EQ(ret, ErrorCode::ERROR_CONTROLLER_INVOKING_FAILED);
    TextListener::setTimeout(false);
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
    TddUtil::SetTestTokenID(TddUtil::AllocTestTokenID(true, false, "undefine"));
    std::shared_ptr<InputMethodPanel> softKeyboardPanel1 = nullptr;
    PanelInfo panelInfo = { .panelType = SOFT_KEYBOARD, .panelFlag = FLG_FIXED };
    auto ret = inputMethodAbility_->CreatePanel(nullptr, panelInfo, softKeyboardPanel1);
    EXPECT_TRUE(softKeyboardPanel1 != nullptr);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    std::shared_ptr<InputMethodPanel> softKeyboardPanel2 = nullptr;
    ret = inputMethodAbility_->CreatePanel(nullptr, panelInfo, softKeyboardPanel2);
    EXPECT_TRUE(softKeyboardPanel2 == nullptr);
    EXPECT_EQ(ret, ErrorCode::ERROR_OPERATE_PANEL);

    ret = inputMethodAbility_->DestroyPanel(softKeyboardPanel1);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ret = inputMethodAbility_->DestroyPanel(softKeyboardPanel2);
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
    std::shared_ptr<InputMethodPanel> statusBar1 = nullptr;
    PanelInfo panelInfo = { .panelType = STATUS_BAR };
    auto ret = inputMethodAbility_->CreatePanel(nullptr, panelInfo, statusBar1);
    EXPECT_TRUE(statusBar1 != nullptr);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    std::shared_ptr<InputMethodPanel> statusBar2 = nullptr;
    ret = inputMethodAbility_->CreatePanel(nullptr, panelInfo, statusBar2);
    EXPECT_TRUE(statusBar2 == nullptr);
    EXPECT_EQ(ret, ErrorCode::ERROR_OPERATE_PANEL);

    ret = inputMethodAbility_->DestroyPanel(statusBar1);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ret = inputMethodAbility_->DestroyPanel(statusBar2);
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
    std::shared_ptr<InputMethodPanel> softKeyboardPanel = nullptr;
    PanelInfo panelInfo1 = { .panelType = SOFT_KEYBOARD, .panelFlag = FLG_FIXED };
    auto ret = inputMethodAbility_->CreatePanel(nullptr, panelInfo1, softKeyboardPanel);
    EXPECT_TRUE(softKeyboardPanel != nullptr);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    PanelInfo panelInfo2 = { .panelType = STATUS_BAR };
    std::shared_ptr<InputMethodPanel> statusBar = nullptr;
    ret = inputMethodAbility_->CreatePanel(nullptr, panelInfo2, statusBar);
    EXPECT_TRUE(statusBar != nullptr);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ret = inputMethodAbility_->DestroyPanel(softKeyboardPanel);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ret = inputMethodAbility_->DestroyPanel(statusBar);
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
    std::shared_ptr<InputMethodPanel> inputMethodPanel = nullptr;
    PanelInfo panelInfo1 = { .panelType = SOFT_KEYBOARD, .panelFlag = FLG_FIXED };
    auto ret = inputMethodAbility_->CreatePanel(nullptr, panelInfo1, inputMethodPanel);
    EXPECT_TRUE(inputMethodPanel != nullptr);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ret = inputMethodAbility_->DestroyPanel(inputMethodPanel);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    PanelInfo panelInfo2 = { .panelType = STATUS_BAR };
    ret = inputMethodAbility_->CreatePanel(nullptr, panelInfo2, inputMethodPanel);
    EXPECT_TRUE(inputMethodPanel != nullptr);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ret = inputMethodAbility_->DestroyPanel(inputMethodPanel);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    panelInfo1.panelFlag = FLG_FLOATING;
    ret = inputMethodAbility_->CreatePanel(nullptr, panelInfo1, inputMethodPanel);
    EXPECT_TRUE(inputMethodPanel != nullptr);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ret = inputMethodAbility_->DestroyPanel(inputMethodPanel);
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
    TddUtil::SetTestTokenID(TddUtil::AllocTestTokenID(true, false, "undefine"));
    std::shared_ptr<InputMethodPanel> softKeyboardPanel1 = nullptr;
    PanelInfo panelInfo = { .panelType = SOFT_KEYBOARD, .panelFlag = FLG_FIXED };
    auto ret = inputMethodAbility_->CreatePanel(nullptr, panelInfo, softKeyboardPanel1);
    EXPECT_TRUE(softKeyboardPanel1 != nullptr);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ret = inputMethodAbility_->DestroyPanel(softKeyboardPanel1);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    std::shared_ptr<InputMethodPanel> softKeyboardPanel2 = nullptr;
    ret = inputMethodAbility_->CreatePanel(nullptr, panelInfo, softKeyboardPanel2);
    EXPECT_TRUE(softKeyboardPanel2 != nullptr);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ret = inputMethodAbility_->DestroyPanel(softKeyboardPanel2);
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
    TddUtil::SetTestTokenID(TddUtil::AllocTestTokenID(true, false, "undefine"));
    std::shared_ptr<InputMethodPanel> softKeyboardPanel1 = nullptr;
    PanelInfo panelInfo = { .panelType = SOFT_KEYBOARD, .panelFlag = FLG_FIXED };
    auto ret = inputMethodAbility_->CreatePanel(nullptr, panelInfo, softKeyboardPanel1);
    EXPECT_TRUE(softKeyboardPanel1 != nullptr);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    std::shared_ptr<InputMethodPanel> softKeyboardPanel2 = nullptr;
    ret = inputMethodAbility_->CreatePanel(nullptr, panelInfo, softKeyboardPanel2);
    EXPECT_TRUE(softKeyboardPanel2 == nullptr);
    EXPECT_EQ(ret, ErrorCode::ERROR_OPERATE_PANEL);

    ret = inputMethodAbility_->DestroyPanel(softKeyboardPanel1);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    std::shared_ptr<InputMethodPanel> softKeyboardPanel3 = nullptr;
    ret = inputMethodAbility_->CreatePanel(nullptr, panelInfo, softKeyboardPanel3);
    EXPECT_TRUE(softKeyboardPanel3 != nullptr);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ret = inputMethodAbility_->DestroyPanel(softKeyboardPanel2);
    EXPECT_EQ(ret, ErrorCode::ERROR_BAD_PARAMETERS);

    ret = inputMethodAbility_->DestroyPanel(softKeyboardPanel3);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
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
    inputMethodAbility_->SetImeListener(std::make_shared<InputMethodEngineListenerImpl>());
    uint32_t windowId = 10;
    inputMethodAbility_->SetCallingWindow(windowId);
    InputMethodAbilityTest::imeListenerCv_.wait_for(lock, std::chrono::seconds(DEALY_TIME), [windowId] {
        return InputMethodAbilityTest::windowId_ == windowId;
    });
    EXPECT_EQ(InputMethodAbilityTest::windowId_, windowId);
}
} // namespace MiscServices
} // namespace OHOS
