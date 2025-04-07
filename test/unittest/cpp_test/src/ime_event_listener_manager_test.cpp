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
#include <gtest/gtest.h>
#include <sys/time.h>

#define PRIVATE public
#define PROTECTED public
#include "ime_event_listener_manager.h"
#include "input_client_service_impl.h"
#undef PRIVATE
#include <cstdint>
#include <functional>
#include <string>
#include <thread>
#include <vector>
#include "global.h"

namespace OHOS {
namespace MiscServices {
using namespace testing::ext;
namespace {
} // namespace

class ImeEventListenerManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void ImeEventListenerManagerTest::SetUpTestCase()
{
    IMSA_HILOGI("ImeEventListenerManagerTest::SetUpTestCase");
}

void ImeEventListenerManagerTest::TearDownTestCase()
{
    IMSA_HILOGI("ImeEventListenerManagerTest::TearDownTestCase");
}

void ImeEventListenerManagerTest::SetUp()
{
    ImeEventListenerManager::GetInstance().imeEventListeners_.clear();
    IMSA_HILOGI("ImeEventListenerManagerTest::SetUp");
}

void ImeEventListenerManagerTest::TearDown()
{
    IMSA_HILOGI("ImeEventListenerManagerTest::TearDown");
}


/**
 * @tc.name: test_UpdateListenerInfo_insert_001
 * @tc.desc: test KeyEvent Callback.
 * @tc.type: FUNC
 */
HWTEST_F(ImeEventListenerManagerTest, test_UpdateListenerInfo_insert_001, TestSize.Level0)
{
    IMSA_HILOGI("test_UpdateListenerInfo_insert_001 TEST START");
    int32_t userId = 101;
    uint32_t eventFlag = 1;
    sptr<IInputClient> client = new (std::nothrow) InputClientServiceImpl();
    ASSERT_NE(client, nullptr);
    int32_t result = ImeEventListenerManager::GetInstance().UpdateListenerInfo(userId, { eventFlag, client});
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
    EXPECT_EQ(ImeEventListenerManager::GetInstance().imeEventListeners_.size(), 1);
    auto it = ImeEventListenerManager::GetInstance().imeEventListeners_.find(userId);
    ASSERT_NE(it, ImeEventListenerManager::GetInstance().imeEventListeners_.end());
    EXPECT_EQ(ImeEventListenerManager::GetInstance().imeEventListeners_.size(), 1);
    auto iter = std::find_if(it->second.begin(), it->second.end(), [&client](const ImeEventListenerInfo &listenerInfo) {
        return (listenerInfo.client != nullptr && client != nullptr
            && listenerInfo.client->AsObject() == client->AsObject());
    });
    ASSERT_NE(iter, it->second.end());
    EXPECT_EQ(iter->eventFlag, eventFlag);
}

/**
 * @tc.name: test_UpdateListenerInfo_update_001
 * @tc.desc: test KeyEvent Callback.
 * @tc.type: FUNC
 */
HWTEST_F(ImeEventListenerManagerTest, test_UpdateListenerInfo_update_001, TestSize.Level0)
{
    IMSA_HILOGI("test_UpdateListenerInfo_update_001 TEST START");
    int32_t userId = 102;
    uint32_t eventFlag = 1;
    sptr<IInputClient> client1 = new (std::nothrow) InputClientServiceImpl();
    ASSERT_NE(client1, nullptr);
    int32_t re = ImeEventListenerManager::GetInstance().UpdateListenerInfo(userId, { eventFlag, client1});
    EXPECT_EQ(re, ErrorCode::NO_ERROR);
    sptr<IInputClient> client2 = new (std::nothrow) InputClientServiceImpl();
    ASSERT_NE(client2, nullptr);
    auto result = ImeEventListenerManager::GetInstance().UpdateListenerInfo(userId, { eventFlag, client2});
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
    EXPECT_EQ(ImeEventListenerManager::GetInstance().imeEventListeners_.size(), 2);
    auto it = ImeEventListenerManager::GetInstance().imeEventListeners_.find(userId);
    ASSERT_NE(it, ImeEventListenerManager::GetInstance().imeEventListeners_.end());
    EXPECT_EQ(ImeEventListenerManager::GetInstance().imeEventListeners_.size(), 2);
    auto iter = std::find_if(it->second.begin(), it->second.end(),
        [&client2](const ImeEventListenerInfo &listenerInfo) {
        return (listenerInfo.client != nullptr && client2 != nullptr
            && listenerInfo.client->AsObject() == client2->AsObject());
    });
    ASSERT_NE(iter, it->second.end());
    EXPECT_EQ(iter->eventFlag, eventFlag);
}

/**
 * @tc.name: test_UpdateListenerInfo_update_002
 * @tc.desc: test KeyEvent Callback.
 * @tc.type: FUNC
 */
HWTEST_F(ImeEventListenerManagerTest, test_UpdateListenerInfo_update_002, TestSize.Level0)
{
    IMSA_HILOGI("test_UpdateListenerInfo_update_002 TEST START");
    int32_t userId = 103;
    uint32_t eventFlag = 1;
    sptr<IInputClient> client = new (std::nothrow) InputClientServiceImpl();
    ASSERT_NE(client, nullptr);
    int32_t re = ImeEventListenerManager::GetInstance().UpdateListenerInfo(userId, { eventFlag, client});
    EXPECT_EQ(re, ErrorCode::NO_ERROR);
    uint32_t eventFlag1 = 0;
    auto result = ImeEventListenerManager::GetInstance().UpdateListenerInfo(userId, { eventFlag1, client});
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
    EXPECT_EQ(ImeEventListenerManager::GetInstance().imeEventListeners_.size(), 0);
    EXPECT_EQ(ImeEventListenerManager::GetInstance().imeEventListeners_.size(), 0);
}

/**
 * @tc.name: test_GetListenerInfosWithRootUser_001
 * @tc.desc: test KeyEvent Callback.
 * @tc.type: FUNC
 */
HWTEST_F(ImeEventListenerManagerTest, test_GetListenerInfosWithRootUser_001, TestSize.Level0)
{
    IMSA_HILOGI("test_GetListenerInfosWithRootUser_001 TEST START");
    int32_t userId = 101;
    size_t size = ImeEventListenerManager::GetInstance().GetListenerInfosWithRootUser(userId).size();
    EXPECT_EQ(size, ErrorCode::NO_ERROR);
}
} // namespace MiscServices
} // namespace OHOS