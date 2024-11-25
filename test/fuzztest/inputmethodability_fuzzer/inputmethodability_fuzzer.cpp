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

#include "inputmethodability_fuzzer.h"

#include <utility>

#include "input_method_ability.h"
#include "input_method_engine_listener_impl.h"

using namespace OHOS::MiscServices;
namespace OHOS {
class KeyboardListenerImpl : public KeyboardListener {
    bool OnKeyEvent(int32_t keyCode, int32_t keyStatus, sptr<KeyEventConsumerProxy> &consumer)
    {
        return true;
    }
    bool OnKeyEvent(const std::shared_ptr<MMI::KeyEvent> &keyEvent, sptr<KeyEventConsumerProxy> &consumer)
    {
        return true;
    }
    bool OnDealKeyEvent(const std::shared_ptr<MMI::KeyEvent> &keyEvent, sptr<KeyEventConsumerProxy> &consumer)
    {
        return true;
    }
    void OnCursorUpdate(int32_t positionX, int32_t positionY, int32_t height) { }
    void OnSelectionChange(int32_t oldBegin, int32_t oldEnd, int32_t newBegin, int32_t newEnd) { }
    void OnTextChange(const std::string &text) { }
    void OnEditorAttributeChange(const InputAttribute &inputAttribute) { }
};

void TestInsertText(const std::string &fuzzedString)
{
    sptr<InputMethodAbility> ability = InputMethodAbility::GetInstance();
    ability->InsertText(std::move(fuzzedString));
}

void TestSetImeListener()
{
    sptr<InputMethodAbility> ability = InputMethodAbility::GetInstance();
    auto engineListener = std::make_shared<InputMethodEngineListenerImpl>();
    ability->SetImeListener(engineListener);
}

void TestSetKdListener()
{
    sptr<InputMethodAbility> ability = InputMethodAbility::GetInstance();
    ability->SetKdListener(nullptr);
}

void TestDeleteForward(int32_t fuzzedInt32)
{
    sptr<InputMethodAbility> ability = InputMethodAbility::GetInstance();
    ability->DeleteForward(fuzzedInt32);
}

void TestDeleteBackward(int32_t fuzzedInt32)
{
    sptr<InputMethodAbility> ability = InputMethodAbility::GetInstance();
    ability->DeleteBackward(fuzzedInt32);
}

void TestSendExtendAction(int32_t fuzzedInt32)
{
    sptr<InputMethodAbility> ability = InputMethodAbility::GetInstance();
    ability->SendExtendAction(fuzzedInt32);
}

void TestHideKeyboardSelf()
{
    sptr<InputMethodAbility> ability = InputMethodAbility::GetInstance();
    ability->HideKeyboardSelf();
}

void TestGetTextBeforeCursor(int32_t fuzzedInt32)
{
    sptr<InputMethodAbility> ability = InputMethodAbility::GetInstance();
    std::u16string text;
    ability->GetTextBeforeCursor(fuzzedInt32, text);
}

void TestGetTextAfterCursor(int32_t fuzzedInt32)
{
    sptr<InputMethodAbility> ability = InputMethodAbility::GetInstance();
    std::u16string text;
    ability->GetTextAfterCursor(fuzzedInt32, text);
}

void TestSendFunctionKey(int32_t fuzzedInt32)
{
    sptr<InputMethodAbility> ability = InputMethodAbility::GetInstance();
    ability->SendFunctionKey(fuzzedInt32);
}

void TestMoveCursor(int32_t fuzzedInt32)
{
    sptr<InputMethodAbility> ability = InputMethodAbility::GetInstance();
    ability->MoveCursor(fuzzedInt32);
}

void TestDispatchKeyEvent(int32_t fuzzedInt32)
{
    sptr<InputMethodAbility> ability = InputMethodAbility::GetInstance();
    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
    keyEvent->SetKeyCode(fuzzedInt32);
    keyEvent->SetKeyAction(fuzzedInt32);
    sptr<KeyEventConsumerProxy> consumer = new (std::nothrow) KeyEventConsumerProxy(nullptr);
    ability->DispatchKeyEvent(keyEvent, consumer);
}

void TestSetCallingWindow(int32_t fuzzedInt32)
{
    sptr<InputMethodAbility> ability = InputMethodAbility::GetInstance();
    ability->SetCallingWindow(fuzzedInt32);
}

void TestGetEnterKeyType()
{
    sptr<InputMethodAbility> ability = InputMethodAbility::GetInstance();
    int32_t keyType;
    ability->GetEnterKeyType(keyType);
}

void TestGetInputPattern()
{
    sptr<InputMethodAbility> ability = InputMethodAbility::GetInstance();
    int32_t inputPattern;
    ability->GetInputPattern(inputPattern);
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    std::string fuzzedString(reinterpret_cast<const char *>(data), size);
    auto fuzzedInt32 = static_cast<int32_t>(size);

    OHOS::TestInsertText(fuzzedString);

    OHOS::TestSetImeListener();

    OHOS::TestSetKdListener();

    OHOS::TestDeleteForward(fuzzedInt32);
    OHOS::TestDeleteBackward(fuzzedInt32);
    OHOS::TestSendExtendAction(fuzzedInt32);

    OHOS::TestHideKeyboardSelf();

    OHOS::TestGetTextBeforeCursor(fuzzedInt32);
    OHOS::TestGetTextAfterCursor(fuzzedInt32);

    OHOS::TestSendFunctionKey(fuzzedInt32);
    OHOS::TestMoveCursor(fuzzedInt32);

    OHOS::TestDispatchKeyEvent(fuzzedInt32);

    OHOS::TestSetCallingWindow(fuzzedInt32);

    OHOS::TestGetEnterKeyType();
    OHOS::TestGetInputPattern();
    return 0;
}