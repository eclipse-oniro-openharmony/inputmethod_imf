/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "identity_checker.h"
#include "im_common_event_manager.h"
#include "ime_cfg_manager.h"
#include "ime_info_inquirer.h"
#include "input_method_system_ability.h"
#include "input_type_manager.h"
#include "mock_identity_checker.h"
#include "mock_im_common_event_manager.h"
#include "mock_ime_cfg_manager.h"
#include "mock_ime_info_inquirer.h"
#include "mock_input_type_manager.h"
#include "mock_ipc_skeleton.h"
#include "mock_user_session.h"
#include "user_session_manager.h"


using namespace testing;
using namespace OHOS::MMI;

class InputMethodSystemAbilityTest : public Test {
protected:
    void SetUp() override
    {
        // 设置模拟对象
        userSessionManager = std::make_shared<MockUserSessionManager>();
        identityChecker = std::make_shared<MockIdentityChecker>();
        imCommonEventManager = std::make_shared<MockImCommonEventManager>();
        imeInfoInquirer = std::make_shared<MockImeInfoInquirer>();
        inputTypeManager = std::make_shared<MockInputTypeManager>();
        imeCfgManager = std::make_shared<MockImeCfgManager>();
        ipcSkeleton = std::make_shared<MockIPCSkeleton>();

        // 设置模拟行为
        ON_CALL(*userSessionManager, GetUserSession(_)).WillByDefault(Return(mockUserSession));
        ON_CALL(*identityChecker, IsBroker(_)).WillByDefault(Return(false));
        ON_CALL(*identityChecker, HasPermission(_, _)).WillByDefault(Return(true));
        ON_CALL(*identityChecker, IsSystemApp(_)).WillByDefault(Return(true));
        ON_CALL(*identityChecker, IsNativeSa(_)).WillByDefault(Return(false));
        ON_CALL(*identityChecker, IsBundleNameValid(_, _)).WillByDefault(Return(true));
        ON_CALL(*identityChecker, IsFocused(_, _, _)).WillByDefault(Return(true));

        ON_CALL(*imCommonEventManager, PublishPanelStatusChangeEvent(_, _, _)).WillByDefault(Return(0));

        ON_CALL(*imeInfoInquirer, GetCurrentInputMethod(_)).WillByDefault(Return(mockProperty));
        ON_CALL(*imeInfoInquirer, IsDefaultImeSet(_)).WillByDefault(Return(true));
        ON_CALL(*imeInfoInquirer, GetCurrentSubtype(_)).WillByDefault(Return(mockSubProperty));
        ON_CALL(*imeInfoInquirer, GetDefaultInputMethod(_, _, _)).WillByDefault(Return(0));
        ON_CALL(*imeInfoInquirer, GetInputMethodConfig(_, _)).WillByDefault(Return(0));
        ON_CALL(*imeInfoInquirer, ListInputMethod(_, _, _, _)).WillByDefault(Return(0));
        ON_CALL(*imeInfoInquirer, ListCurrentInputMethodSubtype(_, _)).WillByDefault(Return(0));
        ON_CALL(*imeInfoInquirer, ListInputMethodSubtype(_, _, _)).WillByDefault(Return(0));

        ON_CALL(*inputTypeManager, IsSupported(_)).WillByDefault(Return(true));
        ON_CALL(*inputTypeManager, IsStarted()).WillByDefault(Return(false));
        ON_CALL(*inputTypeManager, GetCurrentIme()).WillByDefault(Return(mockImeIdentification));
        ON_CALL(*inputTypeManager, Set(_, _)).WillByDefault(Return());

        ON_CALL(*imeCfgManager, GetCurrentImeCfg(_)).WillByDefault(Return(mockImeCfg));
        ON_CALL(*imeCfgManager, ModifyImeCfg(_)).WillByDefault(Return());

        ON_CALL(*mockUserSession, OnShowCurrentInput()).WillByDefault(Return(0));
        ON_CALL(*mockUserSession, OnPanelStatusChange(_, _)).WillByDefault(Return(0));
        ON_CALL(*mockUserSession, OnUpdateListenEventFlag(_)).WillByDefault(Return(0));
        ON_CALL(*mockUserSession, OnSetCallingWindow(_, _)).WillByDefault(Return(0));
        ON_CALL(*mockUserSession, GetInputStartInfo(_, _)).WillByDefault(Return(0));
        ON_CALL(*mockUserSession, IsCurrentImeByPid(_)).WillByDefault(Return(true));
        ON_CALL(*mockUserSession, IsPanelShown(_, _)).WillByDefault(Return(0));
        ON_CALL(*mockUserSession, OnHideCurrentInput()).WillByDefault(Return(0));
        ON_CALL(*mockUserSession, SwitchSubtype(_)).WillByDefault(Return(0));
        ON_CALL(*mockUserSession, StartIme(_)).WillByDefault(Return(true));
        ON_CALL(*mockUserSession, NotifyImeChangeToClients(_, _)).WillByDefault(Return());
        ON_CALL(*mockUserSession, GetSwitchQueue()).WillByDefault(Return(mockSwitchQueue));
        ON_CALL(*mockUserSession, CheckSecurityMode()).WillByDefault(Return(false));

        ON_CALL(*mockSwitchQueue, IsReady(_)).WillByDefault(Return(true));
        ON_CALL(*mockSwitchQueue, Wait(_)).WillByDefault(Return());
        ON_CALL(*mockSwitchQueue, Push(_)).WillByDefault(Return());
        ON_CALL(*mockSwitchQueue, Pop()).WillByDefault(Return());

        ON_CALL(*ipcSkeleton, GetCallingTokenID()).WillByDefault(Return(1));
        ON_CALL(*ipcSkeleton, GetCallingFullTokenID()).WillByDefault(Return(1));
        ON_CALL(*ipcSkeleton, GetCallingPid()).WillByDefault(Return(1));

        // 设置系统能力
        systemAbility = std::make_shared<InputMethodSystemAbility>();
        systemAbility->userSessionManager_ = userSessionManager;
        systemAbility->identityChecker_ = identityChecker;
        systemAbility->imCommonEventManager_ = imCommonEventManager;
        systemAbility->imeInfoInquirer_ = imeInfoInquirer;
        systemAbility->inputTypeManager_ = inputTypeManager;
        systemAbility->imeCfgManager_ = imeCfgManager;
        systemAbility->userId_ = 0;
    }

    void TearDown() override
    {
        // 清理资源
    }

    std::shared_ptr<InputMethodSystemAbility> systemAbility;
    std::shared_ptr<MockUserSessionManager> userSessionManager;
    std::shared_ptr<MockIdentityChecker> identityChecker;
    std::shared_ptr<MockImCommonEventManager> imCommonEventManager;
    std::shared_ptr<MockImeInfoInquirer> imeInfoInquirer;
    std::shared_ptr<MockInputTypeManager> inputTypeManager;
    std::shared_ptr<MockImeCfgManager> imeCfgManager;
    std::shared_ptr<MockIPCSkeleton> ipcSkeleton;
    std::shared_ptr<MockUserSession> mockUserSession = std::make_shared<MockUserSession>();
    std::shared_ptr<MockSwitchQueue> mockSwitchQueue = std::make_shared<MockSwitchQueue>();
    std::shared_ptr<Property> mockProperty = std::make_shared<Property>();
    std::shared_ptr<SubProperty> mockSubProperty = std::make_shared<SubProperty>();
    std::shared_ptr<ImeCfg> mockImeCfg = std::make_shared<ImeCfg>();
    std::shared_ptr<ImeIdentification> mockImeIdentification = std::make_shared<ImeIdentification>();
};

TEST_F(InputMethodSystemAbilityTest, ShowCurrentInput_SessionIsNull_ReturnsError)
{
    ON_CALL(*userSessionManager, GetUserSession(_)).WillByDefault(Return(nullptr));
    EXPECT_EQ(systemAbility->ShowCurrentInput(), ErrorCode::ERROR_NULL_POINTER);
}

TEST_F(InputMethodSystemAbilityTest, PanelStatusChange_SessionIsNull_ReturnsError)
{
    ON_CALL(*userSessionManager, GetUserSession(_)).WillByDefault(Return(nullptr));
    EXPECT_EQ(systemAbility->PanelStatusChange(InputWindowStatus(), ImeWindowInfo()), ErrorCode::ERROR_NULL_POINTER);
}

TEST_F(InputMethodSystemAbilityTest, UpdateListenEventFlag_SessionIsNull_ReturnsError)
{
    ON_CALL(*userSessionManager, GetUserSession(_)).WillByDefault(Return(nullptr));
    InputClientInfo clientInfo;
    EXPECT_EQ(systemAbility->UpdateListenEventFlag(clientInfo, 0), ErrorCode::ERROR_NULL_POINTER);
}

TEST_F(InputMethodSystemAbilityTest, SetCallingWindow_SessionIsNull_ReturnsError)
{
    ON_CALL(*userSessionManager, GetUserSession(_)).WillByDefault(Return(nullptr));
    EXPECT_EQ(systemAbility->SetCallingWindow(1, nullptr), ErrorCode::ERROR_NULL_POINTER);
}

TEST_F(InputMethodSystemAbilityTest, GetInputStartInfo_SessionIsNull_ReturnsError)
{
    ON_CALL(*userSessionManager, GetUserSession(_)).WillByDefault(Return(nullptr));
    bool isInputStart;
    uint32_t callingWndId;
    EXPECT_EQ(systemAbility->GetInputStartInfo(isInputStart, callingWndId), ErrorCode::ERROR_NULL_POINTER);
}

TEST_F(InputMethodSystemAbilityTest, IsCurrentImeByPid_SessionIsNull_ReturnsFalse)
{
    ON_CALL(*userSessionManager, GetUserSession(_)).WillByDefault(Return(nullptr));
    EXPECT_FALSE(systemAbility->IsCurrentImeByPid(1));
}

TEST_F(InputMethodSystemAbilityTest, IsPanelShown_SessionIsNull_ReturnsError)
{
    ON_CALL(*userSessionManager, GetUserSession(_)).WillByDefault(Return(nullptr));
    PanelInfo panelInfo;
    bool isShown;
    EXPECT_EQ(systemAbility->IsPanelShown(panelInfo, isShown), ErrorCode::ERROR_NULL_POINTER);
}

TEST_F(InputMethodSystemAbilityTest, SwitchInputMethod_SessionIsNull_ReturnsError)
{
    ON_CALL(*userSessionManager, GetUserSession(_)).WillByDefault(Return(nullptr));
    EXPECT_EQ(systemAbility->SwitchInputMethod("bundle", "sub", SwitchTrigger::USER), ErrorCode::ERROR_NULL_POINTER);
}

TEST_F(InputMethodSystemAbilityTest, EnableIme_PermissionDenied_ReturnsFalse)
{
    ON_CALL(*identityChecker, IsSystemApp(_)).WillByDefault(Return(false));
    EXPECT_FALSE(systemAbility->EnableIme("bundle"));
}

TEST_F(InputMethodSystemAbilityTest, OnSwitchInputMethod_SessionIsNull_ReturnsError)
{
    ON_CALL(*userSessionManager, GetUserSession(_)).WillByDefault(Return(nullptr));
    SwitchInfo switchInfo;
    EXPECT_EQ(systemAbility->OnSwitchInputMethod(0, switchInfo, SwitchTrigger::USER), ErrorCode::ERROR_NULL_POINTER);
}

TEST_F(InputMethodSystemAbilityTest, OnStartInputType_SessionIsNull_ReturnsError)
{
    ON_CALL(*userSessionManager, GetUserSession(_)).WillByDefault(Return(nullptr));
    SwitchInfo switchInfo;
    EXPECT_EQ(systemAbility->OnStartInputType(0, switchInfo, true), ErrorCode::ERROR_NULL_POINTER);
}

TEST_F(InputMethodSystemAbilityTest, Switch_SessionIsNull_ReturnsError)
{
    ON_CALL(*userSessionManager, GetUserSession(_)).WillByDefault(Return(nullptr));
    std::shared_ptr<ImeInfo> info = std::make_shared<ImeInfo>();
    EXPECT_EQ(systemAbility->Switch(0, "bundle", info), ErrorCode::ERROR_NULL_POINTER);
}

TEST_F(InputMethodSystemAbilityTest, SwitchExtension_SessionIsNull_ReturnsError)
{
    ON_CALL(*userSessionManager, GetUserSession(_)).WillByDefault(Return(nullptr));
    std::shared_ptr<ImeInfo> info = std::make_shared<ImeInfo>();
    EXPECT_EQ(systemAbility->SwitchExtension(0, info), ErrorCode::ERROR_NULL_POINTER);
}

TEST_F(InputMethodSystemAbilityTest, SwitchSubType_SessionIsNull_ReturnsError)
{
    ON_CALL(*userSessionManager, GetUserSession(_)).WillByDefault(Return(nullptr));
    std::shared_ptr<ImeInfo> info = std::make_shared<ImeInfo>();
    EXPECT_EQ(systemAbility->SwitchSubType(0, info), ErrorCode::ERROR_NULL_POINTER);
}

TEST_F(InputMethodSystemAbilityTest, SwitchInputType_SessionIsNull_ReturnsError)
{
    ON_CALL(*userSessionManager, GetUserSession(_)).WillByDefault(Return(nullptr));
    SwitchInfo switchInfo;
    EXPECT_EQ(systemAbility->SwitchInputType(0, switchInfo), ErrorCode::ERROR_NULL_POINTER);
}

TEST_F(InputMethodSystemAbilityTest, HideCurrentInputDeprecated_SessionIsNull_ReturnsError)
{
    ON_CALL(*userSessionManager, GetUserSession(_)).WillByDefault(Return(nullptr));
    EXPECT_EQ(systemAbility->HideCurrentInputDeprecated(), ErrorCode::ERROR_NULL_POINTER);
}

TEST_F(InputMethodSystemAbilityTest, ShowCurrentInputDeprecated_SessionIsNull_ReturnsError)
{
    ON_CALL(*userSessionManager, GetUserSession(_)).WillByDefault(Return(nullptr));
    EXPECT_EQ(systemAbility->ShowCurrentInputDeprecated(), ErrorCode::ERROR_NULL_POINTER);
}