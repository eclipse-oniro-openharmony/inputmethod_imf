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
#include <sys/time.h>
#include <unistd.h>

#include <condition_variable>
#include <string>
#include <vector>

#include "global.h"
#include "ime_info_inquirer.h"
#include "ime_setting_listener_test_impl.h"
#include "input_method_controller.h"
#include "input_method_property.h"
#include "tdd_util.h"

using namespace testing::ext;
namespace OHOS {
namespace MiscServices {
class InputMethodSwitchTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static void CheckCurrentProp(const std::string &extName);
    static void CheckCurrentSubProp(const std::string &extName);
    static void CheckCurrentSubProps();
    static sptr<InputMethodController> imc_;
    static std::string newImeBundleName;
    static std::vector<std::string> newImeSubName;
    static std::string bundleName;
    static std::vector<std::string> extName;
    static std::vector<std::string> language;
    static std::vector<std::string> locale;
    static bool enableOn;
    static std::string beforeValue;
    static std::string allEnableIme;
};
sptr<InputMethodController> InputMethodSwitchTest::imc_;
std::string InputMethodSwitchTest::newImeBundleName = "com.example.newTestIme";
std::vector<std::string> InputMethodSwitchTest::newImeSubName{ "lowerInput", "upperInput", "chineseInput" };
std::string InputMethodSwitchTest::bundleName = "com.example.testIme";
std::vector<std::string> InputMethodSwitchTest::extName{ "InputMethodExtAbility", "InputMethodExtAbility2" };
std::vector<std::string> InputMethodSwitchTest::language{ "chinese", "english" };
std::vector<std::string> InputMethodSwitchTest::locale{ "zh-CN", "en-US" };
bool InputMethodSwitchTest::enableOn = false;
std::string InputMethodSwitchTest::beforeValue;
std::string InputMethodSwitchTest::allEnableIme = "{\"enableImeList\" : {\"100\" : [ \"com.example.newTestIme\", "
                                                  "\"com.example.testIme\"]}}";
constexpr uint32_t IME_EXT_NUM = 2;
constexpr uint32_t NEW_IME_SUBTYPE_NUM = 3;
constexpr uint32_t TOTAL_IME_MIN_NUM = 2;
constexpr uint32_t ENABLE_IME_NUM = 1;
constexpr uint32_t WAIT_IME_READY_TIME = 1;
constexpr const char *ENABLE_IME_KEYWORD = "settings.inputmethod.enable_ime";
void InputMethodSwitchTest::SetUpTestCase(void)
{
    IMSA_HILOGI("InputMethodSwitchTest::SetUpTestCase");
    ImeInfoInquirer::GetInstance().InitSystemConfig();
    enableOn = ImeInfoInquirer::GetInstance().IsEnableInputMethod();
    TddUtil::GrantNativePermission();
    if (enableOn == true) {
        IMSA_HILOGI("Enable ime switch test.");
        int32_t ret = TddUtil::GetEnableData(beforeValue);
        if (ret == ErrorCode::NO_ERROR) {
            TddUtil::PushEnableImeValue(ENABLE_IME_KEYWORD, allEnableIme);
        }
    }
    TddUtil::StorageSelfTokenID();
    TddUtil::SetTestTokenID(
        TddUtil::AllocTestTokenID(true, "ohos.inputMethod.test", { "ohos.permission.CONNECT_IME_ABILITY" }));
    imc_ = InputMethodController::GetInstance();
    auto listener = std::make_shared<ImeSettingListenerTestImpl>();
    ImeEventMonitorManager::GetInstance().RegisterImeEventListener({ EventType::IME_CHANGE }, listener);
}

void InputMethodSwitchTest::TearDownTestCase(void)
{
    IMSA_HILOGI("InputMethodSwitchTest::TearDownTestCase");
    if (enableOn) {
        TddUtil::GrantNativePermission();
        TddUtil::PushEnableImeValue(ENABLE_IME_KEYWORD, beforeValue);
    }
    InputMethodController::GetInstance()->Close();
    TddUtil::RestoreSelfTokenID();
}

void InputMethodSwitchTest::SetUp(void)
{
    IMSA_HILOGI("InputMethodSwitchTest::SetUp");
}

void InputMethodSwitchTest::TearDown(void)
{
    IMSA_HILOGI("InputMethodSwitchTest::TearDown");
}

void InputMethodSwitchTest::CheckCurrentProp(const std::string &extName)
{
    std::shared_ptr<Property> property = imc_->GetCurrentInputMethod();
    ASSERT_TRUE(property != nullptr);
    EXPECT_EQ(property->name, bundleName);
    EXPECT_EQ(property->id, extName);
}

void InputMethodSwitchTest::CheckCurrentSubProp(const std::string &extName)
{
    auto subProperty = imc_->GetCurrentInputMethodSubtype();
    ASSERT_TRUE(subProperty != nullptr);
    EXPECT_EQ(subProperty->id, extName);
    EXPECT_EQ(subProperty->name, bundleName);
}

void InputMethodSwitchTest::CheckCurrentSubProps()
{
    std::vector<SubProperty> subProps;
    auto ret = imc_->ListCurrentInputMethodSubtype(subProps);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ASSERT_EQ(subProps.size(), IME_EXT_NUM);
    for (uint32_t i = 0; i < IME_EXT_NUM; i++) {
        EXPECT_EQ(subProps[i].id, extName[i]);
        EXPECT_EQ(subProps[i].name, bundleName);
        EXPECT_EQ(subProps[i].language, language[i]);
        EXPECT_EQ(subProps[i].locale, locale[i]);
    }
}

/**
* @tc.name: testImeSwitch
* @tc.desc: switch to testIme
* @tc.type: FUNC
* @tc.require: issuesI62BHB
* @tc.author: chenyu
*/
HWTEST_F(InputMethodSwitchTest, testImeSwitch, TestSize.Level0)
{
    IMSA_HILOGI("oldIme testImeSwitch Test START");
    ImeSettingListenerTestImpl::ResetParam();
    // switch to ext testIme
    auto ret = imc_->SwitchInputMethod(SwitchTrigger::CURRENT_IME, bundleName);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(ImeSettingListenerTestImpl::WaitImeChange());
    CheckCurrentProp(extName[0]);
    CheckCurrentSubProp(extName[0]);
    CheckCurrentSubProps();
    sleep(WAIT_IME_READY_TIME);
}

/**
* @tc.name: testSubTypeSwitch_001
* @tc.desc: switch subtype with extName1
* @tc.type: FUNC
* @tc.require: issuesI62BHB
* @tc.author: chenyu
*/
HWTEST_F(InputMethodSwitchTest, testSubTypeSwitch_001, TestSize.Level0)
{
    IMSA_HILOGI("oldIme testSubTypeSwitch_001 Test START");
    ImeSettingListenerTestImpl::ResetParam();
    int32_t ret = imc_->SwitchInputMethod(SwitchTrigger::CURRENT_IME, bundleName, extName[0]);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_FALSE(ImeSettingListenerTestImpl::WaitImeChange());
    CheckCurrentProp(extName[0]);
    CheckCurrentSubProp(extName[0]);
    CheckCurrentSubProps();
}

/**
* @tc.name: testSubTypeSwitch_002
* @tc.desc: switch subtype with extName2
* @tc.type: FUNC
* @tc.require: issuesI62BHB
* @tc.author: chenyu
*/
HWTEST_F(InputMethodSwitchTest, testSubTypeSwitch_002, TestSize.Level0)
{
    IMSA_HILOGI("oldIme testSubTypeSwitch_002 Test START");
    ImeSettingListenerTestImpl::ResetParam();
    int32_t ret = imc_->SwitchInputMethod(SwitchTrigger::CURRENT_IME, bundleName, extName[1]);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(ImeSettingListenerTestImpl::WaitImeChange());
    CheckCurrentProp(extName[0]);
    CheckCurrentSubProp(extName[1]);
    CheckCurrentSubProps();
}

/**
* @tc.name: testSubTypeSwitch_003
* @tc.desc: switch subtype with extName1
* @tc.type: FUNC
* @tc.require: issuesI62BHB
* @tc.author: chenyu
*/
HWTEST_F(InputMethodSwitchTest, testSubTypeSwitch_003, TestSize.Level0)
{
    IMSA_HILOGI("oldIme testSubTypeSwitch_003 Test START");
    ImeSettingListenerTestImpl::ResetParam();
    int32_t ret = imc_->SwitchInputMethod(SwitchTrigger::CURRENT_IME, bundleName, extName[0]);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(ImeSettingListenerTestImpl::WaitImeChange());
    CheckCurrentProp(extName[0]);
    CheckCurrentSubProp(extName[0]);
    CheckCurrentSubProps();
}

/**
* @tc.name: testSubTypeSwitchWithErrorSubName
* @tc.desc: switch subtype with error subName.
* @tc.type: FUNC
* @tc.require: issuesI62BHB
* @tc.author: chenyu
*/
HWTEST_F(InputMethodSwitchTest, testSubTypeSwitchWithErrorSubName, TestSize.Level0)
{
    IMSA_HILOGI("oldIme testSubTypeSwitchWithErrorSubName Test START");
    std::string subName = InputMethodSwitchTest::imc_->GetCurrentInputMethodSubtype()->id;
    int32_t ret = imc_->SwitchInputMethod(SwitchTrigger::CURRENT_IME, bundleName, "error subName");
    EXPECT_EQ(ret, ErrorCode::ERROR_BAD_PARAMETERS);
    CheckCurrentProp(subName);
    CheckCurrentSubProp(subName);
    CheckCurrentSubProps();
}

/**
* @tc.name: testSwitchToCurrentImeWithEmptySubName
* @tc.desc: switch to currentIme witch empty subName.
* @tc.type: FUNC
* @tc.require: issuesI62BHB
* @tc.author: chenyu
*/
HWTEST_F(InputMethodSwitchTest, testSwitchToCurrentImeWithEmptySubName, TestSize.Level0)
{
    IMSA_HILOGI("oldIme testSwitchToCurrentImeWithEmptySubName Test START");
    ImeSettingListenerTestImpl::ResetParam();
    std::string subName = InputMethodSwitchTest::imc_->GetCurrentInputMethodSubtype()->id;
    int32_t ret = imc_->SwitchInputMethod(SwitchTrigger::CURRENT_IME, bundleName);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_FALSE(ImeSettingListenerTestImpl::WaitImeChange());
    CheckCurrentProp(subName);
    CheckCurrentSubProp(subName);
    CheckCurrentSubProps();
}

/**
* @tc.name: testSwitchImeWithErrorBundleName
* @tc.desc: switch ime witch error bundleName
* @tc.type: FUNC
* @tc.require: issuesI62BHB
* @tc.author: chenyu
*/
HWTEST_F(InputMethodSwitchTest, testSwitchImeWithErrorBundleName, TestSize.Level0)
{
    IMSA_HILOGI("oldIme testSwitchImeWithErrorBundleName Test START");
    std::string subName = InputMethodSwitchTest::imc_->GetCurrentInputMethodSubtype()->id;
    int32_t ret = imc_->SwitchInputMethod(SwitchTrigger::CURRENT_IME, "error bundleName", extName[0]);
    if (InputMethodSwitchTest::enableOn) {
        EXPECT_EQ(ret, ErrorCode::ERROR_ENABLE_IME);
    } else {
        EXPECT_EQ(ret, ErrorCode::ERROR_BAD_PARAMETERS);
    }
    CheckCurrentProp(subName);
    CheckCurrentSubProp(subName);
    CheckCurrentSubProps();
}

/**
* @tc.name: testSwitchImeWithErrorBundleNameWitchEmptySubName
* @tc.desc: switch ime witch error bundleName and empty subName
* @tc.type: FUNC
* @tc.require: issuesI62BHB
* @tc.author: chenyu
*/
HWTEST_F(InputMethodSwitchTest, testSwitchImeWithErrorBundleNameWitchEmptySubName, TestSize.Level0)
{
    IMSA_HILOGI("oldIme testSwitchImeWithErrorBundleNameWitchEmptySubName Test START");
    std::string subName = InputMethodSwitchTest::imc_->GetCurrentInputMethodSubtype()->id;
    int32_t ret = imc_->SwitchInputMethod(SwitchTrigger::CURRENT_IME, "error bundleName", " ");
    if (InputMethodSwitchTest::enableOn) {
        EXPECT_EQ(ret, ErrorCode::ERROR_ENABLE_IME);
    } else {
        EXPECT_EQ(ret, ErrorCode::ERROR_BAD_PARAMETERS);
    }
    CheckCurrentProp(subName);
    CheckCurrentSubProp(subName);
    CheckCurrentSubProps();
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
    std::vector<Property> properties = {};
    auto ret = imc_->ListInputMethod(properties);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(properties.size() >= TOTAL_IME_MIN_NUM);
    bool hasIme = false;
    bool hasNewIme = false;
    for (const auto &property : properties) {
        if (property.name == bundleName) {
            hasIme = true;
        }
        if (property.name == newImeBundleName) {
            hasNewIme = true;
        }
    }
    EXPECT_TRUE(hasIme && hasNewIme);
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
    std::vector<Property> disableProperties = {};
    auto ret = imc_->ListInputMethod(false, disableProperties);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_GE(disableProperties.size(), 0);
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
    std::string subName = InputMethodSwitchTest::imc_->GetCurrentInputMethodSubtype()->id;
    std::vector<Property> enableProperties = {};
    auto ret = imc_->ListInputMethod(true, enableProperties);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_GE(enableProperties.size(), ENABLE_IME_NUM);
}

/**
* @tc.name: tesIMCtListInputMethodSubtype_001
* @tc.desc: ListInputMethodSubtype
* @tc.type: FUNC
* @tc.require: issuesI62BHB
* @tc.author: chenyu
*/
HWTEST_F(InputMethodSwitchTest, tesIMCtListInputMethodSubtype_001, TestSize.Level0)
{
    IMSA_HILOGI("IMC tesIMCtListInputMethodSubtype_001 Test Start");
    Property property = { .name = newImeBundleName };
    std::vector<SubProperty> subProps;
    auto ret = imc_->ListInputMethodSubtype(property, subProps);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ASSERT_EQ(subProps.size(), NEW_IME_SUBTYPE_NUM);
    for (uint32_t i = 0; i < NEW_IME_SUBTYPE_NUM; i++) {
        EXPECT_EQ(subProps[i].id, newImeSubName[i]);
        EXPECT_EQ(subProps[i].name, newImeBundleName);
    }
}

/**
* @tc.name: tesIMCtListInputMethodSubtype_002
* @tc.desc: ListInputMethodSubtype
* @tc.type: FUNC
* @tc.require: issuesI62BHB
* @tc.author: chenyu
*/
HWTEST_F(InputMethodSwitchTest, tesIMCtListInputMethodSubtype_002, TestSize.Level0)
{
    IMSA_HILOGI("IMC tesIMCtListInputMethodSubtype_002 Test Start");
    Property property = { .name = bundleName };
    std::vector<SubProperty> subProps;
    auto ret = imc_->ListInputMethodSubtype(property, subProps);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ASSERT_EQ(subProps.size(), IME_EXT_NUM);
    for (uint32_t i = 0; i < IME_EXT_NUM; i++) {
        EXPECT_EQ(subProps[i].id, extName[i]);
        EXPECT_EQ(subProps[i].name, bundleName);
    }
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
    auto ret = imc_->ListInputMethodSubtype(*property, properties);
    EXPECT_EQ(ret, ErrorCode::ERROR_BAD_PARAMETERS);
    EXPECT_TRUE(properties.empty());
}

/**
* @tc.name: testShowOptionalInputMethod
* @tc.desc: IMC ShowOptionalInputMethod
* @tc.type: FUNC
*/
HWTEST_F(InputMethodSwitchTest, testShowOptionalInputMethod, TestSize.Level2)
{
    IMSA_HILOGI("IMC ShowOptionalInputMethod Test START");
    int32_t ret = imc_->ShowOptionalInputMethod();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
* @tc.name: testDisplayOptionalInputMethod
* @tc.desc: IMC DisplayOptionalInputMethod
* @tc.type: FUNC
*/
HWTEST_F(InputMethodSwitchTest, testDisplayOptionalInputMethod, TestSize.Level2)
{
    IMSA_HILOGI("IMC DisplayOptionalInputMethod Test START");
    sleep(2);
    int32_t ret = imc_->DisplayOptionalInputMethod();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
* @tc.name: testCombinationKeySwitchIme_001
* @tc.desc: switch ime by combination key.
* @tc.type: FUNC
* @tc.require: issuesI8RPP3
* @tc.author: mashaoyin
*/
HWTEST_F(InputMethodSwitchTest, testCombinationKeySwitchIme_001, TestSize.Level0)
{
    IMSA_HILOGI("testCombinationKeySwitchIme_001 Test START");
    ImeSettingListenerTestImpl::ResetParam();
    std::shared_ptr<Property> property = imc_->GetCurrentInputMethod();
    std::string result;
    static std::string cmd = "uinput -K -d 2076 -d 2050";
    auto ret = TddUtil::ExecuteCmd(cmd, result);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(ImeSettingListenerTestImpl::WaitImeChange());
    imc_->SwitchInputMethod(SwitchTrigger::CURRENT_IME, property->name, "");
}

/**
* @tc.name: testCombinationKeySwitchIme_002
* @tc.desc: switch ime by combination key.
* @tc.type: FUNC
* @tc.require: issuesI8RPP3
* @tc.author: mashaoyin
*/
HWTEST_F(InputMethodSwitchTest, testCombinationKeySwitchIme_002, TestSize.Level0)
{
    IMSA_HILOGI("testCombinationKeySwitchIme_002 Test START");
    ImeSettingListenerTestImpl::ResetParam();
    std::shared_ptr<Property> property = imc_->GetCurrentInputMethod();
    std::string result;
    static std::string cmd = "uinput -K -d 2077 -d 2050";
    auto ret = TddUtil::ExecuteCmd(cmd, result);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(ImeSettingListenerTestImpl::WaitImeChange());
    imc_->SwitchInputMethod(SwitchTrigger::CURRENT_IME, property->name, "");
}
} // namespace MiscServices
} // namespace OHOS
