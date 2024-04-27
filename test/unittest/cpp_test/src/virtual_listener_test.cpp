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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <string_ex.h>
#include <unistd.h>

#include "global.h"
#include "ime_event_listener.h"
#include "input_method_controller.h"
#include "input_method_engine_listener.h"
#include "input_method_utils.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace MiscServices {
/**
 * @brief Only pure virtual functions are implemented.
 */
class TextListenerImpl : public OnTextChangedListener {
public:
    void InsertText(const std::u16string &text) override
    {
    }
    void DeleteForward(int32_t length) override
    {
    }
    void DeleteBackward(int32_t length) override
    {
    }
    void SendKeyEventFromInputMethod(const KeyEvent &event) override
    {
    }
    void SendKeyboardStatus(const KeyboardStatus &keyboardStatus) override
    {
    }
    void SendFunctionKey(const FunctionKey &functionKey) override
    {
    }
    void SetKeyboardStatus(bool status) override
    {
    }
    void MoveCursor(const Direction direction) override
    {
    }
    void HandleSetSelection(int32_t start, int32_t end) override
    {
    }
    void HandleExtendAction(int32_t action) override
    {
    }
    void HandleSelect(int32_t keyCode, int32_t cursorMoveSkip) override
    {
    }
    std::u16string GetLeftTextOfCursor(int32_t number) override
    {
        return Str8ToStr16("test");
    }
    std::u16string GetRightTextOfCursor(int32_t number) override
    {
        return Str8ToStr16("test");
    }
    int32_t GetTextIndexAtCursor() override
    {
        return 0;
    }
};
/**
 * @brief Only pure virtual functions are implemented.
 */
class EngineListenerImpl : public InputMethodEngineListener {
public:
    void OnKeyboardStatus(bool isShow) override
    {
    }
    void OnInputStart() override
    {
    }
    void OnInputStop() override
    {
    }
    void OnSecurityChange(int32_t security) override
    {
    }
    void OnSetCallingWindow(uint32_t windowId) override
    {
    }
    void OnSetSubtype(const SubProperty &property) override
    {
    }
    void ReceivePrivateCommand(const std::unordered_map<std::string, PrivateDataValue> &privateCommand) override
    {
    }
};
/**
 * @brief Only pure virtual functions are implemented.
 */
class EventListenerImpl : public ImeEventListener {
};

class VirtualListenerTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {
        IMSA_HILOGI("VirtualListenerTest::SetUpTestCase");
        textListener_ = new (std::nothrow) TextListenerImpl();
        eventListener_ = std::make_shared<EventListenerImpl>();
        engineListener_ = std::make_shared<EngineListenerImpl>();
    }
    static void TearDownTestCase(void)
    {
        IMSA_HILOGI("VirtualListenerTest::TearDownTestCase");
    }
    void SetUp()
    {
        IMSA_HILOGI("VirtualListenerTest::SetUp");
    }
    void TearDown()
    {
        IMSA_HILOGI("VirtualListenerTest::TearDown");
    }
    static sptr<OnTextChangedListener> textListener_;
    static std::shared_ptr<ImeEventListener> eventListener_;
    static std::shared_ptr<InputMethodEngineListener> engineListener_;
};
sptr<OnTextChangedListener> VirtualListenerTest::textListener_{ nullptr };
std::shared_ptr<ImeEventListener> VirtualListenerTest::eventListener_{ nullptr };
std::shared_ptr<InputMethodEngineListener> VirtualListenerTest::engineListener_{ nullptr };

/**
 * @tc.name: testOnTextChangedListener_001
 * @tc.desc: Cover non-pure virtual function in class: OnTextChangedListener.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(VirtualListenerTest, testOnTextChangedListener_001, TestSize.Level0)
{
    IMSA_HILOGI("VirtualListenerTest testOnTextChangedListener_001 START");
    PanelStatusInfo statusInfo;
    Range range;
    std::unordered_map<std::string, PrivateDataValue> privateCommand;
    VirtualListenerTest::textListener_->NotifyPanelStatusInfo(statusInfo);
    VirtualListenerTest::textListener_->NotifyKeyboardHeight(0);
    VirtualListenerTest::textListener_->ReceivePrivateCommand(privateCommand);
    VirtualListenerTest::textListener_->SetPreviewText(Str8ToStr16("test"), range);
    VirtualListenerTest::textListener_->FinishTextPreview();
}

/**
 * @tc.name: testInputMethodEngineListener_001
 * @tc.desc: Cover non-pure virtual function in class: InputMethodEngineListener.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(VirtualListenerTest, testInputMethodEngineListener_001, TestSize.Level0)
{
    IMSA_HILOGI("VirtualListenerTest testInputMethodEngineListener_001 START");
    VirtualListenerTest::engineListener_->OnInputFinish();
    VirtualListenerTest::engineListener_->IsEnable();
}

/**
 * @tc.name: testImeEventListener_001
 * @tc.desc: Cover non-pure virtual function in class: ImeEventListener.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(VirtualListenerTest, testImeEventListener_001, TestSize.Level0)
{
    IMSA_HILOGI("VirtualListenerTest testImeEventListener_001 START");
    Property property;
    SubProperty subProperty;
    ImeWindowInfo imeWindowInfo;
    VirtualListenerTest::eventListener_->OnImeChange(property, subProperty);
    VirtualListenerTest::eventListener_->OnImeShow(imeWindowInfo);
    VirtualListenerTest::eventListener_->OnImeHide(imeWindowInfo);
}
} // namespace MiscServices
} // namespace OHOS