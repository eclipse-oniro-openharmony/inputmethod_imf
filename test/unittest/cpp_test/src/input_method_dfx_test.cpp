/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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
#include "ime_cfg_manager.h"
#include "input_method_ability.h"
#include "input_method_controller.h"
#include "input_method_system_ability.h"
#include "inputmethod_sysevent.h"
#include "task_manager.h"
#undef private

#include <gtest/gtest.h>
#include <sys/time.h>
#include <unistd.h>

#include <cstdint>
#include <string>

#include "global.h"
#include "hisysevent_base_manager.h"
#include "hisysevent_listener.h"
#include "hisysevent_manager.h"
#include "hisysevent_query_callback.h"
#include "hisysevent_record.h"
#include "identity_checker_mock.h"
#include "input_method_ability.h"
#include "input_method_controller.h"
#include "input_method_engine_listener_impl.h"
#include "tdd_util.h"
#include "text_listener.h"

using namespace testing::ext;
using namespace OHOS::HiviewDFX;
namespace OHOS {
namespace MiscServices {
using WindowMgr = TddUtil::WindowManager;
constexpr const char *CMD1 = "hidumper -s 3703 -a -a";
constexpr const char *CMD2 = "hidumper -s 3703 -a -h";
constexpr const char *CMD3 = "hidumper -s 3703 -a -test";
constexpr const char *PARAM_KEY = "OPERATE_INFO";
constexpr const char *STATE = "STATE";
constexpr const char *PID = "PID";
constexpr const char *BUNDLE_NAME = "BUNDLE_NAME";
constexpr const char *DOMAIN = "INPUTMETHOD";
constexpr const char *OPERATE_SOFTKEYBOARD_EVENT_NAME = "OPERATE_SOFTKEYBOARD";
constexpr const char *IME_STATE_CHANGED_EVENT_NAME = "IME_STATE_CHANGED";

class Watcher : public HiSysEventListener {
public:
    explicit Watcher(const std::string &operateInfo) : operateInfo_(operateInfo) { }
    virtual ~Watcher() { }
    void OnEvent(std::shared_ptr<HiSysEventRecord> sysEvent) final
    {
        if (sysEvent == nullptr) {
            IMSA_HILOGE("sysEvent is nullptr!");
            return;
        }
        std::string result;
        sysEvent->GetParamValue(PARAM_KEY, result);
        IMSA_HILOGI("result = %{public}s", result.c_str());
        if (result != operateInfo_) {
            IMSA_HILOGE("string is not matched.");
            return;
        }
        std::unique_lock<std::mutex> lock(cvMutex_);
        watcherCv_.notify_all();
    }
    void OnServiceDied() final
    {
        IMSA_HILOGE("Watcher::OnServiceDied");
    }
    std::mutex cvMutex_;
    std::condition_variable watcherCv_;

private:
    std::string operateInfo_;
};

class WatcherImeChange : public HiSysEventListener {
public:
    explicit WatcherImeChange(const std::string &state, const std::string &pid, const std::string &bundleName)
        : state_(state), pid_(pid), bundleName_(bundleName)
    {
    }
    virtual ~WatcherImeChange() { }
    void OnEvent(std::shared_ptr<HiSysEventRecord> sysEvent) final
    {
        if (sysEvent == nullptr) {
            IMSA_HILOGE("sysEvent is nullptr!");
            return;
        }
        std::string pid;
        std::string state;
        std::string bundleName;
        sysEvent->GetParamValue(STATE, state);
        sysEvent->GetParamValue(PID, pid);
        sysEvent->GetParamValue(BUNDLE_NAME, bundleName);
        IMSA_HILOGI("bundleName: %{public}s, state: %{public}s, pid: %{public}s", bundleName.c_str(), state.c_str(),
            pid.c_str());
        if (state != state_ || pid != pid_ || bundleName != bundleName_) {
            IMSA_HILOGE("string is not matched.");
            return;
        }
        std::unique_lock<std::mutex> lock(cvMutexImeChange_);
        watcherCvImeChange_.notify_all();
    }
    void OnServiceDied() final
    {
        IMSA_HILOGE("WatcherImeChange::OnServiceDied");
    }
    std::mutex cvMutexImeChange_;
    std::condition_variable watcherCvImeChange_;

private:
    std::string state_;
    std::string pid_;
    std::string bundleName_;
};

class InputMethodDfxTest : public testing::Test {
public:
    using ExecFunc = std::function<void()>;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    static bool WriteAndWatch(const std::shared_ptr<Watcher> &watcher, const InputMethodDfxTest::ExecFunc &exec);
    static bool WriteAndWatchImeChange(
        const std::shared_ptr<WatcherImeChange> &watcher, const InputMethodDfxTest::ExecFunc &exec);
    void SetUp();
    void TearDown();
    static sptr<InputMethodController> inputMethodController_;
    static sptr<OnTextChangedListener> textListener_;
    static InputMethodAbility &inputMethodAbility_;
    static std::shared_ptr<InputMethodEngineListenerImpl> imeListener_;
    static sptr<InputMethodSystemAbility> imsa_;
private:
    static uint32_t reportIntervalTime;
};
sptr<InputMethodController> InputMethodDfxTest::inputMethodController_;
sptr<OnTextChangedListener> InputMethodDfxTest::textListener_;
InputMethodAbility &InputMethodDfxTest::inputMethodAbility_ = InputMethodAbility::GetInstance();
std::shared_ptr<InputMethodEngineListenerImpl> InputMethodDfxTest::imeListener_;
sptr<InputMethodSystemAbility> InputMethodDfxTest::imsa_;
uint32_t InputMethodDfxTest::reportIntervalTime = 10; //10minutes

bool InputMethodDfxTest::WriteAndWatch(
    const std::shared_ptr<Watcher> &watcher, const InputMethodDfxTest::ExecFunc &exec)
{
    auto currentTime = std::chrono::steady_clock::now();
    const auto& lastTime = InputMethodSysEvent::GetLastOperateTime();
    // Check if the last trigger was more than 10 minutes ago
    if (static_cast<uint32_t>(
            std::chrono::duration_cast<std::chrono::minutes>(currentTime - lastTime).count()) <
        reportIntervalTime) {
        IMSA_HILOGI("Event triggered within 10 minutes, skipping report.");
        return true;
    }
    OHOS::HiviewDFX::ListenerRule listenerRule(
        DOMAIN, OPERATE_SOFTKEYBOARD_EVENT_NAME, "", OHOS::HiviewDFX::RuleType::WHOLE_WORD);
    std::vector<OHOS::HiviewDFX::ListenerRule> sysRules;
    sysRules.emplace_back(listenerRule);
    auto ret = OHOS::HiviewDFX::HiSysEventManager::AddListener(watcher, sysRules);
    if (ret != SUCCESS) {
        IMSA_HILOGE("AddListener failed! ret = %{public}d", ret);
        return false;
    }
    std::unique_lock<std::mutex> lock(watcher->cvMutex_);
    exec();
    bool result = watcher->watcherCv_.wait_for(lock, std::chrono::seconds(1)) != std::cv_status::timeout;
    ret = OHOS::HiviewDFX::HiSysEventManager::RemoveListener(watcher);
    if (ret != SUCCESS || !result) {
        IMSA_HILOGE("RemoveListener ret = %{public}d, wait_for result = %{public}s", ret, result ? "true" : "false");
        return false;
    }
    return true;
}

bool InputMethodDfxTest::WriteAndWatchImeChange(
    const std::shared_ptr<WatcherImeChange> &watcher, const InputMethodDfxTest::ExecFunc &exec)
{
    auto currentTime = std::chrono::steady_clock::now();
    const auto& lastTime = InputMethodSysEvent::GetLastOperateTime();
    // Check if the last trigger was more than 10 minutes ago
    if (static_cast<uint32_t>(
            std::chrono::duration_cast<std::chrono::minutes>(currentTime - lastTime).count()) <
        reportIntervalTime) {
        IMSA_HILOGI("Event triggered within 10 minutes, skipping report.");
        return true;
    }
    OHOS::HiviewDFX::ListenerRule listenerRule(
        DOMAIN, IME_STATE_CHANGED_EVENT_NAME, "", OHOS::HiviewDFX::RuleType::WHOLE_WORD);
    std::vector<OHOS::HiviewDFX::ListenerRule> sysRules;
    sysRules.emplace_back(listenerRule);
    auto ret = OHOS::HiviewDFX::HiSysEventManager::AddListener(watcher, sysRules);
    if (ret != SUCCESS) {
        IMSA_HILOGE("AddListener failed! ret = %{public}d", ret);
        return false;
    }
    std::unique_lock<std::mutex> lock(watcher->cvMutexImeChange_);
    exec();
    bool result = watcher->watcherCvImeChange_.wait_for(lock, std::chrono::seconds(1)) != std::cv_status::timeout;
    ret = OHOS::HiviewDFX::HiSysEventManager::RemoveListener(watcher);
    if (ret != SUCCESS || !result) {
        IMSA_HILOGE("RemoveListener ret = %{public}d, wait_for result = %{public}s", ret, result ? "true" : "false");
        return false;
    }
    return true;
}

void InputMethodDfxTest::SetUpTestCase(void)
{
    IMSA_HILOGI("InputMethodDfxTest::SetUpTestCase");
    IdentityCheckerMock::ResetParam();
    imsa_ = new (std::nothrow) InputMethodSystemAbility();
    if (imsa_ == nullptr) {
        return;
    }
    imsa_->OnStart();
    imsa_->userId_ = TddUtil::GetCurrentUserId();
    imsa_->identityChecker_ = std::make_shared<IdentityCheckerMock>();
    IdentityCheckerMock::SetFocused(true);
 
    imeListener_ = std::make_shared<InputMethodEngineListenerImpl>();
    inputMethodAbility_.abilityManager_ = imsa_;
    TddUtil::InitCurrentImePermissionInfo();
    IdentityCheckerMock::SetBundleName(TddUtil::currentBundleNameMock_);
    inputMethodAbility_.SetCoreAndAgent();
    inputMethodAbility_.SetImeListener(imeListener_);

    inputMethodController_ = InputMethodController::GetInstance();
    inputMethodController_->abilityManager_ = imsa_;
    textListener_ = new TextListener();
}

void InputMethodDfxTest::TearDownTestCase(void)
{
    IMSA_HILOGI("InputMethodDfxTest::TearDownTestCase");
    IdentityCheckerMock::ResetParam();
    imsa_->OnStop();
    ImeCfgManager::GetInstance().imeConfigs_.clear();
}

void InputMethodDfxTest::SetUp(void)
{
    IMSA_HILOGI("InputMethodDfxTest::SetUp");
    TaskManager::GetInstance().SetInited(true);
}

void InputMethodDfxTest::TearDown(void)
{
    IMSA_HILOGI("InputMethodDfxTest::TearDown");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    TaskManager::GetInstance().Reset();
}

/**
 * @tc.name: InputMethodDfxTest_DumpAllMethod_001
 * @tc.desc: DumpAllMethod
 * @tc.type: FUNC
 * @tc.require: issueI61PMG
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodDfxTest, InputMethodDfxTest_DumpAllMethod_001, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodDfxTest::InputMethodDfxTest_DumpAllMethod_001");
    std::string result;
    auto ret = TddUtil::ExecuteCmd(CMD1, result);
    EXPECT_TRUE(ret);
    EXPECT_NE(result.find("imeList"), std::string::npos);
    EXPECT_NE(result.find("com.example.testIme"), std::string::npos);
}

/**
 * @tc.name: InputMethodDfxTest_Dump_ShowHelp_001
 * @tc.desc: Dump ShowHelp.
 * @tc.type: FUNC
 * @tc.require: issueI61PMG
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodDfxTest, InputMethodDfxTest_Dump_ShowHelp_001, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodDfxTest::InputMethodDfxTest_Dump_ShowHelp_001");
    std::string result;
    auto ret = TddUtil::ExecuteCmd(CMD2, result);
    EXPECT_TRUE(ret);
    EXPECT_NE(result.find("Description:"), std::string::npos);
    EXPECT_NE(result.find("-h show help"), std::string::npos);
    EXPECT_NE(result.find("-a dump all input methods"), std::string::npos);
}

/**
 * @tc.name: InputMethodDfxTest_Dump_ShowIllealInformation_001
 * @tc.desc: Dump ShowIllealInformation.
 * @tc.type: FUNC
 * @tc.require: issueI61PMG
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodDfxTest, InputMethodDfxTest_Dump_ShowIllealInformation_001, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodDfxTest::InputMethodDfxTest_Dump_ShowIllealInformation_001");
    std::string result;
    auto ret = TddUtil::ExecuteCmd(CMD3, result);
    EXPECT_TRUE(ret);
    EXPECT_NE(result.find("input dump parameter error,enter '-h' for usage."), std::string::npos);
}

/**
 * @tc.name: InputMethodDfxTest_Hisysevent_Attach
 * @tc.desc: Hisysevent attach.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodDfxTest, InputMethodDfxTest_Hisysevent_Attach, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodDfxTest::InputMethodDfxTest_Hisysevent_Attach");
    auto watcher = std::make_shared<Watcher>(
        InputMethodSysEvent::GetInstance().GetOperateInfo(static_cast<int32_t>(OperateIMEInfoCode::IME_SHOW_ATTACH)));
    auto attach = []() {
        inputMethodController_->Attach(textListener_, true);
    };
    EXPECT_TRUE(InputMethodDfxTest::WriteAndWatch(watcher, attach));
}

/**
 * @tc.name: InputMethodDfxTest_Hisysevent_HideTextInput
 * @tc.desc: Hisysevent HideTextInput.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodDfxTest, InputMethodDfxTest_Hisysevent_HideTextInput, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodDfxTest::InputMethodDfxTest_Hisysevent_HideTextInput");
    auto ret = inputMethodController_->Attach(textListener_, true);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    auto watcher = std::make_shared<Watcher>(InputMethodSysEvent::GetInstance().GetOperateInfo(
        static_cast<int32_t>(OperateIMEInfoCode::IME_HIDE_UNEDITABLE)));
    auto hideTextInput = []() {
        inputMethodController_->HideTextInput();
    };
    EXPECT_TRUE(InputMethodDfxTest::WriteAndWatch(watcher, hideTextInput));
}

/**
 * @tc.name: InputMethodDfxTest_Hisysevent_ShowTextInput
 * @tc.desc: Hisysevent ShowTextInput.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodDfxTest, InputMethodDfxTest_Hisysevent_ShowTextInput, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodDfxTest::InputMethodDfxTest_Hisysevent_ShowTextInput");
    auto watcher = std::make_shared<Watcher>(InputMethodSysEvent::GetInstance().GetOperateInfo(
        static_cast<int32_t>(OperateIMEInfoCode::IME_SHOW_ENEDITABLE)));
    auto showTextInput = []() {
        inputMethodController_->ShowTextInput();
    };
    EXPECT_TRUE(InputMethodDfxTest::WriteAndWatch(watcher, showTextInput));
}

/**
 * @tc.name: InputMethodDfxTest_Hisysevent_HideCurrentInput
 * @tc.desc: Hisysevent HideCurrentInput.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodDfxTest, InputMethodDfxTest_Hisysevent_HideCurrentInput, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodDfxTest::InputMethodDfxTest_Hisysevent_HideCurrentInput");
    auto watcher = std::make_shared<Watcher>(
        InputMethodSysEvent::GetInstance().GetOperateInfo(static_cast<int32_t>(OperateIMEInfoCode::IME_HIDE_NORMAL)));
    auto hideCurrentInput = []() {
        inputMethodController_->HideCurrentInput();
    };
    EXPECT_TRUE(InputMethodDfxTest::WriteAndWatch(watcher, hideCurrentInput));
}

/**
 * @tc.name: InputMethodDfxTest_Hisysevent_ShowCurrentInput
 * @tc.desc: Hisysevent ShowCurrentInput.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodDfxTest, InputMethodDfxTest_Hisysevent_ShowCurrentInput, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodDfxTest::InputMethodDfxTest_Hisysevent_ShowCurrentInput");
    auto watcher = std::make_shared<Watcher>(
        InputMethodSysEvent::GetInstance().GetOperateInfo(static_cast<int32_t>(OperateIMEInfoCode::IME_SHOW_NORMAL)));
    auto showCurrentInput = []() {
        inputMethodController_->ShowCurrentInput();
    };
    EXPECT_TRUE(InputMethodDfxTest::WriteAndWatch(watcher, showCurrentInput));
}

/**
 * @tc.name: InputMethodDfxTest_Hisysevent_HideSoftKeyboard
 * @tc.desc: Hisysevent HideSoftKeyboard.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodDfxTest, InputMethodDfxTest_Hisysevent_HideSoftKeyboard, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodDfxTest::InputMethodDfxTest_Hisysevent_HideSoftKeyboard");
    auto watcher = std::make_shared<Watcher>(
        InputMethodSysEvent::GetInstance().GetOperateInfo(static_cast<int32_t>(OperateIMEInfoCode::IME_HIDE_NORMAL)));
    auto hideSoftKeyboard = []() {
        inputMethodController_->HideSoftKeyboard();
    };
    EXPECT_TRUE(InputMethodDfxTest::WriteAndWatch(watcher, hideSoftKeyboard));
}

/**
 * @tc.name: InputMethodDfxTest_Hisysevent_ShowSoftKeyboard
 * @tc.desc: Hisysevent ShowSoftKeyboard.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodDfxTest, InputMethodDfxTest_Hisysevent_ShowSoftKeyboard, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodDfxTest::InputMethodDfxTest_Hisysevent_ShowSoftKeyboard");
    auto watcher = std::make_shared<Watcher>(
        InputMethodSysEvent::GetInstance().GetOperateInfo(static_cast<int32_t>(OperateIMEInfoCode::IME_SHOW_NORMAL)));
    auto showSoftKeyboard = []() {
        inputMethodController_->ShowSoftKeyboard();
    };
    EXPECT_TRUE(InputMethodDfxTest::WriteAndWatch(watcher, showSoftKeyboard));
}

/**
 * @tc.name: InputMethodDfxTest_Hisysevent_HideKeyboardSelf
 * @tc.desc: Hisysevent HideKeyboardSelf.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodDfxTest, InputMethodDfxTest_Hisysevent_HideKeyboardSelf, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodDfxTest::InputMethodDfxTest_Hisysevent_HideKeyboardSelf");
    auto watcher = std::make_shared<Watcher>(
        InputMethodSysEvent::GetInstance().GetOperateInfo(static_cast<int32_t>(OperateIMEInfoCode::IME_HIDE_SELF)));
    auto hideKeyboardSelf = []() {
        inputMethodAbility_.HideKeyboardSelf();
    };
    EXPECT_TRUE(InputMethodDfxTest::WriteAndWatch(watcher, hideKeyboardSelf));
}

/**
 * @tc.name: InputMethodDfxTest_Hisysevent_Close
 * @tc.desc: Hisysevent Close.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodDfxTest, InputMethodDfxTest_Hisysevent_Close, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodDfxTest::InputMethodDfxTest_Hisysevent_Close");
    auto watcher = std::make_shared<Watcher>(
        InputMethodSysEvent::GetInstance().GetOperateInfo(static_cast<int32_t>(OperateIMEInfoCode::IME_UNBIND)));
    auto close = []() {
        inputMethodController_->Close();
    };
    EXPECT_TRUE(InputMethodDfxTest::WriteAndWatch(watcher, close));
}

/**
 * @tc.name: InputMethodDfxTest_Hisysevent_UnBind
 * @tc.desc: Hisysevent UnBind.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodDfxTest, InputMethodDfxTest_Hisysevent_UnBind, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodDfxTest::InputMethodDfxTest_Hisysevent_UnBind");
    auto watcherImeChange = std::make_shared<WatcherImeChange>(std::to_string(static_cast<int32_t>(ImeState::UNBIND)),
        std::to_string(static_cast<int32_t>(getpid())), TddUtil::currentBundleNameMock_);
    auto imeStateUnBind = []() {
        inputMethodController_->Attach(textListener_, true);
        inputMethodController_->Close();
    };
    EXPECT_TRUE(InputMethodDfxTest::WriteAndWatchImeChange(watcherImeChange, imeStateUnBind));
}

/**
 * @tc.name: InputMethodDfxTest_Hisysevent_Bind
 * @tc.desc: Hisysevent Bind.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodDfxTest, InputMethodDfxTest_Hisysevent_Bind, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodDfxTest::InputMethodDfxTest_Hisysevent_Bind");
    auto watcherImeChange = std::make_shared<WatcherImeChange>(std::to_string(static_cast<int32_t>(ImeState::BIND)),
        std::to_string(static_cast<int32_t>(getpid())), TddUtil::currentBundleNameMock_);
    auto imeStateBind = []() {
        inputMethodController_->RequestShowInput();
    };
    EXPECT_TRUE(InputMethodDfxTest::WriteAndWatchImeChange(watcherImeChange, imeStateBind));
}

/**
 * @tc.name: InputMethod_Dump_HELP
 * @tc.desc: InputMethodDump.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodDfxTest, InputMethodDfxTest_Dump_HELP, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodDump::InputMethod_Dump_HELP");
    std::vector<std::string> args = { "-h" };
    int fd = 1;
    EXPECT_TRUE(InputmethodDump::GetInstance().Dump(fd, args));
}

/**
 * @tc.name: InputMethod_Dump_ALL
 * @tc.desc: InputMethodDump.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodDfxTest, InputMethodDfxTest_Dump_ALL, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodDump::InputMethod_Dump_ALL");
    std::vector<std::string> args = { "-a" };
    int fd = 1;
    EXPECT_FALSE(!InputmethodDump::GetInstance().Dump(fd, args));
}

/**
 * @tc.name: InputMethodDfxTest_Hisysevent_GetOperateAction
 * @tc.desc: Hisysevent GetOperateAction.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodDfxTest, InputMethodDfxTest_Hisysevent_GetOperateAction, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodDfxTest::InputMethodDfxTest_Hisysevent_GetOperateAction");
    std::string ret = "";
    ret = InputMethodSysEvent::GetInstance().GetOperateAction(
        static_cast<int32_t>(OperateIMEInfoCode::IME_SHOW_ATTACH));
    ret = InputMethodSysEvent::GetInstance().GetOperateAction(
        static_cast<int32_t>(OperateIMEInfoCode::IME_SHOW_ENEDITABLE));
    ret = InputMethodSysEvent::GetInstance().GetOperateAction(
        static_cast<int32_t>(OperateIMEInfoCode::IME_SHOW_NORMAL));
    EXPECT_TRUE(ret == "show");

    ret = InputMethodSysEvent::GetInstance().GetOperateAction(static_cast<int32_t>(OperateIMEInfoCode::IME_UNBIND));
    EXPECT_TRUE(ret == "unbind");
    ret = InputMethodSysEvent::GetInstance().GetOperateAction(
        static_cast<int32_t>(OperateIMEInfoCode::IME_HIDE_UNBIND));
    EXPECT_TRUE(ret == "hide and unbind");

    ret = InputMethodSysEvent::GetInstance().GetOperateAction(
        static_cast<int32_t>(OperateIMEInfoCode::IME_HIDE_UNEDITABLE));
    ret = InputMethodSysEvent::GetInstance().GetOperateAction(
        static_cast<int32_t>(OperateIMEInfoCode::IME_HIDE_NORMAL));
    ret = InputMethodSysEvent::GetInstance().GetOperateAction(
        static_cast<int32_t>(OperateIMEInfoCode::IME_HIDE_UNFOCUSED));
    ret = InputMethodSysEvent::GetInstance().GetOperateAction(static_cast<int32_t>(OperateIMEInfoCode::IME_HIDE_SELF));
    EXPECT_TRUE(ret == "hide");

    int32_t invalidNum = -1;
    ret = InputMethodSysEvent::GetInstance().GetOperateAction(invalidNum);
    EXPECT_TRUE(ret == "unknow action.");
}
} // namespace MiscServices
} // namespace OHOS
