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
#define private public
#define protected public
#include "ime_info_inquirer.h"
#include "input_method_ability.h"
#include "input_method_ability_utils.h"
#include "input_method_controller.h"
#include "input_method_panel.h"
#include "input_method_system_ability.h"
#include "task_manager.h"
#undef private

#include <gtest/gtest.h>
#include <sys/time.h>
#include <unistd.h>

#include <condition_variable>
#include <cstdint>
#include <string>

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
constexpr float FIXED_SOFT_KEYBOARD_PANEL_RATIO = 0.7;
const constexpr char *IMMERSIVE_EFFECT = "immersive_effect";
class InputMethodPanelAdjustTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static std::shared_ptr<InputMethodPanel> CreatePanel();
    static void DestroyPanel(const std::shared_ptr<InputMethodPanel> &panel);
    static void Attach();
    static void ImaCreatePanel(const PanelInfo &info, std::shared_ptr<InputMethodPanel> &panel);
    static void ImaDestroyPanel(const std::shared_ptr<InputMethodPanel> &panel);
    static int32_t GetDisplaySize(DisplaySize &size);
    static void SetImmersiveCapacitySupport(bool isSupport);

    static sptr<InputMethodController> imc_;
    static InputMethodAbility &ima_;
    static sptr<InputMethodSystemAbility> imsa_;
    static int32_t currentImeUid_;
    static uint64_t currentImeTokenId_;
    static sptr<OnTextChangedListener> textListener_;
    static std::shared_ptr<InputMethodEngineListener> imeListener_;
    static bool isScbEnable_;
};
sptr<InputMethodController> InputMethodPanelAdjustTest::imc_{ nullptr };
InputMethodAbility &InputMethodPanelAdjustTest::ima_ = InputMethodAbility::GetInstance();
sptr<InputMethodSystemAbility> InputMethodPanelAdjustTest::imsa_{ nullptr };
uint64_t InputMethodPanelAdjustTest::currentImeTokenId_ = 0;
int32_t InputMethodPanelAdjustTest::currentImeUid_ = 0;
sptr<OnTextChangedListener> InputMethodPanelAdjustTest::textListener_{ nullptr };
std::shared_ptr<InputMethodEngineListener> InputMethodPanelAdjustTest::imeListener_{ nullptr };
bool InputMethodPanelAdjustTest::isScbEnable_{ false };
void InputMethodPanelAdjustTest::SetUpTestCase(void)
{
    IMSA_HILOGI("InputMethodPanelAdjustTest::SetUpTestCase");
    IdentityCheckerMock::ResetParam();
    isScbEnable_ = Rosen::SceneBoardJudgement::IsSceneBoardEnabled();
    // storage current token id
    TddUtil::StorageSelfTokenID();

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
    {
        TokenScope scope(currentImeTokenId_);
        ima_.InitConnect();
    }
    ima_.abilityManager_ = imsa_;
    TddUtil::InitCurrentImePermissionInfo();
    IdentityCheckerMock::SetBundleName(TddUtil::currentBundleNameMock_);
    ima_.SetCoreAndAgent();
    InputMethodPanelAdjustTest::ima_.SetImeListener(imeListener_);

    ImaUtils::abilityManager_ = imsa_;
}

void InputMethodPanelAdjustTest::TearDownTestCase(void)
{
    IMSA_HILOGI("InputMethodPanelAdjustTest::TearDownTestCase");
    TddUtil::RestoreSelfTokenID();
    IdentityCheckerMock::ResetParam();
    imsa_->OnStop();
    ImaUtils::abilityManager_ = nullptr;
}

void InputMethodPanelAdjustTest::SetUp(void)
{
    IMSA_HILOGI("InputMethodPanelAdjustTest::SetUp");
    TaskManager::GetInstance().SetInited(true);
    InputMethodPanelAdjustTest::Attach();
}

void InputMethodPanelAdjustTest::TearDown(void)
{
    InputMethodPanelAdjustTest::imc_->Close();
    TddUtil::DestroyWindow();
    TddUtil::RestoreSelfTokenID();
    IMSA_HILOGI("InputMethodPanelAdjustTest::TearDown");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    TaskManager::GetInstance().Reset();
}

std::shared_ptr<InputMethodPanel> InputMethodPanelAdjustTest::CreatePanel()
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

void InputMethodPanelAdjustTest::DestroyPanel(const std::shared_ptr<InputMethodPanel> &panel)
{
    ASSERT_NE(panel, nullptr);
    AccessScope scope(currentImeTokenId_, currentImeUid_);
    auto ret = panel->DestroyPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

void InputMethodPanelAdjustTest::ImaCreatePanel(const PanelInfo &info, std::shared_ptr<InputMethodPanel> &panel)
{
    AccessScope scope(currentImeTokenId_, currentImeUid_);
    auto ret = ima_.CreatePanel(nullptr, info, panel);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

void InputMethodPanelAdjustTest::ImaDestroyPanel(const std::shared_ptr<InputMethodPanel> &panel)
{
    AccessScope scope(currentImeTokenId_, currentImeUid_);
    auto ret = ima_.DestroyPanel(panel);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

void InputMethodPanelAdjustTest::Attach()
{
    IdentityCheckerMock::SetFocused(true);
    auto ret = imc_->Attach(textListener_, false);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    IdentityCheckerMock::SetFocused(false);
}

int32_t InputMethodPanelAdjustTest::GetDisplaySize(DisplaySize &size)
{
    auto defaultDisplay = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    if (defaultDisplay == nullptr) {
        IMSA_HILOGE("GetDefaultDisplay failed!");
        return ErrorCode::ERROR_WINDOW_MANAGER;
    }
    auto width = defaultDisplay->GetWidth();
    auto height = defaultDisplay->GetHeight();
    if (width < height) {
        size.portrait = { .width = width, .height = height };
        size.landscape = { .width = height, .height = width };
    } else {
        size.portrait = { .width = height, .height = width };
        size.landscape = { .width = width, .height = height };
    }
    return ErrorCode::NO_ERROR;
}

void InputMethodPanelAdjustTest::SetImmersiveCapacitySupport(bool isSupport)
{
    IMSA_HILOGI("InputMethodPanelAdjustTest isSupport: %{public}d", isSupport);
    auto supportedCapacityList = ImeInfoInquirer::GetInstance().GetSystemConfig().supportedCapacityList;
    if (isSupport) {
        supportedCapacityList.insert(IMMERSIVE_EFFECT);
    } else {
        supportedCapacityList.erase(IMMERSIVE_EFFECT);
    }
    ImeInfoInquirer::GetInstance().systemConfig_.supportedCapacityList = supportedCapacityList;
}

/**
 * @tc.name: testAdjustEnhancedPanelRect_001
 * @tc.desc: Test AdjustPanelRect with invalid panel type.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelAdjustTest, testAdjustEnhancedPanelRect_001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelAdjustTest testAdjustEnhancedPanelRect_001 Test START");
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo;
    panelInfo.panelType = STATUS_BAR;
    InputMethodPanelAdjustTest::ImaCreatePanel(panelInfo, inputMethodPanel);
    PanelFlag panelFlag = PanelFlag::FLG_FIXED;
    EnhancedLayoutParams params;
    HotAreas hotAreas;
    auto ret = inputMethodPanel->AdjustPanelRect(panelFlag, params, hotAreas);
    EXPECT_EQ(ret, ErrorCode::ERROR_INVALID_PANEL_TYPE);
    InputMethodPanelAdjustTest::ImaDestroyPanel(inputMethodPanel);
}

/**
 * @tc.name: testAdjustEnhancedPanelRect_002
 * @tc.desc: Test AdjustPanelRect with invalid portrait rect position.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelAdjustTest, testAdjustEnhancedPanelRect_002, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelAdjustTest testAdjustEnhancedPanelRect_002 Test START");
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    InputMethodPanelAdjustTest::ImaCreatePanel(panelInfo, inputMethodPanel);
    PanelFlag panelFlag = PanelFlag::FLG_FLOATING;
    Rosen::Rect portraitRect = { -1, -1, 100, 100 };
    Rosen::Rect validRect = { 1, 1, 100, 100 };
    EnhancedLayoutParams params = {
        .isFullScreen = false,
        .portrait = { portraitRect, 50, 50 },
        .landscape = { validRect, 50, 50 },
    };
    HotAreas hotAreas;
    auto ret = inputMethodPanel->AdjustPanelRect(panelFlag, params, hotAreas);
    EXPECT_EQ(ret, ErrorCode::ERROR_PARAMETER_CHECK_FAILED);
    InputMethodPanelAdjustTest::ImaDestroyPanel(inputMethodPanel);
}

/**
 * @tc.name: testAdjustEnhancedPanelRect_003
 * @tc.desc: Test AdjustPanelRect with invalid landscape rect position.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelAdjustTest, testAdjustEnhancedPanelRect_003, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelAdjustTest testAdjustEnhancedPanelRect_003 Test START");
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    InputMethodPanelAdjustTest::ImaCreatePanel(panelInfo, inputMethodPanel);
    PanelFlag panelFlag = PanelFlag::FLG_FLOATING;
    Rosen::Rect portraitRect = { -1, -1, 100, 100 };
    Rosen::Rect validRect = { 1, 1, 100, 100 };
    EnhancedLayoutParams params = {
        .isFullScreen = false,
        .portrait = { validRect, 50, 50 },
        .landscape = { portraitRect, 50, 50 },
    };
    HotAreas hotAreas;
    auto ret = inputMethodPanel->AdjustPanelRect(panelFlag, params, hotAreas);
    EXPECT_EQ(ret, ErrorCode::ERROR_PARAMETER_CHECK_FAILED);
    InputMethodPanelAdjustTest::ImaDestroyPanel(inputMethodPanel);
}

/**
 * @tc.name: testAdjustEnhancedPanelRect_004
 * @tc.desc: Test AdjustPanelRect with invalid portrait rect width.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelAdjustTest, testAdjustEnhancedPanelRect_004, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelAdjustTest testAdjustEnhancedPanelRect_004 Test START");
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    InputMethodPanelAdjustTest::ImaCreatePanel(panelInfo, inputMethodPanel);
    DisplaySize display;
    ASSERT_EQ(InputMethodPanelAdjustTest::GetDisplaySize(display), ErrorCode::NO_ERROR);
    PanelFlag panelFlag = PanelFlag::FLG_FLOATING;
    Rosen::Rect portraitRect = { 0, 0, display.portrait.width + 1, display.portrait.height - 1 };
    Rosen::Rect landscapeRect = { 0, 0, display.landscape.width - 1, display.landscape.height - 1 };
    EnhancedLayoutParams params = {
        .isFullScreen = false,
        .portrait = { portraitRect, 50, 50 },
        .landscape = { landscapeRect, 50, 50 },
    };
    HotAreas hotAreas;
    auto ret = inputMethodPanel->AdjustPanelRect(panelFlag, params, hotAreas);
    EXPECT_EQ(ret, ErrorCode::ERROR_PARAMETER_CHECK_FAILED);
    InputMethodPanelAdjustTest::ImaDestroyPanel(inputMethodPanel);
}

/**
 * @tc.name: testAdjustEnhancedPanelRect_005
 * @tc.desc: Test AdjustPanelRect with invalid portrait rect height.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelAdjustTest, testAdjustEnhancedPanelRect_005, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelAdjustTest testAdjustEnhancedPanelRect_005 Test START");
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    InputMethodPanelAdjustTest::ImaCreatePanel(panelInfo, inputMethodPanel);
    DisplaySize display;
    ASSERT_EQ(InputMethodPanelAdjustTest::GetDisplaySize(display), ErrorCode::NO_ERROR);
    PanelFlag panelFlag = PanelFlag::FLG_FLOATING;
    Rosen::Rect portraitRect = { 0, 0, display.portrait.width - 1, display.portrait.height + 1 };
    Rosen::Rect landscapeRect = { 0, 0, display.landscape.width - 1, display.landscape.height - 1 };
    EnhancedLayoutParams params = {
        .isFullScreen = false,
        .portrait = { portraitRect, 50, 50 },
        .landscape = { landscapeRect, 50, 50 },
    };
    HotAreas hotAreas;
    auto ret = inputMethodPanel->AdjustPanelRect(panelFlag, params, hotAreas);
    EXPECT_EQ(ret, ErrorCode::ERROR_PARAMETER_CHECK_FAILED);
    InputMethodPanelAdjustTest::ImaDestroyPanel(inputMethodPanel);
}

/**
 * @tc.name: testAdjustEnhancedPanelRect_006
 * @tc.desc: Test AdjustPanelRect with invalid landscape rect width.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelAdjustTest, testAdjustEnhancedPanelRect_006, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelAdjustTest testAdjustEnhancedPanelRect_006 Test START");
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    InputMethodPanelAdjustTest::ImaCreatePanel(panelInfo, inputMethodPanel);
    DisplaySize display;
    ASSERT_EQ(InputMethodPanelAdjustTest::GetDisplaySize(display), ErrorCode::NO_ERROR);
    PanelFlag panelFlag = PanelFlag::FLG_FLOATING;
    Rosen::Rect portraitRect = { 0, 0, display.portrait.width - 1, display.portrait.height - 1 };
    Rosen::Rect landscapeRect = { 0, 0, display.landscape.width + 1, display.landscape.height - 1 };
    EnhancedLayoutParams params = {
        .isFullScreen = false,
        .portrait = { portraitRect, 50, 50 },
        .landscape = { landscapeRect, 50, 50 },
    };
    HotAreas hotAreas;
    auto ret = inputMethodPanel->AdjustPanelRect(panelFlag, params, hotAreas);
    EXPECT_EQ(ret, ErrorCode::ERROR_PARAMETER_CHECK_FAILED);
    InputMethodPanelAdjustTest::ImaDestroyPanel(inputMethodPanel);
}

/**
 * @tc.name: testAdjustEnhancedPanelRect_007
 * @tc.desc: Test AdjustPanelRect with invalid landscape rect height.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelAdjustTest, testAdjustEnhancedPanelRect_007, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelAdjustTest testAdjustEnhancedPanelRect_007 Test START");
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    InputMethodPanelAdjustTest::ImaCreatePanel(panelInfo, inputMethodPanel);
    DisplaySize display;
    ASSERT_EQ(InputMethodPanelAdjustTest::GetDisplaySize(display), ErrorCode::NO_ERROR);
    PanelFlag panelFlag = PanelFlag::FLG_FLOATING;
    Rosen::Rect portraitRect = { 0, 0, display.portrait.width - 1, display.portrait.height - 1 };
    Rosen::Rect landscapeRect = { 0, 0, display.landscape.width - 1, display.landscape.height + 1 };
    EnhancedLayoutParams params = {
        .isFullScreen = false,
        .portrait = { portraitRect, 50, 50 },
        .landscape = { landscapeRect, 50, 50 },
    };
    HotAreas hotAreas;
    auto ret = inputMethodPanel->AdjustPanelRect(panelFlag, params, hotAreas);
    EXPECT_EQ(ret, ErrorCode::ERROR_PARAMETER_CHECK_FAILED);
    InputMethodPanelAdjustTest::ImaDestroyPanel(inputMethodPanel);
}

/**
 * @tc.name: testAdjustEnhancedPanelRect_008
 * @tc.desc: Test AdjustPanelRect with invalid portrait avoid Y(< 0).
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelAdjustTest, testAdjustEnhancedPanelRect_008, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelAdjustTest testAdjustEnhancedPanelRect_008 Test START");
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    InputMethodPanelAdjustTest::ImaCreatePanel(panelInfo, inputMethodPanel);
    DisplaySize display;
    ASSERT_EQ(InputMethodPanelAdjustTest::GetDisplaySize(display), ErrorCode::NO_ERROR);
    PanelFlag panelFlag = PanelFlag::FLG_FLOATING;
    Rosen::Rect portraitRect = { 0, 0, display.portrait.width - 1, display.portrait.height - 1 };
    Rosen::Rect landscapeRect = { 0, 0, display.landscape.width - 1, display.landscape.height - 1 };
    EnhancedLayoutParams params = {
        .isFullScreen = false,
        .portrait = { portraitRect, -1, 0 },
        .landscape = { landscapeRect, 1, 0 },
    };
    HotAreas hotAreas;
    auto ret = inputMethodPanel->AdjustPanelRect(panelFlag, params, hotAreas);
    EXPECT_EQ(ret, ErrorCode::ERROR_PARAMETER_CHECK_FAILED);
    InputMethodPanelAdjustTest::ImaDestroyPanel(inputMethodPanel);
}

/**
 * @tc.name: testAdjustEnhancedPanelRect_009
 * @tc.desc: Test AdjustPanelRect with invalid portrait avoid Y(> keyboard height).
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelAdjustTest, testAdjustEnhancedPanelRect_009, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelAdjustTest testAdjustEnhancedPanelRect_009 Test START");
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    InputMethodPanelAdjustTest::ImaCreatePanel(panelInfo, inputMethodPanel);
    DisplaySize display;
    ASSERT_EQ(InputMethodPanelAdjustTest::GetDisplaySize(display), ErrorCode::NO_ERROR);
    PanelFlag panelFlag = PanelFlag::FLG_FLOATING;
    Rosen::Rect portraitRect = { 0, 0, display.portrait.width - 1, display.portrait.height - 1 };
    Rosen::Rect landscapeRect = { 0, 0, display.landscape.width - 1, display.landscape.height - 1 };
    EnhancedLayoutParams params = {
        .isFullScreen = false,
        .portrait = { portraitRect, static_cast<int32_t>(portraitRect.height_) + 1, 0 },
        .landscape = { landscapeRect, static_cast<int32_t>(landscapeRect.height_) - 1, 0 },
    };
    HotAreas hotAreas;
    auto ret = inputMethodPanel->AdjustPanelRect(panelFlag, params, hotAreas);
    EXPECT_EQ(ret, ErrorCode::ERROR_PARAMETER_CHECK_FAILED);
    InputMethodPanelAdjustTest::ImaDestroyPanel(inputMethodPanel);
}

/**
 * @tc.name: testAdjustEnhancedPanelRect_010
 * @tc.desc: Test AdjustPanelRect with invalid landscape avoid Y(< 0).
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelAdjustTest, testAdjustEnhancedPanelRect_010, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelAdjustTest testAdjustEnhancedPanelRect_010 Test START");
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    InputMethodPanelAdjustTest::ImaCreatePanel(panelInfo, inputMethodPanel);
    DisplaySize display;
    ASSERT_EQ(InputMethodPanelAdjustTest::GetDisplaySize(display), ErrorCode::NO_ERROR);
    PanelFlag panelFlag = PanelFlag::FLG_FLOATING;
    Rosen::Rect portraitRect = { 0, 0, display.portrait.width - 1, display.portrait.height - 1 };
    Rosen::Rect landscapeRect = { 0, 0, display.landscape.width - 1, display.landscape.height - 1 };
    EnhancedLayoutParams params = {
        .isFullScreen = false,
        .portrait = { portraitRect, 1, 0 },
        .landscape = { landscapeRect, -1, 0 },
    };
    HotAreas hotAreas;
    auto ret = inputMethodPanel->AdjustPanelRect(panelFlag, params, hotAreas);
    EXPECT_EQ(ret, ErrorCode::ERROR_PARAMETER_CHECK_FAILED);
    InputMethodPanelAdjustTest::ImaDestroyPanel(inputMethodPanel);
}

/**
 * @tc.name: testAdjustEnhancedPanelRect_011
 * @tc.desc: Test AdjustPanelRect with invalid landscape avoid Y(> keyboard height).
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelAdjustTest, testAdjustEnhancedPanelRect_011, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelAdjustTest testAdjustEnhancedPanelRect_011 Test START");
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    InputMethodPanelAdjustTest::ImaCreatePanel(panelInfo, inputMethodPanel);
    DisplaySize display;
    ASSERT_EQ(InputMethodPanelAdjustTest::GetDisplaySize(display), ErrorCode::NO_ERROR);
    PanelFlag panelFlag = PanelFlag::FLG_FLOATING;
    Rosen::Rect portraitRect = { 0, 0, display.portrait.width - 1, display.portrait.height - 1 };
    Rosen::Rect landscapeRect = { 0, 0, display.landscape.width - 1, display.landscape.height - 1 };
    EnhancedLayoutParams params = {
        .isFullScreen = false,
        .portrait = { portraitRect, static_cast<int32_t>(portraitRect.height_) - 1, 0 },
        .landscape = { landscapeRect, static_cast<int32_t>(landscapeRect.height_) + 1, 0 },
    };
    HotAreas hotAreas;
    auto ret = inputMethodPanel->AdjustPanelRect(panelFlag, params, hotAreas);
    EXPECT_EQ(ret, ErrorCode::ERROR_PARAMETER_CHECK_FAILED);
    InputMethodPanelAdjustTest::ImaDestroyPanel(inputMethodPanel);
}

/**
 * @tc.name: testAdjustEnhancedPanelRect_012
 * @tc.desc: Test AdjustPanelRect with fixed, full screen, invalid portrait avoidHeight(> displayHeight * 0.7).
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelAdjustTest, testAdjustEnhancedPanelRect_012, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelAdjustTest testAdjustEnhancedPanelRect_012 Test START");
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    InputMethodPanelAdjustTest::ImaCreatePanel(panelInfo, inputMethodPanel);
    DisplaySize display;
    ASSERT_EQ(InputMethodPanelAdjustTest::GetDisplaySize(display), ErrorCode::NO_ERROR);
    PanelFlag panelFlag = PanelFlag::FLG_FIXED;
    uint32_t portraitAvoidHeight = display.portrait.height * FIXED_SOFT_KEYBOARD_PANEL_RATIO + 1;
    uint32_t portraitAvoidY = display.portrait.height - portraitAvoidHeight;
    uint32_t landscapeAvoidHeight = display.landscape.height * FIXED_SOFT_KEYBOARD_PANEL_RATIO - 1;
    uint32_t landscapeAvoidY = display.landscape.height - landscapeAvoidHeight;
    EnhancedLayoutParams params = {
        .isFullScreen = true,
        .portrait = { {}, static_cast<int32_t>(portraitAvoidY), 0 },
        .landscape = { {}, static_cast<int32_t>(landscapeAvoidY), 0 },
    };
    HotAreas hotAreas;
    auto ret = inputMethodPanel->AdjustPanelRect(panelFlag, params, hotAreas);
    EXPECT_EQ(ret, ErrorCode::ERROR_PARAMETER_CHECK_FAILED);
    InputMethodPanelAdjustTest::ImaDestroyPanel(inputMethodPanel);
}

/**
 * @tc.name: testAdjustEnhancedPanelRect_013
 * @tc.desc: Test AdjustPanelRect with fixed, full screen, invalid landscape avoidHeight(> displayHeight * 0.7).
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelAdjustTest, testAdjustEnhancedPanelRect_013, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelAdjustTest testAdjustEnhancedPanelRect_013 Test START");
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    InputMethodPanelAdjustTest::ImaCreatePanel(panelInfo, inputMethodPanel);
    DisplaySize display;
    ASSERT_EQ(InputMethodPanelAdjustTest::GetDisplaySize(display), ErrorCode::NO_ERROR);
    PanelFlag panelFlag = PanelFlag::FLG_FIXED;
    uint32_t portraitAvoidHeight = display.portrait.height * FIXED_SOFT_KEYBOARD_PANEL_RATIO - 1;
    uint32_t portraitAvoidY = display.portrait.height - portraitAvoidHeight;
    uint32_t landscapeAvoidHeight = display.landscape.height * FIXED_SOFT_KEYBOARD_PANEL_RATIO + 1;
    uint32_t landscapeAvoidY = display.landscape.height - landscapeAvoidHeight;
    EnhancedLayoutParams params = {
        .isFullScreen = true,
        .portrait = { {}, static_cast<int32_t>(portraitAvoidY), 0 },
        .landscape = { {}, static_cast<int32_t>(landscapeAvoidY), 0 },
    };
    HotAreas hotAreas;
    auto ret = inputMethodPanel->AdjustPanelRect(panelFlag, params, hotAreas);
    EXPECT_EQ(ret, ErrorCode::ERROR_PARAMETER_CHECK_FAILED);
    InputMethodPanelAdjustTest::ImaDestroyPanel(inputMethodPanel);
}

/**
 * @tc.name: testAdjustEnhancedPanelRect_014
 * @tc.desc: Test AdjustPanelRect with fixed, non full screen, invalid portrait avoidHeight(> displayHeight * 0.7).
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelAdjustTest, testAdjustEnhancedPanelRect_014, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelAdjustTest testAdjustEnhancedPanelRect_014 Test START");
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    InputMethodPanelAdjustTest::ImaCreatePanel(panelInfo, inputMethodPanel);
    DisplaySize display;
    ASSERT_EQ(InputMethodPanelAdjustTest::GetDisplaySize(display), ErrorCode::NO_ERROR);
    PanelFlag panelFlag = PanelFlag::FLG_FIXED;
    Rosen::Rect portraitRect = { 0, 0, 100, static_cast<uint32_t>(display.portrait.height * 0.8) };
    Rosen::Rect landscapeRect = { 0, 0, 100, static_cast<uint32_t>(display.landscape.height * 0.8) };
    uint32_t portraitAvoidHeight = display.portrait.height * FIXED_SOFT_KEYBOARD_PANEL_RATIO + 1;
    uint32_t portraitAvoidY = portraitRect.height_ - portraitAvoidHeight;
    uint32_t landscapeAvoidHeight = display.landscape.height * FIXED_SOFT_KEYBOARD_PANEL_RATIO - 1;
    uint32_t landscapeAvoidY = landscapeRect.height_ - landscapeAvoidHeight;
    EnhancedLayoutParams params = {
        .isFullScreen = false,
        .portrait = { portraitRect, static_cast<int32_t>(portraitAvoidY), 0 },
        .landscape = { landscapeRect, static_cast<int32_t>(landscapeAvoidY), 0 },
    };
    HotAreas hotAreas;
    auto ret = inputMethodPanel->AdjustPanelRect(panelFlag, params, hotAreas);
    EXPECT_EQ(ret, ErrorCode::ERROR_PARAMETER_CHECK_FAILED);
    InputMethodPanelAdjustTest::ImaDestroyPanel(inputMethodPanel);
}

/**
 * @tc.name: testAdjustEnhancedPanelRect_015
 * @tc.desc: Test AdjustPanelRect with fixed, non full screen, invalid landscape avoidHeight(> displayHeight * 0.7).
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelAdjustTest, testAdjustEnhancedPanelRect_015, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelAdjustTest testAdjustEnhancedPanelRect_015 Test START");
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    InputMethodPanelAdjustTest::ImaCreatePanel(panelInfo, inputMethodPanel);
    DisplaySize display;
    ASSERT_EQ(InputMethodPanelAdjustTest::GetDisplaySize(display), ErrorCode::NO_ERROR);
    PanelFlag panelFlag = PanelFlag::FLG_FIXED;
    Rosen::Rect portraitRect = { 0, 0, 100, static_cast<uint32_t>(display.portrait.height * 0.8) };
    Rosen::Rect landscapeRect = { 0, 0, 100, static_cast<uint32_t>(display.landscape.height * 0.8) };
    uint32_t portraitAvoidHeight = display.portrait.height * FIXED_SOFT_KEYBOARD_PANEL_RATIO - 1;
    uint32_t portraitAvoidY = portraitRect.height_ - portraitAvoidHeight;
    uint32_t landscapeAvoidHeight = display.landscape.height * FIXED_SOFT_KEYBOARD_PANEL_RATIO + 1;
    uint32_t landscapeAvoidY = landscapeRect.height_ - landscapeAvoidHeight;
    EnhancedLayoutParams params = {
        .isFullScreen = false,
        .portrait = { portraitRect, static_cast<int32_t>(portraitAvoidY), 0 },
        .landscape = { landscapeRect, static_cast<int32_t>(landscapeAvoidY), 0 },
    };
    HotAreas hotAreas;
    auto ret = inputMethodPanel->AdjustPanelRect(panelFlag, params, hotAreas);
    EXPECT_EQ(ret, ErrorCode::ERROR_PARAMETER_CHECK_FAILED);
    InputMethodPanelAdjustTest::ImaDestroyPanel(inputMethodPanel);
}

/**
 * @tc.name: testAdjustEnhancedPanelRect_016
 * @tc.desc: Test AdjustPanelRect with fixed, non full screen, success.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelAdjustTest, testAdjustEnhancedPanelRect_016, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelAdjustTest testAdjustEnhancedPanelRect_016 Test START");
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    InputMethodPanelAdjustTest::ImaCreatePanel(panelInfo, inputMethodPanel);
    DisplaySize display;
    ASSERT_EQ(InputMethodPanelAdjustTest::GetDisplaySize(display), ErrorCode::NO_ERROR);
    PanelFlag panelFlag = PanelFlag::FLG_FIXED;
    Rosen::Rect portraitRect = { 0, 0, 100, static_cast<uint32_t>(display.portrait.height * 0.8) };
    Rosen::Rect landscapeRect = { 0, 0, 100, static_cast<uint32_t>(display.landscape.height * 0.8) };
    uint32_t portraitAvoidHeight = display.portrait.height * FIXED_SOFT_KEYBOARD_PANEL_RATIO - 1;
    uint32_t portraitAvoidY = display.portrait.height - portraitAvoidHeight;
    uint32_t landscapeAvoidHeight = display.landscape.height * FIXED_SOFT_KEYBOARD_PANEL_RATIO - 1;
    uint32_t landscapeAvoidY = display.landscape.height - landscapeAvoidHeight;
    EnhancedLayoutParams params = {
        .isFullScreen = true,
        .portrait = { portraitRect, static_cast<int32_t>(portraitAvoidY), 0 },
        .landscape = { landscapeRect, static_cast<int32_t>(landscapeAvoidY), 0 },
    };
    HotAreas hotAreas;
    auto ret = inputMethodPanel->AdjustPanelRect(panelFlag, params, hotAreas);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodPanelAdjustTest::ImaDestroyPanel(inputMethodPanel);
}

/**
 * @tc.name: testAdjustEnhancedPanelRect_017
 * @tc.desc: Test old AdjustPanelRect with 0 width input, then UpdateRegion success.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelAdjustTest, testAdjustEnhancedPanelRect_017, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelAdjustTest testAdjustEnhancedPanelRect_017 Test START");
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    InputMethodPanelAdjustTest::ImaCreatePanel(panelInfo, inputMethodPanel);
    DisplaySize display;
    ASSERT_EQ(InputMethodPanelAdjustTest::GetDisplaySize(display), ErrorCode::NO_ERROR);
    PanelFlag panelFlag = PanelFlag::FLG_FIXED;
    Rosen::Rect portraitRect = { 0, 0, 0, static_cast<uint32_t>(display.portrait.height * 0.4) };
    Rosen::Rect landscapeRect = { 0, 0, 0, static_cast<uint32_t>(display.landscape.height * 0.4) };
    LayoutParams layoutParams = { .landscapeRect = landscapeRect, .portraitRect = portraitRect };
    auto ret = inputMethodPanel->AdjustPanelRect(panelFlag, layoutParams, true);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    Rosen::Rect keyboardHotArea = {};
    bool isPortrait = inputMethodPanel->IsDisplayPortrait();
    if (isPortrait) {
        keyboardHotArea = { 0, 0, static_cast<uint32_t>(display.portrait.width * 0.5), portraitRect.height_ };
    } else {
        keyboardHotArea = { 0, 0, static_cast<uint32_t>(display.landscape.width * 0.5), landscapeRect.height_ };
    }
    std::vector<Rosen::Rect> region = { keyboardHotArea };
    ret = inputMethodPanel->UpdateRegion(region);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    if (isPortrait) {
        EXPECT_EQ(inputMethodPanel->hotAreas_.portrait.keyboardHotArea.size(), 1);
        EXPECT_EQ(inputMethodPanel->hotAreas_.portrait.keyboardHotArea[0], keyboardHotArea);
    } else {
        EXPECT_EQ(inputMethodPanel->hotAreas_.landscape.keyboardHotArea.size(), 1);
        EXPECT_EQ(inputMethodPanel->hotAreas_.landscape.keyboardHotArea[0], keyboardHotArea);
    }
    InputMethodPanelAdjustTest::ImaDestroyPanel(inputMethodPanel);
}

/**
 * @tc.name: testAdjustEnhancedPanelRect_018
 * @tc.desc: Test old AdjustPanelRect, then UpdateRegion, then moveTo can not change hot Areas.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelAdjustTest, testAdjustEnhancedPanelRect_018, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelAdjustTest testAdjustEnhancedPanelRect_018 Test START");
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    InputMethodPanelAdjustTest::ImaCreatePanel(panelInfo, inputMethodPanel);
    DisplaySize display;
    ASSERT_EQ(InputMethodPanelAdjustTest::GetDisplaySize(display), ErrorCode::NO_ERROR);
    PanelFlag panelFlag = PanelFlag::FLG_FLOATING;
    Rosen::Rect portraitRect = { 0, 0, display.portrait.width, static_cast<uint32_t>(display.portrait.height * 0.5) };
    Rosen::Rect landscapeRect = { 0, 0, display.landscape.width,
        static_cast<uint32_t>(display.landscape.height * 0.5) };
    LayoutParams layoutParams = { .landscapeRect = landscapeRect, .portraitRect = portraitRect };
    auto ret = inputMethodPanel->AdjustPanelRect(panelFlag, layoutParams, true);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    Rosen::Rect keyboardHotArea = {};
    bool isPortrait = inputMethodPanel->IsDisplayPortrait();
    if (isPortrait) {
        keyboardHotArea = { 0, 0, static_cast<uint32_t>(portraitRect.width_ * 0.5), portraitRect.height_ };
    } else {
        keyboardHotArea = { 0, 0, static_cast<uint32_t>(landscapeRect.width_ * 0.5), landscapeRect.height_ };
    }
    std::vector<Rosen::Rect> region = { keyboardHotArea };
    ret = inputMethodPanel->UpdateRegion(region);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = inputMethodPanel->MoveTo(1, 1);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    if (isPortrait) {
        EXPECT_EQ(inputMethodPanel->hotAreas_.portrait.keyboardHotArea.size(), 1);
        EXPECT_EQ(inputMethodPanel->hotAreas_.portrait.keyboardHotArea[0], keyboardHotArea);
    } else {
        EXPECT_EQ(inputMethodPanel->hotAreas_.landscape.keyboardHotArea.size(), 1);
        EXPECT_EQ(inputMethodPanel->hotAreas_.landscape.keyboardHotArea[0], keyboardHotArea);
    }
    InputMethodPanelAdjustTest::ImaDestroyPanel(inputMethodPanel);
}

/**
 * @tc.name: testAdjustEnhancedPanelRect_019
 * @tc.desc: Test old AdjustPanelRect, then Resize.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelAdjustTest, testAdjustEnhancedPanelRect_019, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelAdjustTest testAdjustEnhancedPanelRect_019 Test START");
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    InputMethodPanelAdjustTest::ImaCreatePanel(panelInfo, inputMethodPanel);
    inputMethodPanel->isScbEnable_ = true;
    DisplaySize display;
    ASSERT_EQ(InputMethodPanelAdjustTest::GetDisplaySize(display), ErrorCode::NO_ERROR);
    PanelFlag panelFlag = PanelFlag::FLG_FLOATING;
    Rosen::Rect portraitRect = { 0, 0, display.portrait.width, static_cast<uint32_t>(display.portrait.height * 0.5) };
    Rosen::Rect landscapeRect = { 0, 0, display.landscape.width,
        static_cast<uint32_t>(display.landscape.height * 0.5) };
    LayoutParams layoutParams = { .landscapeRect = landscapeRect, .portraitRect = portraitRect };
    auto ret = inputMethodPanel->AdjustPanelRect(panelFlag, layoutParams, true);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    WindowSize size = {};
    bool isPortrait = inputMethodPanel->IsDisplayPortrait();
    if (isPortrait) {
        size = { static_cast<uint32_t>(portraitRect.width_ * 0.5), static_cast<uint32_t>(portraitRect.height_ * 0.5) };
    } else {
        size = { static_cast<uint32_t>(landscapeRect.width_ * 0.5),
            static_cast<uint32_t>(landscapeRect.height_ * 0.5) };
    }
    ret = inputMethodPanel->Resize(size.width, size.height);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    Rosen::Rect newRect = { 0, 0, size.width, size.height };
    if (isPortrait) {
        EXPECT_EQ(inputMethodPanel->keyboardLayoutParams_.PortraitKeyboardRect_, newRect);
        EXPECT_EQ(inputMethodPanel->keyboardLayoutParams_.LandscapeKeyboardRect_, landscapeRect);
    } else {
        EXPECT_EQ(inputMethodPanel->keyboardLayoutParams_.PortraitKeyboardRect_, portraitRect);
        EXPECT_EQ(inputMethodPanel->keyboardLayoutParams_.LandscapeKeyboardRect_, newRect);
    }
    InputMethodPanelAdjustTest::ImaDestroyPanel(inputMethodPanel);
}

/**
 * @tc.name: testAdjustEnhancedPanelRect_020
 * @tc.desc: Test new AdjustPanelRect, then Resize.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelAdjustTest, testAdjustEnhancedPanelRect_020, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelAdjustTest testAdjustEnhancedPanelRect_020 Test START");
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    InputMethodPanelAdjustTest::ImaCreatePanel(panelInfo, inputMethodPanel);
    inputMethodPanel->isScbEnable_ = true;
    DisplaySize display;
    ASSERT_EQ(InputMethodPanelAdjustTest::GetDisplaySize(display), ErrorCode::NO_ERROR);
    PanelFlag panelFlag = PanelFlag::FLG_FIXED;
    Rosen::Rect portraitRect = { 0, 0, 0, static_cast<uint32_t>(display.portrait.height * 0.8) };
    Rosen::Rect landscapeRect = { 0, 0, 0, static_cast<uint32_t>(display.landscape.height * 0.8) };
    uint32_t portraitAvoidHeight = display.portrait.height * FIXED_SOFT_KEYBOARD_PANEL_RATIO - 1;
    uint32_t portraitAvoidY = display.portrait.height - portraitAvoidHeight;
    uint32_t landscapeAvoidHeight = display.landscape.height * FIXED_SOFT_KEYBOARD_PANEL_RATIO - 1;
    uint32_t landscapeAvoidY = display.landscape.height - landscapeAvoidHeight;
    EnhancedLayoutParams params = {
        .isFullScreen = false,
        .portrait = { portraitRect, static_cast<int32_t>(portraitAvoidY), 0 },
        .landscape = { landscapeRect, static_cast<int32_t>(landscapeAvoidY), 0 },
    };
    auto ret = inputMethodPanel->AdjustPanelRect(panelFlag, params, {});
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    WindowSize size = {};
    bool isPortrait = inputMethodPanel->IsDisplayPortrait();
    if (isPortrait) {
        size = { 100, static_cast<uint32_t>(portraitRect.height_ * 0.5) };
    } else {
        size = { 100, static_cast<uint32_t>(landscapeRect.height_ * 0.5) };
    }
    ret = inputMethodPanel->Resize(size.width, size.height);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    if (isPortrait) {
        Rosen::Rect newPortraitRect = { 0, static_cast<int32_t>(display.portrait.height - size.height),
            display.portrait.width, size.height };
        Rosen::Rect newLandRect = { 0, static_cast<int32_t>(display.landscape.height - landscapeRect.height_),
            display.landscape.width, landscapeRect.height_ };
        EXPECT_EQ(inputMethodPanel->keyboardLayoutParams_.PortraitKeyboardRect_, newPortraitRect);
        EXPECT_EQ(inputMethodPanel->keyboardLayoutParams_.LandscapeKeyboardRect_, newLandRect);
    } else {
        Rosen::Rect newPortraitRect = { 0, static_cast<int32_t>(display.portrait.height - portraitRect.height_),
            display.portrait.width, portraitRect.height_ };
        Rosen::Rect newLandRect = { 0, static_cast<int32_t>(display.landscape.height - size.height),
            display.landscape.width, size.height };
        EXPECT_EQ(inputMethodPanel->keyboardLayoutParams_.PortraitKeyboardRect_, newPortraitRect);
        EXPECT_EQ(inputMethodPanel->keyboardLayoutParams_.LandscapeKeyboardRect_, newLandRect);
    }
    InputMethodPanelAdjustTest::ImaDestroyPanel(inputMethodPanel);
}

/**
 * @tc.name: testSetImmersiveEffect_001
 * @tc.desc: Test SetImmersiveEffect device not supported.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelAdjustTest, testSetImmersiveEffect_001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelAdjustTest testSetImmersiveEffect_001 Test START");
    InputMethodPanelAdjustTest::SetImmersiveCapacitySupport(true);
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    InputMethodPanelAdjustTest::ImaCreatePanel(panelInfo, inputMethodPanel);

    ImmersiveEffect immersiveEffect = {
        .gradientHeight = 20, .gradientMode = GradientMode::LINEAR_GRADIENT, .fluidLightMode = FluidLightMode::NONE
    };
    int32_t ret = inputMethodPanel->SetImmersiveEffect(immersiveEffect);
    EXPECT_NE(ret, ErrorCode::ERROR_DEVICE_UNSUPPORTED);
    InputMethodPanelAdjustTest::ImaDestroyPanel(inputMethodPanel);
}

/**
 * @tc.name: testSetImmersiveEffect_002
 * @tc.desc: Test SetImmersiveEffect invalid parameter.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelAdjustTest, testSetImmersiveEffect_002, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelAdjustTest testSetImmersiveEffect_002 Test START");
    InputMethodPanelAdjustTest::SetImmersiveCapacitySupport(true);
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    InputMethodPanelAdjustTest::ImaCreatePanel(panelInfo, inputMethodPanel);
    ImmersiveEffect immersiveEffect;

    immersiveEffect.gradientMode = static_cast<GradientMode>(-1);
    int32_t ret = inputMethodPanel->SetImmersiveEffect(immersiveEffect);
    EXPECT_EQ(ret, ErrorCode::ERROR_PARAMETER_CHECK_FAILED);

    immersiveEffect.gradientMode = GradientMode::END;
    ret = inputMethodPanel->SetImmersiveEffect(immersiveEffect);
    EXPECT_EQ(ret, ErrorCode::ERROR_PARAMETER_CHECK_FAILED);

    immersiveEffect.gradientMode = GradientMode::LINEAR_GRADIENT;
    immersiveEffect.fluidLightMode = static_cast<FluidLightMode>(-1);
    ret = inputMethodPanel->SetImmersiveEffect(immersiveEffect);
    EXPECT_EQ(ret, ErrorCode::ERROR_PARAMETER_CHECK_FAILED);

    immersiveEffect.gradientMode = GradientMode::LINEAR_GRADIENT;
    immersiveEffect.fluidLightMode = FluidLightMode::END;
    ret = inputMethodPanel->SetImmersiveEffect(immersiveEffect);
    EXPECT_EQ(ret, ErrorCode::ERROR_PARAMETER_CHECK_FAILED);
    InputMethodPanelAdjustTest::ImaDestroyPanel(inputMethodPanel);
}

/**
 * @tc.name: testSetImmersiveEffect_003
 * @tc.desc: Test SetImmersiveEffect when not in immersive mode
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelAdjustTest, testSetImmersiveEffect_003, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelAdjustTest testSetImmersiveEffect_003 Test START");
    InputMethodPanelAdjustTest::SetImmersiveCapacitySupport(true);
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    InputMethodPanelAdjustTest::ImaCreatePanel(panelInfo, inputMethodPanel);
    ImmersiveEffect immersiveEffect;

    inputMethodPanel->immersiveMode_ = ImmersiveMode::NONE_IMMERSIVE;
    immersiveEffect = { .gradientMode = GradientMode::LINEAR_GRADIENT, .fluidLightMode = FluidLightMode::NONE };
    int32_t ret = inputMethodPanel->SetImmersiveEffect(immersiveEffect);
    EXPECT_EQ(ret, ErrorCode::ERROR_IMA_INVALID_IMMERSIVE_EFFECT);
    immersiveEffect = { .gradientMode = GradientMode::NONE, .fluidLightMode = FluidLightMode::BACKGROUND_FLUID_LIGHT };
    ret = inputMethodPanel->SetImmersiveEffect(immersiveEffect);
    EXPECT_EQ(ret, ErrorCode::ERROR_IMA_INVALID_IMMERSIVE_EFFECT);
    immersiveEffect = { .gradientMode = GradientMode::NONE, .fluidLightMode = FluidLightMode::NONE };
    ret = inputMethodPanel->SetImmersiveEffect(immersiveEffect);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodPanelAdjustTest::ImaDestroyPanel(inputMethodPanel);
}

/**
 * @tc.name: testSetImmersiveEffect_004
 * @tc.desc: Test SetImmersiveEffect invalid parameter.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelAdjustTest, testSetImmersiveEffect_004, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelAdjustTest testSetImmersiveEffect_004 Test START");
    InputMethodPanelAdjustTest::SetImmersiveCapacitySupport(true);
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    InputMethodPanelAdjustTest::ImaCreatePanel(panelInfo, inputMethodPanel);
    ImmersiveEffect immersiveEffect;

    // The fluid light mode can only be used when the gradient mode is enabled.
    inputMethodPanel->immersiveMode_ = ImmersiveMode::IMMERSIVE;
    immersiveEffect = { .gradientMode = GradientMode::NONE, .fluidLightMode = FluidLightMode::BACKGROUND_FLUID_LIGHT };
    int32_t ret = inputMethodPanel->SetImmersiveEffect(immersiveEffect);
    EXPECT_EQ(ret, ErrorCode::ERROR_IMA_INVALID_IMMERSIVE_EFFECT);

    // Only system applications can set the fluid light mode.
    IdentityCheckerMock::SetSystemApp(false);
    immersiveEffect = { .gradientMode = GradientMode::LINEAR_GRADIENT,
        .fluidLightMode = FluidLightMode::BACKGROUND_FLUID_LIGHT };
    ret = inputMethodPanel->SetImmersiveEffect(immersiveEffect);
    EXPECT_EQ(ret, ErrorCode::ERROR_STATUS_SYSTEM_PERMISSION);

    // The blur height cannot be greater than the screen height.
    IdentityCheckerMock::SetSystemApp(true);
    WindowSize displaySize{ 0, 0 };
    bool isPortrait = inputMethodPanel->IsDisplayPortrait();
    EXPECT_TRUE(inputMethodPanel->GetDisplaySize(isPortrait, displaySize));
    immersiveEffect = { .gradientHeight = displaySize.height + 1,
        .gradientMode = GradientMode::LINEAR_GRADIENT,
        .fluidLightMode = FluidLightMode::BACKGROUND_FLUID_LIGHT };
    ret = inputMethodPanel->SetImmersiveEffect(immersiveEffect);
    EXPECT_EQ(ret, ErrorCode::ERROR_PARAMETER_CHECK_FAILED);

    IdentityCheckerMock::SetSystemApp(true);
    immersiveEffect = { .gradientHeight = displaySize.height - 1,
        .gradientMode = GradientMode::LINEAR_GRADIENT,
        .fluidLightMode = FluidLightMode::BACKGROUND_FLUID_LIGHT };
    Rosen::KeyboardLayoutParams emptyParams;
    inputMethodPanel->keyboardLayoutParams_ = emptyParams;
    ret = inputMethodPanel->SetImmersiveEffect(immersiveEffect);
    EXPECT_EQ(ret, ErrorCode::ERROR_IMA_PRECONDITION_REQUIRED);
    InputMethodPanelAdjustTest::ImaDestroyPanel(inputMethodPanel);
}

/**
 * @tc.name: testSetImmersiveEffect_005
 * @tc.desc: Test SetImmersiveEffect, NormalImePrepare check parameter failed.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelAdjustTest, testSetImmersiveEffect_005, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelAdjustTest testSetImmersiveEffect_005 Test START");
    InputMethodPanelAdjustTest::SetImmersiveCapacitySupport(true);
    IdentityCheckerMock::SetSystemApp(true);
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    InputMethodPanelAdjustTest::ImaCreatePanel(panelInfo, inputMethodPanel);
    inputMethodPanel->immersiveMode_ = ImmersiveMode::IMMERSIVE;

    DisplaySize display;
    ASSERT_EQ(InputMethodPanelAdjustTest::GetDisplaySize(display), ErrorCode::NO_ERROR);
    PanelFlag panelFlag = PanelFlag::FLG_FIXED;
    Rosen::Rect portraitRect = { 0, 0, 0, static_cast<uint32_t>(display.portrait.height * 0.4) };
    Rosen::Rect landscapeRect = { 0, 0, 0, static_cast<uint32_t>(display.landscape.height * 0.4) };
    LayoutParams layoutParams = { .landscapeRect = landscapeRect, .portraitRect = portraitRect };
    auto ret = inputMethodPanel->AdjustPanelRect(panelFlag, layoutParams, true);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ImmersiveEffect immersiveEffect;
    immersiveEffect = { .gradientHeight =
                            static_cast<uint32_t>(display.portrait.height * FIXED_SOFT_KEYBOARD_PANEL_RATIO),
        .gradientMode = GradientMode::LINEAR_GRADIENT,
        .fluidLightMode = FluidLightMode::BACKGROUND_FLUID_LIGHT };
    ret = inputMethodPanel->SetImmersiveEffect(immersiveEffect);
    EXPECT_EQ(ret, ErrorCode::ERROR_PARAMETER_CHECK_FAILED);

    immersiveEffect = { .gradientHeight =
                            static_cast<uint32_t>(display.landscape.height * FIXED_SOFT_KEYBOARD_PANEL_RATIO)
                            - landscapeRect.height_ + 1,
        .gradientMode = GradientMode::LINEAR_GRADIENT,
        .fluidLightMode = FluidLightMode::BACKGROUND_FLUID_LIGHT };
    ret = inputMethodPanel->SetImmersiveEffect(immersiveEffect);
    EXPECT_EQ(ret, ErrorCode::ERROR_PARAMETER_CHECK_FAILED);
    InputMethodPanelAdjustTest::ImaDestroyPanel(inputMethodPanel);
}

/**
 * @tc.name: testSetImmersiveEffect_006
 * @tc.desc: Test SetImmersiveEffect, NormalImePrepare success.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelAdjustTest, testSetImmersiveEffect_006, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelAdjustTest testSetImmersiveEffect_006 Test START");
    InputMethodPanelAdjustTest::SetImmersiveCapacitySupport(true);
    IdentityCheckerMock::SetSystemApp(true);
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    InputMethodPanelAdjustTest::ImaCreatePanel(panelInfo, inputMethodPanel);
    inputMethodPanel->immersiveMode_ = ImmersiveMode::IMMERSIVE;

    DisplaySize display;
    ASSERT_EQ(InputMethodPanelAdjustTest::GetDisplaySize(display), ErrorCode::NO_ERROR);
    PanelFlag panelFlag = PanelFlag::FLG_FIXED;
    Rosen::Rect portraitRect = { 0, 0, 0, static_cast<uint32_t>(display.portrait.height * 0.4) };
    Rosen::Rect landscapeRect = { 0, 0, 0, static_cast<uint32_t>(display.landscape.height * 0.4) };
    LayoutParams layoutParams = { .landscapeRect = landscapeRect, .portraitRect = portraitRect };
    auto ret = inputMethodPanel->AdjustPanelRect(panelFlag, layoutParams, true);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = inputMethodPanel->ShowPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ImmersiveEffect immersiveEffect;
    immersiveEffect = { .gradientHeight = static_cast<uint32_t>(display.landscape.height * 0.1),
        .gradientMode = GradientMode::LINEAR_GRADIENT,
        .fluidLightMode = FluidLightMode::BACKGROUND_FLUID_LIGHT };
    ret = inputMethodPanel->SetImmersiveEffect(immersiveEffect);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodPanelAdjustTest::ImaDestroyPanel(inputMethodPanel);
}

/**
 * @tc.name: testSetImmersiveEffect_007
 * @tc.desc: Test SetImmersiveEffect, FullScreenPrepare check portrait parameter failed.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelAdjustTest, testSetImmersiveEffect_007, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelAdjustTest testSetImmersiveEffect_007 Test START");
    InputMethodPanelAdjustTest::SetImmersiveCapacitySupport(true);
    IdentityCheckerMock::SetSystemApp(true);
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    InputMethodPanelAdjustTest::ImaCreatePanel(panelInfo, inputMethodPanel);
    inputMethodPanel->immersiveMode_ = ImmersiveMode::IMMERSIVE;

    DisplaySize display;
    ASSERT_EQ(InputMethodPanelAdjustTest::GetDisplaySize(display), ErrorCode::NO_ERROR);
    PanelFlag panelFlag = PanelFlag::FLG_FIXED;
    uint32_t portraitAvoidHeight = display.portrait.height * FIXED_SOFT_KEYBOARD_PANEL_RATIO;
    uint32_t portraitAvoidY = display.portrait.height - portraitAvoidHeight;
    uint32_t landscapeAvoidHeight = display.landscape.height * FIXED_SOFT_KEYBOARD_PANEL_RATIO - 1;
    uint32_t landscapeAvoidY = display.landscape.height - landscapeAvoidHeight;
    EnhancedLayoutParams params = {
        .isFullScreen = true,
        .portrait = { {}, static_cast<int32_t>(portraitAvoidY), 0 },
        .landscape = { {}, static_cast<int32_t>(landscapeAvoidY), 0 },
    };
    HotAreas hotAreas;
    auto ret = inputMethodPanel->AdjustPanelRect(panelFlag, params, hotAreas);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ImmersiveEffect immersiveEffect;
    immersiveEffect = { .gradientHeight = 1,
        .gradientMode = GradientMode::LINEAR_GRADIENT,
        .fluidLightMode = FluidLightMode::BACKGROUND_FLUID_LIGHT };
    ret = inputMethodPanel->SetImmersiveEffect(immersiveEffect);
    EXPECT_EQ(ret, ErrorCode::ERROR_PARAMETER_CHECK_FAILED);
}

/**
 * @tc.name: testSetImmersiveEffect_008
 * @tc.desc: Test SetImmersiveEffect, FullScreenPrepare check landscape parameter failed.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelAdjustTest, testSetImmersiveEffect_008, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelAdjustTest testSetImmersiveEffect_008 Test START");
    InputMethodPanelAdjustTest::SetImmersiveCapacitySupport(true);
    IdentityCheckerMock::SetSystemApp(true);
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    InputMethodPanelAdjustTest::ImaCreatePanel(panelInfo, inputMethodPanel);
    inputMethodPanel->immersiveMode_ = ImmersiveMode::IMMERSIVE;

    DisplaySize display;
    ASSERT_EQ(InputMethodPanelAdjustTest::GetDisplaySize(display), ErrorCode::NO_ERROR);
    PanelFlag panelFlag = PanelFlag::FLG_FIXED;
    uint32_t portraitAvoidHeight = display.portrait.height * FIXED_SOFT_KEYBOARD_PANEL_RATIO - 1;
    uint32_t portraitAvoidY = display.portrait.height - portraitAvoidHeight;
    uint32_t landscapeAvoidHeight = display.landscape.height * FIXED_SOFT_KEYBOARD_PANEL_RATIO;
    uint32_t landscapeAvoidY = display.landscape.height - landscapeAvoidHeight;
    EnhancedLayoutParams params = {
        .isFullScreen = true,
        .portrait = { {}, static_cast<int32_t>(portraitAvoidY), 0 },
        .landscape = { {}, static_cast<int32_t>(landscapeAvoidY), 0 },
    };
    HotAreas hotAreas;
    auto ret = inputMethodPanel->AdjustPanelRect(panelFlag, params, hotAreas);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ImmersiveEffect immersiveEffect;
    immersiveEffect = { .gradientHeight = 1,
        .gradientMode = GradientMode::LINEAR_GRADIENT,
        .fluidLightMode = FluidLightMode::BACKGROUND_FLUID_LIGHT };
    ret = inputMethodPanel->SetImmersiveEffect(immersiveEffect);
    EXPECT_EQ(ret, ErrorCode::ERROR_PARAMETER_CHECK_FAILED);
}

/**
 * @tc.name: testSetImmersiveEffect_009
 * @tc.desc: Test SetImmersiveEffect, FullScreenPrepare success.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPanelAdjustTest, testSetImmersiveEffect_009, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelAdjustTest testSetImmersiveEffect_009 Test START");
    InputMethodPanelAdjustTest::SetImmersiveCapacitySupport(true);
    IdentityCheckerMock::SetSystemApp(true);
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    InputMethodPanelAdjustTest::ImaCreatePanel(panelInfo, inputMethodPanel);
    inputMethodPanel->immersiveMode_ = ImmersiveMode::IMMERSIVE;

    DisplaySize display;
    ASSERT_EQ(InputMethodPanelAdjustTest::GetDisplaySize(display), ErrorCode::NO_ERROR);
    PanelFlag panelFlag = PanelFlag::FLG_FIXED;
    uint32_t portraitAvoidHeight = display.portrait.height * FIXED_SOFT_KEYBOARD_PANEL_RATIO - 1;
    uint32_t portraitAvoidY = display.portrait.height - portraitAvoidHeight;
    uint32_t landscapeAvoidHeight = display.landscape.height * FIXED_SOFT_KEYBOARD_PANEL_RATIO - 1;
    uint32_t landscapeAvoidY = display.landscape.height - landscapeAvoidHeight;
    EnhancedLayoutParams params = {
        .isFullScreen = true,
        .portrait = { {}, static_cast<int32_t>(portraitAvoidY), 0 },
        .landscape = { {}, static_cast<int32_t>(landscapeAvoidY), 0 },
    };
    HotAreas hotAreas;
    auto ret = inputMethodPanel->AdjustPanelRect(panelFlag, params, hotAreas);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = inputMethodPanel->ShowPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ImmersiveEffect immersiveEffect;
    immersiveEffect = { .gradientHeight = 1,
        .gradientMode = GradientMode::LINEAR_GRADIENT,
        .fluidLightMode = FluidLightMode::BACKGROUND_FLUID_LIGHT };
    ret = inputMethodPanel->SetImmersiveEffect(immersiveEffect);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}
} // namespace MiscServices
} // namespace OHOS