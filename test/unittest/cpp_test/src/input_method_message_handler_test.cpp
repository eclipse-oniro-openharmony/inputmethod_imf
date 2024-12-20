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
#include "task_manager.h"
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
#include "input_client_stub.h"
#include "input_death_recipient.h"
#include "input_method_ability.h"
#include "input_method_engine_listener_impl.h"
#include "input_method_system_ability_proxy.h"
#include "input_method_utils.h"
#include "keyboard_listener.h"
#include "message_parcel.h"
#include "tdd_util.h"
#include "text_listener.h"
#include "msg_handler_callback_interface.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace MiscServices {
constexpr int32_t MAX_ARRAY_BUFFER_MSG_ID = 256; // 256B
constexpr int32_t MAX_ARRAY_BUFFER_MSG_PARAM = 128 * 1024; // 128KB
class MessageHandlerCallback : public MsgHandlerCallbackInterface {
public:
    using onTerminatedFunc = std::function<void()>;
    using onMessageFunc = std::function<void(const ArrayBuffer &arrayBuffer)>;
    MessageHandlerCallback() {};
    virtual ~MessageHandlerCallback() {};
    virtual int32_t OnTerminated()
    {
        isTriggerOnTerminated_ = true;
        if (onTerminatedFunc_ != nullptr) {
            onTerminatedFunc_();
        }
        return ErrorCode::NO_ERROR;
    }
    virtual int32_t OnMessage(const ArrayBuffer &arrayBuffer)
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
        return ErrorCode::NO_ERROR;
    }
    void SetOnTernimateFunc(onTerminatedFunc func) {
        onTerminatedFunc_ = func;
    }
    void SetOnMessageFunc(onMessageFunc func) {
        onMessageFunc_ = func;
    }
    bool CheckOnTerminatedAndOnMessage(bool isOnTerminatedExcute, bool isOnMessageExcute) {
        return isOnTerminatedExcute == isTriggerOnTerminated_ && isOnMessageExcute == isTriggerOnMessage_;
    }

    static void OnMessageCallback(const ArrayBuffer &arrayBuffer)
    {
        IMSA_HILOGI("OnMessageCallback");
        arrayBuffer_ = arrayBuffer;
        messageHandlerCv_.notify_one();
    }

    static bool WaitSendMessage(const ArrayBuffer &arrayBuffer)
    {
        std::unique_lock<std::mutex> lock(messageHandlerMutex_);
        messageHandlerCv_.wait_for(lock, std::chrono::seconds(1), [&arrayBuffer]() {
            return arrayBuffer_ == arrayBuffer;
        });
        return arrayBuffer_ == arrayBuffer;
    }

    static void ClearArrayBuffer() {
        arrayBuffer_.msgId.clear();
        arrayBuffer_.msgParam.clear();
    }
private:
    onTerminatedFunc onTerminatedFunc_ = nullptr;
    onMessageFunc onMessageFunc_ = nullptr;
    bool isTriggerOnTerminated_ = false;
    bool isTriggerOnMessage_ = false;
    static std::mutex messageHandlerMutex_;
    static std::condition_variable messageHandlerCv_;
    static ArrayBuffer arrayBuffer_;
};
std::mutex MessageHandlerCallback::messageHandlerMutex_;
std::condition_variable MessageHandlerCallback::messageHandlerCv_;
ArrayBuffer MessageHandlerCallback::arrayBuffer_;

class InputMethodMessageHandlerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static void SetSecurityModeEnable(int32_t securityMode);
    static void ResetParam();;
    static sptr<InputMethodController> inputMethodController_;
    static sptr<InputMethodAbility> inputMethodAbility_;
    static sptr<InputMethodSystemAbility> imsa_;
    static sptr<InputMethodSystemAbilityProxy> imsaProxy_;
    static std::shared_ptr<InputMethodEngineListenerImpl> imeListener_;
    static std::shared_ptr<AppExecFwk::EventHandler> textConfigHandler_;
    static sptr<OnTextChangedListener> textListener_;
};
sptr<InputMethodController> InputMethodMessageHandlerTest::inputMethodController_;
sptr<InputMethodAbility> InputMethodMessageHandlerTest::inputMethodAbility_;
sptr<InputMethodSystemAbility> InputMethodMessageHandlerTest::imsa_;
sptr<InputMethodSystemAbilityProxy> InputMethodMessageHandlerTest::imsaProxy_;
std::shared_ptr<InputMethodEngineListenerImpl> InputMethodMessageHandlerTest::imeListener_;
sptr<OnTextChangedListener> InputMethodMessageHandlerTest::textListener_;
std::shared_ptr<AppExecFwk::EventHandler> InputMethodMessageHandlerTest::textConfigHandler_ { nullptr };

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
}

void InputMethodMessageHandlerTest::TearDownTestCase(void)
{
    IMSA_HILOGI("InputMethodMessageHandlerTest::TearDownTestCase");
    inputMethodController_->SetControllerListener(nullptr);
    IdentityCheckerMock::ResetParam();
    imsa_->OnStop();
}

void InputMethodMessageHandlerTest::SetUp(void)
{
    IMSA_HILOGI("InputMethodMessageHandlerTest::SetUp");
    TaskManager::GetInstance().SetInited(true);
}

void InputMethodMessageHandlerTest::TearDown(void)
{
    IMSA_HILOGI("InputMethodMessageHandlerTest::TearDown");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    TaskManager::GetInstance().Reset();
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

    ArrayBuffer arrayBuffer;
    arrayBuffer.msgId = "testMsgId";
    string msgParam = "testParamtestParamtestParamtestParamtestParamtestParam";
    arrayBuffer.msgParam.assign(msgParam.begin(), msgParam.end());
    ret = inputMethodController_->SendMessage(arrayBuffer);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    IMSA_HILOGI("IMC WaitSendMessage Test START");

    EXPECT_TRUE(InputMethodEngineListenerImpl::WaitSendMessage(arrayBuffer));
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
    ArrayBuffer arrayBuffer;
    ret = inputMethodController_->SendMessage(arrayBuffer);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(InputMethodEngineListenerImpl::WaitSendMessage(arrayBuffer));
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
    ArrayBuffer arrayBuffer;
    arrayBuffer.msgId = "testMsgId";
    ret = inputMethodController_->SendMessage(arrayBuffer);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(InputMethodEngineListenerImpl::WaitSendMessage(arrayBuffer));
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
    ArrayBuffer arrayBuffer;
    string msgParam = "testParamtestParamtestParamtestParamtestParamtestParam";
    arrayBuffer.msgParam.assign(msgParam.begin(), msgParam.end());
    ret = inputMethodController_->SendMessage(arrayBuffer);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(InputMethodEngineListenerImpl::WaitSendMessage(arrayBuffer));
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
    ArrayBuffer arrayBuffer;
    arrayBuffer.msgId = std::string(MAX_ARRAY_BUFFER_MSG_ID, 'a');
    string msgParam = "testParamtestParamtestParamtestParamtestParamtestParam";
    arrayBuffer.msgParam.assign(msgParam.begin(), msgParam.end());
    ret = inputMethodController_->SendMessage(arrayBuffer);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(InputMethodEngineListenerImpl::WaitSendMessage(arrayBuffer));
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
    ArrayBuffer arrayBuffer;
    arrayBuffer.msgId = "testMsgId";
    arrayBuffer.msgParam.assign(MAX_ARRAY_BUFFER_MSG_PARAM, 'a');
    ret = inputMethodController_->SendMessage(arrayBuffer);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(InputMethodEngineListenerImpl::WaitSendMessage(arrayBuffer));
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
    ArrayBuffer arrayBuffer;
    arrayBuffer.msgId = "testMsgId";
    arrayBuffer.msgParam.assign(MAX_ARRAY_BUFFER_MSG_PARAM + 1, 'a');
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
    ArrayBuffer arrayBuffer;
    arrayBuffer.msgId = std::string(MAX_ARRAY_BUFFER_MSG_ID + 1, 'a');
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
    ArrayBuffer arrayBuffer;
    arrayBuffer.msgId = std::string(MAX_ARRAY_BUFFER_MSG_ID + 1, 'a');
    arrayBuffer.msgParam.assign(MAX_ARRAY_BUFFER_MSG_PARAM + 1, 'a');
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

    ArrayBuffer arrayBuffer;
    arrayBuffer.msgId = "testMsgId";
    string msgParam = "testParamtestParamtestParamtestParamtestParamtestParam";
    arrayBuffer.msgParam.assign(msgParam.begin(), msgParam.end());
    ret = inputMethodController_->SendMessage(arrayBuffer);
    EXPECT_EQ(ret, ErrorCode::ERROR_SECURITY_MODE_OFF);

    InputMethodMessageHandlerTest::ResetParam();
}

/**
 * @tc.name: testIMCRegisterMsgHandler_001
 * @tc.desc: IMC RegisterMsgHandler, duplicate registration will triggers ex-callback OnTerminated.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodMessageHandlerTest, testIMCRegisterMsgHandler_001, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCRegisterMsgHandler_001 Test START");
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
 * @tc.name: testIMCRegisterMsgHandler_002
 * @tc.desc: IMC RegisterMsgHandler, message handler is globally unique.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodMessageHandlerTest, testIMCRegisterMsgHandler_002, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCRegisterMsgHandler_002 Test START");
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
 * @tc.name: testIMCRegisterMsgHandler_003
 * @tc.desc: IMC RegisterMsgHandler, unregister message handle will trigger OnTerminated.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodMessageHandlerTest, testIMCRegisterMsgHandler_003, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCRegisterMsgHandler_003 Test START");
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
    IMSA_HILOGI("IMC testIMASendMessage_001 Test START");
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
    IMSA_HILOGI("IMC testIMASendMessage_002 Test START");
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
    IMSA_HILOGI("IMC testIMASendMessage_003 Test START");
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
    IMSA_HILOGI("IMC testIMASendMessage_004 Test START");
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
    IMSA_HILOGI("IMC testIMASendMessage_005 Test START");
    auto messageHandler = std::make_shared<MessageHandlerCallback>();
    auto ret = inputMethodController_->RegisterMsgHandler(messageHandler);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodMessageHandlerTest::SetSecurityModeEnable(static_cast<int32_t>(SecurityMode::FULL));
    ret = inputMethodController_->Attach(textListener_);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ArrayBuffer arrayBuffer;
    arrayBuffer.msgId = string(MAX_ARRAY_BUFFER_MSG_ID, 'a');
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
    IMSA_HILOGI("IMC testIMASendMessage_006 Test START");
    auto messageHandler = std::make_shared<MessageHandlerCallback>();
    auto ret = inputMethodController_->RegisterMsgHandler(messageHandler);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodMessageHandlerTest::SetSecurityModeEnable(static_cast<int32_t>(SecurityMode::FULL));
    ret = inputMethodController_->Attach(textListener_);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ArrayBuffer arrayBuffer;
    arrayBuffer.msgId = "testMsgId";
    arrayBuffer.msgParam.assign(MAX_ARRAY_BUFFER_MSG_PARAM, 'a');
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
    IMSA_HILOGI("IMC testIMASendMessage_007 Test START");
    auto messageHandler = std::make_shared<MessageHandlerCallback>();
    auto ret = inputMethodController_->RegisterMsgHandler(messageHandler);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodMessageHandlerTest::SetSecurityModeEnable(static_cast<int32_t>(SecurityMode::FULL));
    ret = inputMethodController_->Attach(textListener_);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ArrayBuffer arrayBuffer;
    arrayBuffer.msgId = "testMsgId";
    arrayBuffer.msgParam.assign(MAX_ARRAY_BUFFER_MSG_PARAM + 1, 'a');
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
    IMSA_HILOGI("IMC testIMASendMessage_008 Test START");
    auto messageHandler = std::make_shared<MessageHandlerCallback>();
    auto ret = inputMethodController_->RegisterMsgHandler(messageHandler);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodMessageHandlerTest::SetSecurityModeEnable(static_cast<int32_t>(SecurityMode::FULL));
    ret = inputMethodController_->Attach(textListener_);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ArrayBuffer arrayBuffer;
    arrayBuffer.msgId = std::string(MAX_ARRAY_BUFFER_MSG_ID + 1, 'a');
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
    IMSA_HILOGI("IMC testIMASendMessage_009 Test START");
    auto messageHandler = std::make_shared<MessageHandlerCallback>();
    auto ret = inputMethodController_->RegisterMsgHandler(messageHandler);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodMessageHandlerTest::SetSecurityModeEnable(static_cast<int32_t>(SecurityMode::FULL));
    ret = inputMethodController_->Attach(textListener_);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ArrayBuffer arrayBuffer;
    arrayBuffer.msgId = std::string(MAX_ARRAY_BUFFER_MSG_ID + 1, 'a');
    arrayBuffer.msgParam.assign(MAX_ARRAY_BUFFER_MSG_PARAM + 1, 'a');
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
    IMSA_HILOGI("IMC testIMASendMessage_010 Test START");
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
    IMSA_HILOGI("IMC testIMASendMessage_011 Test START");
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
    IMSA_HILOGI("IMC testIMASendMessage_012 Test START");
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
    IMSA_HILOGI("IMC testIMASendMessage_013 Test START");
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
 * @tc.desc: SendMessage, the message handler deliver timing, IMC to IMA.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodMessageHandlerTest, testCallbackTiming_001, TestSize.Level0)
{
    IMSA_HILOGI("IMC testCallbackTiming_001 Test START");
    InputMethodMessageHandlerTest::SetSecurityModeEnable(static_cast<int32_t>(SecurityMode::FULL));
    auto ret = inputMethodController_->Attach(textListener_);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ArrayBuffer arrayBuffer;
    arrayBuffer.msgId = "testMsgId";
    string msgParam = "testParamtestParamtestParamtestParamtestParamtestParam";
    arrayBuffer.msgParam.assign(msgParam.begin(), msgParam.end());
    ret = inputMethodController_->SendMessage(arrayBuffer);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    IMSA_HILOGI("IMC WaitSendMessage Test START");
    EXPECT_TRUE(InputMethodEngineListenerImpl::WaitSendMessage(arrayBuffer));
    IMSA_HILOGI("IMC WaitSendMessage Test START");
    InputMethodMessageHandlerTest::ResetParam();
}
} // namespace MiscServices
} // namespace OHOS