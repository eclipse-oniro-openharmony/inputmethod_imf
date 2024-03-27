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
#include "ime_event_monitor_manager.h"
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
#include <vector>

#include "input_method_system_ability.h"
#include "input_method_controller.h"
#include "ability_manager_client.h"
#include "block_data.h"
#include "global.h"
#include "input_window_info.h"
#include "i_input_method_agent.h"
#include "i_input_method_system_ability.h"
#include "if_system_ability_manager.h"
#include "input_client_stub.h"
#include "input_data_channel_stub.h"
#include "input_death_recipient.h"
#include "input_method_ability.h"
#include "ime_setting_listener_test_impl.h"
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
class ImeEventMonitorManagerTest : public testing::Test {
public:
    class IdentityCheckerMock : public IdentityChecker {
    public:
        IdentityCheckerMock() = default;
        virtual ~IdentityCheckerMock() = default;
        bool IsSystemApp(uint64_t fullTokenId) override
        {
            return isSystemApp_;
        }
        bool IsNativeSa(Security::AccessToken::AccessTokenID tokenId) override
        {
            return isNativeSa_;
        }
        static bool isSystemApp_;
        static bool isNativeSa_;
    };
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static sptr<InputMethodSystemAbility> service_;
    static std::shared_ptr<IdentityCheckerMock> identityCheckerMock_;
    static std::shared_ptr<IdentityCheckerImpl> identityCheckerImpl_;
    static sptr<InputMethodController> inputMethodController_;
    static sptr<InputMethodAbility> inputMethodAbility_;
    static sptr<ImeEventMonitorManager> imeEventMonitorManager_;
    bool ImeEventMonitorManagerTest::IdentityCheckerMock::isSystemApp_ = false;
    bool ImeEventMonitorManagerTest::IdentityCheckerMock::isNativeSa_ = false;
};

sptr<InputMethodController> ImeEventMonitorManagerTest::inputMethodController_;
sptr<ImeEventMonitorManager> ImeEventMonitorManagerTest::imeEventMonitorManager_;
sptr<InputMethodAbility> ImeEventMonitorManagerTest::inputMethodAbility_;

void ImeEventMonitorManagerTest::SetUpTestCase(void)
{
    IMSA_HILOGI("ImeEventMonitorManagerTest::SetUpTestCase");
    std::shared_ptr<Property> property = InputMethodController::GetInstance()->GetCurrentInputMethod();
    std::string bundleName = property != nullptr ? property->name : "default.inputmethod.unittest";
    TddUtil::SetTestTokenID(TddUtil::GetTestTokenID(bundleName));
    inputMethodAbility_ = InputMethodAbility::GetInstance();
    inputMethodController_ = InputMethodController::GetInstance();
    imeEventMonitorManager_ = ImeEventMonitorManager::GetInstance();
}

void ImeEventMonitorManagerTest::TearDownTestCase(void)
{
    IMSA_HILOGI("ImeEventMonitorManagerTest::TearDownTestCase");
}

void ImeEventMonitorManagerTest::SetUp(void)
{
    IMSA_HILOGI("ImeEventMonitorManagerTest::SetUp");
}

void ImeEventMonitorManagerTest::TearDown(void)
{
    IMSA_HILOGI("ImeEventMonitorManagerTest::TearDown");
}

/**
 * @tc.name: testImcPanelListening_001
 * @tc.desc: IMC ImcPanelListening.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ImeEventMonitorManagerTest, testImcPanelListening_001, TestSize.Level0)
{
    std::shared_ptr<ImeSettingListenerTestImpl> listener_ = new ImeEventListener();
    ImeEventMonitorManagerTest::IdentityCheckerMock::isNativeSa_ = true;
    int32_t ret = InputMethodEditorTest::inputMethodController_->ShowSoftKeyboard();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    imeEventMonitorManager_->RegisterImeEventListener({ EventType::IME_SHOW }, listener_);
    auto imsa = ImaUtils::GetImsaProxy();
    if (imsa == nullptr) {
        IMSA_HILOGE("imsa is nullptr");
        return;
    }
    ImeWindowInfo info;
    info.panelInfo = { PanelType::SOFT_KEYBOARD, PanelFlag::FLG_FIXED };
    imsa->PanelStatusChange(InputWindowStatus::SHOW, info);
    EXPECT_TRUE(ImeSettingListenerTestImpl::WaitPanelShow());
}

/**
 * @tc.name: testImcPanelListening_002
 * @tc.desc: IMC ImcPanelListening.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ImeEventMonitorManagerTest, testImcPanelListening_002, TestSize.Level0)
{
    std::shared_ptr<ImeSettingListenerTestImpl> listener_ = new ImeEventListener();
    ImeEventMonitorManagerTest::IdentityCheckerMock::isNativeSa_ = true;
    int32_t ret = InputMethodEditorTest::inputMethodController_->ShowSoftKeyboard();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    imeEventMonitorManager_->RegisterImeEventListener({ EventType::IME_SHOW }, listener_);
    auto imsa = ImaUtils::GetImsaProxy();
    if (imsa == nullptr) {
        IMSA_HILOGE("imsa is nullptr");
        return;
    }
    ImeWindowInfo info;
    info.panelInfo = { PanelType::SOFT_KEYBOARD, PanelFlag::FLG_FLOATING };
    imsa->PanelStatusChange(InputWindowStatus::SHOW, info);
    EXPECT_TRUE(ImeSettingListenerTestImpl::WaitPanelShow());
}

/**
 * @tc.name: testImcPanelListening_003
 * @tc.desc: IMC ImcPanelListening.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ImeEventMonitorManagerTest, testImcPanelListening_003, TestSize.Level0)
{
    std::shared_ptr<ImeSettingListenerTestImpl> listener_ = new ImeEventListener();
    ImeEventMonitorManagerTest::IdentityCheckerMock::isNativeSa_ = true;
    int32_t ret = InputMethodEditorTest::inputMethodController_->ShowSoftKeyboard();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    imeEventMonitorManager_->RegisterImeEventListener({ EventType::IME_SHOW }, listener_);
    auto imsa = ImaUtils::GetImsaProxy();
    if (imsa == nullptr) {
        IMSA_HILOGE("imsa is nullptr");
        return;
    }
    ImeWindowInfo info;
    info.panelInfo = { PanelType::SOFT_KEYBOARD, PanelFlag::FLG_CANDIDATE_COLUMN };
    imsa->PanelStatusChange(InputWindowStatus::SHOW, info);
    EXPECT_FALSE(ImeSettingListenerTestImpl::WaitPanelShow());
}

/**
 * @tc.name: testImcPanelListening_004
 * @tc.desc: IMC ImcPanelListening.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ImeEventMonitorManagerTest, testImcPanelListening_004, TestSize.Level0)
{
    std::shared_ptr<ImeSettingListenerTestImpl> listener_ = new ImeEventListener();
    ImeEventMonitorManagerTest::IdentityCheckerMock::isNativeSa_ = true;
    int32_t ret = InputMethodEditorTest::inputMethodController_->ShowSoftKeyboard();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    imeEventMonitorManager_->RegisterImeEventListener({ EventType::IME_SHOW }, listener_);
    auto imsa = ImaUtils::GetImsaProxy();
    if (imsa == nullptr) {
        IMSA_HILOGE("imsa is nullptr");
        return;
    }
    ImeWindowInfo info;
    info.panelInfo = { PanelType::STATUS_BAR, PanelFlag::FLG_CANDIDATE_COLUMN };
    imsa->PanelStatusChange(InputWindowStatus::SHOW, info);
    EXPECT_FALSE(ImeSettingListenerTestImpl::WaitPanelShow());
}

/**
 * @tc.name: testImcPanelListening_005
 * @tc.desc: IMC ImcPanelListening.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ImeEventMonitorManagerTest, testImcPanelListening_005, TestSize.Level0)
{
    std::shared_ptr<ImeSettingListenerTestImpl> listener_ = new ImeEventListener();
    ImeEventMonitorManagerTest::IdentityCheckerMock::isSystemApp_ = true;
    int32_t ret = InputMethodEditorTest::inputMethodController_->ShowSoftKeyboard();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    imeEventMonitorManager_->RegisterImeEventListener({ EventType::IME_SHOW }, listener_);
    auto imsa = ImaUtils::GetImsaProxy();
    if (imsa == nullptr) {
        IMSA_HILOGE("imsa is nullptr");
        return;
    }
    ImeWindowInfo info;
    info.panelInfo = { PanelType::SOFT_KEYBOARD, PanelFlag::FLG_FIXED };
    imsa->PanelStatusChange(InputWindowStatus::SHOW, info);
    EXPECT_TRUE(ImeSettingListenerTestImpl::WaitPanelShow());
}

/**
 * @tc.name: testImcPanelListening_006
 * @tc.desc: IMC ImcPanelListening.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ImeEventMonitorManagerTest, testImcPanelListening_006, TestSize.Level0)
{
    std::shared_ptr<ImeSettingListenerTestImpl> listener_ = new ImeEventListener();
    ImeEventMonitorManagerTest::IdentityCheckerMock::isSystemApp_ = true;
    int32_t ret = InputMethodEditorTest::inputMethodController_->ShowSoftKeyboard();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    imeEventMonitorManager_->RegisterImeEventListener({ EventType::IME_SHOW }, listener_);
    auto imsa = ImaUtils::GetImsaProxy();
    if (imsa == nullptr) {
        IMSA_HILOGE("imsa is nullptr");
        return;
    }
    ImeWindowInfo info;
    info.panelInfo = { PanelType::SOFT_KEYBOARD, PanelFlag::FLG_FLOATING };
    imsa->PanelStatusChange(InputWindowStatus::SHOW, info);
    EXPECT_TRUE(ImeSettingListenerTestImpl::WaitPanelShow());
}

/**
 * @tc.name: testImcPanelListening_007
 * @tc.desc: IMC ImcPanelListening.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ImeEventMonitorManagerTest, testImcPanelListening_007, TestSize.Level0)
{
    std::shared_ptr<ImeSettingListenerTestImpl> listener_ = new ImeEventListener();
    ImeEventMonitorManagerTest::IdentityCheckerMock::isSystemApp_ = true;
    int32_t ret = InputMethodEditorTest::inputMethodController_->ShowSoftKeyboard();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    imeEventMonitorManager_->RegisterImeEventListener({ EventType::IME_SHOW }, listener_);
    auto imsa = ImaUtils::GetImsaProxy();
    if (imsa == nullptr) {
        IMSA_HILOGE("imsa is nullptr");
        return;
    }
    ImeWindowInfo info;
    info.panelInfo = { PanelType::SOFT_KEYBOARD, PanelFlag::FLG_CANDIDATE_COLUMN };
    imsa->PanelStatusChange(InputWindowStatus::SHOW, info);
    EXPECT_FALSE(ImeSettingListenerTestImpl::WaitPanelShow());
}

/**
 * @tc.name: testImcPanelListening_008
 * @tc.desc: IMC ImcPanelListening.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ImeEventMonitorManagerTest, testImcPanelListening_008, TestSize.Level0)
{
    std::shared_ptr<ImeSettingListenerTestImpl> listener_ = new ImeEventListener();
    ImeEventMonitorManagerTest::IdentityCheckerMock::isSystemApp_ = true;
    int32_t ret = InputMethodEditorTest::inputMethodController_->ShowSoftKeyboard();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    imeEventMonitorManager_->RegisterImeEventListener({ EventType::IME_SHOW }, listener_);
    auto imsa = ImaUtils::GetImsaProxy();
    if (imsa == nullptr) {
        IMSA_HILOGE("imsa is nullptr");
        return;
    }
    ImeWindowInfo info;
    info.panelInfo = { PanelType::STATUS_BAR, PanelFlag::FLG_CANDIDATE_COLUMN };
    imsa->PanelStatusChange(InputWindowStatus::SHOW, info);
    EXPECT_FALSE(ImeSettingListenerTestImpl::WaitPanelShow());
}

/**
 * @tc.name: testImcPanelListening_009
 * @tc.desc: IMC ImcPanelListening.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ImeEventMonitorManagerTest, testImcPanelListening_009, TestSize.Level0)
{
    std::shared_ptr<ImeSettingListenerTestImpl> listener1_ = new ImeEventListener();
    std::shared_ptr<ImeSettingListenerTestImpl> listener2_ = new ImeEventListener();
    std::shared_ptr<ImeSettingListenerTestImpl> listener3_ = new ImeEventListener();
    ImeEventMonitorManagerTest::IdentityCheckerMock::isNativeSa_ = true;
    int32_t ret = InputMethodEditorTest::inputMethodController_->ShowSoftKeyboard();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    imeEventMonitorManager_->RegisterImeEventListener({ EventType::IME_SHOW }, listener1_);
    imeEventMonitorManager_->RegisterImeEventListener({ EventType::IME_SHOW }, listener2_);
    imeEventMonitorManager_->RegisterImeEventListener({ EventType::IME_SHOW }, listener3_);
    auto imsa = ImaUtils::GetImsaProxy();
    if (imsa == nullptr) {
        IMSA_HILOGE("imsa is nullptr");
        return;
    }
    ImeWindowInfo info;
    info.panelInfo = { PanelType::SOFT_KEYBOARD, PanelFlag::FLG_FIXED };
    imsa->PanelStatusChange(InputWindowStatus::SHOW, info);
    EXPECT_TRUE(ImeSettingListenerTestImpl::WaitPanelShow());
}

/**
 * @tc.name: testImcPanelListening_010
 * @tc.desc: IMC ImcPanelListening.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ImeEventMonitorManagerTest, testImcPanelListening_010, TestSize.Level0)
{
    std::shared_ptr<ImeSettingListenerTestImpl> listener1_ = new ImeEventListener();
    std::shared_ptr<ImeSettingListenerTestImpl> listener2_ = new ImeEventListener();
    std::shared_ptr<ImeSettingListenerTestImpl> listener3_ = new ImeEventListener();
    ImeEventMonitorManagerTest::IdentityCheckerMock::isNativeSa_ = true;
    int32_t ret = InputMethodEditorTest::inputMethodController_->ShowSoftKeyboard();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    imeEventMonitorManager_->RegisterImeEventListener({ EventType::IME_SHOW }, listener1_);
    imeEventMonitorManager_->RegisterImeEventListener({ EventType::IME_SHOW }, listener2_);
    imeEventMonitorManager_->RegisterImeEventListener({ EventType::IME_SHOW }, listener3_);
    imeEventMonitorManager_->UnRegisterImeEventListener({ EventType::IME_SHOW }, listener3_);
    auto imsa = ImaUtils::GetImsaProxy();
    if (imsa == nullptr) {
        IMSA_HILOGE("imsa is nullptr");
        return;
    }
    ImeWindowInfo info;
    info.panelInfo = { PanelType::SOFT_KEYBOARD, PanelFlag::FLG_FIXED };
    imsa->PanelStatusChange(InputWindowStatus::SHOW, info);
    InputMethodEngineListenerImpl::ResetParam();
    InputMethodEngineListenerImpl::isEnable_ = true;
    InputMethodAbilityInterface::GetInstance().RegisteredProxy();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(ImeSettingListenerTestImpl::WaitPanelShow());
}

/**
 * @tc.name: testImcPanelListening_011
 * @tc.desc: IMC ImcPanelListening.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ImeEventMonitorManagerTest, testImcPanelListening_011, TestSize.Level0)
{
    std::shared_ptr<ImeSettingListenerTestImpl> listener1_ = new ImeEventListener();
    std::shared_ptr<ImeSettingListenerTestImpl> listener2_ = new ImeEventListener();
    std::shared_ptr<ImeSettingListenerTestImpl> listener3_ = new ImeEventListener();
    ImeEventMonitorManagerTest::IdentityCheckerMock::isNativeSa_ = true;
    int32_t ret = InputMethodEditorTest::inputMethodController_->ShowSoftKeyboard();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    imeEventMonitorManager_->RegisterImeEventListener({ EventType::IME_SHOW }, listener1_);
    imeEventMonitorManager_->RegisterImeEventListener({ EventType::IME_SHOW }, listener2_);
    imeEventMonitorManager_->RegisterImeEventListener({ EventType::IME_SHOW }, listener3_);
    imeEventMonitorManager_->UnRegisterImeEventListener({ EventType::IME_SHOW }, listener1_);
    imeEventMonitorManager_->UnRegisterImeEventListener({ EventType::IME_SHOW }, listener2_);
    imeEventMonitorManager_->UnRegisterImeEventListener({ EventType::IME_SHOW }, listener3_);
    auto imsa = ImaUtils::GetImsaProxy();
    if (imsa == nullptr) {
        IMSA_HILOGE("imsa is nullptr");
        return;
    }
    ImeWindowInfo info;
    info.panelInfo = { PanelType::SOFT_KEYBOARD, PanelFlag::FLG_FIXED };
    imsa->PanelStatusChange(InputWindowStatus::SHOW, info);
    EXPECT_FALSE(ImeSettingListenerTestImpl::WaitPanelShow());
}

/**
 * @tc.name: testImcPanelListening_012
 * @tc.desc: IMC ImcPanelListening.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ImeEventMonitorManagerTest, testImcPanelListening_012, TestSize.Level0)
{
    std::shared_ptr<ImeSettingListenerTestImpl> listener1_ = new ImeEventListener();
    std::shared_ptr<ImeSettingListenerTestImpl> listener2_ = new ImeEventListener();
    ImeEventMonitorManagerTest::IdentityCheckerMock::isNativeSa_ = true;
    ImeEventMonitorManagerTest::IdentityCheckerMock::isSystemApp_ = true;
    int32_t ret = InputMethodEditorTest::inputMethodController_->ShowSoftKeyboard();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    imeEventMonitorManager_->RegisterImeEventListener({ EventType::IME_SHOW }, listener1_);
    imeEventMonitorManager_->RegisterImeEventListener({ EventType::IME_SHOW }, listener2_);
    auto imsa = ImaUtils::GetImsaProxy();
    if (imsa == nullptr) {
        IMSA_HILOGE("imsa is nullptr");
        return;
    }
    ImeWindowInfo info;
    info.panelInfo = { PanelType::SOFT_KEYBOARD, PanelFlag::FLG_FIXED };
    imsa->PanelStatusChange(InputWindowStatus::SHOW, info);
    EXPECT_TRUE(ImeSettingListenerTestImpl::WaitPanelShow());
}

/**
 * @tc.name: testImcPanelListening_013
 * @tc.desc: IMC ImcPanelListening.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ImeEventMonitorManagerTest, testImcPanelListening_013, TestSize.Level0)
{
    std::shared_ptr<ImeSettingListenerTestImpl> listener1_ = new ImeEventListener();
    std::shared_ptr<ImeSettingListenerTestImpl> listener2_ = new ImeEventListener();
    ImeEventMonitorManagerTest::IdentityCheckerMock::isNativeSa_ = true;
    int32_t ret = InputMethodEditorTest::inputMethodController_->ShowSoftKeyboard();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    imeEventMonitorManager_->RegisterImeEventListener({ EventType::IME_SHOW }, listener1_);
    imeEventMonitorManager_->RegisterImeEventListener({ EventType::IME_HIDE }, listener2_);
    auto imsa = ImaUtils::GetImsaProxy();
    if (imsa == nullptr) {
        IMSA_HILOGE("imsa is nullptr");
        return;
    }
    ImeWindowInfo info;
    info.panelInfo = { PanelType::SOFT_KEYBOARD, PanelFlag::FLG_FIXED };
    imsa->PanelStatusChange(InputWindowStatus::SHOW, info);
    EXPECT_TRUE(ImeSettingListenerTestImpl::WaitPanelShow());
}

/**
 * @tc.name: testImcPanelListening_014
 * @tc.desc: IMC ImcPanelListening.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ImeEventMonitorManagerTest, testImcPanelListening_014, TestSize.Level0)
{
    std::shared_ptr<ImeSettingListenerTestImpl> listener1_ = new ImeEventListener();
    std::shared_ptr<ImeSettingListenerTestImpl> listener2_ = new ImeEventListener();
    ImeEventMonitorManagerTest::IdentityCheckerMock::isNativeSa_ = true;
    int32_t ret = InputMethodEditorTest::inputMethodController_->ShowSoftKeyboard();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    imeEventMonitorManager_->RegisterImeEventListener({ EventType::IME_SHOW }, listener1_);
    imeEventMonitorManager_->RegisterImeEventListener({ EventType::IME_HIDE }, listener2_);
    imeEventMonitorManager_->UnRegisterImeEventListener({ EventType::IME_SHOW }, listener1_);
    imeEventMonitorManager_->UnRegisterImeEventListener({ EventType::IME_HIDE }, listener2_);
    auto imsa = ImaUtils::GetImsaProxy();
    if (imsa == nullptr) {
        IMSA_HILOGE("imsa is nullptr");
        return;
    }
    ImeWindowInfo info;
    info.panelInfo = { PanelType::SOFT_KEYBOARD, PanelFlag::FLG_FIXED };
    imsa->PanelStatusChange(InputWindowStatus::SHOW, info);
    EXPECT_FALSE(ImeSettingListenerTestImpl::WaitPanelShow());
}
} // namespace MiscServices
} // namespace OHOS