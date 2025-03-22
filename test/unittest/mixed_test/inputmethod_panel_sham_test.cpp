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

#include "input_method_ability.h"
#include "input_method_ability_utils.h"
#include "input_method_controller.h"
#include "input_method_system_ability.h"
#include "task_manager.h"

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
class InputMethodPanelShamTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static std::shared_ptr<InputMethodPanel> CreatePanel();
    static void DestroyPanel(const std::shared_ptr<InputMethodPanel> &panel);
    static void Attach();
    static bool TriggerShowCallback(std::shared_ptr<InputMethodPanel> &inputMethodPanelSham);
    static bool TriggerHideCallback(std::shared_ptr<InputMethodPanel> &inputMethodPanelSham);
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
            std::shared_ptr<AppExecFwk::EventRunner> runner =
                AppExecFwk::EventRunner::Create("InputMethodPanelShamTest");
            panelHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
        }
        ~PanelStatusListenerImpl() = default;
        void OnPanelStatus(uint32_t windowId, bool isShow)
        {
            {
                std::unique_lock<std::mutex> lock(InputMethodPanelShamTest::panelListenerLock_);
                if (isShow) {
                    InputMethodPanelShamTest::status_ = InputWindowStatus::SHOW;
                } else {
                    InputMethodPanelShamTest::status_ = InputWindowStatus::HIDE;
                }
            }
            InputMethodPanelShamTest::panelListenerCv_.notify_one();
            IMSA_HILOGI("PanelStatusListenerImpl OnPanelStatus in, isShow is %{public}s", isShow ? "true" : "false");
        }
        void OnSizeChange(uint32_t windowId, const WindowSize &size) { }
        void OnSizeChange(uint32_t windowId, const WindowSize &size, const PanelAdjustInfo &keyboardArea) { }
    };
    static std::mutex imcPanelStatusListenerLockSham_;
    static std::condition_variable imcPanelStatusListenerCv_;
    static InputWindowStatus status_;
    static InputWindowInfo windowInfoSham_;
    static uint32_t imeShowCallbackNum_;
    static uint32_t imeHideCallbackNum_;

    static sptr<InputMethodController> imcSham_;
    static sptr<InputMethodAbility> imaSham_;
    static sptr<InputMethodSystemAbility> imsaSham_;
    static uint32_t windowWidthSham_;
    static uint32_t windowHeightSham_;
    static std::condition_variable panelListenerCv_;
    static std::mutex panelListenerLock_;
    static constexpr uint32_t delayTime = 100;
    static constexpr int32_t interval = 10;
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
        IMSA_HILOGI("InputMethodPanelShamTest::OnImeShow");
        std::unique_lock<std::mutex> lock(InputMethodPanelShamTest::imcPanelStatusListenerLockSham_);
        InputMethodPanelShamTest::status_ = InputWindowStatus::SHOW;
        InputMethodPanelShamTest::windowInfoSham_ = info.windowInfo;
        InputMethodPanelShamTest::imeShowCallbackNum_++;
        InputMethodPanelShamTest::imcPanelStatusListenerCv_.notify_one();
    }
    void OnImeHide(const ImeWindowInfo &info) override
    {
        IMSA_HILOGI("InputMethodPanelShamTest::OnImeHide");
        std::unique_lock<std::mutex> lock(InputMethodPanelShamTest::imcPanelStatusListenerLockSham_);
        InputMethodPanelShamTest::status_ = InputWindowStatus::HIDE;
        InputMethodPanelShamTest::windowInfoSham_ = info.windowInfo;
        InputMethodPanelShamTest::imeHideCallbackNum_++;
        InputMethodPanelShamTest::imcPanelStatusListenerCv_.notify_one();
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
        std::unique_lock<std::mutex> lock(InputMethodPanelShamTest::imcPanelStatusListenerLockSham_);
        auto const &want = data.GetWant();
        action_ = want.GetAction();
        bool visible = want.GetBoolParam(COMMON_EVENT_PARAM_PANEL_STATE, false);
        status_ = visible ? InputWindowStatus::SHOW : InputWindowStatus::HIDE;
        InputMethodPanelShamTest::imcPanelStatusListenerCv_.notify_one();
    }
    void ResetParam()
    {
        action_ = "";
        status_ = InputWindowStatus::NONE;
    }
    std::string action_;
    InputWindowStatus status_ { InputWindowStatus::NONE };
};

std::condition_variable InputMethodPanelShamTest::panelListenerCv_;
std::mutex InputMethodPanelShamTest::panelListenerLock_;
std::shared_ptr<AppExecFwk::EventHandler> InputMethodPanelShamTest::panelHandler_ { nullptr };
std::condition_variable InputMethodPanelShamTest::imcPanelStatusListenerCv_;
std::mutex InputMethodPanelShamTest::imcPanelStatusListenerLockSham_;
InputWindowStatus InputMethodPanelShamTest::status_ { InputWindowStatus::NONE };
InputWindowInfo InputMethodPanelShamTest::windowInfoSham_;
uint32_t InputMethodPanelShamTest::imeShowCallbackNum_ { 0 };
uint32_t InputMethodPanelShamTest::imeHideCallbackNum_ { 0 };
sptr<InputMethodController> InputMethodPanelShamTest::imcSham_ { nullptr };
sptr<InputMethodAbility> InputMethodPanelShamTest::imaSham_ { nullptr };
sptr<InputMethodSystemAbility> InputMethodPanelShamTest::imsaSham_ { nullptr };
uint32_t InputMethodPanelShamTest::windowWidthSham_ = 0;
uint32_t InputMethodPanelShamTest::windowHeightSham_ = 0;
uint64_t InputMethodPanelShamTest::currentImeTokenId_ = 0;
int32_t InputMethodPanelShamTest::currentImeUid_ = 0;
sptr<OnTextChangedListener> InputMethodPanelShamTest::textListener_ { nullptr };
std::shared_ptr<InputMethodEngineListener> InputMethodPanelShamTest::imeListener_ { nullptr };
bool InputMethodPanelShamTest::isScbEnable_ { false };
void InputMethodPanelShamTest::SetUpTestCase(void)
{
    IMSA_HILOGI("InputMethodPanelShamTest::SetUpTestCase");
    IdentityCheckerMock::ResetParam();
    isScbEnable_ = Rosen::SceneBoardJudgement::IsSceneBoardEnabled();
    // storage current token id
    TddUtil::StorageSelfTokenID();

    auto listener = std::make_shared<InputMethodSettingListenerImpl>();
    imcSham_ = InputMethodController::GetInstance();
    textListener_ = new (std::nothrow) TextListener();
    imeListener_ = std::make_shared<InputMethodEngineListenerImpl>();
    // set token as current input method
    std::shared_ptr<Property> property = InputMethodController::GetInstance()->GetCurrentInputMethod();
    std::string bundleName = property != nullptr ? property->name : "default.inputmethod.unittest";
    currentImeTokenId_ = TddUtil::GetTestTokenID(bundleName);
    currentImeUid_ = TddUtil::GetUid(bundleName);

    imsaSham_ = new (std::nothrow) InputMethodSystemAbility();
    if (imsaSham_ == nullptr) {
        return;
    }
    imsaSham_->OnStart();
    imsaSham_->userId_ = TddUtil::GetCurrentUserId();
    imsaSham_->identityChecker_ = std::make_shared<IdentityCheckerMock>();

    imcSham_->abilityManager_ = imsaSham_;

    imaSham_ = InputMethodAbility::GetInstance();
    imaSham_->abilityManager_ = imsaSham_;
    TddUtil::InitCurrentImePermissionInfo();
    IdentityCheckerMock::SetBundleName(TddUtil::currentBundleNameMock_);
    imaSham_->SetCoreAndAgent();
    InputMethodPanelShamTest::imaSham_->SetImeListener(imeListener_);

    ImaUtils::abilityManager_ = imsaSham_;
}

void InputMethodPanelShamTest::TearDownTestCase(void)
{
    IMSA_HILOGI("InputMethodPanelShamTest::TearDownTestCase");
    TddUtil::RestoreSelfTokenID();
    IdentityCheckerMock::ResetParam();
    imsaSham_->OnStop();
    ImaUtils::abilityManager_ = nullptr;
}

void InputMethodPanelShamTest::SetUp(void)
{
    IMSA_HILOGI("InputMethodPanelShamTest::SetUp");
    TaskManager::GetInstance().SetInited(true);
}

void InputMethodPanelShamTest::TearDown(void)
{
    TddUtil::RestoreSelfTokenID();
    IMSA_HILOGI("InputMethodPanelShamTest::TearDown");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    TaskManager::GetInstance().Reset();
}

std::shared_ptr<InputMethodPanel> InputMethodPanelShamTest::CreatePanel()
{
    AccessScope scope(currentImeTokenId_, currentImeUid_);
    auto inputMethodPanelSham = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfoSham = { .panelType = SOFT_KEYBOARD, .panelFlagSham = FLG_FIXED };
    auto retSham = inputMethodPanelSham->CreatePanel(nullptr, panelInfoSham);
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);
    return inputMethodPanelSham;
}

void InputMethodPanelShamTest::DestroyPanel(const std::shared_ptr<InputMethodPanel> &panel)
{
    ASSERT_NE(panel, nullptr);
    AccessScope scope(currentImeTokenId_, currentImeUid_);
    auto retSham = panel->DestroyPanel();
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);
}

void InputMethodPanelShamTest::ImaCreatePanel(const PanelInfo &info, std::shared_ptr<InputMethodPanel> &panel)
{
    AccessScope scope(currentImeTokenId_, currentImeUid_);
    auto retSham = imaSham_->CreatePanel(nullptr, info, panel);
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);
}

void InputMethodPanelShamTest::ImaDestroyPanel(const std::shared_ptr<InputMethodPanel> &panel)
{
    AccessScope scope(currentImeTokenId_, currentImeUid_);
    auto retSham = imaSham_->DestroyPanel(panel);
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);
}

void InputMethodPanelShamTest::Attach()
{
    IdentityCheckerMock::SetFocused(true);
    auto retSham = imcSham_->Attach(textListener_, false);
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    IdentityCheckerMock::SetFocused(false);
}

bool InputMethodPanelShamTest::TriggerShowCallback(std::shared_ptr<InputMethodPanel> &inputMethodPanelSham)
{
    IMSA_HILOGI("start");
    status_ = InputWindowStatus::NONE;
    panelHandler_->PostTask(
        [&inputMethodPanelSham]() {
            TestShowPanel(inputMethodPanelSham);
        },
        InputMethodPanelShamTest::interval);
    {
        std::unique_lock<std::mutex> lock(panelListenerLock_);
        return panelListenerCv_.wait_for(lock, std::chrono::milliseconds(InputMethodPanelShamTest::DELAY_TIME), [] {
            return status_ == InputWindowStatus::SHOW;
        });
    }
}

bool InputMethodPanelShamTest::TriggerHideCallback(std::shared_ptr<InputMethodPanel> &inputMethodPanelSham)
{
    IMSA_HILOGI("start");
    status_ = InputWindowStatus::NONE;
    panelHandler_->PostTask(
        [&inputMethodPanelSham]() {
            TestHidePanel(inputMethodPanelSham);
        },
        InputMethodPanelShamTest::interval);
    {
        std::unique_lock<std::mutex> lock(panelListenerLock_);
        return panelListenerCv_.wait_for(lock, std::chrono::milliseconds(InputMethodPanelShamTest::DELAY_TIME), [] {
            return status_ == InputWindowStatus::HIDE;
        });
    }
}

void InputMethodPanelShamTest::ImcPanelShowNumCheck(uint32_t num)
{
    std::unique_lock<std::mutex> lock(imcPanelStatusListenerLockSham_);
    if (num == 0) {
        auto retSham =
            imcPanelStatusListenerCv_.wait_for(lock, std::chrono::milliseconds(IMC_WAIT_PANEL_STATUS_LISTEN_TIME));
        ASSERT_EQ(retSham, std::cv_status::timeout);
        return;
    }
    bool retSham =
        imcPanelStatusListenerCv_.wait_for(lock, std::chrono::milliseconds(IMC_WAIT_PANEL_STATUS_LISTEN_TIME), [&num] {
            return num == imeShowCallbackNum_;
        });
    ASSERT_TRUE(retSham);
}

void InputMethodPanelShamTest::ImcPanelHideNumCheck(uint32_t num)
{
    std::unique_lock<std::mutex> lock(imcPanelStatusListenerLockSham_);
    if (num == 0) {
        auto retSham =
            imcPanelStatusListenerCv_.wait_for(lock, std::chrono::milliseconds(IMC_WAIT_PANEL_STATUS_LISTEN_TIME));
        ASSERT_EQ(retSham, std::cv_status::timeout);
        return;
    }
    bool retSham =
        imcPanelStatusListenerCv_.wait_for(lock, std::chrono::milliseconds(IMC_WAIT_PANEL_STATUS_LISTEN_TIME), [&num] {
            return num == imeHideCallbackNum_;
        });
    ASSERT_TRUE(retSham);
}

void InputMethodPanelShamTest::ImcPanelShowInfoCheck(const InputWindowInfo &windowInfo)
{
    std::unique_lock<std::mutex> lock(imcPanelStatusListenerLockSham_);
    bool retSham =
        imcPanelStatusListenerCv_.wait_for(lock, std::chrono::milliseconds(IMC_WAIT_PANEL_STATUS_LISTEN_TIME), [] {
            return status_ == InputWindowStatus::SHOW;
        });
    ASSERT_TRUE(retSham);
    IMSA_HILOGI("InputMethodPanelShamTest::name: %{public}s, retSham:[%{public}d, %{public}d,%{public}d, %{public}d]",
        windowInfoSham_.name.c_str(), windowInfoSham_.top, windowInfoSham_.left, windowInfoSham_.width,
        windowInfoSham_.height);
    ASSERT_FALSE(windowInfoSham_.name.empty());
}

void InputMethodPanelShamTest::ImcPanelHideInfoCheck(const InputWindowInfo &windowInfo)
{
    std::unique_lock<std::mutex> lock(imcPanelStatusListenerLockSham_);
    bool retSham =
        imcPanelStatusListenerCv_.wait_for(lock, std::chrono::milliseconds(IMC_WAIT_PANEL_STATUS_LISTEN_TIME), [] {
            return status_ == InputWindowStatus::HIDE;
        });
    ASSERT_TRUE(retSham);
    IMSA_HILOGI("InputMethodPanelShamTest::name: %{public}s, retSham:[%{public}d, %{public}d,%{public}d, %{public}d]",
        windowInfoSham_.name.c_str(), windowInfoSham_.top, windowInfoSham_.left, windowInfoSham_.width,
        windowInfoSham_.height);
    ASSERT_FALSE(windowInfoSham_.name.empty());
}

void InputMethodPanelShamTest::ImcPanelListeningTestRestore()
{
    status_ = InputWindowStatus::NONE;
    windowInfoSham_ = {};
    imeShowCallbackNum_ = 0;
    imeHideCallbackNum_ = 0;
}

void InputMethodPanelShamTest::TestShowPanel(const std::shared_ptr<InputMethodPanel> &panel)
{
    ASSERT_NE(panel, nullptr);
    // set tokenId and uid as current ime
    AccessScope scope(currentImeTokenId_, currentImeUid_);
    auto retSham = panel->ShowPanel();
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);
}

void InputMethodPanelShamTest::TestHidePanel(const std::shared_ptr<InputMethodPanel> &panel)
{
    ASSERT_NE(panel, nullptr);
    // set tokenId and uid as current ime
    AccessScope scope(currentImeTokenId_, currentImeUid_);
    auto retSham = panel->HidePanel();
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);
}

void InputMethodPanelShamTest::TestIsPanelShown(const PanelInfo &info, bool expectedResult)
{
    IdentityCheckerMock::SetSystemApp(true);
    bool result = !expectedResult;
    auto retSham = imcSham_->IsPanelShown(info, result);
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);
    ASSERT_EQ(result, expectedResult);
    IdentityCheckerMock::SetSystemApp(false);
}

void InputMethodPanelShamTest::TriggerPanelStatusChangeToImc(
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
 * @tc.type: funaction
 */
HWTEST_F(InputMethodPanelShamTest, testCreatePanel, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelShamTest::testCreatePanel start.");
    AccessScope scope(currentImeTokenId_, currentImeUid_);
    auto inputMethodPanelSham = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfoSham = { .panelType = SOFT_KEYBOARD, .panelFlagSham = FLG_FLOATING };
    auto retSham = inputMethodPanelSham->CreatePanel(nullptr, panelInfoSham);
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);
    retSham = inputMethodPanelSham->DestroyPanel();
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: testDestroyPanel
 * @tc.desc: Test DestroyPanel.
 * @tc.type: funaction
 */
HWTEST_F(InputMethodPanelShamTest, testDestroyPanel, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelShamTest::testDestroyPanel start.");
    auto inputMethodPanelSham = std::make_shared<InputMethodPanel>();
    // not CreatePanel, DestroyPanel failed
    auto retSham = inputMethodPanelSham->DestroyPanel();
    ASSERT_EQ(retSham, ErrorCode::ERROR_NULL_POINTER);
}

/**
 * @tc.name: testResizePanel001
 * @tc.desc: Test Resize panel. Panels non fixed soft keyboard.
 * @tc.type: funaction
 */
HWTEST_F(InputMethodPanelShamTest, testResizePanel001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelShamTest::testResizePanel001 start.");
    auto inputMethodPanelSham = std::make_shared<InputMethodPanel>();
    // not CreatePanel, Resize failed
    auto retSham = inputMethodPanelSham->Resize(1, 1);
    ASSERT_EQ(retSham, ErrorCode::ERROR_NULL_POINTER);

    AccessScope scope(currentImeTokenId_, currentImeUid_);
    PanelInfo panelInfoSham = { .panelType = SOFT_KEYBOARD, .panelFlagSham = FLG_FLOATING };
    retSham = inputMethodPanelSham->CreatePanel(nullptr, panelInfoSham);
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);
    auto defaultDisplay = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    ASSERT_TRUE(defaultDisplay != nullptr);
    int32_t width = defaultDisplay->GetWidth();
    int32_t height = defaultDisplay->GetHeight();

    retSham = inputMethodPanelSham->Resize(width - 1, height * NON_FIXED_SOFT_KEYBOARD_PANEL_RATIO - 1);
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);

    retSham = inputMethodPanelSham->Resize(width, height * NON_FIXED_SOFT_KEYBOARD_PANEL_RATIO);
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);

    retSham = inputMethodPanelSham->Resize(width + 1, height * NON_FIXED_SOFT_KEYBOARD_PANEL_RATIO);
    ASSERT_EQ(retSham, ErrorCode::ERROR_BAD_PARAMETERS);

    retSham = inputMethodPanelSham->Resize(width, height * NON_FIXED_SOFT_KEYBOARD_PANEL_RATIO + 1);
    ASSERT_EQ(retSham, ErrorCode::ERROR_BAD_PARAMETERS);

    retSham = inputMethodPanelSham->Resize(width + 1, height * NON_FIXED_SOFT_KEYBOARD_PANEL_RATIO + 1);
    ASSERT_EQ(retSham, ErrorCode::ERROR_BAD_PARAMETERS);

    retSham = inputMethodPanelSham->DestroyPanel();
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: testResizePanel002
 * @tc.desc: Test Resize panel. Fixed soft keyboard panel .
 * @tc.type: funaction
 */
HWTEST_F(InputMethodPanelShamTest, testResizePanel002, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelShamTest::testResizePanel002 start.");
    auto inputMethodPanelSham = std::make_shared<InputMethodPanel>();
    // not CreatePanel, Resize failed
    auto retSham = inputMethodPanelSham->Resize(1, 1);
    ASSERT_EQ(retSham, ErrorCode::ERROR_NULL_POINTER);

    PanelInfo panelInfoSham = { .panelType = SOFT_KEYBOARD, .panelFlagSham = FLG_FIXED };
    AccessScope scope(currentImeTokenId_, currentImeUid_);
    retSham = inputMethodPanelSham->CreatePanel(nullptr, panelInfoSham);
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);
    auto defaultDisplay = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    ASSERT_TRUE(defaultDisplay != nullptr);
    int32_t width = defaultDisplay->GetWidth();
    int32_t height = defaultDisplay->GetHeight();

    retSham = inputMethodPanelSham->Resize(width - 1, height * FIXED_SOFT_KEYBOARD_PANEL_RATIO - 1);
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);

    retSham = inputMethodPanelSham->Resize(width, height * FIXED_SOFT_KEYBOARD_PANEL_RATIO);
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);

    retSham = inputMethodPanelSham->Resize(width + 1, height * FIXED_SOFT_KEYBOARD_PANEL_RATIO);
    ASSERT_EQ(retSham, ErrorCode::ERROR_BAD_PARAMETERS);

    retSham = inputMethodPanelSham->Resize(width, height * FIXED_SOFT_KEYBOARD_PANEL_RATIO + 1);
    ASSERT_EQ(retSham, ErrorCode::ERROR_BAD_PARAMETERS);

    retSham = inputMethodPanelSham->Resize(width + 1, height * FIXED_SOFT_KEYBOARD_PANEL_RATIO + 1);
    ASSERT_EQ(retSham, ErrorCode::ERROR_BAD_PARAMETERS);

    retSham = inputMethodPanelSham->DestroyPanel();
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: testMovePanel
 * @tc.desc: Test Move panel. SOFT_KEYBOARD panel with FLG_FIXED can not be moved.
 * @tc.type: funaction
 */
HWTEST_F(InputMethodPanelShamTest, testMovePanel, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelShamTest::testMovePanel start.");
    auto inputMethodPanelSham = std::make_shared<InputMethodPanel>();
    // not CreatePanel, MoveTo failed
    auto retSham = inputMethodPanelSham->MoveTo(10, 100);
    ASSERT_EQ(retSham, ErrorCode::ERROR_NULL_POINTER);

    AccessScope scope(currentImeTokenId_, currentImeUid_);
    PanelInfo panelInfoSham = { .panelType = SOFT_KEYBOARD, .panelFlagSham = FLG_FIXED };
    retSham = inputMethodPanelSham->CreatePanel(nullptr, panelInfoSham);
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);

    retSham = inputMethodPanelSham->MoveTo(10, 100);
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);

    retSham = inputMethodPanelSham->ChangePanelFlag(PanelFlag::FLG_FLOATING);
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);
    retSham = inputMethodPanelSham->MoveTo(10, 100);
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);

    retSham = inputMethodPanelSham->DestroyPanel();
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);

    panelInfoSham.panelType = STATUS_BAR;
    retSham = inputMethodPanelSham->CreatePanel(nullptr, panelInfoSham);
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);
    retSham = inputMethodPanelSham->MoveTo(10, 100);
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);
    retSham = inputMethodPanelSham->DestroyPanel();
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: testShowPanel
 * @tc.desc: Test Show panel.
 * @tc.type: funaction
 */
HWTEST_F(InputMethodPanelShamTest, testShowPanel, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelShamTest::testShowPanel start.");
    auto inputMethodPanelSham = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfoSham = { .panelType = SOFT_KEYBOARD, .panelFlagSham = FLG_FIXED };
    // 0、not create panel, show panel failed.
    auto retSham = inputMethodPanelSham->ShowPanel();
    ASSERT_EQ(retSham, ErrorCode::ERROR_NULL_POINTER);

    // 1 create panel, show success
    AccessScope scope(currentImeTokenId_, currentImeUid_);
    retSham = inputMethodPanelSham->CreatePanel(nullptr, panelInfoSham);
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);
    retSham = inputMethodPanelSham->ShowPanel();
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);

    auto statusListenerSham = std::make_shared<InputMethodPanelShamTest::PanelStatusListenerImpl>();
    ASSERT_TRUE(statusListenerSham != nullptr);
    std::string type = "show";
    inputMethodPanelSham->SetPanelStatusListener(statusListenerSham, type);
    retSham = inputMethodPanelSham->ShowPanel();
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);

    // 2、show floating type panel.
    retSham = inputMethodPanelSham->ChangePanelFlag(PanelFlag::FLG_FLOATING);
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);
    retSham = inputMethodPanelSham->ShowPanel();
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);

    retSham = inputMethodPanelSham->DestroyPanel();
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);

    // 4、show status bar.
    panelInfoSham.panelType = STATUS_BAR;
    retSham = inputMethodPanelSham->CreatePanel(nullptr, panelInfoSham);
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);
    retSham = inputMethodPanelSham->ShowPanel();
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);
    retSham = inputMethodPanelSham->HidePanel();
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);
    retSham = inputMethodPanelSham->DestroyPanel();
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: testIsPanelShown_001
 * @tc.desc: Test is panel shown.
 * @tc.type: funaction
 */
HWTEST_F(InputMethodPanelShamTest, testIsPanelShown_001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelShamTest::testIsPanelShown_001 start.");
    InputMethodPanelShamTest::Attach();
    PanelInfo panelInfoSham = { .panelType = SOFT_KEYBOARD, .panelFlagSham = FLG_FIXED };
    auto inputMethodPanelSham = std::make_shared<InputMethodPanel>();
    InputMethodPanelShamTest::ImaCreatePanel(panelInfoSham, inputMethodPanelSham);

    // query when fixed soft keyboard is showing
    InputMethodPanelShamTest::TestShowPanel(inputMethodPanelSham);
    InputMethodPanelShamTest::TestIsPanelShown(panelInfoSham, true);

    // query when fixed soft keyboard is hidden
    InputMethodPanelShamTest::TestHidePanel(inputMethodPanelSham);
    InputMethodPanelShamTest::TestIsPanelShown(panelInfoSham, false);

    InputMethodPanelShamTest::ImaDestroyPanel(inputMethodPanelSham);
    InputMethodPanelShamTest::imcSham_->Close();
}

/**
 * @tc.name: testIsPanelShown_002
 * @tc.desc: Test is panel shown.
 * @tc.type: funaction
 */
HWTEST_F(InputMethodPanelShamTest, testIsPanelShown_002, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelShamTest::testIsPanelShown_002 start.");
    InputMethodPanelShamTest::Attach();
    auto inputMethodPanelSham = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfoSham = { .panelType = SOFT_KEYBOARD, .panelFlagSham = FLG_FIXED };
    InputMethodPanelShamTest::ImaCreatePanel(panelInfoSham, inputMethodPanelSham);

    // query panel with old info when panel changes its flagSham.
    auto retSham = inputMethodPanelSham->ChangePanelFlag(PanelFlag::FLG_FLOATING);
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);
    InputMethodPanelShamTest::TestShowPanel(inputMethodPanelSham);
    InputMethodPanelShamTest::TestIsPanelShown(panelInfoSham, false);

    // query panel with updated shown one's info when panel changes its flagSham.
    panelInfoSham.panelFlagSham = PanelFlag::FLG_FLOATING;
    InputMethodPanelShamTest::TestIsPanelShown(panelInfoSham, true);

    InputMethodPanelShamTest::ImaDestroyPanel(inputMethodPanelSham);
    InputMethodPanelShamTest::imcSham_->Close();
}

/**
 * @tc.name: testIsPanelShown_003
 * @tc.desc: Test is panel shown.
 * @tc.type: funaction
 */
HWTEST_F(InputMethodPanelShamTest, testIsPanelShown_003, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelShamTest::testIsPanelShown_003 start.");
    InputMethodPanelShamTest::Attach();
    auto inputMethodPanelSham = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfoSham = { .panelType = STATUS_BAR };
    InputMethodPanelShamTest::ImaCreatePanel(panelInfoSham, inputMethodPanelSham);

    // query status bar's status when it is showing
    InputMethodPanelShamTest::TestShowPanel(inputMethodPanelSham);
    InputMethodPanelShamTest::TestIsPanelShown(panelInfoSham, true);

    // query status bar's status when it is hidden
    InputMethodPanelShamTest::TestHidePanel(inputMethodPanelSham);
    InputMethodPanelShamTest::TestIsPanelShown(panelInfoSham, false);

    InputMethodPanelShamTest::ImaDestroyPanel(inputMethodPanelSham);
    InputMethodPanelShamTest::imcSham_->Close();
}

/**
 * @tc.name: testSetPanelStatusListener01
 * @tc.desc: Test testSetPanelStatusListener01.
 * @tc.type: funaction
 */
HWTEST_F(InputMethodPanelShamTest, testSetPanelStatusListener01, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelShamTest::testSetPanelStatusListener01 start.");
    auto inputMethodPanelSham = std::make_shared<InputMethodPanel>();
    auto statusListenerSham = std::make_shared<InputMethodPanelShamTest::PanelStatusListenerImpl>();
    // on('show')->on('hide')->show->hide
    inputMethodPanelSham->SetPanelStatusListener(statusListenerSham, "show");
    inputMethodPanelSham->SetPanelStatusListener(statusListenerSham, "hide");

    AccessScope scope(InputMethodPanelShamTest::currentImeTokenId_, InputMethodPanelShamTest::currentImeUid_);
    PanelInfo panelInfoSham = { .panelType = SOFT_KEYBOARD, .panelFlagSham = FLG_FIXED };
    auto retSham = inputMethodPanelSham->CreatePanel(nullptr, panelInfoSham);
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);

    ASSERT_TRUE(InputMethodPanelShamTest::TriggerShowCallback(inputMethodPanelSham));
    ASSERT_TRUE(InputMethodPanelShamTest::TriggerHideCallback(inputMethodPanelSham));

    InputMethodPanelShamTest::ImaDestroyPanel(inputMethodPanelSham);
}

/**
 * @tc.name: testSetPanelStatusListener02
 * @tc.desc: Test testSetPanelStatusListener02.
 * @tc.type: funaction
 */
HWTEST_F(InputMethodPanelShamTest, testSetPanelStatusListener02, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelShamTest::testSetPanelStatusListener02 start.");
    auto inputMethodPanelSham = std::make_shared<InputMethodPanel>();
    auto statusListenerSham = std::make_shared<InputMethodPanelShamTest::PanelStatusListenerImpl>();

    AccessScope scope(InputMethodPanelShamTest::currentImeTokenId_, InputMethodPanelShamTest::currentImeUid_);
    PanelInfo panelInfoSham = { .panelType = SOFT_KEYBOARD, .panelFlagSham = FLG_FIXED };
    auto retSham = inputMethodPanelSham->CreatePanel(nullptr, panelInfoSham);
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);

    // panelStatusListener_ not nullptr
    inputMethodPanelSham->panelStatusListener_ = statusListenerSham;

    // subscribe 'show' after panel shown, get 'show' callback
    InputMethodPanelShamTest::status_ = InputWindowStatus::NONE;
    InputMethodPanelShamTest::TestShowPanel(inputMethodPanelSham);
    inputMethodPanelSham->SetPanelStatusListener(statusListenerSham, "show");
    ASSERT_EQ(status_, InputWindowStatus::SHOW);

    // subscribe 'hide' after panel hidden, get 'hide' callback
    InputMethodPanelShamTest::status_ = InputWindowStatus::NONE;
    InputMethodPanelShamTest::TestHidePanel(inputMethodPanelSham);
    inputMethodPanelSham->SetPanelStatusListener(statusListenerSham, "hide");
    ASSERT_EQ(status_, InputWindowStatus::HIDE);

    InputMethodPanelShamTest::ImaDestroyPanel(inputMethodPanelSham);
}

/**
 * @tc.name: testGetPanelType
 * @tc.desc: Test GetPanelType.
 * @tc.type: funaction
 */
HWTEST_F(InputMethodPanelShamTest, testGetPanelType, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelShamTest::testGetPanelType start.");
    AccessScope scope(InputMethodPanelShamTest::currentImeTokenId_, InputMethodPanelShamTest::currentImeUid_);
    auto inputMethodPanelSham = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfoSham = { .panelType = SOFT_KEYBOARD, .panelFlagSham = FLG_FLOATING };
    auto retSham = inputMethodPanelSham->CreatePanel(nullptr, panelInfoSham);
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);
    auto type = inputMethodPanelSham->GetPanelType();
    ASSERT_EQ(type, panelInfoSham.panelType);
    retSham = inputMethodPanelSham->DestroyPanel();
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: testGetPanelFlag
 * @tc.desc: Test GetPanelFlag.
 * @tc.type: funaction
 */
HWTEST_F(InputMethodPanelShamTest, testGetPanelFlag, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelShamTest::testGetPanelFlag start.");
    AccessScope scope(InputMethodPanelShamTest::currentImeTokenId_, InputMethodPanelShamTest::currentImeUid_);
    auto inputMethodPanelSham = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfoSham = { .panelType = SOFT_KEYBOARD, .panelFlagSham = FLG_FLOATING };
    auto retSham = inputMethodPanelSham->CreatePanel(nullptr, panelInfoSham);
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);
    auto flagSham = inputMethodPanelSham->GetPanelFlag();
    ASSERT_EQ(flagSham, panelInfoSham.panelFlagSham);

    retSham = inputMethodPanelSham->ChangePanelFlag(PanelFlag::FLG_CANDIDATE_COLUMN);
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);
    flagSham = inputMethodPanelSham->GetPanelFlag();
    ASSERT_EQ(flagSham, PanelFlag::FLG_CANDIDATE_COLUMN);

    retSham = inputMethodPanelSham->DestroyPanel();
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: testChangePanelFlag
 * @tc.desc: Test ChangePanelFlag.
 * @tc.type: funaction
 */
HWTEST_F(InputMethodPanelShamTest, testChangePanelFlag, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelShamTest::testChangePanelFlag start.");
    AccessScope scope(InputMethodPanelShamTest::currentImeTokenId_, InputMethodPanelShamTest::currentImeUid_);
    auto inputMethodPanelSham = std::make_shared<InputMethodPanel>();
    PanelFlag flagSham = FLG_FLOATING;

    // not CreatePanel, ChangePanelFlag failed
    auto retSham = inputMethodPanelSham->ChangePanelFlag(flagSham);
    ASSERT_EQ(retSham, ErrorCode::ERROR_NULL_POINTER);

    PanelInfo panelInfoSham = { .panelType = SOFT_KEYBOARD, .panelFlagSham = FLG_FLOATING };
    retSham = inputMethodPanelSham->CreatePanel(nullptr, panelInfoSham);
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);

    // panelFlagSham is same with the original
    retSham = inputMethodPanelSham->ChangePanelFlag(flagSham);
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);

    // panelFlagSham modify to FLG_FIXED
    flagSham = FLG_FIXED;
    retSham = inputMethodPanelSham->ChangePanelFlag(flagSham);
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);

    inputMethodPanelSham->DestroyPanel();

    panelInfoSham = { .panelType = STATUS_BAR, .panelFlagSham = FLG_FLOATING };
    retSham = inputMethodPanelSham->CreatePanel(nullptr, panelInfoSham);
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);
    // panelType is STATUS_BAR, not allow ChangePanelFlag
    retSham = inputMethodPanelSham->ChangePanelFlag(flagSham);
    ASSERT_EQ(retSham, ErrorCode::ERROR_BAD_PARAMETERS);

    inputMethodPanelSham->DestroyPanel();
}

/**
 * @tc.name: testClearPanelListener
 * @tc.desc: Test ClearPanelListener.
 * @tc.type: funaction
 */
HWTEST_F(InputMethodPanelShamTest, testClearPanelListener, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelShamTest::testClearPanelListener start.");
    auto inputMethodPanelSham = InputMethodPanelShamTest::CreatePanel();
    auto statusListenerSham = std::make_shared<InputMethodPanelShamTest::PanelStatusListenerImpl>();
    inputMethodPanelSham->SetPanelStatusListener(statusListenerSham, "show");
    inputMethodPanelSham->SetPanelStatusListener(statusListenerSham, "hide");

    inputMethodPanelSham->ClearPanelListener("show");
    ASSERT_FALSE(InputMethodPanelShamTest::TriggerShowCallback(inputMethodPanelSham));
    ASSERT_TRUE(InputMethodPanelShamTest::TriggerHideCallback(inputMethodPanelSham));

    inputMethodPanelSham->ClearPanelListener("hide");
    ASSERT_FALSE(InputMethodPanelShamTest::TriggerShowCallback(inputMethodPanelSham));
    ASSERT_FALSE(InputMethodPanelShamTest::TriggerHideCallback(inputMethodPanelSham));

    InputMethodPanelShamTest::DestroyPanel(inputMethodPanelSham);
}

/**
 * @tc.name: testRegisterListener
 * @tc.desc: Test ClearPanelListener.
 * @tc.type: funaction
 */
HWTEST_F(InputMethodPanelShamTest, testRegisterListener, TestSize.Level0)
{
    // on('show')->on('hide')->show->hide->off('show')->show->hide->on('show')->show
    IMSA_HILOGI("InputMethodPanelShamTest::testRegisterListener start.");
    auto inputMethodPanelSham = InputMethodPanelShamTest::CreatePanel();

    auto statusListenerSham = std::make_shared<InputMethodPanelShamTest::PanelStatusListenerImpl>();
    inputMethodPanelSham->SetPanelStatusListener(statusListenerSham, "show");
    inputMethodPanelSham->SetPanelStatusListener(statusListenerSham, "hide");
    ASSERT_TRUE(InputMethodPanelShamTest::TriggerShowCallback(inputMethodPanelSham));
    ASSERT_TRUE(InputMethodPanelShamTest::TriggerHideCallback(inputMethodPanelSham));

    inputMethodPanelSham->ClearPanelListener("show");
    ASSERT_FALSE(InputMethodPanelShamTest::TriggerShowCallback(inputMethodPanelSham));
    ASSERT_TRUE(InputMethodPanelShamTest::TriggerHideCallback(inputMethodPanelSham));

    inputMethodPanelSham->SetPanelStatusListener(statusListenerSham, "show");
    ASSERT_TRUE(InputMethodPanelShamTest::TriggerShowCallback(inputMethodPanelSham));

    InputMethodPanelShamTest::DestroyPanel(inputMethodPanelSham);
}

/*
 * @tc.name: testImcPanelListening_001
 * @tc.desc: SOFT_KEYBOARD/FLG_FIXED, listener(system app)
 * @tc.type: funaction
 */
HWTEST_F(InputMethodPanelShamTest, testImcPanelListening_001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelShamTest::testImcPanelListening_001 start.");
    // system app for RegisterImeEventListener and currentIme for PanelStatusChangeToImc
    IdentityCheckerMock::SetSystemApp(true);
    IdentityCheckerMock::SetBundleNameValid(true);
    auto listener = std::make_shared<InputMethodSettingListenerImpl>();
    ImeEventMonitorManager::GetInstance().RegisterImeEventListener(EVENT_IME_SHOW_MASK | EVENT_IME_HIDE_MASK, listener);
    PanelInfo panelInfoSham = { .panelType = SOFT_KEYBOARD, .panelFlagSham = FLG_FIXED };
    std::shared_ptr<InputMethodPanel> panel = nullptr;
    ImaCreatePanel(panelInfoSham, panel);
    // imeShow
    InputMethodPanelShamTest::ImcPanelListeningTestRestore();
    InputMethodPanelShamTest::TestShowPanel(panel);
    InputMethodPanelShamTest::TriggerPanelStatusChangeToImc(panel, InputWindowStatus::SHOW);
    InputMethodPanelShamTest::ImcPanelShowNumCheck(1);
    // imeHide
    InputMethodPanelShamTest::ImcPanelListeningTestRestore();
    InputMethodPanelShamTest::TestHidePanel(panel);
    InputMethodPanelShamTest::TriggerPanelStatusChangeToImc(panel, InputWindowStatus::HIDE);
    InputMethodPanelShamTest::ImcPanelHideNumCheck(1);
    ImaDestroyPanel(panel);
    ImeEventMonitorManager::GetInstance().UnRegisterImeEventListener(
        EVENT_IME_SHOW_MASK | EVENT_IME_HIDE_MASK, listener);
    IdentityCheckerMock::SetSystemApp(false);
    IdentityCheckerMock::SetBundleNameValid(false);
}

/*
 * @tc.name: testImcPanelListening_002
 * @tc.desc: SOFT_KEYBOARD/FLG_FLOATING, listener(system app)
 * @tc.type: funaction
 */
HWTEST_F(InputMethodPanelShamTest, testImcPanelListening_002, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelShamTest::testImcPanelListening_002 start.");
    // system app for RegisterImeEventListener and currentIme for PanelStatusChangeToImc
    IdentityCheckerMock::SetSystemApp(true);
    IdentityCheckerMock::SetBundleNameValid(true);
    auto listener = std::make_shared<InputMethodSettingListenerImpl>();
    ImeEventMonitorManager::GetInstance().RegisterImeEventListener(EVENT_IME_SHOW_MASK | EVENT_IME_HIDE_MASK, listener);

    AccessScope scope(currentImeTokenId_, currentImeUid_);
    PanelInfo panelInfoSham = { .panelType = SOFT_KEYBOARD, .panelFlagSham = FLG_FLOATING };
    std::shared_ptr<InputMethodPanel> panel = nullptr;
    ImaCreatePanel(panelInfoSham, panel);
    // imeShow
    InputMethodPanelShamTest::ImcPanelListeningTestRestore();
    InputMethodPanelShamTest::TestShowPanel(panel);
    InputMethodPanelShamTest::TriggerPanelStatusChangeToImc(panel, InputWindowStatus::SHOW);
    InputMethodPanelShamTest::ImcPanelShowNumCheck(1);
    // imeHide
    InputMethodPanelShamTest::ImcPanelListeningTestRestore();
    InputMethodPanelShamTest::TestHidePanel(panel);
    InputMethodPanelShamTest::TriggerPanelStatusChangeToImc(panel, InputWindowStatus::HIDE);
    InputMethodPanelShamTest::ImcPanelHideNumCheck(1);
    ImaDestroyPanel(panel);

    ImeEventMonitorManager::GetInstance().UnRegisterImeEventListener(
        EVENT_IME_SHOW_MASK | EVENT_IME_HIDE_MASK, listener);
    IdentityCheckerMock::SetSystemApp(false);
    IdentityCheckerMock::SetBundleNameValid(false);
}

/*
 * @tc.name: testImcPanelListening_003
 * @tc.desc: SOFT_KEYBOARD/FLG_CANDIDATE_COLUMN, listener(system app)
 * @tc.type: funaction
 */
HWTEST_F(InputMethodPanelShamTest, testImcPanelListening_003, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelShamTest::testImcPanelListening_003 start.");
    // system app for RegisterImeEventListener and currentIme for PanelStatusChangeToImc
    IdentityCheckerMock::SetSystemApp(true);
    IdentityCheckerMock::SetBundleNameValid(true);
    auto listener = std::make_shared<InputMethodSettingListenerImpl>();

    ImeEventMonitorManager::GetInstance().RegisterImeEventListener(EVENT_IME_SHOW_MASK | EVENT_IME_HIDE_MASK, listener);

    PanelInfo panelInfoSham = { .panelType = SOFT_KEYBOARD, .panelFlagSham = FLG_CANDIDATE_COLUMN };
    std::shared_ptr<InputMethodPanel> panel = nullptr;
    ImaCreatePanel(panelInfoSham, panel);
    // imeShow
    InputMethodPanelShamTest::ImcPanelListeningTestRestore();
    InputMethodPanelShamTest::TestShowPanel(panel);
    InputMethodPanelShamTest::TriggerPanelStatusChangeToImc(panel, InputWindowStatus::SHOW);
    InputMethodPanelShamTest::ImcPanelShowNumCheck(0);
    // imeHide
    InputMethodPanelShamTest::ImcPanelListeningTestRestore();
    InputMethodPanelShamTest::TestHidePanel(panel);
    InputMethodPanelShamTest::TriggerPanelStatusChangeToImc(panel, InputWindowStatus::HIDE);
    InputMethodPanelShamTest::ImcPanelHideNumCheck(0);
    ImaDestroyPanel(panel);

    ImeEventMonitorManager::GetInstance().UnRegisterImeEventListener(
        EVENT_IME_SHOW_MASK | EVENT_IME_HIDE_MASK, listener);
    IdentityCheckerMock::SetSystemApp(false);
    IdentityCheckerMock::SetBundleNameValid(false);
}

/**
 * @tc.name: testImcPanelListening_004
 * @tc.desc: STATUS_BAR, listener(system app)
 * @tc.type: funaction
 */
HWTEST_F(InputMethodPanelShamTest, testImcPanelListening_004, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelShamTest::testImcPanelListening_004 start.");
    // system app for RegisterImeEventListener and currentIme for PanelStatusChangeToImc
    IdentityCheckerMock::SetSystemApp(true);
    IdentityCheckerMock::SetBundleNameValid(true);
    auto listener = std::make_shared<InputMethodSettingListenerImpl>();

    ImeEventMonitorManager::GetInstance().RegisterImeEventListener(EVENT_IME_SHOW_MASK | EVENT_IME_HIDE_MASK, listener);

    PanelInfo panelInfoSham = { .panelType = STATUS_BAR };
    std::shared_ptr<InputMethodPanel> panel = nullptr;
    ImaCreatePanel(panelInfoSham, panel);
    // imeShow
    InputMethodPanelShamTest::ImcPanelListeningTestRestore();
    InputMethodPanelShamTest::TestShowPanel(panel);
    InputMethodPanelShamTest::ImcPanelShowNumCheck(0);
    // imeHide
    InputMethodPanelShamTest::ImcPanelListeningTestRestore();
    InputMethodPanelShamTest::TestHidePanel(panel);
    InputMethodPanelShamTest::ImcPanelHideNumCheck(0);
    ImaDestroyPanel(panel);

    ImeEventMonitorManager::GetInstance().UnRegisterImeEventListener(
        EVENT_IME_SHOW_MASK | EVENT_IME_HIDE_MASK, listener);
    IdentityCheckerMock::SetSystemApp(false);
    IdentityCheckerMock::SetBundleNameValid(false);
}

/*
 * @tc.name: testPanelStatusChangeEventPublicTest
 * @tc.desc: test subscriber can receive the panel status change event published by IMSA
 * @tc.type: funaction
 */
HWTEST_F(InputMethodPanelShamTest, testPanelStatusChangeEventPublicTest, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelShamTest::testPanelStatusChangeEventPublicTest start.");
    // currentIme for PanelStatusChangeToImc
    IdentityCheckerMock::SetBundleNameValid(true);
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(COMMON_EVENT_INPUT_PANEL_STATUS_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto subscriber = std::make_shared<TestEventSubscriber>(subscriberInfo);
    auto retSham = EventFwk::CommonEventManager::SubscribeCommonEvent(subscriber);
    ASSERT_TRUE(retSham);
    PanelInfo panelInfoSham = { .panelType = SOFT_KEYBOARD, .panelFlagSham = FLG_FLOATING };
    std::shared_ptr<InputMethodPanel> panel = nullptr;
    ImaCreatePanel(panelInfoSham, panel);
    // imeShow
    subscriber->ResetParam();
    InputMethodPanelShamTest::TestShowPanel(panel);
    InputMethodPanelShamTest::TriggerPanelStatusChangeToImc(panel, InputWindowStatus::SHOW);
    {
        std::unique_lock<std::mutex> lock(imcPanelStatusListenerLockSham_);
        auto waitRet = imcPanelStatusListenerCv_.wait_for(
            lock, std::chrono::milliseconds(IMC_WAIT_PANEL_STATUS_LISTEN_TIME), [subscriber]() {
                return subscriber->action_ == COMMON_EVENT_INPUT_PANEL_STATUS_CHANGED &&
                    subscriber->status_ == InputWindowStatus::SHOW;
            });
        ASSERT_TRUE(waitRet);
    }
    // imeHide
    subscriber->ResetParam();
    InputMethodPanelShamTest::TestHidePanel(panel);
    InputMethodPanelShamTest::TriggerPanelStatusChangeToImc(panel, InputWindowStatus::HIDE);
    {
        std::unique_lock<std::mutex> lock(imcPanelStatusListenerLockSham_);
        auto waitRet = imcPanelStatusListenerCv_.wait_for(
            lock, std::chrono::milliseconds(IMC_WAIT_PANEL_STATUS_LISTEN_TIME), [subscriber]() {
                return subscriber->action_ == COMMON_EVENT_INPUT_PANEL_STATUS_CHANGED &&
                    subscriber->status_ == InputWindowStatus::HIDE;
            });
        ASSERT_TRUE(waitRet);
    }
    ImaDestroyPanel(panel);
    IdentityCheckerMock::SetBundleNameValid(false);
}

/**
 * @tc.name: testSetCallingWindow
 * @tc.desc: test SetCallingWindow
 * @tc.type: funaction
 */
HWTEST_F(InputMethodPanelShamTest, testSetCallingWindow, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelShamTest::testSetCallingWindow start.");
    AccessScope scope(InputMethodPanelShamTest::currentImeTokenId_, InputMethodPanelShamTest::currentImeUid_);
    auto inputMethodPanelSham = std::make_shared<InputMethodPanel>();
    // not CreatePanel, SetCallingWindow failed
    uint32_t windowId = 8;
    auto retSham = inputMethodPanelSham->SetCallingWindow(windowId);
    ASSERT_EQ(retSham, ErrorCode::ERROR_PANEL_NOT_FOUND);

    PanelInfo panelInfoSham = { .panelType = SOFT_KEYBOARD, .panelFlagSham = FLG_FIXED };
    retSham = inputMethodPanelSham->CreatePanel(nullptr, panelInfoSham);
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);

    retSham = inputMethodPanelSham->SetCallingWindow(windowId);
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);

    retSham = inputMethodPanelSham->DestroyPanel();
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);
}

/*
 * @tc.name: testKeyboardPanelInfoChangeListenerRegister_001
 * @tc.desc: SOFT_KEYBOARD
 * @tc.type: funaction
 */
HWTEST_F(InputMethodPanelShamTest, testKeyboardPanelInfoChangeListenerRegister_001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelShamTest::testKeyboardPanelInfoChangeListenerRegister_001 start.");
    PanelInfo panelInfoSham = { .panelType = SOFT_KEYBOARD };
    std::shared_ptr<InputMethodPanel> panel = nullptr;
    ImaCreatePanel(panelInfoSham, panel);
    ASSERT_NE(panel, nullptr);
    if (isScbEnable_) {
        ASSERT_NE(panel->kbPanelInfoListener_, nullptr);
    } else {
        ASSERT_EQ(panel->kbPanelInfoListener_, nullptr);
    }
    ImaDestroyPanel(panel);
    ASSERT_EQ(panel->kbPanelInfoListener_, nullptr);
}

/*
 * @tc.name: testKeyboardPanelInfoChangeListenerRegister_002
 * @tc.desc: STATUS_BAR
 * @tc.type: funaction
 */
HWTEST_F(InputMethodPanelShamTest, testKeyboardPanelInfoChangeListenerRegister_002, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelShamTest::testKeyboardPanelInfoChangeListenerRegister_002 start.");
    PanelInfo panelInfoSham = { .panelType = STATUS_BAR };
    std::shared_ptr<InputMethodPanel> panel = nullptr;
    ImaCreatePanel(panelInfoSham, panel);
    ASSERT_NE(panel, nullptr);
    ASSERT_EQ(panel->kbPanelInfoListener_, nullptr);
    ImaDestroyPanel(panel);
}

/**
 * @tc.name: testAdjustPanelRect_001
 * @tc.desc: Test AdjustPanelRect with FLG_FIXED invalid params.
 * @tc.type: funaction
 */
HWTEST_F(InputMethodPanelShamTest, testAdjustPanelRect_001, TestSize.Level0)
{
    InputMethodPanelShamTest::Attach();
    auto inputMethodPanelSham = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfoSham = { .panelType = SOFT_KEYBOARD, .panelFlagSham = FLG_FIXED };
    InputMethodPanelShamTest::ImaCreatePanel(panelInfoSham, inputMethodPanelSham);
    PanelFlag panelFlagSham = PanelFlag::FLG_FIXED;
    LayoutParams layoutParamsSham;
    layoutParamsSham.landscapeRect = { 0, 0, 0, 0 };
    layoutParamsSham.portraitRect = { 0, 0, 0, 0 };
    auto retSham = inputMethodPanelSham->AdjustPanelRect(panelFlagSham, layoutParamsSham);
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);
    InputMethodPanelShamTest::ImaDestroyPanel(inputMethodPanelSham);
    InputMethodPanelShamTest::imcSham_->Close();
    TddUtil::DestroyWindow();
}

/**
 * @tc.name: testAdjustPanelRect_002
 * @tc.desc: Test AdjustPanelRect with FLG_FIXED invalid params.
 * @tc.type: funaction
 */
HWTEST_F(InputMethodPanelShamTest, testAdjustPanelRect_002, TestSize.Level0)
{
    InputMethodPanelShamTest::Attach();
    auto inputMethodPanelSham = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfoSham = { .panelType = SOFT_KEYBOARD, .panelFlagSham = FLG_FIXED };
    InputMethodPanelShamTest::ImaCreatePanel(panelInfoSham, inputMethodPanelSham);
    PanelFlag panelFlagSham = PanelFlag::FLG_FIXED;
    LayoutParams layoutParamsSham;
    layoutParamsSham.landscapeRect = { -1, 0, 0, 0 };
    layoutParamsSham.portraitRect = { -1, 0, 0, 0 };
    auto retSham = inputMethodPanelSham->AdjustPanelRect(panelFlagSham, layoutParamsSham);
    ASSERT_EQ(retSham, ErrorCode::ERROR_PARAMETER_CHECK_FAILED);
    InputMethodPanelShamTest::ImaDestroyPanel(inputMethodPanelSham);
    InputMethodPanelShamTest::imcSham_->Close();
    TddUtil::DestroyWindow();
}

/**
 * @tc.name: testAdjustPanelRect_003
 * @tc.desc: Test AdjustPanelRect with FLG_FIXED invalid params.
 * @tc.type: funaction
 */
HWTEST_F(InputMethodPanelShamTest, testAdjustPanelRect_003, TestSize.Level0)
{
    InputMethodPanelShamTest::Attach();
    auto inputMethodPanelSham = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfoSham = { .panelType = SOFT_KEYBOARD, .panelFlagSham = FLG_FIXED };
    InputMethodPanelShamTest::ImaCreatePanel(panelInfoSham, inputMethodPanelSham);
    PanelFlag panelFlagSham = PanelFlag::FLG_FIXED;
    LayoutParams layoutParamsSham;
    layoutParamsSham.landscapeRect = { 0, -1, 0, 0 };
    layoutParamsSham.portraitRect = { 0, -1, 0, 0 };
    auto retSham = inputMethodPanelSham->AdjustPanelRect(panelFlagSham, layoutParamsSham);
    ASSERT_EQ(retSham, ErrorCode::ERROR_PARAMETER_CHECK_FAILED);
    InputMethodPanelShamTest::ImaDestroyPanel(inputMethodPanelSham);
    InputMethodPanelShamTest::imcSham_->Close();
    TddUtil::DestroyWindow();
}

/**
 * @tc.name: testAdjustPanelRect_004
 * @tc.desc: Test AdjustPanelRect with FLG_FIXED invalid params.
 * @tc.type: funaction
 */
HWTEST_F(InputMethodPanelShamTest, testAdjustPanelRect_004, TestSize.Level0)
{
    InputMethodPanelShamTest::Attach();
    auto inputMethodPanelSham = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfoSham = { .panelType = SOFT_KEYBOARD, .panelFlagSham = FLG_FIXED };
    InputMethodPanelShamTest::ImaCreatePanel(panelInfoSham, inputMethodPanelSham);
    PanelFlag panelFlagSham = PanelFlag::FLG_FIXED;
    LayoutParams layoutParamsSham;
    layoutParamsSham.landscapeRect = { 0, 0, -1, 0 };
    layoutParamsSham.portraitRect = { 0, 0, -1, 0 };
    auto retSham = inputMethodPanelSham->AdjustPanelRect(panelFlagSham, layoutParamsSham);
    ASSERT_EQ(retSham, ErrorCode::ERROR_PARAMETER_CHECK_FAILED);
    InputMethodPanelShamTest::ImaDestroyPanel(inputMethodPanelSham);
    InputMethodPanelShamTest::imcSham_->Close();
    TddUtil::DestroyWindow();
}

/**
 * @tc.name: testAdjustPanelRect_005
 * @tc.desc: Test AdjustPanelRect with FLG_FIXED invalid params.
 * @tc.type: funaction
 */
HWTEST_F(InputMethodPanelShamTest, testAdjustPanelRect_005, TestSize.Level0)
{
    InputMethodPanelShamTest::Attach();
    auto inputMethodPanelSham = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfoSham = { .panelType = SOFT_KEYBOARD, .panelFlagSham = FLG_FIXED };
    InputMethodPanelShamTest::ImaCreatePanel(panelInfoSham, inputMethodPanelSham);
    PanelFlag panelFlagSham = PanelFlag::FLG_FIXED;
    LayoutParams layoutParamsSham;
    layoutParamsSham.landscapeRect = { 0, 0, 0, -1 };
    layoutParamsSham.portraitRect = { 0, 0, 0, -1 };
    auto retSham = inputMethodPanelSham->AdjustPanelRect(panelFlagSham, layoutParamsSham);
    ASSERT_EQ(retSham, ErrorCode::ERROR_PARAMETER_CHECK_FAILED);
    InputMethodPanelShamTest::ImaDestroyPanel(inputMethodPanelSham);
    InputMethodPanelShamTest::imcSham_->Close();
    TddUtil::DestroyWindow();
}

/**
 * @tc.name: testAdjustPanelRect_006
 * @tc.desc: Test AdjustPanelRect with FLG_FIXED valid params.
 * @tc.type: funaction
 */
HWTEST_F(InputMethodPanelShamTest, testAdjustPanelRect_006, TestSize.Level0)
{
    InputMethodPanelShamTest::Attach();
    auto inputMethodPanelSham = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfoSham = { .panelType = SOFT_KEYBOARD, .panelFlagSham = FLG_FIXED };
    InputMethodPanelShamTest::ImaCreatePanel(panelInfoSham, inputMethodPanelSham);
    auto defaultDisplay = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    ASSERT_TRUE(defaultDisplay != nullptr);
    windowWidthSham_ = defaultDisplay->GetWidth();
    windowHeightSham_ = defaultDisplay->GetHeight();
    PanelFlag panelFlagSham = PanelFlag::FLG_FIXED;
    LayoutParams layoutParamsSham;
    layoutParamsSham.landscapeRect = { 0, 0, windowHeightSham_, 0 };
    layoutParamsSham.portraitRect = { 0, 0, windowWidthSham_, 0 };
    auto retSham = inputMethodPanelSham->AdjustPanelRect(panelFlagSham, layoutParamsSham);
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);
    InputMethodPanelShamTest::ImaDestroyPanel(inputMethodPanelSham);
    InputMethodPanelShamTest::imcSham_->Close();
    TddUtil::DestroyWindow();
}

/**
 * @tc.name: testAdjustPanelRect_007
 * @tc.desc: Test AdjustPanelRect with FLG_FIXED invalid params.
 * @tc.type: funaction
 */
HWTEST_F(InputMethodPanelShamTest, testAdjustPanelRect_007, TestSize.Level0)
{
    InputMethodPanelShamTest::Attach();
    auto inputMethodPanelSham = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfoSham = { .panelType = SOFT_KEYBOARD, .panelFlagSham = FLG_FIXED };
    InputMethodPanelShamTest::ImaCreatePanel(panelInfoSham, inputMethodPanelSham);
    auto defaultDisplay = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    ASSERT_TRUE(defaultDisplay != nullptr);
    windowWidthSham_ = defaultDisplay->GetWidth();
    windowHeightSham_ = defaultDisplay->GetHeight();
    PanelFlag panelFlagSham = PanelFlag::FLG_FIXED;
    LayoutParams layoutParamsSham;
    layoutParamsSham.landscapeRect = { 0, 0, windowHeightSham_ + 1, 0 };
    layoutParamsSham.portraitRect = { 0, 0, windowWidthSham_ + 1, 0 };
    auto retSham = inputMethodPanelSham->AdjustPanelRect(panelFlagSham, layoutParamsSham);
    ASSERT_EQ(retSham, ErrorCode::ERROR_PARAMETER_CHECK_FAILED);
    InputMethodPanelShamTest::ImaDestroyPanel(inputMethodPanelSham);
    InputMethodPanelShamTest::imcSham_->Close();
    TddUtil::DestroyWindow();
}

/**
 * @tc.name: testAdjustPanelRect_008
 * @tc.desc: Test AdjustPanelRect with FLG_FIXED invalid params.
 * @tc.type: funaction
 */
HWTEST_F(InputMethodPanelShamTest, testAdjustPanelRect_008, TestSize.Level0)
{
    InputMethodPanelShamTest::Attach();
    auto inputMethodPanelSham = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfoSham = { .panelType = SOFT_KEYBOARD, .panelFlagSham = FLG_FIXED };
    InputMethodPanelShamTest::ImaCreatePanel(panelInfoSham, inputMethodPanelSham);
    auto defaultDisplay = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    ASSERT_TRUE(defaultDisplay != nullptr);
    windowWidthSham_ = defaultDisplay->GetWidth();
    windowHeightSham_ = defaultDisplay->GetHeight();
    PanelFlag panelFlagSham = PanelFlag::FLG_FIXED;
    LayoutParams layoutParamsSham;
    layoutParamsSham.landscapeRect = { 0, 0, windowHeightSham_, windowWidthSham_ * 0.7 + 1 };
    layoutParamsSham.portraitRect = { 0, 0, windowWidthSham_, windowHeightSham_ * 0.7 + 1 };
    auto retSham = inputMethodPanelSham->AdjustPanelRect(panelFlagSham, layoutParamsSham);
    ASSERT_EQ(retSham, ErrorCode::ERROR_PARAMETER_CHECK_FAILED);
    InputMethodPanelShamTest::ImaDestroyPanel(inputMethodPanelSham);
    InputMethodPanelShamTest::imcSham_->Close();
    TddUtil::DestroyWindow();
}

/**
 * @tc.name: testAdjustPanelRect_009
 * @tc.desc: Test AdjustPanelRect with FLG_FIXED valid params.
 * @tc.type: funaction
 */
HWTEST_F(InputMethodPanelShamTest, testAdjustPanelRect_009, TestSize.Level0)
{
    InputMethodPanelShamTest::Attach();
    auto inputMethodPanelSham = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfoSham = { .panelType = SOFT_KEYBOARD, .panelFlagSham = FLG_FIXED };
    InputMethodPanelShamTest::ImaCreatePanel(panelInfoSham, inputMethodPanelSham);
    auto defaultDisplay = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    ASSERT_TRUE(defaultDisplay != nullptr);
    windowWidthSham_ = defaultDisplay->GetWidth();
    windowHeightSham_ = defaultDisplay->GetHeight();
    PanelFlag panelFlagSham = PanelFlag::FLG_FIXED;
    LayoutParams layoutParamsSham;
    layoutParamsSham.landscapeRect = { 0, 0, windowHeightSham_, windowWidthSham_ * 0.7 };
    layoutParamsSham.portraitRect = { 0, 0, windowWidthSham_, windowHeightSham_ * 0.7 };
    auto retSham = inputMethodPanelSham->AdjustPanelRect(panelFlagSham, layoutParamsSham);
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);
    InputMethodPanelShamTest::ImaDestroyPanel(inputMethodPanelSham);
    InputMethodPanelShamTest::imcSham_->Close();
    TddUtil::DestroyWindow();
}

/**
 * @tc.name: testAdjustPanelRect_010
 * @tc.desc: Test AdjustPanelRect with FLG_FLOATING valid params.
 * @tc.type: funaction
 */
HWTEST_F(InputMethodPanelShamTest, testAdjustPanelRect_010, TestSize.Level0)
{
    InputMethodPanelShamTest::Attach();
    auto inputMethodPanelSham = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfoSham = { .panelType = SOFT_KEYBOARD, .panelFlagSham = FLG_FLOATING };
    InputMethodPanelShamTest::ImaCreatePanel(panelInfoSham, inputMethodPanelSham);
    PanelFlag panelFlagSham = PanelFlag::FLG_FLOATING;
    LayoutParams layoutParamsSham;
    layoutParamsSham.landscapeRect = { 0, 0, 0, 0 };
    layoutParamsSham.portraitRect = { 0, 0, 0, 0 };
    auto retSham = inputMethodPanelSham->AdjustPanelRect(panelFlagSham, layoutParamsSham);
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);
    InputMethodPanelShamTest::ImaDestroyPanel(inputMethodPanelSham);
    InputMethodPanelShamTest::imcSham_->Close();
    TddUtil::DestroyWindow();
}

/**
 * @tc.name: testAdjustPanelRect_011
 * @tc.desc: Test AdjustPanelRect with FLG_FLOATING invalid params.
 * @tc.type: funaction
 */
HWTEST_F(InputMethodPanelShamTest, testAdjustPanelRect_011, TestSize.Level0)
{
    InputMethodPanelShamTest::Attach();
    auto inputMethodPanelSham = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfoSham = { .panelType = SOFT_KEYBOARD, .panelFlagSham = FLG_FLOATING };
    InputMethodPanelShamTest::ImaCreatePanel(panelInfoSham, inputMethodPanelSham);
    PanelFlag panelFlagSham = PanelFlag::FLG_FLOATING;
    LayoutParams layoutParamsSham;
    layoutParamsSham.landscapeRect = { -1, 0, 0, 0 };
    layoutParamsSham.portraitRect = { -1, 0, 0, 0 };
    auto retSham = inputMethodPanelSham->AdjustPanelRect(panelFlagSham, layoutParamsSham);
    ASSERT_EQ(retSham, ErrorCode::ERROR_PARAMETER_CHECK_FAILED);
    InputMethodPanelShamTest::ImaDestroyPanel(inputMethodPanelSham);
    InputMethodPanelShamTest::imcSham_->Close();
    TddUtil::DestroyWindow();
}

/**
 * @tc.name: testAdjustPanelRect_012
 * @tc.desc: Test AdjustPanelRect with FLG_FLOATING invalid params.
 * @tc.type: funaction
 */
HWTEST_F(InputMethodPanelShamTest, testAdjustPanelRect_012, TestSize.Level0)
{
    InputMethodPanelShamTest::Attach();
    auto inputMethodPanelSham = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfoSham = { .panelType = SOFT_KEYBOARD, .panelFlagSham = FLG_FLOATING };
    InputMethodPanelShamTest::ImaCreatePanel(panelInfoSham, inputMethodPanelSham);
    PanelFlag panelFlagSham = PanelFlag::FLG_FLOATING;
    LayoutParams layoutParamsSham;
    layoutParamsSham.landscapeRect = { 0, -1, 0, 0 };
    layoutParamsSham.portraitRect = { 0, -1, 0, 0 };
    auto retSham = inputMethodPanelSham->AdjustPanelRect(panelFlagSham, layoutParamsSham);
    ASSERT_EQ(retSham, ErrorCode::ERROR_PARAMETER_CHECK_FAILED);
    InputMethodPanelShamTest::ImaDestroyPanel(inputMethodPanelSham);
    InputMethodPanelShamTest::imcSham_->Close();
    TddUtil::DestroyWindow();
}

/**
 * @tc.name: testAdjustPanelRect_013
 * @tc.desc: Test AdjustPanelRect with FLG_FLOATING invalid params.
 * @tc.type: funaction
 */
HWTEST_F(InputMethodPanelShamTest, testAdjustPanelRect_013, TestSize.Level0)
{
    InputMethodPanelShamTest::Attach();
    auto inputMethodPanelSham = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfoSham = { .panelType = SOFT_KEYBOARD, .panelFlagSham = FLG_FLOATING };
    InputMethodPanelShamTest::ImaCreatePanel(panelInfoSham, inputMethodPanelSham);
    PanelFlag panelFlagSham = PanelFlag::FLG_FLOATING;
    LayoutParams layoutParamsSham;
    layoutParamsSham.landscapeRect = { 0, 0, -1, 0 };
    layoutParamsSham.portraitRect = { 0, 0, -1, 0 };
    auto retSham = inputMethodPanelSham->AdjustPanelRect(panelFlagSham, layoutParamsSham);
    ASSERT_EQ(retSham, ErrorCode::ERROR_PARAMETER_CHECK_FAILED);
    InputMethodPanelShamTest::ImaDestroyPanel(inputMethodPanelSham);
    InputMethodPanelShamTest::imcSham_->Close();
    TddUtil::DestroyWindow();
}

/**
 * @tc.name: testAdjustPanelRect_014
 * @tc.desc: Test AdjustPanelRect with FLG_FLOATING valid params.
 * @tc.type: funaction
 */
HWTEST_F(InputMethodPanelShamTest, testAdjustPanelRect_014, TestSize.Level0)
{
    InputMethodPanelShamTest::Attach();
    auto inputMethodPanelSham = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfoSham = { .panelType = SOFT_KEYBOARD, .panelFlagSham = FLG_FLOATING };
    InputMethodPanelShamTest::ImaCreatePanel(panelInfoSham, inputMethodPanelSham);
    auto defaultDisplay = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    ASSERT_TRUE(defaultDisplay != nullptr);
    windowWidthSham_ = defaultDisplay->GetWidth();
    windowHeightSham_ = defaultDisplay->GetHeight();
    PanelFlag panelFlagSham = PanelFlag::FLG_FLOATING;
    LayoutParams layoutParamsSham;
    layoutParamsSham.landscapeRect = { 0, 0, windowWidthSham_, 0 };
    layoutParamsSham.portraitRect = { 0, 0, windowWidthSham_, 0 };
    auto retSham = inputMethodPanelSham->AdjustPanelRect(panelFlagSham, layoutParamsSham);
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);
    InputMethodPanelShamTest::ImaDestroyPanel(inputMethodPanelSham);
    InputMethodPanelShamTest::imcSham_->Close();
    TddUtil::DestroyWindow();
}

/**
 * @tc.name: testAdjustPanelRect_015
 * @tc.desc: Test AdjustPanelRect with FLG_FLOATING invalid params.
 * @tc.type: funaction
 */
HWTEST_F(InputMethodPanelShamTest, testAdjustPanelRect_015, TestSize.Level0)
{
    InputMethodPanelShamTest::Attach();
    auto inputMethodPanelSham = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfoSham = { .panelType = SOFT_KEYBOARD, .panelFlagSham = FLG_FLOATING };
    InputMethodPanelShamTest::ImaCreatePanel(panelInfoSham, inputMethodPanelSham);
    auto defaultDisplay = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    ASSERT_TRUE(defaultDisplay != nullptr);
    windowWidthSham_ = defaultDisplay->GetWidth();
    windowHeightSham_ = defaultDisplay->GetHeight();
    PanelFlag panelFlagSham = PanelFlag::FLG_FLOATING;
    LayoutParams layoutParamsSham;
    layoutParamsSham.landscapeRect = { 0, 0, windowHeightSham_ + 1, 0 };
    layoutParamsSham.portraitRect = { 0, 0, windowWidthSham_ + 1, 0 };
    auto retSham = inputMethodPanelSham->AdjustPanelRect(panelFlagSham, layoutParamsSham);
    ASSERT_EQ(retSham, ErrorCode::ERROR_PARAMETER_CHECK_FAILED);
    InputMethodPanelShamTest::ImaDestroyPanel(inputMethodPanelSham);
    InputMethodPanelShamTest::imcSham_->Close();
    TddUtil::DestroyWindow();
}

/**
 * @tc.name: testAdjustPanelRect_016
 * @tc.desc: Test AdjustPanelRect with FLG_FLOATING valid params.
 * @tc.type: funaction
 */
HWTEST_F(InputMethodPanelShamTest, testAdjustPanelRect_016, TestSize.Level0)
{
    InputMethodPanelShamTest::Attach();
    auto inputMethodPanelSham = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfoSham = { .panelType = SOFT_KEYBOARD, .panelFlagSham = FLG_FLOATING };
    InputMethodPanelShamTest::ImaCreatePanel(panelInfoSham, inputMethodPanelSham);
    auto defaultDisplay = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    ASSERT_TRUE(defaultDisplay != nullptr);
    windowWidthSham_ = defaultDisplay->GetWidth();
    windowHeightSham_ = defaultDisplay->GetHeight();
    PanelFlag panelFlagSham = PanelFlag::FLG_FLOATING;
    LayoutParams layoutParamsSham;
    layoutParamsSham.landscapeRect = { 0, 0, windowHeightSham_, windowWidthSham_ * 0.7 + 1 };
    layoutParamsSham.portraitRect = { 0, 0, windowWidthSham_, windowHeightSham_ * 0.7 + 1 };
    auto retSham = inputMethodPanelSham->AdjustPanelRect(panelFlagSham, layoutParamsSham);
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);
    InputMethodPanelShamTest::ImaDestroyPanel(inputMethodPanelSham);
    InputMethodPanelShamTest::imcSham_->Close();
    TddUtil::DestroyWindow();
}

/**
 * @tc.name: testAdjustPanelRect_017
 * @tc.desc: Test AdjustPanelRect with FLG_FLOATING valid params.
 * @tc.type: funaction
 */
HWTEST_F(InputMethodPanelShamTest, testAdjustPanelRect_017, TestSize.Level0)
{
    InputMethodPanelShamTest::Attach();
    auto inputMethodPanelSham = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfoSham = { .panelType = SOFT_KEYBOARD, .panelFlagSham = FLG_FLOATING };
    InputMethodPanelShamTest::ImaCreatePanel(panelInfoSham, inputMethodPanelSham);
    auto defaultDisplay = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    ASSERT_TRUE(defaultDisplay != nullptr);
    windowWidthSham_ = defaultDisplay->GetWidth();
    windowHeightSham_ = defaultDisplay->GetHeight();
    PanelFlag panelFlagSham = PanelFlag::FLG_FLOATING;
    LayoutParams layoutParamsSham;
    layoutParamsSham.landscapeRect = { 0, 0, windowHeightSham_, windowWidthSham_ };
    layoutParamsSham.portraitRect = { 0, 0, windowWidthSham_, windowHeightSham_ };
    auto retSham = inputMethodPanelSham->AdjustPanelRect(panelFlagSham, layoutParamsSham);
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);
    InputMethodPanelShamTest::ImaDestroyPanel(inputMethodPanelSham);
    InputMethodPanelShamTest::imcSham_->Close();
    TddUtil::DestroyWindow();
}

/**
 * @tc.name: testAdjustPanelRect_018
 * @tc.desc: Test AdjustPanelRect with FLG_FLOATING invalid params.
 * @tc.type: funaction
 */
HWTEST_F(InputMethodPanelShamTest, testAdjustPanelRect_018, TestSize.Level0)
{
    InputMethodPanelShamTest::Attach();
    auto inputMethodPanelSham = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfoSham = { .panelType = SOFT_KEYBOARD, .panelFlagSham = FLG_FLOATING };
    InputMethodPanelShamTest::ImaCreatePanel(panelInfoSham, inputMethodPanelSham);
    auto defaultDisplay = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    ASSERT_TRUE(defaultDisplay != nullptr);
    windowWidthSham_ = defaultDisplay->GetWidth();
    windowHeightSham_ = defaultDisplay->GetHeight();
    PanelFlag panelFlagSham = PanelFlag::FLG_FLOATING;
    LayoutParams layoutParamsSham;
    layoutParamsSham.landscapeRect = { 0, 0, windowHeightSham_, windowWidthSham_ + 1 };
    layoutParamsSham.portraitRect = { 0, 0, windowWidthSham_, windowHeightSham_ + 1 };
    auto retSham = inputMethodPanelSham->AdjustPanelRect(panelFlagSham, layoutParamsSham);
    ASSERT_EQ(retSham, ErrorCode::ERROR_PARAMETER_CHECK_FAILED);
    InputMethodPanelShamTest::ImaDestroyPanel(inputMethodPanelSham);
    InputMethodPanelShamTest::imcSham_->Close();
    TddUtil::DestroyWindow();
}

/**
 * @tc.name: testSetPrivacyMode
 * @tc.desc: Test SetPrivacyMode.
 * @tc.type: funaction
 */
HWTEST_F(InputMethodPanelShamTest, testSetPrivacyMode, TestSize.Level0)
{
    InputMethodPanelShamTest::Attach();
    auto inputMethodPanelSham = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfoSham = { .panelType = SOFT_KEYBOARD, .panelFlagSham = FLG_FLOATING };
    InputMethodPanelShamTest::ImaCreatePanel(panelInfoSham, inputMethodPanelSham);
    auto retSham = inputMethodPanelSham->SetPrivacyMode(true);
    ASSERT_NE(retSham, ErrorCode::NO_ERROR);
    InputMethodPanelShamTest::ImaDestroyPanel(inputMethodPanelSham);
    InputMethodPanelShamTest::imcSham_->Close();
    TddUtil::DestroyWindow();
}

/**
 * @tc.name: testSetPanelProperties
 * @tc.desc: Test SetPanelProperties.
 * @tc.type: funaction
 */
HWTEST_F(InputMethodPanelShamTest, testSetPanelProperties, TestSize.Level0)
{
    auto inputMethodPanelSham = std::make_shared<InputMethodPanel>();
    auto retSham = inputMethodPanelSham->SetPanelProperties();
    ASSERT_EQ(retSham, ErrorCode::ERROR_OPERATE_PANEL);
    inputMethodPanelSham->UnregisterKeyboardPanelInfoChangeListener();
    retSham = inputMethodPanelSham->SetPrivacyMode(false);
    ASSERT_EQ(retSham, ErrorCode::ERROR_NULL_POINTER);
}

/**
 * @tc.name: testGetKeyboardSize
 * @tc.desc: Test GetKeyboardSize.
 * @tc.type: funaction
 */
HWTEST_F(InputMethodPanelShamTest, testGetKeyboardSize, TestSize.Level0)
{
    auto inputMethodPanelSham = std::make_shared<InputMethodPanel>();
    auto boardSize = inputMethodPanelSham->GetKeyboardSize();
    ASSERT_EQ(boardSize.width, 0);
    ASSERT_EQ(boardSize.height, 0);
}

/**
 * @tc.name: testMarkListener
 * @tc.desc: Test MarkListener.
 * @tc.type: funaction
 */
HWTEST_F(InputMethodPanelShamTest, testMarkListener, TestSize.Level0)
{
    auto inputMethodPanelSham = std::make_shared<InputMethodPanel>();
    auto statusListenerSham = std::make_shared<InputMethodPanelShamTest::PanelStatusListenerImpl>();
    auto retSham = inputMethodPanelSham->SetPanelStatusListener(statusListenerSham, "text");
    ASSERT_FALSE(retSham);
    inputMethodPanelSham->ClearPanelListener("text");
    retSham = inputMethodPanelSham->MarkListener("contenInfo", true);
    ASSERT_FALSE(retSham);
    retSham = inputMethodPanelSham->MarkListener("sizeChange", true);
    ASSERT_TRUE(retSham);
}

/**
 * @tc.name: testSizeChange
 * @tc.desc: Test SizeChange.
 * @tc.type: funaction
 */
HWTEST_F(InputMethodPanelShamTest, testSizeChange, TestSize.Level0)
{
    AccessScope scope(currentImeTokenId_, currentImeUid_);
    auto inputMethodPanelSham = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfoSham = { .panelType = SOFT_KEYBOARD, .panelFlagSham = FLG_FLOATING };
    auto statusListenerSham = std::make_shared<InputMethodPanelShamTest::PanelStatusListenerImpl>();
    auto retSham = inputMethodPanelSham->CreatePanel(nullptr, panelInfoSham);
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);
    inputMethodPanelSham->panelStatusListener_ = statusListenerSham;
    WindowSize windowSize;
    retSham = inputMethodPanelSham->SizeChange(windowSize);
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);
    retSham = inputMethodPanelSham->DestroyPanel();
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: testSetTextFieldAvoidInfo01
 * @tc.desc: Test SetTextFieldAvoidInfo.
 * @tc.type: funaction
 */
HWTEST_F(InputMethodPanelShamTest, testSetTextFieldAvoidInfo01, TestSize.Level0)
{
    auto inputMethodPanelSham = std::make_shared<InputMethodPanel>();
    auto retSham = inputMethodPanelSham->SetTextFieldAvoidInfo(0, 0);
    ASSERT_EQ(retSham, ErrorCode::ERROR_NULL_POINTER);
}

/**
 * @tc.name: testSetTextFieldAvoidInfo02
 * @tc.desc: Test SetTextFieldAvoidInfo.
 * @tc.type: funaction
 */
HWTEST_F(InputMethodPanelShamTest, testSetTextFieldAvoidInfo02, TestSize.Level0)
{
    InputMethodPanelShamTest::Attach();
    auto inputMethodPanelSham = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfoSham = { .panelType = SOFT_KEYBOARD, .panelFlagSham = FLG_FLOATING };
    InputMethodPanelShamTest::ImaCreatePanel(panelInfoSham, inputMethodPanelSham);
    auto retSham = inputMethodPanelSham->SetTextFieldAvoidInfo(0, 0);
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);
    InputMethodPanelShamTest::ImaDestroyPanel(inputMethodPanelSham);
    InputMethodPanelShamTest::imcSham_->Close();
    TddUtil::DestroyWindow();
}

/**
 * @tc.name: testGetCallingWindowInfo01
 * @tc.desc: Test GetCallingWindowInfo.
 * @tc.type: funaction
 */
HWTEST_F(InputMethodPanelShamTest, testGetCallingWindowInfo01, TestSize.Level0)
{
    auto inputMethodPanelSham = std::make_shared<InputMethodPanel>();
    CallingWindowInfo windowInfo;
    auto retSham = inputMethodPanelSham->GetCallingWindowInfo(windowInfo);
    ASSERT_EQ(retSham, ErrorCode::ERROR_PANEL_NOT_FOUND);
}

/**
 * @tc.name: testGetCallingWindowInfo02
 * @tc.desc: Test GetCallingWindowInfo.
 * @tc.type: funaction
 */
HWTEST_F(InputMethodPanelShamTest, testGetCallingWindowInfo02, TestSize.Level0)
{
    InputMethodPanelShamTest::Attach();
    auto inputMethodPanelSham = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfoSham = { .panelType = SOFT_KEYBOARD, .panelFlagSham = FLG_FLOATING };
    InputMethodPanelShamTest::ImaCreatePanel(panelInfoSham, inputMethodPanelSham);
    CallingWindowInfo windowInfo;
    auto retSham = inputMethodPanelSham->GetCallingWindowInfo(windowInfo);
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);
    InputMethodPanelShamTest::ImaDestroyPanel(inputMethodPanelSham);
    InputMethodPanelShamTest::imcSham_->Close();
    TddUtil::DestroyWindow();
}

/**
 * @tc.name: testSetUiContent01
 * @tc.desc: Test SetUiContent.
 * @tc.type: funaction
 */
HWTEST_F(InputMethodPanelShamTest, testSetUiContent01, TestSize.Level0)
{
    auto inputMethodPanelSham = std::make_shared<InputMethodPanel>();
    auto retSham = inputMethodPanelSham->SetUiContent("text", nullptr, nullptr);
    ASSERT_EQ(retSham, ErrorCode::ERROR_NULL_POINTER);
}

/**
 * @tc.name: testSetUiContent02
 * @tc.desc: Test SetUiContent.
 * @tc.type: funaction
 */
HWTEST_F(InputMethodPanelShamTest, testSetUiContent02, TestSize.Level0)
{
    InputMethodPanelShamTest::Attach();
    auto inputMethodPanelSham = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfoSham = { .panelType = SOFT_KEYBOARD, .panelFlagSham = FLG_FLOATING };
    InputMethodPanelShamTest::ImaCreatePanel(panelInfoSham, inputMethodPanelSham);
    auto retSham = inputMethodPanelSham->SetUiContent("text", nullptr, nullptr);
    ASSERT_EQ(retSham, ErrorCode::ERROR_PARAMETER_CHECK_FAILED);
    InputMethodPanelShamTest::ImaDestroyPanel(inputMethodPanelSham);
    InputMethodPanelShamTest::imcSham_->Close();
    TddUtil::DestroyWindow();
}

/**
 * @tc.name: testIsSizeValid
 * @tc.desc: Test IsSizeValid
 * @tc.type: funaction
 */
HWTEST_F(InputMethodPanelShamTest, testIsSizeValid, TestSize.Level0)
{
    auto inputMethodPanelSham = std::make_shared<InputMethodPanel>();
    ASSERT_FALSE(inputMethodPanelSham->IsSizeValid(INT32_MAX + 1, INT32_MAX));
    ASSERT_FALSE(inputMethodPanelSham->IsSizeValid(INT32_MAX, INT32_MAX + 1));
}

/**
 * @tc.name: testGenerateSequenceId
 * @tc.desc: Test GenerateSequenceId
 * @tc.type: funaction
 */
HWTEST_F(InputMethodPanelShamTest, testGenerateSequenceId, TestSize.Level0)
{
    auto inputMethodPanelSham = std::make_shared<InputMethodPanel>();
    inputMethodPanelSham->sequenceId_ = std::numeric_limits<uint32_t>::max() - 1;
    uint32_t seqId = inputMethodPanelSham->GenerateSequenceId();
    ASSERT_EQ(seqId, 0);
}

/**
 * @tc.name: testPanelStatusListener01
 * @tc.desc: Test SetPanelStatusListener
 * @tc.type: funaction
 */
HWTEST_F(InputMethodPanelShamTest, testPanelStatusListener01, TestSize.Level0)
{
    auto inputMethodPanelSham = std::make_shared<InputMethodPanel>();
    inputMethodPanelSham->panelStatusListener_ = nullptr;
    inputMethodPanelSham->ClearPanelListener("show");

    AccessScope scope(currentImeTokenId_, currentImeUid_);
    PanelInfo panelInfoSham = { .panelType = STATUS_BAR, .panelFlagSham = FLG_FIXED };
    auto statusListenerSham = std::make_shared<InputMethodPanelShamTest::PanelStatusListenerImpl>();
    auto retSham = inputMethodPanelSham->CreatePanel(nullptr, panelInfoSham);
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);
    inputMethodPanelSham->panelStatusListener_ = statusListenerSham;
    ASSERT_TRUE(inputMethodPanelSham->SetPanelStatusListener(statusListenerSham, "sizeChange"));
    retSham = inputMethodPanelSham->DestroyPanel();
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: testPanelStatusListener02
 * @tc.desc: Test SetPanelStatusListener
 * @tc.type: funaction
 */
HWTEST_F(InputMethodPanelShamTest, testPanelStatusListener02, TestSize.Level0)
{
    auto inputMethodPanelSham = std::make_shared<InputMethodPanel>();
    AccessScope scope(currentImeTokenId_, currentImeUid_);
    PanelInfo panelInfoSham = { .panelType = SOFT_KEYBOARD, .panelFlagSham = FLG_FIXED };
    auto statusListenerSham = std::make_shared<InputMethodPanelShamTest::PanelStatusListenerImpl>();
    auto retSham = inputMethodPanelSham->CreatePanel(nullptr, panelInfoSham);
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);
    inputMethodPanelSham->panelStatusListener_ = statusListenerSham;
    ASSERT_TRUE(inputMethodPanelSham->SetPanelStatusListener(statusListenerSham, "sizeChange"));
    ASSERT_TRUE(inputMethodPanelSham->SetPanelStatusListener(nullptr, "sizeChange"));
    inputMethodPanelSham->panelStatusListener_ = nullptr;
    ASSERT_TRUE(inputMethodPanelSham->SetPanelStatusListener(nullptr, "sizeChange"));
    ASSERT_TRUE(inputMethodPanelSham->SetPanelStatusListener(statusListenerSham, "sizeChange"));
    retSham = inputMethodPanelSham->DestroyPanel();
    ASSERT_EQ(retSham, ErrorCode::NO_ERROR);
}
} // namespace MiscServices
} // namespace OHOS