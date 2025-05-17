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

#define private public
#define protected public
#include "input_method_ability.h"
#undef private

#include "input_client_service_impl.h"
#include "input_method_engine_listener_impl.h"

using namespace OHOS::MiscServices;
namespace OHOS {
constexpr int32_t MAIN_USER_ID = 100;
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

bool InitializeClientInfo(InputClientInfo &clientInfo)
{
    sptr<IInputClient> clientStub = new (std::nothrow) InputClientServiceImpl();
    if (clientStub == nullptr) {
        IMSA_HILOGE("failed to create client");
        return false;
    }
    sptr<InputDeathRecipient> deathRecipient = new (std::nothrow) InputDeathRecipient();
    if (deathRecipient == nullptr) {
        IMSA_HILOGE("failed to new deathRecipient");
        return false;
    }
    clientInfo = { .userID = MAIN_USER_ID, .client = clientStub, .deathRecipient = deathRecipient };
    return true;
}

void TestInsertText(const std::string &fuzzedString)
{
    InputMethodAbility::GetInstance().InsertText(std::move(fuzzedString));
}

void TestSetImeListener()
{
    auto engineListener = std::make_shared<InputMethodEngineListenerImpl>();
    InputMethodAbility::GetInstance().SetImeListener(engineListener);
}

void TestSetKdListener()
{
    InputMethodAbility::GetInstance().SetKdListener(nullptr);
}

void TestDeleteForward(int32_t fuzzedInt32)
{
    InputMethodAbility::GetInstance().DeleteForward(fuzzedInt32);
}

void TestDeleteBackward(int32_t fuzzedInt32)
{
    InputMethodAbility::GetInstance().DeleteBackward(fuzzedInt32);
}

void TestSendExtendAction(int32_t fuzzedInt32)
{
    InputMethodAbility::GetInstance().SendExtendAction(fuzzedInt32);
}

void TestHideKeyboardSelf()
{
    InputMethodAbility::GetInstance().HideKeyboardSelf();
}

void TestGetTextBeforeCursor(int32_t fuzzedInt32)
{
    std::u16string text;
    InputMethodAbility::GetInstance().GetTextBeforeCursor(fuzzedInt32, text);
}

void TestGetTextAfterCursor(int32_t fuzzedInt32)
{
    std::u16string text;
    InputMethodAbility::GetInstance().GetTextAfterCursor(fuzzedInt32, text);
}

void TestSendFunctionKey(int32_t fuzzedInt32)
{
    InputMethodAbility::GetInstance().SendFunctionKey(fuzzedInt32);
}

void TestMoveCursor(int32_t fuzzedInt32)
{
    InputMethodAbility::GetInstance().MoveCursor(fuzzedInt32);
}

void TestDispatchKeyEvent(int32_t fuzzedInt32)
{
    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
    keyEvent->SetKeyCode(fuzzedInt32);
    keyEvent->SetKeyAction(fuzzedInt32);
    sptr<KeyEventConsumerProxy> consumer = new (std::nothrow) KeyEventConsumerProxy(nullptr);
    InputMethodAbility::GetInstance().DispatchKeyEvent(keyEvent, consumer);
}

void TestSetCallingWindow(int32_t fuzzedInt32)
{
    InputMethodAbility::GetInstance().SetCallingWindow(fuzzedInt32);
}

void TestGetEnterKeyType()
{
    int32_t keyType;
    InputMethodAbility::GetInstance().GetEnterKeyType(keyType);
}

void TestGetInputPattern()
{
    int32_t inputPattern;
    InputMethodAbility::GetInstance().GetInputPattern(inputPattern);
}

void TestCallingDisplayIdChanged(uint64_t fuzzedUint64)
{
    InputMethodAbility::GetInstance().OnCallingDisplayIdChanged(fuzzedUint64);
}

void TestRegisterProxyIme(uint64_t fuzzedUint64)
{
    InputMethodAbility::GetInstance().RegisterProxyIme(fuzzedUint64);
}

void TestUnregisterProxyIme(uint64_t fuzzedUint64)
{
    InputMethodAbility::GetInstance().UnregisterProxyIme(fuzzedUint64);
}

void TestStartInput(const InputClientInfo &clientInfo, bool isBindFromClient)
{
    InputMethodAbility::GetInstance().StartInput(clientInfo, isBindFromClient);
}

void TestIsDisplayChanged(uint64_t oldDisplayId, uint64_t newDisplayId)
{
    InputMethodAbility::GetInstance().IsDisplayChanged(oldDisplayId, newDisplayId);
}

void TestOnSelectionChange(std::u16string text, int32_t oldBegin, int32_t oldEnd,
    int32_t newBegin, int32_t newEnd)
{
    InputMethodAbility::GetInstance().OnSelectionChange(text, oldBegin, oldEnd, newBegin, newEnd);
}

void TestOperationKeyboard(int32_t cmdId, uint32_t sessionId)
{
    InputMethodAbility::GetInstance().HideKeyboardImplWithoutLock(cmdId, sessionId);
    InputMethodAbility::GetInstance().ShowKeyboardImplWithLock(cmdId);
}
void TestInterfaceCoverage(int32_t dataInt32, bool dataBool, std::u16string &text, int64_t consumeTime)
{
    InputMethodAbility::GetInstance().SelectByMovement(dataInt32);
    InputMethodAbility::GetInstance().GetEnterKeyType(dataInt32);
    InputMethodAbility::GetInstance().GetSecurityMode(dataInt32);
    InputMethodAbility::GetInstance().FinishTextPreview(dataBool);
    InputMethodAbility::GetInstance().GetTextBeforeCursor(dataInt32, text);
    InputMethodAbility::GetInstance().ReportBaseTextOperation(dataInt32, dataInt32, consumeTime);
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    std::string fuzzedString(reinterpret_cast<const char *>(data), size);
    auto fuzzedInt32 = static_cast<int32_t>(size);
    auto fuzzedUint64 = static_cast<uint64_t>(size);
    auto fuzzedInt64 = static_cast<int64_t>(size);
    InputClientInfo clientInfo;
    if (!OHOS::InitializeClientInfo(clientInfo)) {
        return false;
    }
    auto fuzzedBool = static_cast<bool>(data[0] % 2);

    uint64_t dataValue;
    memcpy(&dataValue, data, sizeof(uint64_t));

    int32_t int32Value;
    memcpy(&int32Value, data, sizeof(int32_t));

    std::u16string fuzzedU16String = u"insert text";

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
    OHOS::TestCallingDisplayIdChanged(fuzzedUint64);
    OHOS::TestRegisterProxyIme(fuzzedUint64);
    OHOS::TestUnregisterProxyIme(fuzzedUint64);
    OHOS::TestStartInput(clientInfo, fuzzedBool);
    OHOS::TestIsDisplayChanged(dataValue, fuzzedUint64);
    OHOS::TestOnSelectionChange(fuzzedU16String, fuzzedInt32, fuzzedInt32, int32Value, int32Value);
    OHOS::TestOperationKeyboard(fuzzedInt32, int32Value);
    OHOS::TestInterfaceCoverage(fuzzedInt32, fuzzedBool, fuzzedU16String, fuzzedInt64);
    return 0;
}