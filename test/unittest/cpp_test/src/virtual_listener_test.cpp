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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <string_ex.h>
#include <unistd.h>

#include "global.h"
#include "ime_event_listener.h"
#include "ime_setting_listener_test_impl.h"
#include "ime_system_channel.h"
#include "input_method_controller.h"
#include "input_method_engine_listener.h"
#include "input_method_utils.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace MiscServices {
/**
 * @brief Only pure virtual functions are implemented.
 */
class TextListenerImpl : public OnTextChangedListener {
public:
    void InsertText(const std::u16string &text) override
    {
    }
    void DeleteForward(int32_t length) override
    {
    }
    void DeleteBackward(int32_t length) override
    {
    }
    void SendKeyEventFromInputMethod(const KeyEvent &event) override
    {
    }
    void SendKeyboardStatus(const KeyboardStatus &keyboardStatus) override
    {
    }
    void SendFunctionKey(const FunctionKey &functionKey) override
    {
    }
    void SetKeyboardStatus(bool status) override
    {
    }
    void MoveCursor(const Direction direction) override
    {
    }
    void HandleSetSelection(int32_t start, int32_t end) override
    {
    }
    void HandleExtendAction(int32_t action) override
    {
    }
    void HandleSelect(int32_t keyCode, int32_t cursorMoveSkip) override
    {
    }
    std::u16string GetLeftTextOfCursor(int32_t number) override
    {
        return Str8ToStr16("test");
    }
    std::u16string GetRightTextOfCursor(int32_t number) override
    {
        return Str8ToStr16("test");
    }
    int32_t GetTextIndexAtCursor() override
    {
        return 0;
    }
};
/**
 * @brief Only pure virtual functions are implemented.
 */
class EngineListenerImpl : public InputMethodEngineListener {
public:
    void OnKeyboardStatus(bool isShow) override
    {
    }
    void OnInputStart() override
    {
    }
    int32_t OnInputStop() override
    {
        return ErrorCode::NO_ERROR;
    }
    void OnSecurityChange(int32_t security) override
    {
    }
    void OnSetCallingWindow(uint32_t windowId) override
    {
    }
    void OnSetSubtype(const SubProperty &property) override
    {
    }
    void ReceivePrivateCommand(const std::unordered_map<std::string, PrivateDataValue> &privateCommand) override
    {
    }
};
/**
 * @brief Only pure virtual functions are implemented.
 */
class EventListenerImpl : public ImeEventListener {
};
/**
 * @brief Only pure virtual functions are implemented.
 */
class SystemCmdChannelImpl : public OnSystemCmdListener {
};

class SystemCmdChannelListenerImpl : public OnSystemCmdListener {
public:
    void ReceivePrivateCommand(const std::unordered_map<std::string, PrivateDataValue> &privateCommand) override
    {
        isReceivePrivateCommand_ = true;
    }
    void NotifyPanelStatus(const SysPanelStatus &sysPanelStatus)override
    {
        isNotifyIsShowSysPanel_ = true;
    }
    static void ResetParam()
    {
        isReceivePrivateCommand_ = false;
        isNotifyIsShowSysPanel_ = false;
    }
    static bool isReceivePrivateCommand_;
    static bool isNotifyIsShowSysPanel_;
};
bool SystemCmdChannelListenerImpl::isReceivePrivateCommand_{ false };
bool SystemCmdChannelListenerImpl::isNotifyIsShowSysPanel_{ false };

class VirtualListenerTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {
        IMSA_HILOGI("VirtualListenerTest::SetUpTestCase");
        textListener_ = new (std::nothrow) TextListenerImpl();
        eventListener_ = std::make_shared<EventListenerImpl>();
        engineListener_ = std::make_shared<EngineListenerImpl>();
        systemCmdListener_ = new (std::nothrow) SystemCmdChannelImpl();
    }
    static void TearDownTestCase(void)
    {
        IMSA_HILOGI("VirtualListenerTest::TearDownTestCase");
    }
    void SetUp()
    {
        IMSA_HILOGI("VirtualListenerTest::SetUp");
    }
    void TearDown()
    {
        IMSA_HILOGI("VirtualListenerTest::TearDown");
    }
    static sptr<OnTextChangedListener> textListener_;
    static std::shared_ptr<ImeEventListener> eventListener_;
    static std::shared_ptr<InputMethodEngineListener> engineListener_;
    static sptr<OnSystemCmdListener> systemCmdListener_;
};
sptr<OnTextChangedListener> VirtualListenerTest::textListener_{ nullptr };
std::shared_ptr<ImeEventListener> VirtualListenerTest::eventListener_{ nullptr };
std::shared_ptr<InputMethodEngineListener> VirtualListenerTest::engineListener_{ nullptr };
sptr<OnSystemCmdListener> VirtualListenerTest::systemCmdListener_{ nullptr };

/**
 * @tc.name: testOnTextChangedListener_001
 * @tc.desc: Cover non-pure virtual function in class: OnTextChangedListener.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(VirtualListenerTest, testOnTextChangedListener_001, TestSize.Level0)
{
    IMSA_HILOGI("VirtualListenerTest testOnTextChangedListener_001 START");
    ASSERT_NE(VirtualListenerTest::textListener_, nullptr);
    PanelStatusInfo statusInfo;
    Range range;
    std::unordered_map<std::string, PrivateDataValue> privateCommand;
    VirtualListenerTest::textListener_->NotifyPanelStatusInfo(statusInfo);
    VirtualListenerTest::textListener_->NotifyKeyboardHeight(0);
    int32_t ret = VirtualListenerTest::textListener_->ReceivePrivateCommand(privateCommand);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    VirtualListenerTest::textListener_->FinishTextPreview();
    ret = VirtualListenerTest::textListener_->SetPreviewText(Str8ToStr16("test"), range);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: testInputMethodEngineListener_001
 * @tc.desc: Cover non-pure virtual function in class: InputMethodEngineListener.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(VirtualListenerTest, testInputMethodEngineListener_001, TestSize.Level0)
{
    IMSA_HILOGI("VirtualListenerTest testInputMethodEngineListener_001 START");
    ASSERT_NE(VirtualListenerTest::engineListener_, nullptr);
    VirtualListenerTest::engineListener_->OnInputFinish();
    bool isEnable = VirtualListenerTest::engineListener_->IsEnable();
    EXPECT_FALSE(isEnable);
}

/**
 * @tc.name: testInputMethodEngineListener_002
 * @tc.desc: Cover non-pure virtual function in class: InputMethodEngineListener.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(VirtualListenerTest, testInputMethodEngineListener_002, TestSize.Level0)
{
    IMSA_HILOGI("VirtualListenerTest testInputMethodEngineListener_002 START");
    ASSERT_NE(VirtualListenerTest::engineListener_, nullptr);
    int32_t security = 1;
    VirtualListenerTest::engineListener_->OnSecurityChange(security);
    bool isEnable = VirtualListenerTest::engineListener_->IsEnable();
    EXPECT_FALSE(isEnable);
}

/**
 * @tc.name: testImeEventListener_001
 * @tc.desc: Cover non-pure virtual function in class: ImeEventListener.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(VirtualListenerTest, testImeEventListener_001, TestSize.Level0)
{
    IMSA_HILOGI("VirtualListenerTest testImeEventListener_001 START");
    ASSERT_NE(VirtualListenerTest::eventListener_, nullptr);
    Property property;
    SubProperty subProperty;
    ImeWindowInfo imeWindowInfo;
    auto listener = std::make_shared<ImeSettingListenerTestImpl>();
    ImeSettingListenerTestImpl::ResetParam();
    VirtualListenerTest::eventListener_->OnImeChange(property, subProperty);
    VirtualListenerTest::eventListener_->OnImeShow(imeWindowInfo);
    VirtualListenerTest::eventListener_->OnImeHide(imeWindowInfo);
    EXPECT_FALSE(ImeSettingListenerTestImpl::WaitImeChange());
    EXPECT_FALSE(ImeSettingListenerTestImpl::WaitPanelHide());
    EXPECT_FALSE(ImeSettingListenerTestImpl::WaitPanelHide());
}

/**
 * @tc.name: testOnSystemCmdListener_001
 * @tc.desc: Cover non-pure virtual function in class: OnSystemCmdListener.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(VirtualListenerTest, testOnSystemCmdListener_001, TestSize.Level0)
{
    IMSA_HILOGI("VirtualListenerTest testOnSystemCmdListener_001 START");
    sptr<OnSystemCmdListener> listener = new (std::nothrow) SystemCmdChannelListenerImpl();
    ASSERT_NE(listener, nullptr);
    ASSERT_NE(VirtualListenerTest::systemCmdListener_, nullptr);
    SystemCmdChannelListenerImpl::ResetParam();
    std::unordered_map<std::string, PrivateDataValue> privateCommand;
    VirtualListenerTest::systemCmdListener_->ReceivePrivateCommand(privateCommand);
    VirtualListenerTest::systemCmdListener_->NotifyPanelStatus({ false, 0, 0, 0 });
    EXPECT_FALSE(SystemCmdChannelListenerImpl::isNotifyIsShowSysPanel_);
    EXPECT_FALSE(SystemCmdChannelListenerImpl::isReceivePrivateCommand_);
    SystemCmdChannelListenerImpl::ResetParam();
    listener->ReceivePrivateCommand(privateCommand);
    listener->NotifyPanelStatus({ false, 0, 0, 0 });
    EXPECT_TRUE(SystemCmdChannelListenerImpl::isNotifyIsShowSysPanel_);
    EXPECT_TRUE(SystemCmdChannelListenerImpl::isReceivePrivateCommand_);
}
} // namespace MiscServices
} // namespace OHOS