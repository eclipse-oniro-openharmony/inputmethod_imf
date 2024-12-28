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
#include "input_method_ability_interface.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "input_method_ability.h"
#include "input_method_engine_listener.h"
#include "keyboard_listener.h"

using namespace OHOS::MiscServices;
using namespace testing;

class MockInputMethodAbility : public InputMethodAbility {
public:
    MOCK_METHOD0(SetCoreAndAgent, int32_t());
    MOCK_METHOD1(UnRegisteredProxyIme, int32_t(UnRegisteredType type));
    MOCK_METHOD1(InsertText, int32_t(const std::string &text));
    MOCK_METHOD1(DeleteForward, int32_t(int32_t length));
    MOCK_METHOD1(DeleteBackward, int32_t(int32_t length));
    MOCK_METHOD1(MoveCursor, int32_t(int32_t keyCode));
    MOCK_METHOD1(SetImeListener, void(std::shared_ptr<InputMethodEngineListener> imeListener));
    MOCK_METHOD1(SetKdListener, void(std::shared_ptr<KeyboardListener> kdListener));
};

class MockInputMethodEngineListener : public InputMethodEngineListener {
public:
    MOCK_METHOD0(OnStartInput, void());
    MOCK_METHOD0(OnStopInput, void());
    MOCK_METHOD0(OnUpdateCursor, void());
    MOCK_METHOD0(OnUpdateSelection, void());
    MOCK_METHOD0(OnUpdateSurroundingText, void());
    MOCK_METHOD0(OnUpdateComposition, void());
    MOCK_METHOD0(OnUpdateKeyboardLayout, void());
};

class MockKeyboardListener : public KeyboardListener {
public:
    MOCK_METHOD0(OnKey, void());
    MOCK_METHOD0(OnKeyDown, void());
    MOCK_METHOD0(OnKeyUp, void());
};

class InputMethodAbilityInterfaceTest : public Test {
protected:
    void SetUp() override
    {
        mockInputMethodAbility = std::make_shared<MockInputMethodAbility>();
        ON_CALL(InputMethodAbility, GetInstance()).WillByDefault(Return(mockInputMethodAbility));
    }

    std::shared_ptr<MockInputMethodAbility> mockInputMethodAbility;
};

HWTEST_F(InputMethodAbilityInterfaceTest, registeredProxy_001, TestSize.Level0)
{
    EXPECT_CALL(*mockInputMethodAbility, SetCoreAndAgent()).WillOnce(Return(0));
    int32_t result = InputMethodAbilityInterface::GetInstance().RegisteredProxy();
    EXPECT_EQ(result, 0);
}

HWTEST_F(InputMethodAbilityInterfaceTest, registeredProxy_002, TestSize.Level0)
{
    EXPECT_CALL(*mockInputMethodAbility, UnRegisteredProxyIme(UnRegisteredType::UNREGISTERED_TYPE_IMS))
        .WillOnce(Return(0));
    int32_t result =
        InputMethodAbilityInterface::GetInstance().UnRegisteredProxy(UnRegisteredType::UNREGISTERED_TYPE_IMS);
    EXPECT_EQ(result, 0);
}

HWTEST_F(InputMethodAbilityInterfaceTest, registeredProxy_003, TestSize.Level0)
{
    EXPECT_CALL(*mockInputMethodAbility, InsertText("test")).WillOnce(Return(0));
    int32_t result = InputMethodAbilityInterface::GetInstance().InsertText("test");
    EXPECT_EQ(result, 0);
}

HWTEST_F(InputMethodAbilityInterfaceTest, registeredProxy_004, TestSize.Level0)
{
    EXPECT_CALL(*mockInputMethodAbility, DeleteForward(1)).WillOnce(Return(0));
    int32_t result = InputMethodAbilityInterface::GetInstance().DeleteForward(1);
    EXPECT_EQ(result, 0);
}

HWTEST_F(InputMethodAbilityInterfaceTest, registeredProxy_005, TestSize.Level0)
{
    EXPECT_CALL(*mockInputMethodAbility, DeleteBackward(1)).WillOnce(Return(0));
    int32_t result = InputMethodAbilityInterface::GetInstance().DeleteBackward(1);
    EXPECT_EQ(result, 0);
}

HWTEST_F(InputMethodAbilityInterfaceTest, registeredProxy_006, TestSize.Level0)
{
    EXPECT_CALL(*mockInputMethodAbility, MoveCursor(1)).WillOnce(Return(0));
    int32_t result = InputMethodAbilityInterface::GetInstance().MoveCursor(1);
    EXPECT_EQ(result, 0);
}

HWTEST_F(InputMethodAbilityInterfaceTest, registeredProxy_007, TestSize.Level0)
{
    auto mockImeListener = std::make_shared<MockInputMethodEngineListener>();
    EXPECT_CALL(*mockInputMethodAbility, SetImeListener(mockImeListener));
    InputMethodAbilityInterface::GetInstance().SetImeListener(mockImeListener);
}

HWTEST_F(InputMethodAbilityInterfaceTest, registeredProxy_008, TestSize.Level0)
{
    auto mockKdListener = std::make_shared<MockKeyboardListener>();
    EXPECT_CALL(*mockInputMethodAbility, SetKdListener(mockKdListener));
    InputMethodAbilityInterface::GetInstance().SetKdListener(mockKdListener);
}