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
#define private public
#define protected public
#include "input_method_panel.h"
#undef private

#include <gtest/gtest.h>
#include <sys/time.h>
#include <unistd.h>

#include <condition_variable>
#include <cstdint>
#include <string>

#include "common_event_data.h"
#include "common_event_manager.h"
#include "common_event_subscribe_info.h"
#include "common_event_subscriber.h"
#include "common_event_support.h"
#include "display_manager.h"
#include "global.h"
#include "ime_event_monitor_manager.h"
#include "input_method_ability.h"
#include "input_method_controller.h"
#include "input_method_engine_listener_impl.h"
#include "matching_skills.h"
#include "panel_status_listener.h"
#include "scope_utils.h"
#include "tdd_util.h"
#include "text_listener.h"

using namespace testing::ext;
namespace OHOS {
namespace MiscServices {
constexpr uint32_t IMC_WAIT_PANEL_STATUS_LISTEN_TIME = 200;
constexpr float FIXED_SOFT_KEYBOARD_PANEL_RATIO = 0.7;
constexpr float NON_FIXED_SOFT_KEYBOARD_PANEL_RATIO = 1;
constexpr const char *COMMON_EVENT_INPUT_PANEL_STATUS_CHANGED = "usual.event.imf.input_panel_status_changed";
constexpr const char *COMMON_EVENT_PARAM_PANEL_STATE = "panelState";
enum ListeningStatus : uint32_t { ON, OFF, NONE };
class InputMethodPanelTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static std::shared_ptr<InputMethodPanel> CreatePanel();
    static void DestroyPanel(const std::shared_ptr<InputMethodPanel> &panel);
    static void Attach();
    static void InitPanel();
    static void ClearPanel();
    static bool TriggerShowCallback(std::shared_ptr<InputMethodPanel> &inputMethodPanel);
    static bool TriggerHideCallback(std::shared_ptr<InputMethodPanel> &inputMethodPanel);
    static void ImaCreatePanel(const PanelInfo &info, std::shared_ptr<InputMethodPanel> &panel);
    static void ImaDestroyPanel(const std::shared_ptr<InputMethodPanel> &panel);
    static void ImcPanelListeningTestRestore();
    static void ImcPanelShowNumCheck(uint32_t num);
    static void ImcPanelHideNumCheck(uint32_t num);
    static void ImcPanelShowInfoCheck(const InputWindowInfo &windowInfo);
    static void ImcPanelHideInfoCheck(const InputWindowInfo &windowInfo);
    static void TestShowPanel(const std::shared_ptr<InputMethodPanel> &panel);
    static void TestHidePanel(const std::shared_ptr<InputMethodPanel> &panel);
    static void TestIsPanelShown(const PanelInfo &info, bool expectedResult);
    class PanelStatusListenerImpl : public PanelStatusListener {
    public:
        PanelStatusListenerImpl()
        {
            std::shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create("InputMethodPanelTest");
            panelHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
        }
        ~PanelStatusListenerImpl() = default;
        void OnPanelStatus(uint32_t windowId, bool isShow)
        {
            {
                std::unique_lock<std::mutex> lock(InputMethodPanelTest::panelListenerLock_);
                if (isShow) {
                    InputMethodPanelTest::status_ = InputWindowStatus::SHOW;
                } else {
                    InputMethodPanelTest::status_ = InputWindowStatus::HIDE;
                }
            }
            InputMethodPanelTest::panelListenerCv_.notify_one();
            IMSA_HILOGI("PanelStatusListenerImpl OnPanelStatus in, isShow is %{public}s", isShow ? "true" : "false");
        }
    };
    static std::mutex imcPanelStatusListenerLock_;
    static std::condition_variable imcPanelStatusListenerCv_;
    static InputWindowStatus status_;
    static InputWindowInfo windowInfo_;
    static uint32_t imeShowCallbackNum_;
    static uint32_t imeHideCallbackNum_;

    static sptr<InputMethodController> imc_;
    static sptr<InputMethodAbility> ima_;
    static uint32_t windowWidth_;
    static uint32_t windowHeight_;
    static std::condition_variable panelListenerCv_;
    static std::mutex panelListenerLock_;
    static constexpr uint32_t DELAY_TIME = 100;
    static constexpr int32_t INTERVAL = 10;
    static std::shared_ptr<AppExecFwk::EventHandler> panelHandler_;
    static uint64_t sysTokenId_;
    static int32_t currentImeUid_;
    static uint64_t currentImeTokenId_;
    static sptr<OnTextChangedListener> textListener_;
    static std::shared_ptr<InputMethodEngineListener> imeListener_;
    static std::shared_ptr<InputMethodPanel> inputMethodPanel_;
    static std::shared_ptr<InputMethodPanel> inputMethodStatusBar_;
};
class InputMethodSettingListenerImpl : public ImeEventListener {
public:
    InputMethodSettingListenerImpl() = default;
    ~InputMethodSettingListenerImpl() = default;
    void OnImeShow(const ImeWindowInfo &info) override
    {
        IMSA_HILOGI("InputMethodPanelTest::OnImeShow");
        std::unique_lock<std::mutex> lock(InputMethodPanelTest::imcPanelStatusListenerLock_);
        InputMethodPanelTest::status_ = InputWindowStatus::SHOW;
        InputMethodPanelTest::windowInfo_ = info.windowInfo;
        InputMethodPanelTest::imeShowCallbackNum_++;
        InputMethodPanelTest::imcPanelStatusListenerCv_.notify_one();
    }
    void OnImeHide(const ImeWindowInfo &info) override
    {
        IMSA_HILOGI("InputMethodPanelTest::OnImeHide");
        std::unique_lock<std::mutex> lock(InputMethodPanelTest::imcPanelStatusListenerLock_);
        InputMethodPanelTest::status_ = InputWindowStatus::HIDE;
        InputMethodPanelTest::windowInfo_ = info.windowInfo;
        InputMethodPanelTest::imeHideCallbackNum_++;
        InputMethodPanelTest::imcPanelStatusListenerCv_.notify_one();
    }
};

class TestEventSubscriber : public EventFwk::CommonEventSubscriber {
public:
    explicit TestEventSubscriber(const EventFwk::CommonEventSubscribeInfo &subscribeInfo)
        : EventFwk::CommonEventSubscriber(subscribeInfo)
    {
    }
    void OnReceiveEvent(const EventFwk::CommonEventData &data)
    {
        std::unique_lock<std::mutex> lock(InputMethodPanelTest::imcPanelStatusListenerLock_);
        auto const &want = data.GetWant();
        action_ = want.GetAction();
        bool visible = want.GetBoolParam(COMMON_EVENT_PARAM_PANEL_STATE, false);
        status_ = visible ? InputWindowStatus::SHOW : InputWindowStatus::HIDE;
        InputMethodPanelTest::imcPanelStatusListenerCv_.notify_one();
    }
    void ResetParam()
    {
        action_ = "";
        status_ = InputWindowStatus::NONE;
    }
    std::string action_;
    InputWindowStatus status_{ InputWindowStatus::NONE };
};

std::condition_variable InputMethodPanelTest::panelListenerCv_;
std::mutex InputMethodPanelTest::panelListenerLock_;
std::shared_ptr<AppExecFwk::EventHandler> InputMethodPanelTest::panelHandler_{ nullptr };
std::condition_variable InputMethodPanelTest::imcPanelStatusListenerCv_;
std::mutex InputMethodPanelTest::imcPanelStatusListenerLock_;
InputWindowStatus InputMethodPanelTest::status_{ InputWindowStatus::NONE };
InputWindowInfo InputMethodPanelTest::windowInfo_;
uint32_t InputMethodPanelTest::imeShowCallbackNum_{ 0 };
uint32_t InputMethodPanelTest::imeHideCallbackNum_{ 0 };
sptr<InputMethodController> InputMethodPanelTest::imc_{ nullptr };
sptr<InputMethodAbility> InputMethodPanelTest::ima_{ nullptr };
uint32_t InputMethodPanelTest::windowWidth_ = 0;
uint32_t InputMethodPanelTest::windowHeight_ = 0;
uint64_t InputMethodPanelTest::sysTokenId_ = 0;
uint64_t InputMethodPanelTest::currentImeTokenId_ = 0;
int32_t InputMethodPanelTest::currentImeUid_ = 0;
sptr<OnTextChangedListener> InputMethodPanelTest::textListener_{ nullptr };
std::shared_ptr<InputMethodEngineListener> InputMethodPanelTest::imeListener_{ nullptr };
std::shared_ptr<InputMethodPanel> InputMethodPanelTest::inputMethodPanel_{ nullptr };
std::shared_ptr<InputMethodPanel> InputMethodPanelTest::inputMethodStatusBar_{ nullptr };
void InputMethodPanelTest::SetUpTestCase(void)
{
    IMSA_HILOGI("InputMethodPanelTest::SetUpTestCase");
    // storage current token id
    TddUtil::StorageSelfTokenID();
    auto listener = std::make_shared<InputMethodSettingListenerImpl>();
    imc_ = InputMethodController::GetInstance();
    textListener_ = new (std::nothrow) TextListener();
    imeListener_ = std::make_shared<InputMethodEngineListenerImpl>();
    // set token as current input method
    std::shared_ptr<Property> property = InputMethodController::GetInstance()->GetCurrentInputMethod();
    std::string bundleName = property != nullptr ? property->name : "default.inputmethod.unittest";
    currentImeTokenId_ = TddUtil::GetTestTokenID(bundleName);
    currentImeUid_ = TddUtil::GetUid(bundleName);
    sysTokenId_ = TddUtil::AllocTestTokenID(true, "undefined", {});
    {
        TokenScope tokenScope(currentImeTokenId_);
        ima_ = InputMethodAbility::GetInstance();
        ima_->SetCoreAndAgent();
        InputMethodPanelTest::ima_->SetImeListener(imeListener_);
    }
    InitPanel();
}

void InputMethodPanelTest::TearDownTestCase(void)
{
    IMSA_HILOGI("InputMethodPanelTest::TearDownTestCase");
    ClearPanel();
    TddUtil::RestoreSelfTokenID();
    TddUtil::KillImsaProcess();
}

void InputMethodPanelTest::SetUp(void)
{
    IMSA_HILOGI("InputMethodPanelTest::SetUp");
}

void InputMethodPanelTest::TearDown(void)
{
    TddUtil::RestoreSelfTokenID();
    IMSA_HILOGI("InputMethodPanelTest::TearDown");
}

std::shared_ptr<InputMethodPanel> InputMethodPanelTest::CreatePanel()
{
    AccessScope scope(currentImeTokenId_, currentImeUid_);
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo = { .panelType = SOFT_KEYBOARD, .panelFlag = FLG_FIXED };
    auto ret = inputMethodPanel->CreatePanel(nullptr, panelInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    return inputMethodPanel;
}

void InputMethodPanelTest::DestroyPanel(const std::shared_ptr<InputMethodPanel> &panel)
{
    ASSERT_NE(panel, nullptr);
    AccessScope scope(currentImeTokenId_, currentImeUid_);
    auto ret = panel->DestroyPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

void InputMethodPanelTest::ImaCreatePanel(const PanelInfo &info, std::shared_ptr<InputMethodPanel> &panel)
{
    AccessScope scope(currentImeTokenId_, currentImeUid_);
    auto ret = ima_->CreatePanel(nullptr, info, panel);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

void InputMethodPanelTest::ImaDestroyPanel(const std::shared_ptr<InputMethodPanel> &panel)
{
    AccessScope scope(currentImeTokenId_, currentImeUid_);
    auto ret = ima_->DestroyPanel(panel);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

void InputMethodPanelTest::Attach()
{
    TddUtil::InitWindow(true);
    auto ret = imc_->Attach(textListener_, false);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

void InputMethodPanelTest::InitPanel()
{
    IMSA_HILOGI("start");
    AccessScope scope(currentImeTokenId_, currentImeUid_);
    inputMethodPanel_ = std::make_shared<InputMethodPanel>();
    PanelInfo info = { .panelType = SOFT_KEYBOARD, .panelFlag = FLG_FIXED };
    auto ret = inputMethodPanel_->CreatePanel(nullptr, info);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    auto defaultDisplay = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    ASSERT_TRUE(defaultDisplay != nullptr);
    windowWidth_ = defaultDisplay->GetWidth();
    windowHeight_ = 1;
    ret = inputMethodPanel_->Resize(windowWidth_, windowHeight_);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    inputMethodStatusBar_ = std::make_shared<InputMethodPanel>();
    info = { .panelType = STATUS_BAR };
    ret = inputMethodStatusBar_->CreatePanel(nullptr, info);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    IMSA_HILOGI("end");
}

void InputMethodPanelTest::ClearPanel()
{
    if (inputMethodPanel_ == nullptr) {
        IMSA_HILOGD("nullptr");
        return;
    }
    AccessScope(currentImeTokenId_, currentImeUid_);
    inputMethodPanel_->DestroyPanel();
}

bool InputMethodPanelTest::TriggerShowCallback(std::shared_ptr<InputMethodPanel> &inputMethodPanel)
{
    IMSA_HILOGI("start");
    status_ = InputWindowStatus::NONE;
    panelHandler_->PostTask([&inputMethodPanel]() { TestShowPanel(inputMethodPanel); }, InputMethodPanelTest::INTERVAL);
    {
        std::unique_lock<std::mutex> lock(panelListenerLock_);
        return panelListenerCv_.wait_for(lock, std::chrono::milliseconds(InputMethodPanelTest::DELAY_TIME),
            [] { return status_ == InputWindowStatus::SHOW; });
    }
}

bool InputMethodPanelTest::TriggerHideCallback(std::shared_ptr<InputMethodPanel> &inputMethodPanel)
{
    IMSA_HILOGI("start");
    status_ = InputWindowStatus::NONE;
    panelHandler_->PostTask([&inputMethodPanel]() { TestHidePanel(inputMethodPanel); }, InputMethodPanelTest::INTERVAL);
    {
        std::unique_lock<std::mutex> lock(panelListenerLock_);
        return panelListenerCv_.wait_for(lock, std::chrono::milliseconds(InputMethodPanelTest::DELAY_TIME),
            [] { return status_ == InputWindowStatus::HIDE; });
    }
}

void InputMethodPanelTest::ImcPanelShowNumCheck(uint32_t num)
{
    std::unique_lock<std::mutex> lock(imcPanelStatusListenerLock_);
    if (num == 0) {
        auto ret =
            imcPanelStatusListenerCv_.wait_for(lock, std::chrono::milliseconds(IMC_WAIT_PANEL_STATUS_LISTEN_TIME));
        EXPECT_EQ(ret, std::cv_status::timeout);
        return;
    }
    bool ret = imcPanelStatusListenerCv_.wait_for(lock, std::chrono::milliseconds(IMC_WAIT_PANEL_STATUS_LISTEN_TIME),
        [&num] { return num == imeShowCallbackNum_; });
    EXPECT_TRUE(ret);
}

void InputMethodPanelTest::ImcPanelHideNumCheck(uint32_t num)
{
    std::unique_lock<std::mutex> lock(imcPanelStatusListenerLock_);
    if (num == 0) {
        auto ret =
            imcPanelStatusListenerCv_.wait_for(lock, std::chrono::milliseconds(IMC_WAIT_PANEL_STATUS_LISTEN_TIME));
        EXPECT_EQ(ret, std::cv_status::timeout);
        return;
    }
    bool ret = imcPanelStatusListenerCv_.wait_for(lock, std::chrono::milliseconds(IMC_WAIT_PANEL_STATUS_LISTEN_TIME),
        [&num] { return num == imeHideCallbackNum_; });
    EXPECT_TRUE(ret);
}

void InputMethodPanelTest::ImcPanelShowInfoCheck(const InputWindowInfo &windowInfo)
{
    std::unique_lock<std::mutex> lock(imcPanelStatusListenerLock_);
    bool ret = imcPanelStatusListenerCv_.wait_for(lock, std::chrono::milliseconds(IMC_WAIT_PANEL_STATUS_LISTEN_TIME),
        [] { return status_ == InputWindowStatus::SHOW; });
    EXPECT_TRUE(ret);
    IMSA_HILOGI("InputMethodPanelTest::name: %{public}s, ret:[%{public}d, %{public}d,%{public}d, %{public}d]",
        windowInfo_.name.c_str(), windowInfo_.top, windowInfo_.left, windowInfo_.width, windowInfo_.height);
    EXPECT_FALSE(windowInfo_.name.empty());
}

void InputMethodPanelTest::ImcPanelHideInfoCheck(const InputWindowInfo &windowInfo)
{
    std::unique_lock<std::mutex> lock(imcPanelStatusListenerLock_);
    bool ret = imcPanelStatusListenerCv_.wait_for(lock, std::chrono::milliseconds(IMC_WAIT_PANEL_STATUS_LISTEN_TIME),
        [] { return status_ == InputWindowStatus::HIDE; });
    EXPECT_TRUE(ret);
    IMSA_HILOGI("InputMethodPanelTest::name: %{public}s, ret:[%{public}d, %{public}d,%{public}d, %{public}d]",
        windowInfo_.name.c_str(), windowInfo_.top, windowInfo_.left, windowInfo_.width, windowInfo_.height);
    EXPECT_FALSE(windowInfo_.name.empty());
}

void InputMethodPanelTest::ImcPanelListeningTestRestore()
{
    status_ = InputWindowStatus::NONE;
    windowInfo_ = {};
    imeShowCallbackNum_ = 0;
    imeHideCallbackNum_ = 0;
}

void InputMethodPanelTest::TestShowPanel(const std::shared_ptr<InputMethodPanel> &panel)
{
    ASSERT_NE(panel, nullptr);
    // set tokenId and uid as current ime
    AccessScope scope(currentImeTokenId_, currentImeUid_);
    auto ret = panel->ShowPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

void InputMethodPanelTest::TestHidePanel(const std::shared_ptr<InputMethodPanel> &panel)
{
    ASSERT_NE(panel, nullptr);
    // set tokenId and uid as current ime
    AccessScope scope(currentImeTokenId_, currentImeUid_);
    auto ret = panel->HidePanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

void InputMethodPanelTest::TestIsPanelShown(const PanelInfo &info, bool expectedResult)
{
    // set tokenId as system app
    TokenScope scope(sysTokenId_);
    bool result = !expectedResult;
    auto ret = imc_->IsPanelShown(info, result);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(result, expectedResult);
}

/**
* @tc.name: testCreatePanel
* @tc.desc: Test CreatePanel.
* @tc.type: FUNC
*/
HWTEST_F(InputMethodPanelTest, testCreatePanel, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelTest::testCreatePanel start.");
    AccessScope scope(currentImeTokenId_, currentImeUid_);
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo = { .panelType = SOFT_KEYBOARD, .panelFlag = FLG_FLOATING };
    auto ret = inputMethodPanel->CreatePanel(nullptr, panelInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = inputMethodPanel->DestroyPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
* @tc.name: testDestroyPanel
* @tc.desc: Test DestroyPanel.
* @tc.type: FUNC
*/
HWTEST_F(InputMethodPanelTest, testDestroyPanel, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelTest::testDestroyPanel start.");
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    // not CreatePanel, DestroyPanel failed
    auto ret = inputMethodPanel->DestroyPanel();
    EXPECT_EQ(ret, ErrorCode::ERROR_NULL_POINTER);
}

/**
* @tc.name: testResizePanel001
* @tc.desc: Test Resize panel. Panels non fixed soft keyboard.
* @tc.type: FUNC
*/
HWTEST_F(InputMethodPanelTest, testResizePanel001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelTest::testResizePanel001 start.");
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    // not CreatePanel, Resize failed
    auto ret = inputMethodPanel->Resize(1, 1);
    EXPECT_EQ(ret, ErrorCode::ERROR_NULL_POINTER);

    AccessScope scope(currentImeTokenId_, currentImeUid_);
    PanelInfo panelInfo = { .panelType = SOFT_KEYBOARD, .panelFlag = FLG_FLOATING };
    ret = inputMethodPanel->CreatePanel(nullptr, panelInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    auto defaultDisplay = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    ASSERT_TRUE(defaultDisplay != nullptr);
    int32_t width = defaultDisplay->GetWidth();
    int32_t height = defaultDisplay->GetHeight();

    ret = inputMethodPanel->Resize(width - 1, height * NON_FIXED_SOFT_KEYBOARD_PANEL_RATIO - 1);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ret = inputMethodPanel->Resize(width, height * NON_FIXED_SOFT_KEYBOARD_PANEL_RATIO);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ret = inputMethodPanel->Resize(width + 1, height * NON_FIXED_SOFT_KEYBOARD_PANEL_RATIO);
    EXPECT_EQ(ret, ErrorCode::ERROR_BAD_PARAMETERS);

    ret = inputMethodPanel->Resize(width, height * NON_FIXED_SOFT_KEYBOARD_PANEL_RATIO + 1);
    EXPECT_EQ(ret, ErrorCode::ERROR_BAD_PARAMETERS);

    ret = inputMethodPanel->Resize(width + 1, height * NON_FIXED_SOFT_KEYBOARD_PANEL_RATIO + 1);
    EXPECT_EQ(ret, ErrorCode::ERROR_BAD_PARAMETERS);

    ret = inputMethodPanel->DestroyPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
* @tc.name: testResizePanel002
* @tc.desc: Test Resize panel. Fixed soft keyboard panel .
* @tc.type: FUNC
*/
HWTEST_F(InputMethodPanelTest, testResizePanel002, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelTest::testResizePanel002 start.");
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    // not CreatePanel, Resize failed
    auto ret = inputMethodPanel->Resize(1, 1);
    EXPECT_EQ(ret, ErrorCode::ERROR_NULL_POINTER);

    PanelInfo panelInfo = { .panelType = SOFT_KEYBOARD, .panelFlag = FLG_FIXED };
    AccessScope scope(currentImeTokenId_, currentImeUid_);
    ret = inputMethodPanel->CreatePanel(nullptr, panelInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    auto defaultDisplay = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    ASSERT_TRUE(defaultDisplay != nullptr);
    int32_t width = defaultDisplay->GetWidth();
    int32_t height = defaultDisplay->GetHeight();

    ret = inputMethodPanel->Resize(width - 1, height * FIXED_SOFT_KEYBOARD_PANEL_RATIO - 1);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ret = inputMethodPanel->Resize(width, height * FIXED_SOFT_KEYBOARD_PANEL_RATIO);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ret = inputMethodPanel->Resize(width + 1, height * FIXED_SOFT_KEYBOARD_PANEL_RATIO);
    EXPECT_EQ(ret, ErrorCode::ERROR_BAD_PARAMETERS);

    ret = inputMethodPanel->Resize(width, height * FIXED_SOFT_KEYBOARD_PANEL_RATIO + 1);
    EXPECT_EQ(ret, ErrorCode::ERROR_BAD_PARAMETERS);

    ret = inputMethodPanel->Resize(width + 1, height * FIXED_SOFT_KEYBOARD_PANEL_RATIO + 1);
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
    // not CreatePanel, MoveTo failed
    auto ret = inputMethodPanel->MoveTo(10, 100);
    EXPECT_EQ(ret, ErrorCode::ERROR_NULL_POINTER);

    AccessScope scope(currentImeTokenId_, currentImeUid_);
    PanelInfo panelInfo = { .panelType = SOFT_KEYBOARD, .panelFlag = FLG_FIXED };
    ret = inputMethodPanel->CreatePanel(nullptr, panelInfo);
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

    // 1 create panel, show success
    AccessScope scope(currentImeTokenId_, currentImeUid_);
    ret = inputMethodPanel->CreatePanel(nullptr, panelInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = inputMethodPanel->ShowPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    auto statusListener = std::make_shared<InputMethodPanelTest::PanelStatusListenerImpl>();
    EXPECT_TRUE(statusListener != nullptr);
    std::string type = "show";
    inputMethodPanel->SetPanelStatusListener(statusListener, type);
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
 * @tc.name: testIsPanelShown_001
 * @tc.desc: Test is panel shown.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelTest, testIsPanelShown_001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelTest::testIsPanelShown_001 start.");
    InputMethodPanelTest::Attach();
    PanelInfo panelInfo = { .panelType = SOFT_KEYBOARD, .panelFlag = FLG_FIXED };
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    InputMethodPanelTest::ImaCreatePanel(panelInfo, inputMethodPanel);

    // query when fixed soft keyboard is showing
    InputMethodPanelTest::TestShowPanel(inputMethodPanel);
    InputMethodPanelTest::TestIsPanelShown(panelInfo, true);

    // query when fixed soft keyboard is hidden
    InputMethodPanelTest::TestHidePanel(inputMethodPanel);
    InputMethodPanelTest::TestIsPanelShown(panelInfo, false);

    InputMethodPanelTest::ImaDestroyPanel(inputMethodPanel);
    InputMethodPanelTest::imc_->Close();
    TddUtil::DestroyWindow();
}

/**
 * @tc.name: testIsPanelShown_002
 * @tc.desc: Test is panel shown.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelTest, testIsPanelShown_002, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelTest::testIsPanelShown_002 start.");
    InputMethodPanelTest::Attach();
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo = { .panelType = SOFT_KEYBOARD, .panelFlag = FLG_FIXED };
    InputMethodPanelTest::ImaCreatePanel(panelInfo, inputMethodPanel);

    // query panel with old info when panel changes its flag.
    auto ret = inputMethodPanel->ChangePanelFlag(PanelFlag::FLG_FLOATING);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodPanelTest::TestShowPanel(inputMethodPanel);
    InputMethodPanelTest::TestIsPanelShown(panelInfo, false);

    // query panel with updated shown one's info when panel changes its flag.
    panelInfo.panelFlag = PanelFlag::FLG_FLOATING;
    InputMethodPanelTest::TestIsPanelShown(panelInfo, true);

    InputMethodPanelTest::ImaDestroyPanel(inputMethodPanel);
    InputMethodPanelTest::imc_->Close();
    TddUtil::DestroyWindow();
}

/**
 * @tc.name: testIsPanelShown_003
 * @tc.desc: Test is panel shown.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelTest, testIsPanelShown_003, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelTest::testIsPanelShown_003 start.");
    InputMethodPanelTest::Attach();
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo = { .panelType = STATUS_BAR };
    InputMethodPanelTest::ImaCreatePanel(panelInfo, inputMethodPanel);

    // query status bar's status when it is showing
    InputMethodPanelTest::TestShowPanel(inputMethodPanel);
    InputMethodPanelTest::TestIsPanelShown(panelInfo, true);

    // query status bar's status when it is hidden
    InputMethodPanelTest::TestHidePanel(inputMethodPanel);
    InputMethodPanelTest::TestIsPanelShown(panelInfo, false);

    InputMethodPanelTest::ImaDestroyPanel(inputMethodPanel);
    InputMethodPanelTest::imc_->Close();
    TddUtil::DestroyWindow();
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
    auto statusListener = std::make_shared<InputMethodPanelTest::PanelStatusListenerImpl>();
    // on('show')->on('hide')->show->hide
    inputMethodPanel->SetPanelStatusListener(statusListener, "show");
    inputMethodPanel->SetPanelStatusListener(statusListener, "hide");

    AccessScope scope(InputMethodPanelTest::currentImeTokenId_, InputMethodPanelTest::currentImeUid_);
    PanelInfo panelInfo = { .panelType = SOFT_KEYBOARD, .panelFlag = FLG_FIXED };
    auto ret = inputMethodPanel->CreatePanel(nullptr, panelInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    EXPECT_TRUE(InputMethodPanelTest::TriggerShowCallback(inputMethodPanel));
    EXPECT_TRUE(InputMethodPanelTest::TriggerHideCallback(inputMethodPanel));

    InputMethodPanelTest::ImaDestroyPanel(inputMethodPanel);
}

/**
* @tc.name: testGetPanelType
* @tc.desc: Test GetPanelType.
* @tc.type: FUNC
*/
HWTEST_F(InputMethodPanelTest, testGetPanelType, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelTest::testGetPanelType start.");
    AccessScope scope(InputMethodPanelTest::currentImeTokenId_, InputMethodPanelTest::currentImeUid_);
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
 * @tc.name: testGetPanelFlag
 * @tc.desc: Test GetPanelFlag.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelTest, testGetPanelFlag, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelTest::testGetPanelFlag start.");
    AccessScope scope(InputMethodPanelTest::currentImeTokenId_, InputMethodPanelTest::currentImeUid_);
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo = { .panelType = SOFT_KEYBOARD, .panelFlag = FLG_FLOATING };
    auto ret = inputMethodPanel->CreatePanel(nullptr, panelInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    auto flag = inputMethodPanel->GetPanelFlag();
    EXPECT_EQ(flag, panelInfo.panelFlag);

    ret = inputMethodPanel->ChangePanelFlag(PanelFlag::FLG_CANDIDATE_COLUMN);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    flag = inputMethodPanel->GetPanelFlag();
    EXPECT_EQ(flag, PanelFlag::FLG_CANDIDATE_COLUMN);

    ret = inputMethodPanel->DestroyPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
* @tc.name: testChangePanelFlag
* @tc.desc: Test ChangePanelFlag.
* @tc.type: FUNC
*/
HWTEST_F(InputMethodPanelTest, testChangePanelFlag, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelTest::testChangePanelFlag start.");
    AccessScope scope(InputMethodPanelTest::currentImeTokenId_, InputMethodPanelTest::currentImeUid_);
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelFlag flag = FLG_FLOATING;

    // not CreatePanel, ChangePanelFlag failed
    auto ret = inputMethodPanel->ChangePanelFlag(flag);
    EXPECT_EQ(ret, ErrorCode::ERROR_NULL_POINTER);

    PanelInfo panelInfo = { .panelType = SOFT_KEYBOARD, .panelFlag = FLG_FLOATING };
    ret = inputMethodPanel->CreatePanel(nullptr, panelInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    // panelFlag is same with the original
    ret = inputMethodPanel->ChangePanelFlag(flag);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    // panelFlag modify to FLG_FIXED
    flag = FLG_FIXED;
    ret = inputMethodPanel->ChangePanelFlag(flag);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    inputMethodPanel->DestroyPanel();

    panelInfo = { .panelType = STATUS_BAR, .panelFlag = FLG_FLOATING };
    ret = inputMethodPanel->CreatePanel(nullptr, panelInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    // panelType is STATUS_BAR, not allow ChangePanelFlag
    ret = inputMethodPanel->ChangePanelFlag(flag);
    EXPECT_EQ(ret, ErrorCode::ERROR_BAD_PARAMETERS);

    inputMethodPanel->DestroyPanel();
}

/**
* @tc.name: testClearPanelListener
* @tc.desc: Test ClearPanelListener.
* @tc.type: FUNC
*/
HWTEST_F(InputMethodPanelTest, testClearPanelListener, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelTest::testClearPanelListener start.");
    auto inputMethodPanel = InputMethodPanelTest::CreatePanel();
    auto statusListener = std::make_shared<InputMethodPanelTest::PanelStatusListenerImpl>();
    inputMethodPanel->SetPanelStatusListener(statusListener, "show");
    inputMethodPanel->SetPanelStatusListener(statusListener, "hide");

    inputMethodPanel->ClearPanelListener("show");
    EXPECT_FALSE(InputMethodPanelTest::TriggerShowCallback(inputMethodPanel));
    EXPECT_TRUE(InputMethodPanelTest::TriggerHideCallback(inputMethodPanel));

    inputMethodPanel->ClearPanelListener("hide");
    EXPECT_FALSE(InputMethodPanelTest::TriggerShowCallback(inputMethodPanel));
    EXPECT_FALSE(InputMethodPanelTest::TriggerHideCallback(inputMethodPanel));

    InputMethodPanelTest::DestroyPanel(inputMethodPanel);
}

/**
* @tc.name: testRegisterListener
* @tc.desc: Test ClearPanelListener.
* @tc.type: FUNC
*/
HWTEST_F(InputMethodPanelTest, testRegisterListener, TestSize.Level0)
{
    // on('show')->on('hide')->show->hide->off('show')->show->hide->on('show')->show
    IMSA_HILOGI("InputMethodPanelTest::testRegisterListener start.");
    auto inputMethodPanel = InputMethodPanelTest::CreatePanel();

    auto statusListener = std::make_shared<InputMethodPanelTest::PanelStatusListenerImpl>();
    inputMethodPanel->SetPanelStatusListener(statusListener, "show");
    inputMethodPanel->SetPanelStatusListener(statusListener, "hide");
    EXPECT_TRUE(InputMethodPanelTest::TriggerShowCallback(inputMethodPanel));
    EXPECT_TRUE(InputMethodPanelTest::TriggerHideCallback(inputMethodPanel));

    inputMethodPanel->ClearPanelListener("show");
    EXPECT_FALSE(InputMethodPanelTest::TriggerShowCallback(inputMethodPanel));
    EXPECT_TRUE(InputMethodPanelTest::TriggerHideCallback(inputMethodPanel));

    inputMethodPanel->SetPanelStatusListener(statusListener, "show");
    EXPECT_TRUE(InputMethodPanelTest::TriggerShowCallback(inputMethodPanel));

    InputMethodPanelTest::DestroyPanel(inputMethodPanel);
}

/*
* @tc.name: testImcPanelListening_001
* @tc.desc: SOFT_KEYBOARD|FLG_FIXED  only one listener(system app)
* @tc.type: FUNC
*/
HWTEST_F(InputMethodPanelTest, testImcPanelListening_001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelTest::testImcPanelListening_001 start.");
    auto listener = std::make_shared<InputMethodSettingListenerImpl>();
    {
        // set system app
        TokenScope tokenScope(InputMethodPanelTest::sysTokenId_);
        ImeEventMonitorManager::GetInstance().RegisterImeEventListener(EVENT_IME_SHOW_MASK, listener);
        ImeEventMonitorManager::GetInstance().RegisterImeEventListener(EVENT_IME_HIDE_MASK, listener);
    }
    InputWindowInfo info{ "", 0, 0, InputMethodPanelTest::windowWidth_, InputMethodPanelTest::windowHeight_ };
    InputMethodPanelTest::ImcPanelListeningTestRestore();
    InputMethodPanelTest::TestShowPanel(InputMethodPanelTest::inputMethodPanel_);
    InputMethodPanelTest::ImcPanelShowNumCheck(1);
    InputMethodPanelTest::ImcPanelShowInfoCheck(info);

    InputMethodPanelTest::ImcPanelListeningTestRestore();
    InputMethodPanelTest::TestHidePanel(InputMethodPanelTest::inputMethodPanel_);
    InputMethodPanelTest::ImcPanelHideNumCheck(1);
    InputMethodPanelTest::ImcPanelHideInfoCheck(info);
    {
        // set system app
        TokenScope tokenScope(InputMethodPanelTest::sysTokenId_);
        ImeEventMonitorManager::GetInstance().UnRegisterImeEventListener(EVENT_IME_SHOW_MASK, listener);
        ImeEventMonitorManager::GetInstance().UnRegisterImeEventListener(EVENT_IME_HIDE_MASK, listener);
    }
}

/**
* @tc.name: testImcPanelListening_002
* @tc.desc: SOFT_KEYBOARD|FLG_FLOATING  only one listener(system app)
* @tc.type: FUNC
*/
HWTEST_F(InputMethodPanelTest, testImcPanelListening_002, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelTest::testImcPanelListening_002 start.");
    auto listener = std::make_shared<InputMethodSettingListenerImpl>();
    {
        // set system app
        TokenScope tokenScope(InputMethodPanelTest::sysTokenId_);
        ImeEventMonitorManager::GetInstance().RegisterImeEventListener(EVENT_IME_SHOW_MASK, listener);
        ImeEventMonitorManager::GetInstance().RegisterImeEventListener(EVENT_IME_HIDE_MASK, listener);
    }
    InputWindowInfo info{ "", 0, 0, InputMethodPanelTest::windowWidth_, InputMethodPanelTest::windowHeight_ };
    InputMethodPanelTest::inputMethodPanel_->panelFlag_ = FLG_FLOATING;
    InputMethodPanelTest::ImcPanelListeningTestRestore();
    InputMethodPanelTest::TestShowPanel(InputMethodPanelTest::inputMethodPanel_);
    InputMethodPanelTest::ImcPanelShowNumCheck(1);
    InputMethodPanelTest::ImcPanelShowInfoCheck(info);

    InputMethodPanelTest::ImcPanelListeningTestRestore();
    InputMethodPanelTest::TestHidePanel(InputMethodPanelTest::inputMethodPanel_);
    InputMethodPanelTest::ImcPanelHideNumCheck(1);
    InputMethodPanelTest::ImcPanelHideInfoCheck(info);
    {
        // set system app
        TokenScope tokenScope(InputMethodPanelTest::sysTokenId_);
        ImeEventMonitorManager::GetInstance().UnRegisterImeEventListener(EVENT_IME_SHOW_MASK, listener);
        ImeEventMonitorManager::GetInstance().UnRegisterImeEventListener(EVENT_IME_HIDE_MASK, listener);
    }
    InputMethodPanelTest::inputMethodPanel_->panelFlag_ = FLG_FIXED;
}

/**
* @tc.name: testImcPanelListening_003
* @tc.desc: SOFT_KEYBOARD|FLG_CANDIDATE_COLUMN  only one listener(system app)
* @tc.type: FUNC
*/
HWTEST_F(InputMethodPanelTest, testImcPanelListening_003, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelTest::testImcPanelListening_003 start.");
    auto listener = std::make_shared<InputMethodSettingListenerImpl>();
    {
        // set system app
        TokenScope tokenScope(InputMethodPanelTest::sysTokenId_);
        ImeEventMonitorManager::GetInstance().RegisterImeEventListener(EVENT_IME_SHOW_MASK, listener);
        ImeEventMonitorManager::GetInstance().RegisterImeEventListener(EVENT_IME_HIDE_MASK, listener);
    }
    InputMethodPanelTest::inputMethodPanel_->panelFlag_ = FLG_CANDIDATE_COLUMN;
    InputMethodPanelTest::ImcPanelListeningTestRestore();
    InputMethodPanelTest::TestShowPanel(InputMethodPanelTest::inputMethodPanel_);
    InputMethodPanelTest::ImcPanelShowNumCheck(0);

    InputMethodPanelTest::ImcPanelListeningTestRestore();
    InputMethodPanelTest::TestHidePanel(InputMethodPanelTest::inputMethodPanel_);
    InputMethodPanelTest::ImcPanelHideNumCheck(0);
    {
        // set system app
        TokenScope tokenScope(InputMethodPanelTest::sysTokenId_);
        ImeEventMonitorManager::GetInstance().UnRegisterImeEventListener(EVENT_IME_SHOW_MASK, listener);
        ImeEventMonitorManager::GetInstance().UnRegisterImeEventListener(EVENT_IME_HIDE_MASK, listener);
    }
    InputMethodPanelTest::inputMethodPanel_->panelFlag_ = FLG_FIXED;
}

/**
* @tc.name: testImcPanelListening_004
* @tc.desc: STATUS_BAR  only one listener(system app)
* @tc.type: FUNC
*/
HWTEST_F(InputMethodPanelTest, testImcPanelListening_004, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelTest::testImcPanelListening_004 start.");
    auto listener = std::make_shared<InputMethodSettingListenerImpl>();
    {
        // set system app
        TokenScope tokenScope(InputMethodPanelTest::sysTokenId_);
        ImeEventMonitorManager::GetInstance().RegisterImeEventListener(EVENT_IME_SHOW_MASK, listener);
        ImeEventMonitorManager::GetInstance().RegisterImeEventListener(EVENT_IME_HIDE_MASK, listener);
    }
    InputMethodPanelTest::ImcPanelListeningTestRestore();
    InputMethodPanelTest::TestShowPanel(InputMethodPanelTest::inputMethodStatusBar_);
    InputMethodPanelTest::ImcPanelShowNumCheck(0);

    InputMethodPanelTest::ImcPanelListeningTestRestore();
    InputMethodPanelTest::TestHidePanel(InputMethodPanelTest::inputMethodStatusBar_);
    InputMethodPanelTest::ImcPanelHideNumCheck(0);
    {
        // set system app
        TokenScope tokenScope(InputMethodPanelTest::sysTokenId_);
        ImeEventMonitorManager::GetInstance().UnRegisterImeEventListener(EVENT_IME_SHOW_MASK, listener);
        ImeEventMonitorManager::GetInstance().UnRegisterImeEventListener(EVENT_IME_HIDE_MASK, listener);
    }
}

/*
* @tc.name: testImcPanelListening_005
* @tc.desc: SOFT_KEYBOARD|FLG_FIXED  Multiple listeners(native sa) register
* @tc.type: FUNC
*/
HWTEST_F(InputMethodPanelTest, testImcPanelListening_005, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelTest::testImcPanelListening_005 start.");
    // set native sa
    TddUtil::GrantNativePermission();
    auto listener1 = std::make_shared<InputMethodSettingListenerImpl>();
    auto listener2 = std::make_shared<InputMethodSettingListenerImpl>();
    auto listener3 = std::make_shared<InputMethodSettingListenerImpl>();
    ImeEventMonitorManager::GetInstance().RegisterImeEventListener(EVENT_IME_SHOW_MASK, listener1);
    ImeEventMonitorManager::GetInstance().RegisterImeEventListener(EVENT_IME_HIDE_MASK, listener1);
    ImeEventMonitorManager::GetInstance().RegisterImeEventListener(EVENT_IME_SHOW_MASK, listener2);
    ImeEventMonitorManager::GetInstance().RegisterImeEventListener(EVENT_IME_HIDE_MASK, listener2);
    ImeEventMonitorManager::GetInstance().RegisterImeEventListener(EVENT_IME_SHOW_MASK, listener3);
    TddUtil::RestoreSelfTokenID();
    // cancel native sa

    InputMethodPanelTest::ImcPanelListeningTestRestore();
    InputMethodPanelTest::TestShowPanel(InputMethodPanelTest::inputMethodPanel_);
    InputMethodPanelTest::ImcPanelShowNumCheck(3);

    InputMethodPanelTest::ImcPanelListeningTestRestore();
    InputMethodPanelTest::TestHidePanel(InputMethodPanelTest::inputMethodPanel_);
    InputMethodPanelTest::ImcPanelHideNumCheck(2);

    // set native sa
    TddUtil::GrantNativePermission();
    ImeEventMonitorManager::GetInstance().UnRegisterImeEventListener(EVENT_IME_SHOW_MASK, listener1);
    ImeEventMonitorManager::GetInstance().UnRegisterImeEventListener(EVENT_IME_HIDE_MASK, listener1);
    ImeEventMonitorManager::GetInstance().UnRegisterImeEventListener(EVENT_IME_SHOW_MASK, listener2);
    ImeEventMonitorManager::GetInstance().UnRegisterImeEventListener(EVENT_IME_HIDE_MASK, listener2);
    ImeEventMonitorManager::GetInstance().UnRegisterImeEventListener(EVENT_IME_SHOW_MASK, listener3);
    TddUtil::RestoreSelfTokenID();
    // cancel native sa
}

/*
* @tc.name: testImcPanelListening_006
* @tc.desc: SOFT_KEYBOARD|FLG_FIXED  Multiple listeners(native sa) unregister
* @tc.type: FUNC
*/
HWTEST_F(InputMethodPanelTest, testImcPanelListening_006, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelTest::testImcPanelListening_006 start.");
    // set native sa
    TddUtil::GrantNativePermission();
    auto listener1 = std::make_shared<InputMethodSettingListenerImpl>();
    auto listener2 = std::make_shared<InputMethodSettingListenerImpl>();
    auto listener3 = std::make_shared<InputMethodSettingListenerImpl>();
    ImeEventMonitorManager::GetInstance().RegisterImeEventListener(EVENT_IME_SHOW_MASK, listener1);
    ImeEventMonitorManager::GetInstance().RegisterImeEventListener(EVENT_IME_HIDE_MASK, listener1);
    ImeEventMonitorManager::GetInstance().RegisterImeEventListener(EVENT_IME_SHOW_MASK, listener2);
    ImeEventMonitorManager::GetInstance().RegisterImeEventListener(EVENT_IME_SHOW_MASK, listener3);
    // UnRegister one IME_SHOW listener
    ImeEventMonitorManager::GetInstance().UnRegisterImeEventListener(EVENT_IME_SHOW_MASK, listener1);
    TddUtil::RestoreSelfTokenID();
    // cancel native sa

    InputMethodPanelTest::ImcPanelListeningTestRestore();
    InputMethodPanelTest::TestShowPanel(InputMethodPanelTest::inputMethodPanel_);
    InputMethodPanelTest::ImcPanelShowNumCheck(2);

    InputMethodPanelTest::ImcPanelListeningTestRestore();
    InputMethodPanelTest::TestHidePanel(InputMethodPanelTest::inputMethodPanel_);
    InputMethodPanelTest::ImcPanelHideNumCheck(1);

    // UnRegister all listener
    // set native sa
    TddUtil::GrantNativePermission();
    ImeEventMonitorManager::GetInstance().UnRegisterImeEventListener(EVENT_IME_SHOW_MASK, listener2);
    ImeEventMonitorManager::GetInstance().UnRegisterImeEventListener(EVENT_IME_SHOW_MASK, listener3);
    ImeEventMonitorManager::GetInstance().UnRegisterImeEventListener(EVENT_IME_HIDE_MASK, listener1);
    TddUtil::RestoreSelfTokenID();
    // cancel native sa

    InputMethodPanelTest::ImcPanelListeningTestRestore();
    InputMethodPanelTest::TestShowPanel(InputMethodPanelTest::inputMethodPanel_);
    InputMethodPanelTest::ImcPanelShowNumCheck(0);

    InputMethodPanelTest::ImcPanelListeningTestRestore();
    InputMethodPanelTest::TestHidePanel(InputMethodPanelTest::inputMethodPanel_);
    InputMethodPanelTest::ImcPanelHideNumCheck(0);
}

/*
* @tc.name: testImcPanelListening_007
* @tc.desc: SOFT_KEYBOARD|FLG_FIXED  only one listener(native sa), register/unregister multiple events at a time
* @tc.type: FUNC
*/
HWTEST_F(InputMethodPanelTest, testImcPanelListening_007, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelTest::testImcPanelListening_007 start.");
    // set native sa
    TddUtil::GrantNativePermission();
    auto listener = std::make_shared<InputMethodSettingListenerImpl>();
    ImeEventMonitorManager::GetInstance().RegisterImeEventListener(EVENT_IME_HIDE_MASK | EVENT_IME_SHOW_MASK, listener);
    TddUtil::RestoreSelfTokenID();
    // cancel native sa
    InputMethodPanelTest::ImcPanelListeningTestRestore();
    InputMethodPanelTest::TestShowPanel(InputMethodPanelTest::inputMethodPanel_);
    InputMethodPanelTest::ImcPanelShowNumCheck(1);

    InputMethodPanelTest::ImcPanelListeningTestRestore();
    InputMethodPanelTest::TestHidePanel(InputMethodPanelTest::inputMethodPanel_);
    InputMethodPanelTest::ImcPanelHideNumCheck(1);

    // UnRegister all listener
    // set native sa
    TddUtil::GrantNativePermission();
    ImeEventMonitorManager::GetInstance().UnRegisterImeEventListener(
        EVENT_IME_HIDE_MASK | EVENT_IME_SHOW_MASK, listener);
    TddUtil::RestoreSelfTokenID();
    // cancel native sa

    InputMethodPanelTest::ImcPanelListeningTestRestore();
    InputMethodPanelTest::TestShowPanel(InputMethodPanelTest::inputMethodPanel_);
    InputMethodPanelTest::ImcPanelShowNumCheck(0);

    InputMethodPanelTest::ImcPanelListeningTestRestore();
    InputMethodPanelTest::TestHidePanel(InputMethodPanelTest::inputMethodPanel_);
    InputMethodPanelTest::ImcPanelHideNumCheck(0);
}

/*
* @tc.name: testPanelStatusChangeEventPublicTest
* @tc.desc: test subscriber can receive the panel status change event published by IMSA
* @tc.type: FUNC
*/
HWTEST_F(InputMethodPanelTest, testPanelStatusChangeEventPublicTest, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelTest::testPanelStatusChangeEventPublicTest start.");
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(COMMON_EVENT_INPUT_PANEL_STATUS_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto subscriber = std::make_shared<TestEventSubscriber>(subscriberInfo);
    auto ret = EventFwk::CommonEventManager::SubscribeCommonEvent(subscriber);
    EXPECT_TRUE(ret);

    InputMethodPanelTest::TestShowPanel(InputMethodPanelTest::inputMethodPanel_);
    {
        std::unique_lock<std::mutex> lock(imcPanelStatusListenerLock_);
        auto waitRet = imcPanelStatusListenerCv_.wait_for(
            lock, std::chrono::milliseconds(IMC_WAIT_PANEL_STATUS_LISTEN_TIME), [subscriber]() {
                return subscriber->action_ == COMMON_EVENT_INPUT_PANEL_STATUS_CHANGED
                       && subscriber->status_ == InputWindowStatus::SHOW;
            });
        EXPECT_TRUE(waitRet);
    }

    subscriber->ResetParam();
    InputMethodPanelTest::TestHidePanel(InputMethodPanelTest::inputMethodPanel_);
    {
        std::unique_lock<std::mutex> lock(imcPanelStatusListenerLock_);
        auto waitRet = imcPanelStatusListenerCv_.wait_for(
            lock, std::chrono::milliseconds(IMC_WAIT_PANEL_STATUS_LISTEN_TIME), [subscriber]() {
                return subscriber->action_ == COMMON_EVENT_INPUT_PANEL_STATUS_CHANGED
                       && subscriber->status_ == InputWindowStatus::HIDE;
            });
        EXPECT_TRUE(waitRet);
    }
}

/**
* @tc.name: testSetCallingWindow
* @tc.desc: test SetCallingWindow
* @tc.type: FUNC
*/
HWTEST_F(InputMethodPanelTest, testSetCallingWindow, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelTest::testSetCallingWindow start.");
    AccessScope scope(InputMethodPanelTest::currentImeTokenId_, InputMethodPanelTest::currentImeUid_);
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    // not CreatePanel, SetCallingWindow failed
    uint32_t windowId = 8;
    auto ret = inputMethodPanel->SetCallingWindow(windowId);
    EXPECT_EQ(ret, ErrorCode::ERROR_PANEL_NOT_FOUND);

    PanelInfo panelInfo = { .panelType = SOFT_KEYBOARD, .panelFlag = FLG_FIXED };
    ret = inputMethodPanel->CreatePanel(nullptr, panelInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ret = inputMethodPanel->SetCallingWindow(windowId);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ret = inputMethodPanel->DestroyPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}
} // namespace MiscServices
} // namespace OHOS