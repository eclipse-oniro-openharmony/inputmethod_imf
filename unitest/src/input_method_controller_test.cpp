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

#include <gtest/gtest.h>
#include <sys/time.h>

#include <cstdint>
#include <functional>
#include <string>
#include <thread>
#include <vector>

#include "global.h"
#include "i_input_method_agent.h"
#include "i_input_method_system_ability.h"
#include "input_client_stub.h"
#include "input_data_channel_stub.h"
#include "input_method_setting.h"
#include "input_method_system_ability_proxy.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "utils.h"
#include "message_parcel.h"
#include "token_setproc.h"
#include "accesstoken_kit.h"
#include "nativetoken_kit.h"

using namespace testing::ext;
using namespace OHOS::Security::AccessToken;
namespace OHOS {
namespace MiscServices {
    void GrantNativePermission()
    {
        const char **perms = new const char *[2];
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
        TextListener() {}
        ~TextListener() {}
        void InsertText(const std::u16string& text)
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
        void SendKeyEventFromInputMethod(const KeyEvent& event)
        {
            IMSA_HILOGI("IMC TEST TextListener sendKeyEventFromInputMethod");
        }
        void SendKeyboardInfo(const KeyboardInfo& status)
        {
            IMSA_HILOGI("IMC TEST TextListener SendKeyboardInfo");
        }
        void MoveCursor(const Direction direction)
        {
            IMSA_HILOGI("IMC TEST TextListener MoveCursor");
        }
    };
    class InputMethodControllerTest : public testing::Test {
    public:
        static void SetUpTestCase(void);
        static void TearDownTestCase(void);
        void SetUp();
        void TearDown();
    };

    void InputMethodControllerTest::SetUpTestCase(void)
    {
        IMSA_HILOGI("InputMethodControllerTest::SetUpTestCase");
    }

    void InputMethodControllerTest::TearDownTestCase(void)
    {
        IMSA_HILOGI("InputMethodControllerTest::TearDownTestCase");
    }

    void InputMethodControllerTest::SetUp(void)
    {
        GrantNativePermission();
        IMSA_HILOGI("InputMethodControllerTest::SetUp");
    }

    void InputMethodControllerTest::TearDown(void)
    {
        IMSA_HILOGI("InputMethodControllerTest::TearDown");
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
    * @tc.name: testInputMethodSettingValue
    * @tc.desc: Checkout setting.
    * @tc.type: FUNC
    * @tc.require: issueI5JPMG
    */
    HWTEST_F(InputMethodControllerTest, testInputMethodSettingValue, TestSize.Level0)
    {
        InputMethodSetting setting;
        std::u16string key = InputMethodSetting::CURRENT_INPUT_METHOD_TAG;
        std::u16string oldValue = setting.GetValue(key);
        std::u16string newValue = u"testCurrentImeId";
        setting.SetValue(key, newValue);
        EXPECT_EQ(setting.GetValue(key), newValue);

        setting.SetValue(key, oldValue);
        EXPECT_EQ(setting.GetValue(key), oldValue);
    }

    /**
    * @tc.name: testInputMethodSettingCurrentInputMethod
    * @tc.desc: Checkout setting.
    * @tc.type: FUNC
    */
    HWTEST_F(InputMethodControllerTest, testInputMethodSettingCurrentInputMethod, TestSize.Level0)
    {
        InputMethodSetting setting;
        std::u16string curIme = setting.GetCurrentInputMethod();
        std::u16string testIme = u"testCurrentImeId";
        setting.SetCurrentInputMethod(testIme);
        EXPECT_EQ(setting.GetCurrentInputMethod(), testIme);

        setting.SetCurrentInputMethod(curIme);
        EXPECT_EQ(setting.GetCurrentInputMethod(), curIme);
    }

    /**
    * @tc.name: testInputMethodSettingCurrentKeyboard
    * @tc.desc: Checkout setting.
    * @tc.type: FUNC
    */
    HWTEST_F(InputMethodControllerTest, testInputMethodSettingCurrentKeyboard, TestSize.Level0)
    {
        InputMethodSetting setting;
        int32_t curType = setting.GetCurrentKeyboardType();
        int32_t testType = 10;
        setting.SetCurrentKeyboardType(testType);
        EXPECT_EQ(setting.GetCurrentKeyboardType(), testType);

        setting.SetCurrentKeyboardType(curType);
        EXPECT_EQ(setting.GetCurrentKeyboardType(), curType);

        curType = setting.GetCurrentKeyboardType();
        setting.SetCurrentKeyboardType(testType);
        EXPECT_EQ(setting.GetCurrentKeyboardType(), testType);

        setting.SetCurrentKeyboardType(curType);
        EXPECT_EQ(setting.GetCurrentKeyboardType(), curType);
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
        sptr<InputMethodController> imc = InputMethodController::GetInstance();
        EXPECT_TRUE(imc != nullptr);
        
        std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
        EXPECT_TRUE(keyEvent != nullptr);

        keyEvent->SetKeyAction(2);
        keyEvent->SetKeyCode(2013);

        sptr<OnTextChangedListener> textListener = new TextListener();
        imc->Attach(textListener);

        imc->dispatchKeyEvent(keyEvent);
    }

    /**
    * @tc.name: testInputMethodWholeProcess
    * @tc.desc: Bind IMSA.
    * @tc.type: FUNC
    * @tc.require: issueI5NXHK
    */
    HWTEST_F(InputMethodControllerTest, testInputMethodWholeProcess, TestSize.Level0)
    {
        IMSA_HILOGI("IMC TEST START");
        sptr<InputMethodController> imc = InputMethodController::GetInstance();
        EXPECT_TRUE(imc != nullptr);
        sptr<OnTextChangedListener> textListener = new TextListener();

        IMSA_HILOGI("IMC Attach START");
        imc->Attach(textListener);
        int waitForStatusOk = 2;
        sleep(waitForStatusOk);

        IMSA_HILOGI("IMC Attach isShowKeyboard true START");
        imc->Attach(textListener, true);
        sleep(waitForStatusOk);

        IMSA_HILOGI("IMC Attach isShowKeyboard false START");
        imc->Attach(textListener, false);
        sleep(waitForStatusOk);

        IMSA_HILOGI("IMC ShowCurrentInput START");
        imc->ShowCurrentInput();
        sleep(waitForStatusOk);

        IMSA_HILOGI("IMC ShowTextInput START");
        imc->ShowTextInput();
        sleep(10);

        IMSA_HILOGI("IMC HideTextInput START");
        imc->HideTextInput();
        sleep(waitForStatusOk);

        IMSA_HILOGI("IMC Close START");
        imc->Close();
        sleep(waitForStatusOk);
        IMSA_HILOGI("IMC TEST OVER");
    }

    /**
    * @tc.name: testShowSoftKeyboard
    * @tc.desc: IMC ShowSoftKeyboard
    * @tc.type: FUNC
    */
    HWTEST_F(InputMethodControllerTest, testShowSoftKeyboard, TestSize.Level0)
    {
        IMSA_HILOGI("IMC ShowSoftKeyboard Test START");
        sptr<InputMethodController> imc = InputMethodController::GetInstance();
        EXPECT_NE(imc, nullptr);

        int32_t ret = imc->ShowSoftKeyboard();
        EXPECT_EQ(ret, 0);
    }

    /**
     * @tc.name: testHideSoftKeyboard
     * @tc.desc: IMC HideSoftKeyboard
     * @tc.type: FUNC
     */
    HWTEST_F(InputMethodControllerTest, testHideSoftKeyboard, TestSize.Level0)
    {
        IMSA_HILOGI("IMC HideSoftKeyboard Test START");
        sptr<InputMethodController> imc = InputMethodController::GetInstance();
        EXPECT_NE(imc, nullptr);

        int32_t ret = imc->HideSoftKeyboard();
        EXPECT_EQ(ret, 0);
    }

    /**
     * @tc.name: testShowOptionalInputMethod
     * @tc.desc: IMC ShowOptionalInputMethod
     * @tc.type: FUNC
     */
    HWTEST_F(InputMethodControllerTest, testShowOptionalInputMethod, TestSize.Level2)
    {
        IMSA_HILOGI("IMC ShowOptionalInputMethod Test START");
        sptr<InputMethodController> imc = InputMethodController::GetInstance();
        EXPECT_NE(imc, nullptr);

        int32_t ret = imc->ShowOptionalInputMethod();
        EXPECT_EQ(ret, 0);
    }

    /**
     * @tc.name: testIMCGetCurrentInputMethod
     * @tc.desc: IMC GetCurrentInputMethod
     * @tc.type: FUNC
     * @tc.require: issueI5OX20
     */
    HWTEST_F(InputMethodControllerTest, testIMCGetCurrentInputMethod, TestSize.Level0)
    {
        IMSA_HILOGI("IMC GetCurrentInputMethod Test Start");
        std::shared_ptr<Property> property = InputMethodController::GetInstance()->GetCurrentInputMethod();
        EXPECT_TRUE(property != nullptr);
    }

    /**
    * @tc.name: testIMCShowCurrentInput
    * @tc.desc: IMC ShowCurrentInput.
    * @tc.type: FUNC
    * @tc.require: issueI5NXHK
    */
    HWTEST_F(InputMethodControllerTest, testIMCShowCurrentInput, TestSize.Level0)
    {
        IMSA_HILOGI("IMC ShowCurrentInput Test START");
        sptr<InputMethodController> imc = InputMethodController::GetInstance();
        EXPECT_TRUE(imc != nullptr);
        int32_t ret = imc->ShowCurrentInput();
        EXPECT_TRUE(ret == 0);
    }

    /**
     * @tc.name: testIMCListInputMethod
     * @tc.desc: IMC ListInputMethod
     * @tc.type: FUNC
     * @tc.require: issueI5OX20
     */
    HWTEST_F(InputMethodControllerTest, testIMCListInputMethod, TestSize.Level0)
    {
        IMSA_HILOGI("IMC ListInputMethod Test START");
        sptr<InputMethodController> imc = InputMethodController::GetInstance();
        EXPECT_NE(imc, nullptr);

        IMSA_HILOGI("Test list all input method");
        std::vector<Property> properties = imc->ListInputMethod();
        EXPECT_TRUE(!properties.empty());

        IMSA_HILOGI("Test list disabled input method");
        properties = imc->ListInputMethod(false);
        IMSA_HILOGI("Test list enabled input method");
        properties = imc->ListInputMethod(true);
        EXPECT_TRUE(!properties.empty());
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
        sptr<InputMethodController> imc = InputMethodController::GetInstance();
        EXPECT_TRUE(imc != nullptr);
        int32_t ret = imc->HideCurrentInput();
        EXPECT_TRUE(ret == 0);
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
        sptr<InputMethodController> imc = InputMethodController::GetInstance();
        EXPECT_TRUE(imc != nullptr);
        constexpr int32_t TEXT_LENGTH = 1;
        std::u16string text;
        imc->GetTextBeforeCursor(TEXT_LENGTH, text);
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
        sptr<InputMethodController> imc = InputMethodController::GetInstance();
        EXPECT_TRUE(imc != nullptr);
        constexpr int32_t TEXT_LENGTH = 1;
        std::u16string text;
        imc->GetTextAfterCursor(TEXT_LENGTH, text);
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
        sptr<InputMethodController> imc = InputMethodController::GetInstance();
        EXPECT_TRUE(imc != nullptr);
        int32_t keyType;
        imc->GetEnterKeyType(keyType);
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
        sptr<InputMethodController> imc = InputMethodController::GetInstance();
        EXPECT_TRUE(imc != nullptr);
        int32_t inputPattern;
        imc->GetInputPattern(inputPattern);
        EXPECT_TRUE(inputPattern >= static_cast<int32_t>(TextInputType::NONE)
                    && inputPattern <= static_cast<int32_t>(TextInputType::VISIBLE_PASSWORD));
    }

    /**
    * @tc.name: testIMCSwitchInputMethod
    * @tc.desc: IMC testSwitchInputMethod.
    * @tc.type: FUNC
    * @tc.require:
    */
    HWTEST_F(InputMethodControllerTest, testIMCSwitchInputMethod, TestSize.Level0)
    {
        IMSA_HILOGI("IMC SwitchInputMethod Test START");
        sptr<InputMethodController> imc = InputMethodController::GetInstance();
        EXPECT_TRUE(imc != nullptr);
        int32_t ret = imc->SwitchInputMethod("com.example.kikakeyboard");
        EXPECT_TRUE(ret == 0);
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
        sptr<InputMethodController> imc = InputMethodController::GetInstance();
        EXPECT_TRUE(imc != nullptr);

        imc->OnCursorUpdate({});
    }

    /**
    * @tc.name: testIMCOnSelectionChange
    * @tc.desc: IMC testOnSelectionChange
    * @tc.type: FUNC
    * @tc.require:
    */
    HWTEST_F(InputMethodControllerTest, testIMCOnSelectionChange, TestSize.Level0)
    {
        IMSA_HILOGI("IMC OnSelectionChange Test START");
        sptr<InputMethodController> imc = InputMethodController::GetInstance();
        EXPECT_TRUE(imc != nullptr);

        imc->OnSelectionChange(Str8ToStr16("text"), 1, 1);
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
        sptr<InputMethodController> imc = InputMethodController::GetInstance();
        EXPECT_TRUE(imc != nullptr);
        
        Configuration info;
        info.SetEnterKeyType(EnterKeyType::NONE);
        info.SetTextInputType(TextInputType::TEXT);

        imc->OnConfigurationChange(info);
    }

    /**
    * @tc.name: testIMCSetCallingWindow
    * @tc.desc: IMC testSetCallingWindow.
    * @tc.type: FUNC
    * @tc.require:
    */
    HWTEST_F(InputMethodControllerTest, testIMCSetCallingWindow, TestSize.Level0)
    {
        IMSA_HILOGI("IMC SetCallingWindow Test START");
        constexpr uint32_t WINDOW_ID = 0;
        sptr<InputMethodController> imc = InputMethodController::GetInstance();
        EXPECT_TRUE(imc != nullptr);
        
        imc->SetCallingWindow(WINDOW_ID);
    }
} // namespace MiscServices
} // namespace OHOS
