/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "inputtypemanager_fuzzer.h"

#define private public
#define protected public
#include "inputtypemanager_fuzzer.h"
#undef private

#include <cstddef>
#include <cstdint>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string_ex.h>

#include "global.h"
#include "i_input_method_agent.h"
#include "i_input_method_core.h"
#include "input_client_proxy.h"
#include "input_client_stub.h"
#include "input_method_ability.h"
#include "input_method_agent_proxy.h"
#include "input_method_agent_stub.h"
#include "input_method_core_proxy.h"
#include "input_method_core_stub.h"
#include "input_method_info.h"
#include "input_method_property.h"
#include "input_method_types.h"
#include "iremote_broker.h"
#include "message_parcel.h"


using namespace OHOS::MiscServices;
namespace OHOS {
constexpr size_t THRESHOLD = 10;
constexpr int32_t OFFSET = 4;
constexpr int32_t PRIVATEDATAVALUE = 100;
const std::u16string AGENTSTUB_INTERFACE_TOKEN = u"ohos.miscservices.inputmethod.ISystemCmdChannel";
uint32_t ConvertToUint32(const uint8_t *ptr)
{
    if (ptr == nullptr) {
        return 0;
    }
    uint32_t bigVar = (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | (ptr[3]);
    return bigVar;
}
bool FuzzIsSupported(const uint8_t *rawData, size_t size)
{
    InputType fuzzedBool = static_cast<InputType>(rawData[0] % 2);
    auto fuzzedUint32 = static_cast<uint32_t>(size);

    uint32_t code = ConvertToUint32(rawData);
    rawData = rawData + OFFSET;
    size = size - OFFSET;

    SysPanelStatus sysPanelStatus = {fuzzedBool, 0, fuzzedUint32, fuzzedUint32};

    std::unordered_map <std::string, PrivateDataValue> privateCommand;
    PrivateDataValue privateDataValue1 = std::string("stringValue");
    PrivateDataValue privateDataValue2 = static_cast<int32_t>(fuzzedBool);
    PrivateDataValue privateDataValue3 = PRIVATEDATAVALUE;
    privateCommand.emplace("value1", privateDataValue1);
    privateCommand.emplace("value2", privateDataValue2);
    privateCommand.emplace("value3", privateDataValue3);

    MessageParcel data;
    data.WriteInterfaceToken(AGENTSTUB_INTERFACE_TOKEN);
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    sptr <SystemCmdChannelStub> stub = new SystemCmdChannelStub();
    stub->SendPrivateCommand(privateCommand);
    stub->NotifyPanelStatus(sysPanelStatus);
    stub->OnRemoteRequest(code, data, reply, option);
    InputTypeManager::GetInstance()->IsSupported();
    manager.Init(); // Ensure configuration is initialized
    return true;
}

bool FuzzIsInputType(int32_t userId, const std::string &packageName)
{
    // onUserStarted
    MessageParcel *parcel = new MessageParcel();
    DelayedSingleton<InputTypeManager>::GetInstance()->isScbEnable_ = false;
    DelayedSingleton<InputTypeManager>::GetInstance()->userId_ = MSG_ID_USER_ONE;
    parcel->WriteInt32(MSG_ID_USER_ONE);
    auto msg = std::make_shared<Message>(MessageID::MSG_ID_USER_START, parcel);
    DelayedSingleton<InputTypeManager>::GetInstance()->OnUserStarted(msg.get());

    // onUserRemoved
    MessageParcel *parcel1 = new MessageParcel();
    parcel1->WriteInt32(MSG_ID_USER_TWO);
    auto msg1 = std::make_shared<Message>(MessageID::MSG_ID_USER_REMOVED, parcel1);
    DelayedSingleton<InputTypeManager>::GetInstance()->OnUserRemoved(msg1.get());

    // HandlePackageEvent
    MessageParcel *parcel2 = new (std::nothrow) MessageParcel();
    auto bundleName = "testBundleName1";
    DelayedSingleton<InputTypeManager>::GetInstance()->userId_ = MSG_ID_USER_TWO;
    parcel2->WriteInt32(MSG_ID_USER_ONE);
    parcel2->WriteString(bundleName);
    auto msg2 = std::make_shared<Message>(MessageID::MSG_ID_PACKAGE_REMOVED, parcel2);
    DelayedSingleton<InputTypeManager>::GetInstance()->HandlePackageEvent(msg2.get());

    // OnPackageRemoved
    DelayedSingleton<InputTypeManager>::GetInstance()->userId_ = userId;
    DelayedSingleton<InputTypeManager>::GetInstance()->OnPackageRemoved(userId, bundleName);

    InputTypeManager::GetInstance()->IsInputType();
    return true;
}

bool FuzzGetImeByInputType(int32_t userId, const std::string &packageName)
{
    auto *msg = new Message(MessageID::MSG_ID_PACKAGE_REMOVED, nullptr);
    auto ret = typeManager_->HandlePackageEvent(msg);
    EXPECT_EQ(ret, ErrorCode::ERROR_NULL_POINTER);
    MessageHandler::Instance()->SendMessage(msg);

    // PARCELABLE failed
    MessageParcel *parcel1 = new (std::nothrow) MessageParcel();
    auto bundleName = "testBundleName1";
    parcel1->WriteString(bundleName);
    auto msg1 = std::make_shared<Message>(MessageID::MSG_ID_PACKAGE_REMOVED, parcel1);
    auto ret1 = typeManager_->HandlePackageEvent(msg1.get());

    // userId is not same
    auto parcel2 = new (std::nothrow) MessageParcel();
    auto userId = 50;
    typeManager_->userId_ = 60;
    parcel2->WriteInt32(userId);
    parcel2->WriteString(bundleName);
    auto msg2 = std::make_shared<Message>(MessageID::MSG_ID_PACKAGE_REMOVED, parcel2);
    auto ret2 = typeManager_->HandlePackageEvent(msg2.get());

    // remove bundle not current ime
    auto parcel3 = new (std::nothrow) MessageParcel();
    typeManager_->userId_ = userId;
    ImeCfgManager::GetInstance().imeConfigs_.push_back({ 60, "testBundleName/testExtName", "testSubName", false });
    parcel3->WriteInt32(userId);
    parcel3->WriteString(bundleName);
    auto msg3 = std::make_shared<Message>(MessageID::MSG_ID_PACKAGE_REMOVED, parcel3);
    auto ret3 = typeManager_->HandlePackageEvent(msg3.get());

    InputTypeManager::GetInstance()->GetImeByInputType();
    return true;
}

bool FuzzSet(int32_t userId, const std::string &packageName)
{
    // imeListener_ == nullptr
    auto ret = inputMethodAbility_->ShowKeyboard();

    auto imeListener = std::make_shared<InputMethodEngineListenerImpl>();
    inputMethodAbility_->SetImeListener(imeListener);
    sptr<InputDataChannelStub> channelObject = new InputDataChannelStub();
    inputMethodAbility_->SetInputDataChannel(channelObject->AsObject());
    // panel exist, PanelFlag == FLG_CANDIDATE_COLUMN
    auto panel = std::make_shared<InputMethodPanel>();
    panel->panelFlag_ = FLG_CANDIDATE_COLUMN;
    panel->windowId_ = 2;
    inputMethodAbility_->panels_.Insert(SOFT_KEYBOARD, panel);
    ret = inputMethodAbility_->ShowKeyboard();
    // panel not exist
    inputMethodAbility_->panels_.Clear();
    ret = inputMethodAbility_->ShowKeyboard();

    InputTypeManager::GetInstance()->Set();
    return true;
}

bool FuzzIsStarted(int32_t userId, const std::string &packageName)
{
    sptr<InputMethodAgentStub> agentStub = new InputMethodAgentStub();
    MessageParcel data;
    data.WriteInterfaceToken(AGENTSTUB_INTERFACE_TOKEN);
    MessageParcel reply;
    MessageOption option;
    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
    keyEvent->WriteToParcel(data);
    data.WriteRemoteObject(nullptr);
    auto ret =
        agentStub->OnRemoteRequest(static_cast<uint32_t>(IInputMethodAgent::DISPATCH_KEY_EVENT), data, reply, option);

    InputTypeManager::GetInstance()->IsStarted();
    return true;
}

bool FuzzIsSecurityImeStarted(int32_t userId, const std::string &packageName)
{
    ConvertToUint32(userId, packageName);
    // imeListener_ == nullptr
    auto ret = inputMethodAbility_->ShowKeyboard();

    std::unordered_map<std::string, PrivateDataValue> privateCommand;
    PrivateDataValue privateDataValue1 = std::string("stringValue");
    privateCommand.emplace("value1", privateDataValue1);
    imeSystemChannel_->systemCmdListener_ = nullptr;
    imeSystemChannel_->OnConnectCmdReady(nullptr);
    imeSystemChannel_->GetSmartMenuCfg();
    int32_t ret = imeSystemChannel_->ReceivePrivateCommand(privateCommand);
    EXPECT_EQ(ret, ErrorCode::ERROR_EX_NULL_POINTER);
    ret = imeSystemChannel_->NotifyPanelStatus({ InputType::NONE, 0, 0, 0 });

    InputTypeManager::GetInstance()->IsSecurityImeStarted();
    return true;
}
} // namespace OHOS
/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < OHOS::THRESHOLD) {
        return 0;
    }
    /* Run your code on data */
    OHOS::FuzzIsSupported(data, size);
    OHOS::FuzzIsInputType(data, size);
    OHOS::FuzzSet(data, size);
    return 0;
}