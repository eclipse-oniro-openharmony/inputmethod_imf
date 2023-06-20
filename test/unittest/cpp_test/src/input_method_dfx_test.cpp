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
#include "input_method_ability.h"
#include "input_method_controller.h"
#include "tdd_util.h"

using namespace testing::ext;
namespace OHOS {
namespace MiscServices {
constexpr const int32_t RELOAD_HIVIEW_TIME = 300;
constexpr const char *CMD1 = "hidumper -s 3703 -a -a";
constexpr const char *CMD2 = "hidumper -s 3703 -a -h";
constexpr const char *CMD3 = "hidumper -s 3703 -a -test";
constexpr const char *CMD4 = "hisysevent -l -n INPUTMETHOD -n OPERATE_SOFTKEYBOARD";
constexpr const char *CLEAR_CMD = "rm -rf //data/log/hiview/sys_event_db/INPUTMETHOD";
constexpr const char *CLEAR_CMD1 = "service_control stop hiview";
constexpr const char *CLEAR_CMD2 = "service_control start hiview";

class InputMethodEngineListenerImpl : public InputMethodEngineListener {
public:
    InputMethodEngineListenerImpl() = default;
    ~InputMethodEngineListenerImpl() = default;
    void OnKeyboardStatus(bool isShow)
    {
        IMSA_HILOGI("InputMethodEngineListenerImpl OnKeyboardStatus");
    }
    void OnInputStart()
    {
        IMSA_HILOGI("InputMethodEngineListenerImpl OnInputStart");
    }
    void OnInputStop(const std::string &imeId)
    {
        IMSA_HILOGI("InputMethodEngineListenerImpl OnInputStop");
    }
    void OnSetCallingWindow(uint32_t windowId)
    {
        IMSA_HILOGI("InputMethodEngineListenerImpl OnSetCallingWindow");
    }
    void OnSetSubtype(const SubProperty &property)
    {
        IMSA_HILOGI("InputMethodEngineListenerImpl OnSetSubtype");
    }
};

class TextListener : public OnTextChangedListener {
public:
    TextListener()
    {
        std::shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create("TextListenerNotifier");
        serviceHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    }
    ~TextListener()
    {
    }
    static KeyboardStatus keyboardStatus_;
    static std::mutex cvMutex_;
    static std::condition_variable cv_;
    std::shared_ptr<AppExecFwk::EventHandler> serviceHandler_;
    static bool WaitIMACallback()
    {
        std::unique_lock<std::mutex> lock(TextListener::cvMutex_);
        return TextListener::cv_.wait_for(lock, std::chrono::seconds(1)) != std::cv_status::timeout;
    }
    void InsertText(const std::u16string &text)
    {
        IMSA_HILOGI("IMC TEST TextListener InsertText");
    }
    void DeleteBackward(int32_t length)
    {
        IMSA_HILOGI("IMC TEST TextListener DeleteBackward length: %{public}d", length);
    }
    void SetKeyboardStatus(bool status)
    {
        IMSA_HILOGI("IMC TEST TextListener SetKeyboardStatus %{public}d", status);
    }
    void DeleteForward(int32_t length)
    {
        IMSA_HILOGI("IMC TEST TextListener DeleteForward length: %{public}d", length);
    }
    void SendKeyEventFromInputMethod(const KeyEvent &event)
    {
        IMSA_HILOGI("IMC TEST TextListener sendKeyEventFromInputMethod");
    }
    void SendKeyboardStatus(const KeyboardStatus &keyboardStatus)
    {
        IMSA_HILOGD("TextListener::SendKeyboardStatus %{public}d", static_cast<int>(keyboardStatus));
        constexpr int32_t interval = 20;
        {
            std::unique_lock<std::mutex> lock(cvMutex_);
            IMSA_HILOGD("TextListener::SendKeyboardStatus lock");
            keyboardStatus_ = keyboardStatus;
        }
        serviceHandler_->PostTask([this]() { cv_.notify_all(); }, interval);
        IMSA_HILOGD("TextListener::SendKeyboardStatus notify_all");
    }
    void SendFunctionKey(const FunctionKey &functionKey)
    {
        IMSA_HILOGI("IMC TEST TextListener SendFunctionKey");
    }
    void MoveCursor(const Direction direction)
    {
        IMSA_HILOGI("IMC TEST TextListener MoveCursor");
    }
    void HandleSetSelection(int32_t start, int32_t end)
    {
    }
    void HandleExtendAction(int32_t action)
    {
    }
    void HandleSelect(int32_t keyCode, int32_t cursorMoveSkip)
    {
    }
};
KeyboardStatus TextListener::keyboardStatus_;
std::mutex TextListener::cvMutex_;
std::condition_variable TextListener::cv_;

class InputMethodDfxTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static void ClearHisyseventCache();
    static sptr<InputMethodController> inputMethodController_;
    static sptr<OnTextChangedListener> textListener_;
    static sptr<InputMethodAbility> inputMethodAbility_;
    static std::shared_ptr<InputMethodEngineListenerImpl> imeListener_;
};
sptr<InputMethodController> InputMethodDfxTest::inputMethodController_;
sptr<OnTextChangedListener> InputMethodDfxTest::textListener_;
sptr<InputMethodAbility> InputMethodDfxTest::inputMethodAbility_;
std::shared_ptr<InputMethodEngineListenerImpl> InputMethodDfxTest::imeListener_;

void InputMethodDfxTest::SetUpTestCase(void)
{
    IMSA_HILOGI("InputMethodDfxTest::SetUpTestCase");
    ClearHisyseventCache();
    TddUtil::StorageSelfTokenID();
    std::shared_ptr<Property> property = InputMethodController::GetInstance()->GetCurrentInputMethod();
    std::string bundleName = property != nullptr ? property->name : "default.inputmethod.unittest";
    TddUtil::AllocTestTokenID(bundleName);
    TddUtil::SetTestTokenID();
    inputMethodAbility_ = InputMethodAbility::GetInstance();
    imeListener_ = std::make_shared<InputMethodEngineListenerImpl>();
    inputMethodAbility_->OnImeReady();
    inputMethodAbility_->SetCoreAndAgent();
    inputMethodAbility_->SetImeListener(imeListener_);

    inputMethodController_ = InputMethodController::GetInstance();
    textListener_ = new TextListener();
    TddUtil::StorageSelfUid();
}

void InputMethodDfxTest::TearDownTestCase(void)
{
    IMSA_HILOGI("InputMethodDfxTest::TearDownTestCase");
    TddUtil::RestoreSelfTokenID();
    TddUtil::DeleteTestTokenID();
    ClearHisyseventCache();
}

void InputMethodDfxTest::SetUp(void)
{
    IMSA_HILOGI("InputMethodDfxTest::SetUp");
}

void InputMethodDfxTest::TearDown(void)
{
    IMSA_HILOGI("InputMethodDfxTest::TearDown");
}

void InputMethodDfxTest::ClearHisyseventCache()
{
    FILE *ptr = popen(CLEAR_CMD, "r");
    pclose(ptr);
    ptr = popen(CLEAR_CMD1, "r");
    pclose(ptr);
    ptr = popen(CLEAR_CMD2, "r");
    pclose(ptr);
    ptr = nullptr;
    usleep(RELOAD_HIVIEW_TIME);
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
    TddUtil::SetTestUid();
    std::string result;
    inputMethodController_->Attach(textListener_, true);
    TddUtil::RestoreSelfUid();
    EXPECT_TRUE(TextListener::WaitIMACallback());
    auto ret = TddUtil::ExecuteCmd(std::string(CMD4) + " | grep Attach", result);
    EXPECT_TRUE(ret);
    IMSA_HILOGD("Attach result = %{public}s", result.c_str());
    EXPECT_NE(result.find(InputMethodSysEvent::GetOperateInfo(IME_SHOW_ATTACH)), std::string::npos);
}

/**
* @tc.name: InputMethodDfxTest_Hisysevent_HideTextInput
* @tc.desc: Hisysevent HideTextInput.
* @tc.type: FUNC
*/
HWTEST_F(InputMethodDfxTest, InputMethodDfxTest_Hisysevent_HideTextInput, TestSize.Level0)
{
    std::string result;
    inputMethodController_->HideTextInput();
    auto ret = TddUtil::ExecuteCmd(std::string(CMD4) + " | grep HideTextInput", result);
    EXPECT_TRUE(ret);
    IMSA_HILOGD("HideTextInput result = %{public}s", result.c_str());
    EXPECT_NE(result.find(InputMethodSysEvent::GetOperateInfo(IME_HIDE_UNEDITABLE)), std::string::npos);
}

/**
* @tc.name: InputMethodDfxTest_Hisysevent_ShowTextInput
* @tc.desc: Hisysevent ShowTextInput.
* @tc.type: FUNC
*/
HWTEST_F(InputMethodDfxTest, InputMethodDfxTest_Hisysevent_ShowTextInput, TestSize.Level0)
{
    TddUtil::SetTestUid();
    std::string result;
    inputMethodController_->ShowTextInput();
    TddUtil::RestoreSelfUid();
    auto ret = TddUtil::ExecuteCmd(std::string(CMD4) + " | grep ShowTextInput", result);
    EXPECT_TRUE(ret);
    IMSA_HILOGD("ShowTextInput result = %{public}s", result.c_str());
    EXPECT_NE(result.find(InputMethodSysEvent::GetOperateInfo(IME_SHOW_ENEDITABLE)), std::string::npos);
}

/**
* @tc.name: InputMethodDfxTest_Hisysevent_HideCurrentInput
* @tc.desc: Hisysevent HideCurrentInput.
* @tc.type: FUNC
*/
HWTEST_F(InputMethodDfxTest, InputMethodDfxTest_Hisysevent_HideCurrentInput, TestSize.Level0)
{
    std::string result;
    inputMethodController_->HideCurrentInput();
    auto ret = TddUtil::ExecuteCmd(std::string(CMD4) + " | grep HideSoftKeyboard", result);
    EXPECT_TRUE(ret);
    IMSA_HILOGD("HideCurrentInput result = %{public}s", result.c_str());
    EXPECT_NE(result.find(InputMethodSysEvent::GetOperateInfo(IME_HIDE_NORMAL)), std::string::npos);
}

/**
* @tc.name: InputMethodDfxTest_Hisysevent_ShowCurrentInput
* @tc.desc: Hisysevent ShowCurrentInput.
* @tc.type: FUNC
*/
HWTEST_F(InputMethodDfxTest, InputMethodDfxTest_Hisysevent_ShowCurrentInput, TestSize.Level0)
{
    std::string result;
    inputMethodController_->ShowCurrentInput();
    auto ret = TddUtil::ExecuteCmd(std::string(CMD4) + " | grep ShowSoftKeyboard", result);
    EXPECT_TRUE(ret);
    IMSA_HILOGD("ShowCurrentInput result = %{public}s", result.c_str());
    EXPECT_NE(result.find(InputMethodSysEvent::GetOperateInfo(IME_SHOW_NORMAL)), std::string::npos);
}

/**
* @tc.name: InputMethodDfxTest_Hisysevent_HideSoftKeyboard
* @tc.desc: Hisysevent HideSoftKeyboard.
* @tc.type: FUNC
*/
HWTEST_F(InputMethodDfxTest, InputMethodDfxTest_Hisysevent_HideSoftKeyboard, TestSize.Level0)
{
    std::string result;
    inputMethodController_->HideSoftKeyboard();
    auto ret = TddUtil::ExecuteCmd(std::string(CMD4) + " | grep HideSoftKeyboard", result);
    EXPECT_TRUE(ret);
    IMSA_HILOGD("HideSoftKeyboard result = %{public}s", result.c_str());
    EXPECT_NE(result.find(InputMethodSysEvent::GetOperateInfo(IME_HIDE_NORMAL)), std::string::npos);
}

/**
* @tc.name: InputMethodDfxTest_Hisysevent_ShowSoftKeyboard
* @tc.desc: Hisysevent ShowSoftKeyboard.
* @tc.type: FUNC
*/
HWTEST_F(InputMethodDfxTest, InputMethodDfxTest_Hisysevent_ShowSoftKeyboard, TestSize.Level0)
{
    std::string result;
    inputMethodController_->ShowSoftKeyboard();
    auto ret = TddUtil::ExecuteCmd(std::string(CMD4) + " | grep ShowSoftKeyboard", result);
    EXPECT_TRUE(ret);
    IMSA_HILOGD("ShowSoftKeyboard result = %{public}s", result.c_str());
    EXPECT_NE(result.find(InputMethodSysEvent::GetOperateInfo(IME_SHOW_NORMAL)), std::string::npos);
}

/**
* @tc.name: InputMethodDfxTest_Hisysevent_HideKeyboardSelf
* @tc.desc: Hisysevent HideKeyboardSelf.
* @tc.type: FUNC
*/
HWTEST_F(InputMethodDfxTest, InputMethodDfxTest_Hisysevent_HideKeyboardSelf, TestSize.Level0)
{
    std::string result;
    inputMethodAbility_->HideKeyboardSelf();
    auto ret = TddUtil::ExecuteCmd(std::string(CMD4) + " | grep HideKeyboardSelf", result);
    EXPECT_TRUE(ret);
    IMSA_HILOGD("HideKeyboardSelf result = %{public}s", result.c_str());
    EXPECT_NE(result.find(InputMethodSysEvent::GetOperateInfo(IME_HIDE_SELF)), std::string::npos);
}

/**
* @tc.name: InputMethodDfxTest_Hisysevent_Close
* @tc.desc: Hisysevent Close.
* @tc.type: FUNC
*/
HWTEST_F(InputMethodDfxTest, InputMethodDfxTest_Hisysevent_Close, TestSize.Level0)
{
    std::string result;
    inputMethodController_->Close();
    auto ret = TddUtil::ExecuteCmd(std::string(CMD4) + " | grep Close", result);
    EXPECT_TRUE(ret);
    IMSA_HILOGD("ShowSoftKeyboard result = %{public}s", result.c_str());
    EXPECT_NE(result.find(InputMethodSysEvent::GetOperateInfo(IME_UNBIND)), std::string::npos);
}
} // namespace MiscServices
} // namespace OHOS
