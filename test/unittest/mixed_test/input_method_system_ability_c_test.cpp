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

#include "full_ime_info_manager.h"
#include "identity_checker.h"
#include "ime_cfg_manager.h"
#include "ime_info_inquirer.h"
#include "input_method_system_ability.h"
#include "inputmethod_message_handler.h"
#include "message_parcel.h"
#include "mock_full_ime_info_manager.h"
#include "mock_identity_checker.h"
#include "mock_ime_cfg_manager.h"
#include "mock_ime_info_inquirer.h"
#include "mock_message.h"
#include "mock_inputmethod_message_handler.h"
#include "mock_message_parcel.h"
#include "mock_user_session.h"
#include "mock_user_session_manager.h"
#include "user_session.h"
#include "user_session_manager.h"


using namespace testing;
using namespace OHOS::MMI;

class InputMethodSystemAbilityTest : public Test {
public:
    static void SetUpTestCase()
    {
        // 设置测试案例
    }

    static void TearDownTestCase()
    {
        // 清理测试案例
    }

    void SetUp() override
    {
        // 设置测试
        messageHandler_ = std::make_shared<MockMessageHandler>();
        fullImeInfoManager_ = std::make_shared<MockFullImeInfoManager>();
        userSessionManager_ = std::make_shared<MockUserSessionManager>();
        imeCfgManager_ = std::make_shared<MockImeCfgManager>();
        identityChecker_ = std::make_shared<MockIdentityChecker>();
        message_ = std::make_shared<MockMessage>();
        messageParcel_ = std::make_shared<MockMessageParcel>();
        userSession_ = std::make_shared<MockUserSession>();
        imeInfoInquirer_ = std::make_shared<MockImeInfoInquirer>();

        inputMethodSystemAbility_ = std::make_shared<InputMethodSystemAbility>();
        inputMethodSystemAbility_->messageHandler_ = messageHandler_;
        inputMethodSystemAbility_->fullImeInfoManager_ = fullImeInfoManager_;
        inputMethodSystemAbility_->userSessionManager_ = userSessionManager_;
        inputMethodSystemAbility_->imeCfgManager_ = imeCfgManager_;
        inputMethodSystemAbility_->identityChecker_ = identityChecker_;
        inputMethodSystemAbility_->message_ = message_;
        inputMethodSystemAbility_->messageParcel_ = messageParcel_;
        inputMethodSystemAbility_->userSession_ = userSession_;
        inputMethodSystemAbility_->imeInfoInquirer_ = imeInfoInquirer_;
    }

    void TearDown() override
    {
        // 清理测试
    }

    std::shared_ptr<InputMethodSystemAbility> inputMethodSystemAbility_;
    std::shared_ptr<MockMessageHandler> messageHandler_;
    std::shared_ptr<MockFullImeInfoManager> fullImeInfoManager_;
    std::shared_ptr<MockUserSessionManager> userSessionManager_;
    std::shared_ptr<MockImeCfgManager> imeCfgManager_;
    std::shared_ptr<MockIdentityChecker> identityChecker_;
    std::shared_ptr<MockMessage> message_;
    std::shared_ptr<MockMessageParcel> messageParcel_;
    std::shared_ptr<MockUserSession> userSession_;
    std::shared_ptr<MockImeInfoInquirer> imeInfoInquirer_;
};

TEST_F(InputMethodSystemAbilityTest, WorkThread_MessageHandling)
{
    // 模拟消息处理
    EXPECT_CALL(*messageHandler_, GetMessage()).WillOnce(Return(message_)).WillOnce(Return(nullptr));

    // 模拟消息内容
    EXPECT_CALL(*message_, msgId_).WillOnce(Return(MSG_ID_USER_START)).WillOnce(Return(MSG_ID_USER_REMOVED));

    // 模拟消息内容
    EXPECT_CALL(*message_, msgContent_).WillOnce(Return(messageParcel_.get())).WillOnce(Return(nullptr));

    // 模拟消息内容读取
    EXPECT_CALL(*messageParcel_, ReadInt32()).WillOnce(Return(1));

    // 模拟用户会话管理器
    EXPECT_CALL(*userSessionManager_, GetUserSession(1)).WillOnce(Return(userSession_));

    // 模拟用户会话
    EXPECT_CALL(*userSession_, StopCurrentIme());

    // 模拟输入法配置管理器
    EXPECT_CALL(*imeCfgManager_, DeleteImeCfg(1));

    // 模拟完整输入法信息管理器
    EXPECT_CALL(*fullImeInfoManager_, Delete(1));

    // 运行工作线程
    inputMethodSystemAbility_->WorkThread();
}