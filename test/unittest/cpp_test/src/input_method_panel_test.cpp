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

#include "input_method_ability.h"
#include "input_method_ability_utils.h"
#include "input_method_controller.h"
#include "input_method_system_ability.h"
#include "task_manager.h"
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
#include "identity_checker_mock.h"
#include "ime_event_monitor_manager.h"
#include "input_method_ability.h"
#include "input_method_controller.h"
#include "input_method_engine_listener_impl.h"
#include "matching_skills.h"
#include "panel_status_listener.h"
#include "scene_board_judgement.h"
#include "scope_utils.h"
#include "tdd_util.h"
#include "text_listener.h"

using namespace testing::ext;
using namespace OHOS::Rosen;
namespace OHOS {
namespace MiscServices {
constexpr uint32_t IMC_WAIT_PANEL_STATUS_LISTEN_TIME = 500;
constexpr float FIXED_SOFT_KEYBOARD_PANEL_RATIO = 0.7;
constexpr float NON_FIXED_SOFT_KEYBOARD_PANEL_RATIO = 1;
constexpr const char *COMMON_EVENT_INPUT_PANEL_STATUS_CHANGED = "usual.event.imf.input_panel_status_changed";
constexpr const char *COMMON_EVENT_PARAM_PANEL_STATE = "panelState";
enum ListeningStatus : uint32_t {
    ON,
    OFF,
    NONE
};
class InputMethodPanelTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static std::shared_ptr<InputMethodPanel> CreatePanel();
    static void DestroyPanel(const std::shared_ptr<InputMethodPanel> &panel);
    static void Attach();
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
    static void TriggerPanelStatusChangeToImc(const std::shared_ptr<InputMethodPanel> &panel, InputWindowStatus status);
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
        void OnSizeChange(uint32_t windowId, const WindowSize &size) {}
        void OnSizeChange(
            uint32_t windowId, const WindowSize &size, const PanelAdjustInfo &keyboardArea, const std::string &event)
        {
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
    static sptr<InputMethodSystemAbility> imsa_;
    static uint32_t windowWidth_;
    static uint32_t windowHeight_;
    static std::condition_variable panelListenerCv_;
    static std::mutex panelListenerLock_;
    static constexpr uint32_t DELAY_TIME = 100;
    static constexpr int32_t INTERVAL = 10;
    static std::shared_ptr<AppExecFwk::EventHandler> panelHandler_;
    static int32_t currentImeUid_;
    static uint64_t currentImeTokenId_;
    static sptr<OnTextChangedListener> textListener_;
    static std::shared_ptr<InputMethodEngineListener> imeListener_;
    static bool isScbEnable_;
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
    InputWindowStatus status_ { InputWindowStatus::NONE };
};

std::condition_variable InputMethodPanelTest::panelListenerCv_;
std::mutex InputMethodPanelTest::panelListenerLock_;
std::shared_ptr<AppExecFwk::EventHandler> InputMethodPanelTest::panelHandler_ { nullptr };
std::condition_variable InputMethodPanelTest::imcPanelStatusListenerCv_;
std::mutex InputMethodPanelTest::imcPanelStatusListenerLock_;
InputWindowStatus InputMethodPanelTest::status_ { InputWindowStatus::NONE };
InputWindowInfo InputMethodPanelTest::windowInfo_;
uint32_t InputMethodPanelTest::imeShowCallbackNum_ { 0 };
uint32_t InputMethodPanelTest::imeHideCallbackNum_ { 0 };
sptr<InputMethodController> InputMethodPanelTest::imc_ { nullptr };
sptr<InputMethodAbility> InputMethodPanelTest::ima_ { nullptr };
sptr<InputMethodSystemAbility> InputMethodPanelTest::imsa_ { nullptr };
uint32_t InputMethodPanelTest::windowWidth_ = 0;
uint32_t InputMethodPanelTest::windowHeight_ = 0;
uint64_t InputMethodPanelTest::currentImeTokenId_ = 0;
int32_t InputMethodPanelTest::currentImeUid_ = 0;
sptr<OnTextChangedListener> InputMethodPanelTest::textListener_ { nullptr };
std::shared_ptr<InputMethodEngineListener> InputMethodPanelTest::imeListener_ { nullptr };
bool InputMethodPanelTest::isScbEnable_ { false };
void InputMethodPanelTest::SetUpTestCase(void)
{
    IMSA_HILOGI("InputMethodPanelTest::SetUpTestCase");
    IdentityCheckerMock::ResetParam();
    isScbEnable_ = Rosen::SceneBoardJudgement::IsSceneBoardEnabled();
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

    imsa_ = new (std::nothrow) InputMethodSystemAbility();
    if (imsa_ == nullptr) {
        return;
    }
    imsa_->OnStart();
    imsa_->userId_ = TddUtil::GetCurrentUserId();
    imsa_->identityChecker_ = std::make_shared<IdentityCheckerMock>();

    imc_->abilityManager_ = imsa_;

    ima_ = InputMethodAbility::GetInstance();
    {
        TokenScope scope(currentImeTokenId_);
        ima_->InitConnect();
    }
    ima_->abilityManager_ = imsa_;
    TddUtil::InitCurrentImePermissionInfo();
    IdentityCheckerMock::SetBundleName(TddUtil::currentBundleNameMock_);
    ima_->SetCoreAndAgent();
    InputMethodPanelTest::ima_->SetImeListener(imeListener_);

    ImaUtils::abilityManager_ = imsa_;
}

void InputMethodPanelTest::TearDownTestCase(void)
{
    IMSA_HILOGI("InputMethodPanelTest::TearDownTestCase");
    TddUtil::RestoreSelfTokenID();
    IdentityCheckerMock::ResetParam();
    imsa_->OnStop();
    ImaUtils::abilityManager_ = nullptr;
}

void InputMethodPanelTest::SetUp(void)
{
    IMSA_HILOGI("InputMethodPanelTest::SetUp");
    TaskManager::GetInstance().SetInited(true);
}

void InputMethodPanelTest::TearDown(void)
{
    TddUtil::RestoreSelfTokenID();
    IMSA_HILOGI("InputMethodPanelTest::TearDown");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    TaskManager::GetInstance().Reset();
}

std::shared_ptr<InputMethodPanel> InputMethodPanelTest::CreatePanel()
{
    AccessScope scope(currentImeTokenId_, currentImeUid_);
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    panelInfo.panelFlag = FLG_FIXED;
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
    IdentityCheckerMock::SetFocused(true);
    auto ret = imc_->Attach(textListener_, false);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    IdentityCheckerMock::SetFocused(false);
}

bool InputMethodPanelTest::TriggerShowCallback(std::shared_ptr<InputMethodPanel> &inputMethodPanel)
{
    IMSA_HILOGI("start");
    status_ = InputWindowStatus::NONE;
    panelHandler_->PostTask(
        [&inputMethodPanel]() {
            TestShowPanel(inputMethodPanel);
        },
        InputMethodPanelTest::INTERVAL);
    {
        std::unique_lock<std::mutex> lock(panelListenerLock_);
        return panelListenerCv_.wait_for(lock, std::chrono::milliseconds(InputMethodPanelTest::DELAY_TIME), [] {
            return status_ == InputWindowStatus::SHOW;
        });
    }
}

bool InputMethodPanelTest::TriggerHideCallback(std::shared_ptr<InputMethodPanel> &inputMethodPanel)
{
    IMSA_HILOGI("start");
    status_ = InputWindowStatus::NONE;
    panelHandler_->PostTask(
        [&inputMethodPanel]() {
            TestHidePanel(inputMethodPanel);
        },
        InputMethodPanelTest::INTERVAL);
    {
        std::unique_lock<std::mutex> lock(panelListenerLock_);
        return panelListenerCv_.wait_for(lock, std::chrono::milliseconds(InputMethodPanelTest::DELAY_TIME), [] {
            return status_ == InputWindowStatus::HIDE;
        });
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
    bool ret =
        imcPanelStatusListenerCv_.wait_for(lock, std::chrono::milliseconds(IMC_WAIT_PANEL_STATUS_LISTEN_TIME), [&num] {
            return num == imeShowCallbackNum_;
        });
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
    bool ret =
        imcPanelStatusListenerCv_.wait_for(lock, std::chrono::milliseconds(IMC_WAIT_PANEL_STATUS_LISTEN_TIME), [&num] {
            return num == imeHideCallbackNum_;
        });
    EXPECT_TRUE(ret);
}

void InputMethodPanelTest::ImcPanelShowInfoCheck(const InputWindowInfo &windowInfo)
{
    std::unique_lock<std::mutex> lock(imcPanelStatusListenerLock_);
    bool ret =
        imcPanelStatusListenerCv_.wait_for(lock, std::chrono::milliseconds(IMC_WAIT_PANEL_STATUS_LISTEN_TIME), [] {
            return status_ == InputWindowStatus::SHOW;
        });
    EXPECT_TRUE(ret);
    IMSA_HILOGI("InputMethodPanelTest::name: %{public}s, ret:[%{public}d, %{public}d,%{public}d, %{public}d]",
        windowInfo_.name.c_str(), windowInfo_.top, windowInfo_.left, windowInfo_.width, windowInfo_.height);
    EXPECT_FALSE(windowInfo_.name.empty());
}

void InputMethodPanelTest::ImcPanelHideInfoCheck(const InputWindowInfo &windowInfo)
{
    std::unique_lock<std::mutex> lock(imcPanelStatusListenerLock_);
    bool ret =
        imcPanelStatusListenerCv_.wait_for(lock, std::chrono::milliseconds(IMC_WAIT_PANEL_STATUS_LISTEN_TIME), [] {
            return status_ == InputWindowStatus::HIDE;
        });
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
    IdentityCheckerMock::SetSystemApp(true);
    bool result = !expectedResult;
    auto ret = imc_->IsPanelShown(info, result);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(result, expectedResult);
    IdentityCheckerMock::SetSystemApp(false);
}

void InputMethodPanelTest::TriggerPanelStatusChangeToImc(
    const std::shared_ptr<InputMethodPanel> &panel, InputWindowStatus status)
{
    ASSERT_NE(panel, nullptr);
    if (isScbEnable_) {
        IdentityCheckerMock::SetBundleNameValid(true);
        // add for SetTestTokenID in mainThread, but has no effect for other thread ipc
        panel->PanelStatusChangeToImc(status, { 0, 0, 0, 0 });
        IdentityCheckerMock::SetBundleNameValid(false);
    }
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
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    panelInfo.panelFlag = FLG_FLOATING;
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
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    panelInfo.panelFlag = FLG_FLOATING;
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

    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    panelInfo.panelFlag = FLG_FIXED;
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
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    panelInfo.panelFlag = FLG_FIXED;
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
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    panelInfo.panelFlag = FLG_FIXED;
    // 0、not create panel, show panel failed.
    auto ret = inputMethodPanel->ShowPanel();
    EXPECT_EQ(ret, ErrorCode::ERROR_IMA_NULLPTR);

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
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    panelInfo.panelFlag = FLG_FIXED;
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
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    panelInfo.panelFlag = FLG_FIXED;
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
    PanelInfo panelInfo;
    panelInfo.panelType = STATUS_BAR;
    InputMethodPanelTest::ImaCreatePanel(panelInfo, inputMethodPanel);

    // query status bar's status when it is showing
    InputMethodPanelTest::TestShowPanel(inputMethodPanel);
    InputMethodPanelTest::TestIsPanelShown(panelInfo, true);

    // query status bar's status when it is hidden
    InputMethodPanelTest::TestHidePanel(inputMethodPanel);
    InputMethodPanelTest::TestIsPanelShown(panelInfo, false);

    InputMethodPanelTest::ImaDestroyPanel(inputMethodPanel);
    InputMethodPanelTest::imc_->Close();
}

/**
 * @tc.name: testSetPanelStatusListener01
 * @tc.desc: Test testSetPanelStatusListener01.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelTest, testSetPanelStatusListener01, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelTest::testSetPanelStatusListener01 start.");
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    auto statusListener = std::make_shared<InputMethodPanelTest::PanelStatusListenerImpl>();
    // on('show')->on('hide')->show->hide
    inputMethodPanel->SetPanelStatusListener(statusListener, "show");
    inputMethodPanel->SetPanelStatusListener(statusListener, "hide");

    AccessScope scope(InputMethodPanelTest::currentImeTokenId_, InputMethodPanelTest::currentImeUid_);
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    panelInfo.panelFlag = FLG_FIXED;
    auto ret = inputMethodPanel->CreatePanel(nullptr, panelInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    EXPECT_TRUE(InputMethodPanelTest::TriggerShowCallback(inputMethodPanel));
    EXPECT_TRUE(InputMethodPanelTest::TriggerHideCallback(inputMethodPanel));

    InputMethodPanelTest::ImaDestroyPanel(inputMethodPanel);
}

/**
 * @tc.name: testSetPanelStatusListener02
 * @tc.desc: Test testSetPanelStatusListener02.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelTest, testSetPanelStatusListener02, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelTest::testSetPanelStatusListener02 start.");
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    auto statusListener = std::make_shared<InputMethodPanelTest::PanelStatusListenerImpl>();

    AccessScope scope(InputMethodPanelTest::currentImeTokenId_, InputMethodPanelTest::currentImeUid_);
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    panelInfo.panelFlag = FLG_FIXED;
    auto ret = inputMethodPanel->CreatePanel(nullptr, panelInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    // panelStatusListener_ not nullptr
    inputMethodPanel->panelStatusListener_ = statusListener;

    // subscribe 'show' after panel shown, get 'show' callback
    InputMethodPanelTest::status_ = InputWindowStatus::NONE;
    InputMethodPanelTest::TestShowPanel(inputMethodPanel);
    inputMethodPanel->SetPanelStatusListener(statusListener, "show");
    EXPECT_EQ(status_, InputWindowStatus::SHOW);

    // subscribe 'hide' after panel hidden, get 'hide' callback
    InputMethodPanelTest::status_ = InputWindowStatus::NONE;
    InputMethodPanelTest::TestHidePanel(inputMethodPanel);
    inputMethodPanel->SetPanelStatusListener(statusListener, "hide");
    EXPECT_EQ(status_, InputWindowStatus::HIDE);

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
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    panelInfo.panelFlag = FLG_FLOATING;
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
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    panelInfo.panelFlag = FLG_FLOATING;
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

    PanelInfo panelInfo1;
    panelInfo1.panelType = SOFT_KEYBOARD;
    panelInfo1.panelFlag = FLG_FLOATING;
    ret = inputMethodPanel->CreatePanel(nullptr, panelInfo1);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    // panelFlag is same with the original
    ret = inputMethodPanel->ChangePanelFlag(flag);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    // panelFlag modify to FLG_FIXED
    flag = FLG_FIXED;
    ret = inputMethodPanel->ChangePanelFlag(flag);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    inputMethodPanel->DestroyPanel();

	PanelInfo panelInfo;
    panelInfo.panelType = STATUS_BAR;
    panelInfo.panelFlag = FLG_FLOATING;
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
 * @tc.desc: SOFT_KEYBOARD/FLG_FIXED, listener(system app)
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelTest, testImcPanelListening_001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelTest::testImcPanelListening_001 start.");
    // system app for RegisterImeEventListener and currentIme for PanelStatusChangeToImc
    IdentityCheckerMock::SetSystemApp(true);
    IdentityCheckerMock::SetBundleNameValid(true);
    auto listener = std::make_shared<InputMethodSettingListenerImpl>();
    ImeEventMonitorManager::GetInstance().RegisterImeEventListener(EVENT_IME_SHOW_MASK | EVENT_IME_HIDE_MASK, listener);
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    panelInfo.panelFlag = FLG_FIXED;
    std::shared_ptr<InputMethodPanel> panel = nullptr;
    ImaCreatePanel(panelInfo, panel);
    // imeShow
    InputMethodPanelTest::ImcPanelListeningTestRestore();
    InputMethodPanelTest::TestShowPanel(panel);
    InputMethodPanelTest::TriggerPanelStatusChangeToImc(panel, InputWindowStatus::SHOW);
    InputMethodPanelTest::ImcPanelShowNumCheck(1);
    // imeHide
    InputMethodPanelTest::ImcPanelListeningTestRestore();
    InputMethodPanelTest::TestHidePanel(panel);
    InputMethodPanelTest::TriggerPanelStatusChangeToImc(panel, InputWindowStatus::HIDE);
    InputMethodPanelTest::ImcPanelHideNumCheck(1);
    ImaDestroyPanel(panel);
    ImeEventMonitorManager::GetInstance().UnRegisterImeEventListener(
        EVENT_IME_SHOW_MASK | EVENT_IME_HIDE_MASK, listener);
    IdentityCheckerMock::SetSystemApp(false);
    IdentityCheckerMock::SetBundleNameValid(false);
}

/*
 * @tc.name: testImcPanelListening_002
 * @tc.desc: SOFT_KEYBOARD/FLG_FLOATING, listener(system app)
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelTest, testImcPanelListening_002, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelTest::testImcPanelListening_002 start.");
    // system app for RegisterImeEventListener and currentIme for PanelStatusChangeToImc
    IdentityCheckerMock::SetSystemApp(true);
    IdentityCheckerMock::SetBundleNameValid(true);
    auto listener = std::make_shared<InputMethodSettingListenerImpl>();
    ImeEventMonitorManager::GetInstance().RegisterImeEventListener(EVENT_IME_SHOW_MASK | EVENT_IME_HIDE_MASK, listener);

    AccessScope scope(currentImeTokenId_, currentImeUid_);
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    panelInfo.panelFlag = FLG_FLOATING;
    std::shared_ptr<InputMethodPanel> panel = nullptr;
    ImaCreatePanel(panelInfo, panel);
    // imeShow
    InputMethodPanelTest::ImcPanelListeningTestRestore();
    InputMethodPanelTest::TestShowPanel(panel);
    InputMethodPanelTest::TriggerPanelStatusChangeToImc(panel, InputWindowStatus::SHOW);
    InputMethodPanelTest::ImcPanelShowNumCheck(1);
    // imeHide
    InputMethodPanelTest::ImcPanelListeningTestRestore();
    InputMethodPanelTest::TestHidePanel(panel);
    InputMethodPanelTest::TriggerPanelStatusChangeToImc(panel, InputWindowStatus::HIDE);
    InputMethodPanelTest::ImcPanelHideNumCheck(1);
    ImaDestroyPanel(panel);

    ImeEventMonitorManager::GetInstance().UnRegisterImeEventListener(
        EVENT_IME_SHOW_MASK | EVENT_IME_HIDE_MASK, listener);
    IdentityCheckerMock::SetSystemApp(false);
    IdentityCheckerMock::SetBundleNameValid(false);
}

/*
 * @tc.name: testImcPanelListening_003
 * @tc.desc: SOFT_KEYBOARD/FLG_CANDIDATE_COLUMN, listener(system app)
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelTest, testImcPanelListening_003, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelTest::testImcPanelListening_003 start.");
    // system app for RegisterImeEventListener and currentIme for PanelStatusChangeToImc
    IdentityCheckerMock::SetSystemApp(true);
    IdentityCheckerMock::SetBundleNameValid(true);
    auto listener = std::make_shared<InputMethodSettingListenerImpl>();

    ImeEventMonitorManager::GetInstance().RegisterImeEventListener(EVENT_IME_SHOW_MASK | EVENT_IME_HIDE_MASK, listener);

    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    panelInfo.panelFlag = FLG_CANDIDATE_COLUMN;
    std::shared_ptr<InputMethodPanel> panel = nullptr;
    ImaCreatePanel(panelInfo, panel);
    // imeShow
    InputMethodPanelTest::ImcPanelListeningTestRestore();
    InputMethodPanelTest::TestShowPanel(panel);
    InputMethodPanelTest::TriggerPanelStatusChangeToImc(panel, InputWindowStatus::SHOW);
    InputMethodPanelTest::ImcPanelShowNumCheck(0);
    // imeHide
    InputMethodPanelTest::ImcPanelListeningTestRestore();
    InputMethodPanelTest::TestHidePanel(panel);
    InputMethodPanelTest::TriggerPanelStatusChangeToImc(panel, InputWindowStatus::HIDE);
    InputMethodPanelTest::ImcPanelHideNumCheck(0);
    ImaDestroyPanel(panel);

    ImeEventMonitorManager::GetInstance().UnRegisterImeEventListener(
        EVENT_IME_SHOW_MASK | EVENT_IME_HIDE_MASK, listener);
    IdentityCheckerMock::SetSystemApp(false);
    IdentityCheckerMock::SetBundleNameValid(false);
}

/**
 * @tc.name: testImcPanelListening_004
 * @tc.desc: STATUS_BAR, listener(system app)
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelTest, testImcPanelListening_004, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelTest::testImcPanelListening_004 start.");
    // system app for RegisterImeEventListener and currentIme for PanelStatusChangeToImc
    IdentityCheckerMock::SetSystemApp(true);
    IdentityCheckerMock::SetBundleNameValid(true);
    auto listener = std::make_shared<InputMethodSettingListenerImpl>();

    ImeEventMonitorManager::GetInstance().RegisterImeEventListener(EVENT_IME_SHOW_MASK | EVENT_IME_HIDE_MASK, listener);

    PanelInfo panelInfo;
    panelInfo.panelType = STATUS_BAR;
    std::shared_ptr<InputMethodPanel> panel = nullptr;
    ImaCreatePanel(panelInfo, panel);
    // imeShow
    InputMethodPanelTest::ImcPanelListeningTestRestore();
    InputMethodPanelTest::TestShowPanel(panel);
    InputMethodPanelTest::ImcPanelShowNumCheck(0);
    // imeHide
    InputMethodPanelTest::ImcPanelListeningTestRestore();
    InputMethodPanelTest::TestHidePanel(panel);
    InputMethodPanelTest::ImcPanelHideNumCheck(0);
    ImaDestroyPanel(panel);

    ImeEventMonitorManager::GetInstance().UnRegisterImeEventListener(
        EVENT_IME_SHOW_MASK | EVENT_IME_HIDE_MASK, listener);
    IdentityCheckerMock::SetSystemApp(false);
    IdentityCheckerMock::SetBundleNameValid(false);
}

/*
 * @tc.name: testPanelStatusChangeEventPublicTest
 * @tc.desc: test subscriber can receive the panel status change event published by IMSA
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelTest, testPanelStatusChangeEventPublicTest, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelTest::testPanelStatusChangeEventPublicTest start.");
    // currentIme for PanelStatusChangeToImc
    IdentityCheckerMock::SetBundleNameValid(true);
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(COMMON_EVENT_INPUT_PANEL_STATUS_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto subscriber = std::make_shared<TestEventSubscriber>(subscriberInfo);
    auto ret = EventFwk::CommonEventManager::SubscribeCommonEvent(subscriber);
    EXPECT_TRUE(ret);
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    panelInfo.panelFlag = FLG_FLOATING;
    std::shared_ptr<InputMethodPanel> panel = nullptr;
    ImaCreatePanel(panelInfo, panel);
    // imeShow
    subscriber->ResetParam();
    InputMethodPanelTest::TestShowPanel(panel);
    InputMethodPanelTest::TriggerPanelStatusChangeToImc(panel, InputWindowStatus::SHOW);
    {
        std::unique_lock<std::mutex> lock(imcPanelStatusListenerLock_);
        auto waitRet = imcPanelStatusListenerCv_.wait_for(
            lock, std::chrono::milliseconds(IMC_WAIT_PANEL_STATUS_LISTEN_TIME), [subscriber]() {
                return subscriber->action_ == COMMON_EVENT_INPUT_PANEL_STATUS_CHANGED &&
                    subscriber->status_ == InputWindowStatus::SHOW;
            });
        EXPECT_TRUE(waitRet);
    }
    // imeHide
    subscriber->ResetParam();
    InputMethodPanelTest::TestHidePanel(panel);
    InputMethodPanelTest::TriggerPanelStatusChangeToImc(panel, InputWindowStatus::HIDE);
    {
        std::unique_lock<std::mutex> lock(imcPanelStatusListenerLock_);
        auto waitRet = imcPanelStatusListenerCv_.wait_for(
            lock, std::chrono::milliseconds(IMC_WAIT_PANEL_STATUS_LISTEN_TIME), [subscriber]() {
                return subscriber->action_ == COMMON_EVENT_INPUT_PANEL_STATUS_CHANGED &&
                    subscriber->status_ == InputWindowStatus::HIDE;
            });
        EXPECT_TRUE(waitRet);
    }
    ImaDestroyPanel(panel);
    IdentityCheckerMock::SetBundleNameValid(false);
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

    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    panelInfo.panelFlag = FLG_FIXED;
    ret = inputMethodPanel->CreatePanel(nullptr, panelInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ret = inputMethodPanel->SetCallingWindow(windowId);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ret = inputMethodPanel->DestroyPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/*
 * @tc.name: testKeyboardPanelInfoChangeListenerRegister_001
 * @tc.desc: SOFT_KEYBOARD
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelTest, testKeyboardPanelInfoChangeListenerRegister_001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelTest::testKeyboardPanelInfoChangeListenerRegister_001 start.");
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    std::shared_ptr<InputMethodPanel> panel = nullptr;
    ImaCreatePanel(panelInfo, panel);
    ASSERT_NE(panel, nullptr);
    if (isScbEnable_) {
        EXPECT_NE(panel->kbPanelInfoListener_, nullptr);
    } else {
        EXPECT_EQ(panel->kbPanelInfoListener_, nullptr);
    }
    ImaDestroyPanel(panel);
    EXPECT_EQ(panel->kbPanelInfoListener_, nullptr);
}

/*
 * @tc.name: testKeyboardPanelInfoChangeListenerRegister_002
 * @tc.desc: STATUS_BAR
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelTest, testKeyboardPanelInfoChangeListenerRegister_002, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelTest::testKeyboardPanelInfoChangeListenerRegister_002 start.");
    PanelInfo panelInfo;
    panelInfo.panelType = STATUS_BAR;
    std::shared_ptr<InputMethodPanel> panel = nullptr;
    ImaCreatePanel(panelInfo, panel);
    ASSERT_NE(panel, nullptr);
    EXPECT_EQ(panel->kbPanelInfoListener_, nullptr);
    ImaDestroyPanel(panel);
}

/**
 * @tc.name: testAdjustPanelRect_001
 * @tc.desc: Test AdjustPanelRect with FLG_FIXED invalid params.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelTest, testAdjustPanelRect_001, TestSize.Level0)
{
    InputMethodPanelTest::Attach();
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    panelInfo.panelFlag = FLG_FIXED;
    InputMethodPanelTest::ImaCreatePanel(panelInfo, inputMethodPanel);
    PanelFlag panelFlag = PanelFlag::FLG_FIXED;
    LayoutParams layoutParams;
    layoutParams.landscapeRect = { 0, 0, 0, 0 };
    layoutParams.portraitRect = { 0, 0, 0, 0 };
    auto ret = inputMethodPanel->AdjustPanelRect(panelFlag, layoutParams);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodPanelTest::ImaDestroyPanel(inputMethodPanel);
    InputMethodPanelTest::imc_->Close();
    TddUtil::DestroyWindow();
}

/**
 * @tc.name: testAdjustPanelRect_002
 * @tc.desc: Test AdjustPanelRect with FLG_FIXED invalid params.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelTest, testAdjustPanelRect_002, TestSize.Level0)
{
    InputMethodPanelTest::Attach();
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    panelInfo.panelFlag = FLG_FIXED;
    InputMethodPanelTest::ImaCreatePanel(panelInfo, inputMethodPanel);
    PanelFlag panelFlag = PanelFlag::FLG_FIXED;
    LayoutParams layoutParams;
    layoutParams.landscapeRect = { -1, 0, 0, 0 };
    layoutParams.portraitRect = { -1, 0, 0, 0 };
    auto ret = inputMethodPanel->AdjustPanelRect(panelFlag, layoutParams);
    EXPECT_EQ(ret, ErrorCode::ERROR_PARAMETER_CHECK_FAILED);
    InputMethodPanelTest::ImaDestroyPanel(inputMethodPanel);
    InputMethodPanelTest::imc_->Close();
    TddUtil::DestroyWindow();
}

/**
 * @tc.name: testAdjustPanelRect_003
 * @tc.desc: Test AdjustPanelRect with FLG_FIXED invalid params.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelTest, testAdjustPanelRect_003, TestSize.Level0)
{
    InputMethodPanelTest::Attach();
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    panelInfo.panelFlag = FLG_FIXED;
    InputMethodPanelTest::ImaCreatePanel(panelInfo, inputMethodPanel);
    PanelFlag panelFlag = PanelFlag::FLG_FIXED;
    LayoutParams layoutParams;
    layoutParams.landscapeRect = { 0, -1, 0, 0 };
    layoutParams.portraitRect = { 0, -1, 0, 0 };
    auto ret = inputMethodPanel->AdjustPanelRect(panelFlag, layoutParams);
    EXPECT_EQ(ret, ErrorCode::ERROR_PARAMETER_CHECK_FAILED);
    InputMethodPanelTest::ImaDestroyPanel(inputMethodPanel);
    InputMethodPanelTest::imc_->Close();
    TddUtil::DestroyWindow();
}

/**
 * @tc.name: testAdjustPanelRect_004
 * @tc.desc: Test AdjustPanelRect with FLG_FIXED invalid params.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelTest, testAdjustPanelRect_004, TestSize.Level0)
{
    InputMethodPanelTest::Attach();
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    panelInfo.panelFlag = FLG_FIXED;
    InputMethodPanelTest::ImaCreatePanel(panelInfo, inputMethodPanel);
    PanelFlag panelFlag = PanelFlag::FLG_FIXED;
    LayoutParams layoutParams;
    layoutParams.landscapeRect = { 0, 0, -1, 0 };
    layoutParams.portraitRect = { 0, 0, -1, 0 };
    auto ret = inputMethodPanel->AdjustPanelRect(panelFlag, layoutParams);
    EXPECT_EQ(ret, ErrorCode::ERROR_PARAMETER_CHECK_FAILED);
    InputMethodPanelTest::ImaDestroyPanel(inputMethodPanel);
    InputMethodPanelTest::imc_->Close();
    TddUtil::DestroyWindow();
}

/**
 * @tc.name: testAdjustPanelRect_005
 * @tc.desc: Test AdjustPanelRect with FLG_FIXED invalid params.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelTest, testAdjustPanelRect_005, TestSize.Level0)
{
    InputMethodPanelTest::Attach();
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    panelInfo.panelFlag = FLG_FIXED;
    InputMethodPanelTest::ImaCreatePanel(panelInfo, inputMethodPanel);
    PanelFlag panelFlag = PanelFlag::FLG_FIXED;
    LayoutParams layoutParams;
    layoutParams.landscapeRect = { 0, 0, 0, -1 };
    layoutParams.portraitRect = { 0, 0, 0, -1 };
    auto ret = inputMethodPanel->AdjustPanelRect(panelFlag, layoutParams);
    EXPECT_EQ(ret, ErrorCode::ERROR_PARAMETER_CHECK_FAILED);
    InputMethodPanelTest::ImaDestroyPanel(inputMethodPanel);
    InputMethodPanelTest::imc_->Close();
    TddUtil::DestroyWindow();
}

/**
 * @tc.name: testAdjustPanelRect_006
 * @tc.desc: Test AdjustPanelRect with FLG_FIXED valid params.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelTest, testAdjustPanelRect_006, TestSize.Level0)
{
    InputMethodPanelTest::Attach();
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    panelInfo.panelFlag = FLG_FIXED;
    InputMethodPanelTest::ImaCreatePanel(panelInfo, inputMethodPanel);
    auto defaultDisplay = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    ASSERT_TRUE(defaultDisplay != nullptr);
    windowWidth_ = defaultDisplay->GetWidth();
    windowHeight_ = defaultDisplay->GetHeight();
    PanelFlag panelFlag = PanelFlag::FLG_FIXED;
    LayoutParams layoutParams;
    layoutParams.landscapeRect = { 0, 0, windowHeight_, 0 };
    layoutParams.portraitRect = { 0, 0, windowWidth_, 0 };
    auto ret = inputMethodPanel->AdjustPanelRect(panelFlag, layoutParams);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodPanelTest::ImaDestroyPanel(inputMethodPanel);
    InputMethodPanelTest::imc_->Close();
    TddUtil::DestroyWindow();
}

/**
 * @tc.name: testAdjustPanelRect_007
 * @tc.desc: Test AdjustPanelRect with FLG_FIXED invalid params.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelTest, testAdjustPanelRect_007, TestSize.Level0)
{
    InputMethodPanelTest::Attach();
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    panelInfo.panelFlag = FLG_FIXED;
    InputMethodPanelTest::ImaCreatePanel(panelInfo, inputMethodPanel);
    auto defaultDisplay = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    ASSERT_TRUE(defaultDisplay != nullptr);
    windowWidth_ = defaultDisplay->GetWidth();
    windowHeight_ = defaultDisplay->GetHeight();
    PanelFlag panelFlag = PanelFlag::FLG_FIXED;
    LayoutParams layoutParams;
    layoutParams.landscapeRect = { 0, 0, windowHeight_ + 1, 0 };
    layoutParams.portraitRect = { 0, 0, windowWidth_ + 1, 0 };
    auto ret = inputMethodPanel->AdjustPanelRect(panelFlag, layoutParams);
    EXPECT_EQ(ret, ErrorCode::ERROR_PARAMETER_CHECK_FAILED);
    InputMethodPanelTest::ImaDestroyPanel(inputMethodPanel);
    InputMethodPanelTest::imc_->Close();
    TddUtil::DestroyWindow();
}

/**
 * @tc.name: testAdjustPanelRect_008
 * @tc.desc: Test AdjustPanelRect with FLG_FIXED invalid params.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelTest, testAdjustPanelRect_008, TestSize.Level0)
{
    InputMethodPanelTest::Attach();
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    panelInfo.panelFlag = FLG_FIXED;
    InputMethodPanelTest::ImaCreatePanel(panelInfo, inputMethodPanel);
    auto defaultDisplay = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    ASSERT_TRUE(defaultDisplay != nullptr);
    windowWidth_ = defaultDisplay->GetWidth();
    windowHeight_ = defaultDisplay->GetHeight();
    PanelFlag panelFlag = PanelFlag::FLG_FIXED;
    LayoutParams layoutParams;
    layoutParams.landscapeRect = { 0, 0, windowHeight_, windowWidth_ * 0.7 + 1 };
    layoutParams.portraitRect = { 0, 0, windowWidth_, windowHeight_ * 0.7 + 1 };
    auto ret = inputMethodPanel->AdjustPanelRect(panelFlag, layoutParams);
    EXPECT_EQ(ret, ErrorCode::ERROR_PARAMETER_CHECK_FAILED);
    InputMethodPanelTest::ImaDestroyPanel(inputMethodPanel);
    InputMethodPanelTest::imc_->Close();
    TddUtil::DestroyWindow();
}

/**
 * @tc.name: testAdjustPanelRect_009
 * @tc.desc: Test AdjustPanelRect with FLG_FIXED valid params.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelTest, testAdjustPanelRect_009, TestSize.Level0)
{
    InputMethodPanelTest::Attach();
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    panelInfo.panelFlag = FLG_FIXED;
    InputMethodPanelTest::ImaCreatePanel(panelInfo, inputMethodPanel);
    auto defaultDisplay = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    ASSERT_TRUE(defaultDisplay != nullptr);
    windowWidth_ = defaultDisplay->GetWidth();
    windowHeight_ = defaultDisplay->GetHeight();
    PanelFlag panelFlag = PanelFlag::FLG_FIXED;
    LayoutParams layoutParams;
    layoutParams.landscapeRect = { 0, 0, windowHeight_, windowWidth_ * 0.7 };
    layoutParams.portraitRect = { 0, 0, windowWidth_, windowHeight_ * 0.7 };
    auto ret = inputMethodPanel->AdjustPanelRect(panelFlag, layoutParams);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodPanelTest::ImaDestroyPanel(inputMethodPanel);
    InputMethodPanelTest::imc_->Close();
    TddUtil::DestroyWindow();
}

/**
 * @tc.name: testAdjustPanelRect_010
 * @tc.desc: Test AdjustPanelRect with FLG_FLOATING valid params.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelTest, testAdjustPanelRect_010, TestSize.Level0)
{
    InputMethodPanelTest::Attach();
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    panelInfo.panelFlag = FLG_FLOATING;
    InputMethodPanelTest::ImaCreatePanel(panelInfo, inputMethodPanel);
    PanelFlag panelFlag = PanelFlag::FLG_FLOATING;
    LayoutParams layoutParams;
    layoutParams.landscapeRect = { 0, 0, 0, 0 };
    layoutParams.portraitRect = { 0, 0, 0, 0 };
    auto ret = inputMethodPanel->AdjustPanelRect(panelFlag, layoutParams);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodPanelTest::ImaDestroyPanel(inputMethodPanel);
    InputMethodPanelTest::imc_->Close();
    TddUtil::DestroyWindow();
}

/**
 * @tc.name: testAdjustPanelRect_011
 * @tc.desc: Test AdjustPanelRect with FLG_FLOATING invalid params.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelTest, testAdjustPanelRect_011, TestSize.Level0)
{
    InputMethodPanelTest::Attach();
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    panelInfo.panelFlag = FLG_FLOATING;
    InputMethodPanelTest::ImaCreatePanel(panelInfo, inputMethodPanel);
    PanelFlag panelFlag = PanelFlag::FLG_FLOATING;
    LayoutParams layoutParams;
    layoutParams.landscapeRect = { -1, 0, 0, 0 };
    layoutParams.portraitRect = { -1, 0, 0, 0 };
    auto ret = inputMethodPanel->AdjustPanelRect(panelFlag, layoutParams);
    EXPECT_EQ(ret, ErrorCode::ERROR_PARAMETER_CHECK_FAILED);
    InputMethodPanelTest::ImaDestroyPanel(inputMethodPanel);
    InputMethodPanelTest::imc_->Close();
    TddUtil::DestroyWindow();
}

/**
 * @tc.name: testAdjustPanelRect_012
 * @tc.desc: Test AdjustPanelRect with FLG_FLOATING invalid params.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelTest, testAdjustPanelRect_012, TestSize.Level0)
{
    InputMethodPanelTest::Attach();
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    panelInfo.panelFlag = FLG_FLOATING;
    InputMethodPanelTest::ImaCreatePanel(panelInfo, inputMethodPanel);
    PanelFlag panelFlag = PanelFlag::FLG_FLOATING;
    LayoutParams layoutParams;
    layoutParams.landscapeRect = { 0, -1, 0, 0 };
    layoutParams.portraitRect = { 0, -1, 0, 0 };
    auto ret = inputMethodPanel->AdjustPanelRect(panelFlag, layoutParams);
    EXPECT_EQ(ret, ErrorCode::ERROR_PARAMETER_CHECK_FAILED);
    InputMethodPanelTest::ImaDestroyPanel(inputMethodPanel);
    InputMethodPanelTest::imc_->Close();
    TddUtil::DestroyWindow();
}

/**
 * @tc.name: testAdjustPanelRect_013
 * @tc.desc: Test AdjustPanelRect with FLG_FLOATING invalid params.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelTest, testAdjustPanelRect_013, TestSize.Level0)
{
    InputMethodPanelTest::Attach();
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    panelInfo.panelFlag = FLG_FLOATING;
    InputMethodPanelTest::ImaCreatePanel(panelInfo, inputMethodPanel);
    PanelFlag panelFlag = PanelFlag::FLG_FLOATING;
    LayoutParams layoutParams;
    layoutParams.landscapeRect = { 0, 0, -1, 0 };
    layoutParams.portraitRect = { 0, 0, -1, 0 };
    auto ret = inputMethodPanel->AdjustPanelRect(panelFlag, layoutParams);
    EXPECT_EQ(ret, ErrorCode::ERROR_PARAMETER_CHECK_FAILED);
    InputMethodPanelTest::ImaDestroyPanel(inputMethodPanel);
    InputMethodPanelTest::imc_->Close();
    TddUtil::DestroyWindow();
}

/**
 * @tc.name: testAdjustPanelRect_014
 * @tc.desc: Test AdjustPanelRect with FLG_FLOATING valid params.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelTest, testAdjustPanelRect_014, TestSize.Level0)
{
    InputMethodPanelTest::Attach();
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    panelInfo.panelFlag = FLG_FLOATING;
    InputMethodPanelTest::ImaCreatePanel(panelInfo, inputMethodPanel);
    auto defaultDisplay = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    ASSERT_TRUE(defaultDisplay != nullptr);
    windowWidth_ = defaultDisplay->GetWidth();
    windowHeight_ = defaultDisplay->GetHeight();
    PanelFlag panelFlag = PanelFlag::FLG_FLOATING;
    LayoutParams layoutParams;
    layoutParams.landscapeRect = { 0, 0, windowWidth_, 0 };
    layoutParams.portraitRect = { 0, 0, windowWidth_, 0 };
    auto ret = inputMethodPanel->AdjustPanelRect(panelFlag, layoutParams);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodPanelTest::ImaDestroyPanel(inputMethodPanel);
    InputMethodPanelTest::imc_->Close();
    TddUtil::DestroyWindow();
}

/**
 * @tc.name: testAdjustPanelRect_015
 * @tc.desc: Test AdjustPanelRect with FLG_FLOATING invalid params.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelTest, testAdjustPanelRect_015, TestSize.Level0)
{
    InputMethodPanelTest::Attach();
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    panelInfo.panelFlag = FLG_FLOATING;
    InputMethodPanelTest::ImaCreatePanel(panelInfo, inputMethodPanel);
    auto defaultDisplay = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    ASSERT_TRUE(defaultDisplay != nullptr);
    windowWidth_ = defaultDisplay->GetWidth();
    windowHeight_ = defaultDisplay->GetHeight();
    PanelFlag panelFlag = PanelFlag::FLG_FLOATING;
    LayoutParams layoutParams;
    layoutParams.landscapeRect = { 0, 0, windowHeight_ + 1, 0 };
    layoutParams.portraitRect = { 0, 0, windowWidth_ + 1, 0 };
    auto ret = inputMethodPanel->AdjustPanelRect(panelFlag, layoutParams);
    EXPECT_EQ(ret, ErrorCode::ERROR_PARAMETER_CHECK_FAILED);
    InputMethodPanelTest::ImaDestroyPanel(inputMethodPanel);
    InputMethodPanelTest::imc_->Close();
    TddUtil::DestroyWindow();
}

/**
 * @tc.name: testAdjustPanelRect_016
 * @tc.desc: Test AdjustPanelRect with FLG_FLOATING valid params.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelTest, testAdjustPanelRect_016, TestSize.Level0)
{
    InputMethodPanelTest::Attach();
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    panelInfo.panelFlag = FLG_FLOATING;
    InputMethodPanelTest::ImaCreatePanel(panelInfo, inputMethodPanel);
    auto defaultDisplay = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    ASSERT_TRUE(defaultDisplay != nullptr);
    windowWidth_ = defaultDisplay->GetWidth();
    windowHeight_ = defaultDisplay->GetHeight();
    PanelFlag panelFlag = PanelFlag::FLG_FLOATING;
    LayoutParams layoutParams;
    layoutParams.landscapeRect = { 0, 0, windowHeight_, windowWidth_ * 0.7 + 1 };
    layoutParams.portraitRect = { 0, 0, windowWidth_, windowHeight_ * 0.7 + 1 };
    auto ret = inputMethodPanel->AdjustPanelRect(panelFlag, layoutParams);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodPanelTest::ImaDestroyPanel(inputMethodPanel);
    InputMethodPanelTest::imc_->Close();
    TddUtil::DestroyWindow();
}

/**
 * @tc.name: testAdjustPanelRect_017
 * @tc.desc: Test AdjustPanelRect with FLG_FLOATING valid params.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelTest, testAdjustPanelRect_017, TestSize.Level0)
{
    InputMethodPanelTest::Attach();
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    panelInfo.panelFlag = FLG_FLOATING;
    InputMethodPanelTest::ImaCreatePanel(panelInfo, inputMethodPanel);
    auto defaultDisplay = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    ASSERT_TRUE(defaultDisplay != nullptr);
    windowWidth_ = defaultDisplay->GetWidth();
    windowHeight_ = defaultDisplay->GetHeight();
    PanelFlag panelFlag = PanelFlag::FLG_FLOATING;
    LayoutParams layoutParams;
    layoutParams.landscapeRect = { 0, 0, windowHeight_, windowWidth_ };
    layoutParams.portraitRect = { 0, 0, windowWidth_, windowHeight_ };
    auto ret = inputMethodPanel->AdjustPanelRect(panelFlag, layoutParams);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodPanelTest::ImaDestroyPanel(inputMethodPanel);
    InputMethodPanelTest::imc_->Close();
    TddUtil::DestroyWindow();
}

/**
 * @tc.name: testAdjustPanelRect_018
 * @tc.desc: Test AdjustPanelRect with FLG_FLOATING invalid params.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelTest, testAdjustPanelRect_018, TestSize.Level0)
{
    InputMethodPanelTest::Attach();
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    panelInfo.panelFlag = FLG_FLOATING;
    InputMethodPanelTest::ImaCreatePanel(panelInfo, inputMethodPanel);
    auto defaultDisplay = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    ASSERT_TRUE(defaultDisplay != nullptr);
    windowWidth_ = defaultDisplay->GetWidth();
    windowHeight_ = defaultDisplay->GetHeight();
    PanelFlag panelFlag = PanelFlag::FLG_FLOATING;
    LayoutParams layoutParams;
    layoutParams.landscapeRect = { 0, 0, windowHeight_, windowWidth_ + 1 };
    layoutParams.portraitRect = { 0, 0, windowWidth_, windowHeight_ + 1 };
    auto ret = inputMethodPanel->AdjustPanelRect(panelFlag, layoutParams);
    EXPECT_EQ(ret, ErrorCode::ERROR_PARAMETER_CHECK_FAILED);
    InputMethodPanelTest::ImaDestroyPanel(inputMethodPanel);
    InputMethodPanelTest::imc_->Close();
    TddUtil::DestroyWindow();
}

/**
 * @tc.name: testAdjustKeyboard_001
 * @tc.desc: Test AdjustKeyboard
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelTest, testAdjustKeyboard_001, TestSize.Level0)
{
    InputMethodPanelTest::Attach();
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    panelInfo.panelFlag = FLG_FLOATING;
    InputMethodPanelTest::ImaCreatePanel(panelInfo, inputMethodPanel);
    auto ret = inputMethodPanel->AdjustKeyboard();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodPanelTest::ImaDestroyPanel(inputMethodPanel);
    InputMethodPanelTest::imc_->Close();
    TddUtil::DestroyWindow();
}

/**
 * @tc.name: testSetPrivacyMode
 * @tc.desc: Test SetPrivacyMode.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelTest, testSetPrivacyMode, TestSize.Level0)
{
    InputMethodPanelTest::Attach();
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    panelInfo.panelFlag = FLG_FLOATING;
    InputMethodPanelTest::ImaCreatePanel(panelInfo, inputMethodPanel);
    auto ret = inputMethodPanel->SetPrivacyMode(true);
    EXPECT_NE(ret, ErrorCode::NO_ERROR);
    InputMethodPanelTest::ImaDestroyPanel(inputMethodPanel);
    InputMethodPanelTest::imc_->Close();
    TddUtil::DestroyWindow();
}

/**
 * @tc.name: testSetPanelProperties
 * @tc.desc: Test SetPanelProperties.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelTest, testSetPanelProperties, TestSize.Level0)
{
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    auto ret = inputMethodPanel->SetPanelProperties();
    EXPECT_EQ(ret, ErrorCode::ERROR_OPERATE_PANEL);
    inputMethodPanel->UnregisterKeyboardPanelInfoChangeListener();
    ret = inputMethodPanel->SetPrivacyMode(false);
    EXPECT_EQ(ret, ErrorCode::ERROR_NULL_POINTER);
}

/**
 * @tc.name: testGetKeyboardSize
 * @tc.desc: Test GetKeyboardSize.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelTest, testGetKeyboardSize, TestSize.Level0)
{
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    auto boardSize = inputMethodPanel->GetKeyboardSize();
    EXPECT_EQ(boardSize.width, 0);
    EXPECT_EQ(boardSize.height, 0);
}

/**
 * @tc.name: testMarkListener
 * @tc.desc: Test MarkListener.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelTest, testMarkListener, TestSize.Level0)
{
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    auto statusListener = std::make_shared<InputMethodPanelTest::PanelStatusListenerImpl>();
    auto ret = inputMethodPanel->SetPanelStatusListener(statusListener, "text");
    EXPECT_FALSE(ret);
    inputMethodPanel->ClearPanelListener("text");
    ret = inputMethodPanel->MarkListener("contenInfo", true);
    EXPECT_FALSE(ret);
    ret = inputMethodPanel->MarkListener("sizeChange", true);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: testSizeChange
 * @tc.desc: Test SizeChange.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelTest, testSizeChange, TestSize.Level0)
{
    AccessScope scope(currentImeTokenId_, currentImeUid_);
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    panelInfo.panelFlag = FLG_FLOATING;
    auto statusListener = std::make_shared<InputMethodPanelTest::PanelStatusListenerImpl>();
    auto ret = inputMethodPanel->CreatePanel(nullptr, panelInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    inputMethodPanel->panelStatusListener_ = statusListener;
    WindowSize windowSize;
    ret = inputMethodPanel->SizeChange(windowSize);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = inputMethodPanel->DestroyPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: testSetTextFieldAvoidInfo01
 * @tc.desc: Test SetTextFieldAvoidInfo.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelTest, testSetTextFieldAvoidInfo01, TestSize.Level0)
{
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    auto ret = inputMethodPanel->SetTextFieldAvoidInfo(0, 0);
    EXPECT_EQ(ret, ErrorCode::ERROR_NULL_POINTER);
}

/**
 * @tc.name: testSetTextFieldAvoidInfo02
 * @tc.desc: Test SetTextFieldAvoidInfo.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelTest, testSetTextFieldAvoidInfo02, TestSize.Level0)
{
    InputMethodPanelTest::Attach();
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    panelInfo.panelFlag = FLG_FLOATING;
    InputMethodPanelTest::ImaCreatePanel(panelInfo, inputMethodPanel);
    auto ret = inputMethodPanel->SetTextFieldAvoidInfo(0, 0);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodPanelTest::ImaDestroyPanel(inputMethodPanel);
    InputMethodPanelTest::imc_->Close();
    TddUtil::DestroyWindow();
}

/**
 * @tc.name: testGetCallingWindowInfo01
 * @tc.desc: Test GetCallingWindowInfo.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelTest, testGetCallingWindowInfo01, TestSize.Level0)
{
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    CallingWindowInfo windowInfo;
    auto ret = inputMethodPanel->GetCallingWindowInfo(windowInfo);
    EXPECT_EQ(ret, ErrorCode::ERROR_PANEL_NOT_FOUND);
}

/**
 * @tc.name: testGetCallingWindowInfo02
 * @tc.desc: Test GetCallingWindowInfo.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelTest, testGetCallingWindowInfo02, TestSize.Level0)
{
    InputMethodPanelTest::Attach();
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    panelInfo.panelFlag = FLG_FLOATING;
    InputMethodPanelTest::ImaCreatePanel(panelInfo, inputMethodPanel);
    CallingWindowInfo windowInfo;
    auto ret = inputMethodPanel->GetCallingWindowInfo(windowInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodPanelTest::ImaDestroyPanel(inputMethodPanel);
    InputMethodPanelTest::imc_->Close();
    TddUtil::DestroyWindow();
}

/**
 * @tc.name: testSetUiContent01
 * @tc.desc: Test SetUiContent.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelTest, testSetUiContent01, TestSize.Level0)
{
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    auto ret = inputMethodPanel->SetUiContent("text", nullptr, nullptr);
    EXPECT_EQ(ret, ErrorCode::ERROR_NULL_POINTER);
}

/**
 * @tc.name: testSetUiContent02
 * @tc.desc: Test SetUiContent.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelTest, testSetUiContent02, TestSize.Level0)
{
    InputMethodPanelTest::Attach();
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    panelInfo.panelFlag = FLG_FLOATING;
    InputMethodPanelTest::ImaCreatePanel(panelInfo, inputMethodPanel);
    auto ret = inputMethodPanel->SetUiContent("text", nullptr, nullptr);
    EXPECT_EQ(ret, ErrorCode::ERROR_PARAMETER_CHECK_FAILED);
    InputMethodPanelTest::ImaDestroyPanel(inputMethodPanel);
    InputMethodPanelTest::imc_->Close();
    TddUtil::DestroyWindow();
}

/**
 * @tc.name: testIsSizeValid
 * @tc.desc: Test IsSizeValid
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelTest, testIsSizeValid, TestSize.Level0)
{
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    EXPECT_FALSE(inputMethodPanel->IsSizeValid(INT32_MAX + 1, INT32_MAX));
    EXPECT_FALSE(inputMethodPanel->IsSizeValid(INT32_MAX, INT32_MAX + 1));
}

/**
 * @tc.name: testGenerateSequenceId
 * @tc.desc: Test GenerateSequenceId
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelTest, testGenerateSequenceId, TestSize.Level0)
{
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    inputMethodPanel->sequenceId_ = std::numeric_limits<uint32_t>::max() - 1;
    uint32_t seqId = inputMethodPanel->GenerateSequenceId();
    EXPECT_EQ(seqId, 0);
}

/**
 * @tc.name: testPanelStatusListener01
 * @tc.desc: Test SetPanelStatusListener
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelTest, testPanelStatusListener01, TestSize.Level0)
{
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    inputMethodPanel->panelStatusListener_ = nullptr;
    inputMethodPanel->ClearPanelListener("show");

    AccessScope scope(currentImeTokenId_, currentImeUid_);
    PanelInfo panelInfo;
    panelInfo.panelType = STATUS_BAR;
    panelInfo.panelFlag = FLG_FIXED;
    auto statusListener = std::make_shared<InputMethodPanelTest::PanelStatusListenerImpl>();
    auto ret = inputMethodPanel->CreatePanel(nullptr, panelInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    inputMethodPanel->panelStatusListener_ = statusListener;
    EXPECT_TRUE(inputMethodPanel->SetPanelStatusListener(statusListener, "sizeChange"));
    ret = inputMethodPanel->DestroyPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: testPanelStatusListener02
 * @tc.desc: Test SetPanelStatusListener
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelTest, testPanelStatusListener02, TestSize.Level0)
{
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    AccessScope scope(currentImeTokenId_, currentImeUid_);
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    panelInfo.panelFlag = FLG_FIXED;
    auto statusListener = std::make_shared<InputMethodPanelTest::PanelStatusListenerImpl>();
    auto ret = inputMethodPanel->CreatePanel(nullptr, panelInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    inputMethodPanel->panelStatusListener_ = statusListener;
    EXPECT_TRUE(inputMethodPanel->SetPanelStatusListener(statusListener, "sizeChange"));
    EXPECT_TRUE(inputMethodPanel->SetPanelStatusListener(nullptr, "sizeChange"));
    inputMethodPanel->panelStatusListener_ = nullptr;
    EXPECT_TRUE(inputMethodPanel->SetPanelStatusListener(nullptr, "sizeChange"));
    EXPECT_TRUE(inputMethodPanel->SetPanelStatusListener(statusListener, "sizeChange"));
    ret = inputMethodPanel->DestroyPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: testStartMoving01
 * @tc.desc: Test StartMoving
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelTest, testStartMoving01, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelTest::testStartMoving start.");
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    auto ret = inputMethodPanel->StartMoving();
    EXPECT_EQ(ret, ErrorCode::ERROR_IME);

    AccessScope scope(currentImeTokenId_, currentImeUid_);
    PanelInfo panelInfo;
    panelInfo.panelType = STATUS_BAR;
    panelInfo.panelFlag = FLG_FLOATING;
    ret = inputMethodPanel->CreatePanel(nullptr, panelInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = inputMethodPanel->StartMoving();
    EXPECT_GE(ret, ErrorCode::NO_ERROR);
    ret = inputMethodPanel->DestroyPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    panelInfo.panelType = SOFT_KEYBOARD;
    ret = inputMethodPanel->CreatePanel(nullptr, panelInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = inputMethodPanel->StartMoving();
    EXPECT_EQ(ret, ErrorCode::ERROR_INVALID_PANEL_TYPE);
    ret = inputMethodPanel->DestroyPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    panelInfo.panelType = STATUS_BAR;
    panelInfo.panelFlag = FLG_FIXED;
    ret = inputMethodPanel->CreatePanel(nullptr, panelInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = inputMethodPanel->StartMoving();
    EXPECT_EQ(ret, ErrorCode::ERROR_INVALID_PANEL_FLAG);
    ret = inputMethodPanel->DestroyPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: testGetDisplayId01
 * @tc.desc: Test GetDisplayId
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelTest, testGetDisplayId01, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelTest::testGetDisplayId start.");
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    uint64_t displayId;
    auto ret = inputMethodPanel->GetDisplayId(displayId);
    EXPECT_EQ(ret, ErrorCode::ERROR_IME);

    AccessScope scope(currentImeTokenId_, currentImeUid_);
    PanelInfo panelInfo;
    panelInfo.panelType = STATUS_BAR;
    panelInfo.panelFlag = FLG_FLOATING;
    ret = inputMethodPanel->CreatePanel(nullptr, panelInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ret = inputMethodPanel->GetDisplayId(displayId);
    EXPECT_GE(ret, ErrorCode::NO_ERROR);

    ret = inputMethodPanel->DestroyPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: testSetImmersiveMode
 * @tc.desc: Test set immersive mode.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelTest, testSetImmersiveMode, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelTest::testSetImmersiveMode start.");
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    panelInfo.panelFlag = FLG_FIXED;

    AccessScope scope(currentImeTokenId_, currentImeUid_);
    auto ret = inputMethodPanel->CreatePanel(nullptr, panelInfo);
    EXPECT_EQ(ErrorCode::NO_ERROR, ret);
    ret = inputMethodPanel->ShowPanel();
    EXPECT_EQ(ErrorCode::NO_ERROR, ret);

    ret = inputMethodPanel->SetImmersiveMode(ImmersiveMode::NONE_IMMERSIVE);
    EXPECT_EQ(ErrorCode::NO_ERROR, ret);
    ret = inputMethodPanel->SetImmersiveMode(ImmersiveMode::LIGHT_IMMERSIVE);
    EXPECT_EQ(ErrorCode::NO_ERROR, ret);
    ret = inputMethodPanel->SetImmersiveMode(ImmersiveMode::DARK_IMMERSIVE);
    EXPECT_EQ(ErrorCode::NO_ERROR, ret);
    ret = inputMethodPanel->SetImmersiveMode(ImmersiveMode::IMMERSIVE);
    EXPECT_EQ(ErrorCode::ERROR_PARAMETER_CHECK_FAILED, ret);

    ret = inputMethodPanel->HidePanel();
    EXPECT_EQ(ErrorCode::NO_ERROR, ret);
    ret = inputMethodPanel->DestroyPanel();
    EXPECT_EQ(ErrorCode::NO_ERROR, ret);
}

/**
 * @tc.name: testGetImmersiveMode
 * @tc.desc: Test get immersive mode.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelTest, testGetImmersiveMode, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelTest::testGetImmersiveMode start.");
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    panelInfo.panelFlag = FLG_FIXED;

    AccessScope scope(currentImeTokenId_, currentImeUid_);
    auto ret = inputMethodPanel->CreatePanel(nullptr, panelInfo);
    EXPECT_EQ(ErrorCode::NO_ERROR, ret);
    ret = inputMethodPanel->ShowPanel();
    EXPECT_EQ(ErrorCode::NO_ERROR, ret);

    ret = inputMethodPanel->SetImmersiveMode(ImmersiveMode::NONE_IMMERSIVE);
    EXPECT_EQ(ErrorCode::NO_ERROR, ret);
    auto mode = inputMethodPanel->GetImmersiveMode();
    EXPECT_EQ(ImmersiveMode::NONE_IMMERSIVE, mode);

    ret = inputMethodPanel->HidePanel();
    EXPECT_EQ(ErrorCode::NO_ERROR, ret);
    ret = inputMethodPanel->DestroyPanel();
    EXPECT_EQ(ErrorCode::NO_ERROR, ret);
}


/**
 * @tc.name: testParameterValidationInterface
 * @tc.desc: Test Parameter validation interface
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelTest, testParameterValidationInterface, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelTest::testParameterValidationInterface start.");
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    panelInfo.panelFlag = FLG_FIXED;

    AccessScope scope(currentImeTokenId_, currentImeUid_);
    auto ret = inputMethodPanel->CreatePanel(nullptr, panelInfo);
    EnhancedLayoutParams enhancedLayoutParams;
    FullPanelAdjustInfo adjustInfo;
    enhancedLayoutParams.portrait.avoidY = 10;
    enhancedLayoutParams.landscape.avoidY = 20;
    adjustInfo.portrait = {0, 0, 100, 100};
    adjustInfo.landscape = {0, 0, 200, 200};
    PanelAdjustInfo keyboardArea;

    ret = inputMethodPanel->GetKeyboardArea(PanelFlag::FLG_FIXED, {100, 200}, keyboardArea);

    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(keyboardArea.top, inputMethodPanel->enhancedLayoutParams_.portrait.avoidY);

    uint32_t validWidth = 100;
    uint32_t validHeight = 200;
    ret = inputMethodPanel->ResizeWithoutAdjust(validWidth, validHeight);
    EXPECT_EQ(ErrorCode::NO_ERROR, ret);

    ret = inputMethodPanel->ResizeWithoutAdjust(INT32_MAX + 1, INT32_MAX + 1);
    EXPECT_EQ(ErrorCode::ERROR_BAD_PARAMETERS, ret);

    inputMethodPanel->panelType_ = PanelType::STATUS_BAR;
    ret = inputMethodPanel->IsEnhancedParamValid(PanelFlag::FLG_FIXED, enhancedLayoutParams);
    EXPECT_EQ(ErrorCode::ERROR_INVALID_PANEL_TYPE, ret);

    Rosen::Rect rect1{10, 20, 100, 200};
    WindowSize displaySize1 {800, 600};
    EXPECT_TRUE(inputMethodPanel->IsRectValid(rect1, displaySize1));

    Rosen::Rect rect2{-10, 20, 100, 200};
    WindowSize displaySize2{800, 600};
    EXPECT_FALSE(inputMethodPanel->IsRectValid(rect2, displaySize2));

    Rosen::Rect rect3{10, 20, INT32_MAX, 200};
    WindowSize displaySize3{800, 600};
    EXPECT_FALSE(inputMethodPanel->IsRectValid(rect3, displaySize3));

    Rosen::Rect rect4{10, 20, 9000, 20000};
    WindowSize displaySize4{800, 600};
    EXPECT_FALSE(inputMethodPanel->IsRectValid(rect4, displaySize4));

    inputMethodPanel->window_ = nullptr;
    ret= inputMethodPanel->IsEnhancedParamValid(PanelFlag::FLG_FIXED, enhancedLayoutParams);
    EXPECT_EQ(ErrorCode::ERROR_WINDOW_MANAGER, ret);

    ret = inputMethodPanel->DestroyPanel();
    EXPECT_EQ(ErrorCode::ERROR_NULL_POINTER, ret);
}
 
 /**
  * @tc.name: testMoveEnhancedPanelRect
  * @tc.desc: Test Move Enhanced Panel Rect
  * @tc.type: FUNC
  */
HWTEST_F(InputMethodPanelTest, testMoveEnhancedPanelRect, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelTest::testMoveEnhancedPanelRect start.");
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    panelInfo.panelFlag = FLG_FLOATING;

    AccessScope scope(currentImeTokenId_, currentImeUid_);
    auto ret = inputMethodPanel->CreatePanel(nullptr, panelInfo);
    EXPECT_EQ(ErrorCode::NO_ERROR, ret);

    int32_t portraitX = 10;
    int32_t portraitY = 20;
    int32_t portraitRet = inputMethodPanel->MoveEnhancedPanelRect(portraitX, portraitY);
    EXPECT_EQ(ErrorCode::NO_ERROR, portraitRet);

    int32_t landscapeX = 30;
    int32_t landscapeY = 40;
    int32_t landscapeRet = inputMethodPanel->MoveEnhancedPanelRect(landscapeX, landscapeY);
    EXPECT_EQ(ErrorCode::NO_ERROR, landscapeRet);

    int32_t minX = -100;
    int32_t minY = -200;
    int32_t minRet = inputMethodPanel->MoveEnhancedPanelRect(minX, minY);
    EXPECT_EQ(ErrorCode::ERROR_PARAMETER_CHECK_FAILED, minRet);

    const std::string type = "sizeUpdate";
    EXPECT_TRUE(inputMethodPanel->MarkListener(type, false));

    uint32_t windowWidth = 100;
    bool isPortrait = false;
    ret = inputMethodPanel->GetWindowOrientation(PanelFlag::FLG_FIXED, windowWidth, isPortrait);
    EXPECT_EQ(ErrorCode::NO_ERROR, ret);

    ret = inputMethodPanel->SetImmersiveMode(ImmersiveMode::NONE_IMMERSIVE);
    EXPECT_EQ(ErrorCode::NO_ERROR, ret);
    inputMethodPanel->window_ = nullptr;
    ret = inputMethodPanel->SetImmersiveMode(ImmersiveMode::NONE_IMMERSIVE);
    EXPECT_EQ(ErrorCode::ERROR_IME, ret);

    ret = inputMethodPanel->DestroyPanel();
    EXPECT_EQ(ErrorCode::ERROR_NULL_POINTER, ret);
}
} // namespace MiscServices
} // namespace OHOS