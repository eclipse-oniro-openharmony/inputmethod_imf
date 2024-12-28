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

#include "ime_event_listener.h"
#include "ime_event_monitor_manager.h"
#include "ime_event_monitor_manager_impl.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace OHOS::MiscServices;
using namespace testing;

class MockImeEventListener : public ImeEventListener {
public:
    MOCK_METHOD1(OnImeEvent, void(uint32_t eventFlag));
};

class MockImeEventMonitorManagerImpl : public ImeEventMonitorManagerImpl {
public:
    MOCK_METHOD2(
        RegisterImeEventListener, int32_t(uint32_t eventFlag, const std::shared_ptr<ImeEventListener> &listener));
    MOCK_METHOD2(
        UnRegisterImeEventListener, int32_t(uint32_t eventFlag, const std::shared_ptr<ImeEventListener> &listener));
};

class ImeEventMonitorManagerTest : public Test {
protected:
    void SetUp() override
    {
        // 使用模拟的实现
        auto mockImpl = std::make_shared<MockImeEventMonitorManagerImpl>();
        ON_CALL(*mockImpl, RegisterImeEventListener(_, _)).WillByDefault(Return(ErrorCode::SUCCESS));
        ON_CALL(*mockImpl, UnRegisterImeEventListener(_, _)).WillByDefault(Return(ErrorCode::SUCCESS));
        ImeEventMonitorManagerImpl::SetInstance(mockImpl);
    }

    void TearDown() override
    {
        // 重置实例
        ImeEventMonitorManagerImpl::SetInstance(nullptr);
    }
};

TEST_F(ImeEventMonitorManagerTest, RegisterImeEventListener_InvalidEventFlag_ReturnsError)
{
    auto listener = std::make_shared<MockImeEventListener>();
    int32_t result = ImeEventMonitorManager::GetInstance().RegisterImeEventListener(0, listener);
    EXPECT_EQ(result, ErrorCode::ERROR_BAD_PARAMETERS);
}

TEST_F(ImeEventMonitorManagerTest, RegisterImeEventListener_NullListener_ReturnsError)
{
    int32_t result = ImeEventMonitorManager::GetInstance().RegisterImeEventListener(1, nullptr);
    EXPECT_EQ(result, ErrorCode::ERROR_BAD_PARAMETERS);
}

TEST_F(ImeEventMonitorManagerTest, RegisterImeEventListener_ValidParameters_RegistersSuccessfully)
{
    auto listener = std::make_shared<MockImeEventListener>();
    int32_t result = ImeEventMonitorManager::GetInstance().RegisterImeEventListener(1, listener);
    EXPECT_EQ(result, ErrorCode::SUCCESS);
}

TEST_F(ImeEventMonitorManagerTest, UnRegisterImeEventListener_InvalidEventFlag_ReturnsError)
{
    auto listener = std::make_shared<MockImeEventListener>();
    int32_t result = ImeEventMonitorManager::GetInstance().UnRegisterImeEventListener(0, listener);
    EXPECT_EQ(result, ErrorCode::ERROR_BAD_PARAMETERS);
}

TEST_F(ImeEventMonitorManagerTest, UnRegisterImeEventListener_NullListener_ReturnsError)
{
    int32_t result = ImeEventMonitorManager::GetInstance().UnRegisterImeEventListener(1, nullptr);
    EXPECT_EQ(result, ErrorCode::ERROR_BAD_PARAMETERS);
}

TEST_F(ImeEventMonitorManagerTest, UnRegisterImeEventListener_ValidParameters_UnregistersSuccessfully)
{
    auto listener = std::make_shared<MockImeEventListener>();
    int32_t result = ImeEventMonitorManager::GetInstance().UnRegisterImeEventListener(1, listener);
    EXPECT_EQ(result, ErrorCode::SUCCESS);
}