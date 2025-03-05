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

#include "ime_enabled_info_manager.h"
#include "ime_info_inquirer.h"
#include "input_data_channel_stub.h"
#include "input_method_ability.h"
#include "input_method_system_ability.h"
#include "task_manager.h"
#undef private

#include <event_handler.h>
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
#include <unordered_map>
#include <vector>

#include "ability_manager_client.h"
#include "block_data.h"
#include "global.h"
#include "i_input_method_agent.h"
#include "i_input_method_system_ability.h"
#include "identity_checker_mock.h"
#include "if_system_ability_manager.h"
#include "input_client_stub.h"
#include "input_death_recipient.h"
#include "input_method_ability.h"
#include "input_method_engine_listener_impl.h"
#include "input_method_system_ability_proxy.h"
#include "input_method_utils.h"
#include "iservice_registry.h"
#include "key_event_util.h"
#include "keyboard_listener.h"
#include "message_parcel.h"
#include "scope_utils.h"
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
constexpr uint32_t WAIT_INTERVAL = 500;
constexpr size_t PRIVATE_COMMAND_SIZE_MAX = 32 * 1024;
constexpr uint32_t WAIT_SA_DIE_TIME_OUT = 3;
const std::string IME_KEY = "settings.inputmethod.enable_ime";

class SelectListenerMock : public ControllerListener {
public:
    SelectListenerMock() = default;
    ~SelectListenerMock() override = default;

    void OnSelectByRange(int32_t start, int32_t end) override
    {
        start_ = start;
        end_ = end;
        selectListenerCv_.notify_all();
    }

    void OnSelectByMovement(int32_t direction) override
    {
        direction_ = direction;
        selectListenerCv_.notify_all();
    }
    static void WaitSelectListenerCallback();
    static int32_t start_;
    static int32_t end_;
    static int32_t direction_;
    static std::mutex selectListenerMutex_;
    static std::condition_variable selectListenerCv_;
};

int32_t SelectListenerMock::start_ = 0;
int32_t SelectListenerMock::end_ = 0;
int32_t SelectListenerMock::direction_ = 0;
std::mutex SelectListenerMock::selectListenerMutex_;
std::condition_variable SelectListenerMock::selectListenerCv_;
void SelectListenerMock::WaitSelectListenerCallback()
{
    std::unique_lock<std::mutex> lock(selectListenerMutex_);
    selectListenerCv_.wait_for(lock, std::chrono::milliseconds(WAIT_INTERVAL));
}

class InputMethodControllerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static void SetInputDeathRecipient();
    static void OnRemoteSaDied(const wptr<IRemoteObject> &remote);
    static void CheckKeyEvent(std::shared_ptr<MMI::KeyEvent> keyEvent);
    static bool WaitRemoteDiedCallback();
    static bool WaitKeyboardStatus(bool keyboardState);
    static void TriggerConfigurationChangeCallback(Configuration &info);
    static void TriggerCursorUpdateCallback(CursorInfo &info);
    static void TriggerSelectionChangeCallback(std::u16string &text, int start, int end);
    static void CheckProxyObject();
    static void DispatchKeyEventCallback(std::shared_ptr<MMI::KeyEvent> &keyEvent, bool isConsumed);
    static bool WaitKeyEventCallback();
    static void CheckTextConfig(const TextConfig &config);
    static void ResetKeyboardListenerTextConfig();
    static sptr<InputMethodController> inputMethodController_;
    static sptr<InputMethodAbility> inputMethodAbility_;
    static sptr<InputMethodSystemAbility> imsa_;
    static sptr<InputMethodSystemAbilityProxy> imsaProxy_;
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
    static std::condition_variable keyEventCv_;
    static std::mutex keyEventLock_;
    static bool consumeResult_;
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
        ~KeyboardListenerImpl() {};
        bool OnKeyEvent(int32_t keyCode, int32_t keyStatus, sptr<KeyEventConsumerProxy> &consumer) override
        {
            if (!doesKeyEventConsume_) {
                IMSA_HILOGI("KeyboardListenerImpl doesKeyEventConsume_ false");
                return false;
            }
            IMSA_HILOGI("KeyboardListenerImpl::OnKeyEvent %{public}d %{public}d", keyCode, keyStatus);
            auto keyEvent = KeyEventUtil::CreateKeyEvent(keyCode, keyStatus);
            blockKeyEvent_.SetValue(keyEvent);
            return true;
        }
        bool OnKeyEvent(const std::shared_ptr<MMI::KeyEvent> &keyEvent, sptr<KeyEventConsumerProxy> &consumer) override
        {
            if (!doesFUllKeyEventConsume_) {
                IMSA_HILOGI("KeyboardListenerImpl doesFUllKeyEventConsume_ false");
                return false;
            }
            IMSA_HILOGI("KeyboardListenerImpl::OnKeyEvent %{public}d %{public}d", keyEvent->GetKeyCode(),
                keyEvent->GetKeyAction());
            auto fullKey = keyEvent;
            blockFullKeyEvent_.SetValue(fullKey);
            return true;
        }
        bool OnDealKeyEvent(
            const std::shared_ptr<MMI::KeyEvent> &keyEvent, sptr<KeyEventConsumerProxy> &consumer) override
        {
            IMSA_HILOGI("KeyboardListenerImpl run in");
            bool isKeyCodeConsume = OnKeyEvent(keyEvent->GetKeyCode(), keyEvent->GetKeyAction(), consumer);
            bool isKeyEventConsume = OnKeyEvent(keyEvent, consumer);
            if (consumer != nullptr) {
                consumer->OnKeyEventResult(isKeyEventConsume | isKeyCodeConsume);
            }
            return true;
        }
        void OnCursorUpdate(int32_t positionX, int32_t positionY, int32_t height) override
        {
            IMSA_HILOGI("KeyboardListenerImpl %{public}d %{public}d %{public}d", positionX, positionY, height);
            cursorInfo_ = { static_cast<double>(positionX), static_cast<double>(positionY), 0,
                static_cast<double>(height) };
            InputMethodControllerTest::keyboardListenerCv_.notify_one();
        }
        void OnSelectionChange(int32_t oldBegin, int32_t oldEnd, int32_t newBegin, int32_t newEnd) override
        {
            IMSA_HILOGI(
                "KeyboardListenerImpl %{public}d %{public}d %{public}d %{public}d", oldBegin, oldEnd, newBegin, newEnd);
            oldBegin_ = oldBegin;
            oldEnd_ = oldEnd;
            newBegin_ = newBegin;
            newEnd_ = newEnd;
            InputMethodControllerTest::keyboardListenerCv_.notify_one();
        }
        void OnTextChange(const std::string &text) override
        {
            IMSA_HILOGI("KeyboardListenerImpl text: %{public}s", text.c_str());
            text_ = text;
            InputMethodControllerTest::keyboardListenerCv_.notify_one();
        }
        void OnEditorAttributeChange(const InputAttribute &inputAttribute) override
        {
            IMSA_HILOGI("KeyboardListenerImpl inputPattern: %{public}d, enterKeyType: %{public}d, "
                        "isTextPreviewSupported: %{public}d",
                inputAttribute.inputPattern, inputAttribute.enterKeyType, inputAttribute.isTextPreviewSupported);
            inputAttribute_ = inputAttribute;
            InputMethodControllerTest::keyboardListenerCv_.notify_one();
        }
    };
};
sptr<InputMethodController> InputMethodControllerTest::inputMethodController_;
sptr<InputMethodAbility> InputMethodControllerTest::inputMethodAbility_;
sptr<InputMethodSystemAbility> InputMethodControllerTest::imsa_;
sptr<InputMethodSystemAbilityProxy> InputMethodControllerTest::imsaProxy_;
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
BlockData<std::shared_ptr<MMI::KeyEvent>> InputMethodControllerTest::blockKeyEvent_ {
    InputMethodControllerTest::KEY_EVENT_DELAY_TIME, nullptr
};
BlockData<std::shared_ptr<MMI::KeyEvent>> InputMethodControllerTest::blockFullKeyEvent_ {
    InputMethodControllerTest::KEY_EVENT_DELAY_TIME, nullptr
};
bool InputMethodControllerTest::doesKeyEventConsume_ { false };
bool InputMethodControllerTest::doesFUllKeyEventConsume_ { false };
std::condition_variable InputMethodControllerTest::keyEventCv_;
std::mutex InputMethodControllerTest::keyEventLock_;
bool InputMethodControllerTest::consumeResult_ { false };
std::shared_ptr<AppExecFwk::EventHandler> InputMethodControllerTest::textConfigHandler_ { nullptr };

void InputMethodControllerTest::SetUpTestCase(void)
{
    IMSA_HILOGI("InputMethodControllerTest::SetUpTestCase");
    IdentityCheckerMock::ResetParam();
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

    inputMethodAbility_ = InputMethodAbility::GetInstance();
    inputMethodAbility_->abilityManager_ = imsaProxy_;
    TddUtil::InitCurrentImePermissionInfo();
    IdentityCheckerMock::SetBundleName(TddUtil::currentBundleNameMock_);
    inputMethodAbility_->SetCoreAndAgent();
    controllerListener_ = std::make_shared<SelectListenerMock>();
    textListener_ = new TextListener();
    inputMethodAbility_->SetKdListener(std::make_shared<KeyboardListenerImpl>());
    imeListener_ = std::make_shared<InputMethodEngineListenerImpl>(textConfigHandler_);
    inputMethodAbility_->SetImeListener(imeListener_);

    inputMethodController_ = InputMethodController::GetInstance();
    inputMethodController_->abilityManager_ = imsaProxy_;

    keyEvent_ = KeyEventUtil::CreateKeyEvent(MMI::KeyEvent::KEYCODE_A, MMI::KeyEvent::KEY_ACTION_DOWN);
    keyEvent_->SetFunctionKey(MMI::KeyEvent::NUM_LOCK_FUNCTION_KEY, 0);
    keyEvent_->SetFunctionKey(MMI::KeyEvent::CAPS_LOCK_FUNCTION_KEY, 1);
    keyEvent_->SetFunctionKey(MMI::KeyEvent::SCROLL_LOCK_FUNCTION_KEY, 1);

    SetInputDeathRecipient();
    TextListener::ResetParam();
}

void InputMethodControllerTest::TearDownTestCase(void)
{
    IMSA_HILOGI("InputMethodControllerTest::TearDownTestCase");
    TextListener::ResetParam();
    inputMethodController_->SetControllerListener(nullptr);
    IdentityCheckerMock::ResetParam();
    imsa_->OnStop();
}

void InputMethodControllerTest::SetUp(void)
{
    IMSA_HILOGI("InputMethodControllerTest::SetUp");
    TaskManager::GetInstance().SetInited(true);
}

void InputMethodControllerTest::TearDown(void)
{
    IMSA_HILOGI("InputMethodControllerTest::TearDown");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    TaskManager::GetInstance().Reset();
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
    deathRecipient_->SetDeathRecipient([](const wptr<IRemoteObject> &remote) {
        OnRemoteSaDied(remote);
    });
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
    return onRemoteSaDiedCv_.wait_for(lock, std::chrono::seconds(WAIT_SA_DIE_TIME_OUT)) != std::cv_status::timeout;
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

void InputMethodControllerTest::CheckKeyEvent(std::shared_ptr<MMI::KeyEvent> keyEvent)
{
    ASSERT_NE(keyEvent, nullptr);
    bool ret = keyEvent->GetKeyCode() == keyEvent_->GetKeyCode();
    EXPECT_TRUE(ret);
    ret = keyEvent->GetKeyAction() == keyEvent_->GetKeyAction();
    EXPECT_TRUE(ret);
    ret = keyEvent->GetKeyIntention() == keyEvent_->GetKeyIntention();
    EXPECT_TRUE(ret);
    // check function key state
    ret = keyEvent->GetFunctionKey(MMI::KeyEvent::NUM_LOCK_FUNCTION_KEY) ==
        keyEvent_->GetFunctionKey(MMI::KeyEvent::NUM_LOCK_FUNCTION_KEY);
    EXPECT_TRUE(ret);
    ret = keyEvent->GetFunctionKey(MMI::KeyEvent::CAPS_LOCK_FUNCTION_KEY) ==
        keyEvent_->GetFunctionKey(MMI::KeyEvent::CAPS_LOCK_FUNCTION_KEY);
    EXPECT_TRUE(ret);
    ret = keyEvent->GetFunctionKey(MMI::KeyEvent::SCROLL_LOCK_FUNCTION_KEY) ==
        keyEvent_->GetFunctionKey(MMI::KeyEvent::SCROLL_LOCK_FUNCTION_KEY);
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
}

bool InputMethodControllerTest::WaitKeyboardStatus(bool keyboardState)
{
    return InputMethodEngineListenerImpl::WaitKeyboardStatus(keyboardState);
}

void InputMethodControllerTest::TriggerConfigurationChangeCallback(Configuration &info)
{
    textConfigHandler_->PostTask(
        [info]() {
            inputMethodController_->OnConfigurationChange(info);
        },
        InputMethodControllerTest::TASK_DELAY_TIME, AppExecFwk::EventQueue::Priority::VIP);
    {
        std::unique_lock<std::mutex> lock(InputMethodControllerTest::keyboardListenerMutex_);
        InputMethodControllerTest::keyboardListenerCv_.wait_for(
            lock, std::chrono::seconds(InputMethodControllerTest::DELAY_TIME), [&info] {
                return (static_cast<OHOS::MiscServices::TextInputType>(
                            InputMethodControllerTest::inputAttribute_.inputPattern) == info.GetTextInputType()) &&
                    (static_cast<OHOS::MiscServices::EnterKeyType>(
                         InputMethodControllerTest::inputAttribute_.enterKeyType) == info.GetEnterKeyType());
            });
    }
}

void InputMethodControllerTest::TriggerCursorUpdateCallback(CursorInfo &info)
{
    textConfigHandler_->PostTask(
        [info]() {
            inputMethodController_->OnCursorUpdate(info);
        },
        InputMethodControllerTest::TASK_DELAY_TIME, AppExecFwk::EventQueue::Priority::VIP);
    {
        std::unique_lock<std::mutex> lock(InputMethodControllerTest::keyboardListenerMutex_);
        InputMethodControllerTest::keyboardListenerCv_.wait_for(
            lock, std::chrono::seconds(InputMethodControllerTest::DELAY_TIME), [&info] {
                return InputMethodControllerTest::cursorInfo_ == info;
            });
    }
}

void InputMethodControllerTest::TriggerSelectionChangeCallback(std::u16string &text, int start, int end)
{
    IMSA_HILOGI("InputMethodControllerTest run in");
    textConfigHandler_->PostTask(
        [text, start, end]() {
            inputMethodController_->OnSelectionChange(text, start, end);
        },
        InputMethodControllerTest::TASK_DELAY_TIME, AppExecFwk::EventQueue::Priority::VIP);
    {
        std::unique_lock<std::mutex> lock(InputMethodControllerTest::keyboardListenerMutex_);
        InputMethodControllerTest::keyboardListenerCv_.wait_for(
            lock, std::chrono::seconds(InputMethodControllerTest::DELAY_TIME), [&text, start, end] {
                return InputMethodControllerTest::text_ == Str16ToStr8(text) &&
                    InputMethodControllerTest::newBegin_ == start && InputMethodControllerTest::newEnd_ == end;
            });
    }
    IMSA_HILOGI("InputMethodControllerTest end");
}

void InputMethodControllerTest::CheckTextConfig(const TextConfig &config)
{
    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_EQ(imeListener_->windowId_, config.windowId);
    EXPECT_EQ(cursorInfo_.left, config.cursorInfo.left);
    EXPECT_EQ(cursorInfo_.top, config.cursorInfo.top);
    EXPECT_EQ(cursorInfo_.height, config.cursorInfo.height);
    EXPECT_EQ(newBegin_, config.range.start);
    EXPECT_EQ(newEnd_, config.range.end);
    EXPECT_EQ(inputAttribute_.inputPattern, config.inputAttribute.inputPattern);
    EXPECT_EQ(inputAttribute_.enterKeyType, config.inputAttribute.enterKeyType);
}

void InputMethodControllerTest::DispatchKeyEventCallback(std::shared_ptr<MMI::KeyEvent> &keyEvent, bool isConsumed)
{
    IMSA_HILOGI("InputMethodControllerTest isConsumed: %{public}d", isConsumed);
    consumeResult_ = isConsumed;
    keyEventCv_.notify_one();
}

bool InputMethodControllerTest::WaitKeyEventCallback()
{
    std::unique_lock<std::mutex> lock(keyEventLock_);
    keyEventCv_.wait_for(lock, std::chrono::seconds(DELAY_TIME), [] {
        return consumeResult_;
    });
    return consumeResult_;
}

void InputMethodControllerTest::ResetKeyboardListenerTextConfig()
{
    cursorInfo_ = {};
    oldBegin_ = INVALID_VALUE;
    oldEnd_ = INVALID_VALUE;
    newBegin_ = INVALID_VALUE;
    newEnd_ = INVALID_VALUE;
    text_ = "";
    inputAttribute_ = {};
}

/**
 * @tc.name: testIMCAttach001
 * @tc.desc: IMC Attach.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodControllerTest, testIMCAttach001, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCAttach001 Test START");
    imeListener_->isInputStart_ = false;
    TextListener::ResetParam();
    inputMethodController_->Attach(textListener_, false);
    inputMethodController_->Attach(textListener_);
    inputMethodController_->Attach(textListener_, true);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    EXPECT_EQ(TextListener::keyboardStatus_, KeyboardStatus::SHOW);
    EXPECT_TRUE(imeListener_->isInputStart_ && imeListener_->keyboardState_);
}

/**
 * @tc.name: testIMCAttach002
 * @tc.desc: IMC Attach.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodControllerTest, testIMCAttach002, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCAttach002 Test START");
    TextListener::ResetParam();
    CursorInfo cursorInfo = { 1, 1, 1, 1 };
    Range selectionRange = { 1, 2 };
    InputAttribute attribute = { 1, 1 };
    uint32_t windowId = 10;
    TextConfig textConfig = {
        .inputAttribute = attribute, .cursorInfo = cursorInfo, .range = selectionRange, .windowId = windowId
    };

    inputMethodController_->Attach(textListener_, true, textConfig);
    InputMethodControllerTest::CheckTextConfig(textConfig);

    TextListener::ResetParam();
    cursorInfo = { 2, 2, 2, 2 };
    selectionRange = { 3, 4 };
    attribute = { 2, 2 };
    windowId = 11;
    textConfig = {
        .inputAttribute = attribute, .cursorInfo = cursorInfo, .range = selectionRange, .windowId = windowId
    };
    inputMethodController_->Attach(textListener_, true, textConfig);
    InputMethodControllerTest::CheckTextConfig(textConfig);
}

/**
 * @tc.name: testIMCSetCallingWindow
 * @tc.desc: IMC SetCallingWindow.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodControllerTest, testIMCSetCallingWindow, TestSize.Level0)
{
    IMSA_HILOGI("IMC SetCallingWindow Test START");
    auto ret = inputMethodController_->Attach(textListener_);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    uint32_t windowId = 3;
    ret = inputMethodController_->SetCallingWindow(windowId);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
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
    ASSERT_NE(systemAbilityManager, nullptr);
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
    auto res = inputMethodController_->Attach(textListener_);
    EXPECT_EQ(res, ErrorCode::NO_ERROR);

    bool ret = inputMethodController_->DispatchKeyEvent(keyEvent_, DispatchKeyEventCallback);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    EXPECT_TRUE(WaitKeyEventCallback());

    auto keyEvent = blockKeyEvent_.GetValue();
    ASSERT_NE(keyEvent, nullptr);
    EXPECT_EQ(keyEvent->GetKeyCode(), keyEvent_->GetKeyCode());
    EXPECT_EQ(keyEvent->GetKeyAction(), keyEvent_->GetKeyAction());

    doesKeyEventConsume_ = false;
    ret = inputMethodController_->DispatchKeyEvent(keyEvent_, DispatchKeyEventCallback);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    EXPECT_TRUE(!WaitKeyEventCallback());
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
    int32_t ret = inputMethodController_->DispatchKeyEvent(
        keyEvent_, [](std::shared_ptr<MMI::KeyEvent> &keyEvent, bool isConsumed) {});
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    auto keyEvent = blockFullKeyEvent_.GetValue();
    ASSERT_NE(keyEvent, nullptr);
    CheckKeyEvent(keyEvent);
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
    int32_t ret = inputMethodController_->DispatchKeyEvent(
        keyEvent_, [](std::shared_ptr<MMI::KeyEvent> &keyEvent, bool isConsumed) {});
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    auto keyEvent = blockKeyEvent_.GetValue();
    auto keyFullEvent = blockFullKeyEvent_.GetValue();
    ASSERT_NE(keyEvent, nullptr);
    EXPECT_EQ(keyEvent->GetKeyCode(), keyEvent_->GetKeyCode());
    EXPECT_EQ(keyEvent->GetKeyAction(), keyEvent_->GetKeyAction());
    ASSERT_NE(keyFullEvent, nullptr);
    CheckKeyEvent(keyFullEvent);
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
 * @tc.name: testIMCOnSelectionChange03
 * @tc.desc: Test change selection, 'Attach without config'->'OnSelectionChange(0, 0)', it will get callback.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: Zhaolinglan
 */
HWTEST_F(InputMethodControllerTest, testIMCOnSelectionChange03, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCOnSelectionChange03 Test START");
    InputMethodControllerTest::ResetKeyboardListenerTextConfig();
    InputMethodControllerTest::inputMethodController_->Close();
    auto ret = inputMethodController_->Attach(textListener_, false);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    std::u16string text = Str8ToStr16("");
    int start = 0;
    int end = 0;
    InputMethodControllerTest::TriggerSelectionChangeCallback(text, start, end);
    EXPECT_EQ(InputMethodControllerTest::text_, Str16ToStr8(text));
    EXPECT_EQ(InputMethodControllerTest::oldBegin_, INVALID_VALUE);
    EXPECT_EQ(InputMethodControllerTest::oldEnd_, INVALID_VALUE);
    EXPECT_EQ(InputMethodControllerTest::newBegin_, start);
    EXPECT_EQ(InputMethodControllerTest::newEnd_, end);
}

/**
 * @tc.name: testIMCOnSelectionChange04
 * @tc.desc: Test change selection, 'Attach with range(1, 1)'->'Attach witch range(2, 2)', it will get (1, 1, 2, 2).
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: Zhaolinglan
 */
HWTEST_F(InputMethodControllerTest, testIMCOnSelectionChange04, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCOnSelectionChange04 Test START");
    InputMethodControllerTest::ResetKeyboardListenerTextConfig();
    InputMethodControllerTest::inputMethodController_->Close();
    TextConfig textConfig;
    textConfig.range = { 1, 1 };
    auto ret = inputMethodController_->Attach(textListener_, false, textConfig);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    textConfig.range = { 2, 2 };
    ret = inputMethodController_->Attach(textListener_, false, textConfig);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    EXPECT_EQ(InputMethodControllerTest::oldBegin_, 1);
    EXPECT_EQ(InputMethodControllerTest::oldEnd_, 1);
    EXPECT_EQ(InputMethodControllerTest::newBegin_, 2);
    EXPECT_EQ(InputMethodControllerTest::newEnd_, 2);
}

/**
 * @tc.name: testIMCOnSelectionChange05
 * @tc.desc: 'Attach without range'-> it will get no selectionChange callback.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: Zhaolinglan
 */
HWTEST_F(InputMethodControllerTest, testIMCOnSelectionChange05, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCOnSelectionChange05 Test START");
    InputMethodControllerTest::ResetKeyboardListenerTextConfig();
    InputMethodControllerTest::inputMethodController_->Close();
    inputMethodController_->Attach(textListener_, false);
    EXPECT_EQ(InputMethodControllerTest::oldBegin_, INVALID_VALUE);
    EXPECT_EQ(InputMethodControllerTest::oldEnd_, INVALID_VALUE);
    EXPECT_EQ(InputMethodControllerTest::newBegin_, INVALID_VALUE);
    EXPECT_EQ(InputMethodControllerTest::newEnd_, INVALID_VALUE);
}

/**
 * @tc.name: testIMCOnSelectionChange06
 * @tc.desc: 'Update(1, 2)'->'Attach without range', it will get no selectionChange callback.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: Zhaolinglan
 */
HWTEST_F(InputMethodControllerTest, testIMCOnSelectionChange06, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCOnSelectionChange06 Test START");
    InputMethodControllerTest::inputMethodController_->Close();
    inputMethodController_->Attach(textListener_, false);
    std::u16string text = Str8ToStr16("");
    InputMethodControllerTest::TriggerSelectionChangeCallback(text, 1, 2);
    InputMethodControllerTest::ResetKeyboardListenerTextConfig();
    inputMethodController_->Attach(textListener_, false);
    EXPECT_EQ(InputMethodControllerTest::oldBegin_, INVALID_VALUE);
    EXPECT_EQ(InputMethodControllerTest::oldEnd_, INVALID_VALUE);
    EXPECT_EQ(InputMethodControllerTest::newBegin_, INVALID_VALUE);
    EXPECT_EQ(InputMethodControllerTest::newEnd_, INVALID_VALUE);
}

/**
 * @tc.name: testIMCOnSelectionChange07
 * @tc.desc: 'Attach with range -> Attach without range', it will get no selectionChange callback.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: Zhaolinglan
 */
HWTEST_F(InputMethodControllerTest, testIMCOnSelectionChange07, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCOnSelectionChange07 Test START");
    InputMethodControllerTest::inputMethodController_->Close();
    TextConfig textConfig;
    textConfig.range = { 1, 1 };
    inputMethodController_->Attach(textListener_, false, textConfig);
    std::this_thread::sleep_for(std::chrono::seconds(2));

    InputMethodControllerTest::ResetKeyboardListenerTextConfig();
    inputMethodController_->Attach(textListener_, false);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    EXPECT_EQ(InputMethodControllerTest::oldBegin_, INVALID_VALUE);
    EXPECT_EQ(InputMethodControllerTest::oldEnd_, INVALID_VALUE);
    EXPECT_EQ(InputMethodControllerTest::newBegin_, INVALID_VALUE);
    EXPECT_EQ(InputMethodControllerTest::newEnd_, INVALID_VALUE);
}

/**
 * @tc.name: testIMCOnSelectionChange08
 * @tc.desc: 'OnSelectionChange(1, 2) -> Attach without range -> OnSelectionChange(3, 4)', it will get (1,2,3,4)
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: Zhaolinglan
 */
HWTEST_F(InputMethodControllerTest, testIMCOnSelectionChange08, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCOnSelectionChange08 Test START");
    InputMethodControllerTest::inputMethodController_->Close();
    inputMethodController_->Attach(textListener_, false);
    std::u16string text = Str8ToStr16("");
    InputMethodControllerTest::TriggerSelectionChangeCallback(text, 1, 2);
    InputMethodControllerTest::ResetKeyboardListenerTextConfig();
    inputMethodController_->Attach(textListener_, false);
    InputMethodControllerTest::TriggerSelectionChangeCallback(text, 3, 4);
    EXPECT_EQ(InputMethodControllerTest::oldBegin_, 1);
    EXPECT_EQ(InputMethodControllerTest::oldEnd_, 2);
    EXPECT_EQ(InputMethodControllerTest::newBegin_, 3);
    EXPECT_EQ(InputMethodControllerTest::newEnd_, 4);
}

/**
 * @tc.name: testIMCOnSelectionChange09
 * @tc.desc: Attach with range(0, 0) -> OnSelectionChange("", 0, 0) -> Get 'textChange' and 'selectionChange' Callback
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: Zhaolinglan
 */
HWTEST_F(InputMethodControllerTest, testIMCOnSelectionChange09, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCOnSelectionChange09 Test START");
    InputMethodControllerTest::inputMethodController_->Close();
    TextConfig textConfig;
    textConfig.range = { 0, 0 };
    auto ret = inputMethodController_->Attach(textListener_, false, textConfig);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodControllerTest::ResetKeyboardListenerTextConfig();
    InputMethodControllerTest::text_ = "test";
    std::u16string text = Str8ToStr16("test1");
    InputMethodControllerTest::TriggerSelectionChangeCallback(text, 1, 6);
    EXPECT_EQ(InputMethodControllerTest::text_, "test1");
    EXPECT_EQ(InputMethodControllerTest::newBegin_, 1);
    EXPECT_EQ(InputMethodControllerTest::newEnd_, 6);
}

/**
 * @tc.name: testShowTextInput
 * @tc.desc: IMC ShowTextInput
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerTest, testShowTextInput, TestSize.Level0)
{
    IMSA_HILOGI("IMC ShowTextInput Test START");
    TextListener::ResetParam();
    inputMethodController_->ShowTextInput();
    EXPECT_TRUE(TextListener::WaitSendKeyboardStatusCallback(KeyboardStatus::SHOW));
}

/**
 * @tc.name: testShowSoftKeyboard
 * @tc.desc: IMC ShowSoftKeyboard
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerTest, testShowSoftKeyboard, TestSize.Level0)
{
    IMSA_HILOGI("IMC ShowSoftKeyboard Test START");
    IdentityCheckerMock::SetPermission(true);
    imeListener_->keyboardState_ = false;
    TextListener::ResetParam();
    int32_t ret = inputMethodController_->ShowSoftKeyboard();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(TextListener::WaitSendKeyboardStatusCallback(KeyboardStatus::SHOW) && WaitKeyboardStatus(true));
    IdentityCheckerMock::SetPermission(false);
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
    TextListener::ResetParam();
    int32_t ret = inputMethodController_->ShowCurrentInput();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(TextListener::WaitSendKeyboardStatusCallback(KeyboardStatus::SHOW) && WaitKeyboardStatus(true));
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
    EXPECT_TRUE(keyType >= static_cast<int32_t>(EnterKeyType::UNSPECIFIED) &&
        keyType <= static_cast<int32_t>(EnterKeyType::PREVIOUS));
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
    EXPECT_TRUE(inputPattern >= static_cast<int32_t>(TextInputType::NONE) &&
        inputPattern <= static_cast<int32_t>(TextInputType::VISIBLE_PASSWORD));
}

/**
 * @tc.name: testOnEditorAttributeChanged01
 * @tc.desc: IMC testOnEditorAttributeChanged01.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodControllerTest, testOnEditorAttributeChanged01, TestSize.Level0)
{
    IMSA_HILOGI("IMC testOnEditorAttributeChanged01 Test START");
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
 * @tc.name: testOnEditorAttributeChanged02
 * @tc.desc: Attach(isPreviewSupport) -> OnConfigurationChange -> editorChange callback (isPreviewSupport).
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodControllerTest, testOnEditorAttributeChanged02, TestSize.Level0)
{
    IMSA_HILOGI("IMC testOnEditorAttributeChanged02 Test START");
    InputAttribute attribute = { .inputPattern = static_cast<int32_t>(TextInputType::DATETIME),
        .enterKeyType = static_cast<int32_t>(EnterKeyType::GO),
        .isTextPreviewSupported = true };
    auto ret = inputMethodController_->Attach(textListener_, false, attribute);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    Configuration info;
    info.SetEnterKeyType(EnterKeyType::NEW_LINE);
    info.SetTextInputType(TextInputType::NUMBER);
    InputMethodControllerTest::ResetKeyboardListenerTextConfig();
    InputMethodControllerTest::TriggerConfigurationChangeCallback(info);
    EXPECT_EQ(InputMethodControllerTest::inputAttribute_.inputPattern, static_cast<int32_t>(info.GetTextInputType()));
    EXPECT_EQ(InputMethodControllerTest::inputAttribute_.enterKeyType, static_cast<int32_t>(info.GetEnterKeyType()));
    EXPECT_EQ(InputMethodControllerTest::inputAttribute_.isTextPreviewSupported, attribute.isTextPreviewSupported);
}

/**
 * @tc.name: testHideSoftKeyboard
 * @tc.desc: IMC HideSoftKeyboard
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerTest, testHideSoftKeyboard, TestSize.Level0)
{
    IMSA_HILOGI("IMC HideSoftKeyboard Test START");
    IdentityCheckerMock::SetPermission(true);
    imeListener_->keyboardState_ = true;
    TextListener::ResetParam();
    int32_t ret = inputMethodController_->HideSoftKeyboard();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(TextListener::WaitSendKeyboardStatusCallback(KeyboardStatus::HIDE) && WaitKeyboardStatus(false));
    IdentityCheckerMock::SetPermission(false);
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
    TextListener::ResetParam();
    int32_t ret = inputMethodController_->HideCurrentInput();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(TextListener::WaitSendKeyboardStatusCallback(KeyboardStatus::HIDE) && WaitKeyboardStatus(false));
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
    int32_t ret = inputMethodController_->StopInputSession();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(WaitKeyboardStatus(false));
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
    inputMethodController_->HideTextInput();
    EXPECT_TRUE(WaitKeyboardStatus(false));
}

/**
 * @tc.name: testIMCRequestShowInput.
 * @tc.desc: IMC testIMCRequestShowInput.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerTest, testIMCRequestShowInput, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCRequestShowInput Test START");
    imeListener_->keyboardState_ = false;
    int32_t ret = InputMethodControllerTest::inputMethodController_->RequestShowInput();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(WaitKeyboardStatus(true));
}

/**
 * @tc.name: testIMCRequestHideInput.
 * @tc.desc: IMC testIMCRequestHideInput.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerTest, testIMCRequestHideInput, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCRequestHideInput Test START");
    imeListener_->keyboardState_ = true;
    int32_t ret = InputMethodControllerTest::inputMethodController_->RequestHideInput();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(WaitKeyboardStatus(false));
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
    SelectListenerMock::start_ = 0;
    SelectListenerMock::end_ = 0;
    inputMethodAbility_->SelectByRange(1, 2);
    SelectListenerMock::WaitSelectListenerCallback();
    EXPECT_EQ(SelectListenerMock::start_, 1);
    EXPECT_EQ(SelectListenerMock::end_, 2);

    SelectListenerMock::direction_ = 0;
    inputMethodAbility_->SelectByMovement(static_cast<int32_t>(Direction::UP));
    SelectListenerMock::WaitSelectListenerCallback();
    EXPECT_EQ(SelectListenerMock::direction_, static_cast<int32_t>(Direction::UP));

    SelectListenerMock::direction_ = 0;
    inputMethodAbility_->SelectByMovement(static_cast<int32_t>(Direction::DOWN));
    SelectListenerMock::WaitSelectListenerCallback();
    EXPECT_EQ(SelectListenerMock::direction_, static_cast<int32_t>(Direction::DOWN));

    SelectListenerMock::direction_ = 0;
    inputMethodAbility_->SelectByMovement(static_cast<int32_t>(Direction::LEFT));
    SelectListenerMock::WaitSelectListenerCallback();
    EXPECT_EQ(SelectListenerMock::direction_, static_cast<int32_t>(Direction::LEFT));

    SelectListenerMock::direction_ = 0;
    inputMethodAbility_->SelectByMovement(static_cast<int32_t>(Direction::RIGHT));
    SelectListenerMock::WaitSelectListenerCallback();
    EXPECT_EQ(SelectListenerMock::direction_, static_cast<int32_t>(Direction::RIGHT));
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
 * @tc.name: testGetDefaultInputMethod
 * @tc.desc: IMC GetDefaultInputMethod
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerTest, testGetDefaultInputMethod, TestSize.Level0)
{
    IMSA_HILOGI("IMC testGetDefaultInputMethod Test START");
    std::shared_ptr<Property> property = nullptr;
    int32_t ret = inputMethodController_->GetDefaultInputMethod(property);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ASSERT_NE(property, nullptr);
    EXPECT_FALSE(property->name.empty());
}

/**
 * @tc.name: testGetSystemInputMethodConfig
 * @tc.desc: IMC GetSystemInputMethodConfig
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerTest, GetSystemInputMethodConfig, TestSize.Level0)
{
    IMSA_HILOGI("IMC GetSystemInputMethodConfig Test START");
    OHOS::AppExecFwk::ElementName inputMethodConfig;
    int32_t ret = inputMethodController_->GetInputMethodConfig(inputMethodConfig);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_GE(inputMethodConfig.GetBundleName().length(), 0);
    EXPECT_GE(inputMethodConfig.GetAbilityName().length(), 0);
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
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_EDITABLE);
    EXPECT_NE(TextListener::deleteForwardLength_, deleteForwardLength);

    int32_t deleteBackwardLength = 2;
    ret = inputMethodAbility_->DeleteBackward(deleteBackwardLength);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_EDITABLE);
    EXPECT_NE(TextListener::deleteBackwardLength_, deleteBackwardLength);

    std::string insertText = "t";
    ret = inputMethodAbility_->InsertText(insertText);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_EDITABLE);
    EXPECT_NE(TextListener::insertText_, Str8ToStr16(insertText));

    constexpr int32_t funcKey = 1;
    ret = inputMethodAbility_->SendFunctionKey(funcKey);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_EDITABLE);
    EXPECT_NE(TextListener::key_, funcKey);

    constexpr int32_t keyCode = 4;
    ret = inputMethodAbility_->MoveCursor(keyCode);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_EDITABLE);
    EXPECT_NE(TextListener::direction_, keyCode);
}

/**
 * @tc.name: testIsInputTypeSupported
 * @tc.desc: IsInputTypeSupported
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodControllerTest, testIsInputTypeSupported, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIsInputTypeSupported Test START");
    auto ret = inputMethodController_->IsInputTypeSupported(InputType::NONE);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: testStartInputType
 * @tc.desc: StartInputType
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodControllerTest, testStartInputType, TestSize.Level0)
{
    IMSA_HILOGI("IMC testStartInputType Test START");
    auto ret = inputMethodController_->StartInputType(InputType::NONE);
    EXPECT_NE(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: testSendPrivateCommand_001
 * @tc.desc: IMC SendPrivateCommand without default ime
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: mashaoyin
 */
HWTEST_F(InputMethodControllerTest, testSendPrivateCommand_001, TestSize.Level0)
{
    IMSA_HILOGI("IMC testSendPrivateCommand_001 Test START");
    InputMethodEngineListenerImpl::ResetParam();
    auto ret = inputMethodController_->Attach(textListener_, false);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    std::unordered_map<std::string, PrivateDataValue> privateCommand;
    PrivateDataValue privateDataValue1 = std::string("stringValue");
    privateCommand.emplace("value1", privateDataValue1);
    IdentityCheckerMock::SetBundleNameValid(false);
    ret = inputMethodController_->SendPrivateCommand(privateCommand);
    EXPECT_EQ(ret, ErrorCode::ERROR_NOT_DEFAULT_IME);
    inputMethodController_->Close();
    IdentityCheckerMock::SetBundleNameValid(true);
}

/**
 * @tc.name: testSendPrivateCommand_002
 * @tc.desc: SendPrivateCommand not bound, and empty privateCommand.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: mashaoyin
 */
HWTEST_F(InputMethodControllerTest, testSendPrivateCommand_002, TestSize.Level0)
{
    IMSA_HILOGI("IMC testSendPrivateCommand_002 Test START");
    InputMethodEngineListenerImpl::ResetParam();
    IdentityCheckerMock::SetBundleNameValid(true);
    inputMethodController_->Close();
    std::unordered_map<std::string, PrivateDataValue> privateCommand;
    auto ret = inputMethodController_->SendPrivateCommand(privateCommand);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_BOUND);

    ret = inputMethodController_->Attach(textListener_, false);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = inputMethodController_->SendPrivateCommand(privateCommand);
    EXPECT_EQ(ret, ErrorCode::ERROR_INVALID_PRIVATE_COMMAND_SIZE);
    inputMethodController_->Close();
    IdentityCheckerMock::SetBundleNameValid(false);
}

/**
 * @tc.name: testSendPrivateCommand_003
 * @tc.desc: IMC SendPrivateCommand with normal private command.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: mashaoyin
 */
HWTEST_F(InputMethodControllerTest, testSendPrivateCommand_003, TestSize.Level0)
{
    IMSA_HILOGI("IMC testSendPrivateCommand_003 Test START");
    IdentityCheckerMock::SetBundleNameValid(true);
    InputMethodEngineListenerImpl::ResetParam();
    auto ret = inputMethodController_->Attach(textListener_, false);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    std::unordered_map<std::string, PrivateDataValue> privateCommand;
    PrivateDataValue privateDataValue1 = std::string("stringValue");
    privateCommand.emplace("value1", privateDataValue1);
    ret = inputMethodController_->SendPrivateCommand(privateCommand);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    inputMethodController_->Close();
    IdentityCheckerMock::SetBundleNameValid(false);
}

/**
 * @tc.name: testSendPrivateCommand_004
 * @tc.desc: IMC SendPrivateCommand with correct data format and all data type.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: mashaoyin
 */
HWTEST_F(InputMethodControllerTest, testSendPrivateCommand_004, TestSize.Level0)
{
    IMSA_HILOGI("IMC testSendPrivateCommand_004 Test START");
    IdentityCheckerMock::SetBundleNameValid(true);
    InputMethodEngineListenerImpl::ResetParam();
    auto ret = inputMethodController_->Attach(textListener_, false);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    std::unordered_map<std::string, PrivateDataValue> privateCommand;
    PrivateDataValue privateDataValue1 = std::string("stringValue");
    PrivateDataValue privateDataValue2 = true;
    PrivateDataValue privateDataValue3 = 100;
    privateCommand.emplace("value1", privateDataValue1);
    privateCommand.emplace("value2", privateDataValue2);
    privateCommand.emplace("value3", privateDataValue3);
    ret = inputMethodController_->SendPrivateCommand(privateCommand);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(InputMethodEngineListenerImpl::WaitSendPrivateCommand(privateCommand));
    inputMethodController_->Close();
    IdentityCheckerMock::SetBundleNameValid(false);
}

/**
 * @tc.name: testSendPrivateCommand_005
 * @tc.desc: IMC SendPrivateCommand with more than 5 private command.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: mashaoyin
 */
HWTEST_F(InputMethodControllerTest, testSendPrivateCommand_005, TestSize.Level0)
{
    IMSA_HILOGI("IMC testSendPrivateCommand_005 Test START");
    IdentityCheckerMock::SetBundleNameValid(true);
    InputMethodEngineListenerImpl::ResetParam();
    auto ret = inputMethodController_->Attach(textListener_, false);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    std::unordered_map<std::string, PrivateDataValue> privateCommand;
    PrivateDataValue privateDataValue1 = std::string("stringValue");
    privateCommand.emplace("value1", privateDataValue1);
    privateCommand.emplace("value2", privateDataValue1);
    privateCommand.emplace("value3", privateDataValue1);
    privateCommand.emplace("value4", privateDataValue1);
    privateCommand.emplace("value5", privateDataValue1);
    privateCommand.emplace("value6", privateDataValue1);
    ret = inputMethodController_->SendPrivateCommand(privateCommand);
    EXPECT_EQ(ret, ErrorCode::ERROR_INVALID_PRIVATE_COMMAND_SIZE);
    inputMethodController_->Close();
    IdentityCheckerMock::SetBundleNameValid(false);
}

/**
 * @tc.name: testSendPrivateCommand_006
 * @tc.desc: IMC SendPrivateCommand size is more than 32KB.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: mashaoyin
 */
HWTEST_F(InputMethodControllerTest, testSendPrivateCommand_006, TestSize.Level0)
{
    IMSA_HILOGI("IMC testSendPrivateCommand_006 Test START");
    IdentityCheckerMock::SetBundleNameValid(true);
    InputMethodEngineListenerImpl::ResetParam();
    std::unordered_map<std::string, PrivateDataValue> privateCommand;
    PrivateDataValue privateDataValue1 = std::string("stringValue");
    PrivateDataValue privateDataValue2 = true;
    PrivateDataValue privateDataValue3 = 100;
    privateCommand.emplace("value1", privateDataValue1);
    privateCommand.emplace("value2", privateDataValue2);
    privateCommand.emplace("value3", privateDataValue3);
    TextConfig textConfig;
    textConfig.privateCommand = privateCommand;
    textConfig.windowId = 1;
    auto ret = inputMethodController_->Attach(textListener_, false, textConfig);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(InputMethodEngineListenerImpl::WaitSendPrivateCommand(privateCommand));
    inputMethodController_->Close();
    IdentityCheckerMock::SetBundleNameValid(false);
}

/**
 * @tc.name: testSendPrivateCommand_007
 * @tc.desc: IMC SendPrivateCommand with Attach.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: mashaoyin
 */
HWTEST_F(InputMethodControllerTest, testSendPrivateCommand_007, TestSize.Level0)
{
    IMSA_HILOGI("IMC testSendPrivateCommand_007 Test START");
    IdentityCheckerMock::SetBundleNameValid(true);
    TextListener::ResetParam();
    auto ret = inputMethodController_->Attach(textListener_, false);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    string str(32768, 'a');
    std::unordered_map<std::string, PrivateDataValue> privateCommand;
    PrivateDataValue privateDataValue1 = str;
    privateCommand.emplace("value1", privateDataValue1);
    ret = inputMethodController_->SendPrivateCommand(privateCommand);
    EXPECT_EQ(ret, ErrorCode::ERROR_INVALID_PRIVATE_COMMAND_SIZE);
    inputMethodController_->Close();
    IdentityCheckerMock::SetBundleNameValid(false);
}

/**
 * @tc.name: testSendPrivateCommand_008
 * @tc.desc: IMA SendPrivateCommand total size is 32KB, 32KB - 1, 32KB + 1.
 * @tc.type: IMC
 * @tc.require:
 * @tc.author: mashaoyin
 */
HWTEST_F(InputMethodControllerTest, testSendPrivateCommand_008, TestSize.Level0)
{
    IMSA_HILOGI("IMC testSendPrivateCommand_008 Test START");
    IdentityCheckerMock::SetBundleNameValid(true);
    auto ret = inputMethodController_->Attach(textListener_, false);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    TextListener::ResetParam();
    std::unordered_map<std::string, PrivateDataValue> privateCommand1 {
        { "v", string(PRIVATE_COMMAND_SIZE_MAX - 2, 'a') }
    };
    std::unordered_map<std::string, PrivateDataValue> privateCommand2 {
        { "v", string(PRIVATE_COMMAND_SIZE_MAX - 1, 'a') }
    };
    std::unordered_map<std::string, PrivateDataValue> privateCommand3 {
        { "v", string(PRIVATE_COMMAND_SIZE_MAX, 'a') }
    };
    ret = inputMethodController_->SendPrivateCommand(privateCommand1);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = inputMethodController_->SendPrivateCommand(privateCommand2);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = inputMethodController_->SendPrivateCommand(privateCommand3);
    EXPECT_EQ(ret, ErrorCode::ERROR_INVALID_PRIVATE_COMMAND_SIZE);
    inputMethodController_->Close();
    IdentityCheckerMock::SetBundleNameValid(false);
}

/**
 * @tc.name: testFinishTextPreviewAfterDetach_001
 * @tc.desc: IMC testFinishTextPreviewAfterDetach.
 * @tc.type: IMC
 * @tc.require:
 * @tc.author: zhaolinglan
 */
HWTEST_F(InputMethodControllerTest, testFinishTextPreviewAfterDetach_001, TestSize.Level0)
{
    IMSA_HILOGI("IMC testFinishTextPreviewAfterDetach_001 Test START");
    InputAttribute inputAttribute = { .isTextPreviewSupported = true };
    inputMethodController_->Attach(textListener_, false, inputAttribute);
    TextListener::ResetParam();
    inputMethodController_->Close();
    EXPECT_TRUE(TextListener::isFinishTextPreviewCalled_);
}

/**
 * @tc.name: testFinishTextPreviewAfterDetach_002
 * @tc.desc: IMC testFinishTextPreviewAfterDetach_002.
 * @tc.type: IMC
 * @tc.require:
 * @tc.author: zhaolinglan
 */
HWTEST_F(InputMethodControllerTest, testFinishTextPreviewAfterDetach_002, TestSize.Level0)
{
    IMSA_HILOGI("IMC testFinishTextPreviewAfterDetach_002 Test START");
    InputAttribute inputAttribute = { .isTextPreviewSupported = true };
    inputMethodController_->Attach(textListener_, false, inputAttribute);
    TextListener::ResetParam();
    inputMethodController_->DeactivateClient();
    EXPECT_FALSE(TextListener::isFinishTextPreviewCalled_);
}

/**
 * @tc.name: testOnInputReady
 * @tc.desc: IMC testOnInputReady
 * @tc.type: IMC
 * @tc.require:
 */
HWTEST_F(InputMethodControllerTest, testOnInputReady, TestSize.Level0)
{
    IMSA_HILOGI("IMC OnInputReady Test START");
    InputAttribute inputAttribute = { .isTextPreviewSupported = true };
    inputMethodController_->Attach(textListener_, false, inputAttribute);
    sptr<IRemoteObject> agentObject = nullptr;
    inputMethodController_->OnInputReady(agentObject);
    TextListener::ResetParam();
    inputMethodController_->DeactivateClient();
    EXPECT_FALSE(TextListener::isFinishTextPreviewCalled_);
}

/**
 * @tc.name: testIsPanelShown
 * @tc.desc: IMC testIsPanelShown
 * @tc.type: IMC
 * @tc.require:
 */
HWTEST_F(InputMethodControllerTest, testIsPanelShown, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIsPanelShown Test START");
    InputAttribute inputAttribute = { .isTextPreviewSupported = true };
    inputMethodController_->Attach(textListener_, false, inputAttribute);
    const PanelInfo panelInfo;
    bool isShown = false;
    auto ret = inputMethodController_->IsPanelShown(panelInfo, isShown);
    EXPECT_EQ(ret, ErrorCode::ERROR_STATUS_SYSTEM_PERMISSION);
}

/**
 * @tc.name: testOnKeyEventConsumeResult
 * @tc.desc: IMC OnKeyEventConsumeResult
 * @tc.type: IMC
 * @tc.require:
 */
HWTEST_F(InputMethodControllerTest, testOnKeyEventConsumeResult, TestSize.Level0)
{
    IMSA_HILOGI("IMC testOnKeyEventConsumeResult Test START");
    bool isConsumed = true;
    sptr<IRemoteObject> object;
    std::shared_ptr<KeyEventConsumerProxy> keyEventConsumerProxy = std::make_shared<KeyEventConsumerProxy>(object);
    keyEventConsumerProxy->OnKeyEventConsumeResult(isConsumed);
    keyEventConsumerProxy->OnKeyCodeConsumeResult(isConsumed);
    EXPECT_TRUE(isConsumed);
}

/**
 * @tc.name: test_InputDataChannelStub_OnRemote
 * @tc.desc: Checkout InputDataChannelStub_OnRemote.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerTest, InputDataChannelStub_OnRemote, TestSize.Level0)
{
    IMSA_HILOGI("IMC InputDataChannelStub_OnRemote Test START");
    sptr<InputDataChannelStub> mInputDataChannel = new InputDataChannelStub();
    MessageParcel data;
    MessageParcel reply;
    auto ret = mInputDataChannel->InsertTextOnRemote(data, reply);
    EXPECT_EQ(ret, ErrorCode::ERROR_EX_PARCELABLE);
    ret = mInputDataChannel->DeleteForwardOnRemote(data, reply);
    EXPECT_EQ(ret, ErrorCode::ERROR_EX_PARCELABLE);
    ret = mInputDataChannel->DeleteBackwardOnRemote(data, reply);
    EXPECT_EQ(ret, ErrorCode::ERROR_EX_PARCELABLE);
    ret = mInputDataChannel->GetTextBeforeCursorOnRemote(data, reply);
    EXPECT_EQ(ret, ErrorCode::ERROR_EX_PARCELABLE);
    ret = mInputDataChannel->GetTextAfterCursorOnRemote(data, reply);
    EXPECT_EQ(ret, ErrorCode::ERROR_EX_PARCELABLE);
    ret = mInputDataChannel->SendKeyboardStatusOnRemote(data, reply);
    EXPECT_EQ(ret, ErrorCode::ERROR_EX_PARCELABLE);
    ret = mInputDataChannel->SendFunctionKeyOnRemote(data, reply);
    EXPECT_EQ(ret, ErrorCode::ERROR_EX_PARCELABLE);
    ret = mInputDataChannel->MoveCursorOnRemote(data, reply);
    EXPECT_EQ(ret, ErrorCode::ERROR_EX_PARCELABLE);
    ret = mInputDataChannel->SelectByRangeOnRemote(data, reply);
    EXPECT_EQ(ret, ErrorCode::ERROR_EX_PARCELABLE);
    ret = mInputDataChannel->SelectByMovementOnRemote(data, reply);
    EXPECT_EQ(ret, ErrorCode::ERROR_EX_PARCELABLE);
    ret = mInputDataChannel->HandleExtendActionOnRemote(data, reply);
    EXPECT_EQ(ret, ErrorCode::ERROR_EX_PARCELABLE);
    ret = mInputDataChannel->NotifyPanelStatusInfoOnRemote(data, reply);
    EXPECT_EQ(ret, ErrorCode::ERROR_EX_PARCELABLE);
    ret = mInputDataChannel->NotifyKeyboardHeightOnRemote(data, reply);
    EXPECT_EQ(ret, ErrorCode::ERROR_EX_PARCELABLE);
    ret = mInputDataChannel->SendPrivateCommandOnRemote(data, reply);
    EXPECT_EQ(ret, ErrorCode::ERROR_EX_PARCELABLE);
    ret = mInputDataChannel->SetPreviewTextOnRemote(data, reply);
    EXPECT_EQ(ret, ErrorCode::ERROR_EX_PARCELABLE);
}

/**
 * @tc.name: testIMCDispatchKeyEvent_null
 * @tc.desc: test IMC DispatchKeyEvent with keyEvent null
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodControllerTest, testIMCDispatchKeyEvent_null, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCDispatchKeyEvent_null Test START");
    std::shared_ptr<MMI::KeyEvent> keyEvent = nullptr;
    KeyEventCallback callback;
    auto ret = inputMethodController_->DispatchKeyEvent(keyEvent, callback);
    EXPECT_EQ(ret, ErrorCode::ERROR_EX_NULL_POINTER);
    std::this_thread::sleep_for(std::chrono::seconds(2)); // avoid EventHandler crash
}

/**
 * @tc.name: testIMCReset
 * @tc.desc: test IMC Reset
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodControllerTest, testIMCReset, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCReset Test START");
    auto ret = inputMethodController_->Attach(textListener_, false);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_NE(inputMethodController_->abilityManager_, nullptr);
    EXPECT_NE(inputMethodController_->textListener_, nullptr);
    inputMethodController_->Reset();
    EXPECT_EQ(inputMethodController_->textListener_, nullptr);
    EXPECT_EQ(inputMethodController_->abilityManager_, nullptr);
    inputMethodController_->abilityManager_ = imsaProxy_;
}
/**
 * @tc.name: testGetInputMethodState_001
 * @tc.desc: IMA
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodControllerTest, testGetInputMethodState_001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodControllerTest GetInputMethodState_001 Test START");
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_.clear();
    ImeInfoInquirer::GetInstance().systemConfig_.enableFullExperienceFeature = true;
    ImeInfoInquirer::GetInstance().systemConfig_.enableInputMethodFeature = true;
    ImeEnabledInfo info;
    info.bundleName = TddUtil::currentBundleNameMock_;
    info.enabledStatus = EnabledStatus::FULL_EXPERIENCE_MODE;
    ImeEnabledCfg cfg;
    cfg.enabledInfos.push_back(info);
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_.insert({ imsa_->userId_, cfg });
    EnabledStatus status = EnabledStatus::DISABLED;
    auto ret = inputMethodController_->GetInputMethodState(status);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(status == EnabledStatus::FULL_EXPERIENCE_MODE);
}

/**
 * @tc.name: testGetInputMethodState_002
 * @tc.desc: IMA
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodControllerTest, testGetInputMethodState_002, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodControllerTest GetInputMethodState_002 Test START");
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_.clear();
    ImeInfoInquirer::GetInstance().systemConfig_.enableFullExperienceFeature = false;
    ImeInfoInquirer::GetInstance().systemConfig_.enableInputMethodFeature = false;
    ImeEnabledInfo info;
    info.bundleName = TddUtil::currentBundleNameMock_;
    info.enabledStatus = EnabledStatus::BASIC_MODE;
    ImeEnabledCfg cfg;
    cfg.enabledInfos.push_back(info);
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_.insert({ imsa_->userId_, cfg });
    EnabledStatus status = EnabledStatus::DISABLED;
    auto ret = inputMethodController_->GetInputMethodState(status);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(status == EnabledStatus::FULL_EXPERIENCE_MODE);
}

/**
 * @tc.name: testGetInputMethodState_003
 * @tc.desc: IMA
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodControllerTest, testGetInputMethodState_003, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodControllerTest GetInputMethodState_003 Test START");
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_.clear();
    ImeInfoInquirer::GetInstance().systemConfig_.enableFullExperienceFeature = true;
    ImeInfoInquirer::GetInstance().systemConfig_.enableInputMethodFeature = false;
    ImeEnabledInfo info;
    info.bundleName = TddUtil::currentBundleNameMock_;
    info.enabledStatus = EnabledStatus::BASIC_MODE;
    ImeEnabledCfg cfg;
    cfg.enabledInfos.push_back(info);
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_.insert({ imsa_->userId_, cfg });
    EnabledStatus status = EnabledStatus::DISABLED;
    auto ret = inputMethodController_->GetInputMethodState(status);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(status == EnabledStatus::BASIC_MODE);
}

/**
 * @tc.name: testGetInputMethodState_004
 * @tc.desc: IMA
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodControllerTest, testGetInputMethodState_004, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodControllerTest GetInputMethodState_004 Test START");
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_.clear();
    ImeInfoInquirer::GetInstance().systemConfig_.enableFullExperienceFeature = false;
    ImeInfoInquirer::GetInstance().systemConfig_.enableInputMethodFeature = true;
    ImeEnabledInfo info;
    info.bundleName = TddUtil::currentBundleNameMock_;
    info.enabledStatus = EnabledStatus::BASIC_MODE;
    ImeEnabledCfg cfg;
    cfg.enabledInfos.push_back(info);
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_.insert({ imsa_->userId_, cfg });
    EnabledStatus status = EnabledStatus::DISABLED;
    auto ret = inputMethodController_->GetInputMethodState(status);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(status == EnabledStatus::BASIC_MODE);
}

/**
 * @tc.name: testGetInputMethodState_005
 * @tc.desc: IMA
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodControllerTest, testGetInputMethodState_005, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodControllerTest testGetInputMethodState_005 Test START");
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_.clear();
    ImeInfoInquirer::GetInstance().systemConfig_.enableFullExperienceFeature = true;
    ImeInfoInquirer::GetInstance().systemConfig_.enableInputMethodFeature = true;
    ImeInfoInquirer::GetInstance().systemConfig_.sysSpecialIme = TddUtil::currentBundleNameMock_;
    ImeEnabledInfo info;
    info.bundleName = TddUtil::currentBundleNameMock_;
    info.enabledStatus = EnabledStatus::BASIC_MODE;
    ImeEnabledCfg cfg;
    cfg.enabledInfos.push_back(info);
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_.insert({ imsa_->userId_, cfg });
    EnabledStatus status = EnabledStatus::DISABLED;
    auto ret = inputMethodController_->GetInputMethodState(status);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(status == EnabledStatus::FULL_EXPERIENCE_MODE);
}

/**
 * @tc.name: testIsDefaultImeSetAndEnableIme
 * @tc.desc: test IMC Reset
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodControllerTest, testIsDefaultImeSetAndEnableIme, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIsDefaultImeSetAndEnableIme Test START");
    auto ret = inputMethodController_->IsDefaultImeSet();
    EXPECT_FALSE(ret);
    const std::string bundleName;
    auto enableRet = inputMethodController_->EnableIme(bundleName);
    EXPECT_NE(enableRet, ErrorCode::NO_ERROR);
}
} // namespace MiscServices
} // namespace OHOS
