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

#include "display_manager.h"
#include "global.h"
#include "input_method_ability.h"
#include "input_method_controller.h"
#include "input_method_engine_listener_impl.h"
#include "panel_status_listener.h"
#include "tdd_util.h"
#include "text_listener.h"

using namespace testing::ext;
namespace OHOS {
namespace MiscServices {
constexpr uint32_t IMC_WAIT_PANEL_STATUS_LISTEN_TIME = 200;
constexpr float FIXED_SOFT_KEYBOARD_PANEL_RATIO = 0.7;
constexpr float NON_FIXED_SOFT_KEYBOARD_PANEL_RATIO = 1;
enum ListeningStatus : uint32_t { ON, OFF, NONE };
class InputMethodPanelTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static std::shared_ptr<InputMethodPanel> CreatePanel();
    static void InitPanel();
    static void ClearPanel();
    static void TriggerShowCallback(std::shared_ptr<InputMethodPanel> &inputMethodPanel);
    static void TriggerHideCallback(std::shared_ptr<InputMethodPanel> &inputMethodPanel);
    static void ImcPanelListeningTestRestore();
    static void ImcPanelShowNumCheck(uint32_t num);
    static void ImcPanelHideNumCheck(uint32_t num);
    static void ImcPanelShowInfoCheck(const InputWindowInfo &windowInfo);
    static void ImcPanelHideInfoCheck(const InputWindowInfo &windowInfo);
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
            showPanel_ = isShow;
            hidePanel_ = !isShow;
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
    static bool showPanel_;
    static bool hidePanel_;
    static std::condition_variable panelListenerCv_;
    static std::mutex panelListenerLock_;
    static constexpr uint32_t DELAY_TIME = 100;
    static constexpr int32_t INTERVAL = 10;
    static std::shared_ptr<AppExecFwk::EventHandler> panelHandler_;
    static uint64_t testTokenId_;
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
bool InputMethodPanelTest::showPanel_ = false;
bool InputMethodPanelTest::hidePanel_ = false;
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
uint64_t InputMethodPanelTest::testTokenId_ = 0;
sptr<OnTextChangedListener> InputMethodPanelTest::textListener_{ nullptr };
std::shared_ptr<InputMethodEngineListener> InputMethodPanelTest::imeListener_{ nullptr };
std::shared_ptr<InputMethodPanel> InputMethodPanelTest::inputMethodPanel_{ nullptr };
std::shared_ptr<InputMethodPanel> InputMethodPanelTest::inputMethodStatusBar_{ nullptr };
void InputMethodPanelTest::SetUpTestCase(void)
{
    IMSA_HILOGI("InputMethodPanelTest::SetUpTestCase");
    // storage current token id
    TddUtil::StorageSelfTokenID();
    ima_ = InputMethodAbility::GetInstance();
    auto listener = std::make_shared<InputMethodSettingListenerImpl>();
    imc_ = InputMethodController::GetInstance();
    textListener_ = new (std::nothrow) TextListener();
    imeListener_ = std::make_shared<InputMethodEngineListenerImpl>();
    // set token as current input method
    std::shared_ptr<Property> property = InputMethodController::GetInstance()->GetCurrentInputMethod();
    std::string bundleName = property != nullptr ? property->name : "default.inputmethod.unittest";
    testTokenId_ = TddUtil::AllocTestTokenID(true, bundleName, {});
    InitPanel();
}

void InputMethodPanelTest::TearDownTestCase(void)
{
    IMSA_HILOGI("InputMethodPanelTest::TearDownTestCase");
    ClearPanel();
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
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo = { .panelType = SOFT_KEYBOARD, .panelFlag = FLG_FIXED };
    auto ret = inputMethodPanel->CreatePanel(nullptr, panelInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    return inputMethodPanel;
}

void InputMethodPanelTest::InitPanel()
{
    IMSA_HILOGI("start");
    TddUtil::SetTestTokenID(testTokenId_);
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

    TddUtil::RestoreSelfTokenID();
    IMSA_HILOGI("end");
}

void InputMethodPanelTest::ClearPanel()
{
    if (inputMethodPanel_ == nullptr) {
        IMSA_HILOGD("nullptr");
        return;
    }
    inputMethodPanel_->DestroyPanel();
}

void InputMethodPanelTest::TriggerShowCallback(std::shared_ptr<InputMethodPanel> &inputMethodPanel)
{
    panelHandler_->PostTask([&inputMethodPanel]() { inputMethodPanel->ShowPanel(); }, InputMethodPanelTest::INTERVAL);
    {
        std::unique_lock<std::mutex> lock(InputMethodPanelTest::panelListenerLock_);
        InputMethodPanelTest::panelListenerCv_.wait_for(lock,
            std::chrono::milliseconds(InputMethodPanelTest::DELAY_TIME),
            [] { return InputMethodPanelTest::showPanel_; });
    }
}

void InputMethodPanelTest::TriggerHideCallback(std::shared_ptr<InputMethodPanel> &inputMethodPanel)
{
    panelHandler_->PostTask([&inputMethodPanel]() { inputMethodPanel->HidePanel(); }, InputMethodPanelTest::INTERVAL);
    {
        std::unique_lock<std::mutex> lock(InputMethodPanelTest::panelListenerLock_);
        InputMethodPanelTest::panelListenerCv_.wait_for(lock,
            std::chrono::milliseconds(InputMethodPanelTest::DELAY_TIME),
            [] { return InputMethodPanelTest::hidePanel_; });
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
    EXPECT_EQ(windowInfo_.width, windowInfo.width);
    EXPECT_EQ(windowInfo_.height, windowInfo.height);
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
    EXPECT_EQ(windowInfo_.width, windowInfo.width);
    EXPECT_EQ(windowInfo_.height, windowInfo.height);
}

void InputMethodPanelTest::ImcPanelListeningTestRestore()
{
    status_ = InputWindowStatus::NONE;
    windowInfo_ = {};
    imeShowCallbackNum_ = 0;
    imeHideCallbackNum_ = 0;
}

/**
* @tc.name: testCreatePanel
* @tc.desc: Test CreatePanel.
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

    PanelInfo panelInfo = { .panelType = SOFT_KEYBOARD, .panelFlag = FLG_FLOATING };
    ret = inputMethodPanel->CreatePanel(nullptr, panelInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    auto defaultDisplay = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    EXPECT_TRUE(defaultDisplay != nullptr);
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
    ret = inputMethodPanel->CreatePanel(nullptr, panelInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    auto defaultDisplay = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    EXPECT_TRUE(defaultDisplay != nullptr);
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
    TddUtil::SetTestTokenID(InputMethodPanelTest::testTokenId_);
    TddUtil::InitWindow(true);
    InputMethodPanelTest::ima_->SetImeListener(InputMethodPanelTest::imeListener_);
    int32_t ret = ima_->SetCoreAndAgent();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = InputMethodPanelTest::imc_->Attach(InputMethodPanelTest::textListener_, false);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    bool isShown = false;
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo = { .panelType = SOFT_KEYBOARD, .panelFlag = FLG_FIXED };
    ret = ima_->CreatePanel(nullptr, panelInfo, inputMethodPanel);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    // query when fixed soft keyboard is showing
    ret = inputMethodPanel->ShowPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = imc_->IsPanelShown(panelInfo, isShown);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(isShown);

    // query when fixed soft keyboard is hidden
    ret = inputMethodPanel->HidePanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = imc_->IsPanelShown(panelInfo, isShown);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_FALSE(isShown);

    ret = ima_->DestroyPanel(inputMethodPanel);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
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
    TddUtil::SetTestTokenID(InputMethodPanelTest::testTokenId_);
    TddUtil::InitWindow(true);
    InputMethodPanelTest::ima_->SetImeListener(InputMethodPanelTest::imeListener_);
    int32_t ret = ima_->SetCoreAndAgent();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = InputMethodPanelTest::imc_->Attach(InputMethodPanelTest::textListener_, false);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    bool isShown = false;
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo = { .panelType = SOFT_KEYBOARD, .panelFlag = FLG_FIXED };
    ret = ima_->CreatePanel(nullptr, panelInfo, inputMethodPanel);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ret = inputMethodPanel->ChangePanelFlag(PanelFlag::FLG_FLOATING);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = inputMethodPanel->ShowPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    // query panel with old info when panel changes its flag.
    ret = imc_->IsPanelShown(panelInfo, isShown);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_FALSE(isShown);
    // query panel with updated shown one's info when panel changes its flag.
    panelInfo.panelFlag = PanelFlag::FLG_FLOATING;
    ret = imc_->IsPanelShown(panelInfo, isShown);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(isShown);

    ret = ima_->DestroyPanel(inputMethodPanel);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
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
    TddUtil::SetTestTokenID(InputMethodPanelTest::testTokenId_);
    TddUtil::InitWindow(true);
    InputMethodPanelTest::ima_->SetImeListener(InputMethodPanelTest::imeListener_);
    int32_t ret = ima_->SetCoreAndAgent();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = InputMethodPanelTest::imc_->Attach(InputMethodPanelTest::textListener_, false);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    bool isShown = false;
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo = { .panelType = STATUS_BAR };
    ret = ima_->CreatePanel(nullptr, panelInfo, inputMethodPanel);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    // query status bar's status when it is showing
    ret = inputMethodPanel->ShowPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = imc_->IsPanelShown(panelInfo, isShown);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(isShown);

    // query status bar's status when it is hidden
    ret = inputMethodPanel->HidePanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = imc_->IsPanelShown(panelInfo, isShown);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_FALSE(isShown);

    ret = ima_->DestroyPanel(inputMethodPanel);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
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
    std::string type = "show";
    inputMethodPanel->SetPanelStatusListener(statusListener, type);
    type = "hide";
    inputMethodPanel->SetPanelStatusListener(statusListener, type);

    PanelInfo panelInfo = { .panelType = SOFT_KEYBOARD, .panelFlag = FLG_FIXED };
    auto ret = inputMethodPanel->CreatePanel(nullptr, panelInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    InputMethodPanelTest::TriggerShowCallback(inputMethodPanel);
    EXPECT_TRUE(InputMethodPanelTest::showPanel_);

    InputMethodPanelTest::TriggerHideCallback(inputMethodPanel);
    EXPECT_TRUE(InputMethodPanelTest::hidePanel_);
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
    IMSA_HILOGI("InputMethodPanelTest::testGetPanelType start.");
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

    std::string subscribeType = "show";
    inputMethodPanel->ClearPanelListener(subscribeType);
    InputMethodPanelTest::TriggerShowCallback(inputMethodPanel);
    EXPECT_EQ(InputMethodPanelTest::showPanel_, false);
    InputMethodPanelTest::TriggerHideCallback(inputMethodPanel);
    EXPECT_EQ(InputMethodPanelTest::hidePanel_, true);
    InputMethodPanelTest::hidePanel_ = false;

    subscribeType = "hide";
    inputMethodPanel->ClearPanelListener(subscribeType);

    InputMethodPanelTest::TriggerShowCallback(inputMethodPanel);
    EXPECT_EQ(InputMethodPanelTest::showPanel_, false);
    InputMethodPanelTest::TriggerHideCallback(inputMethodPanel);
    EXPECT_EQ(InputMethodPanelTest::hidePanel_, false);

    auto ret = inputMethodPanel->DestroyPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
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
    std::string type = "show";
    inputMethodPanel->SetPanelStatusListener(statusListener, type);
    type = "hide";
    inputMethodPanel->SetPanelStatusListener(statusListener, type);

    InputMethodPanelTest::TriggerShowCallback(inputMethodPanel);
    EXPECT_TRUE(InputMethodPanelTest::showPanel_);

    InputMethodPanelTest::TriggerHideCallback(inputMethodPanel);
    EXPECT_TRUE(InputMethodPanelTest::hidePanel_);

    type = "show";
    inputMethodPanel->ClearPanelListener(type);

    InputMethodPanelTest::TriggerShowCallback(inputMethodPanel);
    EXPECT_TRUE(!InputMethodPanelTest::showPanel_);

    InputMethodPanelTest::TriggerHideCallback(inputMethodPanel);
    EXPECT_TRUE(InputMethodPanelTest::hidePanel_);

    inputMethodPanel->SetPanelStatusListener(statusListener, type);

    InputMethodPanelTest::TriggerShowCallback(inputMethodPanel);
    EXPECT_TRUE(InputMethodPanelTest::showPanel_);

    auto ret = inputMethodPanel->DestroyPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/*
* @tc.name: testImcPanelListening_001
* @tc.desc: SOFT_KEYBOARD|FLG_FIXED  only one listener(system app)
* @tc.type: FUNC
*/
HWTEST_F(InputMethodPanelTest, testImcPanelListening_001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelTest::testImcPanelListening_001 start.");
    // set system app
    TddUtil::SetTestTokenID(testTokenId_);
    auto listener = std::make_shared<InputMethodSettingListenerImpl>();
    ImeEventMonitorManager::GetInstance().RegisterImeEventListener(EVENT_IME_SHOW_MASK, listener);
    ImeEventMonitorManager::GetInstance().RegisterImeEventListener(EVENT_IME_HIDE_MASK, listener);
    TddUtil::RestoreSelfTokenID();
    // cancel system app
    InputWindowInfo info{ "", 0, 0, InputMethodPanelTest::windowWidth_, InputMethodPanelTest::windowHeight_ };
    InputMethodPanelTest::ImcPanelListeningTestRestore();
    // set current ime
    TddUtil::SetTestTokenID(testTokenId_);
    auto ret = InputMethodPanelTest::inputMethodPanel_->ShowPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodPanelTest::ImcPanelShowNumCheck(1);
    InputMethodPanelTest::ImcPanelShowInfoCheck(info);

    InputMethodPanelTest::ImcPanelListeningTestRestore();
    ret = InputMethodPanelTest::inputMethodPanel_->HidePanel();
    TddUtil::RestoreSelfTokenID();
    // cancel current ime
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodPanelTest::ImcPanelHideNumCheck(1);
    InputMethodPanelTest::ImcPanelHideInfoCheck(info);
    // set system app
    TddUtil::SetTestTokenID(testTokenId_);
    ImeEventMonitorManager::GetInstance().UnRegisterImeEventListener(EVENT_IME_SHOW_MASK, listener);
    ImeEventMonitorManager::GetInstance().UnRegisterImeEventListener(EVENT_IME_HIDE_MASK, listener);
    TddUtil::RestoreSelfTokenID();
    // cancel system app
}

/**
* @tc.name: testImcPanelListening_002
* @tc.desc: SOFT_KEYBOARD|FLG_FLOATING  only one listener(system app)
* @tc.type: FUNC
*/
HWTEST_F(InputMethodPanelTest, testImcPanelListening_002, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelTest::testImcPanelListening_002 start.");
    // set system app
    TddUtil::SetTestTokenID(testTokenId_);
    auto listener = std::make_shared<InputMethodSettingListenerImpl>();
    ImeEventMonitorManager::GetInstance().RegisterImeEventListener(EVENT_IME_SHOW_MASK, listener);
    ImeEventMonitorManager::GetInstance().RegisterImeEventListener(EVENT_IME_HIDE_MASK, listener);
    TddUtil::RestoreSelfTokenID();
    // cancel system app
    InputWindowInfo info{ "", 0, 0, InputMethodPanelTest::windowWidth_, InputMethodPanelTest::windowHeight_ };
    InputMethodPanelTest::inputMethodPanel_->panelFlag_ = FLG_FLOATING;
    InputMethodPanelTest::ImcPanelListeningTestRestore();
    // set current ime
    TddUtil::SetTestTokenID(testTokenId_);
    auto ret = InputMethodPanelTest::inputMethodPanel_->ShowPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodPanelTest::ImcPanelShowNumCheck(1);
    InputMethodPanelTest::ImcPanelShowInfoCheck(info);

    InputMethodPanelTest::ImcPanelListeningTestRestore();
    ret = InputMethodPanelTest::inputMethodPanel_->HidePanel();
    TddUtil::RestoreSelfTokenID();
    // cancel current ime
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodPanelTest::ImcPanelHideNumCheck(1);
    InputMethodPanelTest::ImcPanelHideInfoCheck(info);
    // set system app
    TddUtil::SetTestTokenID(testTokenId_);
    ImeEventMonitorManager::GetInstance().UnRegisterImeEventListener(EVENT_IME_SHOW_MASK, listener);
    ImeEventMonitorManager::GetInstance().UnRegisterImeEventListener(EVENT_IME_HIDE_MASK, listener);
    TddUtil::RestoreSelfTokenID();
    // cancel system app
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
    // set system app
    TddUtil::SetTestTokenID(testTokenId_);
    auto listener = std::make_shared<InputMethodSettingListenerImpl>();
    ImeEventMonitorManager::GetInstance().RegisterImeEventListener(EVENT_IME_SHOW_MASK, listener);
    ImeEventMonitorManager::GetInstance().RegisterImeEventListener(EVENT_IME_HIDE_MASK, listener);
    TddUtil::RestoreSelfTokenID();
    // cancel system app
    InputMethodPanelTest::inputMethodPanel_->panelFlag_ = FLG_CANDIDATE_COLUMN;
    InputMethodPanelTest::ImcPanelListeningTestRestore();
    // set current ime
    TddUtil::SetTestTokenID(testTokenId_);
    auto ret = InputMethodPanelTest::inputMethodPanel_->ShowPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodPanelTest::ImcPanelShowNumCheck(0);

    InputMethodPanelTest::ImcPanelListeningTestRestore();
    ret = InputMethodPanelTest::inputMethodPanel_->HidePanel();
    TddUtil::RestoreSelfTokenID();
    // cancel current ime
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodPanelTest::ImcPanelHideNumCheck(0);
    // set system app
    TddUtil::SetTestTokenID(testTokenId_);
    ImeEventMonitorManager::GetInstance().UnRegisterImeEventListener(EVENT_IME_SHOW_MASK, listener);
    ImeEventMonitorManager::GetInstance().UnRegisterImeEventListener(EVENT_IME_HIDE_MASK, listener);
    TddUtil::RestoreSelfTokenID();
    // cancel system app
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
    // set system app
    TddUtil::SetTestTokenID(testTokenId_);
    auto listener = std::make_shared<InputMethodSettingListenerImpl>();
    ImeEventMonitorManager::GetInstance().RegisterImeEventListener(EVENT_IME_SHOW_MASK, listener);
    ImeEventMonitorManager::GetInstance().RegisterImeEventListener(EVENT_IME_HIDE_MASK, listener);
    TddUtil::RestoreSelfTokenID();
    // cancel system app
    InputMethodPanelTest::ImcPanelListeningTestRestore();
    // set current ime
    TddUtil::SetTestTokenID(testTokenId_);
    auto ret = InputMethodPanelTest::inputMethodStatusBar_->ShowPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodPanelTest::ImcPanelShowNumCheck(0);

    InputMethodPanelTest::ImcPanelListeningTestRestore();
    ret = InputMethodPanelTest::inputMethodStatusBar_->HidePanel();
    TddUtil::RestoreSelfTokenID();
    // cancel current ime
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodPanelTest::ImcPanelHideNumCheck(0);
    // set system app
    TddUtil::SetTestTokenID(testTokenId_);
    ImeEventMonitorManager::GetInstance().UnRegisterImeEventListener(EVENT_IME_SHOW_MASK, listener);
    ImeEventMonitorManager::GetInstance().UnRegisterImeEventListener(EVENT_IME_HIDE_MASK, listener);
    TddUtil::RestoreSelfTokenID();
    // cancel system app
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
    // set current ime
    TddUtil::SetTestTokenID(testTokenId_);
    InputMethodPanelTest::ImcPanelListeningTestRestore();
    auto ret = InputMethodPanelTest::inputMethodPanel_->ShowPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodPanelTest::ImcPanelShowNumCheck(3);

    InputMethodPanelTest::ImcPanelListeningTestRestore();
    ret = InputMethodPanelTest::inputMethodPanel_->HidePanel();
    TddUtil::RestoreSelfTokenID();
    // cancel current ime
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
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
    ImeEventMonitorManager::GetInstance().UnRegisterImeEventListener(EVENT_IME_SHOW_MASK, listener1);
    TddUtil::RestoreSelfTokenID();
    // cancel native sa
    // UnRegister one IME_SHOW listener
    InputMethodPanelTest::ImcPanelListeningTestRestore();
    // set current ime
    TddUtil::SetTestTokenID(testTokenId_);
    auto ret = InputMethodPanelTest::inputMethodPanel_->ShowPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodPanelTest::ImcPanelShowNumCheck(2);

    InputMethodPanelTest::ImcPanelListeningTestRestore();
    ret = InputMethodPanelTest::inputMethodPanel_->HidePanel();
    TddUtil::RestoreSelfTokenID();
    // cancel current ime
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
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
    // set current ime
    TddUtil::SetTestTokenID(testTokenId_);
    ret = InputMethodPanelTest::inputMethodPanel_->ShowPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodPanelTest::ImcPanelShowNumCheck(0);

    InputMethodPanelTest::ImcPanelListeningTestRestore();
    ret = InputMethodPanelTest::inputMethodPanel_->HidePanel();
    TddUtil::RestoreSelfTokenID();
    // cancel current ime
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
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
    // set current ime
    TddUtil::SetTestTokenID(testTokenId_);
    auto ret = InputMethodPanelTest::inputMethodPanel_->ShowPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodPanelTest::ImcPanelShowNumCheck(1);

    InputMethodPanelTest::ImcPanelListeningTestRestore();
    ret = InputMethodPanelTest::inputMethodPanel_->HidePanel();
    TddUtil::RestoreSelfTokenID();
    // cancel current ime
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodPanelTest::ImcPanelHideNumCheck(1);

    // UnRegister all listener
    // set native sa
    TddUtil::GrantNativePermission();
    ImeEventMonitorManager::GetInstance().UnRegisterImeEventListener(
        EVENT_IME_HIDE_MASK | EVENT_IME_SHOW_MASK, listener);
    TddUtil::RestoreSelfTokenID();
    // cancel native sa
    InputMethodPanelTest::ImcPanelListeningTestRestore();
    ret = InputMethodPanelTest::inputMethodPanel_->ShowPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodPanelTest::ImcPanelShowNumCheck(0);

    InputMethodPanelTest::ImcPanelListeningTestRestore();
    // set current ime
    TddUtil::SetTestTokenID(testTokenId_);
    ret = InputMethodPanelTest::inputMethodPanel_->HidePanel();
    TddUtil::RestoreSelfTokenID();
    // cancel current ime
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodPanelTest::ImcPanelHideNumCheck(0);
}

/**
* @tc.name: testSetCallingWindow
* @tc.desc: test SetCallingWindow
* @tc.type: FUNC
*/
HWTEST_F(InputMethodPanelTest, testSetCallingWindow, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelTest::testSetCallingWindow start.");
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    // not CreatePanel, SetCallingWindow failed
    uint32_t windowId = 8;
    auto ret = inputMethodPanel->SetCallingWindow(windowId);
    EXPECT_EQ(ret, ErrorCode::ERROR_NULL_POINTER);

    PanelInfo panelInfo = { .panelType = SOFT_KEYBOARD, .panelFlag = FLG_FIXED };
    ret = inputMethodPanel->CreatePanel(nullptr, panelInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ret = inputMethodPanel->SetCallingWindow(windowId);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}
} // namespace MiscServices
} // namespace OHOS