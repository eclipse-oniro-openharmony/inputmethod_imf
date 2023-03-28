/*
 * Copyright (C) 2021-2023 Huawei Device Co., Ltd.
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

#include "input_method_ability.h"

#include <gtest/gtest.h>
#include <string_ex.h>
#include <unistd.h>

#include <cstdint>
#include <functional>
#include <string>
#include <thread>
#include <vector>

#include "accesstoken_kit.h"
#include "global.h"
#include "i_input_data_channel.h"
#include "input_attribute.h"
#include "input_control_channel_stub.h"
#include "input_data_channel_proxy.h"
#include "input_data_channel_stub.h"
#include "input_method_agent_stub.h"
#include "input_method_controller.h"
#include "input_method_core_proxy.h"
#include "input_method_core_stub.h"
#include "input_method_property.h"
#include "message_handler.h"
#include "nativetoken_kit.h"
#include "os_account_manager.h"
#include "token_setproc.h"

using namespace testing::ext;
using namespace OHOS::Security::AccessToken;
using namespace OHOS::AccountSA;
namespace OHOS {
namespace MiscServices {
constexpr uint32_t DEALY_TIME = 1;
constexpr int32_t MAIN_USER_ID = 100;
std::u16string g_textTemp = u"我們我們ddddd";
class InputMethodAbilityTest : public testing::Test {
public:
    static std::string imeIdStopped_;
    static std::mutex imeListenerCallbackLock_;
    static std::condition_variable imeListenerCv_;
    static bool showKeyboard_;
    static std::mutex textListenerCallbackLock_;
    static std::condition_variable textListenerCv_;
    static int direction_;
    static int deleteForwardLength_;
    static int deleteBackwardLength_;
    static std::u16string insertText_;
    static int key_;
    static int keyboardStatus_;
    static bool status_;
    static sptr<InputMethodController> imc_;
    static sptr<InputMethodAbility> inputMethodAbility_;

    class InputMethodEngineListenerImpl : public InputMethodEngineListener {
    public:
        InputMethodEngineListenerImpl() = default;
        ~InputMethodEngineListenerImpl() = default;

        void OnKeyboardStatus(bool isShow)
        {
            showKeyboard_ = isShow;
            InputMethodAbilityTest::imeListenerCv_.notify_one();
            IMSA_HILOGI("InputMethodEngineListenerImpl OnKeyboardStatus");
        }

        void OnInputStart()
        {
            IMSA_HILOGI("InputMethodEngineListenerImpl OnInputStart");
        }

        void OnInputStop(const std::string &imeId)
        {
            imeIdStopped_ = imeId;
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
    class TextChangeListener : public OnTextChangedListener {
    public:
        void InsertText(const std::u16string &text) override
        {
            insertText_ = text;
            InputMethodAbilityTest::textListenerCv_.notify_one();
        }

        void DeleteForward(int32_t length) override
        {
            deleteForwardLength_ = length;
            InputMethodAbilityTest::textListenerCv_.notify_one();
            IMSA_HILOGI("TextChangeListener: DeleteForward, length is: %{public}d", length);
        }

        void DeleteBackward(int32_t length) override
        {
            deleteBackwardLength_ = length;
            InputMethodAbilityTest::textListenerCv_.notify_one();
            IMSA_HILOGI("TextChangeListener: DeleteBackward, direction is: %{public}d", length);
        }

        void SendKeyEventFromInputMethod(const KeyEvent &event) override
        {
        }

        void SendKeyboardInfo(const KeyboardInfo &info) override
        {
            FunctionKey functionKey = info.GetFunctionKey();
            KeyboardStatus keyboardStatus = info.GetKeyboardStatus();
            key_ = (int)functionKey;
            keyboardStatus_ = (int)keyboardStatus;
            InputMethodAbilityTest::textListenerCv_.notify_one();
        }

        void SetKeyboardStatus(bool status) override
        {
            status_ = status;
        }

        void MoveCursor(const Direction direction) override
        {
            direction_ = (int)direction;
            InputMethodAbilityTest::textListenerCv_.notify_one();
            IMSA_HILOGI("TextChangeListener: MoveCursor, direction is: %{public}d", direction);
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
    };
    static void InitTestConfiguration(const std::string &bundleName)
    {
        IMSA_HILOGI("bundleName: %{public}s", bundleName.c_str());
        std::vector<int32_t> userIds;
        auto ret = OsAccountManager::QueryActiveOsAccountIds(userIds);
        if (ret != ErrorCode::NO_ERROR || userIds.empty()) {
            IMSA_HILOGE("query active os account id failed");
            userIds[0] = MAIN_USER_ID;
        }
        HapInfoParams infoParams = {
            .userID = userIds[0],
            .bundleName = bundleName,
            .instIndex = 0,
            .appIDDesc = "ohos.inputmethod_test.demo"
        };
        HapPolicyParams policyParams = {
            .apl = APL_NORMAL,
            .domain = "test.domain.inputmethod",
            .permList = {},
            .permStateList = {}
        };
        AccessTokenKit::AllocHapToken(infoParams, policyParams);
        auto tokenID = AccessTokenKit::GetHapTokenID(infoParams.userID, infoParams.bundleName, infoParams.instIndex);
        int res = SetSelfTokenID(tokenID);
        if (res == ErrorCode::NO_ERROR) {
            IMSA_HILOGI("SetSelfTokenID success!");
        } else {
            IMSA_HILOGE("SetSelfTokenID fail!");
        }
    }
    static void SetUpTestCase(void)
    {
        IMSA_HILOGI("InputMethodAbilityTest::SetUpTestCase");
        std::shared_ptr<Property> property = InputMethodController::GetInstance()->GetCurrentInputMethod();
        std::string currImeBundleName = property != nullptr ? property->name : "default.inputmethod.unittest";
        InitTestConfiguration(currImeBundleName);
        inputMethodAbility_ = InputMethodAbility::GetInstance();
        inputMethodAbility_->OnImeReady();
        inputMethodAbility_->SetCoreAndAgent();
        sptr<OnTextChangedListener> textListener = new TextChangeListener();
        imc_ = InputMethodController::GetInstance();
        imc_->Attach(textListener);
    }
    static void TearDownTestCase(void)
    {
        IMSA_HILOGI("InputMethodAbilityTest::TearDownTestCase");
        imc_->Close();
    }
    void SetUp()
    {
        IMSA_HILOGI("InputMethodAbilityTest::SetUp");
    }
    void TearDown()
    {
        IMSA_HILOGI("InputMethodAbilityTest::TearDown");
    }
};

std::string InputMethodAbilityTest::imeIdStopped_;
std::mutex InputMethodAbilityTest::imeListenerCallbackLock_;
std::condition_variable InputMethodAbilityTest::imeListenerCv_;
bool InputMethodAbilityTest::showKeyboard_ = true;
std::mutex InputMethodAbilityTest::textListenerCallbackLock_;
std::condition_variable InputMethodAbilityTest::textListenerCv_;
int InputMethodAbilityTest::direction_;
int InputMethodAbilityTest::deleteForwardLength_ = 0;
int InputMethodAbilityTest::deleteBackwardLength_ = 0;
std::u16string InputMethodAbilityTest::insertText_;
int InputMethodAbilityTest::key_ = 0;
int InputMethodAbilityTest::keyboardStatus_;
bool InputMethodAbilityTest::status_;
sptr<InputMethodController> InputMethodAbilityTest::imc_;
sptr<InputMethodAbility> InputMethodAbilityTest::inputMethodAbility_;

/**
* @tc.name: testSerializedInputAttribute
* @tc.desc: Checkout the serialization of InputAttribute.
* @tc.type: FUNC
*/
HWTEST_F(InputMethodAbilityTest, testSerializedInputAttribute, TestSize.Level0)
{
    InputAttribute inAttribute;
    inAttribute.inputPattern = InputAttribute::PATTERN_PASSWORD;
    MessageParcel data;
    EXPECT_TRUE(InputAttribute::Marshalling(inAttribute, data));
    InputAttribute outAttribute;
    EXPECT_TRUE(InputAttribute::Unmarshalling(outAttribute, data));
    EXPECT_TRUE(outAttribute.GetSecurityFlag());
}

/**
* @tc.name: testShowKeyboardInputMethodCoreProxy
* @tc.desc: Test InputMethodCoreProxy ShowKeyboard
* @tc.type: FUNC
* @tc.require: issueI5NXHK
*/
HWTEST_F(InputMethodAbilityTest, testShowKeyboardInputMethodCoreProxy, TestSize.Level0)
{
    sptr<InputMethodCoreStub> coreStub = new InputMethodCoreStub(0);
    sptr<IInputMethodCore> core = coreStub;
    auto msgHandler = new (std::nothrow) MessageHandler();
    coreStub->SetMessageHandler(msgHandler);
    sptr<InputDataChannelStub> channelStub = new InputDataChannelStub();

    MessageParcel data;
    data.WriteRemoteObject(core->AsObject());
    data.WriteRemoteObject(channelStub->AsObject());
    sptr<IRemoteObject> coreObject = data.ReadRemoteObject();
    sptr<IRemoteObject> channelObject = data.ReadRemoteObject();

    sptr<InputMethodCoreProxy> coreProxy = new InputMethodCoreProxy(coreObject);
    sptr<InputDataChannelProxy> channelProxy = new InputDataChannelProxy(channelObject);
    SubProperty subProperty;
    auto ret = coreProxy->showKeyboard(channelProxy, false, subProperty);
    std::unique_lock<std::mutex> lock(InputMethodAbilityTest::imeListenerCallbackLock_);
    auto cvStatus = InputMethodAbilityTest::imeListenerCv_.wait_for(lock, std::chrono::seconds(DEALY_TIME));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(cvStatus, std::cv_status::timeout);
    EXPECT_TRUE(InputMethodAbilityTest::showKeyboard_);
    delete msgHandler;
}

/**
* @tc.name: testHideKeyboardSelfWithoutImeListener
* @tc.desc: InputMethodAbility HideKeyboardSelf Without ImeListener
* @tc.type: FUNC
* @tc.require:
* @tc.author: Hollokin
*/
HWTEST_F(InputMethodAbilityTest, testHideKeyboardSelfWithoutImeListener, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility testHideKeyboardSelfWithoutImeListener START");
    std::unique_lock<std::mutex> lock(InputMethodAbilityTest::imeListenerCallbackLock_);
    InputMethodAbilityTest::showKeyboard_ = true;
    auto ret = inputMethodAbility_->HideKeyboardSelf();
    auto cvStatus = imeListenerCv_.wait_for(lock, std::chrono::seconds(DEALY_TIME));
    EXPECT_EQ(cvStatus, std::cv_status::timeout);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(InputMethodAbilityTest::showKeyboard_);
}

/**
* @tc.name: testHideKeyboardSelf
* @tc.desc: InputMethodAbility HideKeyboardSelf
* @tc.type: FUNC
* @tc.require:
* @tc.author: Hollokin
*/
HWTEST_F(InputMethodAbilityTest, testHideKeyboardSelf, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility testHideKeyboardSelf START");
    std::unique_lock<std::mutex> lock(InputMethodAbilityTest::imeListenerCallbackLock_);
    InputMethodAbilityTest::showKeyboard_ = true;
    inputMethodAbility_->setImeListener(std::make_shared<InputMethodEngineListenerImpl>());
    auto ret = inputMethodAbility_->HideKeyboardSelf();
    InputMethodAbilityTest::imeListenerCv_.wait_for(
        lock, std::chrono::seconds(DEALY_TIME), [] { return InputMethodAbilityTest::showKeyboard_ == false; });
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_FALSE(InputMethodAbilityTest::showKeyboard_);
}

/**
* @tc.name: testMoveCursor
* @tc.desc: InputMethodAbility MoveCursor
* @tc.type: FUNC
* @tc.require:
* @tc.author: Hollokin
*/
HWTEST_F(InputMethodAbilityTest, testMoveCursor, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility MoveCursor Test START");
    constexpr int32_t keyCode = 4;
    auto ret = inputMethodAbility_->MoveCursor(keyCode); // move cursor right
    std::unique_lock<std::mutex> lock(InputMethodAbilityTest::textListenerCallbackLock_);
    InputMethodAbilityTest::textListenerCv_.wait_for(
        lock, std::chrono::seconds(DEALY_TIME), [] { return InputMethodAbilityTest::direction_ == keyCode; });
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(InputMethodAbilityTest::direction_, keyCode);
}

/**
* @tc.name: testInsertText
* @tc.desc: InputMethodAbility InsertText
* @tc.type: FUNC
* @tc.require:
* @tc.author: Hollokin
*/
HWTEST_F(InputMethodAbilityTest, testInsertText, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility InsertText Test START");
    std::string text = "text";
    std::u16string u16Text = Str8ToStr16(text);
    auto ret = inputMethodAbility_->InsertText(text);
    std::unique_lock<std::mutex> lock(InputMethodAbilityTest::textListenerCallbackLock_);
    InputMethodAbilityTest::textListenerCv_.wait_for(
        lock, std::chrono::seconds(DEALY_TIME), [u16Text] { return InputMethodAbilityTest::insertText_ == u16Text; });
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(InputMethodAbilityTest::insertText_, u16Text);
}

/**
* @tc.name: testSendFunctionKey
* @tc.desc: InputMethodAbility SendFunctionKey
* @tc.type: FUNC
* @tc.require:
* @tc.author: Hollokin
*/
HWTEST_F(InputMethodAbilityTest, testSendFunctionKey, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility SendFunctionKey Test START");
    constexpr int32_t funcKey = 1;
    auto ret = inputMethodAbility_->SendFunctionKey(funcKey);
    std::unique_lock<std::mutex> lock(InputMethodAbilityTest::textListenerCallbackLock_);
    InputMethodAbilityTest::textListenerCv_.wait_for(
        lock, std::chrono::seconds(DEALY_TIME), [] { return InputMethodAbilityTest::key_ == funcKey; });
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(InputMethodAbilityTest::key_, funcKey);
}

/**
* @tc.name: testDeleteText
* @tc.desc: InputMethodAbility DeleteForward & DeleteBackward
* @tc.type: FUNC
* @tc.require:
* @tc.author: Hollokin
*/
HWTEST_F(InputMethodAbilityTest, testDeleteText, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility testDelete Test START");
    int32_t deleteForwardLenth = 1;
    auto ret = inputMethodAbility_->DeleteForward(deleteForwardLenth);
    std::unique_lock<std::mutex> lock(InputMethodAbilityTest::textListenerCallbackLock_);
    InputMethodAbilityTest::textListenerCv_.wait_for(lock, std::chrono::seconds(DEALY_TIME),
        [deleteForwardLenth] { return InputMethodAbilityTest::deleteForwardLength_ == deleteForwardLenth; });
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(InputMethodAbilityTest::deleteForwardLength_, deleteForwardLenth);

    int32_t deleteBackwardLenth = 2;
    ret = inputMethodAbility_->DeleteBackward(deleteBackwardLenth);
    InputMethodAbilityTest::textListenerCv_.wait_for(lock, std::chrono::seconds(DEALY_TIME),
        [deleteBackwardLenth] { return InputMethodAbilityTest::deleteBackwardLength_ == deleteBackwardLenth; });
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(InputMethodAbilityTest::deleteBackwardLength_, deleteBackwardLenth);
}

/**
* @tc.name: testGetEnterKeyType
* @tc.desc: InputMethodAbility GetEnterKeyType & GetInputPattern
* @tc.type: FUNC
* @tc.require:
* @tc.author: Hollokin
*/
HWTEST_F(InputMethodAbilityTest, testGetEnterKeyType, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility testGetEnterKeyType START");
    Configuration config;
    EnterKeyType keyType = EnterKeyType::NEXT;
    config.SetEnterKeyType(keyType);
    TextInputType textInputType = TextInputType::DATETIME;
    config.SetTextInputType(textInputType);
    imc_->OnConfigurationChange(config);
    int32_t keyType2;
    auto ret = inputMethodAbility_->GetEnterKeyType(keyType2);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(keyType2, (int)keyType);
    int32_t inputPattern;
    ret = inputMethodAbility_->GetInputPattern(inputPattern);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(inputPattern, (int)textInputType);
}

/**
* @tc.name: testGetTextParamExceptionValidation_001
* @tc.desc: mSelectNewBegin = mSelectNewEnd> size()
* @tc.type: FUNC
* @tc.require: issuesI6PPJM
*/
HWTEST_F(InputMethodAbilityTest, testGetTextParamExceptionValidation_001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility testGetTextParamExceptionValidation_001 START");
    int32_t start = 10;
    int32_t end = 10;
    imc_->OnSelectionChange(g_textTemp, start, end);
    int32_t number = 3;
    std::u16string text;
    auto ret = inputMethodAbility_->GetTextAfterCursor(number, text);
    EXPECT_EQ(ret, ErrorCode::ERROR_CONTROLLER_INVOKING_FAILED);
    EXPECT_EQ(text, u"");
    ret = inputMethodAbility_->GetTextBeforeCursor(number, text);
    EXPECT_EQ(ret, ErrorCode::ERROR_CONTROLLER_INVOKING_FAILED);
    EXPECT_EQ(text, u"");
}

/**
* @tc.name: testGetTextParamExceptionValidation_002
* @tc.desc: mSelectNewBegin > mSelectNewEnd, mSelectNewBegin > size()
* @tc.type: FUNC
* @tc.require: issuesI6PPJM
*/
HWTEST_F(InputMethodAbilityTest, testGetTextParamExceptionValidation_002, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility testGetTextParamExceptionValidation_002 START");
    int32_t start = 11;
    int32_t end = 9;
    imc_->OnSelectionChange(g_textTemp, start, end);
    int32_t number = 3;
    std::u16string text;
    auto ret = inputMethodAbility_->GetTextAfterCursor(number, text);
    EXPECT_EQ(ret, ErrorCode::ERROR_CONTROLLER_INVOKING_FAILED);
    EXPECT_EQ(text, u"");
    ret = inputMethodAbility_->GetTextBeforeCursor(number, text);
    EXPECT_EQ(ret, ErrorCode::ERROR_CONTROLLER_INVOKING_FAILED);
    EXPECT_EQ(text, u"");
}

/**
* @tc.name: testGetTextParamExceptionValidation_003
* @tc.desc: mSelectNewBegin < mSelectNewEnd, mSelectNewEnd > size()
* @tc.type: FUNC
* @tc.require: issuesI6PPJM
*/
HWTEST_F(InputMethodAbilityTest, testGetTextParamExceptionValidation_003, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility testGetTextParamExceptionValidation_003 START");
    int32_t start = 9;
    int32_t end = 11;
    imc_->OnSelectionChange(g_textTemp, start, end);
    int32_t number = 3;
    std::u16string text;
    auto ret = inputMethodAbility_->GetTextAfterCursor(number, text);
    EXPECT_EQ(ret, ErrorCode::ERROR_CONTROLLER_INVOKING_FAILED);
    EXPECT_EQ(text, u"");
    ret = inputMethodAbility_->GetTextBeforeCursor(number, text);
    EXPECT_EQ(ret, ErrorCode::ERROR_CONTROLLER_INVOKING_FAILED);
    EXPECT_EQ(text, u"");
}

/**
* @tc.name: testGetTextAfterCursor_001
* @tc.desc: click textfield, mSelectNewEnd + number > size()
* @tc.type: FUNC
* @tc.require: issuesI6PPJM
*/
HWTEST_F(InputMethodAbilityTest, testGetTextAfterCursor_001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility testGetTextAfterCursor_001 START");
    int32_t start = 8;
    int32_t end = 8;
    imc_->OnSelectionChange(g_textTemp, start, end);
    int32_t number = 3;
    std::u16string text;
    auto ret = inputMethodAbility_->GetTextAfterCursor(number, text);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(text, u"d");
}

/**
* @tc.name: testGetTextAfterCursor_002
* @tc.desc: Select forward, mSelectNewBegin/mSelectNewEnd + number == size()
* @tc.type: FUNC
* @tc.require: issuesI6PPJM
*/
HWTEST_F(InputMethodAbilityTest, testGetTextAfterCursor_002, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility testGetTextAfterCursor_002 START");
    // implemented correctly: mSelectNewBegin > mSelectNewEnd
    int32_t start = 7;
    int32_t end = 4;
    imc_->OnSelectionChange(g_textTemp, start, end);
    int32_t number = 2;
    std::u16string text;
    auto ret = inputMethodAbility_->GetTextAfterCursor(number, text);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(text, u"dd");

    // implemented Currently: mSelectNewBegin < mSelectNewEnd
    start = 4;
    end = 7;
    imc_->OnSelectionChange(g_textTemp, start, end);
    number = 2;
    ret = inputMethodAbility_->GetTextAfterCursor(number, text);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(text, u"dd");
}

/**
* @tc.name: testGetTextAfterCursor_003
* @tc.desc: Select backward, mSelectNewEnd + number < size()
* @tc.type: FUNC
* @tc.require: issuesI6PPJM
*/
HWTEST_F(InputMethodAbilityTest, testGetTextAfterCursor_003, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility testGetTextAfterCursor_003 START");
    int32_t start = 4;
    int32_t end = 5;
    imc_->OnSelectionChange(g_textTemp, start, end);
    int32_t number = 3;
    std::u16string text;
    auto ret = inputMethodAbility_->GetTextAfterCursor(number, text);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(text, u"ddd");
}

/**
* @tc.name: testGetTextBeforeCursor_001
* @tc.desc: click textfield, mSelectNewBegin > number
* @tc.type: FUNC
* @tc.require: issuesI6PPJM
*/
HWTEST_F(InputMethodAbilityTest, testGetTextBeforeCursor_001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility testGetTextBeforeCursor_001 START");
    int32_t start = 6;
    int32_t end = 6;
    imc_->OnSelectionChange(g_textTemp, start, end);
    int32_t number = 3;
    std::u16string text;
    auto ret = inputMethodAbility_->GetTextBeforeCursor(number, text);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(text, u"們dd");
}

/**
* @tc.name: testGetTextBeforeCursor_002
* @tc.desc: Select forward, mSelectNewBegin/mSelectNewEnd == number
* @tc.type: FUNC
* @tc.require: issuesI6PPJM
*/
HWTEST_F(InputMethodAbilityTest, testGetTextBeforeCursor_002, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility testGetTextBeforeCursor_002 START");
    // implemented correctly: mSelectNewBegin > mSelectNewEnd
    int32_t start = 7;
    int32_t end = 4;
    imc_->OnSelectionChange(g_textTemp, start, end);
    int32_t number = 4;
    std::u16string text;
    auto ret = inputMethodAbility_->GetTextBeforeCursor(number, text);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(text, u"我們我們");

    // implemented Currently: mSelectNewBegin < mSelectNewEnd
    start = 4;
    end = 7;
    imc_->OnSelectionChange(g_textTemp, start, end);
    number = 4;
    ret = inputMethodAbility_->GetTextBeforeCursor(number, text);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(text, u"我們我們");
}

/**
* @tc.name: testGetTextBeforeCursor_003
* @tc.desc: Select backward, mSelectNewBegin < number
* @tc.type: FUNC
* @tc.require: issuesI6PPJM
*/
HWTEST_F(InputMethodAbilityTest, testGetTextBeforeCursor_003, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility testGetTextBeforeCursor_003 START");
    int32_t start = 4;
    int32_t end = 5;
    imc_->OnSelectionChange(g_textTemp, start, end);
    int32_t number = 5;
    std::u16string text;
    auto ret = inputMethodAbility_->GetTextBeforeCursor(number, text);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(text, u"我們我們");
}
} // namespace MiscServices
} // namespace OHOS
