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
#include "inputmethod_sysevent.h"

using ::testing::_;
using ::testing::Return;

namespace OHOS {
namespace MiscServices {

#include "hisysevent.h"
#include "gmock/gmock.h"

class MockHiSysEvent {
public:
    MOCK_METHOD(int32_t, HiSysEventWrite, (const std::string&, const std::string&, HiSysEventNameSpace::EventType,
                                              const std::string&, int32_t, const std::string&, int32_t),
        ());
    MOCK_METHOD(int32_t, HiSysEventWrite, (const std::string&, const std::string&, HiSysEventNameSpace::EventType,
                                              const std::string&, int32_t, const std::string&, int32_t,
                                              const std::string&, const std::string&),
        ());
    MOCK_METHOD(int32_t, HiSysEventWrite, (const std::string&, const std::string&, HiSysEventNameSpace::EventType,
                                              const std::string&, int32_t, const std::string&, int32_t,
                                              const std::string&, const std::string&, const std::string&, int32_t),
        ());
};

class MockTimer : public Utils::Timer {
public:
    explicit MockTimer(const std::string& name) : Utils::Timer(name) {}
    MOCK_METHOD(uint32_t, Setup, (), (override));
    MOCK_METHOD(uint32_t, Register, (const TimerCallback&, uint32_t, bool), (override));
    MOCK_METHOD(void, Unregister, (uint32_t), (override));
    MOCK_METHOD(void, Shutdown, (), (override));
};

class InputMethodSysEventTest : public testing::Test {
protected:
    void SetUp() override
    {
        mockHiSysEvent = std::make_unique<MockHiSysEvent>();
    }

    void TearDown() override
    {
        mockHiSysEvent.reset();
    }

    std::unique_ptr<MockHiSysEvent> mockHiSysEvent;
};

TEST_F(InputMethodSysEventTest, TestServiceFaultReporter)
{
    EXPECT_CALL(*mockHiSysEvent,
        HiSysEventWrite("INPUTMETHOD", "SERVICE_INIT_FAILED", "FAULT", "USER_ID", 1001,
        "COMPONENT_ID", testing::StrEq("componentName"), "ERROR_CODE", 123,
        "", 0)).WillOnce(Return(HiviewDFX::SUCCESS));

    InputMethodSysEvent::GetInstance().SetUserId(1001);
    InputMethodSysEvent::GetInstance().ServiceFaultReporter("componentName", 123);
}

TEST_F(InputMethodSysEventTest, TestInputmethodFaultReporter)
{
    EXPECT_CALL(*mockHiSysEvent,
        HiSysEventWrite("INPUTMETHOD", "UNAVAILABLE_INPUTMETHOD", "FAULT", "USER_ID", 1001,
        "APP_NAME", testing::StrEq("appName"), "ERROR_CODE", 123, "INFO",
        testing::StrEq("info"))).WillOnce(Return(HiviewDFX::SUCCESS));

    InputMethodSysEvent::GetInstance().SetUserId(1001);
    InputMethodSysEvent::GetInstance().InputmethodFaultReporter(123, "appName", "info");
}

TEST_F(InputMethodSysEventTest, TestImeUsageBehaviourReporter)
{
    EXPECT_CALL(*mockHiSysEvent, HiSysEventWrite("INPUTMETHOD", "IME_USAGE", "STATISTIC",
        "IME_START", 1, "IME_CHANGE", 2)).WillOnce(Return(HiviewDFX::SUCCESS));

    InputMethodSysEvent::GetInstance().RecordEvent(IMEBehaviour::START_IME);
    InputMethodSysEvent::GetInstance().RecordEvent(IMEBehaviour::CHANGE_IME);
    InputMethodSysEvent::GetInstance().RecordEvent(IMEBehaviour::CHANGE_IME);
    InputMethodSysEvent::GetInstance().ImeUsageBehaviourReporter();
}

TEST_F(InputMethodSysEventTest, TestRecordEvent)
{
    InputMethodSysEvent::GetInstance().RecordEvent(IMEBehaviour::START_IME);
    InputMethodSysEvent::GetInstance().RecordEvent(IMEBehaviour::CHANGE_IME);
    InputMethodSysEvent::GetInstance().RecordEvent(IMEBehaviour::CHANGE_IME);

    EXPECT_EQ(
        InputMethodSysEvent::GetInstance().inputmethodBehaviour_[static_cast<int32_t>(IMEBehaviour::START_IME)], 1);
    EXPECT_EQ(
        InputMethodSysEvent::GetInstance().inputmethodBehaviour_[static_cast<int32_t>(IMEBehaviour::CHANGE_IME)], 2);
}

TEST_F(InputMethodSysEventTest, TestOperateSoftkeyboardBehaviour)
{
    EXPECT_CALL(*mockHiSysEvent,
        HiSysEventWrite("INPUTMETHOD", "OPERATE_SOFTKEYBOARD", "BEHAVIOR",
        "OPERATING", testing::StrEq("show"), "OPERATE_INFO",
        testing::StrEq("Attach: attach, bind and show soft keyboard."))).WillOnce(Return(HiviewDFX::SUCCESS));

    InputMethodSysEvent::GetInstance().OperateSoftkeyboardBehaviour(OperateIMEInfoCode::IME_SHOW_ATTACH);
}

TEST_F(InputMethodSysEventTest, TestReportImeState)
{
    EXPECT_CALL(*mockHiSysEvent,
        HiSysEventWrite("INPUTMETHOD", "IME_STATE_CHANGED", "BEHAVIOR", "STATE", 1,
        "PID", 1234, "BUNDLE_NAME", testing::StrEq("bundleName"))).WillOnce(Return(HiviewDFX::SUCCESS));

    InputMethodSysEvent::GetInstance().ReportImeState(ImeState::STATE_ENABLED, 1234, "bundleName");
}

TEST_F(InputMethodSysEventTest, TestGetOperateInfo)
{
    EXPECT_EQ(
        InputMethodSysEvent::GetInstance().GetOperateInfo(static_cast<int32_t>(OperateIMEInfoCode::IME_SHOW_ATTACH)),
        "Attach: attach, bind and show soft keyboard.");
    EXPECT_EQ(InputMethodSysEvent::GetInstance().GetOperateInfo(9999), "unknow operating.");
}

TEST_F(InputMethodSysEventTest, TestGetOperateAction)
{
    EXPECT_EQ(
        InputMethodSysEvent::GetInstance().GetOperateAction(static_cast<int32_t>(OperateIMEInfoCode::IME_SHOW_ATTACH)),
        "show");
    EXPECT_EQ(InputMethodSysEvent::GetInstance().GetOperateAction(9999), "unknow action.");
}

TEST_F(InputMethodSysEventTest, TestSetUserId)
{
    InputMethodSysEvent::GetInstance().SetUserId(1001);
    EXPECT_EQ(InputMethodSysEvent::GetInstance().userId_, 1001);
}

TEST_F(InputMethodSysEventTest, TestStopTimer)
{
    // 假设 timer_ 已经被初始化
    InputMethodSysEvent::GetInstance().timer_ = std::make_shared<Utils::Timer>("OS_imfTimer");
    InputMethodSysEvent::GetInstance().timer_->Setup();
    InputMethodSysEvent::GetInstance().timer_->Register([]() {}, 1000, true);

    EXPECT_CALL(*mockHiSysEvent, HiSysEventWrite(_, _, _, _, _, _, _, _, _, _, _)).Times(0);

    InputMethodSysEvent::GetInstance().StopTimer();
    EXPECT_EQ(InputMethodSysEvent::GetInstance().timer_, nullptr);
}

TEST_F(InputMethodSysEventTest, TestStartTimer)
{
    EXPECT_CALL(*mockHiSysEvent, HiSysEventWrite(_, _, _, _, _, _, _, _, _, _, _)).Times(0);

    auto callback = []() {};
    InputMethodSysEvent::GetInstance().StartTimer(callback, 1000);
    EXPECT_NE(InputMethodSysEvent::GetInstance().timer_, nullptr);
}

TEST_F(InputMethodSysEventTest, TestStartTimerForReport)
{
    EXPECT_CALL(*mockHiSysEvent, HiSysEventWrite(_, _, _, _, _, _, _, _, _, _, _)).Times(0);

    InputMethodSysEvent::GetInstance().StartTimerForReport();
    EXPECT_NE(InputMethodSysEvent::GetInstance().timer_, nullptr);
}

TEST_F(InputMethodSysEventTest, TestReportSystemShortCut)
{
    EXPECT_CALL(*mockHiSysEvent, HiSysEventWrite("INPUTMETHOD_UE", "SYSTEM_SHORTCUT", "BEHAVIOR",
        "PROCESS_NAME", testing::StrEq("inputmethod_service"), "SHORTCUT_NAME",
        testing::StrEq("shortcutName"))).WillOnce(Return(HiviewDFX::SUCCESS));

    InputMethodSysEvent::GetInstance().ReportSystemShortCut("shortcutName");
}
} // namespace MiscServices
} // namespace OHOS
