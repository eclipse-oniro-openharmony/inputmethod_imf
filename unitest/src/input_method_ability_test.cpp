/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include <cstdint>
#include <functional>
#include <gtest/gtest.h>
#include <string>
#include <thread>
#include <unistd.h>
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
#include "message_handler.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"

using namespace testing::ext;
using namespace OHOS::Security::AccessToken;
namespace OHOS {
namespace MiscServices {
class InputMethodAbilityTest : public testing::Test {
public:
    static std::string imeIdStopped_;
    static bool showKeyboard_;
    static int direction_;
    static int deleteForwardLength_;
    static int deleteBackwardLength_;
    static std::u16string insertText_;
    static int key_;
    static int keyboardStatus_;
    static bool status_;
    static sptr<InputMethodController> imc_;
    static sptr<InputMethodAbility> inputMethodAbility_;

    class KeyboardListenerTestImpl : public KeyboardListener {
        bool OnKeyEvent(int32_t keyCode, int32_t keyStatus)
        {
            return true;
        }
        void OnCursorUpdate(int32_t positionX, int32_t positionY, int32_t height)
        {
        }
        void OnSelectionChange(int32_t oldBegin, int32_t oldEnd, int32_t newBegin, int32_t newEnd)
        {
        }
        void OnTextChange(const std::string &text)
        {
        }
    };
    class InputMethodEngineListenerImpl : public InputMethodEngineListener {
    public:
        InputMethodEngineListenerImpl() = default;
        ~InputMethodEngineListenerImpl() = default;

        void OnKeyboardStatus(bool isShow)
        {
            showKeyboard_ = isShow;
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
        }

        void DeleteForward(int32_t length) override
        {
            deleteForwardLength_ = length;
            IMSA_HILOGI("TextChangeListener: DeleteForward, length is: %{public}d", length);
        }

        void DeleteBackward(int32_t length) override
        {
            deleteBackwardLength_ = length;
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
        }

        void SetKeyboardStatus(bool status) override
        {
            status_ = status;
        }

        void MoveCursor(const Direction direction) override
        {
            direction_ = (int)direction;
            IMSA_HILOGI("TextChangeListener: MoveCursor, direction is: %{public}d", direction);
        }
    };
    void GrantPermission()
    {
        const char **perms = new const char *[1];
        perms[0] = "ohos.permission.CONNECT_IME_ABILITY";
        TokenInfoParams infoInstance = {
            .dcapsNum = 0,
            .permsNum = 1,
            .aclsNum = 0,
            .dcaps = nullptr,
            .perms = perms,
            .acls = nullptr,
            .processName = "inputmethod_imf",
            .aplStr = "system_core",
        };
        uint64_t tokenId = GetAccessTokenId(&infoInstance);
        int res = SetSelfTokenID(tokenId);
        if (res == 0) {
            IMSA_HILOGI("SetSelfTokenID success!");
        } else {
            IMSA_HILOGE("SetSelfTokenID fail!");
        }
        AccessTokenKit::ReloadNativeTokenInfo();
        delete[] perms;
    }
    static void SetUpTestCase(void)
    {
        imc_ = InputMethodController::GetInstance();
        ASSERT_TRUE(imc_ != nullptr);
    }
    static void TearDownTestCase(void)
    {
    }
    void SetUp()
    {
        inputMethodAbility_ = InputMethodAbility::GetInstance();
        GrantPermission();
        IMSA_HILOGI("InputMethodAbilityTest::SetUp");
        sptr<OnTextChangedListener> textListener = new TextChangeListener();
        imc_->Attach(textListener);
    }
    void TearDown()
    {
    }
};

std::string InputMethodAbilityTest::imeIdStopped_;
bool InputMethodAbilityTest::showKeyboard_;
int InputMethodAbilityTest::direction_;
int InputMethodAbilityTest::deleteForwardLength_;
int InputMethodAbilityTest::deleteBackwardLength_;
std::u16string InputMethodAbilityTest::insertText_;
int InputMethodAbilityTest::key_;
int InputMethodAbilityTest::keyboardStatus_;
bool InputMethodAbilityTest::status_;
sptr<InputMethodController> InputMethodAbilityTest::imc_;
sptr<InputMethodAbility> InputMethodAbilityTest::inputMethodAbility_;

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
    auto ret = coreProxy->showKeyboard(channelProxy, true, subProperty);
    delete msgHandler;
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
* @tc.name: testShowKeyboardInputMethodCoreStub
* @tc.desc: Test InputMethodCoreStub ShowKeyboard
* @tc.type: FUNC
* @tc.require: issueI5NXHK
*/
HWTEST_F(InputMethodAbilityTest, testShowKeyboardInputMethodCoreStub, TestSize.Level0)
{
    sptr<InputMethodCoreStub> coreStub = new InputMethodCoreStub(0);
    SubProperty subProperty;
    auto ret = coreStub->showKeyboard(nullptr, true, subProperty);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

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
* @tc.name: testSerializedKeyboardType
* @tc.desc: Checkout the serialization of KeyboardType.
* @tc.type: FUNC
*/
HWTEST_F(InputMethodAbilityTest, testSerializedKeyboardType, TestSize.Level0)
{
    int32_t def_value = 2021;
    sptr<KeyboardType> mKeyboardType = new KeyboardType();
    mKeyboardType->setId(def_value);
    MessageParcel data;
    auto ret = data.WriteParcelable(mKeyboardType);
    EXPECT_TRUE(ret);
    sptr<KeyboardType> deserialization = data.ReadParcelable<KeyboardType>();
    ASSERT_TRUE(deserialization != nullptr);
    EXPECT_TRUE(deserialization->getId() == def_value);
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
    auto ret = inputMethodAbility_->MoveCursor(4); // move cursor right
    usleep(500);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(direction_, 4);
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
    auto ret = inputMethodAbility_->InsertText("text");
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    usleep(500);
    EXPECT_EQ(insertText_, u"text");
}

/**
* @tc.name: testSetImeListener
* @tc.desc: InputMethodAbility SetImeListener
* @tc.type: FUNC
* @tc.require:
* @tc.author: Hollokin
*/
HWTEST_F(InputMethodAbilityTest, testSetImeListener, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility SetImeListener Test START");
    auto listener = std::make_shared<InputMethodEngineListenerImpl>();
    inputMethodAbility_->setImeListener(listener);
    auto ret = imc_->StopInputSession();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(imeIdStopped_, "");

    ret = imc_->ShowSoftKeyboard();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    usleep(500);
    EXPECT_EQ(showKeyboard_, true);

    ret = imc_->HideSoftKeyboard();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(showKeyboard_, false);
}

/**
* @tc.name: testSetKdListener
* @tc.desc: InputMethodAbility SetKdListener
* @tc.type: FUNC
* @tc.require:
* @tc.author: Hollokin
*/
HWTEST_F(InputMethodAbilityTest, testSetKdListener, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility SetKdListener Test START");
    auto keyBoardListener = std::make_shared<KeyboardListenerTestImpl>();
    inputMethodAbility_->setKdListener(keyBoardListener);
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
    auto ret = inputMethodAbility_->SendFunctionKey(0);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
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
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    usleep(500);
    EXPECT_EQ(deleteForwardLength_, deleteForwardLenth);
    int32_t deleteBackwardLenth = 2;
    ret = inputMethodAbility_->DeleteBackward(deleteBackwardLenth);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    usleep(500);
    EXPECT_EQ(deleteBackwardLength_, deleteBackwardLenth);
}

/**
* @tc.name: testGetText001
* @tc.desc: InputMethodAbility GetTextBeforeCursor & GetTextAfterCursor
* @tc.type: FUNC
* @tc.require:
* @tc.author: Hollokin
*/
HWTEST_F(InputMethodAbilityTest, testGetText001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility testGetText001 START");
    imc_->OnSelectionChange(u"onselectionchange", 0, 3);
    std::u16string text;
    auto ret = inputMethodAbility_->GetTextAfterCursor(8, text);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(text, u"electioncha");
    ret = inputMethodAbility_->GetTextBeforeCursor(3, text);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(text, u"");
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
} // namespace MiscServices
} // namespace OHOS
