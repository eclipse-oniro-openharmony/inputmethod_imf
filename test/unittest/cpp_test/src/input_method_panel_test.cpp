/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include "input_method_panel.h"

#include <condition_variable>
#include <cstdint>
#include <gtest/gtest.h>
#include <string>
#include <sys/time.h>
#include <unistd.h>

#include "accesstoken_kit.h"
#include "display_manager.h"
#include "global.h"
#include "input_method_controller.h"
#include "panel_status_listener.h"
#include "token_setproc.h"

using namespace testing::ext;
namespace OHOS {
namespace MiscServices {
using namespace OHOS::Security::AccessToken;
constexpr uint32_t IMC_WAIT_PANEL_STATUS_LISTEN_TIME = 200;
enum ListeningStatus : uint32_t { ON, OFF, NONE };
class InputMethodPanelTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static void RestoreSelfTokenID();
    static void AllocTestTokenID();
    static void SetTestTokenID();
    static void ImcPanelListeningTestCheck(
        InputWindowStatus realStatus, InputWindowStatus waitStatus, const InputWindowInfo &windowInfo);
    static void ImcPanelListeningTestCheck(InputWindowStatus realStatus, InputWindowStatus waitStatus);
    static void ImcPanelListeningTestPrepare(
        std::shared_ptr<InputMethodPanel> inputMethodPanel, const PanelInfo &info, ListeningStatus status);
    static void ImcPanelListeningTestRestore(InputWindowStatus status);
    class PanelStatusListenerImpl : public PanelStatusListener {
    public:
        ~PanelStatusListenerImpl() = default;
        void OnPanelStatus(uint32_t windowId, bool isShow)
        {
            showPanel_ = isShow;
            hidePanel_ = !isShow;
            InputMethodPanelTest::panelListenerCv_.notify_one();
            IMSA_HILOGI("PanelStatusListenerImpl OnPanelStatus in, isShow is %{public}s", isShow ? "true" : "false");
        }
    };
    static std::mutex imcPanelStatusListenerLock_;
    static std::condition_variable imcPanelStatusListenerCv_;
    static InputWindowStatus status_;
    static std::vector<InputWindowInfo> windowInfo_;
    static sptr<InputMethodController> imc_;
    static uint32_t windowWidth_;
    static uint32_t windowHeight_;
    static uint64_t selfTokenId_;
    static uint64_t testTokenIdEx_;
    static bool showPanel_;
    static bool hidePanel_;
    static std::condition_variable panelListenerCv_;
    static std::mutex panelListenerLock_;
    static constexpr uint32_t DEALY_TIME = 1;
};
class InputMethodSettingListenerImpl : public InputMethodSettingListener {
public:
    InputMethodSettingListenerImpl() = default;
    ~InputMethodSettingListenerImpl() = default;
    void OnImeChange(const Property &property, const SubProperty &subProperty)
    {
    }
    void OnPanelStatusChange(const InputWindowStatus &status, const std::vector<InputWindowInfo> &windowInfo)
    {
        IMSA_HILOGI("InputMethodPanelTest::OnPanelStatusChange");
        {
            std::unique_lock<std::mutex> lock(InputMethodPanelTest::imcPanelStatusListenerLock_);
            InputMethodPanelTest::status_ = status;
            InputMethodPanelTest::windowInfo_ = windowInfo;
        }
        InputMethodPanelTest::imcPanelStatusListenerCv_.notify_one();
    }
};
bool InputMethodPanelTest::showPanel_ = false;
bool InputMethodPanelTest::hidePanel_ = false;
std::condition_variable InputMethodPanelTest::panelListenerCv_;
std::mutex InputMethodPanelTest::panelListenerLock_;
std::condition_variable InputMethodPanelTest::imcPanelStatusListenerCv_;
std::mutex InputMethodPanelTest::imcPanelStatusListenerLock_;
InputWindowStatus InputMethodPanelTest::status_{ InputWindowStatus::HIDE };
std::vector<InputWindowInfo> InputMethodPanelTest::windowInfo_;
sptr<InputMethodController> InputMethodPanelTest::imc_;
uint32_t InputMethodPanelTest::windowWidth_ = 0;
uint32_t InputMethodPanelTest::windowHeight_ = 0;
uint64_t InputMethodPanelTest::selfTokenId_ = 0;
uint64_t InputMethodPanelTest::testTokenIdEx_ = 0;
void InputMethodPanelTest::SetUpTestCase(void)
{
    selfTokenId_ = GetSelfTokenID();
    AllocTestTokenID();
    auto listener = std::make_shared<InputMethodSettingListenerImpl>();
    imc_ = InputMethodController::GetInstance();
    imc_->SetSettingListener(listener);
    IMSA_HILOGI("InputMethodPanelTest::SetUpTestCase");
}

void InputMethodPanelTest::TearDownTestCase(void)
{
    IMSA_HILOGI("InputMethodPanelTest::TearDownTestCase");
}

void InputMethodPanelTest::SetUp(void)
{
    IMSA_HILOGI("InputMethodPanelTest::SetUp");
}

void InputMethodPanelTest::TearDown(void)
{
    IMSA_HILOGI("InputMethodPanelTest::TearDown");
}

void InputMethodPanelTest::AllocTestTokenID()
{
    HapInfoParams infoParams = {
        .userID = 1, .bundleName = "imf_panel_test", .instIndex = 0, .appIDDesc = "imf_test", .isSystemApp = true
    };
    HapPolicyParams policyParams = { .apl = APL_NORMAL, .domain = "test.domain", .permList = {}, .permStateList = {} };
    auto tokenInfo = AccessTokenKit::AllocHapToken(infoParams, policyParams);
    testTokenIdEx_ = tokenInfo.tokenIDEx;
}

void InputMethodPanelTest::SetTestTokenID()
{
    auto ret = SetSelfTokenID(testTokenIdEx_);
    IMSA_HILOGD("SetSelfTokenID ret: %{public}d", ret);
}

void InputMethodPanelTest::RestoreSelfTokenID()
{
    auto ret = SetSelfTokenID(selfTokenId_);
    IMSA_HILOGD("SetSelfTokenID ret = %{public}d", ret);
}

void InputMethodPanelTest::ImcPanelListeningTestCheck(
    InputWindowStatus realStatus, InputWindowStatus waitStatus, const InputWindowInfo &windowInfo)
{
    std::unique_lock<std::mutex> lock(imcPanelStatusListenerLock_);
    imcPanelStatusListenerCv_.wait_for(lock, std::chrono::milliseconds(IMC_WAIT_PANEL_STATUS_LISTEN_TIME),
        [&waitStatus] { return waitStatus == status_; });
    EXPECT_EQ(status_, realStatus);
    ASSERT_EQ(windowInfo_.size(), 1);
    IMSA_HILOGI("InputMethodPanelTest::name: %{public}s, top: %{public}d, left: %{public}d",
        windowInfo_[0].name.c_str(), windowInfo_[0].top, windowInfo_[0].left);
    EXPECT_FALSE(windowInfo_[0].name.empty());
    EXPECT_EQ(windowInfo_[0].width, windowInfo.width);
    EXPECT_EQ(windowInfo_[0].height, windowInfo.height);
}

void InputMethodPanelTest::ImcPanelListeningTestCheck(InputWindowStatus realStatus, InputWindowStatus waitStatus)
{
    std::unique_lock<std::mutex> lock(imcPanelStatusListenerLock_);
    imcPanelStatusListenerCv_.wait_for(lock, std::chrono::milliseconds(IMC_WAIT_PANEL_STATUS_LISTEN_TIME),
        [&waitStatus] { return waitStatus == status_; });
    EXPECT_EQ(status_, realStatus);
    EXPECT_TRUE(windowInfo_.empty());
}

void InputMethodPanelTest::ImcPanelListeningTestPrepare(
    std::shared_ptr<InputMethodPanel> inputMethodPanel, const PanelInfo &info, ListeningStatus status)
{
    auto ret = inputMethodPanel->CreatePanel(nullptr, info);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    auto defaultDisplay = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    ASSERT_TRUE(defaultDisplay != nullptr);
    windowWidth_ = defaultDisplay->GetWidth() - 1;
    windowHeight_ = 1;
    ret = inputMethodPanel->Resize(windowWidth_, windowHeight_);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    switch (status) {
        case ListeningStatus::NONE: {
            break;
        }
        case ListeningStatus::ON: {
            imc_->UpdateListenEventFlag(IME_HIDE, true);
            imc_->UpdateListenEventFlag(IME_SHOW, true);
            break;
        }
        case ListeningStatus::OFF: {
            imc_->UpdateListenEventFlag(IME_HIDE, false);
            imc_->UpdateListenEventFlag(IME_SHOW, false);
            break;
        }
        default:
            break;
    }
}

void InputMethodPanelTest::ImcPanelListeningTestRestore(InputWindowStatus status)
{
    status_ = status;
    windowInfo_.clear();
}

/**
* @tc.name: testSetUiContent
* @tc.desc: Test SetUiContent.
* @tc.type: FUNC
*/
HWTEST_F(InputMethodPanelTest, testCreatePanel, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelTest::testCreatePanel start.");
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo = { .panelType = SOFT_KEYBOARD, .panelFlag = FLG_FLOATING };
    auto ret = inputMethodPanel->CreatePanel(nullptr, panelInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = inputMethodPanel->DestroyPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
* @tc.name: testResizePanel
* @tc.desc: Test Resize panel. All panels can be resized.
* @tc.type: FUNC
*/
HWTEST_F(InputMethodPanelTest, testResizePanel, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelTest::testResizePanel start.");
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo = { .panelType = SOFT_KEYBOARD, .panelFlag = FLG_FLOATING };
    auto ret = inputMethodPanel->CreatePanel(nullptr, panelInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    auto defaultDisplay = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    EXPECT_TRUE(defaultDisplay != nullptr);
    int32_t width = defaultDisplay->GetWidth();
    int32_t height = defaultDisplay->GetHeight();

    ret = inputMethodPanel->Resize(width - 1, height / 2 - 1);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ret = inputMethodPanel->Resize(width, height / 2);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ret = inputMethodPanel->Resize(width + 1, height / 2);
    EXPECT_EQ(ret, ErrorCode::ERROR_BAD_PARAMETERS);

    ret = inputMethodPanel->Resize(width, height / 2 + 1);
    EXPECT_EQ(ret, ErrorCode::ERROR_BAD_PARAMETERS);
    ret = inputMethodPanel->DestroyPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
* @tc.name: testMovePanel
* @tc.desc: Test Move panel. SOFT_KEYBOARD panel with FLG_FIXED can not be moved.
* @tc.type: FUNC
*/
HWTEST_F(InputMethodPanelTest, testMovePanel, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelTest::testMovePanel start.");
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo = { .panelType = SOFT_KEYBOARD, .panelFlag = FLG_FIXED };
    auto ret = inputMethodPanel->CreatePanel(nullptr, panelInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ret = inputMethodPanel->MoveTo(10, 100);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ret = inputMethodPanel->ChangePanelFlag(PanelFlag::FLG_FLOATING);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = inputMethodPanel->MoveTo(10, 100);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ret = inputMethodPanel->DestroyPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    panelInfo.panelType = STATUS_BAR;
    ret = inputMethodPanel->CreatePanel(nullptr, panelInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = inputMethodPanel->MoveTo(10, 100);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = inputMethodPanel->DestroyPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
* @tc.name: testShowPanel
* @tc.desc: Test Show panel.
* @tc.type: FUNC
*/
HWTEST_F(InputMethodPanelTest, testShowPanel, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelTest::testShowPanel start.");
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo = { .panelType = SOFT_KEYBOARD, .panelFlag = FLG_FIXED };
    // 0、not create panel, show panel failed.
    auto ret = inputMethodPanel->ShowPanel();
    EXPECT_EQ(ret, ErrorCode::ERROR_NULL_POINTER);
    ret = inputMethodPanel->CreatePanel(nullptr, panelInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = inputMethodPanel->ShowPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    auto statusListener = std::make_shared<InputMethodPanelTest::PanelStatusListenerImpl>();
    EXPECT_TRUE(statusListener != nullptr);
    inputMethodPanel->SetPanelStatusListener(statusListener);
    ret = inputMethodPanel->ShowPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    // 2、show floating type panel.
    ret = inputMethodPanel->ChangePanelFlag(PanelFlag::FLG_FLOATING);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = inputMethodPanel->ShowPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ret = inputMethodPanel->DestroyPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    // 4、show status bar.
    panelInfo.panelType = STATUS_BAR;
    ret = inputMethodPanel->CreatePanel(nullptr, panelInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = inputMethodPanel->ShowPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = inputMethodPanel->HidePanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = inputMethodPanel->DestroyPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
* @tc.name: testSetPanelStatusListener
* @tc.desc: Test SetPanelStatusListener.
* @tc.type: FUNC
*/
HWTEST_F(InputMethodPanelTest, testSetPanelStatusListener, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelTest::testSetPanelStatusListener start.");
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    EXPECT_TRUE(inputMethodPanel != nullptr);
    auto statusListener = std::make_shared<InputMethodPanelTest::PanelStatusListenerImpl>();
    EXPECT_TRUE(statusListener != nullptr);
    inputMethodPanel->SetPanelStatusListener(statusListener);

    PanelInfo panelInfo = { .panelType = SOFT_KEYBOARD, .panelFlag = FLG_FIXED };
    auto ret = inputMethodPanel->CreatePanel(nullptr, panelInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ret = inputMethodPanel->ShowPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    {
        std::unique_lock<std::mutex> lock(InputMethodPanelTest::panelListenerLock_);
        InputMethodPanelTest::panelListenerCv_.wait_for(lock, std::chrono::seconds(InputMethodPanelTest::DEALY_TIME),
            [] { return InputMethodPanelTest::showPanel_ == true; });
    }
    EXPECT_TRUE(InputMethodPanelTest::showPanel_);

    ret = inputMethodPanel->HidePanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    {
        std::unique_lock<std::mutex> lock(InputMethodPanelTest::panelListenerLock_);
        InputMethodPanelTest::panelListenerCv_.wait_for(lock, std::chrono::seconds(InputMethodPanelTest::DEALY_TIME),
            [] { return InputMethodPanelTest::showPanel_ == false; });
    }
    EXPECT_TRUE(!InputMethodPanelTest::showPanel_);
    ret = inputMethodPanel->DestroyPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
* @tc.name: testGetPanelType
* @tc.desc: Test GetPanelType.
* @tc.type: FUNC
*/
HWTEST_F(InputMethodPanelTest, testGetPanelType, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelTest::testSetUiContent start.");
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo = { .panelType = SOFT_KEYBOARD, .panelFlag = FLG_FLOATING };
    auto ret = inputMethodPanel->CreatePanel(nullptr, panelInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    auto type = inputMethodPanel->GetPanelType();
    EXPECT_EQ(type, panelInfo.panelType);
    ret = inputMethodPanel->DestroyPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
* @tc.name: testRemovePanelListener
* @tc.desc: Test RemovePanelListener.
* @tc.type: FUNC
*/
HWTEST_F(InputMethodPanelTest, testRemovePanelListener, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelTest::testRemovePanelListener start.");
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo = { .panelType = SOFT_KEYBOARD, .panelFlag = FLG_FLOATING };
    auto ret = inputMethodPanel->CreatePanel(nullptr, panelInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    auto type = inputMethodPanel->GetPanelType();
    EXPECT_EQ(type, panelInfo.panelType);

    std::string subscribeType = "show";
    inputMethodPanel->RemovePanelListener(subscribeType);
    ret = inputMethodPanel->ShowPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    {
        std::unique_lock<std::mutex> lock(InputMethodPanelTest::panelListenerLock_);
        InputMethodPanelTest::panelListenerCv_.wait_for(lock, std::chrono::seconds(InputMethodPanelTest::DEALY_TIME),
            [] { return InputMethodPanelTest::showPanel_ == false; });
    }
    EXPECT_EQ(InputMethodPanelTest::showPanel_, false);
    ret = inputMethodPanel->HidePanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    {
        std::unique_lock<std::mutex> lock(InputMethodPanelTest::panelListenerLock_);
        InputMethodPanelTest::panelListenerCv_.wait_for(lock, std::chrono::seconds(InputMethodPanelTest::DEALY_TIME),
            [] { return InputMethodPanelTest::hidePanel_ == true; });
    }
    EXPECT_EQ(InputMethodPanelTest::hidePanel_, true);
    InputMethodPanelTest::hidePanel_ = false;

    subscribeType = "hide";
    inputMethodPanel->RemovePanelListener(subscribeType);
    ret = inputMethodPanel->ShowPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    {
        std::unique_lock<std::mutex> lock(InputMethodPanelTest::panelListenerLock_);
        InputMethodPanelTest::panelListenerCv_.wait_for(lock, std::chrono::seconds(InputMethodPanelTest::DEALY_TIME),
            [] { return InputMethodPanelTest::showPanel_ == false; });
    }
    EXPECT_EQ(InputMethodPanelTest::showPanel_, false);
    ret = inputMethodPanel->HidePanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    {
        std::unique_lock<std::mutex> lock(InputMethodPanelTest::panelListenerLock_);
        InputMethodPanelTest::panelListenerCv_.wait_for(lock, std::chrono::seconds(InputMethodPanelTest::DEALY_TIME),
            [] { return InputMethodPanelTest::hidePanel_ == false; });
    }
    EXPECT_EQ(InputMethodPanelTest::hidePanel_, false);

    ret = inputMethodPanel->DestroyPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
* @tc.name: testImcPanelListening_001
* @tc.desc: SOFT_KEYBOARD  FLG_FIXED  no listening set up
* @tc.type: FUNC
*/
HWTEST_F(InputMethodPanelTest, testImcPanelListening_001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelTest::testImcPanelListening_001 start.");
    SetTestTokenID();
    InputMethodPanelTest::ImcPanelListeningTestRestore(InputWindowStatus::HIDE);
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo = { .panelType = SOFT_KEYBOARD, .panelFlag = FLG_FIXED };
    InputMethodPanelTest::ImcPanelListeningTestPrepare(inputMethodPanel, panelInfo, NONE);
    auto ret = inputMethodPanel->ShowPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodPanelTest::ImcPanelListeningTestCheck(InputWindowStatus::HIDE, InputWindowStatus::SHOW);

    InputMethodPanelTest::ImcPanelListeningTestRestore(InputWindowStatus::SHOW);
    ret = inputMethodPanel->HidePanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodPanelTest::ImcPanelListeningTestCheck(InputWindowStatus::SHOW, InputWindowStatus::HIDE);
    RestoreSelfTokenID();
}

/**
* @tc.name: testImcPanelListening_002
* @tc.desc: SOFT_KEYBOARD  FLG_FIXED  Set up listening
* @tc.type: FUNC
*/
HWTEST_F(InputMethodPanelTest, testImcPanelListening_002, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelTest::testImcPanelListening_002 start.");
    SetTestTokenID();
    InputMethodPanelTest::ImcPanelListeningTestRestore(InputWindowStatus::HIDE);
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo = { .panelType = SOFT_KEYBOARD, .panelFlag = FLG_FIXED };
    InputMethodPanelTest::ImcPanelListeningTestPrepare(inputMethodPanel, panelInfo, ON);

    auto ret = inputMethodPanel->ShowPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodPanelTest::ImcPanelListeningTestCheck(InputWindowStatus::SHOW, InputWindowStatus::SHOW,
        { "", 0, 0, InputMethodPanelTest::windowWidth_, InputMethodPanelTest::windowHeight_ });

    InputMethodPanelTest::ImcPanelListeningTestRestore(InputWindowStatus::SHOW);
    ret = inputMethodPanel->HidePanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodPanelTest::ImcPanelListeningTestCheck(InputWindowStatus::HIDE, InputWindowStatus::HIDE,
        { "", 0, 0, InputMethodPanelTest::windowWidth_, InputMethodPanelTest::windowHeight_ });
    RestoreSelfTokenID();
}

/**
* @tc.name: testImcPanelListening_003
* @tc.desc: SOFT_KEYBOARD  FLG_FIXED  Cancel listening
* @tc.type: FUNC
*/
HWTEST_F(InputMethodPanelTest, testImcPanelListening_003, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelTest::testImcPanelListening_003 start.");
    SetTestTokenID();
    InputMethodPanelTest::ImcPanelListeningTestRestore(InputWindowStatus::HIDE);
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo = { .panelType = SOFT_KEYBOARD, .panelFlag = FLG_FIXED };
    InputMethodPanelTest::ImcPanelListeningTestPrepare(inputMethodPanel, panelInfo, OFF);

    auto ret = inputMethodPanel->ShowPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodPanelTest::ImcPanelListeningTestCheck(InputWindowStatus::HIDE, InputWindowStatus::SHOW);

    InputMethodPanelTest::ImcPanelListeningTestRestore(InputWindowStatus::SHOW);
    ret = inputMethodPanel->HidePanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodPanelTest::ImcPanelListeningTestCheck(InputWindowStatus::SHOW, InputWindowStatus::HIDE);
    RestoreSelfTokenID();
}

/**
* @tc.name: testImcPanelListening_004
* @tc.desc: SOFT_KEYBOARD  FLG_FIXED  Set up listening  NO PERMISSION
* @tc.type: FUNC
*/
HWTEST_F(InputMethodPanelTest, testImcPanelListening_004, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelTest::testImcPanelListening_004 start.");
    InputMethodPanelTest::ImcPanelListeningTestRestore(InputWindowStatus::HIDE);
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo = { .panelType = SOFT_KEYBOARD, .panelFlag = FLG_FIXED };
    InputMethodPanelTest::ImcPanelListeningTestPrepare(inputMethodPanel, panelInfo, ON);

    auto ret = inputMethodPanel->ShowPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodPanelTest::ImcPanelListeningTestCheck(InputWindowStatus::HIDE, InputWindowStatus::SHOW);

    InputMethodPanelTest::ImcPanelListeningTestRestore(InputWindowStatus::SHOW);
    ret = inputMethodPanel->HidePanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodPanelTest::ImcPanelListeningTestCheck(InputWindowStatus::SHOW, InputWindowStatus::HIDE);
}
} // namespace MiscServices
} // namespace OHOS