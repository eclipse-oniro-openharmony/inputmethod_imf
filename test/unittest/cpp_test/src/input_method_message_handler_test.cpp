/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
#include "input_method_controller.h"

#include "input_data_channel_stub.h"
#include "input_method_ability.h"
#include "input_method_system_ability.h"
#undef private

#include <event_handler.h>
#include <gtest/gtest.h>
#include <string_ex.h>

#include <condition_variable>
#include <csignal>
#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

#include "global.h"
#include "i_input_method_agent.h"
#include "i_input_method_system_ability.h"
#include "identity_checker_mock.h"
#include "inputmethod_controller_capi.h"
#include "inputmethod_message_handler_proxy_capi.h"
#include "input_client_stub.h"
#include "input_death_recipient.h"
#include "input_method_ability.h"
#include "input_method_engine_listener_impl.h"
#include "input_method_system_ability_proxy.h"
#include "input_method_utils.h"
#include "keyboard_listener.h"
#include "message_parcel.h"
#include "string_ex.h"
#include "tdd_util.h"
#include "text_listener.h"
#include "msg_handler_callback_interface.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace MiscServices {
ArrayBuffer g_arrayBufferCapi;
std::mutex g_messageHandlerMutex;
std::condition_variable g_messageHandlerCv;
bool g_onTerminated = false;
bool g_onMessage = false;
bool g_onTerminatedNew = false;
int32_t OnMessageFunc(InputMethod_MessageHandlerProxy *proxy,
    const char16_t msgId[], size_t msgIdLength, const uint8_t *msgParam, size_t msgParamLength)
{
    std::unique_lock<std::mutex> lock(g_messageHandlerMutex);
    std::u16string msgIdStr(msgId, msgIdLength);
    g_arrayBufferCapi.msgId = Str16ToStr8(msgIdStr);
    g_arrayBufferCapi.msgParam.assign(msgParam, msgParam + msgParamLength);
    g_onMessage = true;
    g_messageHandlerCv.notify_one();
    return 0;
}

bool WaitOnMessageFunc(const ArrayBuffer &arrayBuffer)
{
    std::unique_lock<std::mutex> lock(g_messageHandlerMutex);
    g_messageHandlerCv.wait_for(lock, std::chrono::seconds(1), [&arrayBuffer]() {
        return g_arrayBufferCapi == arrayBuffer;
    });
    return g_arrayBufferCapi == arrayBuffer;
}
int32_t OnTerminatedFunc(InputMethod_MessageHandlerProxy *proxy)
{
    g_onTerminated = true;
    return 0;
}

int32_t OnTerminatedFuncNew(InputMethod_MessageHandlerProxy *proxy)
{
    g_onTerminatedNew = true;
    return 0;
}
void GetTextConfigFunc(InputMethod_TextEditorProxy *proxy, InputMethod_TextConfig *config) { }
void InsertTextFunc(InputMethod_TextEditorProxy *proxy, const char16_t *text, size_t length) { }
void DeleteForwardFunc(InputMethod_TextEditorProxy *proxy, int32_t length) { }
void DeleteBackwardFunc(InputMethod_TextEditorProxy *proxy, int32_t length) { }
void SendKeyboardStatusFunc(InputMethod_TextEditorProxy *proxy, InputMethod_KeyboardStatus status) { }
void SendEnterKeyFunc(InputMethod_TextEditorProxy *proxy, InputMethod_EnterKeyType type) { }
void MoveCursorFunc(InputMethod_TextEditorProxy *proxy, InputMethod_Direction direction) { }
void HandleSetSelectionFunc(InputMethod_TextEditorProxy *proxy, int32_t start, int32_t end) { }
void HandleExtendActionFunc(InputMethod_TextEditorProxy *proxy, InputMethod_ExtendAction action) { }
void GetleftTextOfCursorFunc(InputMethod_TextEditorProxy *proxy, int32_t number, char16_t text[], size_t *length) { }
void GetRightTextOfCursorFunc(InputMethod_TextEditorProxy *proxy, int32_t number, char16_t text[], size_t *length) { }
int32_t GetTextIndexAtCursorFunc(InputMethod_TextEditorProxy *proxy)
{
    return 0;
}
int32_t ReceivePrivateCommandFunc(
    InputMethod_TextEditorProxy *proxy, InputMethod_PrivateCommand *privateCommand[], size_t size)
{
    return 0;
}
int32_t SetPreviewTextFunc(
    InputMethod_TextEditorProxy *proxy, const char16_t *text, size_t length, int32_t start, int32_t end)
{
    return 0;
}
void FinishTextPreviewFunc(InputMethod_TextEditorProxy *proxy) { }
class MessageHandlerCallback : public MsgHandlerCallbackInterface {
public:
    using onTerminatedFunc = std::function<void()>;
    using onMessageFunc = std::function<void(const ArrayBuffer &arrayBuffer)>;
    MessageHandlerCallback() {};
    virtual ~MessageHandlerCallback() {};
    int32_t OnTerminated() override;
    int32_t OnMessage(const ArrayBuffer &arrayBuffer) override;
    void SetOnTernimateFunc(onTerminatedFunc func);
    void SetOnMessageFunc(onMessageFunc func);
    bool CheckOnTerminatedAndOnMessage(bool isOnTerminatedExcute, bool isOnMessageExcute);
    static void OnMessageCallback(const ArrayBuffer &arrayBuffer);
    static bool WaitSendMessage(const ArrayBuffer &arrayBuffer);
    static void ClearArrayBuffer();
    static ArrayBuffer GetTimingArrayBuffer();
    void ClearParam();
private:
    onTerminatedFunc onTerminatedFunc_ = nullptr;
    onMessageFunc onMessageFunc_ = nullptr;
    bool isTriggerOnTerminated_ = false;
    bool isTriggerOnMessage_ = false;
    static std::mutex messageHandlerMutex_;
    static std::condition_variable messageHandlerCv_;
    static ArrayBuffer arrayBuffer_;
    static ArrayBuffer timingArrayBuffer_;
};
std::mutex MessageHandlerCallback::messageHandlerMutex_;
std::condition_variable MessageHandlerCallback::messageHandlerCv_;
ArrayBuffer MessageHandlerCallback::arrayBuffer_;
ArrayBuffer MessageHandlerCallback::timingArrayBuffer_;

int32_t MessageHandlerCallback::OnTerminated()
{
    isTriggerOnTerminated_ = true;
    if (onTerminatedFunc_ != nullptr) {
        onTerminatedFunc_();
    }
    return ErrorCode::NO_ERROR;
}

int32_t MessageHandlerCallback::OnMessage(const ArrayBuffer &arrayBuffer)
{
    std::string msgParam(arrayBuffer_.msgParam.begin(), arrayBuffer_.msgParam.end());
    std::string msgParam1(arrayBuffer.msgParam.begin(), arrayBuffer.msgParam.end());
    IMSA_HILOGE("arrayBuffer_ msgId: %{public}s, msgParam: %{publid}s",
        arrayBuffer_.msgId.c_str(), msgParam.c_str());
    IMSA_HILOGE("arrayBuffer msgId: %{public}s, msgParam: %{publid}s",
        arrayBuffer.msgId.c_str(), msgParam1.c_str());
    isTriggerOnMessage_ = true;
    if (onMessageFunc_ != nullptr) {
        onMessageFunc_(arrayBuffer);
    }
    OnMessageCallback(arrayBuffer);
    timingArrayBuffer_.msgId += arrayBuffer.msgId;
    timingArrayBuffer_.msgParam.insert(
        timingArrayBuffer_.msgParam.end(), arrayBuffer.msgParam.begin(), arrayBuffer.msgParam.end());
    return ErrorCode::NO_ERROR;
}

void MessageHandlerCallback::SetOnTernimateFunc(onTerminatedFunc func)
{
    onTerminatedFunc_ = func;
}

void MessageHandlerCallback::SetOnMessageFunc(onMessageFunc func)
{
    onMessageFunc_ = func;
}

bool MessageHandlerCallback::CheckOnTerminatedAndOnMessage(bool isOnTerminatedExcute, bool isOnMessageExcute)
{
    return isOnTerminatedExcute == isTriggerOnTerminated_ && isOnMessageExcute == isTriggerOnMessage_;
}

void MessageHandlerCallback::OnMessageCallback(const ArrayBuffer &arrayBuffer)
{
    IMSA_HILOGI("OnMessageCallback");
    std::unique_lock<std::mutex> lock(messageHandlerMutex_);
    arrayBuffer_ = arrayBuffer;
    messageHandlerCv_.notify_one();
}

bool MessageHandlerCallback::WaitSendMessage(const ArrayBuffer &arrayBuffer)
{
    std::unique_lock<std::mutex> lock(messageHandlerMutex_);
    messageHandlerCv_.wait_for(lock, std::chrono::seconds(1), [&arrayBuffer]() {
        return arrayBuffer_ == arrayBuffer;
    });
    return arrayBuffer_ == arrayBuffer;
}

void MessageHandlerCallback::ClearArrayBuffer()
{
    arrayBuffer_.msgId.clear();
    arrayBuffer_.msgParam.clear();
    g_arrayBufferCapi.msgId.clear();
    g_arrayBufferCapi.msgParam.clear();
    timingArrayBuffer_.msgId.clear();
    timingArrayBuffer_.msgParam.clear();
}

ArrayBuffer MessageHandlerCallback::GetTimingArrayBuffer()
{
    return timingArrayBuffer_;
}

void MessageHandlerCallback::ClearParam()
{
    isTriggerOnTerminated_ = false;
    isTriggerOnMessage_ = false;
}

class InputMethodMessageHandlerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static void SetSecurityModeEnable(int32_t securityMode);
    static void ResetParam();
    static void ConstructMessageHandlerProxy(InputMethod_MessageHandlerProxy *messageHandlerProxy);
    static void TestGetMessageHandlerProxyMember(InputMethod_MessageHandlerProxy *messageHandlerProxy);
    static void ConstructTextEditorProxy(InputMethod_TextEditorProxy *textEditorProxy);
    static sptr<InputMethodController> inputMethodController_;
    static sptr<InputMethodAbility> inputMethodAbility_;
    static sptr<InputMethodSystemAbility> imsa_;
    static sptr<InputMethodSystemAbilityProxy> imsaProxy_;
    static std::shared_ptr<InputMethodEngineListenerImpl> imeListener_;
    static std::shared_ptr<AppExecFwk::EventHandler> textConfigHandler_;
    static sptr<OnTextChangedListener> textListener_;
    static InputMethod_AttachOptions *option_;
    static InputMethod_TextEditorProxy *textEditorProxy_;
    static InputMethod_MessageHandlerProxy *messageHanlderProxy_;
};
sptr<InputMethodController> InputMethodMessageHandlerTest::inputMethodController_;
sptr<InputMethodAbility> InputMethodMessageHandlerTest::inputMethodAbility_;
sptr<InputMethodSystemAbility> InputMethodMessageHandlerTest::imsa_;
sptr<InputMethodSystemAbilityProxy> InputMethodMessageHandlerTest::imsaProxy_;
std::shared_ptr<InputMethodEngineListenerImpl> InputMethodMessageHandlerTest::imeListener_;
sptr<OnTextChangedListener> InputMethodMessageHandlerTest::textListener_;
std::shared_ptr<AppExecFwk::EventHandler> InputMethodMessageHandlerTest::textConfigHandler_ { nullptr };
InputMethod_AttachOptions *InputMethodMessageHandlerTest::option_ = nullptr;
InputMethod_TextEditorProxy *InputMethodMessageHandlerTest::textEditorProxy_ = nullptr;
InputMethod_MessageHandlerProxy *InputMethodMessageHandlerTest::messageHanlderProxy_ = nullptr;

void InputMethodMessageHandlerTest::SetUpTestCase(void)
{
    IMSA_HILOGI("InputMethodMessageHandlerTest::SetUpTestCase");
    IdentityCheckerMock::ResetParam();
    imsa_ = new (std::nothrow) InputMethodSystemAbility();
    if (imsa_ == nullptr) {
        return;
    }
    imsa_->OnStart();
    imsa_->userId_ = TddUtil::GetCurrentUserId();
    imsa_->identityChecker_ = std::make_shared<IdentityCheckerMock>();
    sptr<InputMethodSystemAbilityStub> serviceStub = imsa_;
    imsaProxy_ = new (std::nothrow) InputMethodSystemAbilityProxy(serviceStub->AsObject());
    if (imsaProxy_ == nullptr) {
        return;
    }
    IdentityCheckerMock::SetFocused(true);

    inputMethodAbility_ = InputMethodAbility::GetInstance();
    inputMethodAbility_->abilityManager_ = imsaProxy_;
    textListener_ = new TextListener();
    TddUtil::InitCurrentImePermissionInfo();
    IdentityCheckerMock::SetBundleName(TddUtil::currentBundleNameMock_);
    inputMethodAbility_->SetCoreAndAgent();
    imeListener_ = std::make_shared<InputMethodEngineListenerImpl>(textConfigHandler_);
    inputMethodAbility_->SetImeListener(imeListener_);

    inputMethodController_ = InputMethodController::GetInstance();
    inputMethodController_->abilityManager_ = imsaProxy_;

    option_ = OH_AttachOptions_Create(true);
    textEditorProxy_ = OH_TextEditorProxy_Create();
    ConstructTextEditorProxy(textEditorProxy_);
    messageHanlderProxy_ = OH_MessageHandlerProxy_Create();
    ConstructMessageHandlerProxy(messageHanlderProxy_);
}

void InputMethodMessageHandlerTest::TearDownTestCase(void)
{
    IMSA_HILOGI("InputMethodMessageHandlerTest::TearDownTestCase");
    inputMethodController_->SetControllerListener(nullptr);
    IdentityCheckerMock::ResetParam();
    imsa_->OnStop();
    OH_AttachOptions_Destroy(option_);
    OH_TextEditorProxy_Destroy(textEditorProxy_);
    OH_MessageHandlerProxy_Destroy(messageHanlderProxy_);
}

void InputMethodMessageHandlerTest::SetUp(void)
{
    IMSA_HILOGI("InputMethodMessageHandlerTest::SetUp");
}

void InputMethodMessageHandlerTest::TearDown(void)
{
    IMSA_HILOGI("InputMethodMessageHandlerTest::TearDown");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    ResetParam();
}

void InputMethodMessageHandlerTest::SetSecurityModeEnable(int32_t securityMode)
{
    if (inputMethodAbility_ != nullptr) {
        inputMethodAbility_->securityMode_.store(securityMode);
    }
}

void InputMethodMessageHandlerTest::ResetParam()
{
    inputMethodController_->Close();
    if (inputMethodAbility_ != nullptr) {
        inputMethodAbility_->securityMode_.store(-1);
    }
    InputMethodEngineListenerImpl::ResetParam();
    MessageHandlerCallback::ClearArrayBuffer();
    inputMethodAbility_->RegisterMsgHandler();
    inputMethodController_->RegisterMsgHandler();
    g_onTerminated = false;
    g_onMessage = false;
    g_onTerminatedNew = false;
}

void InputMethodMessageHandlerTest::ConstructTextEditorProxy(InputMethod_TextEditorProxy *textEditorProxy)
{
    EXPECT_EQ(IME_ERR_OK, OH_TextEditorProxy_SetGetTextConfigFunc(textEditorProxy, GetTextConfigFunc));
    EXPECT_EQ(IME_ERR_OK, OH_TextEditorProxy_SetInsertTextFunc(textEditorProxy, InsertTextFunc));
    EXPECT_EQ(IME_ERR_OK, OH_TextEditorProxy_SetDeleteForwardFunc(textEditorProxy, DeleteForwardFunc));
    EXPECT_EQ(IME_ERR_OK, OH_TextEditorProxy_SetDeleteBackwardFunc(textEditorProxy, DeleteBackwardFunc));
    EXPECT_EQ(IME_ERR_OK, OH_TextEditorProxy_SetSendKeyboardStatusFunc(textEditorProxy, SendKeyboardStatusFunc));
    EXPECT_EQ(IME_ERR_OK, OH_TextEditorProxy_SetSendEnterKeyFunc(textEditorProxy, SendEnterKeyFunc));
    EXPECT_EQ(IME_ERR_OK, OH_TextEditorProxy_SetMoveCursorFunc(textEditorProxy, MoveCursorFunc));
    EXPECT_EQ(IME_ERR_OK, OH_TextEditorProxy_SetHandleSetSelectionFunc(textEditorProxy, HandleSetSelectionFunc));
    EXPECT_EQ(IME_ERR_OK, OH_TextEditorProxy_SetHandleExtendActionFunc(textEditorProxy, HandleExtendActionFunc));
    EXPECT_EQ(IME_ERR_OK, OH_TextEditorProxy_SetGetLeftTextOfCursorFunc(textEditorProxy, GetleftTextOfCursorFunc));
    EXPECT_EQ(IME_ERR_OK, OH_TextEditorProxy_SetGetRightTextOfCursorFunc(textEditorProxy, GetRightTextOfCursorFunc));
    EXPECT_EQ(IME_ERR_OK, OH_TextEditorProxy_SetGetTextIndexAtCursorFunc(textEditorProxy, GetTextIndexAtCursorFunc));
    EXPECT_EQ(IME_ERR_OK, OH_TextEditorProxy_SetReceivePrivateCommandFunc(textEditorProxy, ReceivePrivateCommandFunc));
    EXPECT_EQ(IME_ERR_OK, OH_TextEditorProxy_SetSetPreviewTextFunc(textEditorProxy, SetPreviewTextFunc));
    EXPECT_EQ(IME_ERR_OK, OH_TextEditorProxy_SetFinishTextPreviewFunc(textEditorProxy, FinishTextPreviewFunc));
}

void InputMethodMessageHandlerTest::ConstructMessageHandlerProxy(InputMethod_MessageHandlerProxy *messageHandlerProxy)
{
    EXPECT_EQ(IME_ERR_OK, OH_MessageHandlerProxy_SetOnTerminatedFunc(messageHandlerProxy, OnTerminatedFunc));
    EXPECT_EQ(IME_ERR_OK, OH_MessageHandlerProxy_SetOnMessageFunc(messageHandlerProxy, OnMessageFunc));
}

void InputMethodMessageHandlerTest::TestGetMessageHandlerProxyMember(
    InputMethod_MessageHandlerProxy *messageHandlerProxy)
{
    OH_MessageHandlerProxy_OnTerminatedFunc onTerminatedFunc = nullptr;
    EXPECT_EQ(IME_ERR_OK, OH_MessageHandlerProxy_GetOnTerminatedFunc(messageHandlerProxy, &onTerminatedFunc));
    EXPECT_EQ(OnTerminatedFunc, onTerminatedFunc);

    OH_MessageHandlerProxy_OnMessageFunc onMessageFunc = nullptr;
    EXPECT_EQ(IME_ERR_OK, OH_MessageHandlerProxy_GetOnMessageFunc(messageHandlerProxy, &onMessageFunc));
    EXPECT_EQ(OnMessageFunc, onMessageFunc);
}

/**
 * @tc.name: testIMCSendMessage_001
 * @tc.desc: IMC SendMessage, valid msgId and msgParam.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodMessageHandlerTest, testIMCSendMessage_001, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCSendMessage_001 Test START");
    InputMethodMessageHandlerTest::SetSecurityModeEnable(static_cast<int32_t>(SecurityMode::FULL));
    auto ret = inputMethodController_->Attach(textListener_);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    auto messageHandler = std::make_shared<MessageHandlerCallback>();
    ret = inputMethodAbility_->RegisterMsgHandler(messageHandler);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ArrayBuffer arrayBuffer;
    arrayBuffer.msgId = "testMsgId";
    string msgParam = "testParamtestParamtestParamtestParamtestParamtestParam";
    arrayBuffer.msgParam.assign(msgParam.begin(), msgParam.end());
    ret = inputMethodController_->SendMessage(arrayBuffer);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    EXPECT_TRUE(MessageHandlerCallback::WaitSendMessage(arrayBuffer));
    InputMethodMessageHandlerTest::ResetParam();
}

/**
 * @tc.name: testIMCSendMessage_002
 * @tc.desc: IMC SendMessage, msgId and msgParam is 0.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodMessageHandlerTest, testIMCSendMessage_002, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCSendMessage_002 Test START");
    InputMethodMessageHandlerTest::SetSecurityModeEnable(static_cast<int32_t>(SecurityMode::FULL));
    auto ret = inputMethodController_->Attach(textListener_);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    auto messageHandler = std::make_shared<MessageHandlerCallback>();
    ret = inputMethodAbility_->RegisterMsgHandler(messageHandler);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ArrayBuffer arrayBuffer;
    ret = inputMethodController_->SendMessage(arrayBuffer);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(MessageHandlerCallback::WaitSendMessage(arrayBuffer));
    InputMethodMessageHandlerTest::ResetParam();
}

/**
 * @tc.name: testIMCSendMessage_003
 * @tc.desc: IMC SendMessage, msgId is valid and msgParam is 0.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodMessageHandlerTest, testIMCSendMessage_003, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCSendMessage_003 Test START");
    InputMethodMessageHandlerTest::SetSecurityModeEnable(static_cast<int32_t>(SecurityMode::FULL));
    auto ret = inputMethodController_->Attach(textListener_);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    auto messageHandler = std::make_shared<MessageHandlerCallback>();
    ret = inputMethodAbility_->RegisterMsgHandler(messageHandler);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ArrayBuffer arrayBuffer;
    arrayBuffer.msgId = "testMsgId";
    ret = inputMethodController_->SendMessage(arrayBuffer);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(MessageHandlerCallback::WaitSendMessage(arrayBuffer));
    InputMethodMessageHandlerTest::ResetParam();
}

/**
 * @tc.name: testIMCSendMessage_004
 * @tc.desc: IMC SendMessage, msgId is 0 and msgParam is valid.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodMessageHandlerTest, testIMCSendMessage_004, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCSendMessage_004 Test START");
    InputMethodMessageHandlerTest::SetSecurityModeEnable(static_cast<int32_t>(SecurityMode::FULL));
    auto ret = inputMethodController_->Attach(textListener_);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    auto messageHandler = std::make_shared<MessageHandlerCallback>();
    ret = inputMethodAbility_->RegisterMsgHandler(messageHandler);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ArrayBuffer arrayBuffer;
    string msgParam = "testParamtestParamtestParamtestParamtestParamtestParam";
    arrayBuffer.msgParam.assign(msgParam.begin(), msgParam.end());
    ret = inputMethodController_->SendMessage(arrayBuffer);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(MessageHandlerCallback::WaitSendMessage(arrayBuffer));
    InputMethodMessageHandlerTest::ResetParam();
}

/**
 * @tc.name: testIMCSendMessage_005
 * @tc.desc: IMC SendMessage, msgId is max length and msgParam is valid.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodMessageHandlerTest, testIMCSendMessage_005, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCSendMessage_005 Test START");
    InputMethodMessageHandlerTest::SetSecurityModeEnable(static_cast<int32_t>(SecurityMode::FULL));
    auto ret = inputMethodController_->Attach(textListener_);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    auto messageHandler = std::make_shared<MessageHandlerCallback>();
    ret = inputMethodAbility_->RegisterMsgHandler(messageHandler);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ArrayBuffer arrayBuffer;
    arrayBuffer.msgId = std::string(MAX_ARRAY_BUFFER_MSG_ID_SIZE, 'a');
    string msgParam = "testParamtestParamtestParamtestParamtestParamtestParam";
    arrayBuffer.msgParam.assign(msgParam.begin(), msgParam.end());
    ret = inputMethodController_->SendMessage(arrayBuffer);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(MessageHandlerCallback::WaitSendMessage(arrayBuffer));
    InputMethodMessageHandlerTest::ResetParam();
}

/**
 * @tc.name: testIMCSendMessage_006
 * @tc.desc: IMC SendMessage, msgId is valid and msgParam is max length.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodMessageHandlerTest, testIMCSendMessage_006, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCSendMessage_006 Test START");
    InputMethodMessageHandlerTest::SetSecurityModeEnable(static_cast<int32_t>(SecurityMode::FULL));
    auto ret = inputMethodController_->Attach(textListener_);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    auto messageHandler = std::make_shared<MessageHandlerCallback>();
    ret = inputMethodAbility_->RegisterMsgHandler(messageHandler);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ArrayBuffer arrayBuffer;
    arrayBuffer.msgId = "testMsgId";
    arrayBuffer.msgParam.assign(MAX_ARRAY_BUFFER_MSG_PARAM_SIZE, 'a');
    ret = inputMethodController_->SendMessage(arrayBuffer);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(MessageHandlerCallback::WaitSendMessage(arrayBuffer));
    InputMethodMessageHandlerTest::ResetParam();
}

/**
 * @tc.name: testIMCSendMessage_007
 * @tc.desc: IMC SendMessage, msgId is valid and msgParam is exceeds max length.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodMessageHandlerTest, testIMCSendMessage_007, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCSendMessage_007 Test START");
    InputMethodMessageHandlerTest::SetSecurityModeEnable(static_cast<int32_t>(SecurityMode::FULL));
    auto ret = inputMethodController_->Attach(textListener_);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    auto messageHandler = std::make_shared<MessageHandlerCallback>();
    ret = inputMethodAbility_->RegisterMsgHandler(messageHandler);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ArrayBuffer arrayBuffer;
    arrayBuffer.msgId = "testMsgId";
    arrayBuffer.msgParam.assign(MAX_ARRAY_BUFFER_MSG_PARAM_SIZE + 1, 'a');
    ret = inputMethodController_->SendMessage(arrayBuffer);
    EXPECT_EQ(ret, ErrorCode::ERROR_INVALID_ARRAY_BUFFER_SIZE);
    InputMethodMessageHandlerTest::ResetParam();
}

/**
 * @tc.name: testIMCSendMessage_008
 * @tc.desc: IMC SendMessage, msgId is exceeds max length and msgParam is valid.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodMessageHandlerTest, testIMCSendMessage_008, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCSendMessage_008 Test START");
    InputMethodMessageHandlerTest::SetSecurityModeEnable(static_cast<int32_t>(SecurityMode::FULL));
    auto ret = inputMethodController_->Attach(textListener_);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    auto messageHandler = std::make_shared<MessageHandlerCallback>();
    ret = inputMethodAbility_->RegisterMsgHandler(messageHandler);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ArrayBuffer arrayBuffer;
    arrayBuffer.msgId = std::string(MAX_ARRAY_BUFFER_MSG_ID_SIZE + 1, 'a');
    string msgParam = "testParamtestParamtestParamtestParamtestParamtestParam";
    arrayBuffer.msgParam.assign(msgParam.begin(), msgParam.end());
    ret = inputMethodController_->SendMessage(arrayBuffer);
    EXPECT_EQ(ret, ErrorCode::ERROR_INVALID_ARRAY_BUFFER_SIZE);
    InputMethodMessageHandlerTest::ResetParam();
}

/**
 * @tc.name: testIMCSendMessage_009
 * @tc.desc: IMC SendMessage, msgId and msgParam are all exceed max length.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodMessageHandlerTest, testIMCSendMessage_009, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCSendMessage_009 Test START");
    InputMethodMessageHandlerTest::SetSecurityModeEnable(static_cast<int32_t>(SecurityMode::FULL));
    auto ret = inputMethodController_->Attach(textListener_);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    auto messageHandler = std::make_shared<MessageHandlerCallback>();
    ret = inputMethodAbility_->RegisterMsgHandler(messageHandler);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ArrayBuffer arrayBuffer;
    arrayBuffer.msgId = std::string(MAX_ARRAY_BUFFER_MSG_ID_SIZE + 1, 'a');
    arrayBuffer.msgParam.assign(MAX_ARRAY_BUFFER_MSG_PARAM_SIZE + 1, 'a');
    ret = inputMethodController_->SendMessage(arrayBuffer);
    EXPECT_EQ(ret, ErrorCode::ERROR_INVALID_ARRAY_BUFFER_SIZE);
    InputMethodMessageHandlerTest::ResetParam();
}

/**
 * @tc.name: testIMCSendMessage_010
 * @tc.desc: IMC SendMessage, without Attach.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodMessageHandlerTest, testIMCSendMessage_010, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCSendMessage_010 Test START");
    InputMethodMessageHandlerTest::SetSecurityModeEnable(static_cast<int32_t>(SecurityMode::FULL));
    ArrayBuffer arrayBuffer;
    arrayBuffer.msgId = "testMsgId";
    string msgParam = "testParamtestParamtestParamtestParamtestParamtestParam";
    arrayBuffer.msgParam.assign(msgParam.begin(), msgParam.end());
    auto ret = inputMethodController_->SendMessage(arrayBuffer);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_BOUND);
    InputMethodMessageHandlerTest::ResetParam();
}

/**
 * @tc.name: testIMCSendMessage_011
 * @tc.desc: IMC SendMessage, Attach but not editable.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodMessageHandlerTest, testIMCSendMessage_011, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCSendMessage_011 Test START");
    InputMethodMessageHandlerTest::SetSecurityModeEnable(static_cast<int32_t>(SecurityMode::FULL));
    auto ret = inputMethodController_->Attach(textListener_);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    auto messageHandler = std::make_shared<MessageHandlerCallback>();
    ret = inputMethodAbility_->RegisterMsgHandler(messageHandler);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    inputMethodController_->isEditable_.store(false);

    ArrayBuffer arrayBuffer;
    arrayBuffer.msgId = "testMsgId";
    string msgParam = "testParamtestParamtestParamtestParamtestParamtestParam";
    arrayBuffer.msgParam.assign(msgParam.begin(), msgParam.end());
    ret = inputMethodController_->SendMessage(arrayBuffer);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_EDITABLE);
    InputMethodMessageHandlerTest::ResetParam();
}

/**
 * @tc.name: testIMCSendMessage_012
 * @tc.desc: IMC SendMessage, basic security mode.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodMessageHandlerTest, testIMCSendMessage_012, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCSendMessage_012 Test START");
    InputMethodMessageHandlerTest::SetSecurityModeEnable(static_cast<int32_t>(SecurityMode::BASIC));
    auto ret = inputMethodController_->Attach(textListener_);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    auto messageHandler = std::make_shared<MessageHandlerCallback>();
    ret = inputMethodAbility_->RegisterMsgHandler(messageHandler);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ArrayBuffer arrayBuffer;
    arrayBuffer.msgId = "testMsgId";
    string msgParam = "testParamtestParamtestParamtestParamtestParamtestParam";
    arrayBuffer.msgParam.assign(msgParam.begin(), msgParam.end());
    ret = inputMethodController_->SendMessage(arrayBuffer);
    EXPECT_EQ(ret, ErrorCode::ERROR_SECURITY_MODE_OFF);

    InputMethodMessageHandlerTest::ResetParam();
}


/**
 * @tc.name: testIMCSendMessage_013
 * @tc.desc: IMC SendMessage, without register message handler another side.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodMessageHandlerTest, testIMCSendMessage_013, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCSendMessage_013 Test START");
    InputMethodMessageHandlerTest::SetSecurityModeEnable(static_cast<int32_t>(SecurityMode::FULL));
    auto ret = inputMethodController_->Attach(textListener_);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ArrayBuffer arrayBuffer;
    arrayBuffer.msgId = "testMsgId";
    string msgParam = "testParamtestParamtestParamtestParamtestParamtestParam";
    arrayBuffer.msgParam.assign(msgParam.begin(), msgParam.end());
    ret = inputMethodController_->SendMessage(arrayBuffer);
    EXPECT_EQ(ret, ErrorCode::ERROR_MSG_HANDLER_NOT_REGIST);

    InputMethodMessageHandlerTest::ResetParam();
}

/**
 * @tc.name: testIMCRegisterMsgHandler_001
 * @tc.desc: IMA RegisterMsgHandler, duplicate registration will triggers ex-callback OnTerminated.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodMessageHandlerTest, testIMCRegisterMsgHandler_001, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCRegisterMsgHandler_001 Test START");
    auto exMessageHandler = std::make_shared<MessageHandlerCallback>();
    auto messageHandler = std::make_shared<MessageHandlerCallback>();
    auto ret = inputMethodAbility_->RegisterMsgHandler(exMessageHandler);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(exMessageHandler->CheckOnTerminatedAndOnMessage(false, false));

    ret = inputMethodAbility_->RegisterMsgHandler(messageHandler);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(messageHandler->CheckOnTerminatedAndOnMessage(false, false));
    EXPECT_TRUE(exMessageHandler->CheckOnTerminatedAndOnMessage(true, false));

    ret = inputMethodAbility_->RegisterMsgHandler(nullptr);
    EXPECT_TRUE(messageHandler->CheckOnTerminatedAndOnMessage(true, false));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: testIMCRegisterMsgHandler_002
 * @tc.desc: IMA RegisterMsgHandler, message handler is globally unique.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodMessageHandlerTest, testIMCRegisterMsgHandler_002, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCRegisterMsgHandler_002 Test START");
    auto exMessageHandler = std::make_shared<MessageHandlerCallback>();
    auto messageHandler = std::make_shared<MessageHandlerCallback>();
    auto ret = inputMethodAbility_->RegisterMsgHandler(exMessageHandler);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(exMessageHandler->CheckOnTerminatedAndOnMessage(false, false));
    ret = inputMethodAbility_->RegisterMsgHandler(messageHandler);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(messageHandler->CheckOnTerminatedAndOnMessage(false, false));
    EXPECT_TRUE(exMessageHandler->CheckOnTerminatedAndOnMessage(true, false));
    messageHandler->ClearParam();
    exMessageHandler->ClearParam();

    InputMethodMessageHandlerTest::SetSecurityModeEnable(static_cast<int32_t>(SecurityMode::FULL));
    ret = inputMethodController_->Attach(textListener_);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ArrayBuffer arrayBuffer;
    arrayBuffer.msgId = "testMsgId";
    string msgParam = "testParamtestParamtestParamtestParamtestParamtestParam";
    arrayBuffer.msgParam.assign(msgParam.begin(), msgParam.end());
    ret = inputMethodController_->SendMessage(arrayBuffer);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(messageHandler->CheckOnTerminatedAndOnMessage(false, true));
    EXPECT_TRUE(exMessageHandler->CheckOnTerminatedAndOnMessage(false, false));
    messageHandler->ClearParam();

    ret = inputMethodAbility_->RegisterMsgHandler();
    EXPECT_TRUE(messageHandler->CheckOnTerminatedAndOnMessage(true, false));
    InputMethodMessageHandlerTest::ResetParam();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: testIMCRegisterMsgHandler_003
 * @tc.desc: IMA RegisterMsgHandler, unregister message handle will trigger OnTerminated.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodMessageHandlerTest, testIMCRegisterMsgHandler_003, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCRegisterMsgHandler_003 Test START");
    auto messageHandler = std::make_shared<MessageHandlerCallback>();
    auto ret = inputMethodAbility_->RegisterMsgHandler(messageHandler);
    ret = inputMethodAbility_->RegisterMsgHandler(nullptr);
    EXPECT_TRUE(messageHandler->CheckOnTerminatedAndOnMessage(true, false));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: testIMCRegisterMsgHandler_004
 * @tc.desc: IMC RegisterMsgHandler, duplicate registration will triggers ex-callback OnTerminated.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodMessageHandlerTest, testIMCRegisterMsgHandler_004, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCRegisterMsgHandler_004 Test START");
    auto exMessageHandler = std::make_shared<MessageHandlerCallback>();
    auto messageHandler = std::make_shared<MessageHandlerCallback>();
    auto ret = inputMethodController_->RegisterMsgHandler(exMessageHandler);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(exMessageHandler->CheckOnTerminatedAndOnMessage(false, false));

    ret = inputMethodController_->RegisterMsgHandler(messageHandler);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(messageHandler->CheckOnTerminatedAndOnMessage(false, false));
    EXPECT_TRUE(exMessageHandler->CheckOnTerminatedAndOnMessage(true, false));

    ret = inputMethodController_->RegisterMsgHandler(nullptr);
    EXPECT_TRUE(messageHandler->CheckOnTerminatedAndOnMessage(true, false));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: testIMCRegisterMsgHandler_005
 * @tc.desc: IMC RegisterMsgHandler, message handler is globally unique.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodMessageHandlerTest, testIMCRegisterMsgHandler_005, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCRegisterMsgHandler_005 Test START");
    auto exMessageHandler = std::make_shared<MessageHandlerCallback>();
    auto messageHandler = std::make_shared<MessageHandlerCallback>();
    auto ret = inputMethodController_->RegisterMsgHandler(exMessageHandler);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(exMessageHandler->CheckOnTerminatedAndOnMessage(false, false));
    ret = inputMethodController_->RegisterMsgHandler(messageHandler);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(messageHandler->CheckOnTerminatedAndOnMessage(false, false));
    EXPECT_TRUE(exMessageHandler->CheckOnTerminatedAndOnMessage(true, false));

    InputMethodMessageHandlerTest::SetSecurityModeEnable(static_cast<int32_t>(SecurityMode::FULL));
    ret = inputMethodController_->Attach(textListener_);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ArrayBuffer arrayBuffer;
    arrayBuffer.msgId = "testMsgId";
    string msgParam = "testParamtestParamtestParamtestParamtestParamtestParam";
    arrayBuffer.msgParam.assign(msgParam.begin(), msgParam.end());
    ret = inputMethodAbility_->SendMessage(arrayBuffer);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(messageHandler->CheckOnTerminatedAndOnMessage(false, true));
    EXPECT_TRUE(exMessageHandler->CheckOnTerminatedAndOnMessage(true, false));
    InputMethodMessageHandlerTest::ResetParam();

    ret = inputMethodController_->RegisterMsgHandler(nullptr);
    EXPECT_TRUE(messageHandler->CheckOnTerminatedAndOnMessage(true, true));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: testIMCRegisterMsgHandler_006
 * @tc.desc: IMC RegisterMsgHandler, unregister message handle will trigger OnTerminated.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodMessageHandlerTest, testIMCRegisterMsgHandler_006, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCRegisterMsgHandler_006 Test START");
    auto messageHandler = std::make_shared<MessageHandlerCallback>();
    auto ret = inputMethodController_->RegisterMsgHandler(messageHandler);
    ret = inputMethodController_->RegisterMsgHandler(nullptr);
    EXPECT_TRUE(messageHandler->CheckOnTerminatedAndOnMessage(true, false));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: testIMASendMessage_001
 * @tc.desc: IMA SendMessage, valid msgId and msgParam.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodMessageHandlerTest, testIMASendMessage_001, TestSize.Level0)
{
    IMSA_HILOGI("IMA testIMASendMessage_001 Test START");
    auto messageHandler = std::make_shared<MessageHandlerCallback>();
    auto ret = inputMethodController_->RegisterMsgHandler(messageHandler);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodMessageHandlerTest::SetSecurityModeEnable(static_cast<int32_t>(SecurityMode::FULL));
    ret = inputMethodController_->Attach(textListener_);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ArrayBuffer arrayBuffer;
    arrayBuffer.msgId = "testMsgId";
    string msgParam = "testParamtestParamtestParamtestParamtestParamtestParam";
    arrayBuffer.msgParam.assign(msgParam.begin(), msgParam.end());
    ret = inputMethodAbility_->SendMessage(arrayBuffer);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(MessageHandlerCallback::WaitSendMessage(arrayBuffer));

    InputMethodMessageHandlerTest::ResetParam();
    ret = inputMethodController_->RegisterMsgHandler(nullptr);
    EXPECT_TRUE(messageHandler->CheckOnTerminatedAndOnMessage(true, true));
}

/**
 * @tc.name: testIMASendMessage_002
 * @tc.desc: IMA SendMessage, msgId and msgParam is 0.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodMessageHandlerTest, testIMASendMessage_002, TestSize.Level0)
{
    IMSA_HILOGI("IMA testIMASendMessage_002 Test START");
    auto messageHandler = std::make_shared<MessageHandlerCallback>();
    auto ret = inputMethodController_->RegisterMsgHandler(messageHandler);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodMessageHandlerTest::SetSecurityModeEnable(static_cast<int32_t>(SecurityMode::FULL));
    ret = inputMethodController_->Attach(textListener_);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ArrayBuffer arrayBuffer;
    ret = inputMethodAbility_->SendMessage(arrayBuffer);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(MessageHandlerCallback::WaitSendMessage(arrayBuffer));

    InputMethodMessageHandlerTest::ResetParam();
    ret = inputMethodController_->RegisterMsgHandler(nullptr);
    EXPECT_TRUE(messageHandler->CheckOnTerminatedAndOnMessage(true, true));
}

/**
 * @tc.name: testIMASendMessage_003
 * @tc.desc: IMA SendMessage, msgId is valid and msgParam is 0.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodMessageHandlerTest, testIMASendMessage_003, TestSize.Level0)
{
    IMSA_HILOGI("IMA testIMASendMessage_003 Test START");
    auto messageHandler = std::make_shared<MessageHandlerCallback>();
    auto ret = inputMethodController_->RegisterMsgHandler(messageHandler);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodMessageHandlerTest::SetSecurityModeEnable(static_cast<int32_t>(SecurityMode::FULL));
    ret = inputMethodController_->Attach(textListener_);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ArrayBuffer arrayBuffer;
    arrayBuffer.msgId = "testMsgId";
    ret = inputMethodAbility_->SendMessage(arrayBuffer);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(MessageHandlerCallback::WaitSendMessage(arrayBuffer));

    InputMethodMessageHandlerTest::ResetParam();
    ret = inputMethodController_->RegisterMsgHandler(nullptr);
    EXPECT_TRUE(messageHandler->CheckOnTerminatedAndOnMessage(true, true));
}

/**
 * @tc.name: testIMASendMessage_004
 * @tc.desc: IMA SendMessage, msgId is 0 and msgParam is valid.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodMessageHandlerTest, testIMASendMessage_004, TestSize.Level0)
{
    IMSA_HILOGI("IMA testIMASendMessage_004 Test START");
    auto messageHandler = std::make_shared<MessageHandlerCallback>();
    auto ret = inputMethodController_->RegisterMsgHandler(messageHandler);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodMessageHandlerTest::SetSecurityModeEnable(static_cast<int32_t>(SecurityMode::FULL));
    ret = inputMethodController_->Attach(textListener_);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ArrayBuffer arrayBuffer;
    string msgParam = "testParamtestParamtestParamtestParamtestParamtestParam";
    arrayBuffer.msgParam.assign(msgParam.begin(), msgParam.end());
    ret = inputMethodAbility_->SendMessage(arrayBuffer);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(MessageHandlerCallback::WaitSendMessage(arrayBuffer));

    InputMethodMessageHandlerTest::ResetParam();
    ret = inputMethodController_->RegisterMsgHandler(nullptr);
    EXPECT_TRUE(messageHandler->CheckOnTerminatedAndOnMessage(true, true));
}

/**
 * @tc.name: testIMASendMessage_005
 * @tc.desc: IMA SendMessage, msgId is max length and msgParam is valid.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodMessageHandlerTest, testIMASendMessage_005, TestSize.Level0)
{
    IMSA_HILOGI("IMA testIMASendMessage_005 Test START");
    auto messageHandler = std::make_shared<MessageHandlerCallback>();
    auto ret = inputMethodController_->RegisterMsgHandler(messageHandler);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodMessageHandlerTest::SetSecurityModeEnable(static_cast<int32_t>(SecurityMode::FULL));
    ret = inputMethodController_->Attach(textListener_);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ArrayBuffer arrayBuffer;
    arrayBuffer.msgId = string(MAX_ARRAY_BUFFER_MSG_ID_SIZE, 'a');
    string msgParam = "testParamtestParamtestParamtestParamtestParamtestParam";
    arrayBuffer.msgParam.assign(msgParam.begin(), msgParam.end());
    ret = inputMethodAbility_->SendMessage(arrayBuffer);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(MessageHandlerCallback::WaitSendMessage(arrayBuffer));

    InputMethodMessageHandlerTest::ResetParam();
    ret = inputMethodController_->RegisterMsgHandler(nullptr);
    EXPECT_TRUE(messageHandler->CheckOnTerminatedAndOnMessage(true, true));
}

/**
 * @tc.name: testIMASendMessage_006
 * @tc.desc: IMA SendMessage, msgId is valid and msgParam is max length.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodMessageHandlerTest, testIMASendMessage_006, TestSize.Level0)
{
    IMSA_HILOGI("IMA testIMASendMessage_006 Test START");
    auto messageHandler = std::make_shared<MessageHandlerCallback>();
    auto ret = inputMethodController_->RegisterMsgHandler(messageHandler);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodMessageHandlerTest::SetSecurityModeEnable(static_cast<int32_t>(SecurityMode::FULL));
    ret = inputMethodController_->Attach(textListener_);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ArrayBuffer arrayBuffer;
    arrayBuffer.msgId = "testMsgId";
    arrayBuffer.msgParam.assign(MAX_ARRAY_BUFFER_MSG_PARAM_SIZE, 'a');
    ret = inputMethodAbility_->SendMessage(arrayBuffer);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(MessageHandlerCallback::WaitSendMessage(arrayBuffer));

    InputMethodMessageHandlerTest::ResetParam();
    ret = inputMethodController_->RegisterMsgHandler(nullptr);
    EXPECT_TRUE(messageHandler->CheckOnTerminatedAndOnMessage(true, true));
}

/**
 * @tc.name: testIMASendMessage_007
 * @tc.desc: IMA SendMessage, msgId is valid and msgParam is exceeds max length.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodMessageHandlerTest, testIMASendMessage_007, TestSize.Level0)
{
    IMSA_HILOGI("IMA testIMASendMessage_007 Test START");
    auto messageHandler = std::make_shared<MessageHandlerCallback>();
    auto ret = inputMethodController_->RegisterMsgHandler(messageHandler);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodMessageHandlerTest::SetSecurityModeEnable(static_cast<int32_t>(SecurityMode::FULL));
    ret = inputMethodController_->Attach(textListener_);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ArrayBuffer arrayBuffer;
    arrayBuffer.msgId = "testMsgId";
    arrayBuffer.msgParam.assign(MAX_ARRAY_BUFFER_MSG_PARAM_SIZE + 1, 'a');
    ret = inputMethodAbility_->SendMessage(arrayBuffer);
    EXPECT_EQ(ret, ErrorCode::ERROR_INVALID_ARRAY_BUFFER_SIZE);

    InputMethodMessageHandlerTest::ResetParam();
    ret = inputMethodController_->RegisterMsgHandler(nullptr);
    EXPECT_TRUE(messageHandler->CheckOnTerminatedAndOnMessage(true, false));
}

/**
 * @tc.name: testIMASendMessage_008
 * @tc.desc: IMA SendMessage, msgId is exceeds max length and msgParam is valid.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodMessageHandlerTest, testIMASendMessage_008, TestSize.Level0)
{
    IMSA_HILOGI("IMA testIMASendMessage_008 Test START");
    auto messageHandler = std::make_shared<MessageHandlerCallback>();
    auto ret = inputMethodController_->RegisterMsgHandler(messageHandler);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodMessageHandlerTest::SetSecurityModeEnable(static_cast<int32_t>(SecurityMode::FULL));
    ret = inputMethodController_->Attach(textListener_);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ArrayBuffer arrayBuffer;
    arrayBuffer.msgId = std::string(MAX_ARRAY_BUFFER_MSG_ID_SIZE + 1, 'a');
    string msgParam = "testParamtestParamtestParamtestParamtestParamtestParam";
    arrayBuffer.msgParam.assign(msgParam.begin(), msgParam.end());
    ret = inputMethodAbility_->SendMessage(arrayBuffer);
    EXPECT_EQ(ret, ErrorCode::ERROR_INVALID_ARRAY_BUFFER_SIZE);

    InputMethodMessageHandlerTest::ResetParam();
    ret = inputMethodController_->RegisterMsgHandler(nullptr);
    EXPECT_TRUE(messageHandler->CheckOnTerminatedAndOnMessage(true, false));
}

/**
 * @tc.name: testIMASendMessage_009
 * @tc.desc: IMA SendMessage, msgId and msgParam are all exceed max length.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodMessageHandlerTest, testIMASendMessage_009, TestSize.Level0)
{
    IMSA_HILOGI("IMA testIMASendMessage_009 Test START");
    auto messageHandler = std::make_shared<MessageHandlerCallback>();
    auto ret = inputMethodController_->RegisterMsgHandler(messageHandler);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodMessageHandlerTest::SetSecurityModeEnable(static_cast<int32_t>(SecurityMode::FULL));
    ret = inputMethodController_->Attach(textListener_);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ArrayBuffer arrayBuffer;
    arrayBuffer.msgId = std::string(MAX_ARRAY_BUFFER_MSG_ID_SIZE + 1, 'a');
    arrayBuffer.msgParam.assign(MAX_ARRAY_BUFFER_MSG_PARAM_SIZE + 1, 'a');
    ret = inputMethodAbility_->SendMessage(arrayBuffer);
    EXPECT_EQ(ret, ErrorCode::ERROR_INVALID_ARRAY_BUFFER_SIZE);
    InputMethodMessageHandlerTest::ResetParam();
    ret = inputMethodController_->RegisterMsgHandler(nullptr);
    EXPECT_TRUE(messageHandler->CheckOnTerminatedAndOnMessage(true, false));
}

/**
 * @tc.name: testIMASendMessage_010
 * @tc.desc: IMA SendMessage, without Attach.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodMessageHandlerTest, testIMASendMessage_010, TestSize.Level0)
{
    IMSA_HILOGI("IMA testIMASendMessage_010 Test START");
    auto messageHandler = std::make_shared<MessageHandlerCallback>();
    auto ret = inputMethodController_->RegisterMsgHandler(messageHandler);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodMessageHandlerTest::SetSecurityModeEnable(static_cast<int32_t>(SecurityMode::FULL));

    ArrayBuffer arrayBuffer;
    arrayBuffer.msgId = "testMsgId";
    string msgParam = "testParamtestParamtestParamtestParamtestParamtestParam";
    arrayBuffer.msgParam.assign(msgParam.begin(), msgParam.end());
    ret = inputMethodAbility_->SendMessage(arrayBuffer);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);

    InputMethodMessageHandlerTest::ResetParam();
    ret = inputMethodController_->RegisterMsgHandler(nullptr);
    EXPECT_TRUE(messageHandler->CheckOnTerminatedAndOnMessage(true, false));
}

/**
 * @tc.name: testIMASendMessage_011
 * @tc.desc: IMA SendMessage, Attach but not editable.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodMessageHandlerTest, testIMASendMessage_011, TestSize.Level0)
{
    IMSA_HILOGI("IMA testIMASendMessage_011 Test START");
    auto messageHandler = std::make_shared<MessageHandlerCallback>();
    auto ret = inputMethodController_->RegisterMsgHandler(messageHandler);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodMessageHandlerTest::SetSecurityModeEnable(static_cast<int32_t>(SecurityMode::FULL));
    ret = inputMethodController_->Attach(textListener_);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    inputMethodController_->isEditable_.store(false);

    ArrayBuffer arrayBuffer;
    arrayBuffer.msgId = "testMsgId";
    string msgParam = "testParamtestParamtestParamtestParamtestParamtestParam";
    arrayBuffer.msgParam.assign(msgParam.begin(), msgParam.end());
    ret = inputMethodAbility_->SendMessage(arrayBuffer);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_EDITABLE);

    InputMethodMessageHandlerTest::ResetParam();
    ret = inputMethodController_->RegisterMsgHandler(nullptr);
    EXPECT_TRUE(messageHandler->CheckOnTerminatedAndOnMessage(true, false));
}

/**
 * @tc.name: testIMASendMessage_012
 * @tc.desc: IMA SendMessage, another size was not registered message handler.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodMessageHandlerTest, testIMASendMessage_012, TestSize.Level0)
{
    IMSA_HILOGI("IMA testIMASendMessage_012 Test START");
    InputMethodMessageHandlerTest::SetSecurityModeEnable(static_cast<int32_t>(SecurityMode::FULL));
    auto ret = inputMethodController_->Attach(textListener_);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ArrayBuffer arrayBuffer;
    arrayBuffer.msgId = "testMsgId";
    string msgParam = "testParamtestParamtestParamtestParamtestParamtestParam";
    arrayBuffer.msgParam.assign(msgParam.begin(), msgParam.end());
    ret = inputMethodAbility_->SendMessage(arrayBuffer);
    EXPECT_EQ(ret, ErrorCode::ERROR_MSG_HANDLER_NOT_REGIST);

    InputMethodMessageHandlerTest::ResetParam();
}

/**
 * @tc.name: testIMASendMessage_013
 * @tc.desc: IMA SendMessage, basic security mode.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodMessageHandlerTest, testIMASendMessage_013, TestSize.Level0)
{
    IMSA_HILOGI("IMA testIMASendMessage_013 Test START");
    auto messageHandler = std::make_shared<MessageHandlerCallback>();
    auto ret = inputMethodController_->RegisterMsgHandler(messageHandler);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodMessageHandlerTest::SetSecurityModeEnable(static_cast<int32_t>(SecurityMode::BASIC));
    ret = inputMethodController_->Attach(textListener_);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ArrayBuffer arrayBuffer;
    arrayBuffer.msgId = "testMsgId";
    string msgParam = "testParamtestParamtestParamtestParamtestParamtestParam";
    arrayBuffer.msgParam.assign(msgParam.begin(), msgParam.end());
    ret = inputMethodAbility_->SendMessage(arrayBuffer);
    EXPECT_EQ(ret, ErrorCode::ERROR_SECURITY_MODE_OFF);

    InputMethodMessageHandlerTest::ResetParam();
    ret = inputMethodController_->RegisterMsgHandler(nullptr);
    EXPECT_TRUE(messageHandler->CheckOnTerminatedAndOnMessage(true, false));
}

/**
 * @tc.name: testCallbackTiming_001
 * @tc.desc: IMC SendMessage, the message handler deliver timing, IMC to IMA.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodMessageHandlerTest, testCallbackTiming_001, TestSize.Level0)
{
    ArrayBuffer arrayBufferA;
    arrayBufferA.msgId = "msgIdA";
    string msgParamA = "testParamAtestParamAtestParamAtestParamAtestParamA";
    arrayBufferA.msgParam.assign(msgParamA.begin(), msgParamA.end());
    ArrayBuffer arrayBufferB;
    arrayBufferB.msgId = "msgIdB";
    string msgParamB = "testParamAtestParamAtestParamAtestParamAtestParamB";
    arrayBufferB.msgParam.assign(msgParamB.begin(), msgParamB.end());
    ArrayBuffer arrayBufferC;
    arrayBufferC.msgId = "msgIdC";
    string msgParamC = "testParamAtestParamAtestParamAtestParamAtestParamC";
    arrayBufferC.msgParam.assign(msgParamC.begin(), msgParamC.end());

    ArrayBuffer arrayBuffer;
    arrayBuffer.msgId = arrayBufferA.msgId + arrayBufferB.msgId + arrayBufferC.msgId;
    string msgParam = msgParamA + msgParamB + msgParamC;
    arrayBuffer.msgParam.assign(msgParam.begin(), msgParam.end());

    
    InputMethodMessageHandlerTest::SetSecurityModeEnable(static_cast<int32_t>(SecurityMode::FULL));
    auto ret = inputMethodController_->Attach(textListener_);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    auto messageHandler = std::make_shared<MessageHandlerCallback>();
    ret = inputMethodAbility_->RegisterMsgHandler(messageHandler);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ret = inputMethodController_->SendMessage(arrayBufferA);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = inputMethodController_->SendMessage(arrayBufferB);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = inputMethodController_->SendMessage(arrayBufferC);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    EXPECT_EQ(MessageHandlerCallback::GetTimingArrayBuffer(), arrayBuffer);
    InputMethodMessageHandlerTest::ResetParam();
}

/**
 * @tc.name: textMessageHandlerProxy_001
 * @tc.desc: create and destroy MessageHandlerProxy success
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodMessageHandlerTest, textMessageHandlerProxy_001, TestSize.Level0)
{
    IMSA_HILOGI("Capi textMessageHandlerProxy_001 Test START");
    auto messageHanlderProxy = OH_MessageHandlerProxy_Create();
    ASSERT_NE(nullptr, messageHanlderProxy);
    InputMethodMessageHandlerTest::ConstructMessageHandlerProxy(messageHanlderProxy);
    InputMethodMessageHandlerTest::TestGetMessageHandlerProxyMember(messageHanlderProxy);
    OH_MessageHandlerProxy_Destroy(messageHanlderProxy);
}

/**
 * @tc.name: testSendMessageCapi_001
 * @tc.desc: input parameters invalid.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodMessageHandlerTest, testSendMessageCapi_001, TestSize.Level0)
{
    IMSA_HILOGI("Capi testSendMessageCapi_001 Test START");
    InputMethod_InputMethodProxy *imeProxy = nullptr;
    auto ret = OH_InputMethodController_Attach(textEditorProxy_, option_, &imeProxy);
    EXPECT_EQ(ret, IME_ERR_OK);
    ASSERT_NE(nullptr, imeProxy);

    std::u16string msgIdStr = u"msgId";
    string msgParamStr = "testParamtestParamtestParamtestParamtestParamtestParam";
    vector<uint8_t> msgParam(msgParamStr.begin(), msgParamStr.end());

    std::u16string msgIdStrOverSize(MAX_ARRAY_BUFFER_MSG_ID_SIZE + 1, 'a');
    vector<uint8_t> msgParamOverSize(MAX_ARRAY_BUFFER_MSG_PARAM_SIZE + 1, 'a');

    ret = OH_InputMethodProxy_SendMessage(nullptr,
        msgIdStr.c_str(), msgIdStr.length(), msgParam.data(), msgParam.size());
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    ret = OH_InputMethodProxy_SendMessage(imeProxy,
        nullptr, msgIdStr.length(), msgParam.data(), msgParam.size());
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    ret = OH_InputMethodProxy_SendMessage(imeProxy,
        msgIdStr.c_str(), msgIdStr.length(), nullptr, msgParam.size());
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);

    ret = OH_InputMethodProxy_SendMessage(imeProxy,
        msgIdStrOverSize.c_str(), msgIdStrOverSize.length(), msgParam.data(), msgParam.size());
    EXPECT_EQ(ret, IME_ERR_PARAMCHECK);
    ret = OH_InputMethodProxy_SendMessage(imeProxy,
        msgIdStr.c_str(), msgIdStr.length(), msgParamOverSize.data(), msgParamOverSize.size());
    EXPECT_EQ(ret, IME_ERR_PARAMCHECK);
    ret = OH_InputMethodProxy_SendMessage(imeProxy,
        msgIdStrOverSize.c_str(), msgIdStrOverSize.length(), msgParamOverSize.data(), msgParamOverSize.size());
    EXPECT_EQ(ret, IME_ERR_PARAMCHECK);

    ret = OH_InputMethodController_Detach(imeProxy);
    EXPECT_EQ(ret, IME_ERR_OK);
    InputMethodMessageHandlerTest::ResetParam();
}

/**
 * @tc.name: testSendMessageCapi_002
 * @tc.desc: input valid parameters.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodMessageHandlerTest, testSendMessageCapi_002, TestSize.Level0)
{
    IMSA_HILOGI("Capi testSendMessageCapi_002 Test START");
    InputMethod_InputMethodProxy *imeProxy = nullptr;
    auto ret = OH_InputMethodController_Attach(textEditorProxy_, option_, &imeProxy);
    EXPECT_EQ(ret, IME_ERR_OK);
    ASSERT_NE(nullptr, imeProxy);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    InputMethodMessageHandlerTest::SetSecurityModeEnable(static_cast<int32_t>(SecurityMode::FULL));
    auto messageHandler = std::make_shared<MessageHandlerCallback>();
    auto res = inputMethodAbility_->RegisterMsgHandler(messageHandler);
    EXPECT_EQ(res, ErrorCode::NO_ERROR);

    ArrayBuffer arrayBuffer;
    arrayBuffer.msgId = "msgId";
    std::u16string msgIdStr = Str8ToStr16(arrayBuffer.msgId);
    string msgParamStr = "testParamtestParamtestParamtestParamtestParamtestParam";
    arrayBuffer.msgParam.assign(msgParamStr.begin(), msgParamStr.end());

    ret = OH_InputMethodProxy_SendMessage(
        imeProxy, msgIdStr.c_str(), msgIdStr.length(), arrayBuffer.msgParam.data(), msgParamStr.size());
    EXPECT_EQ(ret, IME_ERR_OK);

    EXPECT_TRUE(MessageHandlerCallback::WaitSendMessage(arrayBuffer));
    ret = OH_InputMethodController_Detach(imeProxy);
    EXPECT_EQ(ret, IME_ERR_OK);
    InputMethodMessageHandlerTest::ResetParam();
    res = inputMethodAbility_->RegisterMsgHandler();
    EXPECT_EQ(res, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: testRecvMessageCapi_001
 * @tc.desc: register inputmethod message handler with invalid param.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodMessageHandlerTest, testRecvMessageCapi_001, TestSize.Level0)
{
    IMSA_HILOGI("Capi testRecvMessageCapi_001 Test START");
    InputMethod_InputMethodProxy *imeProxy = nullptr;
    auto ret = OH_InputMethodController_Attach(textEditorProxy_, option_, &imeProxy);
    EXPECT_EQ(ret, IME_ERR_OK);
    ASSERT_NE(nullptr, imeProxy);

    auto messageHanlderProxy = OH_MessageHandlerProxy_Create();
    ASSERT_NE(nullptr, messageHanlderProxy);
    ret = OH_InputMethodProxy_RecvMessage(imeProxy, messageHanlderProxy);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    
    InputMethodMessageHandlerTest::ConstructMessageHandlerProxy(messageHanlderProxy);
    ret = OH_InputMethodProxy_RecvMessage(nullptr, messageHanlderProxy);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);

    ret = OH_InputMethodController_Detach(imeProxy);
    EXPECT_EQ(ret, IME_ERR_OK);
    OH_MessageHandlerProxy_Destroy(messageHanlderProxy);
    InputMethodMessageHandlerTest::ResetParam();
}

/**
 * @tc.name: testRecvMessageCapi_002
 * @tc.desc: register inputmethod message handler with valid param.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodMessageHandlerTest, testRecvMessageCapi_002, TestSize.Level0)
{
    IMSA_HILOGI("Capi testRecvMessageCapi_002 Test START");
    InputMethod_InputMethodProxy *imeProxy = nullptr;
    auto ret = OH_InputMethodController_Attach(textEditorProxy_, option_, &imeProxy);
    EXPECT_EQ(ret, IME_ERR_OK);
    ASSERT_NE(nullptr, imeProxy);

    ret = OH_InputMethodProxy_RecvMessage(imeProxy, messageHanlderProxy_);
    EXPECT_EQ(ret, IME_ERR_OK);

    ret = OH_InputMethodProxy_RecvMessage(imeProxy, nullptr);
    EXPECT_EQ(ret, IME_ERR_OK);
    ret = OH_InputMethodController_Detach(imeProxy);
    EXPECT_EQ(ret, IME_ERR_OK);
    InputMethodMessageHandlerTest::ResetParam();
}

/**
 * @tc.name: testRecvMessageCapi_003
 * @tc.desc: register another inputmethod message handler, will triiger exMessage handler OnTerminated.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodMessageHandlerTest, testRecvMessageCapi_003, TestSize.Level0)
{
    IMSA_HILOGI("Capi testRecvMessageCapi_003 Test START");
    InputMethod_InputMethodProxy *imeProxy = nullptr;
    auto ret = OH_InputMethodController_Attach(textEditorProxy_, option_, &imeProxy);
    EXPECT_EQ(ret, IME_ERR_OK);
    ASSERT_NE(nullptr, imeProxy);

    ret = OH_InputMethodProxy_RecvMessage(imeProxy, messageHanlderProxy_);
    EXPECT_EQ(ret, IME_ERR_OK);
    auto messageHanlderProxy = OH_MessageHandlerProxy_Create();
    EXPECT_EQ(IME_ERR_OK, OH_MessageHandlerProxy_SetOnTerminatedFunc(messageHanlderProxy, OnTerminatedFuncNew));
    EXPECT_EQ(IME_ERR_OK, OH_MessageHandlerProxy_SetOnMessageFunc(messageHanlderProxy, OnMessageFunc));
    ret = OH_InputMethodProxy_RecvMessage(imeProxy, messageHanlderProxy);
    EXPECT_EQ(ret, IME_ERR_OK);
    EXPECT_TRUE(g_onTerminated);
    EXPECT_FALSE(g_onMessage);
    EXPECT_FALSE(g_onTerminatedNew);

    ret = OH_InputMethodProxy_RecvMessage(imeProxy, nullptr);
    EXPECT_EQ(ret, IME_ERR_OK);
    OH_MessageHandlerProxy_Destroy(messageHanlderProxy);
    ret = OH_InputMethodController_Detach(imeProxy);
    EXPECT_EQ(ret, IME_ERR_OK);
    InputMethodMessageHandlerTest::ResetParam();
}

/**
 * @tc.name: testMessageHandelrCallbackCapi_001
 * @tc.desc: recv message handler from ima.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodMessageHandlerTest, testMessageHandelrCallbackCapi_001, TestSize.Level0)
{
    IMSA_HILOGI("Capi testMessageHandelrCallbackCapi_001 Test START");
    InputMethod_InputMethodProxy *imeProxy = nullptr;
    auto ret = OH_InputMethodController_Attach(textEditorProxy_, option_, &imeProxy);
    EXPECT_EQ(ret, IME_ERR_OK);
    ASSERT_NE(nullptr, imeProxy);
    ret = OH_InputMethodProxy_RecvMessage(imeProxy, messageHanlderProxy_);
    EXPECT_EQ(ret, IME_ERR_OK);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    InputMethodMessageHandlerTest::SetSecurityModeEnable(static_cast<int32_t>(SecurityMode::FULL));

    ArrayBuffer arrayBuffer;
    arrayBuffer.msgId = "msgId";
    std::u16string msgIdStr = Str8ToStr16(arrayBuffer.msgId);
    string msgParamStr = "testParamtestParamtestParamtestParamtestParamtestParam";
    arrayBuffer.msgParam.assign(msgParamStr.begin(), msgParamStr.end());
    EXPECT_EQ(inputMethodAbility_->SendMessage(arrayBuffer), ErrorCode::NO_ERROR);
    EXPECT_TRUE(WaitOnMessageFunc(arrayBuffer));

    ret = OH_InputMethodProxy_RecvMessage(imeProxy, nullptr);
    EXPECT_EQ(ret, IME_ERR_OK);
    ret = OH_InputMethodController_Detach(imeProxy);
    EXPECT_EQ(ret, IME_ERR_OK);
    InputMethodMessageHandlerTest::ResetParam();
}
} // namespace MiscServices
} // namespace OHOS