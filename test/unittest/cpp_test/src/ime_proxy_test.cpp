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

#include "global.h"
#include "ime_setting_listener_test_impl.h"
#include "input_method_ability_interface.h"
#include "input_method_controller.h"
#include "input_method_engine_listener_impl.h"
#include "keyboard_listener_test_impl.h"
#include "tdd_util.h"
#include "unRegistered_type.h"

using namespace testing::ext;
namespace OHOS {
namespace MiscServices {
constexpr int32_t WAIT_START_COMPLETE = 1;
constexpr int32_t WAIT_STOP_COMPLETE = 200;
class ImeProxyTest : public testing::Test {
public:
    static sptr<InputMethodController> imc_;
    static void SetUpTestCase(void)
    {
        RegisterPanelStatusChangeListener();
        InputMethodAbilityInterface::GetInstance().SetImeListener(std::make_shared<InputMethodEngineListenerImpl>());
        InputMethodAbilityInterface::GetInstance().SetKdListener(std::make_shared<KeyboardListenerTestImpl>());
        TddUtil::GrantNativePermission();
    }
    static void TearDownTestCase(void)
    {
    }
    void SetUp()
    {
        IMSA_HILOGI("InputMethodAbilityTest::SetUp");
    }
    void TearDown()
    {
        IMSA_HILOGI("InputMethodAbilityTest::TearDown");
    }
    static void StartApp() // bind client default, not show keyboard
    {
        static std::string cmd = "aa start ability -a EntryAbility -b com.example.editorbox";
        std::string result;
        auto ret = TddUtil::ExecuteCmd(cmd, result);
        EXPECT_TRUE(ret);
        sleep(WAIT_START_COMPLETE);
    }
    static void ClickEditorInPe()
    {
        InputMethodEngineListenerImpl::isEnable_ = false;
        ClickEditor();
    }
    static void ClickEditorInPc()
    {
        InputMethodEngineListenerImpl::isEnable_ = true;
        ClickEditor();
    }
    static void StopEditorInPe()
    {
        InputMethodEngineListenerImpl::isEnable_ = false;
        StopApp();
    }
    static void StopEditorInPc()
    {
        InputMethodEngineListenerImpl::isEnable_ = true;
        StopApp();
    }
    static void StopApp()
    {
        static std::string cmd = "aa force-stop com.example.editorbox";
        std::string result;
        auto ret = TddUtil::ExecuteCmd(cmd, result);
        EXPECT_TRUE(ret);
        usleep(WAIT_STOP_COMPLETE);
    }

private:
    static void RegisterPanelStatusChangeListener()
    {
        imc_ = InputMethodController::GetInstance();
        TddUtil::StorageSelfTokenID();
        TddUtil::SetTestTokenID(TddUtil::AllocTestTokenID(true, false, "ImeProxyTest"));
        imc_->SetSettingListener(std::make_shared<ImeSettingListenerTestImpl>());
        imc_->UpdateListenEventFlag("imeShow", true);
        imc_->UpdateListenEventFlag("imeHide", true);
        TddUtil::RestoreSelfTokenID();
    }
    static void ClickEditor()
    {
        static std::string cmd = "uinput -T -d 200 200 -u 200 200";
        std::string result;
        auto ret = TddUtil::ExecuteCmd(cmd, result);
        EXPECT_TRUE(ret);
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
    InputMethodEngineListenerImpl::isEnable_ = true;
    auto ret = InputMethodAbilityInterface::GetInstance().RegisteredProxy();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = InputMethodAbilityInterface::GetInstance().InsertText("a");
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);

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
    ImeSettingListenerTestImpl::ResetParam();
    // RegisteredProxy not in ima bind
    InputMethodEngineListenerImpl::isEnable_ = true;
    auto ret = InputMethodAbilityInterface::GetInstance().RegisteredProxy();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    StartApp();
    ClickEditorInPc();
    EXPECT_FALSE(ImeSettingListenerTestImpl::WaitPanelShow());
    ret = InputMethodAbilityInterface::GetInstance().InsertText("a");
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    InputMethodAbilityInterface::GetInstance().UnRegisteredProxy(UnRegisteredType::REMOVE_PROXY_IME);
    StopApp();
}

/**
* @tc.name: AttachInPeAfterRegisteredProxyNotInEditor_003
* @tc.desc: not in editor
* @tc.type: FUNC
*/
HWTEST_F(ImeProxyTest, AttachInPeAfterRegisteredProxyNotInEditor_003, TestSize.Level0)
{
    IMSA_HILOGI("ImeProxyTest::AttachInPeAfterRegisteredProxyNotInEditor_003");
    ImeSettingListenerTestImpl::ResetParam();
    // RegisteredProxy not in ima bind
    InputMethodEngineListenerImpl::isEnable_ = true;
    auto ret = InputMethodAbilityInterface::GetInstance().RegisteredProxy();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    StartApp();
    ClickEditorInPe();
    EXPECT_TRUE(ImeSettingListenerTestImpl::WaitPanelShow());
    ret = InputMethodAbilityInterface::GetInstance().InsertText("a");
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);

    InputMethodAbilityInterface::GetInstance().UnRegisteredProxy(UnRegisteredType::REMOVE_PROXY_IME);
    StopApp();
}

/**
* @tc.name: RegisteredProxyInImaEditor_004
* @tc.desc:
* @tc.type: FUNC
*/
HWTEST_F(ImeProxyTest, RegisteredProxyInImaEditor_004, TestSize.Level0)
{
    IMSA_HILOGI("ImeProxyTest::RegisteredProxyInImaEditor_004");
    // open the app, click the edit box in pe, bind ima
    InputMethodEngineListenerImpl::ResetParam();
    ImeSettingListenerTestImpl::ResetParam();
    KeyboardListenerTestImpl::ResetParam();
    StartApp();
    ClickEditorInPe();
    EXPECT_TRUE(ImeSettingListenerTestImpl::WaitPanelShow());

    // RegisteredProxy in ima bind, unbind ima, bind proxy
    InputMethodEngineListenerImpl::isEnable_ = true;
    auto ret = InputMethodAbilityInterface::GetInstance().RegisteredProxy();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(ImeSettingListenerTestImpl::WaitPanelHide());
    EXPECT_TRUE(InputMethodEngineListenerImpl::WaitInputStart());
    EXPECT_TRUE(KeyboardListenerTestImpl::WaitCursorUpdate());
    ret = InputMethodAbilityInterface::GetInstance().InsertText("a");
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    InputMethodAbilityInterface::GetInstance().UnRegisteredProxy(UnRegisteredType::REMOVE_PROXY_IME);
    StopApp();
}

/**
* @tc.name: UnRegisteredAndRegisteredProxyInProxyBind_005
* @tc.desc:
* @tc.type: FUNC
*/
HWTEST_F(ImeProxyTest, UnRegisteredAndRegisteredProxyInProxyBind_005, TestSize.Level0)
{
    IMSA_HILOGI("ImeProxyTest::UnRegisteredAndRegisteredProxyInProxyBind_005");
    // open the app, click the edit box in pe, bind ima
    ImeSettingListenerTestImpl::ResetParam();
    KeyboardListenerTestImpl::ResetParam();
    StartApp();
    ClickEditorInPe();
    ImeSettingListenerTestImpl::WaitPanelShow();
    // RegisteredProxy in ima bind, unbind ima, bind proxy
    InputMethodEngineListenerImpl::isEnable_ = true;
    auto ret = InputMethodAbilityInterface::GetInstance().RegisteredProxy();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = InputMethodAbilityInterface::GetInstance().InsertText("cc");
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(KeyboardListenerTestImpl::WaitCursorUpdate());
    EXPECT_TRUE(KeyboardListenerTestImpl::WaitSelectionChange(2));

    KeyboardListenerTestImpl::ResetParam();
    // RegisteredProxy after UnRegisteredProxy in proxy bind, rebind proxy
    ret = InputMethodAbilityInterface::GetInstance().UnRegisteredProxy(UnRegisteredType::REMOVE_PROXY_IME);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = InputMethodAbilityInterface::GetInstance().InsertText("b");
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);
    InputMethodEngineListenerImpl::isEnable_ = true;
    ret = InputMethodAbilityInterface::GetInstance().RegisteredProxy();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(KeyboardListenerTestImpl::WaitSelectionChange(2));
    EXPECT_TRUE(KeyboardListenerTestImpl::WaitCursorUpdate());
    ret = InputMethodAbilityInterface::GetInstance().InsertText("c");
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    InputMethodAbilityInterface::GetInstance().UnRegisteredProxy(UnRegisteredType::REMOVE_PROXY_IME);
    StopApp();
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
    InputMethodEngineListenerImpl::ResetParam();
    ImeSettingListenerTestImpl::ResetParam();
    InputMethodEngineListenerImpl::isEnable_ = true;
    auto ret = InputMethodAbilityInterface::GetInstance().RegisteredProxy();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    StartApp();
    ClickEditorInPc();
    InputMethodEngineListenerImpl::WaitInputStart();

    ret = InputMethodAbilityInterface::GetInstance().UnRegisteredProxy(UnRegisteredType::REMOVE_PROXY_IME);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(InputMethodEngineListenerImpl::WaitInputFinish());
    EXPECT_FALSE(ImeSettingListenerTestImpl::WaitPanelShow());

    StopApp();
}

/**
* @tc.name: UnRegisteredProxyInImaBind_stop_008
* @tc.desc:
* @tc.type: FUNC
*/
HWTEST_F(ImeProxyTest, UnRegisteredProxyInImaBind_stop_008, TestSize.Level0)
{
    IMSA_HILOGI("ImeProxyTest::UnRegisteredProxyInImaBind_stop_008");
    InputMethodEngineListenerImpl::ResetParam();
    ImeSettingListenerTestImpl::ResetParam();
    InputMethodEngineListenerImpl::isEnable_ = true;
    auto ret = InputMethodAbilityInterface::GetInstance().RegisteredProxy();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    StartApp();
    ClickEditorInPe();
    EXPECT_TRUE(ImeSettingListenerTestImpl::WaitPanelShow());

    ret = InputMethodAbilityInterface::GetInstance().UnRegisteredProxy(UnRegisteredType::REMOVE_PROXY_IME);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_FALSE(ImeSettingListenerTestImpl::WaitPanelHide());

    StopApp();
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
    InputMethodEngineListenerImpl::ResetParam();
    ImeSettingListenerTestImpl::ResetParam();
    InputMethodEngineListenerImpl::isEnable_ = true;
    auto ret = InputMethodAbilityInterface::GetInstance().RegisteredProxy();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    StartApp();
    ClickEditorInPc();
    InputMethodEngineListenerImpl::WaitInputStart();

    ret = InputMethodAbilityInterface::GetInstance().UnRegisteredProxy(UnRegisteredType::SWITCH_PROXY_IME_TO_IME);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(InputMethodEngineListenerImpl::WaitInputFinish());
    EXPECT_TRUE(ImeSettingListenerTestImpl::WaitPanelShow());

    StopApp();
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
* @tc.name: StopTheAppInProxyBindInPe_012
* @tc.desc:
* @tc.type: FUNC
*/
HWTEST_F(ImeProxyTest, StopTheAppInProxyBindInPe_012, TestSize.Level0)
{
    IMSA_HILOGI("ImeProxyTest::StopTheAppInProxyBindInPe_012");
    // open the app, click the edit box in pe, bind ima
    ImeSettingListenerTestImpl::ResetParam();
    StartApp();
    ClickEditorInPe();
    ImeSettingListenerTestImpl::WaitPanelShow();
    // RegisteredProxy in ima bind, unbind ima, bind proxy
    InputMethodEngineListenerImpl::isEnable_ = true;
    auto ret = InputMethodAbilityInterface::GetInstance().RegisteredProxy();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    // stop the app in proxy bind in pe, unbind proxy
    StopEditorInPe();
    ret = InputMethodAbilityInterface::GetInstance().InsertText("d");
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);

    ret = InputMethodAbilityInterface::GetInstance().UnRegisteredProxy(UnRegisteredType::REMOVE_PROXY_IME);
}

/**
* @tc.name: StopTheAppInProxyBindInPc_013
* @tc.desc:
* @tc.type: FUNC
*/
HWTEST_F(ImeProxyTest, StopTheAppInProxyBindInPc_013, TestSize.Level0)
{
    IMSA_HILOGI("ImeProxyTest::StopTheAppInProxyBindInPc_013");
    // open the app, click the edit box in pe, bind ima
    ImeSettingListenerTestImpl::ResetParam();
    StartApp();
    ClickEditorInPe();
    ImeSettingListenerTestImpl::WaitPanelShow();
    // RegisteredProxy in ima bind, unbind ima, bind proxy
    InputMethodEngineListenerImpl::isEnable_ = true;
    auto ret = InputMethodAbilityInterface::GetInstance().RegisteredProxy();

    // stop the app in proxy bind in pc, unbind proxy
    StopEditorInPc();
    ret = InputMethodAbilityInterface::GetInstance().InsertText("d");
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);

    InputMethodAbilityInterface::GetInstance().UnRegisteredProxy(UnRegisteredType::REMOVE_PROXY_IME);
}

/**
* @tc.name: ProxyAndImaSwitchTest_014
* @tc.desc:
* @tc.type: FUNC
*/
HWTEST_F(ImeProxyTest, ProxyAndImaSwitchTest_014, TestSize.Level0)
{
    IMSA_HILOGI("ImeProxyTest::ProxyAndImaSwitchTest_014");
    // open the app, click the edit box in pe, bind ima
    ImeSettingListenerTestImpl::ResetParam();
    InputMethodEngineListenerImpl::ResetParam();
    StartApp();
    ClickEditorInPe();
    ImeSettingListenerTestImpl::WaitPanelShow();
    // RegisteredProxy in ima bind, unbind ima, bind proxy
    InputMethodEngineListenerImpl::isEnable_ = true;
    auto ret = InputMethodAbilityInterface::GetInstance().RegisteredProxy();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    // click the edit box in pc, rebind proxy
    ClickEditorInPc();
    ret = InputMethodAbilityInterface::GetInstance().InsertText("d");
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    // click the edit box in pe, bind ima
    ImeSettingListenerTestImpl::ResetParam();
    ClickEditorInPe();
    EXPECT_TRUE(ImeSettingListenerTestImpl::WaitPanelShow());
    ret = InputMethodAbilityInterface::GetInstance().InsertText("e");
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);
    // click the edit box in pe, rebind ima
    ClickEditorInPe();
    ret = InputMethodAbilityInterface::GetInstance().InsertText("f");
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);
    // click the edit box in pc, bind proxy
    ClickEditorInPc();
    EXPECT_TRUE(ImeSettingListenerTestImpl::WaitPanelHide());
    EXPECT_TRUE(InputMethodEngineListenerImpl::WaitInputStart());
    ret = InputMethodAbilityInterface::GetInstance().InsertText("g");
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    InputMethodAbilityInterface::GetInstance().UnRegisteredProxy(UnRegisteredType::REMOVE_PROXY_IME);
    StopApp();
}

/**
* @tc.name: TextEditingTest_015
* @tc.desc:
* @tc.type: FUNC
*/
HWTEST_F(ImeProxyTest, TextEditingTest_015, TestSize.Level0)
{
    IMSA_HILOGI("ImeProxyTest::TextEditingTest_015");
    // open the app, click the edit box in pe, bind ima
    ImeSettingListenerTestImpl::ResetParam();
    StartApp();
    ClickEditorInPe();
    ImeSettingListenerTestImpl::WaitPanelShow();
    // RegisteredProxy in ima bind, unbind ima, bind proxy
    InputMethodEngineListenerImpl::isEnable_ = true;
    auto ret = InputMethodAbilityInterface::GetInstance().RegisteredProxy();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

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
    ImeSettingListenerTestImpl::ResetParam();
    StartApp();
    ClickEditorInPe();
    ImeSettingListenerTestImpl::WaitPanelShow();
    StopApp();
    ImeSettingListenerTestImpl::WaitPanelHide();
}

/**
* @tc.name: ClientDiedInProxyBind_017
* @tc.desc:
* @tc.type: FUNC
*/
HWTEST_F(ImeProxyTest, ClientDiedInProxyBind_017, TestSize.Level0)
{
    IMSA_HILOGI("ImeProxyTest::ClientDiedInProxyBind_017");
    InputMethodEngineListenerImpl::ResetParam();
    // RegisteredProxy not in ima bind
    InputMethodEngineListenerImpl::isEnable_ = true;
    auto ret = InputMethodAbilityInterface::GetInstance().RegisteredProxy();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    StartApp();
    ClickEditorInPc();
    EXPECT_TRUE(InputMethodEngineListenerImpl::WaitInputStart());
    StopApp();
    EXPECT_TRUE(InputMethodEngineListenerImpl::WaitInputFinish());
    InputMethodAbilityInterface::GetInstance().UnRegisteredProxy(UnRegisteredType::REMOVE_PROXY_IME);
}
} // namespace MiscServices
} // namespace OHOS
