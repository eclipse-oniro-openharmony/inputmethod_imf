/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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
#include <sys/time.h>
#include <unistd.h>

#include <cstdint>
#include <string>

#include "accesstoken_kit.h"
#include "global.h"
#include "input_method_ability.h"
#include "input_method_controller.h"
#include "input_method_property.h"
#include "nativetoken_kit.h"
#include "os_account_manager.h"
#include "securec.h"
#include "token_setproc.h"

using namespace testing::ext;
using namespace OHOS::AccountSA;
using namespace OHOS::Security::AccessToken;
namespace OHOS {
namespace MiscServices {
constexpr int32_t MAIN_USER_ID = 100;
static void InitTestConfiguration(const std::string &bundleName)
{
    IMSA_HILOGI("bundleName: %{public}s", bundleName.c_str());
    std::vector<int32_t> userIds;
    auto ret = OsAccountManager::QueryActiveOsAccountIds(userIds);
    if (ret != ErrorCode::NO_ERROR || userIds.empty()) {
        IMSA_HILOGE("query active os account id failed");
        userIds[0] = MAIN_USER_ID;
    }
    HapInfoParams infoParams = {
        .userID = userIds[0],
        .bundleName = bundleName,
        .instIndex = 0,
        .appIDDesc = "ohos.inputmethod_test.demo"
    };
    HapPolicyParams policyParams = {
        .apl = APL_NORMAL,
        .domain = "test.domain.inputmethod",
        .permList = {},
        .permStateList = {}
    };
    AccessTokenKit::AllocHapToken(infoParams, policyParams);
    auto tokenID = AccessTokenKit::GetHapTokenID(infoParams.userID, infoParams.bundleName, infoParams.instIndex);
    int res = SetSelfTokenID(tokenID);
    if (res == ErrorCode::NO_ERROR) {
        IMSA_HILOGI("SetSelfTokenID success!");
    } else {
        IMSA_HILOGE("SetSelfTokenID fail!");
    }
}

class TextListener : public OnTextChangedListener {
public:
    TextListener() = default;
    ~TextListener() override = default;
    void InsertText(const std::u16string &text) override;
    void DeleteBackward(int32_t length) override;
    void SetKeyboardStatus(bool status) override;
    void DeleteForward(int32_t length) override;
    void SendKeyEventFromInputMethod(const KeyEvent &event) override;
    void SendKeyboardInfo(const KeyboardInfo &status) override;
    void MoveCursor(const Direction direction) override;
    void HandleSetSelection(int32_t start, int32_t end) override;
    void HandleExtendAction(int32_t action) override;
    void HandleSelect(int32_t keyCode, int32_t cursorMoveSkip) override;
};

void TextListener::InsertText(const std::u16string &text)
{
}
void TextListener::DeleteBackward(int32_t length)
{
}
void TextListener::SetKeyboardStatus(bool status)
{
}
void TextListener::DeleteForward(int32_t length)
{
}
void TextListener::SendKeyEventFromInputMethod(const KeyEvent &event)
{
}
void TextListener::SendKeyboardInfo(const KeyboardInfo &status)
{
}
void TextListener::MoveCursor(const Direction direction)
{
}
void TextListener::HandleSetSelection(int32_t start, int32_t end)
{
}
void TextListener::HandleExtendAction(int32_t action)
{
}

void TextListener::HandleSelect(int32_t keyCode, int32_t cursorMoveSkip)
{
}

class PermissionVerificationExceptionTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static sptr<InputMethodController> imc_;
    static sptr<OnTextChangedListener> textListener_;
};
sptr<InputMethodController> PermissionVerificationExceptionTest::imc_;
sptr<OnTextChangedListener> PermissionVerificationExceptionTest::textListener_;
void PermissionVerificationExceptionTest::SetUpTestCase(void)
{
    IMSA_HILOGI("PermissionVerificationExceptionTest::SetUpTestCase");
    textListener_ = new TextListener();
    imc_ = InputMethodController::GetInstance();
    imc_->Attach(textListener_);
}

void PermissionVerificationExceptionTest::TearDownTestCase(void)
{
    IMSA_HILOGI("PermissionVerificationExceptionTest::TearDownTestCase");
}

void PermissionVerificationExceptionTest::SetUp(void)
{
    IMSA_HILOGI("PermissionVerificationExceptionTest::SetUp");

}

void PermissionVerificationExceptionTest::TearDown(void)
{
    IMSA_HILOGI("PermissionVerificationExceptionTest::TearDown");
}

/**
 * @tc.name: ShowSoftKeyboard
 * @tc.desc: PermissionVerificationExceptionTest ShowSoftKeyboard.
 * @tc.type: FUNC
 * @tc.require: issuesI640YZ
 */
HWTEST_F(PermissionVerificationExceptionTest, ShowSoftKeyboard, TestSize.Level0)
{
    IMSA_HILOGI("PermissionTest ShowSoftKeyboard TEST START");
    int32_t ret = PermissionVerificationExceptionTest::imc_->ShowSoftKeyboard();
    EXPECT_EQ(ret, ErrorCode::ERROR_STATUS_PERMISSION_DENIED);
}

/**
 * @tc.name: HideSoftKeyboard
 * @tc.desc: PermissionVerificationExceptionTest HideSoftKeyboard.
 * @tc.type: FUNC
 * @tc.require: issuesI640YZ
 */
HWTEST_F(PermissionVerificationExceptionTest, HideSoftKeyboard, TestSize.Level0)
{
    IMSA_HILOGI("PermissionTest HideSoftKeyboard TEST START");
    int32_t ret = PermissionVerificationExceptionTest::imc_->HideSoftKeyboard();
    EXPECT_EQ(ret, ErrorCode::ERROR_STATUS_PERMISSION_DENIED);
}

/**
 * @tc.name: SwitchInputMethod
 * @tc.desc: PermissionVerificationExceptionTest SwitchInputMethod.
 * @tc.type: FUNC
 * @tc.require: issuesI640YZ
 */
HWTEST_F(PermissionVerificationExceptionTest, SwitchInputMethod, TestSize.Level0)
{
    IMSA_HILOGI("PermissionTest SwitchInputMethod TEST START");
    auto property = PermissionVerificationExceptionTest::imc_->GetCurrentInputMethod();
    ASSERT_TRUE(property != nullptr);
    auto subProperty = PermissionVerificationExceptionTest::imc_->GetCurrentInputMethodSubtype();
    ASSERT_TRUE(subProperty != nullptr);

    int32_t ret = PermissionVerificationExceptionTest::imc_->SwitchInputMethod(property->name, subProperty->label);
    EXPECT_EQ(ret, ErrorCode::ERROR_STATUS_PERMISSION_DENIED);
}

/**
 * @tc.name: ShowOptionalInputMethod
 * @tc.desc: PermissionVerificationExceptionTest ShowOptionalInputMethod.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: Zhaolinglan
 */
HWTEST_F(PermissionVerificationExceptionTest, ShowOptionalInputMethod, TestSize.Level0)
{
    IMSA_HILOGI("PermissionTest ShowOptionalInputMethod TEST START");
    int32_t ret = PermissionVerificationExceptionTest::imc_->ShowOptionalInputMethod();
    EXPECT_EQ(ret, ErrorCode::ERROR_STATUS_PERMISSION_DENIED);
}

/**
 * @tc.name: SetCoreAndAgent
 * @tc.desc: PermissionVerificationExceptionTest SetCoreAndAgent.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: Zhaolinglan
 */
HWTEST_F(PermissionVerificationExceptionTest, SetCoreAndAgent, TestSize.Level0)
{
    IMSA_HILOGI("PermissionTest SetCoreAndAgent TEST START");
    int32_t ret = InputMethodAbility::GetInstance()->SetCoreAndAgent();
    EXPECT_EQ(ret, ErrorCode::ERROR_NOT_CURRENT_IME);
}

/**
 * @tc.name: SetCoreAndAgentPassCheck
 * @tc.desc: PermissionVerificationExceptionTest SetCoreAndAgentPassCheck.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: Zhaolinglan
 */
HWTEST_F(PermissionVerificationExceptionTest, SetCoreAndAgentPassCheck, TestSize.Level0)
{
    IMSA_HILOGI("PermissionTest SetCoreAndAgentPassCheck TEST START");
    auto property = InputMethodController::GetInstance()->GetCurrentInputMethod();
    EXPECT_NE(property, nullptr);
    InitTestConfiguration(property->name);
    int32_t ret = InputMethodAbility::GetInstance()->SetCoreAndAgent();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}
} // namespace MiscServices
} // namespace OHOS