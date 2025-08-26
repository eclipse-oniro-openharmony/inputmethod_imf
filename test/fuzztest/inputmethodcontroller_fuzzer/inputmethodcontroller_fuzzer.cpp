/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
#include "input_method_system_ability_proxy.h"
#include "input_client_service_impl.h"
#undef private

#include "inputmethodcontroller_fuzzer.h"

#include <cstddef>
#include <cstdint>

#include "fuzzer/FuzzedDataProvider.h"
#include "global.h"
#include "input_attribute.h"
#include "key_event.h"
#include "message_parcel.h"
#include "text_listener.h"

using namespace OHOS::MiscServices;
namespace OHOS {
class EventHandlerTextListenerImpl : public TextListener {
public:
    explicit EventHandlerTextListenerImpl(const std::shared_ptr<AppExecFwk::EventHandler> &handler)
    {
        listenerEventHandler_ = handler;
    }
    std::shared_ptr<AppExecFwk::EventHandler> GetEventHandler()
    {
        return listenerEventHandler_;
    }

private:
    std::shared_ptr<AppExecFwk::EventHandler> listenerEventHandler_{ nullptr };
};
constexpr int32_t PRIVATEDATAVALUE = 100;
void TestListInputMethod(sptr<InputMethodController> imc)
{
    std::vector<Property> properties = {};
    imc->ListInputMethod(properties);
    imc->ListInputMethod(false, properties);
    imc->ListInputMethod(true, properties);
    imc->DisplayOptionalInputMethod();
}

void TestListInputMethodSubtype(sptr<InputMethodController> imc, const std::string &fuzzedString, uint32_t fuzzedUint32)
{
    std::vector<SubProperty> subProperties = {};
    Property property;
    property.name = fuzzedString;
    property.id = fuzzedString;
    property.label = fuzzedString;
    property.icon = fuzzedString;
    property.iconId = fuzzedUint32;
    imc->ListInputMethodSubtype(property, subProperties);
}

void TestDispatchKeyEvent(sptr<InputMethodController> imc, int32_t fuzzedInt32)
{
    sptr<OnTextChangedListener> textListener = new TextListener();
    if (textListener == nullptr) {
        return;
    }
    imc->Attach(textListener);
    imc->isBound_.store(true);

    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
    if (keyEvent == nullptr) {
        return;
    }
    keyEvent->SetKeyAction(fuzzedInt32);
    keyEvent->SetKeyCode(fuzzedInt32);
    imc->DispatchKeyEvent(keyEvent, [](std::shared_ptr<MMI::KeyEvent> &keyEvent, bool isConsumed) {});
}

void TestOnSelectionChange(sptr<InputMethodController> imc, std::u16string fuzzedU16String, int fuzzedInt,
    double fuzzedDouble)
{
    sptr<OnTextChangedListener> textListener = new TextListener();
    if (textListener == nullptr) {
        return;
    }
    imc->Attach(textListener);
    imc->isBound_.store(true);

    CursorInfo cursorInfo;
    cursorInfo.height = fuzzedDouble;
    cursorInfo.left = fuzzedDouble;
    cursorInfo.top = fuzzedDouble;
    cursorInfo.width = fuzzedDouble;
    imc->OnCursorUpdate(cursorInfo);

    imc->OnSelectionChange(fuzzedU16String, fuzzedInt, fuzzedInt);
}

void TestOnConfigurationChange(sptr<InputMethodController> imc)
{
    sptr<OnTextChangedListener> textListener = new TextListener();
    if (textListener == nullptr) {
        return;
    }
    imc->Attach(textListener);
    imc->isBound_.store(true);

    Configuration info;
    EnterKeyType keyType = EnterKeyType::DONE;
    info.SetEnterKeyType(keyType);
    TextInputType textInputType = TextInputType::DATETIME;
    info.SetTextInputType(textInputType);
    imc->OnConfigurationChange(info);
    int32_t enterKeyType;
    int32_t inputPattern;
    imc->GetEnterKeyType(enterKeyType);
    imc->GetInputPattern(inputPattern);
}

void TestSwitchInputMethod(SwitchTrigger fuzzedTrigger, sptr<InputMethodController> imc,
    const std::string &fuzzedString)
{
    imc->SwitchInputMethod(fuzzedTrigger, fuzzedString, fuzzedString);
    imc->ShowOptionalInputMethod();
}

void TestSetCallingWindow(sptr<InputMethodController> imc, uint32_t fuzzedUInt32)
{
    sptr<OnTextChangedListener> textListener = new TextListener();
    if (textListener == nullptr) {
        return;
    }
    imc->Attach(textListener);

    imc->SetCallingWindow(fuzzedUInt32);
    imc->ShowSoftKeyboard();
    imc->HideSoftKeyboard();
}

void TestUpdateListenEventFlag(sptr<InputMethodController> imc, uint32_t fuzzedUint32)
{
    imc->UpdateListenEventFlag(static_cast<uint32_t>(fuzzedUint32), static_cast<uint32_t>(fuzzedUint32), true);
    imc->UpdateListenEventFlag(static_cast<uint32_t>(fuzzedUint32), static_cast<uint32_t>(fuzzedUint32), false);
}

void TestAttach(sptr<InputMethodController> imc, int32_t fuzzedInt32)
{
    sptr<OnTextChangedListener> textListener = new TextListener();
    if (textListener == nullptr) {
        return;
    }
    InputAttribute inputAttribute;
    inputAttribute.inputPattern = fuzzedInt32;
    inputAttribute.enterKeyType = fuzzedInt32;
    inputAttribute.inputOption = fuzzedInt32;
    imc->Attach(textListener, true, inputAttribute);
    imc->Attach(textListener, false, inputAttribute);
}

void FUZZRestore(sptr<InputMethodController> imc)
{
    imc->RestoreListenEventFlag();
    imc->RestoreListenInfoInSaDied();
    imc->RestoreClientInfoInSaDied();
}

void InputType(sptr<InputMethodController> imc)
{
    imc->IsInputTypeSupported(InputType::CAMERA_INPUT);
    imc->IsInputTypeSupported(InputType::SECURITY_INPUT);
    imc->StartInputType(InputType::CAMERA_INPUT);
    imc->StartInputType(InputType::SECURITY_INPUT);
}

void FUZZIsPanelShown(sptr<InputMethodController> imc, const uint8_t *data)
{
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    panelInfo.panelFlag = FLG_FIXED;
    bool flag = static_cast<bool>(data[0] % 2);
    imc->IsPanelShown(panelInfo, flag);
}

void FUZZPrintLogIfAceTimeout(sptr<InputMethodController> imc, int64_t start)
{
    imc->PrintLogIfAceTimeout(start);
}

void FUZZGetInputStartInfo(sptr<InputMethodController> imc, bool &dataBool,
    uint32_t &callingWndId, int32_t &int32Value, const std::string &fuzzedString)
{
    imc->GetInputStartInfo(dataBool, callingWndId, int32Value);
    imc->EnableIme(fuzzedString);
    imc->IsCurrentImeByPid(int32Value);
    imc->UpdateTextPreviewState(dataBool);
}

void FUZZSetControllerListener(sptr<InputMethodController> imc,
    uint32_t &uint32Value, const std::string &fuzzedString, bool &dataBool)
{
    sptr<OnTextChangedListener> textListener = new TextListener();
    if (textListener == nullptr) {
        return;
    }
    imc->Attach(textListener);
    static std::vector<SubProperty> subProps;
    static std::shared_ptr<Property> property = std::make_shared<Property>();
    property->name = fuzzedString;
    property->id = fuzzedString;
    property->label = fuzzedString;
    property->icon = fuzzedString;
    property->iconId = uint32Value;

    OHOS::AppExecFwk::ElementName inputMethodConfig;
    inputMethodConfig.SetDeviceID(fuzzedString);
    inputMethodConfig.SetAbilityName(fuzzedString);
    inputMethodConfig.SetBundleName(fuzzedString);
    inputMethodConfig.SetModuleName(fuzzedString);
    wptr<IRemoteObject> agentObject = nullptr;
    SubProperty subProperty;
    subProperty.label = fuzzedString;
    subProperty.labelId = uint32Value;
    subProperty.name = fuzzedString;
    subProperty.id = fuzzedString;
    subProperty.mode = fuzzedString;
    subProperty.locale = fuzzedString;
    subProperty.icon = fuzzedString;
    subProps.push_back(subProperty);
    std::unordered_map <std::string, PrivateDataValue> privateCommand;
    PrivateDataValue privateDataValue1 = fuzzedString;
    PrivateDataValue privateDataValue2 = static_cast<int32_t>(dataBool);
    PrivateDataValue privateDataValue3 = PRIVATEDATAVALUE;
    privateCommand.emplace("value1", privateDataValue1);
    privateCommand.emplace("value2", privateDataValue2);
    privateCommand.emplace("value3", privateDataValue3);
    imc->SetControllerListener(nullptr);
    imc->GetInputMethodConfig(inputMethodConfig);
    imc->OnRemoteSaDied(agentObject);
    imc->SendPrivateCommand(privateCommand);
}
void TestShowTextInputInner(sptr<InputMethodController> imc, const uint8_t *data, size_t size)
{
    sptr<OnTextChangedListener> textListener = new TextListener();
    if (textListener == nullptr) {
        return;
    }
    std::string fuzzedString(reinterpret_cast<const char *>(data), size);
    auto fuzzedBool = static_cast<bool>(data[0] % 2);
    std::vector<uint8_t> msgParam;
    msgParam.push_back(*data);
    std::unordered_map <std::string, PrivateDataValue> privateCommand;
    imc->Attach(textListener);
    AttachOptions attachOptions;
    attachOptions.isShowKeyboard = fuzzedBool;
    attachOptions.requestKeyboardReason = RequestKeyboardReason::NONE;
    ClientType clientType = ClientType::INNER_KIT;
    imc->ShowTextInputInner(attachOptions, clientType);
    imc->isEditable_.store(true);
    imc->SendPrivateData(privateCommand);
}

void FUZZOnTextChangedListener(const uint8_t *data, size_t size)
{
    std::u16string fuzzedU16String = u"insert text";
    std::string fuzzedString(reinterpret_cast<const char *>(data), size);
    auto fuzzInt32 = static_cast<int32_t>(size);
    auto fuzzBool = static_cast<bool>(data[0] % 2);
    FunctionKey key;
    KeyEvent keyEvent;
    Range range = { .start = fuzzInt32, .end = fuzzInt32};
    PanelInfo panelInfo;
    panelInfo.panelType = static_cast<PanelType>(fuzzBool);
    panelInfo.panelFlag = static_cast<PanelFlag>(fuzzBool);
    PanelStatusInfo panelStatusInfo;
    panelStatusInfo.panelInfo = panelInfo;
    panelStatusInfo.visible = fuzzBool;
    panelStatusInfo.trigger = Trigger::IME_APP;
    std::shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create("eventHandlerTextListener");
    auto handler = std::make_shared<AppExecFwk::EventHandler>(runner);
    sptr<OnTextChangedListener> eventHandlerTextListener = new (std::nothrow) EventHandlerTextListenerImpl(handler);
    if (eventHandlerTextListener == nullptr) {
        return;
    }
    std::unordered_map <std::string, PrivateDataValue> privateCommand;
    PrivateDataValue privateDataValue1 = fuzzedString;
    PrivateDataValue privateDataValue2 = static_cast<int32_t>(fuzzBool);
    PrivateDataValue privateDataValue3 = PRIVATEDATAVALUE;
    privateCommand.emplace("value1", privateDataValue1);
    privateCommand.emplace("value2", privateDataValue2);
    privateCommand.emplace("value3", privateDataValue3);
    eventHandlerTextListener->InsertTextV2(fuzzedU16String);
    eventHandlerTextListener->DeleteForwardV2(fuzzInt32);
    eventHandlerTextListener->DeleteBackwardV2(fuzzInt32);
    eventHandlerTextListener->SendKeyboardStatusV2(KeyboardStatus::SHOW);
    eventHandlerTextListener->SendFunctionKeyV2(key);
    eventHandlerTextListener->MoveCursorV2(Direction::DOWN);
    eventHandlerTextListener->HandleExtendActionV2(fuzzInt32);
    eventHandlerTextListener->GetLeftTextOfCursorV2(fuzzInt32);
    eventHandlerTextListener->GetRightTextOfCursorV2(fuzzInt32);
    eventHandlerTextListener->GetTextIndexAtCursorV2();
    eventHandlerTextListener->SendKeyEventFromInputMethodV2(keyEvent);
    eventHandlerTextListener->SetKeyboardStatusV2(fuzzBool);
    eventHandlerTextListener->HandleSetSelectionV2(fuzzInt32, fuzzInt32);
    eventHandlerTextListener->HandleSelectV2(fuzzInt32, fuzzInt32);
    eventHandlerTextListener->OnDetachV2();
    eventHandlerTextListener->NotifyPanelStatusInfoV2(panelStatusInfo);
    eventHandlerTextListener->NotifyKeyboardHeightV2(fuzzInt32);
    eventHandlerTextListener->ReceivePrivateCommandV2(privateCommand);
    eventHandlerTextListener->FinishTextPreviewV2();
    eventHandlerTextListener->SetPreviewTextV2(fuzzedU16String, range);
}

void FUZZCovered(sptr<InputMethodController> imc, const uint8_t *data, size_t size)
{
    std::string fuzzedString(reinterpret_cast<const char *>(data), size);
    std::vector<uint8_t> msgParam;
    msgParam.push_back(*data);
    ArrayBuffer arrayBuffer;
    arrayBuffer.jsArgc = size;
    arrayBuffer.msgId = fuzzedString;
    arrayBuffer.msgParam = msgParam;
    imc->SendMessage(arrayBuffer);
    imc->RecvMessage(arrayBuffer);
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr || size == 0) {
        return 0;
    }
    FuzzedDataProvider provider(data, size);
    std::string fuzzedString(data, data + size);
    std::u16string fuzzedU16String = u"insert text";

    auto fuzzedInt = provider.ConsumeIntegral<int>();
    auto fuzzedInt32 = provider.ConsumeIntegral<int32_t>();
    auto fuzzedUint32 = provider.ConsumeIntegral<uint32_t>();
    auto fuzzedint64 = provider.ConsumeIntegral<int64_t>();
    auto fuzzedDouble = static_cast<double>(size);
    auto fuzzedTrigger = static_cast<SwitchTrigger>(size);
    auto fuzzedBool = provider.ConsumeBool();

    OHOS::sptr<InputMethodController> imc = InputMethodController::GetInstance();

    OHOS::TestListInputMethod(imc);
    OHOS::TestListInputMethodSubtype(imc, fuzzedString, fuzzedUint32);
    OHOS::TestOnSelectionChange(imc, fuzzedU16String, fuzzedInt, fuzzedDouble);
    OHOS::TestOnConfigurationChange(imc);
    OHOS::TestSwitchInputMethod(fuzzedTrigger, imc, fuzzedString);
    OHOS::TestSetCallingWindow(imc, fuzzedUint32);
    OHOS::TestDispatchKeyEvent(imc, fuzzedInt32);
    OHOS::FUZZRestore(imc);
    OHOS::InputType(imc);
    OHOS::FUZZIsPanelShown(imc, data);
    OHOS::FUZZPrintLogIfAceTimeout(imc, fuzzedint64);
    OHOS::TestUpdateListenEventFlag(imc, fuzzedUint32);
    OHOS::FUZZGetInputStartInfo(imc, fuzzedBool, fuzzedUint32, fuzzedInt32, fuzzedString);
    OHOS::FUZZSetControllerListener(imc, fuzzedUint32, fuzzedString, fuzzedBool);
    OHOS::TestShowTextInputInner(imc, data, size);
    OHOS::FUZZCovered(imc, data, size);
    return 0;
}
