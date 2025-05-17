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
#include <gtest/gtest.h>

#include "string_ex.h"

#include "global.h"
#include "native_inputmethod_types.h"

using namespace testing::ext;
using namespace OHOS;
class InputMethodControllerCapiTest : public testing::Test { };
namespace {
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
    auto cursorInfo = OH_CursorInfo_Create(expLeft, expTop, expWidth, expHeight);
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

    OH_CursorInfo_Destroy(cursorInfo);
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
    auto config = OH_TextConfig_Create();
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
    EXPECT_EQ(IME_ERR_OK, OH_TextConfig_SetPreviewTextSupport(config, true));
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

    OH_TextConfig_Destroy(config);
}

/**
 * @tc.name: TestTextConfig_002
 * @tc.desc: create and destroy TestTextConfig success
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, TestTextConfig_002, TestSize.Level0)
{
    auto config = OH_TextConfig_Create();
    ASSERT_NE(nullptr, config);

    // test set and get inputType
    InputMethod_TextInputType expInputType = IME_TEXT_INPUT_TYPE_ONE_TIME_CODE;
    InputMethod_TextInputType actInputType = IME_TEXT_INPUT_TYPE_NONE;
    EXPECT_EQ(IME_ERR_OK, OH_TextConfig_SetInputType(config, expInputType));
    EXPECT_EQ(IME_ERR_OK, OH_TextConfig_GetInputType(config, &actInputType));
    EXPECT_EQ(expInputType, actInputType);
    OH_TextConfig_Destroy(config);
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
    auto textEditorProxy = OH_TextEditorProxy_Create();
    ASSERT_NE(nullptr, textEditorProxy);
    ConstructTextEditorProxy(textEditorProxy);
    TestGetTextEditorProxyMember(textEditorProxy);
    OH_TextEditorProxy_Destroy(textEditorProxy);
}

/**
 * @tc.name: AttachOptions_001
 * @tc.desc: create and destroy AttachOptions success
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, AttachOptions_001, TestSize.Level0)
{
    auto options = OH_AttachOptions_Create(true);
    ASSERT_NE(nullptr, options);

    bool showKeyboard = false;
    EXPECT_EQ(IME_ERR_OK, OH_AttachOptions_IsShowKeyboard(options, &showKeyboard));
    EXPECT_TRUE(showKeyboard);
    OH_AttachOptions_Destroy(options);
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
    auto avoidInfo = OH_TextAvoidInfo_Create(expPositionY, expHeight);
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

    OH_TextAvoidInfo_Destroy(avoidInfo);
}

/**
 * @tc.name: PrivateCommand_001
 * @tc.desc: create and destroy PrivateCommand success
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, PrivateCommand_001, TestSize.Level0)
{
    std::string key = "key";
    auto privateCommand = OH_PrivateCommand_Create(const_cast<char *>(key.c_str()), key.length());
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
    OH_PrivateCommand_Destroy(privateCommand);
}
/**
 * @tc.name: OH_CursorInfo_SetRect_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_CursorInfo_SetRect_001, TestSize.Level0)
{
    double left = 0;
    double top = 0;
    double width = 0;
    double height = 0;
    auto ret = OH_CursorInfo_SetRect(nullptr, left, top, width, height);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
}

/**
 * @tc.name: OH_CursorInfo_GetRect_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_CursorInfo_GetRect_001, TestSize.Level0)
{
    double left = 0;
    double top = 0;
    double width = 0;
    double height = 0;
    auto ret = OH_CursorInfo_GetRect(nullptr, nullptr, nullptr, nullptr, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    auto info = OH_CursorInfo_Create(left, top, width, height);
    ret = OH_CursorInfo_GetRect(info, nullptr, nullptr, nullptr, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    ret = OH_CursorInfo_GetRect(info, &left, nullptr, nullptr, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    ret = OH_CursorInfo_GetRect(info, &left, &top, nullptr, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    ret = OH_CursorInfo_GetRect(info, &left, &top, &width, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    OH_CursorInfo_Destroy(info);
}

/**
 * @tc.name: OH_TextConfig_SetInputType_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_TextConfig_SetInputType_001, TestSize.Level0)
{
    InputMethod_TextInputType inputType = IME_TEXT_INPUT_TYPE_TEXT;
    auto ret = OH_TextConfig_SetInputType(nullptr, inputType);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
}

/**
 * @tc.name: OH_TextConfig_SetEnterKeyType_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_TextConfig_SetEnterKeyType_001, TestSize.Level0)
{
    InputMethod_EnterKeyType enterKeyType = IME_ENTER_KEY_UNSPECIFIED;
    auto ret = OH_TextConfig_SetEnterKeyType(nullptr, enterKeyType);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
}

/**
 * @tc.name: OH_TextConfig_SetPreviewTextSupport_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_TextConfig_SetPreviewTextSupport_001, TestSize.Level0)
{
    bool supported = false;
    auto ret = OH_TextConfig_SetPreviewTextSupport(nullptr, supported);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
}

/**
 * @tc.name: OH_TextConfig_SetSelection_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_TextConfig_SetSelection_001, TestSize.Level0)
{
    int32_t start = 0;
    int32_t end = 0;
    auto ret = OH_TextConfig_SetSelection(nullptr, start, end);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
}

/**
 * @tc.name: OH_TextConfig_SetWindowId_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_TextConfig_SetWindowId_001, TestSize.Level0)
{
    int32_t windowId = 0;
    auto ret = OH_TextConfig_SetWindowId(nullptr, windowId);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
}

/**
 * @tc.name: OH_TextConfig_GetInputType_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_TextConfig_GetInputType_001, TestSize.Level0)
{
    auto ret = OH_TextConfig_GetInputType(nullptr, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    InputMethod_TextConfig *config = OH_TextConfig_Create();
    ASSERT_NE(config, nullptr);
    ret = OH_TextConfig_GetInputType(config, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    OH_TextConfig_Destroy(config);
}

/**
 * @tc.name: OH_TextConfig_GetEnterKeyType_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_TextConfig_GetEnterKeyType_001, TestSize.Level0)
{
    auto ret = OH_TextConfig_GetEnterKeyType(nullptr, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    InputMethod_TextConfig *config = OH_TextConfig_Create();
    ASSERT_NE(config, nullptr);
    ret = OH_TextConfig_GetEnterKeyType(config, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    OH_TextConfig_Destroy(config);
}

/**
 * @tc.name: OH_TextConfig_IsPreviewTextSupported_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_TextConfig_IsPreviewTextSupported_001, TestSize.Level0)
{
    auto ret = OH_TextConfig_IsPreviewTextSupported(nullptr, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    InputMethod_TextConfig *config = OH_TextConfig_Create();
    ASSERT_NE(config, nullptr);
    ret = OH_TextConfig_IsPreviewTextSupported(config, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    OH_TextConfig_Destroy(config);
}

/**
 * @tc.name: OH_TextConfig_GetCursorInfo_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_TextConfig_GetCursorInfo_001, TestSize.Level0)
{
    auto ret = OH_TextConfig_GetCursorInfo(nullptr, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    InputMethod_TextConfig *config = OH_TextConfig_Create();
    ASSERT_NE(config, nullptr);
    ret = OH_TextConfig_GetCursorInfo(config, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    OH_TextConfig_Destroy(config);
}

/**
 * @tc.name: OH_TextConfig_GetTextAvoidInfo_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_TextConfig_GetTextAvoidInfo_001, TestSize.Level0)
{
    auto ret = OH_TextConfig_GetTextAvoidInfo(nullptr, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    InputMethod_TextConfig *config = OH_TextConfig_Create();
    ASSERT_NE(config, nullptr);
    ret = OH_TextConfig_GetTextAvoidInfo(config, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    OH_TextConfig_Destroy(config);
}

/**
 * @tc.name: OH_TextConfig_GetSelection_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_TextConfig_GetSelection_001, TestSize.Level0)
{
    auto ret = OH_TextConfig_GetSelection(nullptr, nullptr, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    InputMethod_TextConfig *config = OH_TextConfig_Create();
    ASSERT_NE(config, nullptr);
    ret = OH_TextConfig_GetSelection(config, nullptr, nullptr);
    EXPECT_EQ(IME_ERR_NULL_POINTER, ret);
    int32_t start = 0;
    ret = OH_TextConfig_GetSelection(config, &start, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    OH_TextConfig_Destroy(config);
}

/**
 * @tc.name: OH_TextConfig_GetWindowId_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_TextConfig_GetWindowId_001, TestSize.Level0)
{
    auto ret = OH_TextConfig_GetWindowId(nullptr, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    InputMethod_TextConfig *config = OH_TextConfig_Create();
    ASSERT_NE(config, nullptr);
    ret = OH_TextConfig_GetWindowId(config, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    OH_TextConfig_Destroy(config);
}

/**
 * @tc.name: OH_TextEditorProxy_SetGetTextConfigFunc_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_TextEditorProxy_SetGetTextConfigFunc_001, TestSize.Level0)
{
    auto ret = OH_TextEditorProxy_SetGetTextConfigFunc(nullptr, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    InputMethod_TextEditorProxy *proxy = OH_TextEditorProxy_Create();
    ASSERT_NE(proxy, nullptr);
    ret = OH_TextEditorProxy_SetGetTextConfigFunc(proxy, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    OH_TextEditorProxy_Destroy(proxy);
}

/**
 * @tc.name: OH_TextEditorProxy_SetInsertTextFunc_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_TextEditorProxy_SetInsertTextFunc_001, TestSize.Level0)
{
    auto ret = OH_TextEditorProxy_SetInsertTextFunc(nullptr, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    InputMethod_TextEditorProxy *proxy = OH_TextEditorProxy_Create();
    ASSERT_NE(proxy, nullptr);
    ret = OH_TextEditorProxy_SetInsertTextFunc(proxy, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    OH_TextEditorProxy_Destroy(proxy);
}

/**
 * @tc.name: OH_TextEditorProxy_SetDeleteForwardFunc_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_TextEditorProxy_SetDeleteForwardFunc_001, TestSize.Level0)
{
    auto ret = OH_TextEditorProxy_SetDeleteForwardFunc(nullptr, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    InputMethod_TextEditorProxy *proxy = OH_TextEditorProxy_Create();
    ASSERT_NE(proxy, nullptr);
    ret = OH_TextEditorProxy_SetDeleteForwardFunc(proxy, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    OH_TextEditorProxy_Destroy(proxy);
}

/**
 * @tc.name: OH_TextEditorProxy_SetDeleteBackwardFunc_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_TextEditorProxy_SetDeleteBackwardFunc_001, TestSize.Level0)
{
    auto ret = OH_TextEditorProxy_SetDeleteBackwardFunc(nullptr, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    InputMethod_TextEditorProxy *proxy = OH_TextEditorProxy_Create();
    ASSERT_NE(proxy, nullptr);
    ret = OH_TextEditorProxy_SetDeleteBackwardFunc(proxy, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    OH_TextEditorProxy_Destroy(proxy);
}

/**
 * @tc.name: OH_TextEditorProxy_SetSendKeyboardStatusFunc_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_TextEditorProxy_SetSendKeyboardStatusFunc_001, TestSize.Level0)
{
    auto ret = OH_TextEditorProxy_SetSendKeyboardStatusFunc(nullptr, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    InputMethod_TextEditorProxy *proxy = OH_TextEditorProxy_Create();
    ASSERT_NE(proxy, nullptr);
    ret = OH_TextEditorProxy_SetSendKeyboardStatusFunc(proxy, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    OH_TextEditorProxy_Destroy(proxy);
}

/**
 * @tc.name: OH_TextEditorProxy_SetSendEnterKeyFunc_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_TextEditorProxy_SetSendEnterKeyFunc_001, TestSize.Level0)
{
    auto ret = OH_TextEditorProxy_SetSendEnterKeyFunc(nullptr, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    InputMethod_TextEditorProxy *proxy = OH_TextEditorProxy_Create();
    ASSERT_NE(proxy, nullptr);
    ret = OH_TextEditorProxy_SetSendEnterKeyFunc(proxy, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    OH_TextEditorProxy_Destroy(proxy);
}

/**
 * @tc.name: OH_TextEditorProxy_SetMoveCursorFunc_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_TextEditorProxy_SetMoveCursorFunc_001, TestSize.Level0)
{
    auto ret = OH_TextEditorProxy_SetMoveCursorFunc(nullptr, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    InputMethod_TextEditorProxy *proxy = OH_TextEditorProxy_Create();
    ASSERT_NE(proxy, nullptr);
    ret = OH_TextEditorProxy_SetMoveCursorFunc(proxy, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    OH_TextEditorProxy_Destroy(proxy);
}

/**
 * @tc.name: OH_TextEditorProxy_SetHandleSetSelectionFunc_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_TextEditorProxy_SetHandleSetSelectionFunc_001, TestSize.Level0)
{
    auto ret = OH_TextEditorProxy_SetHandleSetSelectionFunc(nullptr, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    InputMethod_TextEditorProxy *proxy = OH_TextEditorProxy_Create();
    ASSERT_NE(proxy, nullptr);
    ret = OH_TextEditorProxy_SetHandleSetSelectionFunc(proxy, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    OH_TextEditorProxy_Destroy(proxy);
}

/**
 * @tc.name: OH_TextEditorProxy_SetHandleExtendActionFunc_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_TextEditorProxy_SetHandleExtendActionFunc_001, TestSize.Level0)
{
    auto ret = OH_TextEditorProxy_SetHandleExtendActionFunc(nullptr, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    InputMethod_TextEditorProxy *proxy = OH_TextEditorProxy_Create();
    ASSERT_NE(proxy, nullptr);
    ret = OH_TextEditorProxy_SetHandleExtendActionFunc(proxy, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    OH_TextEditorProxy_Destroy(proxy);
}

/**
 * @tc.name: OH_TextEditorProxy_SetGetLeftTextOfCursorFunc_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_TextEditorProxy_SetGetLeftTextOfCursorFunc_001, TestSize.Level0)
{
    auto ret = OH_TextEditorProxy_SetGetLeftTextOfCursorFunc(nullptr, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    InputMethod_TextEditorProxy *proxy = OH_TextEditorProxy_Create();
    ASSERT_NE(proxy, nullptr);
    ret = OH_TextEditorProxy_SetGetLeftTextOfCursorFunc(proxy, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    OH_TextEditorProxy_Destroy(proxy);
}

/**
 * @tc.name: OH_TextEditorProxy_SetGetRightTextOfCursorFunc_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_TextEditorProxy_SetGetRightTextOfCursorFunc_001, TestSize.Level0)
{
    auto ret = OH_TextEditorProxy_SetGetRightTextOfCursorFunc(nullptr, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    InputMethod_TextEditorProxy *proxy = OH_TextEditorProxy_Create();
    ASSERT_NE(proxy, nullptr);
    ret = OH_TextEditorProxy_SetGetRightTextOfCursorFunc(proxy, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    OH_TextEditorProxy_Destroy(proxy);
}

/**
 * @tc.name: OH_TextEditorProxy_SetGetTextIndexAtCursorFunc_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_TextEditorProxy_SetGetTextIndexAtCursorFunc_001, TestSize.Level0)
{
    auto ret = OH_TextEditorProxy_SetGetTextIndexAtCursorFunc(nullptr, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    InputMethod_TextEditorProxy *proxy = OH_TextEditorProxy_Create();
    ASSERT_NE(proxy, nullptr);
    ret = OH_TextEditorProxy_SetGetTextIndexAtCursorFunc(proxy, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    OH_TextEditorProxy_Destroy(proxy);
}

/**
 * @tc.name: OH_TextEditorProxy_SetReceivePrivateCommandFunc_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_TextEditorProxy_SetReceivePrivateCommandFunc_001, TestSize.Level0)
{
    auto ret = OH_TextEditorProxy_SetReceivePrivateCommandFunc(nullptr, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    InputMethod_TextEditorProxy *proxy = OH_TextEditorProxy_Create();
    ASSERT_NE(proxy, nullptr);
    ret = OH_TextEditorProxy_SetReceivePrivateCommandFunc(proxy, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    OH_TextEditorProxy_Destroy(proxy);
}

/**
 * @tc.name: OH_TextEditorProxy_SetSetPreviewTextFunc_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_TextEditorProxy_SetSetPreviewTextFunc_001, TestSize.Level0)
{
    auto ret = OH_TextEditorProxy_SetSetPreviewTextFunc(nullptr, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    InputMethod_TextEditorProxy *proxy = OH_TextEditorProxy_Create();
    ASSERT_NE(proxy, nullptr);
    ret = OH_TextEditorProxy_SetSetPreviewTextFunc(proxy, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    OH_TextEditorProxy_Destroy(proxy);
}

/**
 * @tc.name: OH_TextEditorProxy_SetFinishTextPreviewFunc_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_TextEditorProxy_SetFinishTextPreviewFunc_001, TestSize.Level0)
{
    auto ret = OH_TextEditorProxy_SetFinishTextPreviewFunc(nullptr, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    InputMethod_TextEditorProxy *proxy = OH_TextEditorProxy_Create();
    ASSERT_NE(proxy, nullptr);
    ret = OH_TextEditorProxy_SetFinishTextPreviewFunc(proxy, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    OH_TextEditorProxy_Destroy(proxy);
}

/**
 * @tc.name: OH_TextEditorProxy_GetGetTextConfigFunc_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_TextEditorProxy_GetGetTextConfigFunc_001, TestSize.Level0)
{
    auto ret = OH_TextEditorProxy_GetGetTextConfigFunc(nullptr, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    InputMethod_TextEditorProxy *proxy = OH_TextEditorProxy_Create();
    ASSERT_NE(proxy, nullptr);
    ret = OH_TextEditorProxy_GetGetTextConfigFunc(proxy, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    OH_TextEditorProxy_Destroy(proxy);
}

/**
 * @tc.name: OH_TextEditorProxy_GetInsertTextFunc_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_TextEditorProxy_GetInsertTextFunc_001, TestSize.Level0)
{
    auto ret = OH_TextEditorProxy_GetInsertTextFunc(nullptr, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    InputMethod_TextEditorProxy *proxy = OH_TextEditorProxy_Create();
    ASSERT_NE(proxy, nullptr);
    ret = OH_TextEditorProxy_GetInsertTextFunc(proxy, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    OH_TextEditorProxy_Destroy(proxy);
}

/**
 * @tc.name: OH_TextEditorProxy_GetDeleteForwardFunc_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_TextEditorProxy_GetDeleteForwardFunc_001, TestSize.Level0)
{
    auto ret = OH_TextEditorProxy_GetDeleteForwardFunc(nullptr, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    InputMethod_TextEditorProxy *proxy = OH_TextEditorProxy_Create();
    ASSERT_NE(proxy, nullptr);
    ret = OH_TextEditorProxy_GetDeleteForwardFunc(proxy, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    OH_TextEditorProxy_Destroy(proxy);
}

/**
 * @tc.name: OH_TextEditorProxy_GetDeleteBackwardFunc_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_TextEditorProxy_GetDeleteBackwardFunc_001, TestSize.Level0)
{
    auto ret = OH_TextEditorProxy_GetDeleteBackwardFunc(nullptr, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    InputMethod_TextEditorProxy *proxy = OH_TextEditorProxy_Create();
    ASSERT_NE(proxy, nullptr);
    ret = OH_TextEditorProxy_GetDeleteBackwardFunc(proxy, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    OH_TextEditorProxy_Destroy(proxy);
}

/**
 * @tc.name: OH_TextEditorProxy_GetSendKeyboardStatusFunc_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_TextEditorProxy_GetSendKeyboardStatusFunc_001, TestSize.Level0)
{
    auto ret = OH_TextEditorProxy_GetSendKeyboardStatusFunc(nullptr, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    InputMethod_TextEditorProxy *proxy = OH_TextEditorProxy_Create();
    ASSERT_NE(proxy, nullptr);
    ret = OH_TextEditorProxy_GetSendKeyboardStatusFunc(proxy, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    OH_TextEditorProxy_Destroy(proxy);
}

/**
 * @tc.name: OH_TextEditorProxy_GetSendEnterKeyFunc_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_TextEditorProxy_GetSendEnterKeyFunc_001, TestSize.Level0)
{
    auto ret = OH_TextEditorProxy_GetSendEnterKeyFunc(nullptr, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    InputMethod_TextEditorProxy *proxy = OH_TextEditorProxy_Create();
    ASSERT_NE(proxy, nullptr);
    ret = OH_TextEditorProxy_GetSendEnterKeyFunc(proxy, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    OH_TextEditorProxy_Destroy(proxy);
}

/**
 * @tc.name: OH_TextEditorProxy_GetMoveCursorFunc_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_TextEditorProxy_GetMoveCursorFunc_001, TestSize.Level0)
{
    auto ret = OH_TextEditorProxy_GetMoveCursorFunc(nullptr, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    InputMethod_TextEditorProxy *proxy = OH_TextEditorProxy_Create();
    ASSERT_NE(proxy, nullptr);
    ret = OH_TextEditorProxy_GetMoveCursorFunc(proxy, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    OH_TextEditorProxy_Destroy(proxy);
}

/**
 * @tc.name: OH_TextEditorProxy_GetHandleSetSelectionFunc_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_TextEditorProxy_GetHandleSetSelectionFunc_001, TestSize.Level0)
{
    auto ret = OH_TextEditorProxy_GetHandleSetSelectionFunc(nullptr, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    InputMethod_TextEditorProxy *proxy = OH_TextEditorProxy_Create();
    ASSERT_NE(proxy, nullptr);
    ret = OH_TextEditorProxy_GetHandleSetSelectionFunc(proxy, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    OH_TextEditorProxy_Destroy(proxy);
}

/**
 * @tc.name: OH_TextEditorProxy_GetHandleExtendActionFunc_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_TextEditorProxy_GetHandleExtendActionFunc_001, TestSize.Level0)
{
    auto ret = OH_TextEditorProxy_GetHandleExtendActionFunc(nullptr, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    InputMethod_TextEditorProxy *proxy = OH_TextEditorProxy_Create();
    ASSERT_NE(proxy, nullptr);
    ret = OH_TextEditorProxy_GetHandleExtendActionFunc(proxy, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    OH_TextEditorProxy_Destroy(proxy);
}

/**
 * @tc.name: OH_TextEditorProxy_GetGetLeftTextOfCursorFunc_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_TextEditorProxy_GetGetLeftTextOfCursorFunc_001, TestSize.Level0)
{
    auto ret = OH_TextEditorProxy_GetGetLeftTextOfCursorFunc(nullptr, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    InputMethod_TextEditorProxy *proxy = OH_TextEditorProxy_Create();
    ASSERT_NE(proxy, nullptr);
    ret = OH_TextEditorProxy_GetGetLeftTextOfCursorFunc(proxy, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    OH_TextEditorProxy_Destroy(proxy);
}

/**
 * @tc.name: OH_TextEditorProxy_GetGetRightTextOfCursorFunc_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_TextEditorProxy_GetGetRightTextOfCursorFunc_001, TestSize.Level0)
{
    auto ret = OH_TextEditorProxy_GetGetRightTextOfCursorFunc(nullptr, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    InputMethod_TextEditorProxy *proxy = OH_TextEditorProxy_Create();
    ASSERT_NE(proxy, nullptr);
    ret = OH_TextEditorProxy_GetGetRightTextOfCursorFunc(proxy, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    OH_TextEditorProxy_Destroy(proxy);
}

/**
 * @tc.name: OH_TextEditorProxy_GetGetTextIndexAtCursorFunc_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_TextEditorProxy_GetGetTextIndexAtCursorFunc_001, TestSize.Level0)
{
    auto ret = OH_TextEditorProxy_GetGetTextIndexAtCursorFunc(nullptr, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    InputMethod_TextEditorProxy *proxy = OH_TextEditorProxy_Create();
    ASSERT_NE(proxy, nullptr);
    ret = OH_TextEditorProxy_GetGetTextIndexAtCursorFunc(proxy, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    OH_TextEditorProxy_Destroy(proxy);
}

/**
 * @tc.name: OH_TextEditorProxy_GetReceivePrivateCommandFunc_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_TextEditorProxy_GetReceivePrivateCommandFunc_001, TestSize.Level0)
{
    auto ret = OH_TextEditorProxy_GetReceivePrivateCommandFunc(nullptr, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    InputMethod_TextEditorProxy *proxy = OH_TextEditorProxy_Create();
    ASSERT_NE(proxy, nullptr);
    ret = OH_TextEditorProxy_GetReceivePrivateCommandFunc(proxy, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    OH_TextEditorProxy_Destroy(proxy);
}

/**
 * @tc.name: OH_TextEditorProxy_GetSetPreviewTextFunc_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_TextEditorProxy_GetSetPreviewTextFunc_001, TestSize.Level0)
{
    auto ret = OH_TextEditorProxy_GetSetPreviewTextFunc(nullptr, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    InputMethod_TextEditorProxy *proxy = OH_TextEditorProxy_Create();
    ASSERT_NE(proxy, nullptr);
    ret = OH_TextEditorProxy_GetSetPreviewTextFunc(proxy, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    OH_TextEditorProxy_Destroy(proxy);
}

/**
 * @tc.name: OH_TextEditorProxy_GetFinishTextPreviewFunc_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_TextEditorProxy_GetFinishTextPreviewFunc_001, TestSize.Level0)
{
    auto ret = OH_TextEditorProxy_GetFinishTextPreviewFunc(nullptr, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    InputMethod_TextEditorProxy *proxy = OH_TextEditorProxy_Create();
    ASSERT_NE(proxy, nullptr);
    ret = OH_TextEditorProxy_GetFinishTextPreviewFunc(proxy, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    OH_TextEditorProxy_Destroy(proxy);
}

/**
 * @tc.name: OH_AttachOptions_IsShowKeyboard_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_AttachOptions_IsShowKeyboard_001, TestSize.Level0)
{
    auto ret = OH_AttachOptions_IsShowKeyboard(nullptr, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    InputMethod_AttachOptions *options = OH_AttachOptions_Create(true);
    ASSERT_NE(options, nullptr);
    ret = OH_AttachOptions_IsShowKeyboard(options, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    OH_AttachOptions_Destroy(options);
}

/**
 * @tc.name: OH_TextAvoidInfo_SetPositionY_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_TextAvoidInfo_SetPositionY_001, TestSize.Level0)
{
    double positionY = 0.0;
    auto ret = OH_TextAvoidInfo_SetPositionY(nullptr, positionY);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
}

/**
 * @tc.name: OH_TextAvoidInfo_SetHeight_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_TextAvoidInfo_SetHeight_001, TestSize.Level0)
{
    double positionY = 0.0;
    auto ret = OH_TextAvoidInfo_SetHeight(nullptr, positionY);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
}

/**
 * @tc.name: OH_TextAvoidInfo_GetPositionY_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_TextAvoidInfo_GetPositionY_001, TestSize.Level0)
{
    double positionY = 0.0;
    double height = 0.0;
    auto ret = OH_TextAvoidInfo_GetPositionY(nullptr, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    InputMethod_TextAvoidInfo *info = OH_TextAvoidInfo_Create(positionY, height);
    ASSERT_NE(info, nullptr);
    ret = OH_TextAvoidInfo_GetPositionY(info, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    OH_TextAvoidInfo_Destroy(info);
}

/**
 * @tc.name: OH_TextAvoidInfo_GetHeight_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_TextAvoidInfo_GetHeight_001, TestSize.Level0)
{
    double positionY = 0.0;
    double height = 0.0;
    auto ret = OH_TextAvoidInfo_GetHeight(nullptr, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    InputMethod_TextAvoidInfo *info = OH_TextAvoidInfo_Create(positionY, height);
    ASSERT_NE(info, nullptr);
    ret = OH_TextAvoidInfo_GetHeight(info, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    OH_TextAvoidInfo_Destroy(info);
}

/**
 * @tc.name: OH_PrivateCommand_SetKey_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_PrivateCommand_SetKey_001, TestSize.Level0)
{
    char key[] = "example key";
    size_t keyLength = strlen(key);
    auto ret = OH_PrivateCommand_SetKey(nullptr, nullptr, keyLength);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    InputMethod_PrivateCommand *command = OH_PrivateCommand_Create(key, keyLength);
    ASSERT_NE(command, nullptr);
    ret = OH_PrivateCommand_SetKey(command, nullptr, keyLength);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    OH_PrivateCommand_Destroy(command);
}

/**
 * @tc.name: OH_PrivateCommand_SetBoolValue_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_PrivateCommand_SetBoolValue_001, TestSize.Level0)
{
    bool value = false;
    auto ret = OH_PrivateCommand_SetBoolValue(nullptr, value);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
}

/**
 * @tc.name: OH_PrivateCommand_SetIntValue_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_PrivateCommand_SetIntValue_001, TestSize.Level0)
{
    int32_t value = 0;
    auto ret = OH_PrivateCommand_SetIntValue(nullptr, value);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
}

/**
 * @tc.name: OH_PrivateCommand_SetIntValue_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_PrivateCommand_SetStrValue_001, TestSize.Level0)
{
    char value[] = "example value";
    size_t valueLength = strlen(value);
    auto ret = OH_PrivateCommand_SetStrValue(nullptr, value, valueLength);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    char key[] = "example key";
    size_t keyLength = strlen(key);
    InputMethod_PrivateCommand *command = OH_PrivateCommand_Create(key, keyLength);
    ASSERT_NE(command, nullptr);
    ret = OH_PrivateCommand_SetStrValue(command, nullptr, valueLength);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    OH_PrivateCommand_Destroy(command);
}

/**
 * @tc.name: OH_PrivateCommand_GetKey_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_PrivateCommand_GetKey_001, TestSize.Level0)
{
    char key[] = "example key";
    size_t keyLength = strlen(key);
    auto ret = OH_PrivateCommand_GetKey(nullptr, nullptr, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    InputMethod_PrivateCommand *command = OH_PrivateCommand_Create(key, keyLength);
    ASSERT_NE(command, nullptr);
    ret = OH_PrivateCommand_GetKey(command, nullptr, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    const char *actStrKey = nullptr;
    ret = OH_PrivateCommand_GetKey(command, &actStrKey, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    OH_PrivateCommand_Destroy(command);
}

/**
 * @tc.name: OH_PrivateCommand_GetValueType_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_PrivateCommand_GetValueType_001, TestSize.Level0)
{
    auto ret = OH_PrivateCommand_GetValueType(nullptr, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    char key[] = "example key";
    size_t keyLength = strlen(key);
    InputMethod_PrivateCommand *command = OH_PrivateCommand_Create(key, keyLength);
    ret = OH_PrivateCommand_GetValueType(command, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    OH_PrivateCommand_Destroy(command);
}

/**
 * @tc.name: OH_PrivateCommand_GetBoolValue_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_PrivateCommand_GetBoolValue_001, TestSize.Level0)
{
    auto ret = OH_PrivateCommand_GetBoolValue(nullptr, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    char key[] = "example key";
    size_t keyLength = strlen(key);
    InputMethod_PrivateCommand *command = OH_PrivateCommand_Create(key, keyLength);
    ret = OH_PrivateCommand_GetBoolValue(command, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    ret = OH_PrivateCommand_GetBoolValue(command, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    int32_t expIntValue = 1;
    EXPECT_EQ(IME_ERR_OK, OH_PrivateCommand_SetIntValue(command, expIntValue));
    bool value = false;
    ret = OH_PrivateCommand_GetBoolValue(command, &value);
    EXPECT_EQ(ret, IME_ERR_QUERY_FAILED);
    OH_PrivateCommand_Destroy(command);
}

/**
 * @tc.name: OH_PrivateCommand_GetIntValue_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_PrivateCommand_GetIntValue_001, TestSize.Level0)
{
    auto ret = OH_PrivateCommand_GetIntValue(nullptr, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    char key[] = "example key";
    size_t keyLength = strlen(key);
    InputMethod_PrivateCommand *command = OH_PrivateCommand_Create(key, keyLength);
    ret = OH_PrivateCommand_GetIntValue(command, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    bool expBoolValue = false;
    EXPECT_EQ(IME_ERR_OK, OH_PrivateCommand_SetBoolValue(command, expBoolValue));
    int32_t value = 0;
    ret = OH_PrivateCommand_GetIntValue(command, &value);
    EXPECT_EQ(ret, IME_ERR_QUERY_FAILED);
    OH_PrivateCommand_Destroy(command);
}

/**
 * @tc.name: OH_PrivateCommand_GetStrValue_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_PrivateCommand_GetStrValue_001, TestSize.Level0)
{
    const char *value = nullptr;
    size_t valueLength = 0;
    auto ret = OH_PrivateCommand_GetStrValue(nullptr, &value, &valueLength);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    char key[] = "example key";
    size_t keyLength = strlen(key);
    InputMethod_PrivateCommand *command = OH_PrivateCommand_Create(key, keyLength);
    ret = OH_PrivateCommand_GetStrValue(command, nullptr, &valueLength);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    ret = OH_PrivateCommand_GetStrValue(command, &value, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    bool expBoolValue = false;
    EXPECT_EQ(IME_ERR_OK, OH_PrivateCommand_SetBoolValue(command, expBoolValue));
    ret = OH_PrivateCommand_GetStrValue(command, &value, &valueLength);
    EXPECT_EQ(ret, IME_ERR_QUERY_FAILED);
    OH_PrivateCommand_Destroy(command);
}

/**
 * @tc.name: OH_InputMethodController_Attach_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_InputMethodController_Attach_001, TestSize.Level0)
{
    auto ret = OH_InputMethodController_Attach(nullptr, nullptr, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    auto textEditorProxy = OH_TextEditorProxy_Create();
    ret = OH_InputMethodController_Attach(textEditorProxy, nullptr, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    ConstructTextEditorProxy(textEditorProxy);
    ret = OH_InputMethodController_Attach(textEditorProxy, nullptr, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    InputMethod_AttachOptions *options = OH_AttachOptions_Create(true);
    ret = OH_InputMethodController_Attach(textEditorProxy, options, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    OH_AttachOptions_Destroy(options);
    OH_TextEditorProxy_Destroy(textEditorProxy);
}

/**
 * @tc.name: OH_InputMethodController_Detach_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_InputMethodController_Detach_001, TestSize.Level0)
{
    auto ret = OH_InputMethodController_Detach(nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
}

/**
 * @tc.name: OH_InputMethodProxy_ShowKeyboard_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_InputMethodProxy_ShowKeyboard_001, TestSize.Level0)
{
    auto ret = OH_InputMethodProxy_ShowKeyboard(nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
}

/**
 * @tc.name: OH_InputMethodProxy_HideKeyboard_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_InputMethodProxy_HideKeyboard_001, TestSize.Level0)
{
    auto ret = OH_InputMethodProxy_HideKeyboard(nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
}

/**
 * @tc.name: OH_InputMethodProxy_NotifySelectionChange_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_InputMethodProxy_NotifySelectionChange_001, TestSize.Level0)
{
    size_t length = 0;
    int start = 0;
    int end = 0;
    auto ret = OH_InputMethodProxy_NotifySelectionChange(nullptr, nullptr, length, start, end);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
}

/**
 * @tc.name: OH_InputMethodProxy_NotifyConfigurationChange_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_InputMethodProxy_NotifyConfigurationChange_001, TestSize.Level0)
{
    InputMethod_EnterKeyType enterKey = IME_ENTER_KEY_UNSPECIFIED;
    InputMethod_TextInputType expInput = IME_TEXT_INPUT_TYPE_NUMBER_DECIMAL;
    auto ret = OH_InputMethodProxy_NotifyConfigurationChange(nullptr, enterKey, expInput);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
}

/**
 * @tc.name: OH_InputMethodProxy_NotifyCursorUpdate_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_InputMethodProxy_NotifyCursorUpdate_001, TestSize.Level0)
{
    double left = 0;
    double top = 1.0;
    double width = 2.0;
    double height = 3.0;
    InputMethod_CursorInfo *cursorInfo = OH_CursorInfo_Create(left, top, width, height);
    auto ret = OH_InputMethodProxy_NotifyCursorUpdate(nullptr, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    ret = OH_InputMethodProxy_NotifyCursorUpdate(nullptr, cursorInfo);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
}

/**
 * @tc.name: OH_InputMethodProxy_SendPrivateCommand_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_InputMethodProxy_SendPrivateCommand_001, TestSize.Level0)
{
    char key[] = "example key";
    size_t keyLength = strlen(key);
    char key1[] = "example key";
    size_t keyLength1 = strlen(key);
    InputMethod_PrivateCommand *privateCommand[] = { OH_PrivateCommand_Create(key, keyLength),
        OH_PrivateCommand_Create(key1, keyLength1), nullptr };
    size_t size = 3;
    auto ret = OH_InputMethodProxy_SendPrivateCommand(nullptr, nullptr, size);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    ret = OH_InputMethodProxy_SendPrivateCommand(nullptr, privateCommand, size);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
}

/**
 * @tc.name: TestAttachWithNullParam_001
 * @tc.desc: input parameters is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, TestAttachWithNullParam_001, TestSize.Level0)
{
    auto ret = OH_InputMethodController_Attach(nullptr, nullptr, nullptr);
    EXPECT_EQ(IME_ERR_NULL_POINTER, ret);

    auto textEditorProxy = OH_TextEditorProxy_Create();
    EXPECT_NE(nullptr, textEditorProxy);

    EXPECT_EQ(IME_ERR_NULL_POINTER, OH_InputMethodController_Attach(textEditorProxy, nullptr, nullptr));

    EXPECT_EQ(IME_ERR_OK, OH_TextEditorProxy_SetGetTextConfigFunc(textEditorProxy, GetTextConfigFunc));
    EXPECT_EQ(IME_ERR_NULL_POINTER, OH_InputMethodController_Attach(textEditorProxy, nullptr, nullptr));

    EXPECT_EQ(IME_ERR_OK, OH_TextEditorProxy_SetInsertTextFunc(textEditorProxy, InsertTextFunc));
    EXPECT_EQ(IME_ERR_NULL_POINTER, OH_InputMethodController_Attach(textEditorProxy, nullptr, nullptr));

    EXPECT_EQ(IME_ERR_OK, OH_TextEditorProxy_SetDeleteForwardFunc(textEditorProxy, DeleteForwardFunc));
    EXPECT_EQ(IME_ERR_NULL_POINTER, OH_InputMethodController_Attach(textEditorProxy, nullptr, nullptr));

    EXPECT_EQ(IME_ERR_OK, OH_TextEditorProxy_SetDeleteBackwardFunc(textEditorProxy, DeleteBackwardFunc));
    EXPECT_EQ(IME_ERR_NULL_POINTER, OH_InputMethodController_Attach(textEditorProxy, nullptr, nullptr));

    EXPECT_EQ(IME_ERR_OK, OH_TextEditorProxy_SetSendKeyboardStatusFunc(textEditorProxy, SendKeyboardStatusFunc));
    EXPECT_EQ(IME_ERR_NULL_POINTER, OH_InputMethodController_Attach(textEditorProxy, nullptr, nullptr));

    EXPECT_EQ(IME_ERR_OK, OH_TextEditorProxy_SetSendEnterKeyFunc(textEditorProxy, SendEnterKeyFunc));
    EXPECT_EQ(IME_ERR_NULL_POINTER, OH_InputMethodController_Attach(textEditorProxy, nullptr, nullptr));

    EXPECT_EQ(IME_ERR_OK, OH_TextEditorProxy_SetMoveCursorFunc(textEditorProxy, MoveCursorFunc));
    EXPECT_EQ(IME_ERR_NULL_POINTER, OH_InputMethodController_Attach(textEditorProxy, nullptr, nullptr));

    EXPECT_EQ(IME_ERR_OK, OH_TextEditorProxy_SetHandleSetSelectionFunc(textEditorProxy, HandleSetSelectionFunc));
    EXPECT_EQ(IME_ERR_NULL_POINTER, OH_InputMethodController_Attach(textEditorProxy, nullptr, nullptr));

    EXPECT_EQ(IME_ERR_OK, OH_TextEditorProxy_SetHandleExtendActionFunc(textEditorProxy, HandleExtendActionFunc));
    EXPECT_EQ(IME_ERR_NULL_POINTER, OH_InputMethodController_Attach(textEditorProxy, nullptr, nullptr));

    EXPECT_EQ(IME_ERR_OK, OH_TextEditorProxy_SetGetLeftTextOfCursorFunc(textEditorProxy, GetleftTextOfCursorFunc));
    EXPECT_EQ(IME_ERR_NULL_POINTER, OH_InputMethodController_Attach(textEditorProxy, nullptr, nullptr));

    EXPECT_EQ(IME_ERR_OK, OH_TextEditorProxy_SetGetRightTextOfCursorFunc(textEditorProxy, GetRightTextOfCursorFunc));
    EXPECT_EQ(IME_ERR_NULL_POINTER, OH_InputMethodController_Attach(textEditorProxy, nullptr, nullptr));

    EXPECT_EQ(IME_ERR_OK, OH_TextEditorProxy_SetGetTextIndexAtCursorFunc(textEditorProxy, GetTextIndexAtCursorFunc));
    EXPECT_EQ(IME_ERR_NULL_POINTER, OH_InputMethodController_Attach(textEditorProxy, nullptr, nullptr));

    EXPECT_EQ(IME_ERR_OK, OH_TextEditorProxy_SetReceivePrivateCommandFunc(textEditorProxy, ReceivePrivateCommandFunc));
    EXPECT_EQ(IME_ERR_NULL_POINTER, OH_InputMethodController_Attach(textEditorProxy, nullptr, nullptr));

    EXPECT_EQ(IME_ERR_OK, OH_TextEditorProxy_SetSetPreviewTextFunc(textEditorProxy, SetPreviewTextFunc));
    EXPECT_EQ(IME_ERR_NULL_POINTER, OH_InputMethodController_Attach(textEditorProxy, nullptr, nullptr));

    OH_TextEditorProxy_Destroy(textEditorProxy);
}

/**
 * @tc.name: TestAttachWithNorrmalParam_001
 * @tc.desc: input parameters is normal
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, TestAttachWithNorrmalParam_001, TestSize.Level0)
{
    auto textEditorProxy = OH_TextEditorProxy_Create();
    EXPECT_NE(nullptr, textEditorProxy);
    ConstructTextEditorProxy(textEditorProxy);

    auto options = OH_AttachOptions_Create(true);
    EXPECT_NE(nullptr, options);
    InputMethod_InputMethodProxy *inputMethodProxy = nullptr;
    EXPECT_EQ(IME_ERR_IMCLIENT, OH_InputMethodController_Attach(textEditorProxy, options, &inputMethodProxy));
    EXPECT_EQ(IME_ERR_IMCLIENT, OH_InputMethodController_Attach(textEditorProxy, options, &inputMethodProxy));

    auto textEditorProxy2 = OH_TextEditorProxy_Create();
    EXPECT_NE(nullptr, textEditorProxy2);
    ConstructTextEditorProxy(textEditorProxy2);
    EXPECT_EQ(IME_ERR_IMCLIENT, OH_InputMethodController_Attach(textEditorProxy2, options, &inputMethodProxy));
    OH_TextEditorProxy_Destroy(textEditorProxy2);

    OH_AttachOptions_Destroy(options);
    OH_TextEditorProxy_Destroy(textEditorProxy);
}

/**
 * @tc.name: TestAttachWithPlaceholderAndAbility_001
 * @tc.desc: the input parameter contains the placeholder and ability name
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, TestAttachWithPlaceholderAndAbility_001, TestSize.Level0)
{
    auto options = OH_AttachOptions_Create(true);
    auto textEditorProxy2 = OH_TextEditorProxy_Create();
    EXPECT_NE(nullptr, textEditorProxy2);
    ConstructTextEditorProxy(textEditorProxy2);
    auto fnGetTextConfigFunc = [](InputMethod_TextEditorProxy *textEditorProxy,
     InputMethod_TextConfig *config) {
        std::u16string  placeholder = u"test placeholder";
        std::u16string abilityName = u"test ability name";
        OH_TextConfig_SetPlaceholder(config, placeholder.data(), placeholder.size());
        OH_TextConfig_SetAbilityName(config, abilityName.data(), abilityName.size());
    };
    OH_TextEditorProxy_SetGetTextConfigFunc(textEditorProxy2, fnGetTextConfigFunc);
    InputMethod_InputMethodProxy *inputMethodProxy = nullptr;
    auto ret = OH_InputMethodController_Attach(textEditorProxy2, options, &inputMethodProxy);
    EXPECT_NE(ret, IME_ERR_OK);
    OH_TextEditorProxy_Destroy(textEditorProxy2);
    OH_AttachOptions_Destroy(options);
}

/**
 * @tc.name: OH_TextConfig_SetPlaceholder_001
 * @tc.desc: Input parameters are valid
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_TextConfig_SetPlaceholder_001, TestSize.Level0)
{
    auto config = OH_TextConfig_Create();
    ASSERT_NE(nullptr, config);
    std::u16string input= u"test";
    auto ret = OH_TextConfig_SetPlaceholder(config, input.data(), input.size());
    EXPECT_EQ(ret, IME_ERR_OK);
    ret = OH_TextConfig_GetPlaceholder(nullptr, nullptr, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    ret = OH_TextConfig_GetPlaceholder(config, nullptr, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    size_t outLen = 513;
    char16_t pOut[513] = {};
    ret = OH_TextConfig_GetPlaceholder(config, pOut, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    outLen = 1;
    ret = OH_TextConfig_GetPlaceholder(config, pOut, &outLen);
    EXPECT_EQ(ret, IME_ERR_PARAMCHECK);
    EXPECT_EQ(outLen -1, input.size());
    outLen = 513;
    ret = OH_TextConfig_GetPlaceholder(config, pOut, &outLen);
    EXPECT_EQ(ret, IME_ERR_OK);
    EXPECT_EQ(outLen -1, input.size());
    outLen = input.size();
    ret = OH_TextConfig_GetPlaceholder(config, pOut, &outLen);
    EXPECT_EQ(ret, IME_ERR_PARAMCHECK);
    std::u16string out(pOut, outLen);
    EXPECT_GT(out.size(), input.size());
    outLen = 4;
    ret = OH_TextConfig_GetPlaceholder(config, pOut, &outLen);
    EXPECT_EQ(ret, IME_ERR_PARAMCHECK);
    OH_TextConfig_Destroy(config);
}

/**
 * @tc.name: OH_TextConfig_SetPlaceholder_002
 * @tc.desc: Invalid test input parameter
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_TextConfig_SetPlaceholder_002, TestSize.Level0) {
    auto config = OH_TextConfig_Create();
    ASSERT_NE(nullptr, config);
    std::u16string input= u"test";
    auto ret = OH_TextConfig_SetPlaceholder(config, input.data(), input.size());
    EXPECT_EQ(ret, IME_ERR_OK);
    ret = OH_TextConfig_SetPlaceholder(config, nullptr, 0);
    EXPECT_EQ(ret, IME_ERR_OK);
    size_t outLen = 512;
    char16_t pOut[512] = {};
    ret = OH_TextConfig_GetPlaceholder(config, pOut, &outLen);
    EXPECT_EQ(ret, IME_ERR_OK);
    EXPECT_EQ(outLen, 1);
    OH_TextConfig_Destroy(config);
}

/**
 * @tc.name: OH_TextConfig_SetPlaceholder_003
 * @tc.desc: Invalid test input parameter
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_TextConfig_SetPlaceholder_003, TestSize.Level0) {
    auto config = OH_TextConfig_Create();
    ASSERT_NE(nullptr, config);
    auto ret = OH_TextConfig_SetPlaceholder(config, nullptr, 257);
    EXPECT_EQ(ret, IME_ERR_OK);
    ret = OH_TextConfig_SetPlaceholder(config, nullptr, 1);
    EXPECT_EQ(ret, IME_ERR_OK);
    ret = OH_TextConfig_SetPlaceholder(nullptr, nullptr, 1);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    std::u16string input = u"";
    for (int i = 0; i < MAX_PLACEHOLDER_SIZE; ++i) {
        input.append(u"𪛊");
    }
    IMSA_HILOGI("inputLen:%{public}zu,input:%{public}s", input.size(), Str16ToStr8(input).c_str());
    ret = OH_TextConfig_SetPlaceholder(config, input.data(), input.size());
    EXPECT_EQ(ret, IME_ERR_OK);
    std::u16string out(MAX_PLACEHOLDER_INPUT_SIZE, u'\0');
    size_t outLen = out.size();
    ret = OH_TextConfig_GetPlaceholder(config, out.data(), &outLen);
    EXPECT_EQ(ret, IME_ERR_OK);
    EXPECT_EQ(out.size(), outLen);
    EXPECT_EQ(out[out.size() - 1], 0);
    out.pop_back();
    EXPECT_EQ(out.compare(input), 0);
    input.append(u"a");
    IMSA_HILOGI("inputLen:%{public}zu,input:%{public}s", input.size(), Str16ToStr8(input).c_str());
    ret = OH_TextConfig_SetPlaceholder(config, input.data(), input.size());
    EXPECT_EQ(ret, IME_ERR_OK);
    std::u16string out2(MAX_PLACEHOLDER_INPUT_SIZE, u'\0');
    outLen = out2.size();
    ret = OH_TextConfig_GetPlaceholder(config, out2.data(), &outLen);
    EXPECT_EQ(ret, IME_ERR_OK);
    EXPECT_EQ(out2[out2.size() - 1], 0);
    input[input.size() - 1] = u'\0';
    EXPECT_EQ(out2.compare(input), 0);
    OH_TextConfig_Destroy(config);
}

/**
 * @tc.name: OH_TextConfig_SetAbilityName_001
 * @tc.desc: Input parameters are valid
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_TextConfig_SetAbilityName_001, TestSize.Level0) {
    auto config = OH_TextConfig_Create();
    ASSERT_NE(nullptr, config);
    std::u16string input= u"test";
    auto ret = OH_TextConfig_SetAbilityName(config, input.data(), input.size());
    EXPECT_EQ(ret, IME_ERR_OK);
    ret = OH_TextConfig_GetAbilityName(nullptr, nullptr, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    ret = OH_TextConfig_GetAbilityName(config, nullptr, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    size_t outLen = 66;
    char16_t pOut[66] = {};
    ret = OH_TextConfig_GetAbilityName(config, pOut, nullptr);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    outLen = 1;
    ret = OH_TextConfig_GetAbilityName(config, pOut, &outLen);
    EXPECT_EQ(ret, IME_ERR_PARAMCHECK);
    EXPECT_EQ(outLen, 5);
    outLen = 65;
    ret = OH_TextConfig_GetAbilityName(config, pOut, &outLen);
    EXPECT_EQ(ret, IME_ERR_OK);
    EXPECT_EQ(outLen, 5);
    outLen = 5;
    ret = OH_TextConfig_GetAbilityName(config, pOut, &outLen);
    EXPECT_EQ(ret, IME_ERR_OK);
    std::u16string out(pOut, outLen);
    IMSA_HILOGI("outLen:%{public}zu,out:%{public}s,outSize:%{public}zu", outLen,
        Str16ToStr8(out).c_str(), out.size());
    EXPECT_GT(out.size(), input.size());
    outLen = input.size();
    ret = OH_TextConfig_GetAbilityName(config, pOut, &outLen);
    EXPECT_EQ(ret, IME_ERR_PARAMCHECK);
    OH_TextConfig_Destroy(config);
}

/**
 * @tc.name: OH_TextConfig_SetAbilityName_002
 * @tc.desc: Invalid test input parameter
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_TextConfig_SetAbilityName_002, TestSize.Level0) {
    auto config = OH_TextConfig_Create();
    ASSERT_NE(nullptr, config);
    std::u16string input= u"test";
    auto ret = OH_TextConfig_SetAbilityName(config, input.data(), input.size());
    EXPECT_EQ(ret, IME_ERR_OK);
    ret = OH_TextConfig_SetAbilityName(config, nullptr, 0);
    EXPECT_EQ(ret, IME_ERR_OK);
    char16_t *pOut = nullptr;
    size_t outLen = 0;
    ret = OH_TextConfig_GetAbilityName(config, pOut, &outLen);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    EXPECT_EQ(outLen, 1);
    EXPECT_EQ(pOut, nullptr);
    OH_TextConfig_Destroy(config);
}

/**
 * @tc.name: OH_TextConfig_SetAbilityName_003
 * @tc.desc: Invalid test input parameter
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_TextConfig_SetAbilityName_003, TestSize.Level0) {
    auto config = OH_TextConfig_Create();
    ASSERT_NE(nullptr, config);
    auto ret = OH_TextConfig_SetAbilityName(config, nullptr, 128);
    EXPECT_EQ(ret, IME_ERR_OK);
    ret = OH_TextConfig_SetAbilityName(config, nullptr, 1);
    EXPECT_EQ(ret, IME_ERR_OK);
    char16_t input[] = u"0";
    ret = OH_TextConfig_SetAbilityName(config, input, 0);
    EXPECT_EQ(ret, IME_ERR_OK);
    ret = OH_TextConfig_SetAbilityName(config, input, 1);
    EXPECT_EQ(ret, IME_ERR_OK);
    ret = OH_TextConfig_SetAbilityName(nullptr, nullptr, 1);
    EXPECT_EQ(ret, IME_ERR_NULL_POINTER);
    OH_TextConfig_Destroy(config);
}

/**
 * @tc.name: OH_TextConfig_SetAbilityName_004
 * @tc.desc: Invalid test input parameter
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodControllerCapiTest, OH_TextConfig_SetAbilityName_004, TestSize.Level0) {
    auto config = OH_TextConfig_Create();
    ASSERT_NE(nullptr, config);
    std::u16string input = u"";
    for (int i = 0; i < MAX_ABILITY_NAME_SIZE; ++i) {
        input.append(u"𪛊");
    }
    IMSA_HILOGI("inputLen:%{public}zu,input:%{public}s", input.size(), Str16ToStr8(input).c_str());
    auto ret = OH_TextConfig_SetAbilityName(config, input.data(), input.size());
    EXPECT_EQ(ret, IME_ERR_OK);
    std::u16string out(MAX_ABILITY_NAME_INPUT_SIZE, u'\0');
    size_t outLen = out.size();
    ret = OH_TextConfig_GetAbilityName(config, out.data(), &outLen);
    EXPECT_EQ(ret, IME_ERR_OK);
    IMSA_HILOGI("outLen:%{public}zu,input:%{public}s,outSize:%{public}zu,inputSize:%{public}zu", outLen,
        Str16ToStr8(input).c_str(), out.size(), input.size());
    EXPECT_GT(out.size(), input.size());
    EXPECT_EQ(out[out.size() - 1], 0);
    out.pop_back();
    EXPECT_EQ(out.compare(input), 0);
    input.append(u"a");
    IMSA_HILOGI("inputLen:%{public}zu,input:%{public}s", input.size(), Str16ToStr8(input).c_str());
    ret = OH_TextConfig_SetAbilityName(config, input.data(), input.size());
    EXPECT_EQ(ret, IME_ERR_OK);
    std::u16string out2(MAX_ABILITY_NAME_INPUT_SIZE, u'\0');
    outLen = out2.size();
    ret = OH_TextConfig_GetAbilityName(config, out2.data(), &outLen);
    EXPECT_EQ(ret, IME_ERR_OK);
    EXPECT_EQ(out2[out2.size() - 1], 0);
    input[input.size() -1] = u'\0';
    EXPECT_EQ(out2.compare(input), 0);
    char16_t charInput[65] = u"123456789\0123456789\0012345678901\023456789";
    size_t charInputLen = 32;
    IMSA_HILOGI("inputLen:%{public}zu,input:%{public}s", charInputLen, Str16ToStr8(input).c_str());
    ret = OH_TextConfig_SetAbilityName(config, charInput, charInputLen);
    EXPECT_EQ(ret, IME_ERR_OK);
    char16_t outChar[66] = {};
    outLen = 33;
    ret = OH_TextConfig_GetAbilityName(config, outChar, &outLen);
    EXPECT_EQ(ret, IME_ERR_OK);
    out = std::u16string(outChar, outLen);
    auto utf8Out = Str16ToStr8(outChar);
    IMSA_HILOGI("outLen:%{public}zu,out:%{public}s, utf8len:%{public}zu", outLen, utf8Out.c_str(), utf8Out.size());
    OH_TextConfig_Destroy(config);
}
} // namespace