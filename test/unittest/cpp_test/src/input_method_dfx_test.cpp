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
#include "inputmethod_sysevent.h"
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
constexpr const char *DOMAIN = "INPUTMETHOD";
constexpr const char *EVENT_NAME = "OPERATE_SOFTKEYBOARD";

class Watcher : public HiSysEventListener {
public:
    explicit Watcher(const std::string &operateInfo) : operateInfo_(operateInfo)
    {
    }
    virtual ~Watcher()
    {
    }
    void OnEvent(std::shared_ptr<HiSysEventRecord> sysEvent) final
    {
        if (sysEvent == nullptr) {
            IMSA_HILOGE("sysEvent is nullptr!");
            return;
        }
        std::string result;
        sysEvent->GetParamValue(PARAM_KEY, result);
        IMSA_HILOGD("result = %{public}s", result.c_str());
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

class InputMethodDfxTest : public testing::Test {
public:
    using ExecFunc = std::function<void()>;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    static bool WriteAndWatch(std::shared_ptr<Watcher> watcher, InputMethodDfxTest::ExecFunc exec);
    void SetUp();
    void TearDown();
    static sptr<InputMethodController> inputMethodController_;
    static sptr<OnTextChangedListener> textListener_;
    static sptr<InputMethodAbility> inputMethodAbility_;
    static std::shared_ptr<InputMethodEngineListenerImpl> imeListener_;
};
sptr<InputMethodController> InputMethodDfxTest::inputMethodController_;
sptr<OnTextChangedListener> InputMethodDfxTest::textListener_;
sptr<InputMethodAbility> InputMethodDfxTest::inputMethodAbility_;
std::shared_ptr<InputMethodEngineListenerImpl> InputMethodDfxTest::imeListener_;

bool InputMethodDfxTest::WriteAndWatch(std::shared_ptr<Watcher> watcher, InputMethodDfxTest::ExecFunc exec)
{
    OHOS::HiviewDFX::ListenerRule listenerRule(DOMAIN, EVENT_NAME, "", OHOS::HiviewDFX::RuleType::WHOLE_WORD);
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

void InputMethodDfxTest::SetUpTestCase(void)
{
    IMSA_HILOGI("InputMethodDfxTest::SetUpTestCase");
    TddUtil::StorageSelfTokenID();
    std::shared_ptr<Property> property = InputMethodController::GetInstance()->GetCurrentInputMethod();
    std::string bundleName = property != nullptr ? property->name : "default.inputmethod.unittest";
    TddUtil::SetTestTokenID(TddUtil::GetTestTokenID(bundleName));
    inputMethodAbility_ = InputMethodAbility::GetInstance();
    imeListener_ = std::make_shared<InputMethodEngineListenerImpl>();
    inputMethodAbility_->OnImeReady();
    inputMethodAbility_->SetCoreAndAgent();
    inputMethodAbility_->SetImeListener(imeListener_);

    inputMethodController_ = InputMethodController::GetInstance();
    textListener_ = new TextListener();
    TddUtil::SetTestTokenID(TddUtil::AllocTestTokenID(true, false, "undefine"));
    TddUtil::WindowManager::RegisterFocusChangeListener();
    WindowMgr::CreateWindow();
    WindowMgr::ShowWindow();
    bool isFocused = FocusChangedListenerTestImpl::isFocused_->GetValue();
    IMSA_HILOGI("getFocus end, isFocused = %{public}d", isFocused);
}

void InputMethodDfxTest::TearDownTestCase(void)
{
    IMSA_HILOGI("InputMethodDfxTest::TearDownTestCase");
    TddUtil::RestoreSelfTokenID();
    WindowMgr::HideWindow();
    WindowMgr::DestroyWindow();
}

void InputMethodDfxTest::SetUp(void)
{
    IMSA_HILOGI("InputMethodDfxTest::SetUp");
}

void InputMethodDfxTest::TearDown(void)
{
    IMSA_HILOGI("InputMethodDfxTest::TearDown");
}

/**
* @tc.name: InputMethodDfxTest_DumpAllMethod_001
* @tc.desc: DumpAllMethod
* @tc.type: FUNC
* @tc.require: issueI61PMG
* @tc.author: chenyu
*/
HWTEST_F(InputMethodDfxTest, InputMethodDfxTest_DumpAllMethod_001, TestSize.Level0)
{
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
HWTEST_F(InputMethodDfxTest, InputMethodDfxTest_Dump_ShowHelp_001, TestSize.Level0)
{
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
HWTEST_F(InputMethodDfxTest, InputMethodDfxTest_Dump_ShowIllealInformation_001, TestSize.Level0)
{
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
HWTEST_F(InputMethodDfxTest, InputMethodDfxTest_Hisysevent_Attach, TestSize.Level0)
{
    auto watcher = std::make_shared<Watcher>(
        InputMethodSysEvent::GetInstance().GetOperateInfo(static_cast<int32_t>(OperateIMEInfoCode::IME_SHOW_ATTACH)));
    auto attach = []() { inputMethodController_->Attach(textListener_, true); };
    EXPECT_TRUE(InputMethodDfxTest::WriteAndWatch(watcher, attach));
}

/**
* @tc.name: InputMethodDfxTest_Hisysevent_HideTextInput
* @tc.desc: Hisysevent HideTextInput.
* @tc.type: FUNC
*/
HWTEST_F(InputMethodDfxTest, InputMethodDfxTest_Hisysevent_HideTextInput, TestSize.Level0)
{
    auto ret = inputMethodController_->Attach(textListener_, true);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    auto watcher = std::make_shared<Watcher>(InputMethodSysEvent::GetInstance().GetOperateInfo(
        static_cast<int32_t>(OperateIMEInfoCode::IME_HIDE_UNEDITABLE)));
    auto hideTextInput = []() { inputMethodController_->HideTextInput(); };
    EXPECT_TRUE(InputMethodDfxTest::WriteAndWatch(watcher, hideTextInput));
}

/**
* @tc.name: InputMethodDfxTest_Hisysevent_ShowTextInput
* @tc.desc: Hisysevent ShowTextInput.
* @tc.type: FUNC
*/
HWTEST_F(InputMethodDfxTest, InputMethodDfxTest_Hisysevent_ShowTextInput, TestSize.Level0)
{
    auto watcher = std::make_shared<Watcher>(InputMethodSysEvent::GetInstance().GetOperateInfo(
        static_cast<int32_t>(OperateIMEInfoCode::IME_SHOW_ENEDITABLE)));
    auto showTextInput = []() { inputMethodController_->ShowTextInput(); };
    EXPECT_TRUE(InputMethodDfxTest::WriteAndWatch(watcher, showTextInput));
}

/**
* @tc.name: InputMethodDfxTest_Hisysevent_HideCurrentInput
* @tc.desc: Hisysevent HideCurrentInput.
* @tc.type: FUNC
*/
HWTEST_F(InputMethodDfxTest, InputMethodDfxTest_Hisysevent_HideCurrentInput, TestSize.Level0)
{
    auto watcher = std::make_shared<Watcher>(
        InputMethodSysEvent::GetInstance().GetOperateInfo(static_cast<int32_t>(OperateIMEInfoCode::IME_HIDE_NORMAL)));
    auto hideCurrentInput = []() { inputMethodController_->HideCurrentInput(); };
    EXPECT_TRUE(InputMethodDfxTest::WriteAndWatch(watcher, hideCurrentInput));
}

/**
* @tc.name: InputMethodDfxTest_Hisysevent_ShowCurrentInput
* @tc.desc: Hisysevent ShowCurrentInput.
* @tc.type: FUNC
*/
HWTEST_F(InputMethodDfxTest, InputMethodDfxTest_Hisysevent_ShowCurrentInput, TestSize.Level0)
{
    auto watcher = std::make_shared<Watcher>(
        InputMethodSysEvent::GetInstance().GetOperateInfo(static_cast<int32_t>(OperateIMEInfoCode::IME_SHOW_NORMAL)));
    auto showCurrentInput = []() { inputMethodController_->ShowCurrentInput(); };
    EXPECT_TRUE(InputMethodDfxTest::WriteAndWatch(watcher, showCurrentInput));
}

/**
* @tc.name: InputMethodDfxTest_Hisysevent_HideSoftKeyboard
* @tc.desc: Hisysevent HideSoftKeyboard.
* @tc.type: FUNC
*/
HWTEST_F(InputMethodDfxTest, InputMethodDfxTest_Hisysevent_HideSoftKeyboard, TestSize.Level0)
{
    auto watcher = std::make_shared<Watcher>(
        InputMethodSysEvent::GetInstance().GetOperateInfo(static_cast<int32_t>(OperateIMEInfoCode::IME_HIDE_NORMAL)));
    auto hideSoftKeyboard = []() { inputMethodController_->HideSoftKeyboard(); };
    EXPECT_TRUE(InputMethodDfxTest::WriteAndWatch(watcher, hideSoftKeyboard));
}

/**
* @tc.name: InputMethodDfxTest_Hisysevent_ShowSoftKeyboard
* @tc.desc: Hisysevent ShowSoftKeyboard.
* @tc.type: FUNC
*/
HWTEST_F(InputMethodDfxTest, InputMethodDfxTest_Hisysevent_ShowSoftKeyboard, TestSize.Level0)
{
    auto watcher = std::make_shared<Watcher>(
        InputMethodSysEvent::GetInstance().GetOperateInfo(static_cast<int32_t>(OperateIMEInfoCode::IME_SHOW_NORMAL)));
    auto showSoftKeyboard = []() { inputMethodController_->ShowSoftKeyboard(); };
    EXPECT_TRUE(InputMethodDfxTest::WriteAndWatch(watcher, showSoftKeyboard));
}

/**
* @tc.name: InputMethodDfxTest_Hisysevent_HideKeyboardSelf
* @tc.desc: Hisysevent HideKeyboardSelf.
* @tc.type: FUNC
*/
HWTEST_F(InputMethodDfxTest, InputMethodDfxTest_Hisysevent_HideKeyboardSelf, TestSize.Level0)
{
    auto watcher = std::make_shared<Watcher>(
        InputMethodSysEvent::GetInstance().GetOperateInfo(static_cast<int32_t>(OperateIMEInfoCode::IME_HIDE_SELF)));
    auto hideKeyboardSelf = []() { inputMethodAbility_->HideKeyboardSelf(); };
    EXPECT_TRUE(InputMethodDfxTest::WriteAndWatch(watcher, hideKeyboardSelf));
}

/**
* @tc.name: InputMethodDfxTest_Hisysevent_Close
* @tc.desc: Hisysevent Close.
* @tc.type: FUNC
*/
HWTEST_F(InputMethodDfxTest, InputMethodDfxTest_Hisysevent_Close, TestSize.Level0)
{
    auto watcher = std::make_shared<Watcher>(
        InputMethodSysEvent::GetInstance().GetOperateInfo(static_cast<int32_t>(OperateIMEInfoCode::IME_UNBIND)));
    auto close = []() { inputMethodController_->Close(); };
    EXPECT_TRUE(InputMethodDfxTest::WriteAndWatch(watcher, close));
}
} // namespace MiscServices
} // namespace OHOS
