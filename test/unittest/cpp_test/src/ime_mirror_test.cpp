/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include <gtest/hwext/gtest-multithread.h>

#define private public
#include "input_method_ability.h"
#include "input_method_ability_interface.h"
#include "task_manager.h"
#undef private
#include "input_method_controller.h"
#include "input_method_engine_listener_impl.h"
#include "keyboard_listener_test_impl.h"
#include "scope_utils.h"
#include "sys_cfg_parser.h"
#include "text_listener.h"
#include "ime_setting_listener_test_impl.h"
#include "ime_event_monitor_manager_impl.h"

using namespace testing::ext;
using namespace testing::mt;
namespace OHOS {
namespace MiscServices {
constexpr int32_t INVALID_UID = -1;
class ImeMirrorTest : public testing::Test {
public:
    static sptr<InputMethodController> imc_;
    static bool isImeMirrorFeatureEnabled_;
    static int32_t agentUid_;
    static void SetUpTestCase(void)
    {
        IMSA_HILOGI("ImeMirrorTest::SetUpTestCase");
        TddUtil::StorageSelfTokenID();
        TddUtil::InitWindow(true);
        imc_ = InputMethodController::GetInstance();

        TddUtil::SetTestTokenID(TddUtil::AllocTestTokenID(true, "ImeProxyTest"));
        auto listener = std::make_shared<ImeSettingListenerTestImpl>();
        ImeEventMonitorManagerImpl::GetInstance().RegisterImeEventListener(
            EVENT_IME_HIDE_MASK | EVENT_IME_SHOW_MASK | EVENT_IME_CHANGE_MASK, listener);

        ImeSettingListenerTestImpl::ResetParam();
        TddUtil::SetTestTokenID(
            TddUtil::AllocTestTokenID(true, "ImeProxyTest", { "ohos.permission.CONNECT_IME_ABILITY" }));
        TddUtil::EnabledAllIme();
        SubProperty subProp;
        subProp.name = "com.example.testIme";
        subProp.id = "InputMethodExtAbility";
        auto ret = imc_->SwitchInputMethod(SwitchTrigger::CURRENT_IME, subProp.name, subProp.id);
        EXPECT_EQ(ret, ErrorCode::NO_ERROR);
        EXPECT_TRUE(ImeSettingListenerTestImpl::WaitImeChange(subProp));
        TddUtil::RestoreSelfTokenID();

        // native sa permission
        TddUtil::GrantNativePermission();
        SystemConfig systemConfig;
        SysCfgParser::ParseSystemConfig(systemConfig);
        isImeMirrorFeatureEnabled_ =
            systemConfig.supportedCapacityList.find("ime_mirror") != systemConfig.supportedCapacityList.end();
        if (isImeMirrorFeatureEnabled_) {
            if (systemConfig.proxyImeUidList.empty()) {
                isImeMirrorFeatureEnabled_ = false;
            }
            for (auto id : systemConfig.proxyImeUidList) {
                agentUid_ = id;
            }
        }
    }
    static void TearDownTestCase(void)
    {
        IMSA_HILOGI("ImeMirrorTest::TearDownTestCase");
        TddUtil::DestroyWindow();
        TddUtil::RestoreSelfTokenID();
    }
    void SetUp()
    {
        if (!isImeMirrorFeatureEnabled_) {
            GTEST_SKIP() << "ime mirror ime feature is not enabled";
        }
        IMSA_HILOGI("ImeMirrorTest::SetUp");
        InputMethodAbilityInterface::GetInstance().SetImeListener(std::make_shared<InputMethodEngineListenerImpl>());
        InputMethodAbilityInterface::GetInstance().SetKdListener(std::make_shared<KeyboardListenerTestImpl>());
        TaskManager::GetInstance().SetInited(true);
        InputMethodEngineListenerImpl::ResetParam();
        KeyboardListenerTestImpl::ResetParam();
    }
    void TearDown()
    {
        IMSA_HILOGI("ImeMirrorTest::TearDown");
        TaskManager::GetInstance().Reset();
    }

    static int32_t Attach(bool isShowKeyboard = true)
    {
        TextConfig config;
        config.cursorInfo = { .left = 0, .top = 1, .width = 0.5, .height = 1.2 };
        sptr<OnTextChangedListener> testListener = new TextListener();
        auto ret = imc_->Attach(testListener, isShowKeyboard, config);
        return ret;
    }
    static void Close()
    {
        imc_->Close();
    }

    static void AttachAndRegisterProxy()
    {
        {
            UidScope uidScope(ImeMirrorTest::agentUid_);
            auto ret = InputMethodAbilityInterface::GetInstance().BindImeMirror();
            EXPECT_EQ(ret, ErrorCode::NO_ERROR);
        }

        auto ret = Attach();
        EXPECT_EQ(ret, ErrorCode::NO_ERROR);
        EXPECT_TRUE(InputMethodEngineListenerImpl::WaitInputStart());
    }

    static void CloseAndUnregisterProxy()
    {
        Close();
        EXPECT_TRUE(InputMethodEngineListenerImpl::WaitInputFinish());
        {
            UidScope uidScope(ImeMirrorTest::agentUid_);
            auto ret = InputMethodAbilityInterface::GetInstance().UnbindImeMirror();
            EXPECT_EQ(ret, ErrorCode::NO_ERROR);
        }
    }
};
sptr<InputMethodController> ImeMirrorTest::imc_;
bool ImeMirrorTest::isImeMirrorFeatureEnabled_ { false };
int32_t ImeMirrorTest::agentUid_ { INVALID_UID };

/**
 * @tc.name: RegisterUnbindImeMirrorWithoutPermission_fail
 * @tc.desc: Register or unregister ime mirror without permission
 * @tc.type: FUNC
 */
HWTEST_F(ImeMirrorTest, RegisterUnbindImeMirrorWithoutPermission_fail, TestSize.Level1)
{
    IMSA_HILOGI("ImeMirrorTest::RegisterUnbindImeMirrorWithoutPermission_fail start");
    auto ret = InputMethodAbilityInterface::GetInstance().BindImeMirror();
    EXPECT_EQ(ret, ErrorCode::ERROR_NOT_AI_APP_IME);
    ret = InputMethodAbilityInterface::GetInstance().UnbindImeMirror();
    EXPECT_EQ(ret, ErrorCode::ERROR_NOT_AI_APP_IME);
}

/**
 * @tc.name: BindImeMirrorAndVerifyTextSelectionConfig_success
 * @tc.desc: Register ime mirror and verify text\selection\config success
 * @tc.type: FUNC
 */
HWTEST_F(ImeMirrorTest, BindImeMirrorAndVerifyTextSelectionConfig_success, TestSize.Level1)
{
    IMSA_HILOGI("ImeMirrorTest::BindImeMirrorAndVerifyTextSelectionConfig_success start");
    AttachAndRegisterProxy();
    auto instance = InputMethodAbilityInterface::GetInstance();
    auto ret = instance.InsertText("1234567890");
    EXPECT_EQ(ret, ErrorCode::ERROR_IMA_CHANNEL_NULLPTR);

    ret = ImeMirrorTest::imc_->OnSelectionChange(u"1234567890", 1, 1);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(KeyboardListenerTestImpl::WaitTextChange("1234567890"));
    EXPECT_TRUE(KeyboardListenerTestImpl::WaitSelectionChange(1));

    Configuration config;
    config.SetEnterKeyType(EnterKeyType::GO);
    ret = ImeMirrorTest::imc_->OnConfigurationChange(config);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputAttribute attr;
    attr.enterKeyType = static_cast<int32_t>(EnterKeyType::GO);
    EXPECT_FALSE(KeyboardListenerTestImpl::WaitEditorAttributeChange(attr));

    ret = ImeMirrorTest::imc_->SendFunctionKey(static_cast<int32_t>(EnterKeyType::NEW_LINE));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(KeyboardListenerTestImpl::WaitFunctionKey(static_cast<int32_t>(EnterKeyType::NEW_LINE)));

    CloseAndUnregisterProxy();
}

/**
 * @tc.name: BindImeMirrorAndVerifyPasswordTextHandling_fail
 * @tc.desc: Register ime mirror and verify password text handling fail
 * @tc.type: FUNC
 */
HWTEST_F(ImeMirrorTest, BindImeMirrorAndVerifyPasswordTextHandling_fail, TestSize.Level1)
{
    IMSA_HILOGI("ImeMirrorTest::BindImeMirrorAndVerifyPasswordTextHandling_fail start");
    AttachAndRegisterProxy();
    // Test multiple secure input types
    std::vector<TextInputType> secureTypes = { TextInputType::NEW_PASSWORD, TextInputType::NUMBER_PASSWORD,
        TextInputType::VISIBLE_PASSWORD, TextInputType::SCREEN_LOCK_PASSWORD };
    for (auto type : secureTypes) {
        Configuration config;
        config.SetTextInputType(type);
        auto ret = ImeMirrorTest::imc_->OnConfigurationChange(config);
        EXPECT_EQ(ret, ErrorCode::NO_ERROR);

        ret = ImeMirrorTest::imc_->OnSelectionChange(u"secure123", 1, 1);
        EXPECT_EQ(ret, ErrorCode::NO_ERROR);
        // Secure text should not be received by proxy
        EXPECT_FALSE(KeyboardListenerTestImpl::WaitTextChange("secure123"));

        ret = ImeMirrorTest::imc_->SendFunctionKey(static_cast<int32_t>(EnterKeyType::NEW_LINE));
        EXPECT_EQ(ret, ErrorCode::NO_ERROR);
        EXPECT_FALSE(KeyboardListenerTestImpl::WaitFunctionKey(static_cast<int32_t>(EnterKeyType::NEW_LINE)));
    }

    CloseAndUnregisterProxy();
}

/**
 * @tc.name: BindImeMirrorAndVerifyLongTextHandling_success
 * @tc.desc: Verify long text processing capability after registering ime mirror
 * @tc.type: FUNC
 */
HWTEST_F(ImeMirrorTest, BindImeMirrorAndVerifyLongTextHandling_success, TestSize.Level1)
{
    IMSA_HILOGI("ImeMirrorTest::BindImeMirrorAndVerifyLongTextHandling_success start");
    AttachAndRegisterProxy();

    // Generate extra-long text
    std::u16string longText;
    for (int i = 0; i < 1000; ++i) {
        longText += u"test";
    }
    auto ret = ImeMirrorTest::imc_->OnSelectionChange(longText, 500, 500);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    // Verify long text is received correctly
    EXPECT_TRUE(KeyboardListenerTestImpl::WaitTextChange(std::string(longText.begin(), longText.end())));

    CloseAndUnregisterProxy();
}

/**
 * @tc.name: RegisterAndUnbindImeMirrorImmediately_VerifyInputListenerInvalid_fail
 * @tc.desc: Register and immediately unregister ime mirror, verify input start/finish listener is invalid
 * @tc.type: FUNC
 */
HWTEST_F(ImeMirrorTest, RegisterAndUnbindImeMirrorImmediately_VerifyInputListenerInvalid_fail, TestSize.Level1)
{
    IMSA_HILOGI("ImeMirrorTest::RegisterAndUnbindImeMirrorImmediately_VerifyInputListenerInvalid_fail start");
    {
        UidScope uidScope(ImeMirrorTest::agentUid_);
        auto ret = InputMethodAbilityInterface::GetInstance().BindImeMirror();
        EXPECT_EQ(ret, ErrorCode::NO_ERROR);
        ret = InputMethodAbilityInterface::GetInstance().UnbindImeMirror();
        EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    }

    auto ret = Attach();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_FALSE(InputMethodEngineListenerImpl::WaitInputStart());
    Close();
    EXPECT_FALSE(InputMethodEngineListenerImpl::WaitInputFinish());
}

/**
 * @tc.name: UnregisterProxyDuringTextInput_success
 * @tc.desc: Unregister proxy during text input and verify no further text reception
 * @tc.type: FUNC
 */
HWTEST_F(ImeMirrorTest, UnregisterProxyDuringTextInput_success, TestSize.Level1)
{
    IMSA_HILOGI("ImeMirrorTest::UnregisterProxyDuringTextInput_success start");
    AttachAndRegisterProxy();

    // Send text before unregistration
    auto ret = imc_->OnSelectionChange(u"before unregister", 0, 0);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(KeyboardListenerTestImpl::WaitTextChange("before unregister"));

    // Unregister proxy
    {
        UidScope uidScope(agentUid_);
        ret = InputMethodAbilityInterface::GetInstance().UnbindImeMirror();
        EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    }

    // Send text after unregistration, proxy should not receive it
    ret = imc_->OnSelectionChange(u"after unregister", 0, 0);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_FALSE(KeyboardListenerTestImpl::WaitTextChange("after unregister"));

    InputMethodEngineListenerImpl::ResetParam();
    Close();
    EXPECT_FALSE(InputMethodEngineListenerImpl::WaitInputFinish());
}

/**
 * @tc.name: RepeatBindImeMirror_success
 * @tc.desc: Test multiple registrations of ime mirror, verify duplicate registration handling logic
 * @tc.type: FUNC
 */
HWTEST_F(ImeMirrorTest, RepeatBindImeMirror_success, TestSize.Level1)
{
    IMSA_HILOGI("ImeMirrorTest::RepeatBindImeMirror_success start");
    {
        UidScope uidScope(ImeMirrorTest::agentUid_);
        // First registration should succeed
        auto ret = InputMethodAbilityInterface::GetInstance().BindImeMirror();
        EXPECT_EQ(ret, ErrorCode::NO_ERROR);
        // Duplicate registration should return success (idempotent handling)
        ret = InputMethodAbilityInterface::GetInstance().BindImeMirror();
        EXPECT_EQ(ret, ErrorCode::NO_ERROR);
        // Unregister once
        ret = InputMethodAbilityInterface::GetInstance().UnbindImeMirror();
        EXPECT_EQ(ret, ErrorCode::NO_ERROR);
        // Unregister again
        ret = InputMethodAbilityInterface::GetInstance().UnbindImeMirror();
        EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    }
}

/**
 * @tc.name: RegisterAfterAttach_success
 * @tc.desc: Verify proxy registration works correctly when Register is called after Attach
 * @tc.type: FUNC
 */
HWTEST_F(ImeMirrorTest, RegisterAfterAttach_success, TestSize.Level1)
{
    IMSA_HILOGI("ImeMirrorTest::RegisterAfterAttach_success start");

    // Step 1: Perform Attach first without registration
    auto ret = Attach();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_FALSE(InputMethodEngineListenerImpl::WaitInputStart());

    // Step 2: Register proxy after Attach
    {
        UidScope uidScope(agentUid_);
        ret = InputMethodAbilityInterface::GetInstance().BindImeMirror();
        EXPECT_EQ(ret, ErrorCode::NO_ERROR);
        EXPECT_TRUE(InputMethodEngineListenerImpl::WaitInputStart());
    }

    // Step 3: Verify text can be received after post-attach registration
    ret = imc_->OnSelectionChange(u"register after attach", 0, 0);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(KeyboardListenerTestImpl::WaitTextChange("register after attach"));

    // Cleanup
    {
        UidScope uidScope(ImeMirrorTest::agentUid_);
        auto ret = InputMethodAbilityInterface::GetInstance().UnbindImeMirror();
        EXPECT_EQ(ret, ErrorCode::NO_ERROR);
        EXPECT_TRUE(InputMethodEngineListenerImpl::WaitInputFinish());
    }
    Close();
}

/**
 * @tc.name: BindImeMirror_WillNotChangeClientAndImeBinding
 * @tc.desc: BindImeMirror should not change client and ime binding relationship
 * @tc.type: FUNC
 */
HWTEST_F(ImeMirrorTest, BindImeMirror_WillNotChangeClientAndImeBinding, TestSize.Level1)
{
    IMSA_HILOGI("ImeMirrorTest::BindImeMirror_WillNotChangeClientAndImeBinding start");

    // Step 1: Perform Attach first without bind
    auto ret = Attach(false);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_FALSE(InputMethodEngineListenerImpl::WaitInputStart());

    // Step 2: bind ime mirror after Attach
    {
        UidScope uidScope(agentUid_);
        ret = InputMethodAbilityInterface::GetInstance().BindImeMirror();
        EXPECT_EQ(ret, ErrorCode::NO_ERROR);
        EXPECT_TRUE(InputMethodEngineListenerImpl::WaitInputStart());
    }

    ret = imc_->ShowTextInput();
    EXPECT_FALSE(InputMethodEngineListenerImpl::WaitKeyboardStatus(true));

    CloseAndUnregisterProxy();
}

/**
 * @tc.name: multiThreadAttachRegisterTest_001
 * @tc.desc: test ime Attach and Register ime mirror ime in multi-thread
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ImeMirrorTest, multiThreadAttachRegisterTest_001, TestSize.Level1)
{
    IMSA_HILOGI("ImeMirrorTest::multiThreadAttachRegisterTest_001");
    SET_THREAD_NUM(1);
    auto attachTask = []() {
        auto ret = ImeMirrorTest::Attach();
        EXPECT_EQ(ret, ErrorCode::NO_ERROR);
        ret = ImeMirrorTest::imc_->OnSelectionChange(u"1234567890", 1, 1);
        EXPECT_EQ(ret, ErrorCode::NO_ERROR);
        ImeMirrorTest::Close();
    };

    auto registerTask = []() {
        UidScope uidScope(ImeMirrorTest::agentUid_);
        auto ret = InputMethodAbilityInterface::GetInstance().BindImeMirror();
        EXPECT_EQ(ret, ErrorCode::NO_ERROR);
        EXPECT_TRUE(InputMethodEngineListenerImpl::WaitInputStart());
        EXPECT_TRUE(KeyboardListenerTestImpl::WaitTextChange("1234567890"));
        ret = InputMethodAbilityInterface::GetInstance().UnbindImeMirror();
        EXPECT_EQ(ret, ErrorCode::NO_ERROR);
        EXPECT_TRUE(InputMethodEngineListenerImpl::WaitInputFinish());
    };
    MTEST_ADD_TASK(RANDOM_THREAD_ID, attachTask);
    MTEST_ADD_TASK(RANDOM_THREAD_ID, registerTask);
}

/**
 * @tc.name: multiThreadAttachRegisterTest_002
 * @tc.desc: test ime Attach and Register ime mirror ime in multi-thread
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ImeMirrorTest, multiThreadAttachRegisterTest_002, TestSize.Level1)
{
    IMSA_HILOGI("ImeMirrorTest::multiThreadAttachRegisterTest_002");
    MTEST_POST_RUN();
}
} // namespace MiscServices
} // namespace OHOS