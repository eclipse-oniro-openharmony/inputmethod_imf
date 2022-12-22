/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#include "input_method_controller.h"

#include <event_handler.h>
#include <gtest/gtest.h>
#include <sys/time.h>

#include <condition_variable>
#include <cstdint>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "accesstoken_kit.h"
#include "global.h"
#include "i_input_method_agent.h"
#include "i_input_method_system_ability.h"
#include "input_client_stub.h"
#include "input_data_channel_stub.h"
#include "input_method_ability.h"
#include "input_method_engine_listener.h"
#include "input_method_system_ability_proxy.h"
#include "input_method_utils.h"
#include "iservice_registry.h"
#include "keyboard_listener.h"
#include "message_parcel.h"
#include "nativetoken_kit.h"
#include "system_ability_definition.h"
#include "token_setproc.h"
#include "utils.h"

using namespace testing::ext;
using namespace OHOS::Security::AccessToken;
namespace OHOS {
namespace MiscServices {
    void GrantNativePermission()
    {
        const char **perms = new const char *[1];
        perms[0] = "ohos.permission.CONNECT_IME_ABILITY";
        TokenInfoParams infoInstance = {
            .dcapsNum = 0,
            .permsNum = 1,
            .aclsNum = 0,
            .dcaps = nullptr,
            .perms = perms,
            .acls = nullptr,
            .processName = "inputmethod_imf",
            .aplStr = "system_core",
        };
        uint64_t tokenId = GetAccessTokenId(&infoInstance);
        int res = SetSelfTokenID(tokenId);
        if (res == 0) {
            IMSA_HILOGI("SetSelfTokenID success!");
        } else {
            IMSA_HILOGE("SetSelfTokenID fail!");
        }
        AccessTokenKit::ReloadNativeTokenInfo();
        delete[] perms;
    }

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
        static KeyboardInfo keyboardInfo_;
        static std::mutex cvMutex_;
        static std::condition_variable cv_;
        std::shared_ptr<AppExecFwk::EventHandler> serviceHandler_;
        static bool WaitIMACallback()
        {
            std::unique_lock<std::mutex> lock(TextListener::cvMutex_);
            return TextListener::cv_.wait_for(lock, std::chrono::seconds(1)) != std::cv_status::timeout;
        }
        void InsertText(const std::u16string &text)
        {
            IMSA_HILOGI("IMC TEST TextListener InsertText: %{public}s", MiscServices::Utils::ToStr8(text).c_str());
        }

        void DeleteBackward(int32_t length)
        {
            IMSA_HILOGI("IMC TEST TextListener DeleteBackward length: %{public}d", length);
        }

        void SetKeyboardStatus(bool status)
        {
            IMSA_HILOGI("IMC TEST TextListener SetKeyboardStatus %{public}d", status);
        }
        void DeleteForward(int32_t length)
        {
            IMSA_HILOGI("IMC TEST TextListener DeleteForward length: %{public}d", length);
        }
        void SendKeyEventFromInputMethod(const KeyEvent &event)
        {
            IMSA_HILOGI("IMC TEST TextListener sendKeyEventFromInputMethod");
        }
        void SendKeyboardInfo(const KeyboardInfo &status)
        {
            IMSA_HILOGD("TextListener::SendKeyboardInfo %{public}d", status.GetKeyboardStatus());
            constexpr int32_t INTERVAL = 20;
            {
                std::unique_lock<std::mutex> lock(cvMutex_);
                IMSA_HILOGD("TextListener::SendKeyboardInfo lock");
                keyboardInfo_ = status;
            }
            serviceHandler_->PostTask([this]() { cv_.notify_all(); }, INTERVAL);
            IMSA_HILOGD("TextListener::SendKeyboardInfo notify_all");
        }
        void MoveCursor(const Direction direction)
        {
            IMSA_HILOGI("IMC TEST TextListener MoveCursor");
        }
    };
    KeyboardInfo TextListener::keyboardInfo_;
    std::mutex TextListener::cvMutex_;
    std::condition_variable TextListener::cv_;

    class KeyboardListenerImpl : public KeyboardListener {
    public:
        KeyboardListenerImpl(){};
        ~KeyboardListenerImpl(){};
        static int32_t keyCode_;
        static int32_t keyStatus_;
        static CursorInfo cursorInfo_;
        bool OnKeyEvent(int32_t keyCode, int32_t keyStatus) override;
        void OnCursorUpdate(int32_t positionX, int32_t positionY, int32_t height) override;
        void OnSelectionChange(int32_t oldBegin, int32_t oldEnd, int32_t newBegin, int32_t newEnd) override;
        void OnTextChange(const std::string &text) override;
    };
    int32_t KeyboardListenerImpl::keyCode_ = 0;
    int32_t KeyboardListenerImpl::keyStatus_ = 0;
    CursorInfo KeyboardListenerImpl::cursorInfo_ = {};
    bool KeyboardListenerImpl::OnKeyEvent(int32_t keyCode, int32_t keyStatus)
    {
        IMSA_HILOGD("KeyboardListenerImpl::OnKeyEvent %{public}d %{public}d", keyCode, keyStatus);
        keyCode_ = keyCode;
        keyStatus_ = keyStatus;
        return true;
    }
    void KeyboardListenerImpl::OnCursorUpdate(int32_t positionX, int32_t positionY, int32_t height)
    {
        IMSA_HILOGD(
            "KeyboardListenerImpl::OnCursorUpdate %{public}d %{public}d %{public}d", positionX, positionY, height);
        cursorInfo_ = { static_cast<double>(positionX), static_cast<double>(positionY), 0,
            static_cast<double>(height) };
    }
    void KeyboardListenerImpl::OnSelectionChange(int32_t oldBegin, int32_t oldEnd, int32_t newBegin, int32_t newEnd)
    {
    }
    void KeyboardListenerImpl::OnTextChange(const std::string &text)
    {
    }

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
        keyboardState_ = isShow;
    }
    void InputMethodEngineListenerImpl::OnInputStart()
    {
        isInputStart_ = true;
    }
    void InputMethodEngineListenerImpl::OnInputStop(const std::string &imeId)
    {
        isInputStart_ = false;
    }
    void InputMethodEngineListenerImpl::OnSetCallingWindow(uint32_t windowId)
    {
        windowId_ = windowId;
    }
    void InputMethodEngineListenerImpl::OnSetSubtype(const SubProperty &property)
    {
        IMSA_HILOGD("InputMethodEngineListenerImpl::OnSetSubtype");
    }

    class InputMethodControllerTest : public testing::Test {
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
    };
    sptr<InputMethodController> InputMethodControllerTest::inputMethodController_;
    sptr<InputMethodAbility> InputMethodControllerTest::inputMethodAbility_;
    std::shared_ptr<MMI::KeyEvent> InputMethodControllerTest::keyEvent_;
    std::shared_ptr<KeyboardListenerImpl> InputMethodControllerTest::kbListener_;
    std::shared_ptr<InputMethodEngineListenerImpl> InputMethodControllerTest::imeListener_;
    sptr<OnTextChangedListener> InputMethodControllerTest::textListener_;

    void InputMethodControllerTest::SetUpTestCase(void)
    {
        IMSA_HILOGI("InputMethodControllerTest::SetUpTestCase");
        GrantNativePermission();
        inputMethodAbility_ = InputMethodAbility::GetInstance();
        inputMethodAbility_->OnImeReady();
        kbListener_ = std::make_shared<KeyboardListenerImpl>();
        imeListener_ = std::make_shared<InputMethodEngineListenerImpl>();
        textListener_ = new TextListener();
        inputMethodAbility_->setKdListener(kbListener_);
        inputMethodAbility_->setImeListener(imeListener_);
        inputMethodController_ = InputMethodController::GetInstance();

        keyEvent_ = MMI::KeyEvent::Create();
        constexpr int32_t KEY_ACTION = 2;
        constexpr int32_t KEY_CODE = 2001;
        keyEvent_->SetKeyAction(KEY_ACTION);
        keyEvent_->SetKeyCode(KEY_CODE);
    }

    void InputMethodControllerTest::TearDownTestCase(void)
    {
        IMSA_HILOGI("InputMethodControllerTest::TearDownTestCase");
    }

    void InputMethodControllerTest::SetUp(void)
    {
        IMSA_HILOGI("InputMethodControllerTest::SetUp");
    }

    void InputMethodControllerTest::TearDown(void)
    {
        IMSA_HILOGI("InputMethodControllerTest::TearDown");
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
        bool ret = inputMethodController_->dispatchKeyEvent(keyEvent_);
        usleep(300);
        ret = ret && kbListener_->keyCode_ == keyEvent_->GetKeyCode()
              && kbListener_->keyStatus_ == keyEvent_->GetKeyAction();
        EXPECT_TRUE(ret);
    }

    /**
     * @tc.name: testIMCOnCursorUpdate
     * @tc.desc: IMC testOnCursorUpdate
     * @tc.type: FUNC
     * @tc.require:
     */
    HWTEST_F(InputMethodControllerTest, testIMCOnCursorUpdate, TestSize.Level0)
    {
        IMSA_HILOGI("IMC OnCursorUpdate Test START");
        inputMethodController_->OnCursorUpdate({ 1, 2, 3, 4 });
        usleep(300);
        bool result = kbListener_->cursorInfo_.left == static_cast<double>(1)
                      && kbListener_->cursorInfo_.top == static_cast<double>(2)
                      && kbListener_->cursorInfo_.height == static_cast<double>(4);
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
        TextListener::keyboardInfo_.SetKeyboardStatus(static_cast<int32_t>(KeyboardStatus::NONE));
        inputMethodController_->ShowTextInput();
        EXPECT_TRUE(TextListener::WaitIMACallback());
        EXPECT_TRUE(TextListener::keyboardInfo_.GetKeyboardStatus() == KeyboardStatus::SHOW);
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
        TextListener::keyboardInfo_.SetKeyboardStatus(static_cast<int32_t>(KeyboardStatus::NONE));
        int32_t ret = inputMethodController_->ShowSoftKeyboard();
        EXPECT_TRUE(TextListener::WaitIMACallback());
        EXPECT_EQ(ret, ErrorCode::NO_ERROR);
        EXPECT_TRUE(
            imeListener_->keyboardState_ && TextListener::keyboardInfo_.GetKeyboardStatus() == KeyboardStatus::SHOW);
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
        TextListener::keyboardInfo_.SetKeyboardStatus(static_cast<int32_t>(KeyboardStatus::NONE));
        int32_t ret = inputMethodController_->ShowCurrentInput();
        EXPECT_TRUE(TextListener::WaitIMACallback());
        EXPECT_EQ(ret, ErrorCode::NO_ERROR);
        EXPECT_TRUE(
            imeListener_->keyboardState_ && TextListener::keyboardInfo_.GetKeyboardStatus() == KeyboardStatus::SHOW);
    }

    /**
     * @tc.name: testShowOptionalInputMethod
     * @tc.desc: IMC ShowOptionalInputMethod
     * @tc.type: FUNC
     */
    HWTEST_F(InputMethodControllerTest, testShowOptionalInputMethod, TestSize.Level2)
    {
        IMSA_HILOGI("IMC ShowOptionalInputMethod Test START");
        int32_t ret = inputMethodController_->ShowOptionalInputMethod();
        EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    }

    /**
     * @tc.name: testDisplayOptionalInputMethod
     * @tc.desc: IMC DisplayOptionalInputMethod
     * @tc.type: FUNC
     */
    HWTEST_F(InputMethodControllerTest, testDisplayOptionalInputMethod, TestSize.Level2)
    {
        IMSA_HILOGI("IMC DisplayOptionalInputMethod Test START");
        sleep(2);
        int32_t ret = inputMethodController_->DisplayOptionalInputMethod();
        EXPECT_EQ(ret, ErrorCode::NO_ERROR);
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
        inputMethodController_->OnConfigurationChange(info);

        auto keyType = static_cast<int32_t>(EnterKeyType::UNSPECIFIED);
        auto inputPattern = static_cast<int32_t>(TextInputType::NONE);
        inputMethodController_->GetEnterKeyType(keyType);
        inputMethodController_->GetInputPattern(inputPattern);
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
        TextListener::keyboardInfo_.SetKeyboardStatus(static_cast<int32_t>(KeyboardStatus::NONE));
        int32_t ret = inputMethodController_->HideSoftKeyboard();
        EXPECT_TRUE(TextListener::WaitIMACallback());
        EXPECT_EQ(ret, ErrorCode::NO_ERROR);
        EXPECT_TRUE(
            !imeListener_->keyboardState_ && TextListener::keyboardInfo_.GetKeyboardStatus() == KeyboardStatus::HIDE);
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
        TextListener::keyboardInfo_.SetKeyboardStatus(static_cast<int32_t>(KeyboardStatus::NONE));
        int32_t ret = inputMethodController_->HideCurrentInput();
        EXPECT_TRUE(TextListener::WaitIMACallback());
        EXPECT_EQ(ret, ErrorCode::NO_ERROR);
        EXPECT_TRUE(
            !imeListener_->keyboardState_ && TextListener::keyboardInfo_.GetKeyboardStatus() == KeyboardStatus::HIDE);
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
        TextListener::keyboardInfo_.SetKeyboardStatus(static_cast<int32_t>(KeyboardStatus::NONE));
        int32_t ret = inputMethodController_->StopInputSession();
        EXPECT_TRUE(TextListener::WaitIMACallback());
        EXPECT_EQ(ret, ErrorCode::NO_ERROR);
        EXPECT_TRUE(
            !imeListener_->keyboardState_ && TextListener::keyboardInfo_.GetKeyboardStatus() == KeyboardStatus::HIDE);
    }

    /**
     * @tc.name: testIMCHideTextInput.
     * @tc.desc: IMC testHideTextInput.
     * @tc.type: FUNC
     */
    HWTEST_F(InputMethodControllerTest, testIMCHideTextInput, TestSize.Level0)
    {
        IMSA_HILOGI("IMC InputStopSession Test START");
        imeListener_->keyboardState_ = true;
        TextListener::keyboardInfo_.SetKeyboardStatus(static_cast<int32_t>(KeyboardStatus::NONE));
        inputMethodController_->HideTextInput();
        EXPECT_TRUE(TextListener::WaitIMACallback());
        EXPECT_TRUE(
            !imeListener_->keyboardState_ && TextListener::keyboardInfo_.GetKeyboardStatus() == KeyboardStatus::HIDE);
    }

    /**
     * @tc.name: testIMCClose.
     * @tc.desc: IMC Close.
     * @tc.type: FUNC
     */
    HWTEST_F(InputMethodControllerTest, testIMCClose, TestSize.Level0)
    {
        IMSA_HILOGI("IMC Close Test START");
        imeListener_->keyboardState_ = true;
        TextListener::keyboardInfo_.SetKeyboardStatus(static_cast<int32_t>(KeyboardStatus::NONE));
        inputMethodController_->Close();

        bool ret = inputMethodController_->dispatchKeyEvent(keyEvent_);
        EXPECT_FALSE(ret);

        auto ret1 = inputMethodController_->ShowSoftKeyboard();
        EXPECT_EQ(ret1, ErrorCode::ERROR_CLIENT_NOT_FOUND);

        ret1 = inputMethodController_->HideSoftKeyboard();
        EXPECT_EQ(ret1, ErrorCode::ERROR_CLIENT_NOT_FOUND);

        ret1 = inputMethodController_->StopInputSession();
        EXPECT_EQ(ret1, ErrorCode::ERROR_CLIENT_NOT_FOUND);
    }

    /**
     * @tc.name: testDeathRecipient
     * @tc.desc: test DeathRecipient.
     * @tc.type: FUNC
     */
    HWTEST_F(InputMethodControllerTest, testDeathRecipient, TestSize.Level0)
    {
        IMSA_HILOGI("IMC OnRemoteDied Test START");
        auto deadObject = new ImsaDeathRecipient();
        sptr<ISystemAbilityManager> systemAbilityManager =
            SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        auto systemAbility = systemAbilityManager->GetSystemAbility(INPUT_METHOD_SYSTEM_ABILITY_ID, "");
        deadObject->OnRemoteDied(systemAbility);
        InputMethodController::GetInstance()->OnRemoteSaDied(systemAbility);
        delete deadObject;
    }
} // namespace MiscServices
} // namespace OHOS
