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

#include "native_text_changed_listener.h"

#include <gtest/gtest.h>

using namespace testing::ext;

namespace OHOS {
namespace MiscServices {
class NativeTextChangedListenerTest : public testing::Test { };

static void TestNativeTextChangedListener(NativeTextChangedListener *listener)
{
    listener->InsertText(u"");
    listener->DeleteForward(0);
    listener->DeleteBackward(0);
    listener->SendKeyboardStatus(KeyboardStatus::HIDE);
    FunctionKey functionKey;
    listener->SendFunctionKey(functionKey);
    listener->MoveCursor(OHOS::MiscServices::Direction::LEFT);
    listener->HandleSetSelection(0, 0);
    listener->HandleExtendAction(0);
    EXPECT_EQ(u"", listener->GetLeftTextOfCursor(0));
    EXPECT_EQ(u"", listener->GetLeftTextOfCursor(1));
    EXPECT_EQ(u"", listener->GetRightTextOfCursor(0));
    EXPECT_EQ(u"", listener->GetRightTextOfCursor(1));
    EXPECT_EQ(0, listener->GetTextIndexAtCursor());
    listener->FinishTextPreview();
}

/**
 * @tc.name: NativeTextChangedListener_001
 * @tc.desc: Test textEditor_ is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(NativeTextChangedListenerTest, NativeTextChangedListener_001, TestSize.Level0)
{
    NativeTextChangedListener listener(nullptr);
    TestNativeTextChangedListener(&listener);
    std::unordered_map<std::string, PrivateDataValue> privateCommand;
    EXPECT_EQ(ErrorCode::ERROR_NULL_POINTER, listener.ReceivePrivateCommand(privateCommand));
    EXPECT_EQ(ErrorCode::ERROR_NULL_POINTER, listener.SetPreviewText(u"", Range({ 0, 0 })));
}

/**
 * @tc.name: CallbackIsNullTest_001
 * @tc.desc: Test callback is null.
 * @tc.type: FUNC
 */
HWTEST_F(NativeTextChangedListenerTest, CallbackIsNullTest_001, TestSize.Level0)
{
    auto textEditor = OH_TextEditorProxy_Create();
    EXPECT_NE(nullptr, textEditor);
    NativeTextChangedListener listener(textEditor);
    TestNativeTextChangedListener(&listener);
    std::unordered_map<std::string, PrivateDataValue> privateCommand;
    EXPECT_EQ(ErrorCode::ERROR_NULL_POINTER, listener.ReceivePrivateCommand(privateCommand));
    EXPECT_EQ(ErrorCode::ERROR_NULL_POINTER, listener.SetPreviewText(u"", Range({ 0, 0 })));
    OH_TextEditorProxy_Destroy(textEditor);
}

void GetTextConfig(InputMethod_TextEditorProxy *proxy, InputMethod_TextConfig *config) { }
void InsertText(InputMethod_TextEditorProxy *proxy, const char16_t *text, size_t length) { }
void DeleteForward(InputMethod_TextEditorProxy *proxy, int32_t length) { }
void DeleteBackward(InputMethod_TextEditorProxy *proxy, int32_t length) { }
void SendKeyboardStatus(InputMethod_TextEditorProxy *proxy, InputMethod_KeyboardStatus status) { }
void SendEnterKey(InputMethod_TextEditorProxy *proxy, InputMethod_EnterKeyType type) { }
void MoveCursor(InputMethod_TextEditorProxy *proxy, InputMethod_Direction direction) { }
void HandleSetSelection(InputMethod_TextEditorProxy *proxy, int32_t start, int32_t end) { }
void HandleExtendAction(InputMethod_TextEditorProxy *proxy, InputMethod_ExtendAction action) { }
void GetleftTextOfCursor(InputMethod_TextEditorProxy *proxy, int32_t number, char16_t text[], size_t *length)
{
    *length = 0;
}
void GetRightTextOfCursor(InputMethod_TextEditorProxy *proxy, int32_t number, char16_t text[], size_t *length)
{
    *length = 0;
}
int32_t GetTextIndexAtCursor(InputMethod_TextEditorProxy *proxy)
{
    return 0;
}
int32_t ReceivePrivateCommand(
    InputMethod_TextEditorProxy *proxy, InputMethod_PrivateCommand *privateCommand[], size_t size)
{
    return 0;
}
int32_t SetPreviewText(
    InputMethod_TextEditorProxy *proxy, const char16_t *text, size_t length, int32_t start, int32_t end)
{
    return 0;
}
void FinishTextPreview(InputMethod_TextEditorProxy *proxy) { }
static void ConstructTextEditorProxy(InputMethod_TextEditorProxy *textEditorProxy)
{
    EXPECT_EQ(IME_ERR_OK, OH_TextEditorProxy_SetGetTextConfigFunc(textEditorProxy, GetTextConfig));
    EXPECT_EQ(IME_ERR_OK, OH_TextEditorProxy_SetInsertTextFunc(textEditorProxy, InsertText));
    EXPECT_EQ(IME_ERR_OK, OH_TextEditorProxy_SetDeleteForwardFunc(textEditorProxy, DeleteForward));
    EXPECT_EQ(IME_ERR_OK, OH_TextEditorProxy_SetDeleteBackwardFunc(textEditorProxy, DeleteBackward));
    EXPECT_EQ(IME_ERR_OK, OH_TextEditorProxy_SetSendKeyboardStatusFunc(textEditorProxy, SendKeyboardStatus));
    EXPECT_EQ(IME_ERR_OK, OH_TextEditorProxy_SetSendEnterKeyFunc(textEditorProxy, SendEnterKey));
    EXPECT_EQ(IME_ERR_OK, OH_TextEditorProxy_SetMoveCursorFunc(textEditorProxy, MoveCursor));
    EXPECT_EQ(IME_ERR_OK, OH_TextEditorProxy_SetHandleSetSelectionFunc(textEditorProxy, HandleSetSelection));
    EXPECT_EQ(IME_ERR_OK, OH_TextEditorProxy_SetHandleExtendActionFunc(textEditorProxy, HandleExtendAction));
    EXPECT_EQ(IME_ERR_OK, OH_TextEditorProxy_SetGetLeftTextOfCursorFunc(textEditorProxy, GetleftTextOfCursor));
    EXPECT_EQ(IME_ERR_OK, OH_TextEditorProxy_SetGetRightTextOfCursorFunc(textEditorProxy, GetRightTextOfCursor));
    EXPECT_EQ(IME_ERR_OK, OH_TextEditorProxy_SetGetTextIndexAtCursorFunc(textEditorProxy, GetTextIndexAtCursor));
    EXPECT_EQ(IME_ERR_OK, OH_TextEditorProxy_SetReceivePrivateCommandFunc(textEditorProxy, ReceivePrivateCommand));
    EXPECT_EQ(IME_ERR_OK, OH_TextEditorProxy_SetSetPreviewTextFunc(textEditorProxy, SetPreviewText));
    EXPECT_EQ(IME_ERR_OK, OH_TextEditorProxy_SetFinishTextPreviewFunc(textEditorProxy, FinishTextPreview));
}

/**
 * @tc.name: CallbackFinishTest_001
 * @tc.desc: Test callback finish.
 * @tc.type: FUNC
 */
HWTEST_F(NativeTextChangedListenerTest, CallbackFinishTest_001, TestSize.Level0)
{
    auto textEditor = OH_TextEditorProxy_Create();
    EXPECT_NE(nullptr, textEditor);
    ConstructTextEditorProxy(textEditor);
    NativeTextChangedListener listener(textEditor);
    TestNativeTextChangedListener(&listener);
    std::unordered_map<std::string, PrivateDataValue> privateCommand;
    EXPECT_EQ(0, listener.ReceivePrivateCommand(privateCommand));
    EXPECT_EQ(0, listener.SetPreviewText(u"", Range({ 0, 0 })));
    OH_TextEditorProxy_Destroy(textEditor);
}

/**
 * @tc.name: ConvertKeyBoardStarusTest_001
 * @tc.desc: Test ConvertKeyBoardStarus.
 * @tc.type: FUNC
 */
HWTEST_F(NativeTextChangedListenerTest, ConvertKeyBoardStarusTest_001, TestSize.Level0)
{
    auto textEditor = OH_TextEditorProxy_Create();
    EXPECT_NE(nullptr, textEditor);
    ConstructTextEditorProxy(textEditor);
    NativeTextChangedListener listener(textEditor);
    listener.SendKeyboardStatus(KeyboardStatus::NONE);
    listener.SendKeyboardStatus(KeyboardStatus::HIDE);
    listener.SendKeyboardStatus(KeyboardStatus::SHOW);
}
/**
 * @tc.name: ConvertToCEnterKeyTypeTest_001
 * @tc.desc: Test ConvertToCEnterKeyType.
 * @tc.type: FUNC
 */
HWTEST_F(NativeTextChangedListenerTest, ConvertToCEnterKeyTypeTest_001, TestSize.Level0)
{
    auto textEditor = OH_TextEditorProxy_Create();
    EXPECT_NE(nullptr, textEditor);
    ConstructTextEditorProxy(textEditor);
    NativeTextChangedListener listener(textEditor);
    FunctionKey key;
    key.SetEnterKeyType(EnterKeyType::UNSPECIFIED);
    listener.SendFunctionKey(key);
    key.SetEnterKeyType(EnterKeyType::NONE);
    listener.SendFunctionKey(key);
    key.SetEnterKeyType(EnterKeyType::GO);
    listener.SendFunctionKey(key);
    key.SetEnterKeyType(EnterKeyType::SEARCH);
    listener.SendFunctionKey(key);
    key.SetEnterKeyType(EnterKeyType::SEND);
    listener.SendFunctionKey(key);
    key.SetEnterKeyType(EnterKeyType::NEXT);
    listener.SendFunctionKey(key);
    key.SetEnterKeyType(EnterKeyType::DONE);
    listener.SendFunctionKey(key);
    key.SetEnterKeyType(EnterKeyType::PREVIOUS);
    listener.SendFunctionKey(key);
    key.SetEnterKeyType(EnterKeyType::NEW_LINE);
    listener.SendFunctionKey(key);
}

/**
 * @tc.name: ConvertToCDirectionTest_001
 * @tc.desc: Test ConvertToCDirection.
 * @tc.type: FUNC
 */
HWTEST_F(NativeTextChangedListenerTest, ConvertToCDirectionTest_001, TestSize.Level1)
{
    auto textEditor = OH_TextEditorProxy_Create();
    EXPECT_NE(nullptr, textEditor);
    ConstructTextEditorProxy(textEditor);
    NativeTextChangedListener listener(textEditor);
    listener.MoveCursor(OHOS::MiscServices::Direction::NONE);
    listener.MoveCursor(OHOS::MiscServices::Direction::UP);
    listener.MoveCursor(OHOS::MiscServices::Direction::DOWN);
    listener.MoveCursor(OHOS::MiscServices::Direction::RIGHT);
    listener.MoveCursor(OHOS::MiscServices::Direction::LEFT);
}
/**
 * @tc.name: ConvertToCExtendActionTest_001
 * @tc.desc: Test ConvertToCExtendAction.
 * @tc.type: FUNC
 */
HWTEST_F(NativeTextChangedListenerTest, ConvertToCExtendActionTest_001, TestSize.Level1)
{
    auto textEditor = OH_TextEditorProxy_Create();
    EXPECT_NE(nullptr, textEditor);
    ConstructTextEditorProxy(textEditor);
    NativeTextChangedListener listener(textEditor);
    listener.HandleExtendAction(static_cast<int32_t>(OHOS::MiscServices::ExtendAction::SELECT_ALL));
    listener.HandleExtendAction(static_cast<int32_t>(OHOS::MiscServices::ExtendAction::COPY));
    listener.HandleExtendAction(static_cast<int32_t>(OHOS::MiscServices::ExtendAction::CUT));
    listener.HandleExtendAction(static_cast<int32_t>(OHOS::MiscServices::ExtendAction::PASTE));
}
} // namespace MiscServices
} // namespace OHOS