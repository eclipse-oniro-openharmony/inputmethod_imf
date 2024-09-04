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

#define private public
#define protected public
#include "input_method_ability.h"
#include "input_method_controller.h"
#include "input_method_system_ability.h"
#include "task_manager.h"
#undef private

#include <gtest/gtest.h>
#include <string_ex.h>

#include "global.h"
#include "identity_checker_mock.h"
#include "input_attribute.h"
#include "input_method_engine_listener_impl.h"
#include "input_method_system_ability_proxy.h"
#include "input_method_system_ability_stub.h"
#include "keyboard_listener_test_impl.h"
#include "tdd_util.h"
#include "text_listener.h"

using namespace testing::ext;
namespace OHOS {
namespace MiscServices {
class InputMethodAttachTest : public testing::Test {
public:
    static sptr<InputMethodController> inputMethodController_;
    static sptr<InputMethodAbility> inputMethodAbility_;
    static sptr<InputMethodSystemAbilityProxy> imsaProxy_;
    static sptr<InputMethodSystemAbility> imsa_;
    static void SetUpTestCase(void)
    {
        IMSA_HILOGI("InputMethodAttachTest::SetUpTestCase");
        IdentityCheckerMock::ResetParam();
        imsa_ = new (std::nothrow) InputMethodSystemAbility();
        if (imsa_ == nullptr) {
            return;
        }
        imsa_->OnStart();
        imsa_->userId_ = TddUtil::GetCurrentUserId();
        imsa_->identityChecker_ = std::make_shared<IdentityCheckerMock>();
        sptr<InputMethodSystemAbilityStub> serviceStub = imsa_;
        imsaProxy_ = new InputMethodSystemAbilityProxy(serviceStub->AsObject());
        if (imsaProxy_ == nullptr) {
            return;
        }
        IdentityCheckerMock::SetFocused(true);

        inputMethodAbility_ = InputMethodAbility::GetInstance();
        inputMethodAbility_->abilityManager_ = imsaProxy_;
        TddUtil::InitCurrentImePermissionInfo();
        IdentityCheckerMock::SetBundleName(TddUtil::currentBundleNameMock_);
        inputMethodAbility_->SetCoreAndAgent();
        inputMethodAbility_->SetImeListener(std::make_shared<InputMethodEngineListenerImpl>());

        inputMethodController_ = InputMethodController::GetInstance();
        inputMethodController_->abilityManager_ = imsaProxy_;
    }
    static void TearDownTestCase(void)
    {
        IMSA_HILOGI("InputMethodAttachTest::TearDownTestCase");
        IdentityCheckerMock::ResetParam();
        imsa_->OnStop();
    }
    void SetUp()
    {
        IMSA_HILOGI("InputMethodAttachTest::SetUp");
        TaskManager::GetInstance().SetInited(true);
    }
    void TearDown()
    {
        IMSA_HILOGI("InputMethodAttachTest::TearDown");
        inputMethodController_->Close();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        TaskManager::GetInstance().Reset();
    }
};
sptr<InputMethodController> InputMethodAttachTest::inputMethodController_;
sptr<InputMethodAbility> InputMethodAttachTest::inputMethodAbility_;
sptr<InputMethodSystemAbilityProxy> InputMethodAttachTest::imsaProxy_;
sptr<InputMethodSystemAbility> InputMethodAttachTest::imsa_;

/**
 * @tc.name: testAttach001
 * @tc.desc: test Attach
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodAttachTest, testAttach001, TestSize.Level0)
{
    IMSA_HILOGI("test testAttach001 after attach.");
    sptr<OnTextChangedListener> textListener = new TextListener();
    auto ret = inputMethodController_->Attach(textListener);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    std::this_thread::sleep_for(std::chrono::seconds(2));

    int32_t keyType = -1;
    ret = inputMethodAbility_->GetEnterKeyType(keyType);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(keyType, 0);
    int32_t inputPattern = -1;
    ret = inputMethodAbility_->GetInputPattern(inputPattern);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    auto pattern = InputAttribute::PATTERN_TEXT;
    EXPECT_EQ(inputPattern, pattern);
}

/**
 * @tc.name: testAttach002
 * @tc.desc: test Attach
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodAttachTest, testAttach002, TestSize.Level0)
{
    IMSA_HILOGI("test testAttach002 after attach.");
    sptr<OnTextChangedListener> textListener = new TextListener();
    auto ret = inputMethodController_->Attach(textListener, false);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    int32_t keyType = -1;
    ret = inputMethodAbility_->GetEnterKeyType(keyType);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(keyType, 0);
    int32_t inputPattern = -1;
    ret = inputMethodAbility_->GetInputPattern(inputPattern);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    auto pattern = InputAttribute::PATTERN_TEXT;
    EXPECT_EQ(inputPattern, pattern);
}

/**
 * @tc.name: testAttach003
 * @tc.desc: test Attach
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodAttachTest, testAttach003, TestSize.Level0)
{
    IMSA_HILOGI("test testAttach003 after attach.");
    sptr<OnTextChangedListener> textListener = new TextListener();
    InputAttribute attribute;
    attribute.inputPattern = 2;
    attribute.enterKeyType = 1;
    auto ret = inputMethodController_->Attach(textListener, true, attribute);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    int32_t keyType = -1;
    ret = inputMethodAbility_->GetEnterKeyType(keyType);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(keyType, attribute.enterKeyType);
    int32_t inputPattern = -1;
    ret = inputMethodAbility_->GetInputPattern(inputPattern);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(inputPattern, attribute.inputPattern);
}

/**
 * @tc.name: testAttach004
 * @tc.desc: test Attach
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodAttachTest, testAttach004, TestSize.Level0)
{
    IMSA_HILOGI("test testAttach004 after attach.");
    sptr<OnTextChangedListener> textListener = new TextListener();
    InputAttribute attribute;
    attribute.inputPattern = 3;
    attribute.enterKeyType = 2;
    TextConfig config;
    config.inputAttribute = attribute;
    auto ret = inputMethodController_->Attach(textListener, false, config);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    int32_t keyType = -1;
    ret = inputMethodAbility_->GetEnterKeyType(keyType);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(keyType, config.inputAttribute.enterKeyType);
    int32_t inputPattern = -1;
    ret = inputMethodAbility_->GetInputPattern(inputPattern);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(inputPattern, config.inputAttribute.inputPattern);
}

/**
 * @tc.name: testAttach005
 * @tc.desc: test Attach, test optional param in TextConfig
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodAttachTest, testAttach005, TestSize.Level0)
{
    IMSA_HILOGI("test testAttach005 after attach.");
    sptr<OnTextChangedListener> textListener = new TextListener();
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
    Range selectionRange;
    selectionRange.start = 0;
    selectionRange.end = 2;
    config.range = selectionRange;
    config.windowId = 10;
    inputMethodController_->Close();
    auto ret = inputMethodController_->Attach(textListener, true, config);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    int32_t keyType = -1;
    ret = inputMethodAbility_->GetEnterKeyType(keyType);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(keyType, config.inputAttribute.enterKeyType);
    int32_t inputPattern = -1;
    ret = inputMethodAbility_->GetInputPattern(inputPattern);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(inputPattern, config.inputAttribute.inputPattern);

    TextTotalConfig textConfig;
    ret = inputMethodAbility_->GetTextConfig(textConfig);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(textConfig.inputAttribute, config.inputAttribute);
    EXPECT_EQ(textConfig.windowId, config.windowId);
    EXPECT_EQ(textConfig.cursorInfo, config.cursorInfo);
    EXPECT_EQ(textConfig.textSelection.newBegin, config.range.start);
    EXPECT_EQ(textConfig.textSelection.newEnd, config.range.end);
    EXPECT_EQ(textConfig.textSelection.oldBegin, INVALID_VALUE);
    EXPECT_EQ(textConfig.textSelection.oldEnd, INVALID_VALUE);
}

/**
 * @tc.name: testAttach006
 * @tc.desc: test Attach
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodAttachTest, testAttach006, TestSize.Level0)
{
    IMSA_HILOGI("test testAttach006 attach.");
    InputMethodAttachTest::inputMethodController_->Close();
    TextListener::ResetParam();
    sptr<OnTextChangedListener> textListener = new TextListener();
    auto ret = InputMethodAttachTest::inputMethodController_->Attach(textListener, false);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(TextListener::WaitSendKeyboardStatusCallback(KeyboardStatus::NONE));
    std::this_thread::sleep_for(std::chrono::seconds(2));

    InputMethodAttachTest::inputMethodController_->Close();
    TextListener::ResetParam();
    ret = InputMethodAttachTest::inputMethodController_->Attach(textListener, true);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(TextListener::WaitSendKeyboardStatusCallback(KeyboardStatus::SHOW));
}

/**
 * @tc.name: testOnConfigurationChangeWithoutAttach
 * @tc.desc: test OnConfigurationChange without attach
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodAttachTest, testOnConfigurationChangeWithoutAttach, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAttachTest testOnConfigurationChangeWithoutAttach in.");
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
    sptr<OnTextChangedListener> textListener = new TextListener();
    auto ret = inputMethodController_->Attach(textListener);
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
}

/**
 * @tc.name: testGetTextConfig
 * @tc.desc: test GetTextConfig of InputMethodAbility
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodAttachTest, testGetTextConfig, TestSize.Level0)
{
    IMSA_HILOGI("test OnConfigurationChange001 after attach.");
    sptr<OnTextChangedListener> textListener = new TextListener();
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
    Range selectionRange;
    selectionRange.start = 0;
    selectionRange.end = 2;
    config.range = selectionRange;
    config.windowId = 10;
    inputMethodController_->Close();
    auto ret = inputMethodController_->Attach(textListener, false, config);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    TextTotalConfig totalConfig;
    ret = inputMethodAbility_->GetTextConfig(totalConfig);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    EXPECT_EQ(totalConfig.inputAttribute, attribute);
    EXPECT_EQ(totalConfig.cursorInfo, cursorInfo);
    EXPECT_EQ(totalConfig.textSelection.newBegin, selectionRange.start);
    EXPECT_EQ(totalConfig.textSelection.newEnd, selectionRange.end);
    EXPECT_EQ(totalConfig.textSelection.oldBegin, INVALID_VALUE);
    EXPECT_EQ(totalConfig.textSelection.oldEnd, INVALID_VALUE);
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
    sptr<OnTextChangedListener> textListener = new TextListener();
    auto ret = inputMethodController_->Attach(textListener);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    CursorInfo cursorInfo = { .top = 5, .left = 5, .height = 5, .width = 0.8 };
    ret = inputMethodController_->OnCursorUpdate(cursorInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    TextTotalConfig totalConfig;
    ret = inputMethodAbility_->GetTextConfig(totalConfig);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(totalConfig.cursorInfo, cursorInfo);
}

/**
 * @tc.name: testOnCursorUpdateAfterAttach002
 * @tc.desc: test OnCursorUpdate after attach
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodAttachTest, testOnCursorUpdateAfterAttach002, TestSize.Level0)
{
    IMSA_HILOGI("test testOnCursorUpdateAfterAttach002.");
    sptr<OnTextChangedListener> textListener = new TextListener();
    InputAttribute attribute;
    attribute.inputPattern = 3;
    attribute.enterKeyType = 2;
    TextConfig config;
    config.inputAttribute = attribute;
    config.cursorInfo = { .top = 1, .left = 1, .height = 1, .width = 0.4 };
    auto ret = inputMethodController_->Attach(textListener, true, config);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    CursorInfo cursorInfo = { .top = 5, .left = 5, .height = 5, .width = 0.8 };
    ret = inputMethodController_->OnCursorUpdate(cursorInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    TextTotalConfig totalConfig;
    ret = inputMethodAbility_->GetTextConfig(totalConfig);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(totalConfig.cursorInfo, cursorInfo);
}

/**
 * @tc.name: testOnSelectionChangeAfterAttach002
 * @tc.desc: test OnSelectionChange after attach
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodAttachTest, testOnSelectionChangeAfterAttach002, TestSize.Level0)
{
    IMSA_HILOGI("test testOnSelectionChangeAfterAttach002.");
    sptr<OnTextChangedListener> textListener = new TextListener();
    InputAttribute attribute;
    attribute.inputPattern = 3;
    attribute.enterKeyType = 2;
    TextConfig config;
    config.inputAttribute = attribute;
    config.range = { .start = 1, .end = 2 };
    inputMethodController_->Close();
    auto ret = inputMethodController_->Attach(textListener, false, config);
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
    sptr<OnTextChangedListener> textListener = new TextListener();
    auto ret = inputMethodController_->Attach(textListener);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    Configuration config;
    config.SetTextInputType(TextInputType::DATETIME);
    config.SetEnterKeyType(EnterKeyType::NEXT);
    ret = inputMethodController_->OnConfigurationChange(config);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    TextTotalConfig totalConfig;
    ret = inputMethodAbility_->GetTextConfig(totalConfig);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(totalConfig.inputAttribute.inputPattern, static_cast<int32_t>(TextInputType::DATETIME));
    EXPECT_EQ(totalConfig.inputAttribute.enterKeyType, static_cast<int32_t>(EnterKeyType::NEXT));
}

/**
 * @tc.name: testOnConfigurationChangeAfterAttach002
 * @tc.desc: test OnConfigurationChange after attach
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodAttachTest, testOnConfigurationChangeAfterAttach002, TestSize.Level0)
{
    IMSA_HILOGI("test testOnConfigurationChangeAfterAttach002.");
    sptr<OnTextChangedListener> textListener = new TextListener();
    InputAttribute attribute;
    attribute.inputPattern = 3;
    attribute.enterKeyType = 2;
    TextConfig config;
    config.inputAttribute = attribute;
    auto ret = inputMethodController_->Attach(textListener, false, config);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    Configuration configuration;
    configuration.SetTextInputType(TextInputType::DATETIME);
    configuration.SetEnterKeyType(EnterKeyType::NEXT);
    ret = inputMethodController_->OnConfigurationChange(configuration);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    TextTotalConfig totalConfig;
    ret = inputMethodAbility_->GetTextConfig(totalConfig);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(totalConfig.inputAttribute.inputPattern, static_cast<int32_t>(configuration.GetTextInputType()));
    EXPECT_EQ(totalConfig.inputAttribute.enterKeyType, static_cast<int32_t>(configuration.GetEnterKeyType()));
}

/**
 * @tc.name: testSetCallingWindowAfterAttach002
 * @tc.desc: test SetCallingWindow after attach
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodAttachTest, testSetCallingWindowAfterAttach002, TestSize.Level0)
{
    IMSA_HILOGI("test testSetCallingWindowAfterAttach002.");
    sptr<OnTextChangedListener> textListener = new TextListener();
    InputAttribute attribute;
    attribute.inputPattern = 3;
    attribute.enterKeyType = 2;
    TextConfig config;
    config.inputAttribute = attribute;
    config.windowId = 88;
    auto ret = inputMethodController_->Attach(textListener, false, config);
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
    sptr<OnTextChangedListener> textListener = new TextListener();
    auto ret = inputMethodController_->Attach(textListener);
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
    ret = inputMethodController_->Attach(textListener, false, config);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    TextTotalConfig totalConfig;
    ret = inputMethodAbility_->GetTextConfig(totalConfig);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(totalConfig.cursorInfo, cursorInfo2);
}

/**
 * @tc.name: testOnSelectionChange
 * @tc.desc: test OnSelectionChange after attach
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodAttachTest, testOnSelectionChange, TestSize.Level0)
{
    IMSA_HILOGI("test testOnSelectionChange.");
    sptr<OnTextChangedListener> textListener = new TextListener();
    auto ret = inputMethodController_->Attach(textListener);
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
    ret = inputMethodController_->Attach(textListener, false, config);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    TextTotalConfig totalConfig;
    ret = inputMethodAbility_->GetTextConfig(totalConfig);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(totalConfig.textSelection.newBegin, config.range.start);
    EXPECT_EQ(totalConfig.textSelection.newEnd, config.range.end);
    EXPECT_EQ(totalConfig.textSelection.oldBegin, start);
    EXPECT_EQ(totalConfig.textSelection.oldEnd, end);
}

/**
 * @tc.name: testOnConfigurationChange002
 * @tc.desc: test OnConfigurationChange after attach
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodAttachTest, testOnConfigurationChange002, TestSize.Level0)
{
    IMSA_HILOGI("test testOnConfigurationChange002.");
    sptr<OnTextChangedListener> textListener = new TextListener();
    auto ret = inputMethodController_->Attach(textListener);
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
    ret = inputMethodController_->Attach(textListener, false, config);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    TextTotalConfig totalConfig;
    ret = inputMethodAbility_->GetTextConfig(totalConfig);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    EXPECT_EQ(totalConfig.inputAttribute, config.inputAttribute);
}

/**
 * @tc.name: testSetCallingWindow
 * @tc.desc: test SetCallingWindow after attach
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodAttachTest, testSetCallingWindow, TestSize.Level0)
{
    IMSA_HILOGI("test testSetCallingWindow.");
    sptr<OnTextChangedListener> textListener = new TextListener();
    auto ret = inputMethodController_->Attach(textListener);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    uint32_t windowId = 88;
    ret = inputMethodController_->SetCallingWindow(windowId);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    InputAttribute attribute;
    attribute.inputPattern = 3;
    attribute.enterKeyType = 2;
    TextConfig config;
    config.windowId = 77;
    ret = inputMethodController_->Attach(textListener, false, config);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    TextTotalConfig totalConfig;
    ret = inputMethodAbility_->GetTextConfig(totalConfig);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    EXPECT_EQ(totalConfig.windowId, config.windowId);
}
/**
 * @tc.name: testImeCallbackInAttach
 * @tc.desc: test ime can receive callback in Attach
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodAttachTest, testImeCallbackInAttach, TestSize.Level0)
{
    IMSA_HILOGI("test testImeCallbackInAttach.");
    inputMethodAbility_->SetImeListener(std::make_shared<InputMethodEngineListenerImpl>());
    inputMethodAbility_->SetKdListener(std::make_shared<KeyboardListenerTestImpl>());
    sptr<OnTextChangedListener> textListener = new TextListener();
    InputMethodEngineListenerImpl::ResetParam();
    KeyboardListenerTestImpl::ResetParam();
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
    Range selectionRange;
    selectionRange.start = 5;
    selectionRange.end = 2;
    config.range = selectionRange;
    config.windowId = 10;
    auto ret = inputMethodController_->Attach(textListener, true, config);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    EXPECT_TRUE(KeyboardListenerTestImpl::WaitSelectionChange(selectionRange.start));
    EXPECT_TRUE(KeyboardListenerTestImpl::WaitCursorUpdate());
    EXPECT_TRUE(KeyboardListenerTestImpl::WaitEditorAttributeChange(attribute));
    EXPECT_TRUE(InputMethodEngineListenerImpl::WaitSetCallingWindow(config.windowId));
}
} // namespace MiscServices
} // namespace OHOS
