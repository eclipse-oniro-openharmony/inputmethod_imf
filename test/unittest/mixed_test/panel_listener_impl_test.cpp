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

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <map>
#include <mutex>
#include "panel_listener_impl.h"
#include "mock_js_callback_object.h"
#include "mock_event_handler.h"
#include "mock_js_callback_handler.h"

using namespace OHOS::MiscServices;
using namespace testing;

class PanelListenerImplTest : public Test {
protected:
    void SetUp() override
    {
        panelListener = PanelListenerImpl::GetInstance();
        mockEventHandler = std::make_shared<MockEventHandler>();
        panelListener->SetEventHandler(mockEventHandler);
    }

    void TearDown() override
    {
        panelListener->RemoveAllCallbacks();
    }

    std::shared_ptr<PanelListenerImpl> panelListener;
    std::shared_ptr<MockEventHandler> mockEventHandler;
};

HWTEST_F(PanelListenerImplTest, GetInstance_SingletonPattern_ReturnsSameInstance) {
    auto instance1 = PanelListenerImpl::GetInstance();
    auto instance2 = PanelListenerImpl::GetInstance();
    EXPECT_EQ(instance1, instance2);
}

HWTEST_F(PanelListenerImplTest, Subscribe_ValidCallback_CallbackStored) {
    uint32_t windowId = 1;
    std::string type = "show";
    auto mockCallback = std::make_shared<MockJSCallbackObject>();
    panelListener->Subscribe(windowId, type, mockCallback);

    auto callback = panelListener->GetCallback(windowId, type);
    EXPECT_EQ(callback, mockCallback);
}

HWTEST_F(PanelListenerImplTest, RemoveInfo_ValidCallback_CallbackRemoved) {
    uint32_t windowId = 1;
    std::string type = "show";
    auto mockCallback = std::make_shared<MockJSCallbackObject>();
    panelListener->Subscribe(windowId, type, mockCallback);
    panelListener->RemoveInfo(type, windowId);

    auto callback = panelListener->GetCallback(windowId, type);
    EXPECT_EQ(callback, nullptr);
}

HWTEST_F(PanelListenerImplTest, OnPanelStatus_ValidCallback_CallbackInvoked) {
    uint32_t windowId = 1;
    std::string type = "show";
    auto mockCallback = std::make_shared<MockJSCallbackObject>();
    panelListener->Subscribe(windowId, type, mockCallback);

    EXPECT_CALL(*mockCallback, Call()).Times(1);
    panelListener->OnPanelStatus(windowId, true);
}

HWTEST_F(PanelListenerImplTest, OnSizeChange_ValidCallback_CallbackInvoked) {
    uint32_t windowId = 1;
    std::string type = "sizeChange";
    auto mockCallback = std::make_shared<MockJSCallbackObject>();
    panelListener->Subscribe(windowId, type, mockCallback);

    WindowSize size = {100, 200};
    EXPECT_CALL(*mockCallback, Call()).Times(1);
    panelListener->OnSizeChange(windowId, size);
}

HWTEST_F(PanelListenerImplTest, SetEventHandler_ValidEventHandler_HandlerSet) {
    auto newEventHandler = std::make_shared<MockEventHandler>();
    panelListener->SetEventHandler(newEventHandler);

    auto handler = panelListener->GetEventHandler();
    EXPECT_EQ(handler, newEventHandler);
}

HWTEST_F(PanelListenerImplTest, GetCallback_ValidWindowIdAndType_ReturnsCallback) {
    uint32_t windowId = 1;
    std::string type = "show";
    auto mockCallback = std::make_shared<MockJSCallbackObject>();
    panelListener->Subscribe(windowId, type, mockCallback);

    auto callback = panelListener->GetCallback(windowId, type);
    EXPECT_EQ(callback, mockCallback);
}

HWTEST_F(PanelListenerImplTest, GetCallback_InvalidWindowIdOrType_ReturnsNull) {
    uint32_t windowId = 1;
    std::string type = "invalidType";
    auto callback = panelListener->GetCallback(windowId, type);
    EXPECT_EQ(callback, nullptr);
}