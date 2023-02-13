/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#include <string>
#include <sys/time.h>
#include <unistd.h>
#include <vector>

#include "accesstoken_kit.h"
#include "global.h"
#include "input_method_controller.h"
#include "input_method_property.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"

using namespace testing::ext;
using namespace OHOS::Security::AccessToken;
namespace OHOS {
namespace MiscServices {
class InputMethodSwitchTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static void GrantNativePermission();
    static std::mutex imeChangeFlagLock;
    static std::condition_variable conditionVar;
    static bool imeChangeFlag;
    static std::string extBundleName;
    static std::string extAbilityName;
};
std::mutex InputMethodSwitchTest::imeChangeFlagLock;
std::condition_variable InputMethodSwitchTest::conditionVar;
bool InputMethodSwitchTest::imeChangeFlag = false;
std::string InputMethodSwitchTest::extBundleName = "com.example.testIme";
std::string InputMethodSwitchTest::extAbilityName = "InputMethodExtAbility";
constexpr uint32_t DEALY_TIME = 1;
class InputMethodSettingListenerImpl : public InputMethodSettingListener {
public:
    InputMethodSettingListenerImpl() = default;
    ~InputMethodSettingListenerImpl() = default;
    void OnImeChange(const Property &property, const SubProperty &subProperty)
    {
        {
            std::unique_lock<std::mutex> lock(InputMethodSwitchTest::imeChangeFlagLock);
            InputMethodSwitchTest::imeChangeFlag = true;
        }
        InputMethodSwitchTest::conditionVar.notify_one();
        IMSA_HILOGI("InputMethodSettingListenerImpl OnImeChange");
    }
};
constexpr int32_t MINIMUM_INSTALL_INPUTMETHOD_NUM = 2;
constexpr int32_t MINIMUM_DISABLE_INPUTMETHOD_NUM = 1;
constexpr int32_t EXT_INPUTMETHOD_SUBTYPE_NUM = 1;
void InputMethodSwitchTest::SetUpTestCase(void)
{
    IMSA_HILOGI("InputMethodSwitchTest::SetUpTestCase");
    GrantNativePermission();
}

void InputMethodSwitchTest::TearDownTestCase(void)
{
    IMSA_HILOGI("InputMethodSwitchTest::TearDownTestCase");
}

void InputMethodSwitchTest::SetUp(void)
{
    IMSA_HILOGI("InputMethodSwitchTest::SetUp");
}

void InputMethodSwitchTest::TearDown(void)
{
    IMSA_HILOGI("InputMethodSwitchTest::TearDown");
}

void InputMethodSwitchTest::GrantNativePermission()
{
    const char **perms = new const char *[1];
    perms[0] = "ohos.permission.CONNECT_IME_ABILITY";
    TokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = 1,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = "inputmethod_imf",
        .aplStr = "system_core",
    };
    uint64_t tokenId = GetAccessTokenId(&infoInstance);
    int res = SetSelfTokenID(tokenId);
    if (res == 0) {
        IMSA_HILOGI("SetSelfTokenID success!");
    } else {
        IMSA_HILOGE("SetSelfTokenID fail!");
    }
    AccessTokenKit::ReloadNativeTokenInfo();
    delete[] perms;
}

/**
* @tc.name: testIMCSetImeListener
* @tc.desc: IMC testSetImeListener.
* @tc.type: FUNC
* @tc.require: issuesI640YZ
*/
HWTEST_F(InputMethodSwitchTest, testIMCSetImeListener, TestSize.Level0)
{
    IMSA_HILOGI("IMC SetImeListener Test START");
    auto imc = InputMethodController::GetInstance();
    ASSERT_TRUE(imc != nullptr);
    auto listener = std::make_shared<InputMethodSettingListenerImpl>();
    imc->SetImeListener(listener);
}

/**
* @tc.name: testIMCGetCurrentInputMethod
* @tc.desc: IMC GetCurrentInputMethod
* @tc.type: FUNC
* @tc.require: issuesI62BHB
* @tc.author: chenyu
*/
HWTEST_F(InputMethodSwitchTest, testIMCGetCurrentInputMethod, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCGetCurrentInputMethod Test Start");
    sptr<InputMethodController> imc = InputMethodController::GetInstance();
    ASSERT_TRUE(imc != nullptr);

    std::shared_ptr<Property> property = imc->GetCurrentInputMethod();
    ASSERT_TRUE(property != nullptr);
    EXPECT_NE(property->name, InputMethodSwitchTest::extBundleName);
}

/**
* @tc.name: testIMCListInputMethod
* @tc.desc: IMC ListInputMethod
* @tc.type: FUNC
* @tc.require: issuesI62BHB
* @tc.author: chenyu
*/
HWTEST_F(InputMethodSwitchTest, testIMCListInputMethod, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCListInputMethod Test Start");
    sptr<InputMethodController> imc = InputMethodController::GetInstance();
    ASSERT_TRUE(imc != nullptr);

    std::vector<Property> properties = {};
    auto ret = imc->ListInputMethod(properties);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(properties.size() >= MINIMUM_INSTALL_INPUTMETHOD_NUM);
    bool hasExtInputMethod = false;
    for (size_t i = 0; i < properties.size(); i++) {
        if (properties[i].name == InputMethodSwitchTest::extBundleName) {
            hasExtInputMethod = true;
            break;
        }
    }
    EXPECT_TRUE(hasExtInputMethod);
}

/**
* @tc.name: testIMCListInputMethodDisable
* @tc.desc: IMC ListInputMethod
* @tc.type: FUNC
* @tc.require: issuesI62BHB
* @tc.author: chenyu
*/
HWTEST_F(InputMethodSwitchTest, testIMCListInputMethodDisable, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCListInputMethodDisable Test Start");
    sptr<InputMethodController> imc = InputMethodController::GetInstance();
    ASSERT_TRUE(imc != nullptr);

    std::vector<Property> disableProperties = {};
    auto ret = imc->ListInputMethod(false, disableProperties);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(disableProperties.size() >= MINIMUM_DISABLE_INPUTMETHOD_NUM);
    bool hasExtInputMethod = false;
    for (size_t i = 0; i < disableProperties.size(); i++) {
        if (disableProperties[i].name == InputMethodSwitchTest::extBundleName) {
            hasExtInputMethod = true;
            break;
        }
    }
    EXPECT_TRUE(hasExtInputMethod);
}

/**
* @tc.name: testIMCListInputMethodEnable
* @tc.desc: IMC ListInputMethod
* @tc.type: FUNC
* @tc.require: issuesI62BHB
* @tc.author: chenyu
*/
HWTEST_F(InputMethodSwitchTest, testIMCListInputMethodEnable, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCListInputMethodEnable Test Start");
    sptr<InputMethodController> imc = InputMethodController::GetInstance();
    ASSERT_TRUE(imc != nullptr);

    std::vector<Property> enableProperties = {};
    auto ret = imc->ListInputMethod(true, enableProperties);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ASSERT_TRUE(!enableProperties.empty());
    EXPECT_NE(enableProperties[0].name, InputMethodSwitchTest::extBundleName);
}

/**
* @tc.name: testIMCGetCurrentInputMethodSubtype
* @tc.desc: GetCurrentInputMethodSubtype
* @tc.type: FUNC
* @tc.require: issuesI62BHB
* @tc.author: chenyu
*/
HWTEST_F(InputMethodSwitchTest, testIMCGetCurrentInputMethodSubtype, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCGetCurrentInputMethodSubtype Test Start");
    sptr<InputMethodController> imc = InputMethodController::GetInstance();
    ASSERT_TRUE(imc != nullptr);

    auto subProperty = imc->GetCurrentInputMethodSubtype();
    EXPECT_TRUE(subProperty != nullptr);
    EXPECT_NE(subProperty->label, InputMethodSwitchTest::extAbilityName);
}

/**
* @tc.name: testIMCListCurrentInputMethodSubtype
* @tc.desc: ListCurrentInputMethodSubtype
* @tc.type: FUNC
* @tc.require: issuesI62BHB
* @tc.author: chenyu
*/
HWTEST_F(InputMethodSwitchTest, testIMCListCurrentInputMethodSubtype, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCListCurrentInputMethodSubtype Test Start");
    sptr<InputMethodController> imc = InputMethodController::GetInstance();
    ASSERT_TRUE(imc != nullptr);

    std::vector<SubProperty> subProperties = {};
    auto ret = imc->ListCurrentInputMethodSubtype(subProperties);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ASSERT_TRUE(!subProperties.empty());
    EXPECT_NE(subProperties[0].label, InputMethodSwitchTest::extAbilityName);
}

/**
* @tc.name: testIMCListInputMethodSubtype
* @tc.desc: ListInputMethodSubtype
* @tc.type: FUNC
* @tc.require: issuesI62BHB
* @tc.author: chenyu
*/
HWTEST_F(InputMethodSwitchTest, tesIMCtListInputMethodSubtype, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCListInputMethodSubtype Test Start");
    sptr<InputMethodController> imc = InputMethodController::GetInstance();
    ASSERT_TRUE(imc != nullptr);

    Property property = { .name = InputMethodSwitchTest::extBundleName };
    std::vector<SubProperty> subProps;
    auto ret = imc->ListInputMethodSubtype(property, subProps);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ASSERT_EQ(subProps.size(), EXT_INPUTMETHOD_SUBTYPE_NUM);
    EXPECT_EQ(subProps[0].label, InputMethodSwitchTest::extAbilityName);
}

/**
 * @tc.name: testIMCListInputMethodSubtypeWithErrorBundleName
 * @tc.desc: IMC ListInputMethodSubtype
 * @tc.type: FUNC
 * @tc.require: issuesI62BHB
* @tc.author: chenyu
 */
HWTEST_F(InputMethodSwitchTest, testIMCListInputMethodSubtypeWithErrorBundleName, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCListInputMethodSubtypeWitchErrorBundleName Test START");
    std::shared_ptr<Property> property = std::make_shared<Property>();
    std::vector<SubProperty> properties = {};
    auto ret = InputMethodController::GetInstance()->ListInputMethodSubtype(*property, properties);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(properties.empty());
}

/**
* @tc.name: testIMCSwitchInputMethod
* @tc.desc: IMC testSwitchInputMethod.
* @tc.type: FUNC
* @tc.require: issuesI62BHB
* @tc.author: chenyu
*/
HWTEST_F(InputMethodSwitchTest, testIMCSwitchInputMethod, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCSwitchInputMethod Test START");
    sptr<InputMethodController> imc = InputMethodController::GetInstance();
    ASSERT_TRUE(imc != nullptr);
    // get default inputmethod
    std::shared_ptr<Property> defaultProperty = imc->GetCurrentInputMethod();
    ASSERT_TRUE(defaultProperty != nullptr);
    std::shared_ptr<SubProperty> defaultSubProperty = imc->GetCurrentInputMethodSubtype();
    ASSERT_TRUE(defaultSubProperty != nullptr);

    // switch to ext inputmethod
    auto ret = imc->SwitchInputMethod(InputMethodSwitchTest::extBundleName, InputMethodSwitchTest::extAbilityName);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    std::unique_lock<std::mutex> lock(InputMethodSwitchTest::imeChangeFlagLock);
    InputMethodSwitchTest::conditionVar.wait_for(
        lock, std::chrono::seconds(DEALY_TIME), [] { return InputMethodSwitchTest::imeChangeFlag == true; });
    std::shared_ptr<Property> extProperty = imc->GetCurrentInputMethod();
    ASSERT_TRUE(extProperty != nullptr);
    EXPECT_EQ(extProperty->name, InputMethodSwitchTest::extBundleName);
    std::shared_ptr<SubProperty> extSubProperty = imc->GetCurrentInputMethodSubtype();
    ASSERT_TRUE(extSubProperty != nullptr);
    EXPECT_EQ(extSubProperty->label, InputMethodSwitchTest::extAbilityName);

    // Switch to default inputmethod
    ret = imc->SwitchInputMethod(defaultProperty->name, defaultSubProperty->label);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    std::shared_ptr<Property> defaultProperty1 = imc->GetCurrentInputMethod();
    ASSERT_TRUE(defaultProperty1 != nullptr);
    EXPECT_EQ(defaultProperty1->name, defaultProperty->name);
    EXPECT_NE(defaultProperty1->name, InputMethodSwitchTest::extBundleName);
    std::shared_ptr<SubProperty> defaultSubProperty1 = imc->GetCurrentInputMethodSubtype();
    ASSERT_TRUE(defaultSubProperty1 != nullptr);
    EXPECT_EQ(defaultSubProperty1->label, defaultSubProperty->label);
    EXPECT_NE(defaultSubProperty1->label, InputMethodSwitchTest::extAbilityName);
}

/**
* @tc.name: testIMCSwitchInputMethodSelf
* @tc.desc: IMC testSwitchInputMethod.
* @tc.type: FUNC
* @tc.require: issuesI62BHB
* @tc.author: chenyu
*/
HWTEST_F(InputMethodSwitchTest, testIMCSwitchInputMethodSelf, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCSwitchInputMethodSelf Test START");
    sptr<InputMethodController> imc = InputMethodController::GetInstance();
    ASSERT_TRUE(imc != nullptr);

    std::shared_ptr<Property> property = imc->GetCurrentInputMethod();
    ASSERT_TRUE(property != nullptr);
    auto subProperty = imc->GetCurrentInputMethodSubtype();
    ASSERT_TRUE(subProperty != nullptr);

    int32_t ret = imc->SwitchInputMethod(property->name, subProperty->label);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    std::shared_ptr<Property> property1 = imc->GetCurrentInputMethod();
    ASSERT_TRUE(property1 != nullptr);
    EXPECT_EQ(property1->name, property->name);
    auto subProperty1 = imc->GetCurrentInputMethodSubtype();
    ASSERT_TRUE(subProperty1 != nullptr);
    EXPECT_EQ(subProperty1->label, subProperty->label);
}

/**
* @tc.name: testIMCSwitchInputMethodWithErrorBundleName
* @tc.desc: IMC testSwitchInputMethod.
* @tc.type: FUNC
* @tc.require: issuesI62BHB
* @tc.author: chenyu
*/
HWTEST_F(InputMethodSwitchTest, testIMCSwitchInputMethodWithErrorBundleName, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCSwitchInputMethodWithErrorBundleName Test START");
    sptr<InputMethodController> imc = InputMethodController::GetInstance();
    ASSERT_TRUE(imc != nullptr);

    std::shared_ptr<Property> property = imc->GetCurrentInputMethod();
    ASSERT_TRUE(property != nullptr);
    auto subProperty = imc->GetCurrentInputMethodSubtype();
    ASSERT_TRUE(subProperty != nullptr);

    std::string name = "error bundleName";
    int32_t ret = imc->SwitchInputMethod(name, InputMethodSwitchTest::extAbilityName);
    EXPECT_EQ(ret, ErrorCode::ERROR_BAD_PARAMETERS);

    std::shared_ptr<Property> property1 = imc->GetCurrentInputMethod();
    ASSERT_TRUE(property1 != nullptr);
    EXPECT_EQ(property1->name, property->name);
    EXPECT_NE(property1->name, InputMethodSwitchTest::extBundleName);
    auto subProperty1 = imc->GetCurrentInputMethodSubtype();
    ASSERT_TRUE(subProperty1 != nullptr);
    EXPECT_EQ(subProperty1->label, subProperty->label);
    EXPECT_NE(subProperty1->label, InputMethodSwitchTest::extAbilityName);
}

/**
* @tc.name: testIMCSwitchInputMethodWithErrorSubName
* @tc.desc: IMC testSwitchInputMethod.
* @tc.type: FUNC
* @tc.require: issuesI62BHB
* @tc.author: chenyu
*/
HWTEST_F(InputMethodSwitchTest, testIMCSwitchInputMethodWithErrorSubName, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCSwitchInputMethodWithErrorSubName Test START");
    sptr<InputMethodController> imc = InputMethodController::GetInstance();
    ASSERT_TRUE(imc != nullptr);

    std::shared_ptr<Property> property = imc->GetCurrentInputMethod();
    ASSERT_TRUE(property != nullptr);
    auto subProperty = imc->GetCurrentInputMethodSubtype();
    ASSERT_TRUE(subProperty != nullptr);

    std::string subName = "error subName";
    int32_t ret = imc->SwitchInputMethod(InputMethodSwitchTest::extBundleName, subName);
    EXPECT_EQ(ret, ErrorCode::ERROR_SWITCH_IME);

    std::shared_ptr<Property> property1 = imc->GetCurrentInputMethod();
    ASSERT_TRUE(property1 != nullptr);
    EXPECT_EQ(property1->name, property->name);
    EXPECT_NE(property1->name, InputMethodSwitchTest::extBundleName);
    auto subProperty1 = imc->GetCurrentInputMethodSubtype();
    ASSERT_TRUE(subProperty1 != nullptr);
    EXPECT_EQ(subProperty1->label, subProperty->label);
    EXPECT_NE(subProperty1->label, InputMethodSwitchTest::extAbilityName);
}

/**
* @tc.name: testIMCSwitchInputMethodSelfWithoutSubName
* @tc.desc: IMC testSwitchInputMethod.
* @tc.type: FUNC
* @tc.require: issuesI62BHB
* @tc.author: chenyu
*/
HWTEST_F(InputMethodSwitchTest, testIMCSwitchInputMethodSelfWithoutSubName, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCSwitchInputMethodSelfWithoutSubName Test START");
    sptr<InputMethodController> imc = InputMethodController::GetInstance();
    ASSERT_TRUE(imc != nullptr);

    std::shared_ptr<Property> property = imc->GetCurrentInputMethod();
    ASSERT_TRUE(property != nullptr);
    auto subProperty = imc->GetCurrentInputMethodSubtype();
    ASSERT_TRUE(subProperty != nullptr);

    std::string subName;
    int32_t ret = imc->SwitchInputMethod(property->name, subName);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    std::shared_ptr<Property> property1 = imc->GetCurrentInputMethod();
    ASSERT_TRUE(property1 != nullptr);
    EXPECT_EQ(property1->name, property->name);
    EXPECT_NE(property1->name, InputMethodSwitchTest::extBundleName);
    auto subProperty1 = imc->GetCurrentInputMethodSubtype();
    ASSERT_TRUE(subProperty1 != nullptr);
    EXPECT_EQ(subProperty1->label, subProperty->label);
    EXPECT_NE(subProperty1->label, InputMethodSwitchTest::extAbilityName);
}

/**
* @tc.name: testIMCSwitchInputMethodWithErrorBundleNameWithoutSubName
* @tc.desc: IMC testSwitchInputMethod.
* @tc.type: FUNC
* @tc.require: issuesI62BHB
* @tc.author: chenyu
*/
HWTEST_F(InputMethodSwitchTest, testIMCSwitchInputMethodWithErrorBundleNameWithoutSubName, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCSwitchInputMethodWithErrorBundleNameWithoutSubName Test START");
    sptr<InputMethodController> imc = InputMethodController::GetInstance();
    ASSERT_TRUE(imc != nullptr);

    std::shared_ptr<Property> property = imc->GetCurrentInputMethod();
    ASSERT_TRUE(property != nullptr);
    auto subProperty = imc->GetCurrentInputMethodSubtype();
    ASSERT_TRUE(subProperty != nullptr);

    std::string name = "error bundleName";
    std::string subName;
    int32_t ret = imc->SwitchInputMethod(name, subName);
    EXPECT_EQ(ret, ErrorCode::ERROR_SWITCH_IME);

    std::shared_ptr<Property> property1 = imc->GetCurrentInputMethod();
    ASSERT_TRUE(property1 != nullptr);
    EXPECT_EQ(property1->name, property->name);
    EXPECT_NE(property1->name, InputMethodSwitchTest::extBundleName);
    auto subProperty1 = imc->GetCurrentInputMethodSubtype();
    ASSERT_TRUE(subProperty1 != nullptr);
    EXPECT_EQ(subProperty1->label, subProperty->label);
    EXPECT_NE(subProperty1->label, InputMethodSwitchTest::extAbilityName);
}
} // namespace MiscServices
} // namespace OHOS
