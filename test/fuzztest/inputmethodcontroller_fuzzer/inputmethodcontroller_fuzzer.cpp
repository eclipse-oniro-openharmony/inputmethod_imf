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

#include "inputmethodcontroller_fuzzer.h"

#include <cstddef>
#include <cstdint>

#include "global.h"
#include "input_method_controller.h"
#include "key_event.h"
#include "message_parcel.h"
#include "input_attribute.h"
#include "text_listener.h"

using namespace OHOS::MiscServices;
namespace OHOS {
class SettingListener : public InputMethodSettingListener {
    void OnImeChange(const Property &property, const SubProperty &subProperty) {}
    void OnPanelStatusChange(const InputWindowStatus &status, const std::vector<InputWindowInfo> &windowInfo) {}
};

void TestListInputMethod(sptr<InputMethodController> imc)
{
    std::vector<Property> properties = {};
    imc->ListInputMethod(properties);
    imc->ListInputMethod(false, properties);
    imc->ListInputMethod(true, properties);
    imc->DisplayOptionalInputMethod();
}

void TestListInputMethodSubtype(sptr<InputMethodController> imc, const std::string &fuzzedString, int32_t fuzzedInt32)
{
    std::vector<SubProperty> subProperties = {};
    Property property;
    property.name = fuzzedString;
    property.id = fuzzedString;
    property.label = fuzzedString;
    property.icon = fuzzedString;
    property.iconId = fuzzedInt32;
    imc->ListInputMethodSubtype(property, subProperties);
}

void TestDispatchKeyEvent(sptr<InputMethodController> imc, int32_t fuzzedInt32)
{
    sptr<OnTextChangedListener> textListener = new TextListener();
    imc->Attach(textListener);

    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
    keyEvent->SetKeyAction(fuzzedInt32);
    keyEvent->SetKeyCode(fuzzedInt32);
    imc->DispatchKeyEvent(keyEvent);
}

void TestOnSelectionChange(
    sptr<InputMethodController> imc, std::u16string fuzzedU16String, int fuzzedInt, double fuzzedDouble)
{
    sptr<OnTextChangedListener> textListener = new TextListener();
    imc->Attach(textListener);

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
    imc->Attach(textListener);

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

void TestSwitchInputMethod(SwitchTrigger fuzzedTrigger, sptr<InputMethodController> imc, std::string fuzzedString)
{
    imc->SwitchInputMethod(fuzzedTrigger, fuzzedString, fuzzedString);
    imc->ShowOptionalInputMethod();
}

void TestSetCallingWindow(sptr<InputMethodController> imc, uint32_t fuzzedUInt32)
{
    sptr<OnTextChangedListener> textListener = new TextListener();
    imc->Attach(textListener);

    imc->SetCallingWindow(fuzzedUInt32);
    imc->ShowSoftKeyboard();
    imc->HideSoftKeyboard();
}

void TestShowSomething(sptr<InputMethodController> imc)
{
    sptr<OnTextChangedListener> textListener = new TextListener();
    imc->Attach(textListener);
    imc->ShowCurrentInput();
    imc->HideCurrentInput();

    imc->ShowTextInput();
    imc->HideTextInput();

    imc->GetCurrentInputMethod();
    imc->GetCurrentInputMethodSubtype();

    auto settingListener = std::make_shared<SettingListener>();
    imc->SetSettingListener(settingListener);
    imc->UpdateListenEventFlag("imeChange", true);

    imc->StopInputSession();
    imc->Close();
}

void TestUpdateListenEventFlag(sptr<InputMethodController> imc, const std::string &fuzzedString)
{
    imc->UpdateListenEventFlag(fuzzedString, true);
    imc->UpdateListenEventFlag(fuzzedString, false);
}

void TestAttach(sptr<InputMethodController> imc, int32_t fuzzedInt32)
{
    sptr<OnTextChangedListener> textListener = new TextListener();
    InputAttribute inputAttribute;
    inputAttribute.inputPattern = fuzzedInt32;
    inputAttribute.enterKeyType = fuzzedInt32;
    inputAttribute.inputOption = fuzzedInt32;
    imc->Attach(textListener, true, inputAttribute);
    imc->Attach(textListener, false, inputAttribute);
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    std::string fuzzedString(data, data + size);
    std::u16string fuzzedU16String = u"insert text";

    auto fuzzedInt = static_cast<int>(size);
    auto fuzzedInt32 = static_cast<int32_t>(size);
    auto fuzzedUInt32 = static_cast<uint32_t>(size);
    auto fuzzedDouble = static_cast<double>(size);
    auto fuzzedTrigger = static_cast<SwitchTrigger>(size);

    OHOS::sptr<InputMethodController> imc = InputMethodController::GetInstance();

    OHOS::TestListInputMethod(imc);
    OHOS::TestListInputMethodSubtype(imc, fuzzedString, fuzzedInt32);
    OHOS::TestOnSelectionChange(imc, fuzzedU16String, fuzzedInt, fuzzedDouble);
    OHOS::TestOnConfigurationChange(imc);
    OHOS::TestSwitchInputMethod(fuzzedTrigger, imc, fuzzedString);
    OHOS::TestSetCallingWindow(imc, fuzzedUInt32);
    OHOS::TestDispatchKeyEvent(imc, fuzzedInt32);
    OHOS::TestShowSomething(imc);
    OHOS::TestUpdateListenEventFlag(imc, fuzzedString);
    return 0;
}
