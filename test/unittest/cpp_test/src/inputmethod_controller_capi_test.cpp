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
#include "inputmethod_controller_capi.h"
#include <gtest/gtest.h>

using namespace testing::ext;
class InputMethodControllerCapiTest : public testing::Test { };

/**
 * @tc.name: TestCursorInfo_001
 * @tc.desc: create and destroy TestCursorInfo success
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, TestCursorInfo_001, TestSize.Level0)
{
    double expLeft = 1.1;
    double expTop = 2.2;
    double expWidth = 3.3;
    double expHeight = 4.4;
    auto cursorInfo = OH_CursorInfo_New(expLeft, expTop, expWidth, expHeight);
    ASSERT_NE(nullptr, cursorInfo);

    double actLeft = 0;
    double actTop = 0;
    double actWidth = 0;
    double actHeight = 0;
    EXPECT_EQ(IME_ERR_OK, OH_CursorInfo_GetRect(cursorInfo, &actLeft, &actTop, &actWidth, &actHeight));
    EXPECT_EQ(expLeft, actLeft);
    EXPECT_EQ(expTop, actTop);
    EXPECT_EQ(expWidth, actWidth);
    EXPECT_EQ(expHeight, actHeight);

    // test set rect
    expLeft = 1;
    expTop = 2;
    expWidth = 3;
    expHeight = 4;
    EXPECT_EQ(IME_ERR_OK, OH_CursorInfo_SetRect(cursorInfo, expLeft, expTop, expWidth, expHeight));
    EXPECT_EQ(IME_ERR_OK, OH_CursorInfo_GetRect(cursorInfo, &actLeft, &actTop, &actWidth, &actHeight));
    EXPECT_EQ(expLeft, actLeft);
    EXPECT_EQ(expTop, actTop);
    EXPECT_EQ(expWidth, actWidth);
    EXPECT_EQ(expHeight, actHeight);

    OH_CursorInfo_Delete(cursorInfo);
}

static void TestCursorInfoOfTextConfig(InputMethod_TextConfig *config)
{
    InputMethod_CursorInfo *cursorInfo = nullptr;
    EXPECT_EQ(IME_ERR_OK, OH_TextConfig_GetCursorInfo(config, &cursorInfo));
    // test set and get cursorInfo rect
    double expLeft = 1.1;
    double expTop = 2.2;
    double expWidth = 3.3;
    double expHeight = 4.4;
    EXPECT_EQ(IME_ERR_OK, OH_CursorInfo_SetRect(cursorInfo, expLeft, expTop, expWidth, expHeight));
    double actLeft = 0.0;
    double actTop = 0.0;
    double actWidth = 0.0;
    double actHeight = 0.0;
    EXPECT_EQ(IME_ERR_OK, OH_CursorInfo_GetRect(cursorInfo, &actLeft, &actTop, &actWidth, &actHeight));
    EXPECT_EQ(expLeft, actLeft);
    EXPECT_EQ(expTop, actTop);
    EXPECT_EQ(expWidth, actWidth);
    EXPECT_EQ(expHeight, actHeight);
}

/**
 * @tc.name: TestTextConfig_001
 * @tc.desc: create and destroy TestTextConfig success
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, TestTextConfig_001, TestSize.Level0)
{
    auto config = OH_TextConfig_New();
    ASSERT_NE(nullptr, config);

    // test set and get inputType
    InputMethod_TextInputType expInputType = IME_TEXT_INPUT_TYPE_NUMBER_DECIMAL;
    InputMethod_TextInputType actInputType = IME_TEXT_INPUT_TYPE_NONE;
    EXPECT_EQ(IME_ERR_OK, OH_TextConfig_SetInputType(config, expInputType));
    EXPECT_EQ(IME_ERR_OK, OH_TextConfig_GetInputType(config, &actInputType));
    EXPECT_EQ(expInputType, actInputType);

    // test set and get enterKeyType
    InputMethod_EnterKeyType expEnterKeyType = IME_ENTER_KEY_SEARCH;
    EXPECT_EQ(IME_ERR_OK, OH_TextConfig_SetEnterKeyType(config, expEnterKeyType));
    InputMethod_EnterKeyType actEnterKeyType = IME_ENTER_KEY_UNSPECIFIED;
    EXPECT_EQ(IME_ERR_OK, OH_TextConfig_GetEnterKeyType(config, &actEnterKeyType));
    EXPECT_EQ(expEnterKeyType, actEnterKeyType);

    // test set and get isPreviewTextSupported
    EXPECT_EQ(IME_ERR_OK, OH_TextConfig_SetIsPreviewTextSupported(config, true));
    bool isPreviewTextSupported = false;
    EXPECT_EQ(IME_ERR_OK, OH_TextConfig_IsPreviewTextSupported(config, &isPreviewTextSupported));
    EXPECT_TRUE(isPreviewTextSupported);

    // test set and get selection
    int32_t expStart = 1;
    int32_t expEnd = 2;
    EXPECT_EQ(IME_ERR_OK, OH_TextConfig_SetSelection(config, expStart, expEnd));
    int32_t actStart = 0;
    int32_t actEnd = 0;
    EXPECT_EQ(IME_ERR_OK, OH_TextConfig_GetSelection(config, &actStart, &actEnd));
    EXPECT_EQ(expStart, actStart);
    EXPECT_EQ(expEnd, actEnd);

    // test set and get windowId
    int32_t expWindowId = 1;
    EXPECT_EQ(IME_ERR_OK, OH_TextConfig_SetWindowId(config, expWindowId));
    int32_t actWindowId = 0;
    EXPECT_EQ(IME_ERR_OK, OH_TextConfig_GetWindowId(config, &actWindowId));
    EXPECT_EQ(expWindowId, actWindowId);

    TestCursorInfoOfTextConfig(config);

    InputMethod_TextAvoidInfo *textAvoidInfo = nullptr;
    EXPECT_EQ(IME_ERR_OK, OH_TextConfig_GetTextAvoidInfo(config, &textAvoidInfo));

    // test set and get text avoid info member
    double expPositionY = 10.0;
    double expHeight = 20.0;
    EXPECT_EQ(IME_ERR_OK, OH_TextAvoidInfo_SetPositionY(textAvoidInfo, expPositionY));
    EXPECT_EQ(IME_ERR_OK, OH_TextAvoidInfo_SetHeight(textAvoidInfo, expHeight));
    double actPositionY = 0.0;
    double actHeight = 0.0;
    EXPECT_EQ(IME_ERR_OK, OH_TextAvoidInfo_GetPositionY(textAvoidInfo, &actPositionY));
    EXPECT_EQ(IME_ERR_OK, OH_TextAvoidInfo_GetHeight(textAvoidInfo, &actHeight));
    EXPECT_EQ(expPositionY, actPositionY);
    EXPECT_EQ(expHeight, actHeight);

    OH_TextConfig_Delete(config);
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
static void ConstructTextEditorProxy(InputMethod_TextEditorProxy *textEditorProxy)
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

static void TestGetTextEditorProxyMember(InputMethod_TextEditorProxy *textEditorProxy)
{
    OH_TextEditorProxy_GetTextConfigFunc getTextCOnfigFunc = nullptr;
    EXPECT_EQ(IME_ERR_OK, OH_TextEditorProxy_GetGetTextConfigFunc(textEditorProxy, &getTextCOnfigFunc));
    EXPECT_EQ(GetTextConfigFunc, getTextCOnfigFunc);

    OH_TextEditorProxy_InsertTextFunc insertTextFunc = nullptr;
    EXPECT_EQ(IME_ERR_OK, OH_TextEditorProxy_GetInsertTextFunc(textEditorProxy, &insertTextFunc));
    EXPECT_EQ(InsertTextFunc, insertTextFunc);

    OH_TextEditorProxy_DeleteForwardFunc deleteForwardFunc = nullptr;
    EXPECT_EQ(IME_ERR_OK, OH_TextEditorProxy_GetDeleteForwardFunc(textEditorProxy, &deleteForwardFunc));
    EXPECT_EQ(DeleteForwardFunc, deleteForwardFunc);

    OH_TextEditorProxy_DeleteBackwardFunc deleteBackwardFunc = nullptr;
    EXPECT_EQ(IME_ERR_OK, OH_TextEditorProxy_GetDeleteBackwardFunc(textEditorProxy, &deleteBackwardFunc));
    EXPECT_EQ(DeleteBackwardFunc, deleteBackwardFunc);

    OH_TextEditorProxy_SendKeyboardStatusFunc sendKeyboardStatusFunc = nullptr;
    EXPECT_EQ(IME_ERR_OK, OH_TextEditorProxy_GetSendKeyboardStatusFunc(textEditorProxy, &sendKeyboardStatusFunc));
    EXPECT_EQ(SendKeyboardStatusFunc, sendKeyboardStatusFunc);

    OH_TextEditorProxy_SendEnterKeyFunc sendEnterKeyFunc = nullptr;
    EXPECT_EQ(IME_ERR_OK, OH_TextEditorProxy_GetSendEnterKeyFunc(textEditorProxy, &sendEnterKeyFunc));
    EXPECT_EQ(SendEnterKeyFunc, sendEnterKeyFunc);

    OH_TextEditorProxy_MoveCursorFunc moveCursorFunc = nullptr;
    EXPECT_EQ(IME_ERR_OK, OH_TextEditorProxy_GetMoveCursorFunc(textEditorProxy, &moveCursorFunc));
    EXPECT_EQ(MoveCursorFunc, moveCursorFunc);

    OH_TextEditorProxy_HandleSetSelectionFunc handleSetSelectionFunc = nullptr;
    EXPECT_EQ(IME_ERR_OK, OH_TextEditorProxy_GetHandleSetSelectionFunc(textEditorProxy, &handleSetSelectionFunc));
    EXPECT_EQ(HandleSetSelectionFunc, handleSetSelectionFunc);

    OH_TextEditorProxy_HandleExtendActionFunc handleExtendActionFunc = nullptr;
    EXPECT_EQ(IME_ERR_OK, OH_TextEditorProxy_GetHandleExtendActionFunc(textEditorProxy, &handleExtendActionFunc));
    EXPECT_EQ(HandleExtendActionFunc, handleExtendActionFunc);

    OH_TextEditorProxy_GetLeftTextOfCursorFunc getLeftTextOfCursorFunc = nullptr;
    EXPECT_EQ(IME_ERR_OK, OH_TextEditorProxy_GetGetLeftTextOfCursorFunc(textEditorProxy, &getLeftTextOfCursorFunc));
    EXPECT_EQ(GetleftTextOfCursorFunc, getLeftTextOfCursorFunc);

    OH_TextEditorProxy_GetRightTextOfCursorFunc getRightTextOfCursorFunc = nullptr;
    EXPECT_EQ(IME_ERR_OK, OH_TextEditorProxy_GetGetRightTextOfCursorFunc(textEditorProxy, &getRightTextOfCursorFunc));
    EXPECT_EQ(GetRightTextOfCursorFunc, getRightTextOfCursorFunc);

    OH_TextEditorProxy_GetTextIndexAtCursorFunc getTextIndexAtCursorFunc = nullptr;
    EXPECT_EQ(IME_ERR_OK, OH_TextEditorProxy_GetGetTextIndexAtCursorFunc(textEditorProxy, &getTextIndexAtCursorFunc));
    EXPECT_EQ(GetTextIndexAtCursorFunc, getTextIndexAtCursorFunc);

    OH_TextEditorProxy_ReceivePrivateCommandFunc receivePrivateCommandFunc = nullptr;
    EXPECT_EQ(IME_ERR_OK, OH_TextEditorProxy_GetReceivePrivateCommandFunc(textEditorProxy, &receivePrivateCommandFunc));
    EXPECT_EQ(ReceivePrivateCommandFunc, receivePrivateCommandFunc);

    OH_TextEditorProxy_SetPreviewTextFunc setPreviewTextFunc = nullptr;
    EXPECT_EQ(IME_ERR_OK, OH_TextEditorProxy_GetSetPreviewTextFunc(textEditorProxy, &setPreviewTextFunc));
    EXPECT_EQ(SetPreviewTextFunc, setPreviewTextFunc);

    OH_TextEditorProxy_FinishTextPreviewFunc finishTextPreviewFunc = nullptr;
    EXPECT_EQ(IME_ERR_OK, OH_TextEditorProxy_GetFinishTextPreviewFunc(textEditorProxy, &finishTextPreviewFunc));
    EXPECT_EQ(FinishTextPreviewFunc, finishTextPreviewFunc);
}

/**
 * @tc.name: TextEditorProxy_001
 * @tc.desc: create and destroy TextEditorProxy success
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, TextEditorProxy_001, TestSize.Level0)
{
    auto textEditorProxy = OH_TextEditorProxy_New();
    ASSERT_NE(nullptr, textEditorProxy);
    ConstructTextEditorProxy(textEditorProxy);
    TestGetTextEditorProxyMember(textEditorProxy);
    OH_TextEditorProxy_Delete(textEditorProxy);
}

/**
 * @tc.name: AttachOptions_001
 * @tc.desc: create and destroy AttachOptions success
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, AttachOptions_001, TestSize.Level0)
{
    auto options = OH_AttachOptions_New(true);
    ASSERT_NE(nullptr, options);

    bool showKeyboard = false;
    EXPECT_EQ(IME_ERR_OK, OH_AttachOptions_IsShowKeyboard(options, &showKeyboard));
    EXPECT_TRUE(showKeyboard);
    OH_AttachOptions_Delete(options);
}

/**
 * @tc.name: TextAvoidInfo_001
 * @tc.desc: create and destroy TextAvoidInfo success
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, TextAvoidInfo_001, TestSize.Level0)
{
    double expPositionY = 1.1;
    double expHeight = 1.2;
    auto avoidInfo = OH_TextAvoidInfo_New(expPositionY, expHeight);
    ASSERT_NE(nullptr, avoidInfo);

    double actPositionY = 0.0;
    double actHeight = 0.0;
    EXPECT_EQ(IME_ERR_OK, OH_TextAvoidInfo_GetPositionY(avoidInfo, &actPositionY));
    EXPECT_EQ(IME_ERR_OK, OH_TextAvoidInfo_GetHeight(avoidInfo, &actHeight));
    EXPECT_EQ(expPositionY, actPositionY);
    EXPECT_EQ(expHeight, actHeight);

    // test set positionY and height
    expPositionY = 2.1;
    expHeight = 2.2;
    EXPECT_EQ(IME_ERR_OK, OH_TextAvoidInfo_SetPositionY(avoidInfo, expPositionY));
    EXPECT_EQ(IME_ERR_OK, OH_TextAvoidInfo_SetHeight(avoidInfo, expHeight));
    EXPECT_EQ(IME_ERR_OK, OH_TextAvoidInfo_GetPositionY(avoidInfo, &actPositionY));
    EXPECT_EQ(IME_ERR_OK, OH_TextAvoidInfo_GetHeight(avoidInfo, &actHeight));
    EXPECT_EQ(expPositionY, actPositionY);
    EXPECT_EQ(expHeight, actHeight);

    OH_TextAvoidInfo_Delete(avoidInfo);
}

/**
 * @tc.name: PrivateCommand_001
 * @tc.desc: create and destroy PrivateCommand success
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, PrivateCommand_001, TestSize.Level0)
{
    std::string key = "key";
    auto privateCommand = OH_PrivateCommand_New(const_cast<char *>(key.c_str()), key.length());
    ASSERT_NE(nullptr, privateCommand);

    // test set bool value
    EXPECT_EQ(IME_ERR_OK, OH_PrivateCommand_SetBoolValue(privateCommand, true));
    bool actBoolValue = false;
    EXPECT_EQ(IME_ERR_OK, OH_PrivateCommand_GetBoolValue(privateCommand, &actBoolValue));
    EXPECT_TRUE(actBoolValue);

    // test set int value
    int32_t expIntValue = 1;
    EXPECT_EQ(IME_ERR_OK, OH_PrivateCommand_SetIntValue(privateCommand, expIntValue));
    int32_t actIntValue = 0;
    EXPECT_EQ(IME_ERR_OK, OH_PrivateCommand_GetIntValue(privateCommand, &actIntValue));
    EXPECT_EQ(expIntValue, actIntValue);

    // test set string value
    std::string expStrValue = "string";
    EXPECT_EQ(IME_ERR_OK,
        OH_PrivateCommand_SetStrValue(privateCommand, const_cast<char *>(expStrValue.c_str()), expStrValue.length()));
    const char *actStrValue = nullptr;
    size_t actStrValueLength = 0;
    EXPECT_EQ(IME_ERR_OK, OH_PrivateCommand_GetStrValue(privateCommand, &actStrValue, &actStrValueLength));
    EXPECT_EQ(expStrValue, std::string(actStrValue, actStrValueLength));

    // test get value type
    InputMethod_CommandValueType valueType = IME_COMMAND_VALUE_TYPE_NONE;
    EXPECT_EQ(IME_ERR_OK, OH_PrivateCommand_GetValueType(privateCommand, &valueType));
    EXPECT_EQ(IME_COMMAND_VALUE_TYPE_STRING, valueType);

    // test get and set key
    const char *actStrKey = nullptr;
    size_t actStrKeyLength = 0;
    EXPECT_EQ(IME_ERR_OK, OH_PrivateCommand_GetKey(privateCommand, &actStrKey, &actStrKeyLength));
    EXPECT_EQ(key, std::string(actStrKey, actStrKeyLength));
    std::string newKey = "newKey";
    EXPECT_EQ(
        IME_ERR_OK, OH_PrivateCommand_SetKey(privateCommand, const_cast<char *>(newKey.c_str()), newKey.length()));
    EXPECT_EQ(IME_ERR_OK, OH_PrivateCommand_GetKey(privateCommand, &actStrKey, &actStrKeyLength));
    EXPECT_EQ(newKey, std::string(actStrKey, actStrKeyLength));
    OH_PrivateCommand_Delete(privateCommand);
}