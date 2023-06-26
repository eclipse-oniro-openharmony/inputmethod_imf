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
#include "global.h"
#include "i_input_method_agent.h"
#include "i_input_method_system_ability.h"
#include "if_system_ability_manager.h"
#include "input_client_stub.h"
#include "input_data_channel_stub.h"
#include "input_death_recipient.h"
#include "input_method_ability.h"
#include "input_method_engine_listener.h"
#include "input_method_system_ability_proxy.h"
#include "input_method_utils.h"
#include "iservice_registry.h"
#include "keyboard_listener.h"
#include "message_parcel.h"
#include "system_ability.h"
#include "system_ability_definition.h"
#include "tdd_util.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace MiscServices {
constexpr const char *CMD_PIDOF_IMS = "pidof inputmethod_ser";
constexpr uint32_t DEALY_TIME = 1;
constexpr int32_t BUFF_LENGTH = 10;
    class TextListener : public OnTextChangedListener {
    public:
        TextListener()
        {
            std::shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create("TextListenerNotifier");
            serviceHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
        }
        ~TextListener()
        {
        }
        static KeyboardStatus keyboardStatus_;
        static std::mutex cvMutex_;
        static std::condition_variable cv_;
        std::shared_ptr<AppExecFwk::EventHandler> serviceHandler_;
        static int32_t direction_;
        static int32_t deleteForwardLength_;
        static int32_t deleteBackwardLength_;
        static std::u16string insertText_;
        static int32_t key_;
        static bool WaitIMACallback()
        {
            std::unique_lock<std::mutex> lock(TextListener::cvMutex_);
            return TextListener::cv_.wait_for(lock, std::chrono::seconds(1)) != std::cv_status::timeout;
        }
        void InsertText(const std::u16string &text)
        {
            IMSA_HILOGI("IMC TEST TextListener InsertText: %{public}s", Str16ToStr8(text).c_str());
            insertText_ = text;
        }

        void DeleteBackward(int32_t length)
        {
            IMSA_HILOGI("IMC TEST TextListener DeleteBackward length: %{public}d", length);
            deleteBackwardLength_ = length;
        }

        void SetKeyboardStatus(bool status)
        {
            IMSA_HILOGI("IMC TEST TextListener SetKeyboardStatus %{public}d", status);
        }
        void DeleteForward(int32_t length)
        {
            IMSA_HILOGI("IMC TEST TextListener DeleteForward length: %{public}d", length);
            deleteForwardLength_ = length;
        }
        void SendKeyEventFromInputMethod(const KeyEvent &event)
        {
            IMSA_HILOGI("IMC TEST TextListener sendKeyEventFromInputMethod");
        }
        void SendKeyboardStatus(const KeyboardStatus &keyboardStatus)
        {
            IMSA_HILOGD("TextListener::SendKeyboardStatus %{public}d", static_cast<int>(keyboardStatus));
            constexpr int32_t interval = 20;
            {
                std::unique_lock<std::mutex> lock(cvMutex_);
                IMSA_HILOGD("TextListener::SendKeyboardStatus lock");
                keyboardStatus_ = keyboardStatus;
            }
            serviceHandler_->PostTask([this]() { cv_.notify_all(); }, interval);
            IMSA_HILOGD("TextListener::SendKeyboardStatus notify_all");
        }
        void SendFunctionKey(const FunctionKey &functionKey)
        {
            IMSA_HILOGI("IMC TEST TextListener SendFunctionKey");
            EnterKeyType enterKeyType = functionKey.GetEnterKeyType();
            key_ = static_cast<int32_t>(enterKeyType);
        }
        void MoveCursor(const Direction direction)
        {
            IMSA_HILOGI("IMC TEST TextListener MoveCursor");
            direction_ = static_cast<int32_t>(direction);
        }
        void HandleSetSelection(int32_t start, int32_t end)
        {
        }
        void HandleExtendAction(int32_t action)
        {
        }
        void HandleSelect(int32_t keyCode, int32_t cursorMoveSkip)
        {
        }
    };
    KeyboardStatus TextListener::keyboardStatus_;
    std::mutex TextListener::cvMutex_;
    std::condition_variable TextListener::cv_;
    int32_t TextListener::direction_ = 0;
    int32_t TextListener::deleteForwardLength_ = 0;
    int32_t TextListener::deleteBackwardLength_ = 0;
    std::u16string TextListener::insertText_;
    int32_t TextListener::key_ = 0;

    class InputMethodEngineListenerImpl : public InputMethodEngineListener {
    public:
        InputMethodEngineListenerImpl(){};
        ~InputMethodEngineListenerImpl(){};
        static bool keyboardState_;
        static bool isInputStart_;
        static uint32_t windowId_;
        void OnKeyboardStatus(bool isShow) override;
        void OnInputStart() override;
        void OnInputStop(const std::string &imeId) override;
        void OnSetCallingWindow(uint32_t windowId) override;
        void OnSetSubtype(const SubProperty &property) override;
    };
    bool InputMethodEngineListenerImpl::keyboardState_ = false;
    bool InputMethodEngineListenerImpl::isInputStart_ = false;
    uint32_t InputMethodEngineListenerImpl::windowId_ = 0;

    void InputMethodEngineListenerImpl::OnKeyboardStatus(bool isShow)
    {
        IMSA_HILOGI("InputMethodEngineListenerImpl::OnKeyboardStatus %{public}s", isShow ? "show" : "hide");
        keyboardState_ = isShow;
    }
    void InputMethodEngineListenerImpl::OnInputStart()
    {
        IMSA_HILOGI("InputMethodEngineListenerImpl::OnInputStart");
        isInputStart_ = true;
    }
    void InputMethodEngineListenerImpl::OnInputStop(const std::string &imeId)
    {
        IMSA_HILOGI("InputMethodEngineListenerImpl::OnInputStop %{public}s", imeId.c_str());
        isInputStart_ = false;
    }
    void InputMethodEngineListenerImpl::OnSetCallingWindow(uint32_t windowId)
    {
        IMSA_HILOGI("InputMethodEngineListenerImpl::OnSetCallingWindow %{public}d", windowId);
        windowId_ = windowId;
    }
    void InputMethodEngineListenerImpl::OnSetSubtype(const SubProperty &property)
    {
        IMSA_HILOGD("InputMethodEngineListenerImpl::OnSetSubtype");
    }

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
        static pid_t GetPid();
        static void SetInputDeathRecipient();
        static void OnRemoteSaDied(const wptr<IRemoteObject> &remote);
        static bool WaitRemoteDiedCallback();
        static sptr<InputMethodController> inputMethodController_;
        static sptr<InputMethodAbility> inputMethodAbility_;
        static std::shared_ptr<MMI::KeyEvent> keyEvent_;
        static std::shared_ptr<InputMethodEngineListenerImpl> imeListener_;
        static std::shared_ptr<SelectListenerMock> controllerListener_;
        static sptr<OnTextChangedListener> textListener_;
        static std::mutex keyboardListenerMutex_;
        static std::condition_variable keyboardListenerCv_;
        static std::mutex onRemoteSaDiedMutex_;
        static std::condition_variable onRemoteSaDiedCv_;
        static sptr<InputDeathRecipient> deathRecipient_;
        static int32_t keyCode_;
        static int32_t keyStatus_;
        static CursorInfo cursorInfo_;
        static int32_t oldBegin_;
        static int32_t oldEnd_;
        static int32_t newBegin_;
        static int32_t newEnd_;
        static std::string text_;

        class KeyboardListenerImpl : public KeyboardListener {
        public:
            KeyboardListenerImpl(){};
            ~KeyboardListenerImpl(){};
            bool OnKeyEvent(int32_t keyCode, int32_t keyStatus) override
            {
                IMSA_HILOGD("KeyboardListenerImpl::OnKeyEvent %{public}d %{public}d", keyCode, keyStatus);
                keyCode_ = keyCode;
                keyStatus_ = keyStatus;
                InputMethodControllerTest::keyboardListenerCv_.notify_one();
                return true;
            }
            void OnCursorUpdate(int32_t positionX, int32_t positionY, int32_t height) override
            {
                IMSA_HILOGD("KeyboardListenerImpl::OnCursorUpdate %{public}d %{public}d %{public}d", positionX,
                    positionY, height);
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
        };
    };
    sptr<InputMethodController> InputMethodControllerTest::inputMethodController_;
    sptr<InputMethodAbility> InputMethodControllerTest::inputMethodAbility_;
    std::shared_ptr<MMI::KeyEvent> InputMethodControllerTest::keyEvent_;
    std::shared_ptr<InputMethodEngineListenerImpl> InputMethodControllerTest::imeListener_;
    std::shared_ptr<SelectListenerMock> InputMethodControllerTest::controllerListener_;
    sptr<OnTextChangedListener> InputMethodControllerTest::textListener_;
    int32_t InputMethodControllerTest::keyCode_ = 0;
    int32_t InputMethodControllerTest::keyStatus_ = 0;
    CursorInfo InputMethodControllerTest::cursorInfo_ = {};
    int32_t InputMethodControllerTest::oldBegin_ = 0;
    int32_t InputMethodControllerTest::oldEnd_ = 0;
    int32_t InputMethodControllerTest::newBegin_ = 0;
    int32_t InputMethodControllerTest::newEnd_ = 0;
    std::string InputMethodControllerTest::text_;
    std::mutex InputMethodControllerTest::keyboardListenerMutex_;
    std::condition_variable InputMethodControllerTest::keyboardListenerCv_;
    sptr<InputDeathRecipient> InputMethodControllerTest::deathRecipient_;
    std::mutex InputMethodControllerTest::onRemoteSaDiedMutex_;
    std::condition_variable InputMethodControllerTest::onRemoteSaDiedCv_;

    void InputMethodControllerTest::SetUpTestCase(void)
    {
        IMSA_HILOGI("InputMethodControllerTest::SetUpTestCase");
        TddUtil::StorageSelfTokenID();
        // Set the tokenID to the tokenID of the current ime
        std::shared_ptr<Property> property = InputMethodController::GetInstance()->GetCurrentInputMethod();
        std::string bundleName = property != nullptr ? property->name : "default.inputmethod.unittest";
        TddUtil::AllocTestTokenID(bundleName);
        TddUtil::SetTestTokenID();
        inputMethodAbility_ = InputMethodAbility::GetInstance();
        inputMethodAbility_->SetCoreAndAgent();
        inputMethodAbility_->OnImeReady();
        imeListener_ = std::make_shared<InputMethodEngineListenerImpl>();
        controllerListener_ = std::make_shared<SelectListenerMock>();
        textListener_ = new TextListener();
        inputMethodAbility_->SetKdListener(std::make_shared<KeyboardListenerImpl>());
        inputMethodAbility_->SetImeListener(imeListener_);

        inputMethodController_ = InputMethodController::GetInstance();
        keyEvent_ = MMI::KeyEvent::Create();
        constexpr int32_t keyAction = 2;
        constexpr int32_t keyCode = 2001;
        keyEvent_->SetKeyAction(keyAction);
        keyEvent_->SetKeyCode(keyCode);
        // Set the uid to the uid of the focus app
        TddUtil::StorageSelfUid();
        TddUtil::SetTestUid();
        SetInputDeathRecipient();
    }

    void InputMethodControllerTest::TearDownTestCase(void)
    {
        IMSA_HILOGI("InputMethodControllerTest::TearDownTestCase");
        TddUtil::RestoreSelfTokenID();
        TddUtil::DeleteTestTokenID();
        TddUtil::RestoreSelfUid();
    }

    void InputMethodControllerTest::SetUp(void)
    {
        IMSA_HILOGI("InputMethodControllerTest::SetUp");
    }

    void InputMethodControllerTest::TearDown(void)
    {
        IMSA_HILOGI("InputMethodControllerTest::TearDown");
    }

    pid_t InputMethodControllerTest::GetPid()
    {
        char buff[BUFF_LENGTH] = { 0 };
        FILE *fp = popen(CMD_PIDOF_IMS, "r");
        EXPECT_TRUE(fp != nullptr);
        fgets(buff, sizeof(buff), fp);
        pid_t pid = atoi(buff);
        pclose(fp);
        fp = nullptr;
        return pid;
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
        return onRemoteSaDiedCv_.wait_for(lock, std::chrono::seconds(1)) != std::cv_status::timeout;
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
     * @tc.name: testIMCdispatchKeyEvent
     * @tc.desc: IMC testdispatchKeyEvent.
     * @tc.type: FUNC
     * @tc.require:
     */
    HWTEST_F(InputMethodControllerTest, testIMCdispatchKeyEvent, TestSize.Level0)
    {
        IMSA_HILOGI("IMC dispatchKeyEvent Test START");
        bool ret = inputMethodController_->DispatchKeyEvent(keyEvent_);
        usleep(300);
        ret = ret && InputMethodControllerTest::keyCode_ == keyEvent_->GetKeyCode()
              && InputMethodControllerTest::keyStatus_ == keyEvent_->GetKeyAction();
        EXPECT_TRUE(ret);
    }

    /**
     * @tc.name: testIMCOnCursorUpdate01
     * @tc.desc: IMC testOnCursorUpdate
     * @tc.type: FUNC
     * @tc.require:
     * @tc.author: Zhaolinglan
     */
    HWTEST_F(InputMethodControllerTest, testIMCOnCursorUpdate01, TestSize.Level0)
    {
        IMSA_HILOGI("IMC testIMCOnCursorUpdate01 Test START");
        CursorInfo info = { 1, 3, 0, 5 };
        inputMethodController_->OnCursorUpdate(info);
        bool ret = false;
        {
            std::unique_lock<std::mutex> lock(InputMethodControllerTest::keyboardListenerMutex_);
            ret = InputMethodControllerTest::keyboardListenerCv_.wait_for(lock, std::chrono::seconds(DEALY_TIME),
                [&info] { return InputMethodControllerTest::cursorInfo_ == info; });
        }
        EXPECT_TRUE(ret);
        bool result = InputMethodControllerTest::cursorInfo_ == info;
        EXPECT_TRUE(result);

        InputMethodControllerTest::cursorInfo_ = {};
        inputMethodController_->Attach(textListener_, false);
        inputMethodController_->OnCursorUpdate(info);
        {
            std::unique_lock<std::mutex> lk(InputMethodControllerTest::keyboardListenerMutex_);
            ret = InputMethodControllerTest::keyboardListenerCv_.wait_for(lk, std::chrono::seconds(DEALY_TIME),
                [&info] { return InputMethodControllerTest::cursorInfo_ == info; });
        }
        EXPECT_FALSE(ret);
        result = InputMethodControllerTest::cursorInfo_ == info;
        EXPECT_FALSE(result);
    }

    /**
     * @tc.name: testIMCOnCursorUpdate02
     * @tc.desc: IMC testOnCursorUpdate
     * @tc.type: FUNC
     * @tc.require:
     * @tc.author: Zhaolinglan
     */
    HWTEST_F(InputMethodControllerTest, testIMCOnCursorUpdate02, TestSize.Level0)
    {
        IMSA_HILOGI("IMC testIMCOnCursorUpdate02 Test START");
        CursorInfo info = { 2, 4, 0, 6 };
        inputMethodController_->OnCursorUpdate(info);
        bool ret = false;
        {
            std::unique_lock<std::mutex> lock(InputMethodControllerTest::keyboardListenerMutex_);
            ret = InputMethodControllerTest::keyboardListenerCv_.wait_for(lock, std::chrono::seconds(DEALY_TIME),
                [&info] { return InputMethodControllerTest::cursorInfo_ == info; });
        }
        EXPECT_TRUE(ret);
        bool result = InputMethodControllerTest::cursorInfo_ == info;
        EXPECT_TRUE(result);

        InputMethodControllerTest::cursorInfo_ = {};
        InputMethodControllerTest::inputMethodController_->Close();
        inputMethodController_->Attach(textListener_, false);
        inputMethodController_->OnCursorUpdate(info);
        {
            std::unique_lock<std::mutex> lk(InputMethodControllerTest::keyboardListenerMutex_);
            ret = InputMethodControllerTest::keyboardListenerCv_.wait_for(lk, std::chrono::seconds(DEALY_TIME),
                [&info] { return InputMethodControllerTest::cursorInfo_ == info; });
        }
        EXPECT_TRUE(ret);
        result = InputMethodControllerTest::cursorInfo_ == info;
        EXPECT_TRUE(result);
    }

    /**
     * @tc.name: testIMCOnSelectionChange01
     * @tc.desc: IMC test IMC OnSelectionChange
     * @tc.type: FUNC
     * @tc.require:
     * @tc.author: Zhaolinglan
     */
    HWTEST_F(InputMethodControllerTest, testIMCOnSelectionChange01, TestSize.Level0)
    {
        IMSA_HILOGI("IMC testIMCOnSelectionChange01 Test START");
        std::u16string text = Str8ToStr16("testSelect");
        int start = 1;
        int end = 2;
        inputMethodController_->OnSelectionChange(text, start, end);
        bool ret = false;
        {
            std::unique_lock<std::mutex> lock(InputMethodControllerTest::keyboardListenerMutex_);
            ret = InputMethodControllerTest::keyboardListenerCv_.wait_for(lock, std::chrono::seconds(DEALY_TIME),
                [&text] { return InputMethodControllerTest::text_ == Str16ToStr8(text); });
        }
        EXPECT_TRUE(ret);
        bool result = InputMethodControllerTest::text_ == Str16ToStr8(text);
        EXPECT_TRUE(result);

        InputMethodControllerTest::text_ = "";
        inputMethodController_->Attach(textListener_, false);
        inputMethodController_->OnSelectionChange(text, start, end);
        {
            std::unique_lock<std::mutex> lk(InputMethodControllerTest::keyboardListenerMutex_);
            ret = InputMethodControllerTest::keyboardListenerCv_.wait_for(lk, std::chrono::seconds(DEALY_TIME),
                [&text] { return InputMethodControllerTest::text_ == Str16ToStr8(text); });
        }
        EXPECT_FALSE(ret);
        result = InputMethodControllerTest::text_ == Str16ToStr8(text);
        EXPECT_FALSE(result);
    }

    /**
     * @tc.name: testIMCOnSelectionChange02
     * @tc.desc: IMC test IMC OnSelectionChange
     * @tc.type: FUNC
     * @tc.require:
     * @tc.author: Zhaolinglan
     */
    HWTEST_F(InputMethodControllerTest, testIMCOnSelectionChange02, TestSize.Level0)
    {
        IMSA_HILOGI("IMC testIMCOnSelectionChange02 Test START");
        std::u16string text = Str8ToStr16("testSelect2");
        int start = 1;
        int end = 2;
        inputMethodController_->OnSelectionChange(text, start, end);
        bool ret = false;
        {
            std::unique_lock<std::mutex> lock(InputMethodControllerTest::keyboardListenerMutex_);
            ret = InputMethodControllerTest::keyboardListenerCv_.wait_for(lock, std::chrono::seconds(DEALY_TIME),
                [&text] { return InputMethodControllerTest::text_ == Str16ToStr8(text); });
        }
        EXPECT_TRUE(ret);
        bool result = InputMethodControllerTest::text_ == Str16ToStr8(text);
        EXPECT_TRUE(result);

        InputMethodControllerTest::text_ = "";
        InputMethodControllerTest::inputMethodController_->Close();
        inputMethodController_->Attach(textListener_, false);
        inputMethodController_->OnSelectionChange(text, start, end);
        {
            std::unique_lock<std::mutex> lk(InputMethodControllerTest::keyboardListenerMutex_);
            ret = InputMethodControllerTest::keyboardListenerCv_.wait_for(lk, std::chrono::seconds(DEALY_TIME),
                [&text] { return InputMethodControllerTest::text_ == Str16ToStr8(text); });
        }
        EXPECT_TRUE(ret);
        result = InputMethodControllerTest::text_ == Str16ToStr8(text);
        EXPECT_TRUE(result);
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
     * @tc.name: testIMCGetTextBeforeCursor
     * @tc.desc: IMC testGetTextBeforeCursor.
     * @tc.type: FUNC
     * @tc.require:
     */
    HWTEST_F(InputMethodControllerTest, testIMCGetTextBeforeCursor, TestSize.Level2)
    {
        IMSA_HILOGI("IMC GetTextBeforeCursor Test START");
        inputMethodController_->OnSelectionChange(Str8ToStr16(""), 0, 0);
        constexpr int32_t TEXT_LENGTH = 1;
        std::u16string text;
        inputMethodController_->GetTextBeforeCursor(TEXT_LENGTH, text);
        EXPECT_TRUE(text.size() == 0);
    }

    /**
     * @tc.name: testIMCGetTextAfterCursor
     * @tc.desc: IMC testGetTextAfterCursor.
     * @tc.type: FUNC
     * @tc.require:
     */
    HWTEST_F(InputMethodControllerTest, testIMCGetTextAfterCursor, TestSize.Level2)
    {
        IMSA_HILOGI("IMC GetTextAfterCursor Test START");
        inputMethodController_->OnSelectionChange(Str8ToStr16(""), 0, 0);
        constexpr int32_t TEXT_LENGTH = 1;
        std::u16string text;
        inputMethodController_->GetTextAfterCursor(TEXT_LENGTH, text);
        EXPECT_TRUE(text.size() == 0);
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
    * @tc.name: testIMCOnConfigurationChange
    * @tc.desc: IMC testOnConfigurationChange.
    * @tc.type: FUNC
    * @tc.require:
    */
    HWTEST_F(InputMethodControllerTest, testIMCOnConfigurationChange, TestSize.Level0)
    {
        IMSA_HILOGI("IMC OnConfigurationChange Test START");
        Configuration info;
        info.SetEnterKeyType(EnterKeyType::GO);
        info.SetTextInputType(TextInputType::TEXT);
        int32_t ret = inputMethodController_->Close();
        EXPECT_EQ(ret, ErrorCode::NO_ERROR);
        ret = inputMethodController_->OnConfigurationChange(info);
        EXPECT_EQ(ret, ErrorCode::NO_ERROR);

        auto keyType = static_cast<int32_t>(EnterKeyType::UNSPECIFIED);
        auto inputPattern = static_cast<int32_t>(TextInputType::NONE);
        ret = inputMethodController_->GetEnterKeyType(keyType);
        EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_EDITABLE);
        ret = inputMethodController_->GetInputPattern(inputPattern);
        EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_EDITABLE);

        ret = inputMethodController_->Attach(textListener_, false);
        EXPECT_EQ(ret, ErrorCode::NO_ERROR);

        ret = inputMethodController_->GetEnterKeyType(keyType);
        EXPECT_EQ(ret, ErrorCode::NO_ERROR);
        ret = inputMethodController_->GetInputPattern(inputPattern);
        EXPECT_EQ(ret, ErrorCode::NO_ERROR);
        EXPECT_TRUE(static_cast<OHOS::MiscServices::EnterKeyType>(keyType) == EnterKeyType::GO
                    && static_cast<OHOS::MiscServices::TextInputType>(inputPattern) == TextInputType::TEXT);
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
        pid_t pid = GetPid();
        EXPECT_TRUE(pid > 0);
        ret = kill(pid, SIGTERM);
        EXPECT_EQ(ret, 0);
        EXPECT_TRUE(WaitRemoteDiedCallback());
        inputMethodController_->OnRemoteSaDied(nullptr);
        EXPECT_TRUE(TextListener::WaitIMACallback());
        bool result = inputMethodController_->WasAttached();
        EXPECT_TRUE(result);
        inputMethodController_->Close();
    }
} // namespace MiscServices
} // namespace OHOS
