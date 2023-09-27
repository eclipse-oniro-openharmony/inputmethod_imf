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

using namespace testing::ext;
namespace OHOS {
namespace MiscServices {
class ImeProxyTest : public testing::Test {
public:
    static std::shared_ptr<InputMethodAbilityInterface> proxy_;
    static sptr<InputMethodController> imc_;
    static void SetUpTestCase(void)
    {
        RegisterPanelStatusChangeListener();
        proxy_ = InputMethodAbilityInterface::GetInstance();
        proxy_->SetImeListener(std::make_shared<InputMethodEngineListenerImpl>());
        proxy_->SetKdListener(std::make_shared<KeyboardListenerTestImpl>());
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
        sleep(1);
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
std::shared_ptr<InputMethodAbilityInterface> ImeProxyTest::proxy_;
sptr<InputMethodController> ImeProxyTest::imc_;

/**
* @tc.name: RegisteredProxyNotInEditor
* @tc.desc: not in editor
* @tc.type: FUNC
*/
HWTEST_F(ImeProxyTest, RegisteredProxyNotInEditor, TestSize.Level0)
{
    IMSA_HILOGI("ImeProxyTest::RegisteredProxyNotInEditor");
    // RegisteredProxy not in ima bind
    InputMethodEngineListenerImpl::isEnable_ = true;
    auto ret = proxy_->RegisteredProxy();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = proxy_->InsertText("a");
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);

    proxy_->UnRegisteredProxy(0);
}

/**
* @tc.name: AttachInPcAfterRegisteredProxyNotInEditor
* @tc.desc: not in editor
* @tc.type: FUNC
*/
HWTEST_F(ImeProxyTest, AttachInPcAfterRegisteredProxyNotInEditor, TestSize.Level0)
{
    IMSA_HILOGI("ImeProxyTest::AttachInPcAfterRegisteredProxyNotInEditor");
    ImeSettingListenerTestImpl::ResetParam();
    // RegisteredProxy not in ima bind
    InputMethodEngineListenerImpl::isEnable_ = true;
    auto ret = proxy_->RegisteredProxy();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    StartApp();
    ClickEditorInPc();
    EXPECT_FALSE(ImeSettingListenerTestImpl::WaitPanelShow());
    ret = proxy_->InsertText("a");
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    proxy_->UnRegisteredProxy(0);
    StopApp();
}

/**
* @tc.name: AttachInPeAfterRegisteredProxyNotInEditor
* @tc.desc: not in editor
* @tc.type: FUNC
*/
HWTEST_F(ImeProxyTest, AttachInPeAfterRegisteredProxyNotInEditor, TestSize.Level0)
{
    IMSA_HILOGI("ImeProxyTest::AttachInPeAfterRegisteredProxyNotInEditor");
    ImeSettingListenerTestImpl::ResetParam();
    // RegisteredProxy not in ima bind
    InputMethodEngineListenerImpl::isEnable_ = true;
    auto ret = proxy_->RegisteredProxy();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    StartApp();
    ClickEditorInPe();
    EXPECT_TRUE(ImeSettingListenerTestImpl::WaitPanelShow());
    ret = proxy_->InsertText("a");
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);

    proxy_->UnRegisteredProxy(0);
    StopApp();
}

/**
* @tc.name: RegisteredProxyInImaEditor
* @tc.desc:
* @tc.type: FUNC
*/
HWTEST_F(ImeProxyTest, RegisteredProxyInImaEditor, TestSize.Level0)
{
    IMSA_HILOGI("ImeProxyTest::RegisteredProxyInImaEditor");
    // open the app, click the edit box in pe, bind ima
    InputMethodEngineListenerImpl::ResetParam();
    ImeSettingListenerTestImpl::ResetParam();
    KeyboardListenerTestImpl::ResetParam();
    StartApp();
    ClickEditorInPe();
    EXPECT_TRUE(ImeSettingListenerTestImpl::WaitPanelShow());

    // RegisteredProxy in ima bind, unbind ima, bind proxy
    InputMethodEngineListenerImpl::isEnable_ = true;
    auto ret = proxy_->RegisteredProxy();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(ImeSettingListenerTestImpl::WaitPanelHide());
    EXPECT_TRUE(InputMethodEngineListenerImpl::WaitInputStart());
    EXPECT_TRUE(KeyboardListenerTestImpl::WaitCursorUpdate());
    ret = proxy_->InsertText("a");
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    proxy_->UnRegisteredProxy(0);
    StopApp();
}

/**
* @tc.name: UnRegisteredAndRegisteredProxyInProxyBind
* @tc.desc: in ima Editor, keyboard show
* @tc.type: FUNC
*/
HWTEST_F(ImeProxyTest, UnRegisteredAndRegisteredProxyInProxyBind, TestSize.Level0)
{
    IMSA_HILOGI("ImeProxyTest::UnRegisteredAndRegisteredProxyInProxyBind");
    // open the app, click the edit box in pe, bind ima
    ImeSettingListenerTestImpl::ResetParam();
    StartApp();
    ClickEditorInPe();
    ImeSettingListenerTestImpl::WaitPanelShow();
    // RegisteredProxy in ima bind, unbind ima, bind proxy
    InputMethodEngineListenerImpl::isEnable_ = true;
    auto ret = proxy_->RegisteredProxy();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    // RegisteredProxy after UnRegisteredProxy in proxy bind, rebind proxy
    ret = proxy_->UnRegisteredProxy(0);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = proxy_->InsertText("b");
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);
    InputMethodEngineListenerImpl::isEnable_ = true;
    ret = proxy_->RegisteredProxy();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = proxy_->InsertText("c");
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    proxy_->UnRegisteredProxy(0);
    StopApp();
}

/**
* @tc.name: UnRegisteredProxyNotInBind_stop
* @tc.desc:
* @tc.type: FUNC
*/
HWTEST_F(ImeProxyTest, UnRegisteredProxyNotInBind_stop, TestSize.Level0)
{
    IMSA_HILOGI("ImeProxyTest::UnRegisteredProxyNotInBind_stop");
    InputMethodEngineListenerImpl::ResetParam();
    InputMethodEngineListenerImpl::isEnable_ = true;
    auto ret = proxy_->RegisteredProxy();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = proxy_->UnRegisteredProxy(0);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_FALSE(InputMethodEngineListenerImpl::WaitInputFinish());
}

/**
* @tc.name: UnRegisteredProxyInProxyBind_stop
* @tc.desc:
* @tc.type: FUNC
*/
HWTEST_F(ImeProxyTest, UnRegisteredProxyInProxyBind_stop, TestSize.Level0)
{
    IMSA_HILOGI("ImeProxyTest::UnRegisteredProxyInProxyBind_stop");
    InputMethodEngineListenerImpl::ResetParam();
    ImeSettingListenerTestImpl::ResetParam();
    InputMethodEngineListenerImpl::isEnable_ = true;
    auto ret = proxy_->RegisteredProxy();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    StartApp();
    ClickEditorInPc();
    InputMethodEngineListenerImpl::WaitInputStart();

    ret = proxy_->UnRegisteredProxy(0);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(InputMethodEngineListenerImpl::WaitInputFinish());
    EXPECT_FALSE(ImeSettingListenerTestImpl::WaitPanelShow());

    StopApp();
}

/**
* @tc.name: UnRegisteredProxyInImaBind_stop
* @tc.desc:
* @tc.type: FUNC
*/
HWTEST_F(ImeProxyTest, UnRegisteredProxyInImaBind_stop, TestSize.Level0)
{
    IMSA_HILOGI("ImeProxyTest::UnRegisteredProxyInImaBind_stop");
    InputMethodEngineListenerImpl::ResetParam();
    ImeSettingListenerTestImpl::ResetParam();
    InputMethodEngineListenerImpl::isEnable_ = true;
    auto ret = proxy_->RegisteredProxy();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    StartApp();
    ClickEditorInPe();
    EXPECT_TRUE(ImeSettingListenerTestImpl::WaitPanelShow());

    ret = proxy_->UnRegisteredProxy(0);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_FALSE(ImeSettingListenerTestImpl::WaitPanelHide());

    StopApp();
}

/**
* @tc.name: UnRegisteredProxyNotInBind_switch
* @tc.desc:
* @tc.type: FUNC
*/
HWTEST_F(ImeProxyTest, UnRegisteredProxyNotInBind_switch, TestSize.Level0)
{
    IMSA_HILOGI("ImeProxyTest::UnRegisteredProxyNotInBind_switch");
    InputMethodEngineListenerImpl::isEnable_ = true;
    auto ret = proxy_->RegisteredProxy();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = proxy_->UnRegisteredProxy(1);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_BOUND);
}

/**
* @tc.name: UnRegisteredProxyInProxyBind_switch
* @tc.desc:
* @tc.type: FUNC
*/
HWTEST_F(ImeProxyTest, UnRegisteredProxyInProxyBind_switch, TestSize.Level0)
{
    IMSA_HILOGI("ImeProxyTest::UnRegisteredProxyInProxyBind_switch");
    InputMethodEngineListenerImpl::ResetParam();
    ImeSettingListenerTestImpl::ResetParam();
    InputMethodEngineListenerImpl::isEnable_ = true;
    auto ret = proxy_->RegisteredProxy();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    StartApp();
    ClickEditorInPc();
    InputMethodEngineListenerImpl::WaitInputStart();

    ret = proxy_->UnRegisteredProxy(1);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(InputMethodEngineListenerImpl::WaitInputFinish());
    EXPECT_TRUE(ImeSettingListenerTestImpl::WaitPanelShow());

    StopApp();
}

/**
* @tc.name: UnRegisteredProxyInImaBindWithoutKeyBoardShow_switch
* @tc.desc:
* @tc.type: FUNC
*/
HWTEST_F(ImeProxyTest, UnRegisteredProxyInImaBindWithoutKeyBoardShow_switch, TestSize.Level0)
{
    IMSA_HILOGI("ImeProxyTest::UnRegisteredProxyInImaBindWithoutKeyBoardShow_switch");
    InputMethodEngineListenerImpl::ResetParam();
    ImeSettingListenerTestImpl::ResetParam();
    InputMethodEngineListenerImpl::isEnable_ = true;
    auto ret = proxy_->RegisteredProxy();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    // bind ima, but not show keyboard
    InputMethodEngineListenerImpl::isEnable_ = false;
    StartApp();
    EXPECT_FALSE(ImeSettingListenerTestImpl::WaitPanelShow());
    ret = proxy_->InsertText("a");
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);

    ret = proxy_->UnRegisteredProxy(1);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(ImeSettingListenerTestImpl::WaitPanelShow());

    StopApp();
}

/**
* @tc.name: UnRegisteredProxyWithErrorType
* @tc.desc:
* @tc.type: FUNC
*/
HWTEST_F(ImeProxyTest, UnRegisteredProxyWithErrorType, TestSize.Level0)
{
    IMSA_HILOGI("ImeProxyTest::UnRegisteredProxyWithErrorType");
    InputMethodEngineListenerImpl::isEnable_ = true;
    auto ret = proxy_->RegisteredProxy();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = proxy_->UnRegisteredProxy(3);
    EXPECT_EQ(ret, ErrorCode::ERROR_BAD_PARAMETERS);
}

/**
* @tc.name: StopTheAppInProxyBindInPe
* @tc.desc:
* @tc.type: FUNC
*/
HWTEST_F(ImeProxyTest, StopTheAppInProxyBindInPe, TestSize.Level0)
{
    IMSA_HILOGI("ImeProxyTest::StopTheAppInProxyBindInPe");
    // open the app, click the edit box in pe, bind ima
    ImeSettingListenerTestImpl::ResetParam();
    StartApp();
    ClickEditorInPe();
    ImeSettingListenerTestImpl::WaitPanelShow();
    // RegisteredProxy in ima bind, unbind ima, bind proxy
    InputMethodEngineListenerImpl::isEnable_ = true;
    auto ret = proxy_->RegisteredProxy();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    // stop the app in proxy bind in pe, unbind proxy
    StopEditorInPe();
    ret = proxy_->InsertText("d");
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);

    ret = proxy_->UnRegisteredProxy(0);
}

/**
* @tc.name: StopTheAppInProxyBindInPc
* @tc.desc:
* @tc.type: FUNC
*/
HWTEST_F(ImeProxyTest, StopTheAppInProxyBindInPc, TestSize.Level0)
{
    IMSA_HILOGI("ImeProxyTest::StopTheAppInProxyBindInPc");
    // open the app, click the edit box in pe, bind ima
    ImeSettingListenerTestImpl::ResetParam();
    StartApp();
    ClickEditorInPe();
    ImeSettingListenerTestImpl::WaitPanelShow();
    // RegisteredProxy in ima bind, unbind ima, bind proxy
    InputMethodEngineListenerImpl::isEnable_ = true;
    auto ret = proxy_->RegisteredProxy();

    // stop the app in proxy bind in pc, unbind proxy
    StopEditorInPc();
    ret = proxy_->InsertText("d");
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);

    proxy_->UnRegisteredProxy(0);
}

/**
* @tc.name: ProxyAndImaSwitchTest
* @tc.desc:
* @tc.type: FUNC
*/
HWTEST_F(ImeProxyTest, ProxyAndImaSwitchTest, TestSize.Level0)
{
    IMSA_HILOGI("ImeProxyTest::ProxyAndImaSwitchTest");
    // open the app, click the edit box in pe, bind ima
    ImeSettingListenerTestImpl::ResetParam();
    StartApp();
    ClickEditorInPe();
    ImeSettingListenerTestImpl::WaitPanelShow();
    // RegisteredProxy in ima bind, unbind ima, bind proxy
    InputMethodEngineListenerImpl::isEnable_ = true;
    auto ret = proxy_->RegisteredProxy();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    // click the edit box in pc, rebind proxy
    ClickEditorInPc();
    ret = proxy_->InsertText("d");
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    // click the edit box in pe, bind ima
    ImeSettingListenerTestImpl::ResetParam();
    ClickEditorInPe();
    EXPECT_TRUE(ImeSettingListenerTestImpl::WaitPanelShow());
    ret = proxy_->InsertText("e");
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);
    // click the edit box in pe, rebind ima
    ClickEditorInPe();
    ret = proxy_->InsertText("f");
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);
    // click the edit box in pc, bind proxy
    ClickEditorInPc();
    EXPECT_TRUE(ImeSettingListenerTestImpl::WaitPanelHide());
    ret = proxy_->InsertText("g");
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    proxy_->UnRegisteredProxy(0);
    StopApp();
}

/**
* @tc.name: TextEditingTest
* @tc.desc:
* @tc.type: FUNC
*/
HWTEST_F(ImeProxyTest, TextEditingTest, TestSize.Level0)
{
    IMSA_HILOGI("ImeProxyTest::TextEditingTest");
    // open the app, click the edit box in pe, bind ima
    ImeSettingListenerTestImpl::ResetParam();
    StartApp();
    ClickEditorInPe();
    ImeSettingListenerTestImpl::WaitPanelShow();
    // RegisteredProxy in ima bind, unbind ima, bind proxy
    InputMethodEngineListenerImpl::isEnable_ = true;
    auto ret = proxy_->RegisteredProxy();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    KeyboardListenerTestImpl::ResetParam();
    ret = proxy_->InsertText("abc");
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(KeyboardListenerTestImpl::WaitSelectionChange(3));
    EXPECT_TRUE(KeyboardListenerTestImpl::WaitCursorUpdate());
    EXPECT_TRUE(KeyboardListenerTestImpl::WaitTextChange("abc"));

    KeyboardListenerTestImpl::ResetParam();
    ret = proxy_->DeleteForward(1);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(KeyboardListenerTestImpl::WaitSelectionChange(2));
    EXPECT_TRUE(KeyboardListenerTestImpl::WaitCursorUpdate());
    EXPECT_TRUE(KeyboardListenerTestImpl::WaitTextChange("ab"));

    KeyboardListenerTestImpl::ResetParam();
    ret = proxy_->MoveCursor(3); //left
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(KeyboardListenerTestImpl::WaitSelectionChange(1));
    EXPECT_TRUE(KeyboardListenerTestImpl::WaitCursorUpdate());
    EXPECT_TRUE(KeyboardListenerTestImpl::WaitTextChange("ab"));

    KeyboardListenerTestImpl::ResetParam();
    ret = proxy_->DeleteBackward(1);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(KeyboardListenerTestImpl::WaitSelectionChange(1));
    EXPECT_TRUE(KeyboardListenerTestImpl::WaitTextChange("a"));

    proxy_->UnRegisteredProxy(0);
    StopApp();
}
} // namespace MiscServices
} // namespace OHOS
