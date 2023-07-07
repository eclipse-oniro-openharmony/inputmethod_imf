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

#include <gtest/gtest.h>
#include <string_ex.h>

#include "global.h"
#include "input_attribute.h"
#include "input_method_ability.h"
#include "input_method_controller.h"
#include "tdd_util.h"

using namespace testing::ext;
namespace OHOS {
namespace MiscServices {
class InputMethodAttachTest : public testing::Test {
public:
    static sptr<InputMethodController> inputMethodController_;
    static sptr<InputMethodAbility> inputMethodAbility_;

    class EngineListenerImpl : public InputMethodEngineListener {
    public:
        EngineListenerImpl() = default;
        ~EngineListenerImpl() = default;

        void OnKeyboardStatus(bool isShow)
        {
            IMSA_HILOGI("EngineListenerImpl OnKeyboardStatus");
        }

        void OnInputStart()
        {
            IMSA_HILOGI("EngineListenerImpl OnInputStart");
        }

        void OnInputStop(const std::string &imeId)
        {
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
        void InsertText(const std::u16string &text) override {}

        void DeleteForward(int32_t length) override {}

        void DeleteBackward(int32_t length) override {}

        void SendKeyEventFromInputMethod(const KeyEvent &event) override {}

        void SendKeyboardStatus(const KeyboardStatus &keyboardStatus) override {}

        void SendFunctionKey(const FunctionKey &functionKey) override {}

        void SetKeyboardStatus(bool status) override {}

        void MoveCursor(const Direction direction) override {}

        void HandleSetSelection(int32_t start, int32_t end) override {}

        void HandleExtendAction(int32_t action) override {}

        void HandleSelect(int32_t keyCode, int32_t cursorMoveSkip) override {}
    };
    static void SetUpTestCase(void)
    {
        IMSA_HILOGI("InputMethodAttachTest::SetUpTestCase");
        // Set the tokenID to the tokenID of the current ime
        TddUtil::StorageSelfTokenID();
        std::shared_ptr<Property> property = InputMethodController::GetInstance()->GetCurrentInputMethod();
        std::string bundleName = property != nullptr ? property->name : "default.inputmethod.unittest";
        TddUtil::SetTestTokenID(TddUtil::GetTestTokenID(bundleName));
        inputMethodAbility_ = InputMethodAbility::GetInstance();
        inputMethodAbility_->OnImeReady();
        inputMethodAbility_->SetCoreAndAgent();
        TddUtil::RestoreSelfTokenID();

        // Set the uid to the uid of the focus app
        TddUtil::StorageSelfUid();
        TddUtil::SetTestUid();
        inputMethodController_ = InputMethodController::GetInstance();
    }
    static void TearDownTestCase(void)
    {
        IMSA_HILOGI("InputMethodAttachTest::TearDownTestCase");
        TddUtil::RestoreSelfUid();
    }
    void SetUp()
    {
        IMSA_HILOGI("InputMethodAttachTest::SetUp");
    }
    void TearDown()
    {
        IMSA_HILOGI("InputMethodAttachTest::TearDown");
        inputMethodController_->Close();
    }
};
sptr<InputMethodController> InputMethodAttachTest::inputMethodController_;
sptr<InputMethodAbility> InputMethodAttachTest::inputMethodAbility_;

/**
 * @tc.name: testAttach001
 * @tc.desc: test Attach
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodAttachTest, testAttach001, TestSize.Level0)
{
    IMSA_HILOGI("test testAttach001 after attach.");
    sptr<OnTextChangedListener> textChangeListenerImpl = new TextChangeListenerImpl();
    auto ret = inputMethodController_->Attach(textChangeListenerImpl);
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
 * @tc.name: testAttach002
 * @tc.desc: test Attach
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodAttachTest, testAttach002, TestSize.Level0)
{
    IMSA_HILOGI("test testAttach002 after attach.");
    sptr<OnTextChangedListener> textChangeListenerImpl = new TextChangeListenerImpl();
    auto ret = inputMethodController_->Attach(textChangeListenerImpl, false);
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
    sptr<OnTextChangedListener> textChangeListenerImpl = new TextChangeListenerImpl();
    InputAttribute attribute;
    attribute.inputPattern = 2;
    attribute.enterKeyType = 1;
    auto ret = inputMethodController_->Attach(textChangeListenerImpl, true, attribute);
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
    sptr<OnTextChangedListener> textChangeListenerImpl = new TextChangeListenerImpl();
    InputAttribute attribute;
    attribute.inputPattern = 3;
    attribute.enterKeyType = 2;
    TextConfig config;
    config.inputAttribute = attribute;
    auto ret = inputMethodController_->Attach(textChangeListenerImpl, false, config);
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
    sptr<OnTextChangedListener> textChangeListenerImpl = new TextChangeListenerImpl();
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
    auto ret = inputMethodController_->Attach(textChangeListenerImpl, true, config);
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
    EXPECT_EQ(textConfig.inputAttribute.inputPattern, config.inputAttribute.inputPattern);
    EXPECT_EQ(textConfig.inputAttribute.enterKeyType, config.inputAttribute.enterKeyType);
    EXPECT_EQ(textConfig.windowId, config.windowId);
    EXPECT_EQ(textConfig.cursorInfo, config.cursorInfo);
    EXPECT_EQ(textConfig.textSelection.newBegin, config.range.start);
    EXPECT_EQ(textConfig.textSelection.newEnd, config.range.end);
    EXPECT_EQ(textConfig.textSelection.oldBegin, 0);
    EXPECT_EQ(textConfig.textSelection.oldEnd, 0);
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
    sptr<OnTextChangedListener> textChangeListenerImpl = new TextChangeListenerImpl();
    auto ret = inputMethodController_->Attach(textChangeListenerImpl);
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
    sptr<OnTextChangedListener> textChangeListenerImpl = new TextChangeListenerImpl();
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
    auto ret = inputMethodController_->Attach(textChangeListenerImpl, false, config);
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
    sptr<OnTextChangedListener> textChangeListenerImpl = new TextChangeListenerImpl();
    auto ret = inputMethodController_->Attach(textChangeListenerImpl);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    CursorInfo cursorInfo = { .top = 5, .left = 5, .height = 5, .width = 0.8 };
    ret = inputMethodController_->OnCursorUpdate(cursorInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    TextTotalConfig totalConfig;
    ret = inputMethodAbility_->GetTextConfig(totalConfig);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(totalConfig.cursorInfo.height, -1);
    EXPECT_EQ(totalConfig.cursorInfo.width, -1);
    EXPECT_EQ(totalConfig.cursorInfo.left, -1);
    EXPECT_EQ(totalConfig.cursorInfo.top, -1);
}

/**
 * @tc.name: testOnCursorUpdateAfterAttach002
 * @tc.desc: test OnCursorUpdate after attach
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodAttachTest, testOnCursorUpdateAfterAttach002, TestSize.Level0)
{
    IMSA_HILOGI("test testOnCursorUpdateAfterAttach002.");
    sptr<OnTextChangedListener> textChangeListenerImpl = new TextChangeListenerImpl();
    InputAttribute attribute;
    attribute.inputPattern = 3;
    attribute.enterKeyType = 2;
    TextConfig config;
    config.inputAttribute = attribute;
    config.cursorInfo = { .top = 1, .left = 1, .height = 1, .width = 0.4 };
    auto ret = inputMethodController_->Attach(textChangeListenerImpl, true, config);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    CursorInfo cursorInfo = { .top = 5, .left = 5, .height = 5, .width = 0.8 };
    ret = inputMethodController_->OnCursorUpdate(cursorInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    TextTotalConfig totalConfig;
    ret = inputMethodAbility_->GetTextConfig(totalConfig);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(totalConfig.cursorInfo.height, config.cursorInfo.height);
    EXPECT_EQ(totalConfig.cursorInfo.width, config.cursorInfo.width);
    EXPECT_EQ(totalConfig.cursorInfo.left, config.cursorInfo.left);
    EXPECT_EQ(totalConfig.cursorInfo.top, config.cursorInfo.top);
}

/**
 * @tc.name: testOnSelectionChangeAfterAttach002
 * @tc.desc: test OnSelectionChange after attach
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodAttachTest, testOnSelectionChangeAfterAttach002, TestSize.Level0)
{
    IMSA_HILOGI("test testOnSelectionChangeAfterAttach002.");
    sptr<OnTextChangedListener> textChangeListenerImpl = new TextChangeListenerImpl();
    InputAttribute attribute;
    attribute.inputPattern = 3;
    attribute.enterKeyType = 2;
    TextConfig config;
    config.inputAttribute = attribute;
    config.range = { .start = 1, .end = 2 };
    auto ret = inputMethodController_->Attach(textChangeListenerImpl, false, config);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    int start = 0;
    int end = 1;
    ret = inputMethodController_->OnSelectionChange(Str8ToStr16("aaa"), start, end);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    TextTotalConfig totalConfig;
    ret = inputMethodAbility_->GetTextConfig(totalConfig);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(totalConfig.textSelection.newBegin, config.range.start);
    EXPECT_EQ(totalConfig.textSelection.newEnd, config.range.end);
    EXPECT_EQ(totalConfig.textSelection.oldBegin, 0);
    EXPECT_EQ(totalConfig.textSelection.oldEnd, 0);
}

/**
 * @tc.name: testOnConfigurationChangeAfterAttach001
 * @tc.desc: test OnConfigurationChange after attach
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodAttachTest, testOnConfigurationChangeAfterAttach001, TestSize.Level0)
{
    IMSA_HILOGI("test testOnConfigurationChangeAfterAttach001.");
    sptr<OnTextChangedListener> textChangeListenerImpl = new TextChangeListenerImpl();
    auto ret = inputMethodController_->Attach(textChangeListenerImpl);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    Configuration config;
    config.SetTextInputType(TextInputType::DATETIME);
    config.SetEnterKeyType(EnterKeyType::NEXT);
    ret = inputMethodController_->OnConfigurationChange(config);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    TextTotalConfig totalConfig;
    ret = inputMethodAbility_->GetTextConfig(totalConfig);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(totalConfig.inputAttribute.inputPattern, 1);
    EXPECT_EQ(totalConfig.inputAttribute.enterKeyType, 0);
}

/**
 * @tc.name: testOnConfigurationChangeAfterAttach002
 * @tc.desc: test OnConfigurationChange after attach
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodAttachTest, testOnConfigurationChangeAfterAttach002, TestSize.Level0)
{
    IMSA_HILOGI("test testOnConfigurationChangeAfterAttach002.");
    sptr<OnTextChangedListener> textChangeListenerImpl = new TextChangeListenerImpl();
    InputAttribute attribute;
    attribute.inputPattern = 3;
    attribute.enterKeyType = 2;
    TextConfig config;
    config.inputAttribute = attribute;
    auto ret = inputMethodController_->Attach(textChangeListenerImpl, false, config);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    Configuration configuration;
    configuration.SetTextInputType(TextInputType::DATETIME);
    configuration.SetEnterKeyType(EnterKeyType::NEXT);
    ret = inputMethodController_->OnConfigurationChange(configuration);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    TextTotalConfig totalConfig;
    ret = inputMethodAbility_->GetTextConfig(totalConfig);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(totalConfig.inputAttribute.inputPattern, static_cast<int32_t>(config.inputAttribute.inputPattern));
    EXPECT_EQ(totalConfig.inputAttribute.enterKeyType, static_cast<int32_t>(config.inputAttribute.enterKeyType));
}

/**
 * @tc.name: testSetCallingWindowAfterAttach002
 * @tc.desc: test SetCallingWindow after attach
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodAttachTest, testSetCallingWindowAfterAttach002, TestSize.Level0)
{
    IMSA_HILOGI("test testSetCallingWindowAfterAttach002.");
    sptr<OnTextChangedListener> textChangeListenerImpl = new TextChangeListenerImpl();
    InputAttribute attribute;
    attribute.inputPattern = 3;
    attribute.enterKeyType = 2;
    TextConfig config;
    config.inputAttribute = attribute;
    config.windowId = 88;
    auto ret = inputMethodController_->Attach(textChangeListenerImpl, false, config);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    uint32_t windowId = 99;
    ret = inputMethodController_->SetCallingWindow(windowId);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    TextTotalConfig totalConfig;
    ret = inputMethodAbility_->GetTextConfig(totalConfig);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(totalConfig.windowId, config.windowId);
}

/**
 * @tc.name: testOnCursorUpdate001
 * @tc.desc: test OnCursorUpdate after attach
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodAttachTest, testOnCursorUpdate001, TestSize.Level0)
{
    IMSA_HILOGI("test testOnCursorUpdate001.");
    sptr<OnTextChangedListener> textChangeListenerImpl = new TextChangeListenerImpl();
    auto ret = inputMethodController_->Attach(textChangeListenerImpl);
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
    ret = inputMethodController_->Attach(textChangeListenerImpl, false, config);
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
    sptr<OnTextChangedListener> textChangeListenerImpl = new TextChangeListenerImpl();
    auto ret = inputMethodController_->Attach(textChangeListenerImpl);
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
    ret = inputMethodController_->Attach(textChangeListenerImpl, false, config);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    TextTotalConfig totalConfig;
    ret = inputMethodAbility_->GetTextConfig(totalConfig);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(totalConfig.textSelection.newBegin, config.range.start);
    EXPECT_EQ(totalConfig.textSelection.newEnd, config.range.end);
    EXPECT_EQ(totalConfig.textSelection.oldBegin, 0);
    EXPECT_EQ(totalConfig.textSelection.oldEnd, 0);
}

/**
 * @tc.name: testOnConfigurationChange002
 * @tc.desc: test OnConfigurationChange after attach
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodAttachTest, testOnConfigurationChange002, TestSize.Level0)
{
    IMSA_HILOGI("test testOnConfigurationChange002.");
    sptr<OnTextChangedListener> textChangeListenerImpl = new TextChangeListenerImpl();
    auto ret = inputMethodController_->Attach(textChangeListenerImpl);
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
    ret = inputMethodController_->Attach(textChangeListenerImpl, false, config);
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
    sptr<OnTextChangedListener> textChangeListenerImpl = new TextChangeListenerImpl();
    auto ret = inputMethodController_->Attach(textChangeListenerImpl);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    uint32_t windowId = 88;
    ret = inputMethodController_->SetCallingWindow(windowId);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    InputAttribute attribute;
    attribute.inputPattern = 3;
    attribute.enterKeyType = 2;
    TextConfig config;
    config.windowId = 77;
    ret = inputMethodController_->Attach(textChangeListenerImpl, false, config);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    TextTotalConfig totalConfig;
    ret = inputMethodAbility_->GetTextConfig(totalConfig);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    EXPECT_EQ(totalConfig.windowId, config.windowId);
}
} // namespace MiscServices
} // namespace OHOS
