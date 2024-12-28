/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "system_language_observer.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "message.h"
#include "message_handler.h"
#include "parameter.h"

using namespace testing;
using namespace OHOS::MiscServices;

class MockMessageHandler : public MessageHandler {
public:
    MOCK_METHOD1(SendMessage, void(Message *msg));
};

class MockParameter : public Parameter {
public:
    MOCK_METHOD3(WatchParameter, int(const char *key, ParameterChangeCallback callback, void *context));
};

class SystemLanguageObserverTest : public Test {
protected:
    void SetUp() override
    {
        messageHandler = new MockMessageHandler();
        parameter = new MockParameter();
        MessageHandler::SetInstance(messageHandler);
        Parameter::SetInstance(parameter);
    }

    void TearDown() override
    {
        MessageHandler::SetInstance(nullptr);
        Parameter::SetInstance(nullptr);
        delete messageHandler;
        delete parameter;
    }

    MockMessageHandler *messageHandler;
    MockParameter *parameter;
};

/**
 * @tc.name: Watch_ShouldCallWatchParameter
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SystemLanguageObserverTest, Watch_ShouldCallWatchParameter, TestSize.Level0)
{
    EXPECT_CALL(*parameter, WatchParameter(SYSTEM_LANGUAGE_KEY, _, _)).Times(1);
    SystemLanguageObserver::GetInstance().Watch();
}

/**
 * @tc.name: OnChange_KeyMatches_ShouldSendMessage
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SystemLanguageObserverTest, OnChange_KeyMatches_ShouldSendMessage, TestSize.Level0)
{
    Message *mockMessage = new Message(MessageID::MSG_ID_SYS_LANGUAGE_CHANGED, nullptr);
    EXPECT_CALL(*messageHandler, SendMessage(mockMessage)).Times(1);

    SystemLanguageObserver::GetInstance().OnChange(SYSTEM_LANGUAGE_KEY, "newLanguage", nullptr);

    delete mockMessage;
}

/**
 * @tc.name: OnChange_KeyDoesNotMatch_ShouldNotSendMessage
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SystemLanguageObserverTest, OnChange_KeyDoesNotMatch_ShouldNotSendMessage, TestSize.Level0)
{
    EXPECT_CALL(*messageHandler, SendMessage(_)).Times(0);

    SystemLanguageObserver::GetInstance().OnChange("wrongKey", "newLanguage", nullptr);
}

/**
 * @tc.name: OnChange_MessageCreationFails_ShouldNotSendMessage
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SystemLanguageObserverTest, OnChange_MessageCreationFails_ShouldNotSendMessage, TestSize.Level0)
{
    EXPECT_CALL(*messageHandler, SendMessage(_)).Times(0);

    // 模拟内存分配失败
    ON_CALL(*parameter, WatchParameter(SYSTEM_LANGUAGE_KEY, _, _)).WillByDefault(Return(0));
    SystemLanguageObserver::GetInstance().Watch();

    // 模拟消息创建失败
    ON_CALL(*parameter, WatchParameter(SYSTEM_LANGUAGE_KEY, _, _)).WillByDefault(Return(-1));
    SystemLanguageObserver::GetInstance().OnChange(SYSTEM_LANGUAGE_KEY, "newLanguage", nullptr);
}