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
#define private public
#define protected public
#include "ime_event_monitor_manager.h"

#include "ime_event_monitor_manager_impl.h"
#include "input_method_controller.h"
#undef private
#include <gtest/gtest.h>
#include <sys/time.h>
#include <unistd.h>

#include "ime_setting_listener_test_impl.h"
#include "tdd_util.h"

using namespace testing::ext;
namespace OHOS {
namespace MiscServices {
class ImeEventMonitorManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void ImeEventMonitorManagerTest::SetUpTestCase()
{
    TddUtil::StorageSelfTokenID();
    IMSA_HILOGI("ImeEventMonitorManagerTest::SetUpTestCase");
}

void ImeEventMonitorManagerTest::TearDownTestCase()
{
    IMSA_HILOGI("ImeEventMonitorManagerTest::TearDownTestCase");
}

void ImeEventMonitorManagerTest::SetUp()
{
    // native sa
    TddUtil::GrantNativePermission();
    IMSA_HILOGI("ImeEventMonitorManagerTest::SetUp");
}

void ImeEventMonitorManagerTest::TearDown()
{
    TddUtil::RestoreSelfTokenID();
    ImeEventMonitorManagerImpl::GetInstance().listeners_.clear();
    InputMethodController::GetInstance()->clientInfo_.eventFlag = 0;
    IMSA_HILOGI("ImeEventMonitorManagerTest::TearDown");
}

/**
* @tc.name: testRegisterImeEventListener_001
* @tc.desc: type size over max
* @tc.type: FUNC
*/
HWTEST_F(ImeEventMonitorManagerTest, testRegisterImeEventListener_001, TestSize.Level0)
{
    IMSA_HILOGI("testRegisterImeEventListener_001 start.");
    auto listener = std::make_shared<ImeSettingListenerTestImpl>();
    auto ret = ImeEventMonitorManager::GetInstance().RegisterImeEventListener(
        { EventType::IME_HIDE, EventType::IME_SHOW, EventType::IME_CHANGE, static_cast<EventType>(10) }, listener);
    EXPECT_EQ(ret, ErrorCode::ERROR_BAD_PARAMETERS);
    EXPECT_TRUE(ImeEventMonitorManagerImpl::GetInstance().listeners_.empty());
}

/**
* @tc.name: testUnRegisterImeEventListener_002
* @tc.desc: type size over max
* @tc.type: FUNC
*/
HWTEST_F(ImeEventMonitorManagerTest, testUnRegisterImeEventListener_002, TestSize.Level0)
{
    IMSA_HILOGI("testUnRegisterImeEventListener_002 start.");
    auto listener = std::make_shared<ImeSettingListenerTestImpl>();
    auto ret = ImeEventMonitorManager::GetInstance().UnRegisterImeEventListener(
        { EventType::IME_HIDE, EventType::IME_SHOW, EventType::IME_CHANGE, static_cast<EventType>(10) }, listener);
    EXPECT_EQ(ret, ErrorCode::ERROR_BAD_PARAMETERS);
    EXPECT_TRUE(ImeEventMonitorManagerImpl::GetInstance().listeners_.empty());
}

/**
* @tc.name: testRegisterImeEventListener_003
* @tc.desc: type is error
* @tc.type: FUNC
*/
HWTEST_F(ImeEventMonitorManagerTest, testRegisterImeEventListener_003, TestSize.Level0)
{
    IMSA_HILOGI("testRegisterImeEventListener_003 start.");
    auto listener = std::make_shared<ImeSettingListenerTestImpl>();
    auto ret = ImeEventMonitorManager::GetInstance().RegisterImeEventListener({ EventType::IME_CHANGE }, listener);
    EXPECT_EQ(ret, ErrorCode::ERROR_BAD_PARAMETERS);
    EXPECT_TRUE(ImeEventMonitorManagerImpl::GetInstance().listeners_.empty());
}

/**
* @tc.name: testUnRegisterImeEventListener_004
* @tc.desc: type is error
* @tc.type: FUNC
*/
HWTEST_F(ImeEventMonitorManagerTest, testUnRegisterImeEventListener_004, TestSize.Level0)
{
    IMSA_HILOGI("testUnRegisterImeEventListener_004 start.");
    auto listener = std::make_shared<ImeSettingListenerTestImpl>();
    auto ret = ImeEventMonitorManager::GetInstance().UnRegisterImeEventListener(
        { EventType::IME_HIDE, EventType::IME_CHANGE }, listener);
    EXPECT_EQ(ret, ErrorCode::ERROR_BAD_PARAMETERS);
    EXPECT_TRUE(ImeEventMonitorManagerImpl::GetInstance().listeners_.empty());
}

/**
* @tc.name: testRegisterImeEventListener_005
* @tc.desc: listener is nullptr
* @tc.type: FUNC
*/
HWTEST_F(ImeEventMonitorManagerTest, testRegisterImeEventListener_005, TestSize.Level0)
{
    IMSA_HILOGI("testRegisterImeEventListener_005 start.");
    auto ret = ImeEventMonitorManager::GetInstance().RegisterImeEventListener({ EventType::IME_HIDE }, nullptr);
    EXPECT_EQ(ret, ErrorCode::ERROR_BAD_PARAMETERS);
    EXPECT_TRUE(ImeEventMonitorManagerImpl::GetInstance().listeners_.empty());
}

/**
* @tc.name: testUnRegisterImeEventListener_006
* @tc.desc: listener is nullptr
* @tc.type: FUNC
*/
HWTEST_F(ImeEventMonitorManagerTest, testUnRegisterImeEventListener_006, TestSize.Level0)
{
    IMSA_HILOGI("testUnRegisterImeEventListener_006 start.");
    auto ret = ImeEventMonitorManager::GetInstance().UnRegisterImeEventListener({ EventType::IME_HIDE }, nullptr);
    EXPECT_EQ(ret, ErrorCode::ERROR_BAD_PARAMETERS);
    EXPECT_TRUE(ImeEventMonitorManagerImpl::GetInstance().listeners_.empty());
}

/**
* @tc.name: testRegisterImeEventListener_007
* @tc.desc: UpdateListenEventFlag filed
* @tc.type: FUNC
*/
HWTEST_F(ImeEventMonitorManagerTest, testRegisterImeEventListener_007, TestSize.Level0)
{
    IMSA_HILOGI("testRegisterImeEventListener_007 start.");
    TddUtil::RestoreSelfTokenID();
    auto listener = std::make_shared<ImeSettingListenerTestImpl>();
    auto ret = ImeEventMonitorManager::GetInstance().RegisterImeEventListener({ EventType::IME_HIDE }, listener);
    EXPECT_EQ(ret, ErrorCode::ERROR_STATUS_SYSTEM_PERMISSION);
    EXPECT_TRUE(ImeEventMonitorManagerImpl::GetInstance().listeners_.empty());
}

/**
* @tc.name: testUnRegisterImeEventListener_008
* @tc.desc: one listener register one event, unregister with UpdateListenEventFlag filed
* @tc.type: FUNC
*/
HWTEST_F(ImeEventMonitorManagerTest, testUnRegisterImeEventListener_008, TestSize.Level0)
{
    IMSA_HILOGI("testUnRegisterImeEventListener_008 start.");
    auto listener = std::make_shared<ImeSettingListenerTestImpl>();
    auto ret = ImeEventMonitorManager::GetInstance().RegisterImeEventListener({ EventType::IME_HIDE }, listener);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(ImeEventMonitorManagerImpl::GetInstance().listeners_.size(), 1);
    EXPECT_EQ(InputMethodController::GetInstance()->clientInfo_.eventFlag, 4);
    TddUtil::RestoreSelfTokenID();
    ret = ImeEventMonitorManager::GetInstance().UnRegisterImeEventListener({ EventType::IME_HIDE }, listener);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(ImeEventMonitorManagerImpl::GetInstance().listeners_.empty());
    EXPECT_EQ(InputMethodController::GetInstance()->clientInfo_.eventFlag, 0);
}

/**
* @tc.name: testRegisterImeEventListener_009
* @tc.desc: one listener register one event
* @tc.type: FUNC
*/
HWTEST_F(ImeEventMonitorManagerTest, testRegisterImeEventListener_009, TestSize.Level0)
{
    IMSA_HILOGI("testRegisterImeEventListener_009 start.");
    auto listener = std::make_shared<ImeSettingListenerTestImpl>();
    auto ret = ImeEventMonitorManager::GetInstance().RegisterImeEventListener({ EventType::IME_HIDE }, listener);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(ImeEventMonitorManagerImpl::GetInstance().listeners_.size(), 1);
    EXPECT_EQ(InputMethodController::GetInstance()->clientInfo_.eventFlag, 4);
    auto it = ImeEventMonitorManagerImpl::GetInstance().listeners_.find(EventType::IME_HIDE);
    ASSERT_NE(it, ImeEventMonitorManagerImpl::GetInstance().listeners_.end());
    ASSERT_EQ(it->second.size(), 1);
    auto iter = it->second.find(listener);
    EXPECT_NE(iter, it->second.end());
}

/**
* @tc.name: testRegisterImeEventListener_010
* @tc.desc: one listener register all event
* @tc.type: FUNC
*/
HWTEST_F(ImeEventMonitorManagerTest, testRegisterImeEventListener_010, TestSize.Level0)
{
    IMSA_HILOGI("testRegisterImeEventListener_010 start.");
    auto listener = std::make_shared<ImeSettingListenerTestImpl>();
    auto ret = ImeEventMonitorManager::GetInstance().RegisterImeEventListener(
        { EventType::IME_HIDE, EventType::IME_SHOW }, listener);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(InputMethodController::GetInstance()->clientInfo_.eventFlag, 6);
    EXPECT_EQ(ImeEventMonitorManagerImpl::GetInstance().listeners_.size(), 2);
    auto it = ImeEventMonitorManagerImpl::GetInstance().listeners_.find(EventType::IME_HIDE);
    ASSERT_NE(it, ImeEventMonitorManagerImpl::GetInstance().listeners_.end());
    ASSERT_EQ(it->second.size(), 1);
    auto iter = it->second.find(listener);
    EXPECT_NE(iter, it->second.end());

    it = ImeEventMonitorManagerImpl::GetInstance().listeners_.find(EventType::IME_SHOW);
    ASSERT_NE(it, ImeEventMonitorManagerImpl::GetInstance().listeners_.end());
    ASSERT_EQ(it->second.size(), 1);
    iter = it->second.find(listener);
    EXPECT_NE(iter, it->second.end());
}

/**
* @tc.name: testRegisterImeEventListener_011
* @tc.desc: one listener register two same event
* @tc.type: FUNC
*/
HWTEST_F(ImeEventMonitorManagerTest, testRegisterImeEventListener_011, TestSize.Level0)
{
    IMSA_HILOGI("testRegisterImeEventListener_011 start.");
    auto listener = std::make_shared<ImeSettingListenerTestImpl>();
    auto ret = ImeEventMonitorManager::GetInstance().RegisterImeEventListener(
        { EventType::IME_SHOW, EventType::IME_SHOW }, listener);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(InputMethodController::GetInstance()->clientInfo_.eventFlag, 2);
    EXPECT_EQ(ImeEventMonitorManagerImpl::GetInstance().listeners_.size(), 1);
    auto it = ImeEventMonitorManagerImpl::GetInstance().listeners_.find(EventType::IME_SHOW);
    ASSERT_NE(it, ImeEventMonitorManagerImpl::GetInstance().listeners_.end());
    ASSERT_EQ(it->second.size(), 1);
    auto iter = it->second.find(listener);
    EXPECT_NE(iter, it->second.end());
}

/**
* @tc.name: testRegisterImeEventListener_012
* @tc.desc: two listener register same event
* @tc.type: FUNC
*/
HWTEST_F(ImeEventMonitorManagerTest, testRegisterImeEventListener_012, TestSize.Level0)
{
    IMSA_HILOGI("testRegisterImeEventListener_012 start.");
    auto listener = std::make_shared<ImeSettingListenerTestImpl>();
    auto listener1 = std::make_shared<ImeSettingListenerTestImpl>();
    auto ret = ImeEventMonitorManager::GetInstance().RegisterImeEventListener(
        { EventType::IME_SHOW, EventType::IME_HIDE }, listener);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = ImeEventMonitorManager::GetInstance().RegisterImeEventListener(
        { EventType::IME_SHOW, EventType::IME_HIDE }, listener1);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(InputMethodController::GetInstance()->clientInfo_.eventFlag, 6);
    EXPECT_EQ(ImeEventMonitorManagerImpl::GetInstance().listeners_.size(), 2);
    auto it = ImeEventMonitorManagerImpl::GetInstance().listeners_.find(EventType::IME_HIDE);
    ASSERT_NE(it, ImeEventMonitorManagerImpl::GetInstance().listeners_.end());
    ASSERT_EQ(it->second.size(), 2);
    auto iter = it->second.find(listener);
    EXPECT_NE(iter, it->second.end());
    iter = it->second.find(listener1);
    EXPECT_NE(iter, it->second.end());
    it = ImeEventMonitorManagerImpl::GetInstance().listeners_.find(EventType::IME_SHOW);
    ASSERT_NE(it, ImeEventMonitorManagerImpl::GetInstance().listeners_.end());
    ASSERT_EQ(it->second.size(), 2);
    iter = it->second.find(listener);
    EXPECT_NE(iter, it->second.end());
    iter = it->second.find(listener1);
    EXPECT_NE(iter, it->second.end());
}

/**
* @tc.name: testRegisterImeEventListener_013
* @tc.desc: two listener register not same event
* @tc.type: FUNC
*/
HWTEST_F(ImeEventMonitorManagerTest, testRegisterImeEventListener_013, TestSize.Level0)
{
    IMSA_HILOGI("testRegisterImeEventListener_013 start.");
    auto listener = std::make_shared<ImeSettingListenerTestImpl>();
    auto listener1 = std::make_shared<ImeSettingListenerTestImpl>();
    auto ret = ImeEventMonitorManager::GetInstance().RegisterImeEventListener(
        { EventType::IME_SHOW, EventType::IME_SHOW }, listener);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = ImeEventMonitorManager::GetInstance().RegisterImeEventListener(
        { EventType::IME_SHOW, EventType::IME_HIDE }, listener1);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(InputMethodController::GetInstance()->clientInfo_.eventFlag, 6);

    EXPECT_EQ(ImeEventMonitorManagerImpl::GetInstance().listeners_.size(), 2);
    auto it = ImeEventMonitorManagerImpl::GetInstance().listeners_.find(EventType::IME_SHOW);
    ASSERT_NE(it, ImeEventMonitorManagerImpl::GetInstance().listeners_.end());
    ASSERT_EQ(it->second.size(), 2);
    auto iter = it->second.find(listener);
    EXPECT_NE(iter, it->second.end());
    iter = it->second.find(listener1);
    EXPECT_NE(iter, it->second.end());
    it = ImeEventMonitorManagerImpl::GetInstance().listeners_.find(EventType::IME_HIDE);
    ASSERT_NE(it, ImeEventMonitorManagerImpl::GetInstance().listeners_.end());
    ASSERT_EQ(it->second.size(), 1);
    iter = it->second.find(listener1);
    EXPECT_NE(iter, it->second.end());
}

/**
* @tc.name: testUnRegisterImeEventListener_014
* @tc.desc: one listener register one event, unregister
* @tc.type: FUNC
*/
HWTEST_F(ImeEventMonitorManagerTest, testUnRegisterImeEventListener_014, TestSize.Level0)
{
    IMSA_HILOGI("testUnRegisterImeEventListener_014 start.");
    auto listener = std::make_shared<ImeSettingListenerTestImpl>();
    auto ret = ImeEventMonitorManager::GetInstance().RegisterImeEventListener({ EventType::IME_SHOW }, listener);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(InputMethodController::GetInstance()->clientInfo_.eventFlag, 2);
    EXPECT_EQ(ImeEventMonitorManagerImpl::GetInstance().listeners_.size(), 1);
    ret = ImeEventMonitorManager::GetInstance().UnRegisterImeEventListener({ EventType::IME_SHOW }, listener);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(ImeEventMonitorManagerImpl::GetInstance().listeners_.empty());
    EXPECT_EQ(InputMethodController::GetInstance()->clientInfo_.eventFlag, 0);
}

/**
* @tc.name: testUnRegisterImeEventListener_015
* @tc.desc: one listener register all events, unregister one events
* @tc.type: FUNC
*/
HWTEST_F(ImeEventMonitorManagerTest, testUnRegisterImeEventListener_015, TestSize.Level0)
{
    IMSA_HILOGI("testUnRegisterImeEventListener_015 start.");
    auto listener = std::make_shared<ImeSettingListenerTestImpl>();
    auto ret = ImeEventMonitorManager::GetInstance().RegisterImeEventListener(
        { EventType::IME_SHOW, EventType::IME_HIDE }, listener);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(InputMethodController::GetInstance()->clientInfo_.eventFlag, 6);
    EXPECT_EQ(ImeEventMonitorManagerImpl::GetInstance().listeners_.size(), 2);
    ret = ImeEventMonitorManager::GetInstance().UnRegisterImeEventListener({ EventType::IME_HIDE }, listener);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(InputMethodController::GetInstance()->clientInfo_.eventFlag, 2);
    EXPECT_EQ(ImeEventMonitorManagerImpl::GetInstance().listeners_.size(), 1);
}

/**
* @tc.name: testUnRegisterImeEventListener_016
* @tc.desc: one listener register all events, unregister all events
* @tc.type: FUNC
*/
HWTEST_F(ImeEventMonitorManagerTest, testUnRegisterImeEventListener_016, TestSize.Level0)
{
    IMSA_HILOGI("testUnRegisterImeEventListener_016 start.");
    auto listener = std::make_shared<ImeSettingListenerTestImpl>();
    auto ret = ImeEventMonitorManager::GetInstance().RegisterImeEventListener(
        { EventType::IME_SHOW, EventType::IME_HIDE }, listener);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(InputMethodController::GetInstance()->clientInfo_.eventFlag, 6);
    EXPECT_EQ(ImeEventMonitorManagerImpl::GetInstance().listeners_.size(), 2);
    ret = ImeEventMonitorManager::GetInstance().UnRegisterImeEventListener(
        { EventType::IME_SHOW, EventType::IME_HIDE }, listener);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(ImeEventMonitorManagerImpl::GetInstance().listeners_.empty());
    EXPECT_EQ(InputMethodController::GetInstance()->clientInfo_.eventFlag, 0);
}

/**
* @tc.name: testUnRegisterImeEventListener_017
* @tc.desc: two listener register same event, unregister one listener
* @tc.type: FUNC
*/
HWTEST_F(ImeEventMonitorManagerTest, testUnRegisterImeEventListener_017, TestSize.Level0)
{
    IMSA_HILOGI("testUnRegisterImeEventListener_017 start.");
    auto listener = std::make_shared<ImeSettingListenerTestImpl>();
    auto listener1 = std::make_shared<ImeSettingListenerTestImpl>();
    auto ret = ImeEventMonitorManager::GetInstance().RegisterImeEventListener({ EventType::IME_SHOW }, listener);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = ImeEventMonitorManager::GetInstance().RegisterImeEventListener({ EventType::IME_SHOW }, listener1);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(InputMethodController::GetInstance()->clientInfo_.eventFlag, 2);
    EXPECT_EQ(ImeEventMonitorManagerImpl::GetInstance().listeners_.size(), 1);
    ret = ImeEventMonitorManager::GetInstance().UnRegisterImeEventListener({ EventType::IME_SHOW }, listener);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(InputMethodController::GetInstance()->clientInfo_.eventFlag, 2);
    EXPECT_EQ(ImeEventMonitorManagerImpl::GetInstance().listeners_.size(), 1);
    auto it = ImeEventMonitorManagerImpl::GetInstance().listeners_.find(EventType::IME_SHOW);
    ASSERT_NE(it, ImeEventMonitorManagerImpl::GetInstance().listeners_.end());
    ASSERT_EQ(it->second.size(), 1);
    auto iter = it->second.find(listener1);
    EXPECT_NE(iter, it->second.end());
}

/**
* @tc.name: testUnRegisterImeEventListener_018
* @tc.desc: two listener register same event, unregister one listener with error event
* @tc.type: FUNC
*/
HWTEST_F(ImeEventMonitorManagerTest, testUnRegisterImeEventListener_018, TestSize.Level0)
{
    IMSA_HILOGI("testUnRegisterImeEventListener_018 start.");
    auto listener = std::make_shared<ImeSettingListenerTestImpl>();
    auto listener1 = std::make_shared<ImeSettingListenerTestImpl>();
    auto ret = ImeEventMonitorManager::GetInstance().RegisterImeEventListener({ EventType::IME_SHOW }, listener);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = ImeEventMonitorManager::GetInstance().RegisterImeEventListener({ EventType::IME_SHOW }, listener1);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(InputMethodController::GetInstance()->clientInfo_.eventFlag, 2);
    EXPECT_EQ(ImeEventMonitorManagerImpl::GetInstance().listeners_.size(), 1);
    ret = ImeEventMonitorManager::GetInstance().UnRegisterImeEventListener({ EventType::IME_HIDE }, listener);
    EXPECT_EQ(ret, ErrorCode::ERROR_BAD_PARAMETERS);
    EXPECT_EQ(InputMethodController::GetInstance()->clientInfo_.eventFlag, 2);
    EXPECT_EQ(ImeEventMonitorManagerImpl::GetInstance().listeners_.size(), 1);
    auto it = ImeEventMonitorManagerImpl::GetInstance().listeners_.find(EventType::IME_SHOW);
    ASSERT_NE(it, ImeEventMonitorManagerImpl::GetInstance().listeners_.end());
    ASSERT_EQ(it->second.size(), 2);
    auto iter = it->second.find(listener);
    EXPECT_NE(iter, it->second.end());
    iter = it->second.find(listener1);
    EXPECT_NE(iter, it->second.end());
}
/**
* @tc.name: testUnRegisterImeEventListener_019
* @tc.desc: two listener register same event, unregister one listener with error listener
* @tc.type: FUNC
*/
HWTEST_F(ImeEventMonitorManagerTest, testUnRegisterImeEventListener_019, TestSize.Level0)
{
    IMSA_HILOGI("testUnRegisterImeEventListener_019 start.");
    auto listener = std::make_shared<ImeSettingListenerTestImpl>();
    auto listener1 = std::make_shared<ImeSettingListenerTestImpl>();
    auto listener2 = std::make_shared<ImeSettingListenerTestImpl>();
    auto ret = ImeEventMonitorManager::GetInstance().RegisterImeEventListener({ EventType::IME_SHOW }, listener);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = ImeEventMonitorManager::GetInstance().RegisterImeEventListener({ EventType::IME_SHOW }, listener1);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(InputMethodController::GetInstance()->clientInfo_.eventFlag, 2);
    EXPECT_EQ(ImeEventMonitorManagerImpl::GetInstance().listeners_.size(), 1);
    ret = ImeEventMonitorManager::GetInstance().UnRegisterImeEventListener({ EventType::IME_SHOW }, listener2);
    EXPECT_EQ(ret, ErrorCode::ERROR_BAD_PARAMETERS);
    EXPECT_EQ(InputMethodController::GetInstance()->clientInfo_.eventFlag, 2);
    EXPECT_EQ(ImeEventMonitorManagerImpl::GetInstance().listeners_.size(), 1);
    auto it = ImeEventMonitorManagerImpl::GetInstance().listeners_.find(EventType::IME_SHOW);
    ASSERT_NE(it, ImeEventMonitorManagerImpl::GetInstance().listeners_.end());
    ASSERT_EQ(it->second.size(), 2);
    auto iter = it->second.find(listener);
    EXPECT_NE(iter, it->second.end());
    iter = it->second.find(listener1);
    EXPECT_NE(iter, it->second.end());
}
/********************************* all test is for innerkit above ***************************************************/
/**
* @tc.name: testRegisterImeEventListener_020
* @tc.desc: two listener, one is innerkit(register all event), one is js(register one event)
* @tc.type: FUNC
*/
HWTEST_F(ImeEventMonitorManagerTest, testRegisterImeEventListener_020, TestSize.Level0)
{
    IMSA_HILOGI("testRegisterImeEventListener_020 start.");
    auto listener = std::make_shared<ImeSettingListenerTestImpl>();
    auto listener1 = std::make_shared<ImeSettingListenerTestImpl>();
    auto ret = ImeEventMonitorManagerImpl::GetInstance().RegisterImeEventListener({ EventType::IME_CHANGE }, listener);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = ImeEventMonitorManager::GetInstance().RegisterImeEventListener({ EventType::IME_SHOW, IME_HIDE }, listener1);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(InputMethodController::GetInstance()->clientInfo_.eventFlag, 7);
    EXPECT_EQ(ImeEventMonitorManagerImpl::GetInstance().listeners_.size(), 3);
}

/**
* @tc.name: testUnRegisterImeEventListener_021
* @tc.desc: two listener, one is innerkit(register all event), one is js(register all event), js unregister IME_CHANGE
* @tc.type: FUNC
*/
HWTEST_F(ImeEventMonitorManagerTest, testUnRegisterImeEventListener_021, TestSize.Level0)
{
    IMSA_HILOGI("testUnRegisterImeEventListener_021 start.");
    auto listener = std::make_shared<ImeSettingListenerTestImpl>();
    auto listener1 = std::make_shared<ImeSettingListenerTestImpl>();
    auto ret = ImeEventMonitorManagerImpl::GetInstance().RegisterImeEventListener({ EventType::IME_CHANGE }, listener);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = ImeEventMonitorManagerImpl::GetInstance().RegisterImeEventListener({ EventType::IME_HIDE }, listener);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = ImeEventMonitorManagerImpl::GetInstance().RegisterImeEventListener({ EventType::IME_SHOW }, listener);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = ImeEventMonitorManager::GetInstance().RegisterImeEventListener({ EventType::IME_SHOW, IME_HIDE }, listener1);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(InputMethodController::GetInstance()->clientInfo_.eventFlag, 7);
    EXPECT_EQ(ImeEventMonitorManagerImpl::GetInstance().listeners_.size(), 3);
    ret = ImeEventMonitorManagerImpl::GetInstance().UnRegisterImeEventListener({ EventType::IME_CHANGE }, listener);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(InputMethodController::GetInstance()->clientInfo_.eventFlag, 6);
    EXPECT_EQ(ImeEventMonitorManagerImpl::GetInstance().listeners_.size(), 2);
}
} // namespace MiscServices
} // namespace OHOS