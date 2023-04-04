/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include "global.h"
#include "input_method_controller.h"
#include "securec.h"

using namespace testing::ext;
namespace OHOS {
namespace MiscServices {
class PermissionVerficationExceptionTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static sptr<InputMethodController> imc_;
};
sptr<InputMethodController> PermissionVerficationExceptionTest::imc_;
void PermissionVerficationExceptionTest::SetUpTestCase(void)
{
    IMSA_HILOGI("PermissionVerficationExceptionTest::SetUpTestCase");
}

void PermissionVerficationExceptionTest::TearDownTestCase(void)
{
    IMSA_HILOGI("PermissionVerficationExceptionTest::TearDownTestCase");
}

void PermissionVerficationExceptionTest::SetUp(void)
{
    imc_ = InputMethodController::GetInstance();
    IMSA_HILOGI("PermissionVerficationExceptionTest::SetUp");
}

void PermissionVerficationExceptionTest::TearDown(void)
{
    IMSA_HILOGI("PermissionVerficationExceptionTest::TearDown");
}

/**
* @tc.name: ShowSoftKeyboard
* @tc.desc: PermissionVerficationExceptionTest ShowSoftKeyboard.
* @tc.type: FUNC
* @tc.require: issuesI640YZ
*/
HWTEST_F(PermissionVerficationExceptionTest, ShowSoftKeyboard, TestSize.Level0)
{
    int32_t ret = PermissionVerficationExceptionTest::imc_->ShowSoftKeyboard();
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_EDITABLE);
}

/**
* @tc.name: HideSoftKeyboard
* @tc.desc: PermissionVerficationExceptionTest HideSoftKeyboard.
* @tc.type: FUNC
* @tc.require: issuesI640YZ
*/
HWTEST_F(PermissionVerficationExceptionTest, HideSoftKeyboard, TestSize.Level0)
{
    int32_t ret = PermissionVerficationExceptionTest::imc_->HideSoftKeyboard();
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_EDITABLE);
}

/**
* @tc.name: DisplayOptionalInputMethod
* @tc.desc: PermissionVerficationExceptionTest DisplayOptionalInputMethod.
* @tc.type: FUNC
* @tc.require: issuesI640YZ
*/
HWTEST_F(PermissionVerficationExceptionTest, DisplayOptionalInputMethod, TestSize.Level0)
{
    int32_t ret = PermissionVerficationExceptionTest::imc_->ShowOptionalInputMethod();
    EXPECT_EQ(ret, ErrorCode::ERROR_STATUS_PERMISSION_DENIED);
}

/**
* @tc.name: SwitchInputMethod
* @tc.desc: PermissionVerficationExceptionTest SwitchInputMethod.
* @tc.type: FUNC
* @tc.require: issuesI640YZ
*/
HWTEST_F(PermissionVerficationExceptionTest, SwitchInputMethod, TestSize.Level0)
{
    auto property = PermissionVerficationExceptionTest::imc_->GetCurrentInputMethod();
    ASSERT_TRUE(property != nullptr);
    auto subProperty = PermissionVerficationExceptionTest::imc_->GetCurrentInputMethodSubtype();
    ASSERT_TRUE(subProperty != nullptr);

    int32_t ret = PermissionVerficationExceptionTest::imc_->SwitchInputMethod(property->name, subProperty->id);
    EXPECT_EQ(ret, ErrorCode::ERROR_STATUS_PERMISSION_DENIED);
}
} // namespace MiscServices
} // namespace OHOS