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

#include "wms_connection_observer.h"

#include <gtest/gtest.h>

#include <functional>

#include "global.h"

namespace OHOS {
namespace MiscServices {
class WmsConnectionObserverTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
    }
    static void TearDownTestCase()
    {
    }
    void SetUp() override
    {
        observer_ = std::make_unique<WmsConnectionObserver>();
        observer_->SetChangeHandler(changeHandler_);
    }
    void TearDown() override
    {
        observer_.reset();
    }

    void OnChange(bool isConnected, int32_t userId, int32_t screenId)
    {
        changeHandlerCalled_ = true;
        lastIsConnected_ = isConnected;
        lastUserId_ = userId;
        lastScreenId_ = screenId;
    }

    std::unique_ptr<WmsConnectionObserver> observer_;
    std::function<void(bool, int32_t, int32_t)> changeHandler_ =
        [this](bool isConnected, int32_t userId, int32_t screenId) { OnChange(isConnected, userId, screenId); };
    bool changeHandlerCalled_ = false;
    bool lastIsConnected_ = false;
    int32_t lastUserId_ = -1;
    int32_t lastScreenId_ = -1;
};

/**
 * @tc.name: OnConnected_UserAddedAndHandlerCalled
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WmsConnectionObserverTest, OnConnected_UserAddedAndHandlerCalled, TestSize.Level0)
{
    observer_->OnConnected(100, 0);
    EXPECT_TRUE(observer_->IsWmsConnected(100));
    EXPECT_TRUE(changeHandlerCalled_);
    EXPECT_TRUE(lastIsConnected_);
    EXPECT_EQ(lastUserId_, 100);
    EXPECT_EQ(lastScreenId_, 0);
}

/**
 * @tc.name: OnDisconnected_UserRemovedAndHandlerCalled
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WmsConnectionObserverTest, OnDisconnected_UserRemovedAndHandlerCalled, TestSize.Level0)
{
    observer_->OnConnected(100, 0);
    observer_->OnDisconnected(100, 0);
    EXPECT_FALSE(observer_->IsWmsConnected(100));
    EXPECT_TRUE(changeHandlerCalled_);
    EXPECT_FALSE(lastIsConnected_);
    EXPECT_EQ(lastUserId_, 100);
    EXPECT_EQ(lastScreenId_, 0);
}

/**
 * @tc.name: Add_UserAdded
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WmsConnectionObserverTest, Add_UserAdded, TestSize.Level0)
{
    observer_->Add(100);
    EXPECT_TRUE(observer_->IsWmsConnected(100));
}

/**
 * @tc.name: Remove_UserRemoved
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WmsConnectionObserverTest, Remove_UserRemoved, TestSize.Level0)
{
    observer_->Add(100);
    observer_->Remove(100);
    EXPECT_FALSE(observer_->IsWmsConnected(100));
}

/**
 * @tc.name: IsWmsConnected_UserConnected_ReturnsTrue
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WmsConnectionObserverTest, IsWmsConnected_UserConnected_ReturnsTrue, TestSize.Level0)
{
    observer_->Add(100);
    EXPECT_TRUE(observer_->IsWmsConnected(100));
}

/**
 * @tc.name: IsWmsConnected_UserNotConnected_ReturnsFalse
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WmsConnectionObserverTest, IsWmsConnected_UserNotConnected_ReturnsFalse, TestSize.Level0)
{
    EXPECT_FALSE(observer_->IsWmsConnected(100));
}
} // namespace MiscServices
} // namespace OHOS