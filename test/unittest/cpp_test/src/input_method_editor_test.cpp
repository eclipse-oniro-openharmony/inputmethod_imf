/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include <event_handler.h>
#include <gtest/gtest.h>
#include <string_ex.h>
#include <sys/time.h>

#include <condition_variable>
#include <cstdint>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "global.h"
#include "i_input_method_agent.h"
#include "i_input_method_system_ability.h"
#include "identity_checker_mock.h"
#include "input_client_stub.h"
#include "input_data_channel_stub.h"
#include "input_method_ability.h"
#include "input_method_controller.h"
#include "input_method_engine_listener_impl.h"
#include "input_method_system_ability_proxy.h"
#include "input_method_utils.h"
#include "keyboard_listener.h"
#include "message_parcel.h"
#include "tdd_util.h"
#include "text_listener.h"

using namespace testing::ext;
namespace OHOS {
namespace MiscServices {
class KeyboardListenerImpl : public KeyboardListener {
public:
    KeyboardListenerImpl() {};
    ~KeyboardListenerImpl() {};
    static int32_t keyCode_;
    static int32_t keyStatus_;
    static CursorInfo cursorInfo_;
    bool OnDealKeyEvent(const std::shared_ptr<MMI::KeyEvent> &keyEvent, sptr<KeyEventConsumerProxy> &consumer) override;
    bool OnKeyEvent(int32_t keyCode, int32_t keyStatus, sptr<KeyEventConsumerProxy> &consumer) override;
    bool OnKeyEvent(const std::shared_ptr<MMI::KeyEvent> &keyEvent, sptr<KeyEventConsumerProxy> &consumer) override;
    void OnCursorUpdate(int32_t positionX, int32_t positionY, int32_t height) override;
    void OnSelectionChange(int32_t oldBegin, int32_t oldEnd, int32_t newBegin, int32_t newEnd) override;
    void OnTextChange(const std::string &text) override;
    void OnEditorAttributeChange(const InputAttribute &inputAttribute) override;
};
int32_t KeyboardListenerImpl::keyCode_ = 0;
int32_t KeyboardListenerImpl::keyStatus_ = 0;
CursorInfo KeyboardListenerImpl::cursorInfo_ = {};
bool KeyboardListenerImpl::OnKeyEvent(int32_t keyCode, int32_t keyStatus, sptr<KeyEventConsumerProxy> &consumer)
{
    IMSA_HILOGD("KeyboardListenerImpl::OnKeyEvent %{public}d %{public}d", keyCode, keyStatus);
    keyCode_ = keyCode;
    keyStatus_ = keyStatus;
    return true;
}
bool KeyboardListenerImpl::OnKeyEvent(
    const std::shared_ptr<MMI::KeyEvent> &keyEvent, sptr<KeyEventConsumerProxy> &consumer)
{
    return true;
}
bool KeyboardListenerImpl::OnDealKeyEvent(
    const std::shared_ptr<MMI::KeyEvent> &keyEvent, sptr<KeyEventConsumerProxy> &consumer)
{
    bool isKeyCodeConsume = OnKeyEvent(keyEvent->GetKeyCode(), keyEvent->GetKeyAction(), consumer);
    bool isKeyEventConsume = OnKeyEvent(keyEvent, consumer);
    if (consumer != nullptr) {
        consumer->OnKeyEventResult(isKeyEventConsume | isKeyCodeConsume);
    }
    return true;
}
void KeyboardListenerImpl::OnCursorUpdate(int32_t positionX, int32_t positionY, int32_t height)
{
    IMSA_HILOGD("KeyboardListenerImpl::OnCursorUpdate %{public}d %{public}d %{public}d", positionX, positionY, height);
    cursorInfo_ = { static_cast<double>(positionX), static_cast<double>(positionY), 0, static_cast<double>(height) };
}
void KeyboardListenerImpl::OnSelectionChange(int32_t oldBegin, int32_t oldEnd, int32_t newBegin, int32_t newEnd) { }
void KeyboardListenerImpl::OnTextChange(const std::string &text) { }
void KeyboardListenerImpl::OnEditorAttributeChange(const InputAttribute &inputAttribute) { }

class InputMethodEditorTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static sptr<InputMethodController> inputMethodController_;
    static sptr<InputMethodAbility> inputMethodAbility_;
    static std::shared_ptr<MMI::KeyEvent> keyEvent_;
    static std::shared_ptr<KeyboardListenerImpl> kbListener_;
    static std::shared_ptr<InputMethodEngineListenerImpl> imeListener_;
    static sptr<OnTextChangedListener> textListener_;
    static sptr<InputMethodSystemAbility> imsa_;
    static constexpr int32_t waitTaskEmptyTimes_ = 100;
    static constexpr int32_t waitTaskEmptyinterval_ = 20;

    static bool IsTaskEmpty()
    {
        return TaskManager::GetInstance().curTask_ == nullptr &&
               TaskManager::GetInstance().amsTasks_.empty() &&
               TaskManager::GetInstance().imaTasks_.empty() &&
               TaskManager::GetInstance().imsaTasks_.empty() &&
               TaskManager::GetInstance().innerTasks_.empty();
    }
};
sptr<InputMethodController> InputMethodEditorTest::inputMethodController_;
sptr<InputMethodAbility> InputMethodEditorTest::inputMethodAbility_;
std::shared_ptr<MMI::KeyEvent> InputMethodEditorTest::keyEvent_;
std::shared_ptr<KeyboardListenerImpl> InputMethodEditorTest::kbListener_;
std::shared_ptr<InputMethodEngineListenerImpl> InputMethodEditorTest::imeListener_;
sptr<OnTextChangedListener> InputMethodEditorTest::textListener_;
sptr<InputMethodSystemAbility> InputMethodEditorTest::imsa_;

void InputMethodEditorTest::SetUpTestCase(void)
{
    IMSA_HILOGI("InputMethodEditorTest::SetUpTestCase");
    IdentityCheckerMock::ResetParam();

    imsa_ = new (std::nothrow) InputMethodSystemAbility();
    if (imsa_ == nullptr) {
        return;
    }
    imsa_->OnStart();
    imsa_->userId_ = TddUtil::GetCurrentUserId();
    imsa_->identityChecker_ = std::make_shared<IdentityCheckerMock>();

    inputMethodAbility_ = InputMethodAbility::GetInstance();
    inputMethodAbility_->abilityManager_ = imsa_;
    TddUtil::InitCurrentImePermissionInfo();
    IdentityCheckerMock::SetBundleName(TddUtil::currentBundleNameMock_);
    inputMethodAbility_->SetCoreAndAgent();
    kbListener_ = std::make_shared<KeyboardListenerImpl>();
    imeListener_ = std::make_shared<InputMethodEngineListenerImpl>();
    inputMethodAbility_->SetKdListener(kbListener_);
    inputMethodAbility_->SetImeListener(imeListener_);

    textListener_ = new TextListener();
    inputMethodController_ = InputMethodController::GetInstance();
    inputMethodController_->abilityManager_ = imsa_;

    keyEvent_ = MMI::KeyEvent::Create();
    constexpr int32_t keyAction = 2;
    constexpr int32_t keyCode = 2001;
    keyEvent_->SetKeyAction(keyAction);
    keyEvent_->SetKeyCode(keyCode);
    IdentityCheckerMock::SetFocused(true);
    int32_t ret = InputMethodEditorTest::inputMethodController_->Attach(InputMethodEditorTest::textListener_, false);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = InputMethodEditorTest::inputMethodController_->Close();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    IdentityCheckerMock::SetFocused(false);
    TextListener::ResetParam();
}

void InputMethodEditorTest::TearDownTestCase(void)
{
    IMSA_HILOGI("InputMethodEditorTest::TearDownTestCase");
    TextListener::ResetParam();
    IdentityCheckerMock::ResetParam();
    imsa_->OnStop();
}

void InputMethodEditorTest::SetUp(void)
{
    IMSA_HILOGI("InputMethodEditorTest::SetUp");
    TaskManager::GetInstance().SetInited(true);
}

void InputMethodEditorTest::TearDown(void)
{
    IMSA_HILOGI("InputMethodEditorTest::TearDown");
    BlockRetry(waitTaskEmptyinterval_, waitTaskEmptyTimes_, IsTaskEmpty);
    TaskManager::GetInstance().Reset();
}

/**
 * @tc.name: testIMCAttachUnfocused
 * @tc.desc: InputMethodEditorTest Attach.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodEditorTest, testIMCAttachUnfocused, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodEditorTest Attach Unfocused Test START");
    int32_t ret = InputMethodEditorTest::inputMethodController_->Attach(InputMethodEditorTest::textListener_, false);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_FOCUSED);
    ret = InputMethodEditorTest::inputMethodController_->Attach(InputMethodEditorTest::textListener_);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_FOCUSED);
    ret = InputMethodEditorTest::inputMethodController_->Attach(InputMethodEditorTest::textListener_, true);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_FOCUSED);
}

/**
 * @tc.name: test Unfocused
 * @tc.desc: InputMethodEditorTest Unfocused
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodEditorTest, testUnfocused, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodEditorTest Unfocused Test START");
    int32_t ret = InputMethodEditorTest::inputMethodController_->ShowTextInput();
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_BOUND);
    ret = InputMethodEditorTest::inputMethodController_->DispatchKeyEvent(
        InputMethodEditorTest::keyEvent_, [](std::shared_ptr<MMI::KeyEvent> &keyEvent, bool isConsumed) {});
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_EDITABLE);
    ret = InputMethodEditorTest::inputMethodController_->ShowSoftKeyboard();
    EXPECT_EQ(ret, ErrorCode::ERROR_STATUS_PERMISSION_DENIED);
    ret = InputMethodEditorTest::inputMethodController_->HideSoftKeyboard();
    EXPECT_EQ(ret, ErrorCode::ERROR_STATUS_PERMISSION_DENIED);
    ret = InputMethodEditorTest::inputMethodController_->StopInputSession();
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_FOCUSED);
    ret = InputMethodEditorTest::inputMethodController_->ShowCurrentInput();
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_EDITABLE);
    ret = InputMethodEditorTest::inputMethodController_->HideCurrentInput();
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_EDITABLE);
    ret = InputMethodEditorTest::inputMethodController_->OnCursorUpdate({});
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_BOUND);
    ret = InputMethodEditorTest::inputMethodController_->OnSelectionChange(Str8ToStr16(""), 0, 0);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_BOUND);
    ret = InputMethodEditorTest::inputMethodController_->SetCallingWindow(1);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_BOUND);
    ret = InputMethodEditorTest::inputMethodController_->OnConfigurationChange({});
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_BOUND);
}

/**
 * @tc.name: testRequestInput001.
 * @tc.desc: InputMethodEditorTest RequestShowInput/RequestHideInput neither permitted nor focused.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodEditorTest, testRequestInput001, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodEditorTest testRequestInput001 Test START");
    int32_t ret = InputMethodEditorTest::inputMethodController_->RequestShowInput();
    EXPECT_EQ(ret, ErrorCode::ERROR_STATUS_PERMISSION_DENIED);
    ret = InputMethodEditorTest::inputMethodController_->RequestHideInput();
    EXPECT_EQ(ret, ErrorCode::ERROR_STATUS_PERMISSION_DENIED);
}

/**
 * @tc.name: testRequestInput002.
 * @tc.desc: InputMethodEditorTest RequestShowInput/RequestHideInput with permitted and not focused.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodEditorTest, testRequestInput002, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodEditorTest testRequestInput002 Test START");
    IdentityCheckerMock::SetPermission(true);
    int32_t ret = InputMethodEditorTest::inputMethodController_->RequestShowInput();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = InputMethodEditorTest::inputMethodController_->RequestHideInput();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    IdentityCheckerMock::SetPermission(false);
}

/**
 * @tc.name: test AttachFocused
 * @tc.desc: InputMethodEditorTest Attach Focused
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodEditorTest, testAttachFocused, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodEditorTest Attach Focused Test START");
    IdentityCheckerMock::SetFocused(true);
    InputMethodEditorTest::imeListener_->isInputStart_ = false;
    InputMethodEditorTest::imeListener_->keyboardState_ = false;
    int32_t ret = InputMethodEditorTest::inputMethodController_->Attach(InputMethodEditorTest::textListener_, false);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    std::this_thread::sleep_for(std::chrono::seconds(2));

    InputMethodEditorTest::imeListener_->isInputStart_ = false;
    InputMethodEditorTest::imeListener_->keyboardState_ = false;
    ret = InputMethodEditorTest::inputMethodController_->Attach(InputMethodEditorTest::textListener_);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    EXPECT_EQ(TextListener::keyboardStatus_, KeyboardStatus::SHOW);
    EXPECT_TRUE(InputMethodEngineListenerImpl::keyboardState_);

    InputMethodEditorTest::imeListener_->isInputStart_ = false;
    InputMethodEditorTest::imeListener_->keyboardState_ = false;
    ret = InputMethodEditorTest::inputMethodController_->Attach(InputMethodEditorTest::textListener_, true);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    EXPECT_EQ(TextListener::keyboardStatus_, KeyboardStatus::SHOW);
    EXPECT_TRUE(InputMethodEngineListenerImpl::keyboardState_);

    InputMethodEditorTest::inputMethodController_->Close();
    IdentityCheckerMock::SetFocused(false);
}

/**
 * @tc.name: testShowSoftKeyboard
 * @tc.desc: InputMethodEditorTest ShowSoftKeyboard
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodEditorTest, testShowSoftKeyboard, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodEditorTest ShowSoftKeyboard Test START");
    IdentityCheckerMock::SetFocused(true);
    IdentityCheckerMock::SetPermission(true);
    InputMethodEditorTest::imeListener_->keyboardState_ = false;
    TextListener::ResetParam();
    int32_t ret = InputMethodEditorTest::inputMethodController_->Attach(InputMethodEditorTest::textListener_, false);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ret = InputMethodEditorTest::inputMethodController_->ShowSoftKeyboard();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    EXPECT_TRUE(imeListener_->keyboardState_ && TextListener::WaitSendKeyboardStatusCallback(KeyboardStatus::SHOW));
    InputMethodEditorTest::inputMethodController_->Close();
    IdentityCheckerMock::SetFocused(false);
    IdentityCheckerMock::SetPermission(false);
    BlockRetry(waitTaskEmptyinterval_, waitTaskEmptyTimes_, IsTaskEmpty);
}

/**
 * @tc.name: testIMCHideTextInput.
 * @tc.desc: InputMethodEditorTest testHideTextInput.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodEditorTest, testIMCHideTextInput, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodEditorTest HideTextInputAndShowTextInput Test START");
    IdentityCheckerMock::SetFocused(true);
    IdentityCheckerMock::SetPermission(true);
    int32_t ret = InputMethodEditorTest::inputMethodController_->Attach(InputMethodEditorTest::textListener_, true);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    imeListener_->keyboardState_ = true;
    InputMethodEditorTest::inputMethodController_->HideTextInput();
    ret = InputMethodEditorTest::inputMethodController_->DispatchKeyEvent(
        InputMethodEditorTest::keyEvent_, [](std::shared_ptr<MMI::KeyEvent> &keyEvent, bool isConsumed) {});
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_EDITABLE);
    ret = InputMethodEditorTest::inputMethodController_->ShowSoftKeyboard();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = InputMethodEditorTest::inputMethodController_->HideSoftKeyboard();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = InputMethodEditorTest::inputMethodController_->ShowCurrentInput();
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_EDITABLE);
    ret = InputMethodEditorTest::inputMethodController_->HideCurrentInput();
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_EDITABLE);
    ret = InputMethodEditorTest::inputMethodController_->OnCursorUpdate({});
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_EDITABLE);
    ret = InputMethodEditorTest::inputMethodController_->OnSelectionChange(Str8ToStr16(""), 0, 0);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_EDITABLE);
    ret = InputMethodEditorTest::inputMethodController_->SetCallingWindow(1);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_EDITABLE);
    ret = InputMethodEditorTest::inputMethodController_->OnConfigurationChange({});
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_EDITABLE);
    InputMethodEditorTest::inputMethodController_->Close();
    IdentityCheckerMock::SetFocused(false);
    IdentityCheckerMock::SetPermission(false);
    BlockRetry(waitTaskEmptyinterval_, waitTaskEmptyTimes_, IsTaskEmpty);
}

/**
 * @tc.name: testIMCDeactivateClient.
 * @tc.desc: InputMethodEditorTest testIMCDeactivateClient.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodEditorTest, testIMCDeactivateClient, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodEditorTest testIMCDeactivateClient Test START");
    IdentityCheckerMock::SetFocused(true);
    IdentityCheckerMock::SetPermission(true);
    int32_t ret = inputMethodController_->Attach(InputMethodEditorTest::textListener_, true);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    inputMethodController_->DeactivateClient();
    ret = inputMethodController_->DispatchKeyEvent(
        InputMethodEditorTest::keyEvent_, [](std::shared_ptr<MMI::KeyEvent> &keyEvent, bool isConsumed) {});
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_EDITABLE);
    ret = inputMethodController_->ShowTextInput();
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_BOUND);
    ret = inputMethodController_->HideTextInput();
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_BOUND);
    ret = inputMethodController_->ShowSoftKeyboard();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = inputMethodController_->HideSoftKeyboard();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = inputMethodController_->ShowCurrentInput();
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_EDITABLE);
    ret = inputMethodController_->HideCurrentInput();
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_EDITABLE);
    ret = inputMethodController_->OnCursorUpdate({});
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_BOUND);
    ret = inputMethodController_->OnSelectionChange(Str8ToStr16(""), 0, 0);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_BOUND);
    ret = inputMethodController_->SetCallingWindow(1);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_BOUND);
    ret = inputMethodController_->OnConfigurationChange({});
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_BOUND);

    ret = inputMethodController_->Attach(InputMethodEditorTest::textListener_, true);
    ret = inputMethodController_->ShowTextInput();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodEditorTest::inputMethodController_->Close();
    IdentityCheckerMock::SetFocused(false);
    IdentityCheckerMock::SetPermission(false);
    BlockRetry(waitTaskEmptyinterval_, waitTaskEmptyTimes_, IsTaskEmpty);
}

/**
 * @tc.name: testShowTextInput
 * @tc.desc: InputMethodEditorTest ShowTextInput
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodEditorTest, testShowTextInput, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodEditorTest ShowTextInput Test START");
    IdentityCheckerMock::SetFocused(true);
    int32_t ret = InputMethodEditorTest::inputMethodController_->Attach(InputMethodEditorTest::textListener_, true);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodEditorTest::inputMethodController_->HideTextInput();

    ret = InputMethodEditorTest::inputMethodController_->ShowTextInput();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    bool consumeResult = false;
    ret = InputMethodEditorTest::inputMethodController_->DispatchKeyEvent(InputMethodEditorTest::keyEvent_,
        [&consumeResult](std::shared_ptr<MMI::KeyEvent> &keyEvent, bool isConsumed) { consumeResult = isConsumed; });
    usleep(1000);
    ret =
        ret && kbListener_->keyCode_ == keyEvent_->GetKeyCode() && kbListener_->keyStatus_ == keyEvent_->GetKeyAction();
    EXPECT_TRUE(consumeResult);
    InputMethodEditorTest::inputMethodController_->Close();
    IdentityCheckerMock::SetFocused(false);
    BlockRetry(waitTaskEmptyinterval_, waitTaskEmptyTimes_, IsTaskEmpty);
}

/**
 * @tc.name: testIMCClose.
 * @tc.desc: InputMethodEditorTest Close.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodEditorTest, testIMCClose, TestSize.Level1)
{
    IMSA_HILOGI("IMC Close Test START");
    IdentityCheckerMock::SetFocused(true);
    IdentityCheckerMock::SetPermission(true);
    int32_t ret = InputMethodEditorTest::inputMethodController_->Attach(InputMethodEditorTest::textListener_, true);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodEditorTest::inputMethodController_->Close();

    ret = InputMethodEditorTest::inputMethodController_->ShowTextInput();
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_BOUND);
    ret = InputMethodEditorTest::inputMethodController_->DispatchKeyEvent(
        InputMethodEditorTest::keyEvent_, [](std::shared_ptr<MMI::KeyEvent> &keyEvent, bool isConsumed) {});
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_EDITABLE);
    ret = InputMethodEditorTest::inputMethodController_->ShowSoftKeyboard();
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_FOUND);
    ret = InputMethodEditorTest::inputMethodController_->HideSoftKeyboard();
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_FOUND);
    ret = InputMethodEditorTest::inputMethodController_->ShowCurrentInput();
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_EDITABLE);
    ret = InputMethodEditorTest::inputMethodController_->HideCurrentInput();
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_EDITABLE);
    ret = InputMethodEditorTest::inputMethodController_->OnCursorUpdate({});
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_BOUND);
    ret = InputMethodEditorTest::inputMethodController_->OnSelectionChange(Str8ToStr16(""), 0, 0);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_BOUND);
    ret = InputMethodEditorTest::inputMethodController_->SetCallingWindow(1);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_BOUND);
    ret = InputMethodEditorTest::inputMethodController_->OnConfigurationChange({});
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_BOUND);
    IdentityCheckerMock::SetFocused(false);
    IdentityCheckerMock::SetPermission(false);
    BlockRetry(waitTaskEmptyinterval_, waitTaskEmptyTimes_, IsTaskEmpty);
}

/**
 * @tc.name: testRequestShowInput.
 * @tc.desc: InputMethodEditorTest testRequestShowInput with focused.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodEditorTest, testRequestShowInput, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodEditorTest testRequestShowInput Test START");
    IdentityCheckerMock::SetFocused(true);
    imeListener_->keyboardState_ = false;
    int32_t ret = InputMethodEditorTest::inputMethodController_->RequestShowInput();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(InputMethodEngineListenerImpl::WaitKeyboardStatus(true));
    IdentityCheckerMock::SetFocused(false);
}

/**
 * @tc.name: testRequestHideInput_001.
 * @tc.desc: InputMethodEditorTest testRequestHideInput with focused.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodEditorTest, testRequestHideInput_001, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodEditorTest testRequestHideInput_001 Test START");
    IdentityCheckerMock::SetFocused(true);
    imeListener_->keyboardState_ = true;
    int32_t ret = InputMethodEditorTest::inputMethodController_->RequestHideInput();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    IdentityCheckerMock::SetFocused(false);
}

/**
 * @tc.name: testRequestHideInput_002.
 * @tc.desc: InputMethodEditorTest testRequestHideInput with focused.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodEditorTest, testRequestHideInput_002, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodEditorTest testRequestHideInput_002 Test START");
    IdentityCheckerMock::SetFocused(true);
    int32_t ret = InputMethodEditorTest::inputMethodController_->Attach(InputMethodEditorTest::textListener_, false);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    std::this_thread::sleep_for(std::chrono::seconds(2));

    imeListener_->keyboardState_ = true;
    ret = InputMethodEditorTest::inputMethodController_->RequestHideInput();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    EXPECT_FALSE(imeListener_->keyboardState_);
    InputMethodEditorTest::inputMethodController_->Close();
    IdentityCheckerMock::SetFocused(false);
}
} // namespace MiscServices
} // namespace OHOS
