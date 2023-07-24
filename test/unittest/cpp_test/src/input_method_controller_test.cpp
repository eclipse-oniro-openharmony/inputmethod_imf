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
#include "input_method_controller.h"
#undef private

#include <event_handler.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <string_ex.h>
#include <sys/time.h>

#include <condition_variable>
#include <csignal>
#include <cstdint>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "ability_manager_client.h"
#include "block_data.h"
#include "global.h"
#include "i_input_method_agent.h"
#include "i_input_method_system_ability.h"
#include "if_system_ability_manager.h"
#include "input_client_stub.h"
#include "input_data_channel_stub.h"
#include "input_death_recipient.h"
#include "input_method_ability.h"
#include "input_method_engine_listener_impl.h"
#include "input_method_system_ability_proxy.h"
#include "input_method_utils.h"
#include "iservice_registry.h"
#include "key_event_util.h"
#include "keyboard_listener.h"
#include "message_parcel.h"
#include "system_ability.h"
#include "system_ability_definition.h"
#include "tdd_util.h"
#include "text_listener.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace MiscServices {
constexpr uint32_t RETRY_TIME = 200 * 1000;
constexpr uint32_t RETRY_TIMES = 5;
using WindowMgr = TddUtil::WindowManager;

class SelectListenerMock : public ControllerListener {
public:
    SelectListenerMock() = default;
    ~SelectListenerMock() override = default;

    MOCK_METHOD2(OnSelectByRange, void(int32_t start, int32_t end));
    MOCK_METHOD1(OnSelectByMovement, void(int32_t direction));
};

class InputMethodControllerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static void SetInputDeathRecipient();
    static void OnRemoteSaDied(const wptr<IRemoteObject> &remote);
    static bool CheckKeyEvent(std::shared_ptr<MMI::KeyEvent> keyEvent);
    static bool WaitRemoteDiedCallback();
    static void WaitKeyboardStatusCallback(bool keyboardState);
    static void TriggerConfigurationChangeCallback(Configuration &info);
    static void TriggerCursorUpdateCallback(CursorInfo &info);
    static void TriggerSelectionChangeCallback(std::u16string &text, int start, int end);
    static void CheckProxyObject();
    static sptr<InputMethodController> inputMethodController_;
    static sptr<InputMethodAbility> inputMethodAbility_;
    static std::shared_ptr<MMI::KeyEvent> keyEvent_;
    static std::shared_ptr<InputMethodEngineListenerImpl> imeListener_;
    static std::shared_ptr<SelectListenerMock> controllerListener_;
    static sptr<OnTextChangedListener> textListener_;
    static std::mutex keyboardListenerMutex_;
    static std::condition_variable keyboardListenerCv_;
    static BlockData<std::shared_ptr<MMI::KeyEvent>> blockKeyEvent_;
    static BlockData<std::shared_ptr<MMI::KeyEvent>> blockFullKeyEvent_;
    static std::mutex onRemoteSaDiedMutex_;
    static std::condition_variable onRemoteSaDiedCv_;
    static sptr<InputDeathRecipient> deathRecipient_;
    static CursorInfo cursorInfo_;
    static int32_t oldBegin_;
    static int32_t oldEnd_;
    static int32_t newBegin_;
    static int32_t newEnd_;
    static std::string text_;
    static bool doesKeyEventConsume_;
    static bool doesFUllKeyEventConsume_;
    static InputAttribute inputAttribute_;
    static std::shared_ptr<AppExecFwk::EventHandler> textConfigHandler_;
    static constexpr uint32_t DELAY_TIME = 1;
    static constexpr uint32_t KEY_EVENT_DELAY_TIME = 100;
    static constexpr int32_t TASK_DELAY_TIME = 10;

    class KeyboardListenerImpl : public KeyboardListener {
    public:
        KeyboardListenerImpl()
        {
            std::shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create("InputMethodControllerTe"
                                                                                              "st");
            textConfigHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
        };
        ~KeyboardListenerImpl(){};
        bool OnKeyEvent(int32_t keyCode, int32_t keyStatus) override
        {
            if (!doesKeyEventConsume_) {
                return false;
            }
            IMSA_HILOGI("KeyboardListenerImpl::OnKeyEvent %{public}d %{public}d", keyCode, keyStatus);
            auto keyEvent = KeyEventUtil::CreateKeyEvent(keyCode, keyStatus);
            blockKeyEvent_.SetValue(keyEvent);
            return true;
        }
        bool OnKeyEvent(const std::shared_ptr<MMI::KeyEvent> &keyEvent) override
        {
            if (!doesFUllKeyEventConsume_) {
                return false;
            }
            IMSA_HILOGI("KeyboardListenerImpl::OnKeyEvent %{public}d %{public}d", keyEvent->GetKeyCode(),
                keyEvent->GetKeyAction());
            auto fullKey = keyEvent;
            blockFullKeyEvent_.SetValue(fullKey);
            return true;
        }
        void OnCursorUpdate(int32_t positionX, int32_t positionY, int32_t height) override
        {
            IMSA_HILOGD(
                "KeyboardListenerImpl::OnCursorUpdate %{public}d %{public}d %{public}d", positionX, positionY, height);
            cursorInfo_ = { static_cast<double>(positionX), static_cast<double>(positionY), 0,
                static_cast<double>(height) };
            InputMethodControllerTest::keyboardListenerCv_.notify_one();
        }
        void OnSelectionChange(int32_t oldBegin, int32_t oldEnd, int32_t newBegin, int32_t newEnd) override
        {
            IMSA_HILOGD("KeyboardListenerImpl::OnSelectionChange %{public}d %{public}d %{public}d %{public}d",
                oldBegin, oldEnd, newBegin, newBegin);
            oldBegin_ = oldBegin;
            oldEnd_ = oldEnd;
            newBegin_ = newBegin;
            newEnd_ = newEnd;
            InputMethodControllerTest::keyboardListenerCv_.notify_one();
        }
        void OnTextChange(const std::string &text) override
        {
            IMSA_HILOGD("KeyboardListenerImpl::OnTextChange text: %{public}s", text.c_str());
            text_ = text;
            InputMethodControllerTest::keyboardListenerCv_.notify_one();
        }
        void OnEditorAttributeChange(const InputAttribute &inputAttribute) override
        {
            IMSA_HILOGD("KeyboardListenerImpl in.");
            inputAttribute_ = inputAttribute;
            InputMethodControllerTest::keyboardListenerCv_.notify_one();
        }
    };
};
sptr<InputMethodController> InputMethodControllerTest::inputMethodController_;
sptr<InputMethodAbility> InputMethodControllerTest::inputMethodAbility_;
std::shared_ptr<MMI::KeyEvent> InputMethodControllerTest::keyEvent_;
std::shared_ptr<InputMethodEngineListenerImpl> InputMethodControllerTest::imeListener_;
std::shared_ptr<SelectListenerMock> InputMethodControllerTest::controllerListener_;
sptr<OnTextChangedListener> InputMethodControllerTest::textListener_;
CursorInfo InputMethodControllerTest::cursorInfo_ = {};
int32_t InputMethodControllerTest::oldBegin_ = 0;
int32_t InputMethodControllerTest::oldEnd_ = 0;
int32_t InputMethodControllerTest::newBegin_ = 0;
int32_t InputMethodControllerTest::newEnd_ = 0;
std::string InputMethodControllerTest::text_;
InputAttribute InputMethodControllerTest::inputAttribute_;
std::mutex InputMethodControllerTest::keyboardListenerMutex_;
std::condition_variable InputMethodControllerTest::keyboardListenerCv_;
sptr<InputDeathRecipient> InputMethodControllerTest::deathRecipient_;
std::mutex InputMethodControllerTest::onRemoteSaDiedMutex_;
std::condition_variable InputMethodControllerTest::onRemoteSaDiedCv_;
BlockData<std::shared_ptr<MMI::KeyEvent>> InputMethodControllerTest::blockKeyEvent_{
    InputMethodControllerTest::KEY_EVENT_DELAY_TIME, nullptr
};
BlockData<std::shared_ptr<MMI::KeyEvent>> InputMethodControllerTest::blockFullKeyEvent_{
    InputMethodControllerTest::KEY_EVENT_DELAY_TIME, nullptr
};
bool InputMethodControllerTest::doesKeyEventConsume_{ false };
bool InputMethodControllerTest::doesFUllKeyEventConsume_{ false };
std::shared_ptr<AppExecFwk::EventHandler> InputMethodControllerTest::textConfigHandler_{ nullptr };

void InputMethodControllerTest::SetUpTestCase(void)
{
    IMSA_HILOGI("InputMethodControllerTest::SetUpTestCase");
    TddUtil::StorageSelfTokenID();
    // Set the tokenID to the tokenID of the current ime
    std::shared_ptr<Property> property = InputMethodController::GetInstance()->GetCurrentInputMethod();
    std::string bundleName = property != nullptr ? property->name : "default.inputmethod.unittest";
    TddUtil::SetTestTokenID(TddUtil::GetTestTokenID(bundleName));
    inputMethodAbility_ = InputMethodAbility::GetInstance();
    inputMethodAbility_->SetCoreAndAgent();
    inputMethodAbility_->OnImeReady();
    imeListener_ = std::make_shared<InputMethodEngineListenerImpl>();
    controllerListener_ = std::make_shared<SelectListenerMock>();
    textListener_ = new TextListener();
    inputMethodAbility_->SetKdListener(std::make_shared<KeyboardListenerImpl>());
    inputMethodAbility_->SetImeListener(imeListener_);
    inputMethodController_ = InputMethodController::GetInstance();

    keyEvent_ = KeyEventUtil::CreateKeyEvent(MMI::KeyEvent::KEYCODE_A, MMI::KeyEvent::KEY_ACTION_DOWN);
    keyEvent_->SetFunctionKey(MMI::KeyEvent::NUM_LOCK_FUNCTION_KEY, 0);
    keyEvent_->SetFunctionKey(MMI::KeyEvent::CAPS_LOCK_FUNCTION_KEY, 1);
    keyEvent_->SetFunctionKey(MMI::KeyEvent::SCROLL_LOCK_FUNCTION_KEY, 1);
    TddUtil::SetTestTokenID(TddUtil::AllocTestTokenID(false, true, "undefine"));

    WindowMgr::CreateWindow();
    WindowMgr::ShowWindow();
    SetInputDeathRecipient();
    TextListener::ResetParam();
}

void InputMethodControllerTest::TearDownTestCase(void)
{
    IMSA_HILOGI("InputMethodControllerTest::TearDownTestCase");
    TddUtil::RestoreSelfTokenID();
    TextListener::ResetParam();
    WindowMgr::HideWindow();
    WindowMgr::DestroyWindow();
}

void InputMethodControllerTest::SetUp(void)
{
    IMSA_HILOGI("InputMethodControllerTest::SetUp");
}

void InputMethodControllerTest::TearDown(void)
{
    IMSA_HILOGI("InputMethodControllerTest::TearDown");
}

void InputMethodControllerTest::SetInputDeathRecipient()
{
    IMSA_HILOGI("InputMethodControllerTest::SetInputDeathRecipient");
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        IMSA_HILOGI("InputMethodControllerTest, system ability manager is nullptr");
        return;
    }
    auto systemAbility = systemAbilityManager->GetSystemAbility(INPUT_METHOD_SYSTEM_ABILITY_ID, "");
    if (systemAbility == nullptr) {
        IMSA_HILOGI("InputMethodControllerTest, system ability is nullptr");
        return;
    }
    deathRecipient_ = new (std::nothrow) InputDeathRecipient();
    if (deathRecipient_ == nullptr) {
        IMSA_HILOGE("InputMethodControllerTest, new death recipient failed");
        return;
    }
    deathRecipient_->SetDeathRecipient([](const wptr<IRemoteObject> &remote) { OnRemoteSaDied(remote); });
    if ((systemAbility->IsProxyObject()) && (!systemAbility->AddDeathRecipient(deathRecipient_))) {
        IMSA_HILOGE("InputMethodControllerTest, failed to add death recipient.");
        return;
    }
}

void InputMethodControllerTest::OnRemoteSaDied(const wptr<IRemoteObject> &remote)
{
    IMSA_HILOGI("InputMethodControllerTest::OnRemoteSaDied");
    onRemoteSaDiedCv_.notify_one();
}

bool InputMethodControllerTest::WaitRemoteDiedCallback()
{
    IMSA_HILOGI("InputMethodControllerTest::WaitRemoteDiedCallback");
    std::unique_lock<std::mutex> lock(onRemoteSaDiedMutex_);
    // 2 means wait 2 seconds.
    return onRemoteSaDiedCv_.wait_for(lock, std::chrono::seconds(2)) != std::cv_status::timeout;
}

void InputMethodControllerTest::CheckProxyObject()
{
    for (uint32_t retryTimes = 0; retryTimes < RETRY_TIMES; ++retryTimes) {
        IMSA_HILOGI("times = %{public}d", retryTimes);
        sptr<ISystemAbilityManager> systemAbilityManager =
            SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (systemAbilityManager == nullptr) {
            IMSA_HILOGI("system ability manager is nullptr");
            continue;
        }
        auto systemAbility = systemAbilityManager->CheckSystemAbility(INPUT_METHOD_SYSTEM_ABILITY_ID);
        if (systemAbility != nullptr) {
            IMSA_HILOGI("CheckProxyObject success!");
            break;
        }
        usleep(RETRY_TIME);
    }
}

bool InputMethodControllerTest::CheckKeyEvent(std::shared_ptr<MMI::KeyEvent> keyEvent)
{
    bool ret = keyEvent->GetKeyCode() == keyEvent_->GetKeyCode();
    EXPECT_TRUE(ret);
    ret = keyEvent->GetKeyAction() == keyEvent_->GetKeyAction();
    EXPECT_TRUE(ret);
    ret = keyEvent->GetKeyIntention() == keyEvent_->GetKeyIntention();
    EXPECT_TRUE(ret);
    // check function key state
    ret = keyEvent->GetFunctionKey(MMI::KeyEvent::NUM_LOCK_FUNCTION_KEY)
          == keyEvent_->GetFunctionKey(MMI::KeyEvent::NUM_LOCK_FUNCTION_KEY);
    EXPECT_TRUE(ret);
    ret = keyEvent->GetFunctionKey(MMI::KeyEvent::CAPS_LOCK_FUNCTION_KEY)
          == keyEvent_->GetFunctionKey(MMI::KeyEvent::CAPS_LOCK_FUNCTION_KEY);
    EXPECT_TRUE(ret);
    ret = keyEvent->GetFunctionKey(MMI::KeyEvent::SCROLL_LOCK_FUNCTION_KEY)
          == keyEvent_->GetFunctionKey(MMI::KeyEvent::SCROLL_LOCK_FUNCTION_KEY);
    EXPECT_TRUE(ret);
    // check KeyItem
    ret = keyEvent->GetKeyItems().size() == keyEvent_->GetKeyItems().size();
    EXPECT_TRUE(ret);
    ret = keyEvent->GetKeyItem()->GetKeyCode() == keyEvent_->GetKeyItem()->GetKeyCode();
    EXPECT_TRUE(ret);
    ret = keyEvent->GetKeyItem()->GetDownTime() == keyEvent_->GetKeyItem()->GetDownTime();
    EXPECT_TRUE(ret);
    ret = keyEvent->GetKeyItem()->GetDeviceId() == keyEvent_->GetKeyItem()->GetDeviceId();
    EXPECT_TRUE(ret);
    ret = keyEvent->GetKeyItem()->IsPressed() == keyEvent_->GetKeyItem()->IsPressed();
    EXPECT_TRUE(ret);
    ret = keyEvent->GetKeyItem()->GetUnicode() == keyEvent_->GetKeyItem()->GetUnicode();
    EXPECT_TRUE(ret);
    return ret;
}

void InputMethodControllerTest::WaitKeyboardStatusCallback(bool keyboardState)
{
    std::unique_lock<std::mutex> lock(InputMethodEngineListenerImpl::imeListenerMutex_);
    InputMethodEngineListenerImpl::imeListenerCv_.wait_for(lock,
        std::chrono::seconds(InputMethodControllerTest::DELAY_TIME),
        [keyboardState] { return InputMethodEngineListenerImpl::keyboardState_ == keyboardState; });
}

void InputMethodControllerTest::TriggerConfigurationChangeCallback(Configuration &info)
{
    textConfigHandler_->PostTask(
        [info]() { inputMethodController_->OnConfigurationChange(info); }, InputMethodControllerTest::TASK_DELAY_TIME);
    {
        std::unique_lock<std::mutex> lock(InputMethodControllerTest::keyboardListenerMutex_);
        InputMethodControllerTest::keyboardListenerCv_.wait_for(
            lock, std::chrono::seconds(InputMethodControllerTest::DELAY_TIME), [&info] {
                return (static_cast<OHOS::MiscServices::TextInputType>(
                            InputMethodControllerTest::inputAttribute_.inputPattern)
                           == info.GetTextInputType())
                       && (static_cast<OHOS::MiscServices::EnterKeyType>(
                               InputMethodControllerTest::inputAttribute_.enterKeyType)
                           == info.GetEnterKeyType());
            });
    }
}

void InputMethodControllerTest::TriggerCursorUpdateCallback(CursorInfo &info)
{
    textConfigHandler_->PostTask(
        [info]() { inputMethodController_->OnCursorUpdate(info); }, InputMethodControllerTest::TASK_DELAY_TIME);
    {
        std::unique_lock<std::mutex> lock(InputMethodControllerTest::keyboardListenerMutex_);
        InputMethodControllerTest::keyboardListenerCv_.wait_for(lock,
            std::chrono::seconds(InputMethodControllerTest::DELAY_TIME),
            [&info] { return InputMethodControllerTest::cursorInfo_ == info; });
    }
}

void InputMethodControllerTest::TriggerSelectionChangeCallback(std::u16string &text, int start, int end)
{
    textConfigHandler_->PostTask([text, start, end]() { inputMethodController_->OnSelectionChange(text, start, end); },
        InputMethodControllerTest::TASK_DELAY_TIME);
    {
        std::unique_lock<std::mutex> lock(InputMethodControllerTest::keyboardListenerMutex_);
        InputMethodControllerTest::keyboardListenerCv_.wait_for(lock,
            std::chrono::seconds(InputMethodControllerTest::DELAY_TIME),
            [&text] { return InputMethodControllerTest::text_ == Str16ToStr8(text); });
    }
}

/**
     * @tc.name: testIMCAttach
     * @tc.desc: IMC Attach.
     * @tc.type: FUNC
     * @tc.require:
     */
HWTEST_F(InputMethodControllerTest, testIMCAttach, TestSize.Level0)
{
    IMSA_HILOGD("IMC Attach Test START");
    imeListener_->isInputStart_ = false;
    inputMethodController_->Attach(textListener_, false);
    inputMethodController_->Attach(textListener_);
    inputMethodController_->Attach(textListener_, true);
    EXPECT_TRUE(TextListener::WaitIMACallback());
    EXPECT_TRUE(imeListener_->isInputStart_ && imeListener_->keyboardState_);
}

/**
     * @tc.name: testIMCSetCallingWindow
     * @tc.desc: IMC SetCallingWindow.
     * @tc.type: FUNC
     * @tc.require:
     */
HWTEST_F(InputMethodControllerTest, testIMCSetCallingWindow, TestSize.Level0)
{
    IMSA_HILOGD("IMC SetCallingWindow Test START");
    uint32_t windowId = 3;
    inputMethodController_->SetCallingWindow(windowId);
    EXPECT_EQ(windowId, imeListener_->windowId_);
}

/**
     * @tc.name: testGetIMSAProxy
     * @tc.desc: Get Imsa Proxy.
     * @tc.type: FUNC
     */
HWTEST_F(InputMethodControllerTest, testGetIMSAProxy, TestSize.Level0)
{
    auto systemAbilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(systemAbilityManager != nullptr);
    auto systemAbility = systemAbilityManager->GetSystemAbility(INPUT_METHOD_SYSTEM_ABILITY_ID, "");
    EXPECT_TRUE(systemAbility != nullptr);
}

/**
     * @tc.name: testWriteReadIInputDataChannel
     * @tc.desc: Checkout IInputDataChannel.
     * @tc.type: FUNC
     */
HWTEST_F(InputMethodControllerTest, testWriteReadIInputDataChannel, TestSize.Level0)
{
    sptr<InputDataChannelStub> mInputDataChannel = new InputDataChannelStub();
    MessageParcel data;
    auto ret = data.WriteRemoteObject(mInputDataChannel->AsObject());
    EXPECT_TRUE(ret);
    auto remoteObject = data.ReadRemoteObject();
    sptr<IInputDataChannel> iface = iface_cast<IInputDataChannel>(remoteObject);
    EXPECT_TRUE(iface != nullptr);
}

/**
     * @tc.name: testIMCBindToIMSA
     * @tc.desc: Bind IMSA.
     * @tc.type: FUNC
     */
HWTEST_F(InputMethodControllerTest, testIMCBindToIMSA, TestSize.Level0)
{
    sptr<InputClientStub> mClient = new InputClientStub();
    MessageParcel data;
    auto ret = data.WriteRemoteObject(mClient->AsObject());
    EXPECT_TRUE(ret);
    auto remoteObject = data.ReadRemoteObject();
    sptr<IInputClient> iface = iface_cast<IInputClient>(remoteObject);
    EXPECT_TRUE(iface != nullptr);
}

/**
     * @tc.name: testIMCDispatchKeyEvent001
     * @tc.desc: test IMC DispatchKeyEvent with 'keyDown/KeyUP'.
     * @tc.type: FUNC
     * @tc.require:
     */
HWTEST_F(InputMethodControllerTest, testIMCDispatchKeyEvent001, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCDispatchKeyEvent001 Test START");
    doesKeyEventConsume_ = true;
    doesFUllKeyEventConsume_ = false;
    blockKeyEvent_.Clear(nullptr);
    bool ret = inputMethodController_->DispatchKeyEvent(keyEvent_);
    EXPECT_TRUE(ret);
    auto keyEvent = blockKeyEvent_.GetValue();
    EXPECT_NE(keyEvent, nullptr);
    ret = keyEvent->GetKeyCode() == keyEvent_->GetKeyCode() && keyEvent->GetKeyAction() == keyEvent_->GetKeyAction();
    EXPECT_TRUE(ret);
}

/**
     * @tc.name: testIMCDispatchKeyEvent002
     * @tc.desc: test IMC DispatchKeyEvent with 'keyEvent'.
     * @tc.type: FUNC
     * @tc.require:
     */
HWTEST_F(InputMethodControllerTest, testIMCDispatchKeyEvent002, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCDispatchKeyEvent002 Test START");
    doesKeyEventConsume_ = false;
    doesFUllKeyEventConsume_ = true;
    blockFullKeyEvent_.Clear(nullptr);
    bool ret = inputMethodController_->DispatchKeyEvent(keyEvent_);
    EXPECT_TRUE(ret);
    auto keyEvent = blockFullKeyEvent_.GetValue();
    EXPECT_NE(keyEvent, nullptr);
    EXPECT_TRUE(CheckKeyEvent(keyEvent));
}

/**
     * @tc.name: testIMCDispatchKeyEvent003
     * @tc.desc: test IMC DispatchKeyEvent with 'keyDown/KeyUP' and 'keyEvent'.
     * @tc.type: FUNC
     * @tc.require:
     */
HWTEST_F(InputMethodControllerTest, testIMCDispatchKeyEvent003, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCDispatchKeyEvent003 Test START");
    doesKeyEventConsume_ = true;
    doesFUllKeyEventConsume_ = true;
    blockKeyEvent_.Clear(nullptr);
    blockFullKeyEvent_.Clear(nullptr);
    bool ret = inputMethodController_->DispatchKeyEvent(keyEvent_);
    EXPECT_TRUE(ret);
    auto keyEvent = blockKeyEvent_.GetValue();
    auto keyFullEvent = blockFullKeyEvent_.GetValue();
    EXPECT_NE(keyEvent, nullptr);
    EXPECT_NE(keyFullEvent, nullptr);
    EXPECT_EQ(keyEvent->GetKeyCode(), keyEvent_->GetKeyCode());
    EXPECT_EQ(keyEvent->GetKeyAction(), keyEvent_->GetKeyAction());
    EXPECT_TRUE(CheckKeyEvent(keyFullEvent));
}

/**
     * @tc.name: testIMCOnCursorUpdate01
     * @tc.desc: Test update cursorInfo, call 'OnCursorUpdate' twice, if cursorInfo is the same,
     *           the second time will not get callback.
     * @tc.type: FUNC
     * @tc.require:
     * @tc.author: Zhaolinglan
     */
HWTEST_F(InputMethodControllerTest, testIMCOnCursorUpdate01, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCOnCursorUpdate01 Test START");
    auto ret = inputMethodController_->Attach(textListener_, false);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    CursorInfo info = { 1, 3, 0, 5 };
    InputMethodControllerTest::TriggerCursorUpdateCallback(info);
    EXPECT_EQ(InputMethodControllerTest::cursorInfo_, info);

    InputMethodControllerTest::cursorInfo_ = {};
    InputMethodControllerTest::TriggerCursorUpdateCallback(info);
    EXPECT_FALSE(InputMethodControllerTest::cursorInfo_ == info);
}

/**
     * @tc.name: testIMCOnCursorUpdate02
     * @tc.desc: Test update cursorInfo, 'Attach'->'OnCursorUpdate'->'Close'->'Attach'->'OnCursorUpdate',
     *           it will get callback two time.
     * @tc.type: FUNC
     * @tc.require:
     * @tc.author: Zhaolinglan
     */
HWTEST_F(InputMethodControllerTest, testIMCOnCursorUpdate02, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCOnCursorUpdate02 Test START");
    auto ret = inputMethodController_->Attach(textListener_, false);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    CursorInfo info = { 2, 4, 0, 6 };
    InputMethodControllerTest::TriggerCursorUpdateCallback(info);
    EXPECT_EQ(InputMethodControllerTest::cursorInfo_, info);

    InputMethodControllerTest::cursorInfo_ = {};
    ret = InputMethodControllerTest::inputMethodController_->Close();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = inputMethodController_->Attach(textListener_, false);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodControllerTest::TriggerCursorUpdateCallback(info);
    EXPECT_EQ(InputMethodControllerTest::cursorInfo_, info);
}

/**
     * @tc.name: testIMCOnSelectionChange01
     * @tc.desc: Test change selection, call 'OnSelectionChange' twice, if selection is the same,
     *           the second time will not get callback.
     * @tc.type: FUNC
     * @tc.require:
     * @tc.author: Zhaolinglan
     */
HWTEST_F(InputMethodControllerTest, testIMCOnSelectionChange01, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCOnSelectionChange01 Test START");
    auto ret = inputMethodController_->Attach(textListener_, false);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    std::u16string text = Str8ToStr16("testSelect");
    int start = 1;
    int end = 2;
    InputMethodControllerTest::TriggerSelectionChangeCallback(text, start, end);
    EXPECT_EQ(InputMethodControllerTest::text_, Str16ToStr8(text));

    InputMethodControllerTest::text_ = "";
    InputMethodControllerTest::TriggerSelectionChangeCallback(text, start, end);
    EXPECT_NE(InputMethodControllerTest::text_, Str16ToStr8(text));
}

/**
     * @tc.name: testIMCOnSelectionChange02
     * @tc.desc: Test change selection, 'Attach'->'OnSelectionChange'->'Close'->'Attach'->'OnSelectionChange',
     *           it will get callback two time.
     * @tc.type: FUNC
     * @tc.require:
     * @tc.author: Zhaolinglan
     */
HWTEST_F(InputMethodControllerTest, testIMCOnSelectionChange02, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCOnSelectionChange02 Test START");
    auto ret = inputMethodController_->Attach(textListener_, false);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    std::u16string text = Str8ToStr16("testSelect2");
    int start = 1;
    int end = 2;
    InputMethodControllerTest::TriggerSelectionChangeCallback(text, start, end);
    EXPECT_EQ(InputMethodControllerTest::text_, Str16ToStr8(text));

    InputMethodControllerTest::text_ = "";
    InputMethodControllerTest::inputMethodController_->Close();
    inputMethodController_->Attach(textListener_, false);
    InputMethodControllerTest::TriggerSelectionChangeCallback(text, start, end);
    EXPECT_EQ(InputMethodControllerTest::text_, Str16ToStr8(text));
}

/**
     * @tc.name: testShowTextInput
     * @tc.desc: IMC ShowTextInput
     * @tc.type: FUNC
     */
HWTEST_F(InputMethodControllerTest, testShowTextInput, TestSize.Level0)
{
    IMSA_HILOGI("IMC ShowTextInput Test START");
    TextListener::keyboardStatus_ = KeyboardStatus::NONE;
    inputMethodController_->ShowTextInput();
    EXPECT_TRUE(TextListener::WaitIMACallback());
    EXPECT_TRUE(TextListener::keyboardStatus_ == KeyboardStatus::SHOW);
}

/**
     * @tc.name: testShowSoftKeyboard
     * @tc.desc: IMC ShowSoftKeyboard
     * @tc.type: FUNC
     */
HWTEST_F(InputMethodControllerTest, testShowSoftKeyboard, TestSize.Level0)
{
    IMSA_HILOGI("IMC ShowSoftKeyboard Test START");
    imeListener_->keyboardState_ = false;
    TextListener::keyboardStatus_ = KeyboardStatus::NONE;
    int32_t ret = inputMethodController_->ShowSoftKeyboard();
    EXPECT_TRUE(TextListener::WaitIMACallback());
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(imeListener_->keyboardState_ && TextListener::keyboardStatus_ == KeyboardStatus::SHOW);
}

/**
     * @tc.name: testShowCurrentInput
     * @tc.desc: IMC ShowCurrentInput
     * @tc.type: FUNC
     */
HWTEST_F(InputMethodControllerTest, testShowCurrentInput, TestSize.Level0)
{
    IMSA_HILOGI("IMC ShowCurrentInput Test START");
    imeListener_->keyboardState_ = false;
    TextListener::keyboardStatus_ = KeyboardStatus::NONE;
    int32_t ret = inputMethodController_->ShowCurrentInput();
    EXPECT_TRUE(TextListener::WaitIMACallback());
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(imeListener_->keyboardState_ && TextListener::keyboardStatus_ == KeyboardStatus::SHOW);
}

/**
     * @tc.name: testIMCGetEnterKeyType
     * @tc.desc: IMC testGetEnterKeyType.
     * @tc.type: FUNC
     * @tc.require:
     */
HWTEST_F(InputMethodControllerTest, testIMCGetEnterKeyType, TestSize.Level0)
{
    IMSA_HILOGI("IMC GetEnterKeyType Test START");
    int32_t keyType;
    inputMethodController_->GetEnterKeyType(keyType);
    EXPECT_TRUE(keyType >= static_cast<int32_t>(EnterKeyType::UNSPECIFIED)
                && keyType <= static_cast<int32_t>(EnterKeyType::PREVIOUS));
}

/**
     * @tc.name: testIMCGetInputPattern
     * @tc.desc: IMC testGetInputPattern.
     * @tc.type: FUNC
     * @tc.require:
     */
HWTEST_F(InputMethodControllerTest, testIMCGetInputPattern, TestSize.Level0)
{
    IMSA_HILOGI("IMC GetInputPattern Test START");
    int32_t inputPattern;
    inputMethodController_->GetInputPattern(inputPattern);
    EXPECT_TRUE(inputPattern >= static_cast<int32_t>(TextInputType::NONE)
                && inputPattern <= static_cast<int32_t>(TextInputType::VISIBLE_PASSWORD));
}

/**
     * @tc.name: testOnEditorAttributeChanged
     * @tc.desc: IMC testOnEditorAttributeChanged.
     * @tc.type: FUNC
     * @tc.require:
     */
HWTEST_F(InputMethodControllerTest, testOnEditorAttributeChanged, TestSize.Level0)
{
    IMSA_HILOGI("IMC testOnEditorAttributeChanged Test START");
    auto ret = inputMethodController_->Attach(textListener_, false);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    Configuration info;
    info.SetEnterKeyType(EnterKeyType::GO);
    info.SetTextInputType(TextInputType::NUMBER);
    InputMethodControllerTest::TriggerConfigurationChangeCallback(info);
    EXPECT_EQ(InputMethodControllerTest::inputAttribute_.inputPattern, static_cast<int32_t>(info.GetTextInputType()));
    EXPECT_EQ(InputMethodControllerTest::inputAttribute_.enterKeyType, static_cast<int32_t>(info.GetEnterKeyType()));
}

/**
     * @tc.name: testHideSoftKeyboard
     * @tc.desc: IMC HideSoftKeyboard
     * @tc.type: FUNC
     */
HWTEST_F(InputMethodControllerTest, testHideSoftKeyboard, TestSize.Level0)
{
    IMSA_HILOGI("IMC HideSoftKeyboard Test START");
    imeListener_->keyboardState_ = true;
    TextListener::keyboardStatus_ = KeyboardStatus::NONE;
    int32_t ret = inputMethodController_->HideSoftKeyboard();
    EXPECT_TRUE(TextListener::WaitIMACallback());
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(!imeListener_->keyboardState_ && TextListener::keyboardStatus_ == KeyboardStatus::HIDE);
}

/**
     * @tc.name: testIMCHideCurrentInput
     * @tc.desc: IMC HideCurrentInput.
     * @tc.type: FUNC
     * @tc.require:
     */
HWTEST_F(InputMethodControllerTest, testIMCHideCurrentInput, TestSize.Level0)
{
    IMSA_HILOGI("IMC HideCurrentInput Test START");
    imeListener_->keyboardState_ = true;
    TextListener::keyboardStatus_ = KeyboardStatus::NONE;
    int32_t ret = inputMethodController_->HideCurrentInput();
    EXPECT_TRUE(TextListener::WaitIMACallback());
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(!imeListener_->keyboardState_ && TextListener::keyboardStatus_ == KeyboardStatus::HIDE);
}

/**
    * @tc.name: testIMCInputStopSession
    * @tc.desc: IMC testInputStopSession.
    * @tc.type: FUNC
    * @tc.require: issueI5U8FZ
    * @tc.author: Hollokin
    */
HWTEST_F(InputMethodControllerTest, testIMCInputStopSession, TestSize.Level0)
{
    IMSA_HILOGI("IMC StopInputSession Test START");
    imeListener_->keyboardState_ = true;
    TextListener::keyboardStatus_ = KeyboardStatus::NONE;
    int32_t ret = inputMethodController_->StopInputSession();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    WaitKeyboardStatusCallback(false);
    EXPECT_TRUE(!imeListener_->keyboardState_);
}

/**
     * @tc.name: testIMCHideTextInput.
     * @tc.desc: IMC testHideTextInput.
     * @tc.type: FUNC
     */
HWTEST_F(InputMethodControllerTest, testIMCHideTextInput, TestSize.Level0)
{
    IMSA_HILOGI("IMC HideTextInput Test START");
    imeListener_->keyboardState_ = true;
    TextListener::keyboardStatus_ = KeyboardStatus::NONE;
    inputMethodController_->HideTextInput();
    WaitKeyboardStatusCallback(false);
    EXPECT_TRUE(!imeListener_->keyboardState_);
}

/**
     * @tc.name: testSetControllerListener
     * @tc.desc: IMC SetControllerListener
     * @tc.type: FUNC
     */
HWTEST_F(InputMethodControllerTest, testSetControllerListener, TestSize.Level0)
{
    IMSA_HILOGI("IMC SetControllerListener Test START");
    inputMethodController_->SetControllerListener(controllerListener_);

    int32_t ret = inputMethodController_->Attach(textListener_, false);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_CALL(*controllerListener_, OnSelectByRange(Eq(1), Eq(2))).Times(1);
    inputMethodAbility_->SelectByRange(1, 2);

    Sequence s;
    EXPECT_CALL(*controllerListener_, OnSelectByMovement(Eq(static_cast<int32_t>(Direction::UP))))
        .Times(1)
        .InSequence(s);
    EXPECT_CALL(*controllerListener_, OnSelectByMovement(Eq(static_cast<int32_t>(Direction::DOWN))))
        .Times(1)
        .InSequence(s);
    EXPECT_CALL(*controllerListener_, OnSelectByMovement(Eq(static_cast<int32_t>(Direction::LEFT))))
        .Times(1)
        .InSequence(s);
    EXPECT_CALL(*controllerListener_, OnSelectByMovement(Eq(static_cast<int32_t>(Direction::RIGHT))))
        .Times(1)
        .InSequence(s);
    inputMethodAbility_->SelectByMovement(static_cast<int32_t>(Direction::UP));
    inputMethodAbility_->SelectByMovement(static_cast<int32_t>(Direction::DOWN));
    inputMethodAbility_->SelectByMovement(static_cast<int32_t>(Direction::LEFT));
    inputMethodAbility_->SelectByMovement(static_cast<int32_t>(Direction::RIGHT));
}

/**
     * @tc.name: testWasAttached
     * @tc.desc: IMC WasAttached
     * @tc.type: FUNC
     */
HWTEST_F(InputMethodControllerTest, testWasAttached, TestSize.Level0)
{
    IMSA_HILOGI("IMC WasAttached Test START");
    inputMethodController_->Close();
    bool result = inputMethodController_->WasAttached();
    EXPECT_FALSE(result);
    int32_t ret = inputMethodController_->Attach(textListener_, false);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    result = inputMethodController_->WasAttached();
    EXPECT_TRUE(result);
    inputMethodController_->Close();
}

/**
    * @tc.name: testWithoutEditableState
    * @tc.desc: IMC testWithoutEditableState
    * @tc.type: FUNC
    * @tc.require:
    */
HWTEST_F(InputMethodControllerTest, testWithoutEditableState, TestSize.Level0)
{
    IMSA_HILOGI("IMC WithouteEditableState Test START");
    auto ret = inputMethodController_->Attach(textListener_, false);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = inputMethodController_->HideTextInput();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    int32_t deleteForwardLength = 1;
    ret = inputMethodAbility_->DeleteForward(deleteForwardLength);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    usleep(100);
    EXPECT_NE(TextListener::deleteForwardLength_, deleteForwardLength);

    int32_t deleteBackwardLength = 2;
    ret = inputMethodAbility_->DeleteBackward(deleteBackwardLength);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    usleep(100);
    EXPECT_NE(TextListener::deleteBackwardLength_, deleteBackwardLength);

    std::string insertText = "t";
    ret = inputMethodAbility_->InsertText(insertText);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    usleep(100);
    EXPECT_NE(TextListener::insertText_, Str8ToStr16(insertText));

    constexpr int32_t funcKey = 1;
    ret = inputMethodAbility_->SendFunctionKey(funcKey);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    usleep(100);
    EXPECT_NE(TextListener::key_, funcKey);

    constexpr int32_t keyCode = 4;
    ret = inputMethodAbility_->MoveCursor(keyCode);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    usleep(100);
    EXPECT_NE(TextListener::direction_, keyCode);
}

/**
     * @tc.name: testOnRemoteDied
     * @tc.desc: IMC OnRemoteDied
     * @tc.type: FUNC
     */
HWTEST_F(InputMethodControllerTest, testOnRemoteDied, TestSize.Level0)
{
    IMSA_HILOGI("IMC OnRemoteDied Test START");
    int32_t ret = inputMethodController_->Attach(textListener_, true);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    pid_t pid = TddUtil::GetImsaPid();
    EXPECT_TRUE(pid > 0);
    ret = kill(pid, SIGTERM);
    EXPECT_EQ(ret, 0);
    EXPECT_TRUE(WaitRemoteDiedCallback());
    CheckProxyObject();
    inputMethodController_->OnRemoteSaDied(nullptr);
    EXPECT_TRUE(TextListener::WaitIMACallback());
    bool result = inputMethodController_->WasAttached();
    EXPECT_TRUE(result);
    inputMethodController_->Close();
}
} // namespace MiscServices
} // namespace OHOS
