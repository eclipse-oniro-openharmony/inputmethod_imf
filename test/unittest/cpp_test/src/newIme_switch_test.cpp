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

#include <string>
#include <vector>

#include "global.h"
#include "ime_event_monitor_manager_impl.h"
#include "ime_setting_listener_test_impl.h"
#include "input_method_controller.h"
#include "input_method_property.h"
#include "tdd_util.h"

using namespace testing::ext;
namespace OHOS {
namespace MiscServices {
class NewImeSwitchTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static void CheckCurrentProp();
    static void CheckCurrentSubProp(const std::string &subName);
    static void CheckCurrentSubProps();
    static sptr<InputMethodController> imc_;
    static std::string bundleName;
    static std::string extName;
    static std::vector<std::string> subName;
    static std::vector<std::string> locale;
    static std::vector<std::string> language;
    static std::string beforeValue;
    static std::string allEnableIme;
};
sptr<InputMethodController> NewImeSwitchTest::imc_;
std::string NewImeSwitchTest::bundleName = "com.example.newTestIme";
std::string NewImeSwitchTest::extName = "InputMethodExtAbility";
std::vector<std::string> NewImeSwitchTest::subName{ "lowerInput", "upperInput", "chineseInput" };
std::vector<std::string> NewImeSwitchTest::locale{ "en-US", "en-US", "zh-CN" };
std::vector<std::string> NewImeSwitchTest::language{ "english", "english", "chinese" };
std::string NewImeSwitchTest::beforeValue;
std::string NewImeSwitchTest::allEnableIme = "{\"enableImeList\" : {\"100\" : [ \"com.example.newTestIme\"]}}";
constexpr uint32_t IME_SUBTYPE_NUM = 3;
constexpr uint32_t WAIT_IME_READY_TIME = 1;
constexpr const char *ENABLE_IME_KEYWORD = "settings.inputmethod.enable_ime";
void NewImeSwitchTest::SetUpTestCase(void)
{
    IMSA_HILOGI("NewImeSwitchTest::SetUpTestCase");
    TddUtil::GrantNativePermission();
    TddUtil::GetEnableData(beforeValue);
    TddUtil::PushEnableImeValue(ENABLE_IME_KEYWORD, allEnableIme);
    TddUtil::StorageSelfTokenID();
    TddUtil::SetTestTokenID(
        TddUtil::AllocTestTokenID(true, "ohos.inputMethod.test", { "ohos.permission.CONNECT_IME_ABILITY" }));
    imc_ = InputMethodController::GetInstance();
    auto listener = std::make_shared<ImeSettingListenerTestImpl>();
    ImeEventMonitorManagerImpl::GetInstance().RegisterImeEventListener(EVENT_IME_CHANGE_MASK, listener);
}

void NewImeSwitchTest::TearDownTestCase(void)
{
    IMSA_HILOGI("NewImeSwitchTest::TearDownTestCase");
    TddUtil::GrantNativePermission();
    TddUtil::PushEnableImeValue(ENABLE_IME_KEYWORD, beforeValue);
    InputMethodController::GetInstance()->Close();
    TddUtil::RestoreSelfTokenID();
}

void NewImeSwitchTest::SetUp(void)
{
    IMSA_HILOGI("NewImeSwitchTest::SetUp");
}

void NewImeSwitchTest::TearDown(void)
{
    IMSA_HILOGI("NewImeSwitchTest::TearDown");
}

void NewImeSwitchTest::CheckCurrentProp()
{
    std::shared_ptr<Property> property = imc_->GetCurrentInputMethod();
    ASSERT_TRUE(property != nullptr);
    EXPECT_EQ(property->name, bundleName);
    EXPECT_EQ(property->id, extName);
}

void NewImeSwitchTest::CheckCurrentSubProp(const std::string &subName)
{
    auto subProperty = imc_->GetCurrentInputMethodSubtype();
    ASSERT_TRUE(subProperty != nullptr);
    EXPECT_EQ(subProperty->id, subName);
    EXPECT_EQ(subProperty->name, bundleName);
}

void NewImeSwitchTest::CheckCurrentSubProps()
{
    std::vector<SubProperty> subProps;
    auto ret = imc_->ListCurrentInputMethodSubtype(subProps);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ASSERT_EQ(subProps.size(), IME_SUBTYPE_NUM);
    for (uint32_t i = 0; i < IME_SUBTYPE_NUM; i++) {
        EXPECT_EQ(subProps[i].id, subName[i]);
        EXPECT_EQ(subProps[i].name, bundleName);
        EXPECT_EQ(subProps[i].language, language[i]);
        EXPECT_EQ(subProps[i].locale, locale[i]);
    }
}

/**
* @tc.name: testNewImeSwitch
* @tc.desc: switch ime to newTestIme.
* @tc.type: FUNC
* @tc.require:
* @tc.author: chenyu
*/
HWTEST_F(NewImeSwitchTest, testNewImeSwitch, TestSize.Level0)
{
    IMSA_HILOGI("newIme testNewImeSwitch Test START");
    ImeSettingListenerTestImpl::ResetParam();
    // switch to newTestIme
    auto ret = imc_->SwitchInputMethod(SwitchTrigger::CURRENT_IME, bundleName);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(ImeSettingListenerTestImpl::WaitImeChange());
    CheckCurrentProp();
    CheckCurrentSubProp(subName[0]);
    CheckCurrentSubProps();
    sleep(WAIT_IME_READY_TIME);
}

/**
* @tc.name: testSubTypeSwitch_001
* @tc.desc: switch subtype with subName1.
* @tc.type: FUNC
* @tc.require:
* @tc.author: chenyu
*/
HWTEST_F(NewImeSwitchTest, testSubTypeSwitch_001, TestSize.Level0)
{
    IMSA_HILOGI("newIme testSubTypeSwitch_001 Test START");
    ImeSettingListenerTestImpl::ResetParam();
    int32_t ret = imc_->SwitchInputMethod(SwitchTrigger::CURRENT_IME, bundleName, subName[0]);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_FALSE(ImeSettingListenerTestImpl::WaitImeChange());
    CheckCurrentProp();
    CheckCurrentSubProp(subName[0]);
    CheckCurrentSubProps();
}

/**
* @tc.name: testSubTypeSwitch_002
* @tc.desc: switch subtype with subName2.
* @tc.type: FUNC
* @tc.require:
* @tc.author: chenyu
*/
HWTEST_F(NewImeSwitchTest, testSubTypeSwitch_002, TestSize.Level0)
{
    IMSA_HILOGI("newIme testSubTypeSwitch_002 Test START");
    ImeSettingListenerTestImpl::ResetParam();
    int32_t ret = imc_->SwitchInputMethod(SwitchTrigger::CURRENT_IME, bundleName, subName[1]);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(ImeSettingListenerTestImpl::WaitImeChange());
    CheckCurrentProp();
    CheckCurrentSubProp(subName[1]);
    CheckCurrentSubProps();
}

/**
* @tc.name: testSubTypeSwitch_003
* @tc.desc: switch subtype with subName3.
* @tc.type: FUNC
* @tc.require:
* @tc.author: chenyu
*/
HWTEST_F(NewImeSwitchTest, testSubTypeSwitch_003, TestSize.Level0)
{
    IMSA_HILOGI("newIme testSubTypeSwitch_003 Test START");
    ImeSettingListenerTestImpl::ResetParam();
    int32_t ret = imc_->SwitchInputMethod(SwitchTrigger::CURRENT_IME, bundleName, subName[2]);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(ImeSettingListenerTestImpl::WaitImeChange());
    CheckCurrentProp();
    CheckCurrentSubProp(subName[2]);
    CheckCurrentSubProps();
}

/**
* @tc.name: testSubTypeSwitch_004
* @tc.desc: switch subtype witch subName1.
* @tc.type: FUNC
* @tc.require:
* @tc.author: chenyu
*/
HWTEST_F(NewImeSwitchTest, testSubTypeSwitch_004, TestSize.Level0)
{
    IMSA_HILOGI("newIme testSubTypeSwitch_004 Test START");
    ImeSettingListenerTestImpl::ResetParam();
    int32_t ret = imc_->SwitchInputMethod(SwitchTrigger::CURRENT_IME, bundleName, subName[0]);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(ImeSettingListenerTestImpl::WaitImeChange());
    CheckCurrentProp();
    CheckCurrentSubProp(subName[0]);
    CheckCurrentSubProps();
}

/**
* @tc.name: testSubTypeSwitchWithErrorSubName
* @tc.desc: switch subtype with error subName.
* @tc.type: FUNC
* @tc.require:
* @tc.author: chenyu
*/
HWTEST_F(NewImeSwitchTest, testSubTypeSwitchWithErrorSubName, TestSize.Level0)
{
    IMSA_HILOGI("newIme testSubTypeSwitchWithErrorSubName Test START");
    int32_t ret = imc_->SwitchInputMethod(SwitchTrigger::CURRENT_IME, bundleName, "errorSubName");
    EXPECT_EQ(ret, ErrorCode::ERROR_BAD_PARAMETERS);
    CheckCurrentProp();
    CheckCurrentSubProp(subName[0]);
    CheckCurrentSubProps();
}

/**
* @tc.name: testSwitchToCurrentImeWithEmptySubName
* @tc.desc: switch to currentIme witch empty subName.
* @tc.type: FUNC
* @tc.require:
* @tc.author: chenyu
*/
HWTEST_F(NewImeSwitchTest, testSwitchToCurrentImeWithEmptySubName, TestSize.Level0)
{
    IMSA_HILOGI("newIme testSwitchToCurrentImeWithEmptySubName Test START");
    ImeSettingListenerTestImpl::ResetParam();
    int32_t ret = imc_->SwitchInputMethod(SwitchTrigger::CURRENT_IME, bundleName);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_FALSE(ImeSettingListenerTestImpl::WaitImeChange());
    CheckCurrentProp();
    CheckCurrentSubProp(subName[0]);
    CheckCurrentSubProps();
}

/**
* @tc.name: testSwitchInputMethod_001
* @tc.desc: switch ime to newTestIme and switch the subtype to subName1.
* @tc.type: FUNC
* @tc.require:
* @tc.author: weishaoxiong
*/
HWTEST_F(NewImeSwitchTest, testSwitchInputMethod_001, TestSize.Level0)
{
    IMSA_HILOGI("newIme testSwitchInputMethod_001 Test START");
    ImeSettingListenerTestImpl::ResetParam();
    auto ret = imc_->SwitchInputMethod(SwitchTrigger::SYSTEM_APP, bundleName, subName[1]);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(ImeSettingListenerTestImpl::WaitImeChange());
    CheckCurrentProp();
    CheckCurrentSubProp(subName[1]);
    CheckCurrentSubProps();
}

/**
* @tc.name: testSwitchInputMethod_002
* @tc.desc: switch the subtype to subName0.
* @tc.type: FUNC
* @tc.require:
* @tc.author: weishaoxiong
*/
HWTEST_F(NewImeSwitchTest, testSwitchInputMethod_002, TestSize.Level0)
{
    IMSA_HILOGI("newIme testSwitchInputMethod_002 Test START");
    ImeSettingListenerTestImpl::ResetParam();
    auto ret = imc_->SwitchInputMethod(SwitchTrigger::SYSTEM_APP, bundleName, subName[0]);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(ImeSettingListenerTestImpl::WaitImeChange());
    CheckCurrentProp();
    CheckCurrentSubProp(subName[0]);
    CheckCurrentSubProps();
}

/**
* @tc.name: testSwitchInputMethod_003
* @tc.desc: switch ime to newTestIme.
* @tc.type: FUNC
* @tc.require:
* @tc.author: weishaoxiong
*/
HWTEST_F(NewImeSwitchTest, testSwitchInputMethod_003, TestSize.Level0)
{
    IMSA_HILOGI("newIme testSwitchInputMethod_003 Test START");
    auto ret = imc_->SwitchInputMethod(SwitchTrigger::SYSTEM_APP, bundleName);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    CheckCurrentProp();
    CheckCurrentSubProp(subName[0]);
    CheckCurrentSubProps();
}

/**
* @tc.name: testSwitchInputMethod_004
* @tc.desc: The caller is not a system app.
* @tc.type: FUNC
* @tc.require:
* @tc.author: weishaoxiong
*/
HWTEST_F(NewImeSwitchTest, testSwitchInputMethod_004, TestSize.Level0)
{
    TddUtil::SetTestTokenID(
        TddUtil::AllocTestTokenID(false, "ohos.inputMethod.test", { "ohos.permission.CONNECT_IME_ABILITY" }));
    IMSA_HILOGI("newIme testSwitchInputMethod_004 Test START");
    auto ret = imc_->SwitchInputMethod(SwitchTrigger::SYSTEM_APP, bundleName);
    EXPECT_EQ(ret, ErrorCode::ERROR_STATUS_SYSTEM_PERMISSION);
}

/**
* @tc.name: testSwitchInputMethod_005
* @tc.desc: The caller has no permissions.
* @tc.type: FUNC
* @tc.require:
* @tc.author: weishaoxiong
*/
HWTEST_F(NewImeSwitchTest, testSwitchInputMethod_005, TestSize.Level0)
{
    TddUtil::SetTestTokenID(TddUtil::AllocTestTokenID(true, "ohos.inputMethod.test", {}));
    IMSA_HILOGI("newIme testSwitchInputMethod_005 Test START");
    auto ret = imc_->SwitchInputMethod(SwitchTrigger::SYSTEM_APP, bundleName);
    EXPECT_EQ(ret, ErrorCode::ERROR_STATUS_PERMISSION_DENIED);
}
} // namespace MiscServices
} // namespace OHOS
