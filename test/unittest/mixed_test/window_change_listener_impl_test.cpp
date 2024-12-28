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
#include <gmock/gmock.h>
#include "window_change_listener_impl.h"
#include "mock_window_change_listener_impl.h"

using namespace testing;
using namespace OHOS::MiscServices;

class WindowChangeListenerImplTest : public Test {
protected:
    std::shared_ptr<MockWindowChangeListenerImpl> listener;
    std::shared_ptr<MockChangeHandler> changeHandler;

    void SetUp() override
    {
        listener = std::make_shared<MockWindowChangeListenerImpl>();
        changeHandler = std::make_shared<MockChangeHandler>();
        listener->SetChangeHandler(changeHandler);
    }
};

HWTEST_F(WindowChangeListenerImplTest, OnSizeChange_HandlerCalledWithCorrectSize) {
    Rosen::Rect rect = {0, 0, 100, 200}; // width = 100, height = 200
    Rosen::WindowSizeChangeReason reason = Rosen::WindowSizeChangeReason::UNKNOWN;

    EXPECT_CALL(*changeHandler, operator()(100, 200)).Times(1);

    listener->OnSizeChange(rect, reason, nullptr);
}