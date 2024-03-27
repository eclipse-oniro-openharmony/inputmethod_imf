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

#include "ability_manager_client.h"
#include "global.h"
#include "ime_setting_listener_test_impl.h"
#include "input_method_ability_interface.h"
#include "input_method_controller.h"
#include "input_method_engine_listener_impl.h"
#include "keyboard_listener_test_impl.h"
#include "tdd_util.h"
#include "text_listener.h"
#include "unRegistered_type.h"
using namespace testing::ext;
namespace OHOS {
namespace MiscServices {
constexpr int32_t RETRY_INTERVAL = 100;
constexpr int32_t RETRY_TIME = 30;
constexpr int32_t WAIT_APP_START_COMPLETE = 1;
constexpr int32_t WAIT_BIND_COMPLETE = 1;
constexpr const char *BUNDLENAME = "com.example.editorbox";
class ImeProxyTest : public testing::Test {
public:
    static sptr<InputMethodController> imc_;
    static void SetUpTestCase(void)
    {
        TddUtil::InitWindow(false);
        imc_ = InputMethodController::GetInstance();
        RegisterImeSettingListener();
        SwitchToTestIme();
        InputMethodAbilityInterface::GetInstance().SetImeListener(std::make_shared<InputMethodEngineListenerImpl>());
        InputMethodAbilityInterface::GetInstance().SetKdListener(std::make_shared<KeyboardListenerTestImpl>());
        TddUtil::GrantNativePermission();
    }
    static void TearDownTestCase(void)
    {
        TddUtil::DestroyWindow();
    }
    void SetUp()
    {
        IMSA_HILOGI("InputMethodAbilityTest::SetUp");
    }
    void TearDown()
    {
        IMSA_HILOGI("InputMethodAbilityTest::TearDown");
    }

    static int32_t Attach(bool isPc)
    {
        isPc ? InputMethodEngineListenerImpl::isEnable_ = true : InputMethodEngineListenerImpl::isEnable_ = false;
        TextConfig config;
        config.cursorInfo = { .left = 0, .top = 1, .width = 0.5, .height = 1.2 };
        sptr<OnTextChangedListener> testListener = new TextListener();
        auto ret = imc_->Attach(testListener, true, config);
        return ret;
    }
    static void Close(bool isPc)
    {
        isPc ? InputMethodEngineListenerImpl::isEnable_ = true : InputMethodEngineListenerImpl::isEnable_ = false;
        imc_->Close();
    }

    static void StartApp() // bind client default, not show keyboard
    {
        static std::string cmd = "aa start ability -a EntryAbility -b com.example.editorbox";
        std::string result;
        auto ret = TddUtil::ExecuteCmd(cmd, result);
        EXPECT_TRUE(ret);
        BlockRetry(RETRY_INTERVAL, RETRY_TIME,
            []() { return AAFwk::AbilityManagerClient::GetInstance()->GetTopAbility().GetBundleName() == BUNDLENAME; });
        IMSA_HILOGI("start app success");
        sleep(WAIT_APP_START_COMPLETE); // ensure app start complete
    }

    static void ClickEditor(bool isPc)
    {
        isPc ? InputMethodEngineListenerImpl::isEnable_ = true : InputMethodEngineListenerImpl::isEnable_ = false;
        static std::string cmd = "uinput -T -d 200 200 -u 200 200";
        std::string result;
        auto ret = TddUtil::ExecuteCmd(cmd, result);
        EXPECT_TRUE(ret);
    }

    static void StopApp()
    {
        static std::string cmd = "aa force-stop com.example.editorbox";
        std::string result;
        auto ret = TddUtil::ExecuteCmd(cmd, result);
        EXPECT_TRUE(ret);
        BlockRetry(RETRY_INTERVAL, RETRY_TIME,
            []() { return AAFwk::AbilityManagerClient::GetInstance()->GetTopAbility().GetBundleName() != BUNDLENAME; });
        IMSA_HILOGI("stop app success");
    }

    static void EnsureBindComplete()
    {
        sleep(WAIT_BIND_COMPLETE);
    }

    static void SwitchToTestIme()
    {
        ImeSettingListenerTestImpl::ResetParam();
        TddUtil::SetTestTokenID(
            TddUtil::AllocTestTokenID(false, "undefined", { "ohos.permission.CONNECT_IME_ABILITY" }));
        TddUtil::GrantNativePermission();
        std::string beforeValue;
        TddUtil::GetEnableData(beforeValue);
        std::string allEnableIme = "{\"enableImeList\" : {\"100\" : [ \"com.example.testIme\"]}}";
        TddUtil::PushEnableImeValue("settings.inputmethod.enable_ime", allEnableIme);
        SubProperty subProp = {
            .name = "com.example.testIme",
            .id = "InputMethodExtAbility",
        };
        auto ret = imc_->SwitchInputMethod(SwitchTrigger::CURRENT_IME, subProp.name, subProp.id);
        EXPECT_EQ(ret, ErrorCode::NO_ERROR);
        EXPECT_TRUE(ImeSettingListenerTestImpl::WaitImeChange(subProp));
        TddUtil::RestoreSelfTokenID();
        TddUtil::PushEnableImeValue("settings.inputmethod.enable_ime", beforeValue);
    }

private:
    static void RegisterImeSettingListener()
    {
        TddUtil::StorageSelfTokenID();
        TddUtil::SetTestTokenID(TddUtil::AllocTestTokenID(true, "ImeProxyTest"));
        imc_->UpdateListenEventFlag(EventType::IME_SHOW, true);
        imc_->UpdateListenEventFlag(EventType::IME_HIDE, true);
        imc_->UpdateListenEventFlag(EventType::IME_CHANGE, true);
        TddUtil::RestoreSelfTokenID();
    }
};
sptr<InputMethodController> ImeProxyTest::imc_;

/**
* @tc.name: RegisteredProxyNotInEditor_001
* @tc.desc: not in editor
* @tc.type: FUNC
*/
HWTEST_F(ImeProxyTest, RegisteredProxyNotInEditor_001, TestSize.Level0)
{
    IMSA_HILOGI("ImeProxyTest::RegisteredProxyNotInEditor_001");
    // RegisteredProxy not in ima bind
    InputMethodEngineListenerImpl::ResetParam();
    InputMethodEngineListenerImpl::isEnable_ = true;
    auto ret = InputMethodAbilityInterface::GetInstance().RegisteredProxy();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_FALSE(InputMethodEngineListenerImpl::WaitInputStart());

    InputMethodAbilityInterface::GetInstance().UnRegisteredProxy(UnRegisteredType::REMOVE_PROXY_IME);
}

/**
* @tc.name: AttachInPcAfterRegisteredProxyNotInEditor_002
* @tc.desc: not in editor
* @tc.type: FUNC
*/
HWTEST_F(ImeProxyTest, AttachInPcAfterRegisteredProxyNotInEditor_002, TestSize.Level0)
{
    IMSA_HILOGI("ImeProxyTest::AttachInPcAfterRegisteredProxyNotInEditor_002");
    ASSERT_TRUE(TddUtil::GetFocused());
    // RegisteredProxy not in ima bind
    InputMethodEngineListenerImpl::isEnable_ = true;
    auto ret = InputMethodAbilityInterface::GetInstance().RegisteredProxy();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    // mock click the edit box in pc, bind proxy
    ImeSettingListenerTestImpl::ResetParam();
    InputMethodEngineListenerImpl::ResetParam();
    ret = Attach(true);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_FALSE(ImeSettingListenerTestImpl::WaitPanelShow());
    EXPECT_TRUE(InputMethodEngineListenerImpl::WaitInputStart());
    Close(false);
    InputMethodAbilityInterface::GetInstance().UnRegisteredProxy(UnRegisteredType::REMOVE_PROXY_IME);
    TddUtil::GetUnfocused();
}

/**
* @tc.name: AttachInPeAfterRegisteredProxyNotInEditor_003
* @tc.desc: not in editor
* @tc.type: FUNC
*/
HWTEST_F(ImeProxyTest, AttachInPeAfterRegisteredProxyNotInEditor_003, TestSize.Level0)
{
    IMSA_HILOGI("ImeProxyTest::AttachInPeAfterRegisteredProxyNotInEditor_003");
    ASSERT_TRUE(TddUtil::GetFocused());
    // RegisteredProxy not in ima bind
    InputMethodEngineListenerImpl::isEnable_ = true;
    auto ret = InputMethodAbilityInterface::GetInstance().RegisteredProxy();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    // mock click the edit box in pe, bind ima
    ImeSettingListenerTestImpl::ResetParam();
    InputMethodEngineListenerImpl::ResetParam();
    ret = Attach(false);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(ImeSettingListenerTestImpl::WaitPanelShow());
    EXPECT_FALSE(InputMethodEngineListenerImpl::WaitInputStart());
    Close(false);
    InputMethodAbilityInterface::GetInstance().UnRegisteredProxy(UnRegisteredType::REMOVE_PROXY_IME);
    TddUtil::GetUnfocused();
}

/**
* @tc.name: RegisteredProxyInImaEditor_004
* @tc.desc:
* @tc.type: FUNC
*/
HWTEST_F(ImeProxyTest, RegisteredProxyInImaEditor_004, TestSize.Level0)
{
    IMSA_HILOGI("ImeProxyTest::RegisteredProxyInImaEditor_004");
    ASSERT_TRUE(TddUtil::GetFocused());
    // mock click the edit box in pe, bind ima
    ImeSettingListenerTestImpl::ResetParam();
    auto ret = Attach(false);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(ImeSettingListenerTestImpl::WaitPanelShow());

    // RegisteredProxy in ima bind, unbind ima, bind proxy
    InputMethodEngineListenerImpl::ResetParam();
    ImeSettingListenerTestImpl::ResetParam();
    KeyboardListenerTestImpl::ResetParam();
    InputMethodEngineListenerImpl::isEnable_ = true;
    ret = InputMethodAbilityInterface::GetInstance().RegisteredProxy();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(ImeSettingListenerTestImpl::WaitPanelHide());
    EXPECT_TRUE(InputMethodEngineListenerImpl::WaitInputStart());
    EXPECT_TRUE(KeyboardListenerTestImpl::WaitCursorUpdate());
    Close(false);
    InputMethodAbilityInterface::GetInstance().UnRegisteredProxy(UnRegisteredType::REMOVE_PROXY_IME);
    TddUtil::GetUnfocused();
}

/**
* @tc.name: UnRegisteredAndRegisteredProxyInProxyBind_005
* @tc.desc:
* @tc.type: FUNC
*/
HWTEST_F(ImeProxyTest, UnRegisteredAndRegisteredProxyInProxyBind_005, TestSize.Level0)
{
    IMSA_HILOGI("ImeProxyTest::UnRegisteredAndRegisteredProxyInProxyBind_005");
    ASSERT_TRUE(TddUtil::GetFocused());
    auto ret = InputMethodAbilityInterface::GetInstance().RegisteredProxy();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    // mock click the edit box in pc, bind proxy
    InputMethodEngineListenerImpl::ResetParam();
    ret = Attach(true);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(InputMethodEngineListenerImpl::WaitInputStart());

    // UnRegisteredProxy in proxy bind
    InputMethodEngineListenerImpl::ResetParam();
    ret = InputMethodAbilityInterface::GetInstance().UnRegisteredProxy(UnRegisteredType::REMOVE_PROXY_IME);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(InputMethodEngineListenerImpl::WaitInputFinish());
    ret = InputMethodAbilityInterface::GetInstance().InsertText("b");
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);

    // RegisteredProxy proxy, rebind proxy
    InputMethodEngineListenerImpl::ResetParam();
    InputMethodEngineListenerImpl::isEnable_ = true;
    ret = InputMethodAbilityInterface::GetInstance().RegisteredProxy();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(InputMethodEngineListenerImpl::WaitInputStart());
    Close(false);
    InputMethodAbilityInterface::GetInstance().UnRegisteredProxy(UnRegisteredType::REMOVE_PROXY_IME);
    TddUtil::GetUnfocused();
}

/**
* @tc.name: UnRegisteredProxyNotInBind_stop_006
* @tc.desc:
* @tc.type: FUNC
*/
HWTEST_F(ImeProxyTest, UnRegisteredProxyNotInBind_stop_006, TestSize.Level0)
{
    IMSA_HILOGI("ImeProxyTest::UnRegisteredProxyNotInBind_stop_006");
    InputMethodEngineListenerImpl::ResetParam();
    InputMethodEngineListenerImpl::isEnable_ = true;
    auto ret = InputMethodAbilityInterface::GetInstance().RegisteredProxy();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = InputMethodAbilityInterface::GetInstance().UnRegisteredProxy(UnRegisteredType::REMOVE_PROXY_IME);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_FALSE(InputMethodEngineListenerImpl::WaitInputFinish());
}

/**
* @tc.name: UnRegisteredProxyInProxyBind_stop_007
* @tc.desc:
* @tc.type: FUNC
*/
HWTEST_F(ImeProxyTest, UnRegisteredProxyInProxyBind_stop_007, TestSize.Level0)
{
    IMSA_HILOGI("ImeProxyTest::UnRegisteredProxyInProxyBind_stop_007");
    ASSERT_TRUE(TddUtil::GetFocused());
    InputMethodEngineListenerImpl::isEnable_ = true;
    auto ret = InputMethodAbilityInterface::GetInstance().RegisteredProxy();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    // mock click the edit box in pc, bind proxy
    InputMethodEngineListenerImpl::ResetParam();
    ret = Attach(true);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(InputMethodEngineListenerImpl::WaitInputStart());

    InputMethodEngineListenerImpl::ResetParam();
    ImeSettingListenerTestImpl::ResetParam();
    ret = InputMethodAbilityInterface::GetInstance().UnRegisteredProxy(UnRegisteredType::REMOVE_PROXY_IME);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(InputMethodEngineListenerImpl::WaitInputFinish());
    EXPECT_FALSE(ImeSettingListenerTestImpl::WaitPanelShow());
    Close(false);
    TddUtil::GetUnfocused();
}

/**
* @tc.name: UnRegisteredProxyInImaBind_stop_008
* @tc.desc:
* @tc.type: FUNC
*/
HWTEST_F(ImeProxyTest, UnRegisteredProxyInImaBind_stop_008, TestSize.Level0)
{
    IMSA_HILOGI("ImeProxyTest::UnRegisteredProxyInImaBind_stop_008");
    ASSERT_TRUE(TddUtil::GetFocused());
    InputMethodEngineListenerImpl::isEnable_ = true;
    auto ret = InputMethodAbilityInterface::GetInstance().RegisteredProxy();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    // mock click the edit box in pe, bind ima
    ImeSettingListenerTestImpl::ResetParam();
    ret = Attach(false);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(ImeSettingListenerTestImpl::WaitPanelShow());

    InputMethodEngineListenerImpl::ResetParam();
    ImeSettingListenerTestImpl::ResetParam();
    ret = InputMethodAbilityInterface::GetInstance().UnRegisteredProxy(UnRegisteredType::REMOVE_PROXY_IME);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_FALSE(ImeSettingListenerTestImpl::WaitPanelHide());
    EXPECT_FALSE(InputMethodEngineListenerImpl::WaitInputFinish());
    Close(false);
    TddUtil::GetUnfocused();
}

/**
* @tc.name: UnRegisteredProxyNotInBind_switch_009
* @tc.desc:
* @tc.type: FUNC
*/
HWTEST_F(ImeProxyTest, UnRegisteredProxyNotInBind_switch_009, TestSize.Level0)
{
    IMSA_HILOGI("ImeProxyTest::UnRegisteredProxyNotInBind_switch_009");
    InputMethodEngineListenerImpl::isEnable_ = true;
    auto ret = InputMethodAbilityInterface::GetInstance().RegisteredProxy();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = InputMethodAbilityInterface::GetInstance().UnRegisteredProxy(UnRegisteredType::SWITCH_PROXY_IME_TO_IME);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_BOUND);
}

/**
* @tc.name: UnRegisteredProxyInProxyBind_switch_010
* @tc.desc:
* @tc.type: FUNC
*/
HWTEST_F(ImeProxyTest, UnRegisteredProxyInProxyBind_switch_010, TestSize.Level0)
{
    IMSA_HILOGI("ImeProxyTest::UnRegisteredProxyInProxyBind_switch_010");
    ASSERT_TRUE(TddUtil::GetFocused());
    InputMethodEngineListenerImpl::isEnable_ = true;
    auto ret = InputMethodAbilityInterface::GetInstance().RegisteredProxy();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    // mock click the edit box in pc, bind proxy
    InputMethodEngineListenerImpl::ResetParam();
    ret = Attach(true);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(InputMethodEngineListenerImpl::WaitInputStart());

    ImeSettingListenerTestImpl::ResetParam();
    InputMethodEngineListenerImpl::ResetParam();
    ret = InputMethodAbilityInterface::GetInstance().UnRegisteredProxy(UnRegisteredType::SWITCH_PROXY_IME_TO_IME);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(InputMethodEngineListenerImpl::WaitInputFinish());
    EXPECT_TRUE(ImeSettingListenerTestImpl::WaitPanelShow());
    Close(false);
    TddUtil::GetUnfocused();
}

/**
* @tc.name: UnRegisteredProxyWithErrorType_011
* @tc.desc:
* @tc.type: FUNC
*/
HWTEST_F(ImeProxyTest, UnRegisteredProxyWithErrorType_011, TestSize.Level0)
{
    IMSA_HILOGI("ImeProxyTest::UnRegisteredProxyWithErrorType_011");
    InputMethodEngineListenerImpl::isEnable_ = true;
    auto ret = InputMethodAbilityInterface::GetInstance().RegisteredProxy();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = InputMethodAbilityInterface::GetInstance().UnRegisteredProxy(static_cast<UnRegisteredType>(3));
    EXPECT_EQ(ret, ErrorCode::ERROR_BAD_PARAMETERS);
}

/**
* @tc.name: AppUnFocusInProxyBindInPe_012
* @tc.desc:
* @tc.type: FUNC
*/
HWTEST_F(ImeProxyTest, AppUnFocusInProxyBindInPe_012, TestSize.Level0)
{
    IMSA_HILOGI("ImeProxyTest::AppUnFocusInProxyBindInPe_012");
    ASSERT_TRUE(TddUtil::GetFocused());
    auto ret = InputMethodAbilityInterface::GetInstance().RegisteredProxy();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    // mock click the edit box in pc, bind proxy
    InputMethodEngineListenerImpl::ResetParam();
    ret = Attach(true);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(InputMethodEngineListenerImpl::WaitInputStart());

    // mock app unFocus in proxy bind in pe, unbind proxy
    Close(false);
    EXPECT_TRUE(InputMethodEngineListenerImpl::WaitInputFinish());
    ret = InputMethodAbilityInterface::GetInstance().InsertText("d");
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);

    ret = InputMethodAbilityInterface::GetInstance().UnRegisteredProxy(UnRegisteredType::REMOVE_PROXY_IME);
    TddUtil::GetUnfocused();
}

/**
* @tc.name: AppUnFocusInProxyBindInPc_013
* @tc.desc:
* @tc.type: FUNC
*/
HWTEST_F(ImeProxyTest, AppUnFocusInProxyBindInPc_013, TestSize.Level0)
{
    IMSA_HILOGI("ImeProxyTest::AppUnFocusInProxyBindInPc_013");
    ASSERT_TRUE(TddUtil::GetFocused());
    auto ret = InputMethodAbilityInterface::GetInstance().RegisteredProxy();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    // open the app, click the edit box in pc, bind proxy
    InputMethodEngineListenerImpl::ResetParam();
    ret = Attach(true);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(InputMethodEngineListenerImpl::WaitInputStart());

    // mock app unFocus in proxy bind in pc, unbind proxy
    Close(true);
    EXPECT_TRUE(InputMethodEngineListenerImpl::WaitInputFinish());
    ret = InputMethodAbilityInterface::GetInstance().InsertText("d");
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);

    InputMethodAbilityInterface::GetInstance().UnRegisteredProxy(UnRegisteredType::REMOVE_PROXY_IME);
    TddUtil::GetUnfocused();
}

/**
* @tc.name: ProxyAndImaSwitchTest_014
* @tc.desc:
* @tc.type: FUNC
*/
HWTEST_F(ImeProxyTest, ProxyAndImaSwitchTest_014, TestSize.Level0)
{
    IMSA_HILOGI("ImeProxyTest::ProxyAndImaSwitchTest_014");
    ASSERT_TRUE(TddUtil::GetFocused());
    auto ret = InputMethodAbilityInterface::GetInstance().RegisteredProxy();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    // mock click the edit box in pe, bind ima
    ImeSettingListenerTestImpl::ResetParam();
    ret = Attach(false);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(ImeSettingListenerTestImpl::WaitPanelShow());

    // mock click the edit box in pc, unbind ima, bind proxy
    ImeSettingListenerTestImpl::ResetParam();
    InputMethodEngineListenerImpl::ResetParam();
    ret = Attach(true);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(InputMethodEngineListenerImpl::WaitInputStart());
    EXPECT_TRUE(ImeSettingListenerTestImpl::WaitPanelHide());

    // mock click the edit box in pe, unbind proxy, bind ima
    ImeSettingListenerTestImpl::ResetParam();
    InputMethodEngineListenerImpl::ResetParam();
    ret = Attach(false);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(ImeSettingListenerTestImpl::WaitPanelShow());
    EXPECT_TRUE(InputMethodEngineListenerImpl::WaitInputFinish());

    InputMethodAbilityInterface::GetInstance().UnRegisteredProxy(UnRegisteredType::REMOVE_PROXY_IME);
    Close(false);
    TddUtil::GetUnfocused();
}

/**
* @tc.name: TextEditingTest_015
* @tc.desc:
* @tc.type: FUNC
*/
HWTEST_F(ImeProxyTest, TextEditingTest_015, TestSize.Level0)
{
    IMSA_HILOGI("ImeProxyTest::TextEditingTest_015");
    auto ret = InputMethodAbilityInterface::GetInstance().RegisteredProxy();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    // open the app, click the edit box in pc, bind proxy
    StartApp();
    InputMethodEngineListenerImpl::ResetParam();
    ClickEditor(true);
    EXPECT_TRUE(InputMethodEngineListenerImpl::WaitInputStart());
    EnsureBindComplete();

    KeyboardListenerTestImpl::ResetParam();
    ret = InputMethodAbilityInterface::GetInstance().InsertText("abc");
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(KeyboardListenerTestImpl::WaitSelectionChange(3));
    EXPECT_TRUE(KeyboardListenerTestImpl::WaitCursorUpdate());
    EXPECT_TRUE(KeyboardListenerTestImpl::WaitTextChange("abc"));

    KeyboardListenerTestImpl::ResetParam();
    ret = InputMethodAbilityInterface::GetInstance().DeleteForward(1);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(KeyboardListenerTestImpl::WaitSelectionChange(2));
    EXPECT_TRUE(KeyboardListenerTestImpl::WaitCursorUpdate());
    EXPECT_TRUE(KeyboardListenerTestImpl::WaitTextChange("ab"));

    KeyboardListenerTestImpl::ResetParam();
    ret = InputMethodAbilityInterface::GetInstance().MoveCursor(3); //left
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(KeyboardListenerTestImpl::WaitSelectionChange(1));
    EXPECT_TRUE(KeyboardListenerTestImpl::WaitCursorUpdate());
    EXPECT_TRUE(KeyboardListenerTestImpl::WaitTextChange("ab"));

    KeyboardListenerTestImpl::ResetParam();
    ret = InputMethodAbilityInterface::GetInstance().DeleteBackward(1);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(KeyboardListenerTestImpl::WaitSelectionChange(1));
    EXPECT_TRUE(KeyboardListenerTestImpl::WaitTextChange("a"));

    InputMethodAbilityInterface::GetInstance().UnRegisteredProxy(UnRegisteredType::REMOVE_PROXY_IME);
    StopApp();
}

/**
* @tc.name: ClientDiedInImaBind_016
* @tc.desc:
* @tc.type: FUNC
*/
HWTEST_F(ImeProxyTest, ClientDiedInImaBind_016, TestSize.Level0)
{
    IMSA_HILOGI("ImeProxyTest::ClientDiedInImaBind_016");
    // open the app, click the edit box in pe, bind ima
    StartApp();
    ImeSettingListenerTestImpl::ResetParam();
    ClickEditor(false);
    EXPECT_TRUE(ImeSettingListenerTestImpl::WaitPanelShow());
    EnsureBindComplete();

    ImeSettingListenerTestImpl::ResetParam();
    StopApp();
    EXPECT_TRUE(ImeSettingListenerTestImpl::WaitPanelHide());
}

/**
* @tc.name: ClientDiedInProxyBind_017
* @tc.desc:
* @tc.type: FUNC
*/
HWTEST_F(ImeProxyTest, ClientDiedInProxyBind_017, TestSize.Level0)
{
    IMSA_HILOGI("ImeProxyTest::ClientDiedInProxyBind_017");
    // RegisteredProxy not in ima bind
    InputMethodEngineListenerImpl::isEnable_ = true;
    auto ret = InputMethodAbilityInterface::GetInstance().RegisteredProxy();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    StartApp();
    InputMethodEngineListenerImpl::ResetParam();
    ClickEditor(true);
    EXPECT_TRUE(InputMethodEngineListenerImpl::WaitInputStart());
    EnsureBindComplete();

    InputMethodEngineListenerImpl::ResetParam();
    StopApp();
    EXPECT_TRUE(InputMethodEngineListenerImpl::WaitInputFinish());
    InputMethodAbilityInterface::GetInstance().UnRegisteredProxy(UnRegisteredType::REMOVE_PROXY_IME);
}
} // namespace MiscServices
} // namespace OHOS
