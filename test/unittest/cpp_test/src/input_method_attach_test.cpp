/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#include <cstdint>
#include <functional>
#include <gtest/gtest.h>
#include <string>
#include <string_ex.h>
#include <thread>
#include <unistd.h>
#include <vector>

#include "ability_manager_client.h"
#include "accesstoken_kit.h"
#include "global.h"
#include "i_input_data_channel.h"
#include "input_attribute.h"
#include "input_control_channel_stub.h"
#include "input_data_channel_proxy.h"
#include "input_data_channel_stub.h"
#include "input_method_ability.h"
#include "input_method_agent_stub.h"
#include "input_method_controller.h"
#include "input_method_core_proxy.h"
#include "input_method_core_stub.h"
#include "input_method_panel.h"
#include "message_handler.h"
#include "os_account_manager.h"
#include "token_setproc.h"

using namespace testing::ext;
using namespace OHOS::Security::AccessToken;
using namespace OHOS::AccountSA;
namespace OHOS {
namespace MiscServices {
constexpr uint32_t DEALY_TIME = 1;
std::u16string g_textTemp = u"我們我們ddddd";
constexpr int32_t MAIN_USER_ID = 100;
class InputMethodAttachTest : public testing::Test {
public:
    static std::string imeIdStopped_;
    static std::mutex imeListenerCallbackLock_;
    static std::condition_variable imeListenerCv_;
    static bool showKeyboard_;
    static std::mutex textListenerCallbackLock_;
    static std::condition_variable textListenerCv_;
    static int direction_;
    static int deleteForwardLength_;
    static int deleteBackwardLength_;
    static std::u16string insertText_;
    static int key_;
    static int keyboardStatus_;
    static bool status_;
    static int selectionStart_;
    static int selectionEnd_;
    static int selectionDirection_;
    static int32_t action_;
    static constexpr int CURSOR_DIRECTION_BASE_VALUE = 2011;
    static sptr<InputMethodController> inputMethodController_;
    static sptr<InputMethodAbility> inputMethodAbility_;
    static uint64_t selfTokenID_;
    static AccessTokenID testTokenID_;

    class EngineListenerImpl : public InputMethodEngineListener {
    public:
        EngineListenerImpl() = default;
        ~EngineListenerImpl() = default;

        void OnKeyboardStatus(bool isShow)
        {
            showKeyboard_ = isShow;
            InputMethodAttachTest::imeListenerCv_.notify_one();
            IMSA_HILOGI("EngineListenerImpl OnKeyboardStatus");
        }

        void OnInputStart()
        {
            IMSA_HILOGI("EngineListenerImpl OnInputStart");
        }

        void OnInputStop(const std::string &imeId)
        {
            imeIdStopped_ = imeId;
            IMSA_HILOGI("EngineListenerImpl OnInputStop");
        }

        void OnSetCallingWindow(uint32_t windowId)
        {
            IMSA_HILOGI("EngineListenerImpl OnSetCallingWindow");
        }

        void OnSetSubtype(const SubProperty &property)
        {
            IMSA_HILOGI("EngineListenerImpl OnSetSubtype");
        }
    };
    class TextChangeListenerImpl : public OnTextChangedListener {
    public:
        void InsertText(const std::u16string &text) override
        {
            insertText_ = text;
            InputMethodAttachTest::textListenerCv_.notify_one();
        }

        void DeleteForward(int32_t length) override
        {
            deleteForwardLength_ = length;
            InputMethodAttachTest::textListenerCv_.notify_one();
            IMSA_HILOGI("TextChangeListenerImpl: DeleteForward, length is: %{public}d", length);
        }

        void DeleteBackward(int32_t length) override
        {
            deleteBackwardLength_ = length;
            InputMethodAttachTest::textListenerCv_.notify_one();
            IMSA_HILOGI("TextChangeListenerImpl: DeleteBackward, direction is: %{public}d", length);
        }

        void SendKeyEventFromInputMethod(const KeyEvent &event) override {}

        void SendKeyboardStatus(const KeyboardStatus &keyboardStatus) override {}

        void SendFunctionKey(const FunctionKey &functionKey) override
        {
            EnterKeyType enterKeyType = functionKey.GetEnterKeyType();
            key_ = static_cast<int>(enterKeyType);
            InputMethodAttachTest::textListenerCv_.notify_one();
        }

        void SetKeyboardStatus(bool status) override
        {
            status_ = status;
        }

        void MoveCursor(const Direction direction) override
        {
            direction_ = (int)direction;
            InputMethodAttachTest::textListenerCv_.notify_one();
            IMSA_HILOGI("TextChangeListenerImpl: MoveCursor, direction is: %{public}d", direction);
        }

        void HandleSetSelection(int32_t start, int32_t end) override
        {
            selectionStart_ = start;
            selectionEnd_ = end;
            InputMethodAttachTest::textListenerCv_.notify_one();
            IMSA_HILOGI("TextChangeListenerImpl, selectionStart_: %{public}d, selectionEnd_: %{public}d",
                selectionStart_, selectionEnd_);
        }

        void HandleExtendAction(int32_t action) override
        {
            action_ = action;
            InputMethodAttachTest::textListenerCv_.notify_one();
            IMSA_HILOGI("HandleExtendAction, action_: %{public}d", action_);
        }

        void HandleSelect(int32_t keyCode, int32_t cursorMoveSkip) override
        {
            selectionDirection_ = keyCode;
            InputMethodAttachTest::textListenerCv_.notify_one();
            IMSA_HILOGI("TextChangeListenerImpl, selectionDirection_: %{public}d", selectionDirection_);
        }
    };
    static void AllocTestTokenID(const std::string &bundleName)
    {
        IMSA_HILOGI("bundleName: %{public}s", bundleName.c_str());
        std::vector<int32_t> userIds;
        auto ret = OsAccountManager::QueryActiveOsAccountIds(userIds);
        if (ret != ErrorCode::NO_ERROR || userIds.empty()) {
            IMSA_HILOGE("query active os account id failed");
            userIds[0] = MAIN_USER_ID;
        }
        HapInfoParams infoParams = { .userID = userIds[0],
            .bundleName = bundleName,
            .instIndex = 0,
            .appIDDesc = "ohos.inputmethod_test.demo" };
        PermissionStateFull permissionState = { .permissionName = "ohos.permission.CONNECT_IME_ABILITY",
            .isGeneral = true,
            .resDeviceID = { "local" },
            .grantStatus = { PermissionState::PERMISSION_GRANTED },
            .grantFlags = { 1 } };
        HapPolicyParams policyParams = { .apl = APL_NORMAL,
            .domain = "test.domain.inputmethod",
            .permList = {},
            .permStateList = { permissionState } };

        AccessTokenKit::AllocHapToken(infoParams, policyParams);
        testTokenID_ = AccessTokenKit::GetHapTokenID(infoParams.userID, infoParams.bundleName, infoParams.instIndex);
    }
    static void DeleteTestTokenID()
    {
        AccessTokenKit::DeleteToken(testTokenID_);
    }
    static void SetTestTokenID()
    {
        auto ret = SetSelfTokenID(testTokenID_);
        IMSA_HILOGI("SetSelfTokenID ret: %{public}d", ret);
    }
    static void RestoreSelfTokenID()
    {
        auto ret = SetSelfTokenID(selfTokenID_);
        IMSA_HILOGI("SetSelfTokenID ret = %{public}d", ret);
    }
    static void SetUpTestCase(void)
    {
        IMSA_HILOGI("InputMethodAttachTest::SetUpTestCase");
        inputMethodController_ = InputMethodController::GetInstance();
        inputMethodAbility_ = InputMethodAbility::GetInstance();
        inputMethodAbility_->OnImeReady();
        inputMethodAbility_->SetCoreAndAgent();
        inputMethodAbility_->SetImeListener(std::make_shared<EngineListenerImpl>());
    }
    static void TearDownTestCase(void)
    {
        IMSA_HILOGI("InputMethodAttachTest::TearDownTestCase");
    }
    void SetUp()
    {
        IMSA_HILOGI("InputMethodAttachTest::SetUp");
    }
    void TearDown()
    {
        IMSA_HILOGI("InputMethodAttachTest::TearDown");
    }
};

std::string InputMethodAttachTest::imeIdStopped_;
std::mutex InputMethodAttachTest::imeListenerCallbackLock_;
std::condition_variable InputMethodAttachTest::imeListenerCv_;
bool InputMethodAttachTest::showKeyboard_ = false;
std::mutex InputMethodAttachTest::textListenerCallbackLock_;
std::condition_variable InputMethodAttachTest::textListenerCv_;
int InputMethodAttachTest::direction_;
int InputMethodAttachTest::deleteForwardLength_ = 0;
int InputMethodAttachTest::deleteBackwardLength_ = 0;
std::u16string InputMethodAttachTest::insertText_;
int InputMethodAttachTest::key_ = 0;
int InputMethodAttachTest::keyboardStatus_;
bool InputMethodAttachTest::status_;
int InputMethodAttachTest::selectionStart_ = -1;
int InputMethodAttachTest::selectionEnd_ = -1;
int InputMethodAttachTest::selectionDirection_ = 0;
int32_t InputMethodAttachTest::action_ = 0;
sptr<InputMethodController> InputMethodAttachTest::inputMethodController_;
sptr<InputMethodAbility> InputMethodAttachTest::inputMethodAbility_;
uint64_t InputMethodAttachTest::selfTokenID_ = 0;
AccessTokenID InputMethodAttachTest::testTokenID_ = 0;

/**
 * @tc.name: testAttach001
 * @tc.desc: test Attach
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodAttachTest, testAttach001, TestSize.Level0)
{
    IMSA_HILOGI("test testAttach001 after attach.");
    sptr<OnTextChangedListener> TextChangeListenerImpl = new TextChangeListenerImpl();
    auto ret = inputMethodController_->Attach(TextChangeListenerImpl);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    int32_t keyType = -1;
    ret = inputMethodAbility_->GetEnterKeyType(keyType);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(keyType, 0);
    int32_t inputPattern = -1;
    ret = inputMethodAbility_->GetInputPattern(inputPattern);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(inputPattern, InputAttribute::PATTERN_TEXT);
    EXPECT_EQ(InputMethodAttachTest::showKeyboard_, true);

    ret = inputMethodController_->Close();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: testAttach002
 * @tc.desc: test Attach
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodAttachTest, testAttach002, TestSize.Level0)
{
    IMSA_HILOGI("test testAttach002 after attach.");
    sptr<OnTextChangedListener> TextChangeListenerImpl = new TextChangeListenerImpl();
    auto ret = inputMethodController_->Attach(TextChangeListenerImpl, false);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    int32_t keyType = -1;
    ret = inputMethodAbility_->GetEnterKeyType(keyType);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(keyType, 0);
    int32_t inputPattern = -1;
    ret = inputMethodAbility_->GetInputPattern(inputPattern);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(inputPattern, InputAttribute::PATTERN_TEXT);

    EXPECT_EQ(InputMethodAttachTest::showKeyboard_, false);
    ret = inputMethodController_->Close();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: testAttach003
 * @tc.desc: test Attach
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodAttachTest, testAttach003, TestSize.Level0)
{
    IMSA_HILOGI("test testAttach003 after attach.");
    sptr<OnTextChangedListener> TextChangeListenerImpl = new TextChangeListenerImpl();
    InputAttribute attribute;
    attribute.inputPattern = 2;
    attribute.enterKeyType = 1;
    auto ret = inputMethodController_->Attach(TextChangeListenerImpl, true, attribute);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    int32_t keyType = -1;
    ret = inputMethodAbility_->GetEnterKeyType(keyType);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(keyType, attribute.enterKeyType);
    int32_t inputPattern = -1;
    ret = inputMethodAbility_->GetInputPattern(inputPattern);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(inputPattern, attribute.inputPattern);

    EXPECT_EQ(InputMethodAttachTest::showKeyboard_, true);
    ret = inputMethodController_->Close();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: testAttach004
 * @tc.desc: test Attach
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodAttachTest, testAttach004, TestSize.Level0)
{
    IMSA_HILOGI("test testAttach004 after attach.");
    sptr<OnTextChangedListener> TextChangeListenerImpl = new TextChangeListenerImpl();
    InputAttribute attribute;
    attribute.inputPattern = 3;
    attribute.enterKeyType = 2;
    TextConfig config;
    config.inputAttribute = attribute;
    auto ret = inputMethodController_->Attach(TextChangeListenerImpl, false, config);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    int32_t keyType = -1;
    ret = inputMethodAbility_->GetEnterKeyType(keyType);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(keyType, config.inputAttribute.enterKeyType);
    int32_t inputPattern = -1;
    ret = inputMethodAbility_->GetInputPattern(inputPattern);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(inputPattern, config.inputAttribute.inputPattern);

    EXPECT_EQ(InputMethodAttachTest::showKeyboard_, false);
    ret = inputMethodController_->Close();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: testAttach005
 * @tc.desc: test Attach, test optional param in TextConfig
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodAttachTest, testAttach005, TestSize.Level0)
{
    IMSA_HILOGI("test testAttach005 after attach.");
    sptr<OnTextChangedListener> TextChangeListenerImpl = new TextChangeListenerImpl();
    InputAttribute attribute;
    attribute.inputPattern = 3;
    attribute.enterKeyType = 2;
    TextConfig config;
    config.inputAttribute = attribute;
    CursorInfo cursorInfo;
    cursorInfo.left = 0;
    cursorInfo.top = 1;
    cursorInfo.width = 0.5;
    cursorInfo.height = 1.2;
    config.cursorInfo = cursorInfo;
    SelectionRange selectionRange;
    selectionRange.start = 0;
    selectionRange.end = 2;
    config.range = selectionRange;
    config.windowId = 10;
    auto ret = inputMethodController_->Attach(TextChangeListenerImpl, true, config);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    int32_t keyType = -1;
    ret = inputMethodAbility_->GetEnterKeyType(keyType);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(keyType, config.inputAttribute.enterKeyType);
    int32_t inputPattern = -1;
    ret = inputMethodAbility_->GetInputPattern(inputPattern);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(inputPattern, config.inputAttribute.inputPattern);

    EXPECT_EQ(InputMethodAttachTest::showKeyboard_, true);
    TextTotalConfig textConfig;
    ret = inputMethodAbility_->GetTextConfig(textConfig);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(textConfig.inputAttribute, config.inputAttribute);
    EXPECT_EQ(textConfig.windowId, config.windowId);
    EXPECT_EQ(textConfig.cursorInfo, config.cursorInfo);
    EXPECT_EQ(textConfig.textSelection.newBegin, config.range.start);
    EXPECT_EQ(textConfig.textSelection.newEnd, config.range.end);
    EXPECT_EQ(textConfig.textSelection.oldBegin, 0);
    EXPECT_EQ(textConfig.textSelection.oldEnd, 0);
    ret = inputMethodController_->Close();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: testOnConfigurationChangeWithOutAttach
 * @tc.desc: test OnConfigurationChange without attach
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodAttachTest, testOnConfigurationChangeWithOutAttach, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAttachTest testOnConfigurationChangeWithOutAttach in.");
    Configuration config;
    EnterKeyType keyType = EnterKeyType::NEXT;
    config.SetEnterKeyType(keyType);
    TextInputType textInputType = TextInputType::DATETIME;
    config.SetTextInputType(textInputType);
    auto ret = inputMethodController_->OnConfigurationChange(config);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_BOUND);
}

/**
 * @tc.name: testOnConfigurationChange
 * @tc.desc: test OnConfigurationChange after attach
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodAttachTest, testOnConfigurationChange, TestSize.Level0)
{
    IMSA_HILOGI("test OnConfigurationChange after attach.");
    sptr<OnTextChangedListener> TextChangeListenerImpl = new TextChangeListenerImpl();
    auto ret = inputMethodController_->Attach(TextChangeListenerImpl);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    Configuration config;
    EnterKeyType keyType = EnterKeyType::NEXT;
    config.SetEnterKeyType(keyType);
    TextInputType textInputType = TextInputType::DATETIME;
    config.SetTextInputType(textInputType);
    ret = inputMethodController_->OnConfigurationChange(config);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    int32_t keyType2;
    ret = inputMethodAbility_->GetEnterKeyType(keyType2);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(keyType2, (int)keyType);
    int32_t inputPattern;
    ret = inputMethodAbility_->GetInputPattern(inputPattern);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(inputPattern, (int)textInputType);

    ret = inputMethodController_->Close();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: testGetTextConfig
 * @tc.desc: test GetTextConfig of InputMethodAbility
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodAttachTest, testGetTextConfig, TestSize.Level0)
{
    IMSA_HILOGI("test OnConfigurationChange001 after attach.");
    sptr<OnTextChangedListener> TextChangeListenerImpl = new TextChangeListenerImpl();
    InputAttribute attribute;
    attribute.inputPattern = 3;
    attribute.enterKeyType = 2;
    TextConfig config;
    config.inputAttribute = attribute;
    CursorInfo cursorInfo;
    cursorInfo.left = 0;
    cursorInfo.top = 1;
    cursorInfo.width = 0.5;
    cursorInfo.height = 1.2;
    config.cursorInfo = cursorInfo;
    SelectionRange selectionRange;
    selectionRange.start = 0;
    selectionRange.end = 2;
    config.range = selectionRange;
    config.windowId = 10;
    auto ret = inputMethodController_->Attach(TextChangeListenerImpl, false, config);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    TextTotalConfig totalConfig;
    ret = inputMethodAbility_->GetTextConfig(totalConfig);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    EXPECT_EQ(totalConfig.inputAttribute.inputPattern, attribute.inputPattern);
    EXPECT_EQ(totalConfig.inputAttribute.enterKeyType, attribute.enterKeyType);
    EXPECT_EQ(totalConfig.cursorInfo.height, cursorInfo.height);
    EXPECT_EQ(totalConfig.cursorInfo.width, cursorInfo.width);
    EXPECT_EQ(totalConfig.cursorInfo.left, cursorInfo.left);
    EXPECT_EQ(totalConfig.cursorInfo.top, cursorInfo.top);
    EXPECT_EQ(totalConfig.textSelection.newBegin, selectionRange.start);
    EXPECT_EQ(totalConfig.textSelection.newEnd, selectionRange.end);
    EXPECT_EQ(totalConfig.textSelection.oldBegin, 0);
    EXPECT_EQ(totalConfig.textSelection.oldEnd, 0);
    EXPECT_EQ(totalConfig.windowId, config.windowId);
}

/**
 * @tc.name: testOnCursorUpdateAfterAttach001
 * @tc.desc: test OnCursorUpdate after attach
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodAttachTest, testOnCursorUpdateAfterAttach001, TestSize.Level0)
{
    IMSA_HILOGI("test testOnCursorUpdateAfterAttach001.");
    sptr<OnTextChangedListener> TextChangeListenerImpl = new TextChangeListenerImpl();
    auto ret = inputMethodController_->Attach(TextChangeListenerImpl);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    CursorInfo cursorInfo = { .top = 5, .left = 5, .height = 5, .width = 0.8 };
    ret = inputMethodController_->OnCursorUpdate(cursorInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    TextTotalConfig totalConfig;
    ret = inputMethodAbility_->GetTextConfig(totalConfig);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(totalConfig.cursorInfo.height, cursorInfo.height);
    EXPECT_EQ(totalConfig.cursorInfo.width, cursorInfo.width);
    EXPECT_EQ(totalConfig.cursorInfo.left, cursorInfo.left);
    EXPECT_EQ(totalConfig.cursorInfo.top, cursorInfo.top);
}

/**
 * @tc.name: testOnCursorUpdateAfterAttach002
 * @tc.desc: test OnCursorUpdate after attach
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodAttachTest, testOnCursorUpdateAfterAttach002, TestSize.Level0)
{
    IMSA_HILOGI("test testOnCursorUpdateAfterAttach002.");
    sptr<OnTextChangedListener> TextChangeListenerImpl = new TextChangeListenerImpl();
    InputAttribute attribute;
    attribute.inputPattern = 3;
    attribute.enterKeyType = 2;
    TextConfig config;
    config.inputAttribute = attribute;
    config.cursorInfo = { .top = 1, .left = 1, .height = 1, .width = 0.4 };
    auto ret = inputMethodController_->Attach(TextChangeListenerImpl, true, config);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    CursorInfo cursorInfo = { .top = 5, .left = 5, .height = 5, .width = 0.8 };
    ret = inputMethodController_->OnCursorUpdate(cursorInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    TextTotalConfig totalConfig;
    ret = inputMethodAbility_->GetTextConfig(totalConfig);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(totalConfig.cursorInfo.height, cursorInfo.height);
    EXPECT_EQ(totalConfig.cursorInfo.width, cursorInfo.width);
    EXPECT_EQ(totalConfig.cursorInfo.left, cursorInfo.left);
    EXPECT_EQ(totalConfig.cursorInfo.top, cursorInfo.top);
}

/**
 * @tc.name: testOnSelectionChangeAfterAttach001
 * @tc.desc: test OnSelectionChange after attach
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodAttachTest, testOnSelectionChangeAfterAttach001, TestSize.Level0)
{
    IMSA_HILOGI("test testOnSelectionChangeAfterAttach001.");
    sptr<OnTextChangedListener> TextChangeListenerImpl = new TextChangeListenerImpl();
    auto ret = inputMethodController_->Attach(TextChangeListenerImpl);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    int start = 0;
    int end = 1;
    ret = inputMethodController_->OnSelectionChange(Str8ToStr16("aaa"), start, end);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    TextTotalConfig totalConfig;
    ret = inputMethodAbility_->GetTextConfig(totalConfig);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(totalConfig.textSelection.newBegin, start);
    EXPECT_EQ(totalConfig.textSelection.newEnd, end);
    EXPECT_EQ(totalConfig.textSelection.oldBegin, 0);
    EXPECT_EQ(totalConfig.textSelection.oldEnd, 0);
}

/**
 * @tc.name: testOnSelectionChangeAfterAttach002
 * @tc.desc: test OnSelectionChange after attach
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodAttachTest, testOnSelectionChangeAfterAttach002, TestSize.Level0)
{
    IMSA_HILOGI("test testOnSelectionChangeAfterAttach002.");
    sptr<OnTextChangedListener> TextChangeListenerImpl = new TextChangeListenerImpl();
    InputAttribute attribute;
    attribute.inputPattern = 3;
    attribute.enterKeyType = 2;
    TextConfig config;
    config.inputAttribute = attribute;
    config.range = { .start = 1, .end = 2 };
    auto ret = inputMethodController_->Attach(TextChangeListenerImpl, false, config);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    int start = 0;
    int end = 1;
    ret = inputMethodController_->OnSelectionChange(Str8ToStr16("aaa"), start, end);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    TextTotalConfig totalConfig;
    ret = inputMethodAbility_->GetTextConfig(totalConfig);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(totalConfig.textSelection.newBegin, start);
    EXPECT_EQ(totalConfig.textSelection.newEnd, end);
    EXPECT_EQ(totalConfig.textSelection.oldBegin, config.range.start);
    EXPECT_EQ(totalConfig.textSelection.oldEnd, config.range.end);
}

/**
 * @tc.name: testOnConfigurationChangeAfterAttach001
 * @tc.desc: test OnConfigurationChange after attach
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodAttachTest, testOnConfigurationChangeAfterAttach001, TestSize.Level0)
{
    IMSA_HILOGI("test testOnConfigurationChangeAfterAttach001.");
    sptr<OnTextChangedListener> TextChangeListenerImpl = new TextChangeListenerImpl();
    auto ret = inputMethodController_->Attach(TextChangeListenerImpl);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    Configuration config;
    config.SetTextInputType(TextInputType::DATETIME);
    config.SetEnterKeyType(EnterKeyType::NEXT);
    ret = inputMethodController_->OnConfigurationChange(config);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    TextTotalConfig totalConfig;
    ret = inputMethodAbility_->GetTextConfig(totalConfig);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(totalConfig.inputAttribute.inputPattern, config.GetTextInputType());
    EXPECT_EQ(totalConfig.inputAttribute.enterKeyType, config.GetEnterKeyType());
}

/**
 * @tc.name: testOnConfigurationChangeAfterAttach002
 * @tc.desc: test OnConfigurationChange after attach
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodAttachTest, testOnConfigurationChangeAfterAttach002, TestSize.Level0)
{
    IMSA_HILOGI("test testOnConfigurationChangeAfterAttach002.");
    sptr<OnTextChangedListener> TextChangeListenerImpl = new TextChangeListenerImpl();
    InputAttribute attribute;
    attribute.inputPattern = 3;
    attribute.enterKeyType = 2;
    TextConfig config;
    config.inputAttribute = attribute;
    auto ret = inputMethodController_->Attach(TextChangeListenerImpl, false, config);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    Configuration configuration;
    configuration.SetTextInputType(TextInputType::DATETIME);
    configuration.SetEnterKeyType(EnterKeyType::NEXT);
    ret = inputMethodController_->OnConfigurationChange(configuration);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    TextTotalConfig totalConfig;
    ret = inputMethodAbility_->GetTextConfig(totalConfig);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(totalConfig.inputAttribute.inputPattern, configuration.GetTextInputType());
    EXPECT_EQ(totalConfig.inputAttribute.enterKeyType, configuration.GetEnterKeyType());
}

/**
 * @tc.name: testSetCallingWindowAfterAttach001
 * @tc.desc: test SetCallingWindow after attach
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodAttachTest, testSetCallingWindowAfterAttach001, TestSize.Level0)
{
    IMSA_HILOGI("test testSetCallingWindowAfterAttach001.");
    sptr<OnTextChangedListener> TextChangeListenerImpl = new TextChangeListenerImpl();
    auto ret = inputMethodController_->Attach(TextChangeListenerImpl);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    uint32_t windowId = 99;
    ret = inputMethodController_->SetCallingWindow(windowId);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    TextTotalConfig totalConfig;
    ret = inputMethodAbility_->GetTextConfig(totalConfig);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(totalConfig.windowId, windowId);
}

/**
 * @tc.name: testSetCallingWindowAfterAttach002
 * @tc.desc: test SetCallingWindow after attach
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodAttachTest, testSetCallingWindowAfterAttach002, TestSize.Level0)
{
    IMSA_HILOGI("test testSetCallingWindowAfterAttach002.");
    sptr<OnTextChangedListener> TextChangeListenerImpl = new TextChangeListenerImpl();
    InputAttribute attribute;
    attribute.inputPattern = 3;
    attribute.enterKeyType = 2;
    TextConfig config;
    config.inputAttribute = attribute;
    config.windowId = 88;
    auto ret = inputMethodController_->Attach(TextChangeListenerImpl, false, config);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    uint32_t windowId = 99;
    ret = inputMethodController_->SetCallingWindow(windowId);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    TextTotalConfig totalConfig;
    ret = inputMethodAbility_->GetTextConfig(totalConfig);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(totalConfig.windowId, windowId);
}

/**
 * @tc.name: testOnCursorUpdate001
 * @tc.desc: test OnCursorUpdate after attach
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodAttachTest, testOnCursorUpdate001, TestSize.Level0)
{
    IMSA_HILOGI("test testOnCursorUpdate001.");
    sptr<OnTextChangedListener> TextChangeListenerImpl = new TextChangeListenerImpl();
    auto ret = inputMethodController_->Attach(TextChangeListenerImpl);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    CursorInfo cursorInfo = { .top = 5, .left = 5, .height = 5, .width = 0.8 };
    ret = inputMethodController_->OnCursorUpdate(cursorInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    InputAttribute attribute;
    attribute.inputPattern = 3;
    attribute.enterKeyType = 2;
    TextConfig config;
    config.inputAttribute = attribute;
    CursorInfo cursorInfo2 = { .top = 10, .left = 9, .width = 8, .height = 7 };
    config.cursorInfo = cursorInfo2;
    ret = inputMethodController_->Attach(TextChangeListenerImpl, false, config);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    TextTotalConfig totalConfig;
    ret = inputMethodAbility_->GetTextConfig(totalConfig);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(totalConfig.cursorInfo.height, cursorInfo2.height);
    EXPECT_EQ(totalConfig.cursorInfo.width, cursorInfo2.width);
    EXPECT_EQ(totalConfig.cursorInfo.left, cursorInfo2.left);
    EXPECT_EQ(totalConfig.cursorInfo.top, cursorInfo2.top);
}

/**
 * @tc.name: testOnSelectionChange
 * @tc.desc: test OnSelectionChange after attach
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodAttachTest, testOnSelectionChange, TestSize.Level0)
{
    IMSA_HILOGI("test testOnSelectionChange.");
    sptr<OnTextChangedListener> TextChangeListenerImpl = new TextChangeListenerImpl();
    auto ret = inputMethodController_->Attach(TextChangeListenerImpl);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    int start = 0;
    int end = 1;
    ret = inputMethodController_->OnSelectionChange(Str8ToStr16("bbb"), start, end);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    InputAttribute attribute;
    attribute.inputPattern = 3;
    attribute.enterKeyType = 2;
    TextConfig config;
    config.inputAttribute = attribute;
    config.range.start = 10;
    config.range.end = 20;
    ret = inputMethodController_->Attach(TextChangeListenerImpl, false, config);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    TextTotalConfig totalConfig;
    ret = inputMethodAbility_->GetTextConfig(totalConfig);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(totalConfig.textSelection.newBegin, config.range.start);
    EXPECT_EQ(totalConfig.textSelection.newEnd, config.range.start);
    // todo
    EXPECT_EQ(totalConfig.textSelection.oldBegin, 0);
    EXPECT_EQ(totalConfig.textSelection.oldEnd, 0);
}

/**
 * @tc.name: testOnConfigurationChange
 * @tc.desc: test OnConfigurationChange after attach
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodAttachTest, testOnConfigurationChange, TestSize.Level0)
{
    IMSA_HILOGI("test testOnConfigurationChange.");
    sptr<OnTextChangedListener> TextChangeListenerImpl = new TextChangeListenerImpl();
    auto ret = inputMethodController_->Attach(TextChangeListenerImpl);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    Configuration configuration;
    configuration.SetTextInputType(TextInputType::DATETIME);
    configuration.SetEnterKeyType(EnterKeyType::NEXT);
    ret = inputMethodController_->OnConfigurationChange(configuration);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    InputAttribute attribute;
    attribute.inputPattern = 3;
    attribute.enterKeyType = 2;
    TextConfig config;
    config.inputAttribute = attribute;
    config.inputAttribute.enterKeyType = 5;
    config.inputAttribute.inputPattern = 5;
    ret = inputMethodController_->Attach(TextChangeListenerImpl, false, config);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    TextTotalConfig totalConfig;
    ret = inputMethodAbility_->GetTextConfig(totalConfig);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    EXPECT_EQ(totalConfig.inputAttribute.inputPattern, config.inputAttribute.inputPattern);
    EXPECT_EQ(totalConfig.inputAttribute.enterKeyType, config.inputAttribute.enterKeyType);
}

/**
 * @tc.name: testSetCallingWindow
 * @tc.desc: test SetCallingWindow after attach
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodAttachTest, testSetCallingWindow, TestSize.Level0)
{
    IMSA_HILOGI("test testSetCallingWindow.");
    sptr<OnTextChangedListener> TextChangeListenerImpl = new TextChangeListenerImpl();
    auto ret = inputMethodController_->Attach(TextChangeListenerImpl);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    uint32_t windowId = 88;
    ret = inputMethodController_->SetCallingWindow(windowId);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    InputAttribute attribute;
    attribute.inputPattern = 3;
    attribute.enterKeyType = 2;
    TextConfig config;
    config.windowId = 77;
    ret = inputMethodController_->Attach(TextChangeListenerImpl, false, config);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    TextTotalConfig totalConfig;
    ret = inputMethodAbility_->GetTextConfig(totalConfig);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    EXPECT_EQ(totalConfig.windowId, config.windowId);
}
} // namespace MiscServices
} // namespace OHOS
