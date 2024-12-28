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

#include "ime_event_monitor_manager_impl.h"

#include <gtest/gtest.h>
#include <memory>
#include <mutex>
#include <set>

#include "event_status_manager.h"
#include "ime_event_listener.h"
#include "ime_window_info.h"
#include "input_method_controller.h"

using namespace OHOS::MiscServices;

class MockImeEventListener : public ImeEventListener {
public:
    MOCK_METHOD1(OnImeChange, void(const Property &property, const SubProperty &subProperty));
    MOCK_METHOD1(OnInputStart, void(uint32_t callingWndId));
    MOCK_METHOD0(OnInputStop, void());
    MOCK_METHOD1(OnImeShow, void(const ImeWindowInfo &info));
    MOCK_METHOD1(OnImeHide, void(const ImeWindowInfo &info));
};

class MockInputMethodController : public InputMethodController {
public:
    MOCK_METHOD3(UpdateListenEventFlag, int32_t(uint32_t finalEventFlag, uint32_t eventFlag, bool isRegister));
    MOCK_METHOD2(GetInputStartInfo, int32_t(bool &isInputStart, uint32_t &callingWindowId));
};

class MockEventStatusManager {
public:
    static bool IsInputStatusChangedOn(uint32_t eventFlag)
    {
        return eventFlag == EVENT_INPUT_STATUS_CHANGED_MASK;
    }
};

class MockImeWindowInfo {
public:
    uint32_t windowId = 0;
};

class ImeEventMonitorManagerImplTest : public testing::Test {
protected:
    void SetUp() override
    {
        imeEventMonitorManager = std::make_unique<ImeEventMonitorManagerImpl>();
        listener = std::make_shared<MockImeEventListener>();
        inputMethodController = std::make_shared<MockInputMethodController>();
        imeEventMonitorManager->SetInputMethodController(inputMethodController);
    }

    void TearDown() override
    {
        imeEventMonitorManager.reset();
        listener.reset();
        inputMethodController.reset();
    }

    std::unique_ptr<ImeEventMonitorManagerImpl> imeEventMonitorManager;
    std::shared_ptr<MockImeEventListener> listener;
    std::shared_ptr<MockInputMethodController> inputMethodController;
};

HWTEST_F(ImeEventMonitorManagerImplTest, registerImeEventListener_001, TestSize.Level0)
{
    EXPECT_CALL(*inputMethodController, UpdateListenEventFlag(testing::_, testing::_, true))
        .WillOnce(testing::Return(ErrorCode::NO_ERROR));

    int32_t result = imeEventMonitorManager->RegisterImeEventListener(EVENT_IME_CHANGE_MASK, listener);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

HWTEST_F(ImeEventMonitorManagerImplTest, registerImeEventListener_002, TestSize.Level0)
{
    EXPECT_CALL(*inputMethodController, UpdateListenEventFlag(testing::_, testing::_, true))
        .WillOnce(testing::Return(ErrorCode::ERROR_BAD_PARAMETERS));

    int32_t result = imeEventMonitorManager->RegisterImeEventListener(EVENT_IME_CHANGE_MASK, listener);
    EXPECT_EQ(result, ErrorCode::ERROR_BAD_PARAMETERS);
}

HWTEST_F(ImeEventMonitorManagerImplTest, registerImeEventListener_003, TestSize.Level0)
{
    EXPECT_CALL(*inputMethodController, UpdateListenEventFlag(testing::_, testing::_, false))
        .WillOnce(testing::Return(ErrorCode::NO_ERROR));

    int32_t result = imeEventMonitorManager->UnRegisterImeEventListener(EVENT_IME_CHANGE_MASK, listener);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

HWTEST_F(ImeEventMonitorManagerImplTest, registerImeEventListener_004, TestSize.Level0)
{
    EXPECT_CALL(*inputMethodController, UpdateListenEventFlag(testing::_, testing::_, false))
        .WillOnce(testing::Return(ErrorCode::ERROR_BAD_PARAMETERS));

    int32_t result = imeEventMonitorManager->UnRegisterImeEventListener(EVENT_IME_CHANGE_MASK, listener);
    EXPECT_EQ(result, ErrorCode::ERROR_BAD_PARAMETERS);
}

HWTEST_F(ImeEventMonitorManagerImplTest, registerImeEventListener_005, TestSize.Level0)
{
    EXPECT_CALL(*listener, OnImeChange(testing::_, testing::_)).Times(1);

    imeEventMonitorManager->RegisterImeEventListener(EVENT_IME_CHANGE_MASK, listener);
    imeEventMonitorManager->OnImeChange(Property(), SubProperty());
}

HWTEST_F(ImeEventMonitorManagerImplTest, registerImeEventListener_006, TestSize.Level0)
{
    MockImeWindowInfo info;
    info.windowId = 1;

    EXPECT_CALL(*listener, OnImeShow(testing::_)).Times(1);
    EXPECT_CALL(*listener, OnImeHide(testing::_)).Times(1);

    imeEventMonitorManager->RegisterImeEventListener(EVENT_IME_SHOW_MASK, listener);
    imeEventMonitorManager->RegisterImeEventListener(EVENT_IME_HIDE_MASK, listener);

    imeEventMonitorManager->OnPanelStatusChange(InputWindowStatus::SHOW, info);
    imeEventMonitorManager->OnPanelStatusChange(InputWindowStatus::HIDE, info);
}

HWTEST_F(ImeEventMonitorManagerImplTest, registerImeEventListener_007, TestSize.Level0)
{
    EXPECT_CALL(*listener, OnInputStart(testing::_)).Times(1);

    imeEventMonitorManager->RegisterImeEventListener(EVENT_INPUT_STATUS_CHANGED_MASK, listener);
    imeEventMonitorManager->OnInputStart(1);
}

HWTEST_F(ImeEventMonitorManagerImplTest, registerImeEventListener_008, TestSize.Level0)
{
    EXPECT_CALL(*listener, OnInputStop()).Times(1);

    imeEventMonitorManager->RegisterImeEventListener(EVENT_INPUT_STATUS_CHANGED_MASK, listener);
    imeEventMonitorManager->OnInputStop();
}