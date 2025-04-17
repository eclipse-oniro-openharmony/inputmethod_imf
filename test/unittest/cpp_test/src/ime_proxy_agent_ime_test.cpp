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

#define private public
#include "input_method_ability.h"
#include "task_manager.h"
#undef private

#include <gtest/gtest.h>

#include "ability_manager_client.h"
#include "global.h"
#include "ime_event_monitor_manager_impl.h"
#include "ime_setting_listener_test_impl.h"
#include "input_method_ability_interface.h"
#include "input_method_controller.h"
#include "input_method_engine_listener_impl.h"
#include "input_method_types.h"
#include "keyboard_listener_test_impl.h"
#include "scope_utils.h"
#include "sys_cfg_parser.h"
#include "tdd_util.h"
#include "text_listener.h"
using namespace testing::ext;
namespace OHOS {
namespace MiscServices {
constexpr uint64_t AGENT_IME_DISPLAY_ID = 666;
constexpr int32_t INVALID_UID = -1;
class ImeProxyAgentImeTest : public testing::Test {
public:
    static sptr<InputMethodController> imc_;
    static bool isAgentFeatureEnabled_;
    static int32_t agentUid_;
    static void SetUpTestCase(void)
    {
        IMSA_HILOGI("ImeProxyAgentImeTest::SetUpTestCase");
        TddUtil::StorageSelfTokenID();
        TddUtil::InitWindow(false);
        imc_ = InputMethodController::GetInstance();
        RegisterImeSettingListener();
        // native sa permission
        TddUtil::GrantNativePermission();
        SystemConfig systemConfig;
        SysCfgParser::ParseSystemConfig(systemConfig);
        isAgentFeatureEnabled_ = systemConfig.enableAppAgentFeature;
        if (isAgentFeatureEnabled_) {
            if (systemConfig.proxyImeUidList.empty()) {
                isAgentFeatureEnabled_ = false;
            }
            for (auto id : systemConfig.proxyImeUidList) {
                agentUid_ = id;
            }
        }
    }
    static void TearDownTestCase(void)
    {
        IMSA_HILOGI("ImeProxyAgentImeTest::TearDownTestCase");
        TddUtil::DestroyWindow();
        TddUtil::RestoreSelfTokenID();
        TddUtil::KillImsaProcess();
    }
    void SetUp()
    {
        IMSA_HILOGI("ImeProxyAgentImeTest::SetUp");
        InputMethodAbilityInterface::GetInstance().SetImeListener(std::make_shared<InputMethodEngineListenerImpl>());
        InputMethodAbilityInterface::GetInstance().SetKdListener(std::make_shared<KeyboardListenerTestImpl>());
        TaskManager::GetInstance().SetInited(true);
    }
    void TearDown()
    {
        IMSA_HILOGI("InputMethodAbilityTest::TearDown");
        std::this_thread::sleep_for(std::chrono::seconds(1));
        TaskManager::GetInstance().Reset();
    }

    static int32_t Attach()
    {
        TextConfig config;
        config.cursorInfo = { .left = 0, .top = 1, .width = 0.5, .height = 1.2 };
        sptr<OnTextChangedListener> testListener = new TextListener();
        auto ret = imc_->Attach(testListener, true, config);
        return ret;
    }

    static void Close()
    {
        imc_->Close();
    }

private:
    static void RegisterImeSettingListener()
    {
        TddUtil::StorageSelfTokenID();
        TddUtil::SetTestTokenID(TddUtil::AllocTestTokenID(true, "ImeProxyAgentImeTest"));
        auto listener = std::make_shared<ImeSettingListenerTestImpl>();
        ImeEventMonitorManagerImpl::GetInstance().RegisterImeEventListener(
            EVENT_IME_HIDE_MASK | EVENT_IME_SHOW_MASK | EVENT_IME_CHANGE_MASK, listener);
        TddUtil::RestoreSelfTokenID();
    }
};
sptr<InputMethodController> ImeProxyAgentImeTest::imc_;
bool ImeProxyAgentImeTest::isAgentFeatureEnabled_{ false };
int32_t ImeProxyAgentImeTest::agentUid_{ INVALID_UID };

/**
 * @tc.name: testRegisterProxyIme_001
 * @tc.desc: agent feature enalbed, invalid uid
 * @tc.type: FUNC
 */
HWTEST_F(ImeProxyAgentImeTest, testRegisterProxyIme_001, TestSize.Level1)
{
    IMSA_HILOGI("ImeProxyAgentImeTest::testRegisterProxyIme_001 start");
    if (!ImeProxyAgentImeTest::isAgentFeatureEnabled_) {
        EXPECT_EQ(ImeProxyAgentImeTest::agentUid_, INVALID_UID);
    } else {
        auto ret = InputMethodAbilityInterface::GetInstance().RegisterProxyIme(AGENT_IME_DISPLAY_ID);
        EXPECT_EQ(ret, ErrorCode::ERROR_NOT_AI_APP_IME);
        InputMethodAbilityInterface::GetInstance().UnregisterProxyIme(AGENT_IME_DISPLAY_ID);
        EXPECT_EQ(ret, ErrorCode::ERROR_NOT_AI_APP_IME);
    }
}

/**
 * @tc.name: testRegisterProxyIme_002
 * @tc.desc: agent feature enabled, valid uid
 * @tc.type: FUNC
 */
HWTEST_F(ImeProxyAgentImeTest, testRegisterProxyIme_002, TestSize.Level1)
{
    IMSA_HILOGI("ImeProxyAgentImeTest::testRegisterProxyIme_002 start");
    if (!ImeProxyAgentImeTest::isAgentFeatureEnabled_) {
        EXPECT_EQ(ImeProxyAgentImeTest::agentUid_, INVALID_UID);
    } else {
        UidScope scope(ImeProxyAgentImeTest::agentUid_);
        auto ret = InputMethodAbilityInterface::GetInstance().RegisterProxyIme(AGENT_IME_DISPLAY_ID);
        EXPECT_EQ(ret, ErrorCode::NO_ERROR);
        ret = InputMethodAbilityInterface::GetInstance().UnregisterProxyIme(AGENT_IME_DISPLAY_ID);
        EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    }
}
} // namespace MiscServices
} // namespace OHOS
