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
class NewImeSwitchTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static void GrantNativePermission();
    static void CheckCurrentProp();
    static void CheckCurrentSubProp(const std::string &subName);
    static void CheckCurrentSubProps();
    static std::mutex imeChangeFlagLock;
    static std::condition_variable conditionVar;
    static bool imeChangeFlag;
    static sptr<InputMethodController> imc_;
    static std::string bundleName;
    static std::string extName;
    static std::vector<std::string> subName;
    static std::vector<std::string> locale;
    static std::vector<std::string> language;
};
std::mutex NewImeSwitchTest::imeChangeFlagLock;
std::condition_variable NewImeSwitchTest::conditionVar;
bool NewImeSwitchTest::imeChangeFlag = false;
sptr<InputMethodController> NewImeSwitchTest::imc_;
std::string NewImeSwitchTest::bundleName = "com.example.newTestIme";
std::string NewImeSwitchTest::extName = "InputMethodExtAbility";
std::vector<std::string> NewImeSwitchTest::subName{ "lowerInput", "upperInput", "chineseInput" };
std::vector<std::string> NewImeSwitchTest::locale{ "en-US", "en-US", "zh-CN" };
std::vector<std::string> NewImeSwitchTest::language{ "english", "english", "chinese" };
constexpr uint32_t IME_SUBTYPE_NUM = 3;
constexpr uint32_t SUBTYPE_SWITCH_DELAY_TIME = 10;
constexpr uint32_t IME_SWITCH_DELAY_TIME = 200;
constexpr uint32_t WAIT_IME_READY_TIME = 1;
class InputMethodSettingListenerImpl : public InputMethodSettingListener {
public:
    InputMethodSettingListenerImpl() = default;
    ~InputMethodSettingListenerImpl() = default;
    void OnImeChange(const Property &property, const SubProperty &subProperty)
    {
        {
            std::unique_lock<std::mutex> lock(NewImeSwitchTest::imeChangeFlagLock);
            NewImeSwitchTest::imeChangeFlag = true;
        }
        NewImeSwitchTest::conditionVar.notify_one();
        IMSA_HILOGI("InputMethodSettingListenerImpl OnImeChange");
    }
};
void NewImeSwitchTest::SetUpTestCase(void)
{
    IMSA_HILOGI("NewImeSwitchTest::SetUpTestCase");
    GrantNativePermission();
    imc_ = InputMethodController::GetInstance();
    imc_->SetSettingListener(std::make_shared<InputMethodSettingListenerImpl>());
}

void NewImeSwitchTest::TearDownTestCase(void)
{
    IMSA_HILOGI("NewImeSwitchTest::TearDownTestCase");
    InputMethodController::GetInstance()->Close();
}

void NewImeSwitchTest::SetUp(void)
{
    IMSA_HILOGI("NewImeSwitchTest::SetUp");
}

void NewImeSwitchTest::TearDown(void)
{
    IMSA_HILOGI("NewImeSwitchTest::TearDown");
}

void NewImeSwitchTest::GrantNativePermission()
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
    std::unique_lock<std::mutex> lock(imeChangeFlagLock);
    imeChangeFlag = false;
    // switch to newTestIme
    auto ret = imc_->SwitchInputMethod(bundleName);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    conditionVar.wait_for(lock, std::chrono::milliseconds(IME_SWITCH_DELAY_TIME), [] { return imeChangeFlag == true; });
    EXPECT_TRUE(imeChangeFlag);
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
    std::unique_lock<std::mutex> lock(imeChangeFlagLock);
    imeChangeFlag = false;
    int32_t ret = imc_->SwitchInputMethod(bundleName, subName[0]);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    conditionVar.wait_for(
        lock, std::chrono::milliseconds(SUBTYPE_SWITCH_DELAY_TIME), [] { return imeChangeFlag == true; });
    EXPECT_FALSE(imeChangeFlag);
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
    std::unique_lock<std::mutex> lock(imeChangeFlagLock);
    imeChangeFlag = false;
    int32_t ret = imc_->SwitchInputMethod(bundleName, subName[1]);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    conditionVar.wait_for(
        lock, std::chrono::milliseconds(SUBTYPE_SWITCH_DELAY_TIME), [] { return imeChangeFlag == true; });
    EXPECT_TRUE(imeChangeFlag);
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
    std::unique_lock<std::mutex> lock(imeChangeFlagLock);
    imeChangeFlag = false;
    int32_t ret = imc_->SwitchInputMethod(bundleName, subName[2]);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    conditionVar.wait_for(
        lock, std::chrono::milliseconds(SUBTYPE_SWITCH_DELAY_TIME), [] { return imeChangeFlag == true; });
    EXPECT_TRUE(imeChangeFlag);
    EXPECT_TRUE(imeChangeFlag);
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
    std::unique_lock<std::mutex> lock(imeChangeFlagLock);
    imeChangeFlag = false;
    int32_t ret = imc_->SwitchInputMethod(bundleName, subName[0]);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    conditionVar.wait_for(
        lock, std::chrono::milliseconds(SUBTYPE_SWITCH_DELAY_TIME), [] { return imeChangeFlag == true; });
    EXPECT_TRUE(imeChangeFlag);
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
    int32_t ret = imc_->SwitchInputMethod(bundleName, "errorSubName");
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
    std::unique_lock<std::mutex> lock(imeChangeFlagLock);
    imeChangeFlag = false;
    int32_t ret = imc_->SwitchInputMethod(bundleName);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    conditionVar.wait_for(
        lock, std::chrono::milliseconds(SUBTYPE_SWITCH_DELAY_TIME), [] { return imeChangeFlag == true; });
    EXPECT_FALSE(imeChangeFlag);
    CheckCurrentProp();
    CheckCurrentSubProp(subName[0]);
    CheckCurrentSubProps();
}
} // namespace MiscServices
} // namespace OHOS
