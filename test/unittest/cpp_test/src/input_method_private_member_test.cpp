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
#define private public
#define protected public
#include "ime_cfg_manager.h"
#include "ime_info_inquirer.h"
#include "input_method_controller.h"
#include "input_method_system_ability.h"
#include "peruser_session.h"
#include "wms_connection_observer.h"
#undef private
#include <gtest/gtest.h>
#include <sys/time.h>
#include <unistd.h>

#include <string>
#include <vector>

#include "application_info.h"
#include "global.h"
#include "i_input_method_agent.h"
#include "i_input_method_core.h"
#include "ime_cfg_manager.h"
#include "input_method_agent_proxy.h"
#include "input_method_agent_stub.h"
#include "input_method_core_stub.h"
#include "keyboard_event.h"
#include "os_account_manager.h"
#include "tdd_util.h"

using namespace testing::ext;
namespace OHOS {
namespace MiscServices {
constexpr uint32_t MAX_SUBTYPE_NUM = 256;
class InputMethodPrivateMemberTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static sptr<InputMethodSystemAbility> service_;
};
constexpr std::int32_t MAIN_USER_ID = 100;
void InputMethodPrivateMemberTest::SetUpTestCase(void)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest::SetUpTestCase");
    service_ = new InputMethodSystemAbility();
    service_->OnStart();
}

void InputMethodPrivateMemberTest::TearDownTestCase(void)
{
    service_->OnStop();
    TddUtil::KillImsaProcess();
    IMSA_HILOGI("InputMethodPrivateMemberTest::TearDownTestCase");
}

void InputMethodPrivateMemberTest::SetUp(void)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest::SetUp");
    ImeCfgManager::GetInstance().imeConfigs_.clear();
    ImeInfoInquirer::GetInstance().SetCurrentImeInfo(nullptr);
}

void InputMethodPrivateMemberTest::TearDown(void)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest::TearDown");
    ImeCfgManager::GetInstance().imeConfigs_.clear();
    ImeInfoInquirer::GetInstance().SetCurrentImeInfo(nullptr);
}
sptr<InputMethodSystemAbility> InputMethodPrivateMemberTest::service_;

/**
* @tc.name: SA_TestOnStart
* @tc.desc: SA OnStart.
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(InputMethodPrivateMemberTest, SA_TestOnStart, TestSize.Level0)
{
    InputMethodSystemAbility ability;
    ability.state_ = ServiceRunningState::STATE_RUNNING;
    ability.OnStart();
    EXPECT_EQ(ability.userSession_, nullptr);
}

/**
* @tc.name: SA_GetExtends
* @tc.desc: SA GetExtends.
* @tc.type: FUNC
* @tc.require: issuesI640YZ
*/
HWTEST_F(InputMethodPrivateMemberTest, SA_GetExtends, TestSize.Level0)
{
    constexpr int32_t metaDataNums = 5;
    ImeInfoInquirer inquirer;
    std::vector<Metadata> metaData;
    Metadata metadata[metaDataNums] = { { "language", "english", "" }, { "mode", "mode", "" },
        { "locale", "local", "" }, { "icon", "icon", "" }, { "", "", "" } };
    for (auto const &data : metadata) {
        metaData.emplace_back(data);
    }
    auto subProperty = inquirer.GetExtends(metaData);
    EXPECT_EQ(subProperty.language, "english");
    EXPECT_EQ(subProperty.mode, "mode");
    EXPECT_EQ(subProperty.locale, "local");
    EXPECT_EQ(subProperty.icon, "icon");
}

/**
* @tc.name: SA_TestOnPackageRemoved
* @tc.desc: SA_TestOnPackageRemoved
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(InputMethodPrivateMemberTest, SA_TestOnPackageRemoved, TestSize.Level0)
{
    // msg is nullptr
    auto *msg = new Message(MessageID::MSG_ID_PACKAGE_REMOVED, nullptr);
    auto ret = service_->OnPackageRemoved(msg);
    EXPECT_EQ(ret, ErrorCode::ERROR_NULL_POINTER);
    MessageHandler::Instance()->SendMessage(msg);

    // PARCELABLE failed
    MessageParcel *parcel1 = new (std::nothrow) MessageParcel();
    auto bundleName = "testBundleName1";
    parcel1->WriteString(bundleName);
    auto msg1 = std::make_shared<Message>(MessageID::MSG_ID_PACKAGE_REMOVED, parcel1);
    auto ret1 = service_->OnPackageRemoved(msg1.get());
    EXPECT_EQ(ret1, ErrorCode::ERROR_EX_PARCELABLE);

    // userId is not same
    auto parcel2 = new (std::nothrow) MessageParcel();
    auto userId = 50;
    service_->userId_ = 60;
    parcel2->WriteInt32(userId);
    parcel2->WriteString(bundleName);
    auto msg2 = std::make_shared<Message>(MessageID::MSG_ID_PACKAGE_REMOVED, parcel2);
    auto ret2 = service_->OnPackageRemoved(msg2.get());
    EXPECT_EQ(ret2, ErrorCode::NO_ERROR);

    //remove bundle not current ime
    auto parcel3 = new (std::nothrow) MessageParcel();
    service_->userId_ = userId;
    ImeCfgManager::GetInstance().imeConfigs_.push_back({ 60, "testBundleName/testExtName", "testSubName" });
    parcel3->WriteInt32(userId);
    parcel3->WriteString(bundleName);
    auto msg3 = std::make_shared<Message>(MessageID::MSG_ID_PACKAGE_REMOVED, parcel3);
    auto ret3 = service_->OnPackageRemoved(msg3.get());
    EXPECT_EQ(ret3, ErrorCode::NO_ERROR);
}

/**
* @tc.name: SA_TestOnUserStarted
* @tc.desc: SA_TestOnUserStarted.
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(InputMethodPrivateMemberTest, SA_TestOnUserStarted, TestSize.Level0)
{
    // msg is nullptr
    auto *msg = new Message(MessageID::MSG_ID_USER_START, nullptr);
    auto ret = service_->OnUserStarted(msg);
    EXPECT_EQ(ret, ErrorCode::ERROR_NULL_POINTER);
    MessageHandler::Instance()->SendMessage(msg);

    // userId is same
    service_->userId_ = 50;
    MessageParcel *parcel1 = new MessageParcel();
    parcel1->WriteInt32(50);
    auto msg1 = std::make_shared<Message>(MessageID::MSG_ID_USER_START, parcel1);
    auto ret1 = service_->OnUserStarted(msg1.get());
    EXPECT_EQ(ret1, ErrorCode::NO_ERROR);

    //start ime failed
    service_->userId_ = 50;
    auto currentUserId = TddUtil::GetCurrentUserId();
    ImeCfgManager::GetInstance().imeConfigs_.push_back({ currentUserId, "testBundleName/testExtName", "testSubName" });
    auto parcel2 = new MessageParcel();
    parcel2->WriteInt32(currentUserId);
    auto msg2 = std::make_shared<Message>(MessageID::MSG_ID_USER_START, parcel2);
    auto ret2 = service_->OnUserStarted(msg2.get());
    EXPECT_EQ(ret2, ErrorCode::ERROR_IME_START_FAILED);
}

/**
* @tc.name: SA_TestOnUserRemoved
* @tc.desc: SA_TestOnUserRemoved.
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(InputMethodPrivateMemberTest, SA_TestOnUserRemoved, TestSize.Level0)
{
    // msg is nullptr
    auto *msg = new Message(MessageID::MSG_ID_USER_REMOVED, nullptr);
    auto ret = service_->OnUserRemoved(msg);
    EXPECT_EQ(ret, ErrorCode::ERROR_NULL_POINTER);
    MessageHandler::Instance()->SendMessage(msg);

    // move userId
    MessageParcel *parcel1 = new MessageParcel();
    parcel1->WriteInt32(60);
    auto msg1 = std::make_shared<Message>(MessageID::MSG_ID_USER_REMOVED, parcel1);
    auto ret1 = service_->OnUserRemoved(msg1.get());
    EXPECT_EQ(ret1, ErrorCode::NO_ERROR);
}

/**
* @tc.name: SA_ListInputMethodInfoWithInexistentUserId
* @tc.desc: SA ListInputMethodInfo With Inexistent UserId.
* @tc.type: FUNC
* @tc.require: issuesI669E8
*/
HWTEST_F(InputMethodPrivateMemberTest, SA_ListInputMethodInfoWithInexistentUserId, TestSize.Level0)
{
    ImeInfoInquirer inquirer;
    constexpr int32_t userId = 1;
    auto inputMethodInfos = inquirer.ListInputMethodInfo(userId);
    EXPECT_TRUE(inputMethodInfos.empty());
}

/**
* @tc.name: IMC_ListInputMethodCommonWithErrorStatus
* @tc.desc: IMC ListInputMethodCommon With Error Status.
* @tc.type: FUNC
* @tc.require: issuesI669E8
*/
HWTEST_F(InputMethodPrivateMemberTest, IMC_ListInputMethodCommonWithErrorStatus, TestSize.Level0)
{
    std::vector<Property> props;
    auto ret = InputMethodController::GetInstance()->ListInputMethodCommon(static_cast<InputMethodStatus>(5), props);
    EXPECT_EQ(ret, ErrorCode::ERROR_BAD_PARAMETERS);
    EXPECT_TRUE(props.empty());
}

/**
 * @tc.name: PerUserSessionCoreOrAgentNullptr
 * @tc.desc: Test PerUserSession with core nullptr.
 * @tc.type: FUNC
 * @tc.require: issuesI794QF
 * @tc.author: Zhaolinglan
 */
HWTEST_F(InputMethodPrivateMemberTest, PerUserSessionCoreOrAgentNullptr, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest PerUserSessionCoreOrAgentNullptr TEST START");
    auto userSession = std::make_shared<PerUserSession>(MAIN_USER_ID);
    auto imc = InputMethodController::GetInstance();
    int32_t ret = userSession->ShowKeyboard(imc->clientInfo_.client);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_FOUND);
    ret = userSession->HideKeyboard(imc->clientInfo_.client);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_FOUND);
    ret = userSession->InitInputControlChannel();
    EXPECT_EQ(ret, ErrorCode::ERROR_IME_NOT_STARTED);
    userSession->StopInputService("", "");
    ret = userSession->SwitchSubtype({});
    EXPECT_EQ(ret, ErrorCode::ERROR_IME_NOT_STARTED);
}

/**
 * @tc.name: PerUserSessionClientError
 * @tc.desc: Test PerUserSession with client error.
 * @tc.type: FUNC
 * @tc.require: issuesI794QF
 * @tc.author: Zhaolinglan
 */
HWTEST_F(InputMethodPrivateMemberTest, PerUserSessionClientError, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest PerUserSessionClientError TEST START");
    auto userSession = std::make_shared<PerUserSession>(MAIN_USER_ID);
    auto imc = InputMethodController::GetInstance();
    sptr<InputMethodCoreStub> core = new InputMethodCoreStub();

    auto clientInfo = userSession->GetClientInfo(imc->clientInfo_.client->AsObject());
    EXPECT_EQ(clientInfo, nullptr);

    userSession->SetCurrentClient(nullptr);
    userSession->OnUnfocused(0, 0);
    int32_t ret = userSession->OnHideCurrentInput();
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_FOUND);
    ret = userSession->OnShowCurrentInput();
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_FOUND);

    userSession->SetCurrentClient(imc->clientInfo_.client);
    ret = userSession->OnShowCurrentInput();
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_FOUND);
}

/**
 * @tc.name: PerUserSessionParameterNullptr001
 * @tc.desc: Test PerUserSession with parameter client nullptr.
 * @tc.type: FUNC
 * @tc.require: issuesI794QF
 * @tc.author: Zhaolinglan
 */
HWTEST_F(InputMethodPrivateMemberTest, PerUserSessionParameterNullptr001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest PerUserSessionParameterNullptr001 TEST START");
    auto userSession = std::make_shared<PerUserSession>(MAIN_USER_ID);
    sptr<IRemoteObject> agent = nullptr;
    InputClientInfo clientInfo;
    int32_t ret = userSession->OnStartInput(clientInfo, agent);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);
    ret = userSession->OnReleaseInput(nullptr);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);
    auto client = userSession->GetClientInfo(nullptr);
    EXPECT_EQ(client, nullptr);
}

/**
 * @tc.name: PerUserSessionParameterNullptr003
 * @tc.desc: Test PerUserSession with parameter nullptr.
 * @tc.type: FUNC
 * @tc.require: issuesI794QF
 * @tc.author: Zhaolinglan
 */
HWTEST_F(InputMethodPrivateMemberTest, PerUserSessionParameterNullptr003, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest PerUserSessionParameterNullptr003 TEST START");
    auto userSession = std::make_shared<PerUserSession>(MAIN_USER_ID);
    sptr<InputMethodCoreStub> core = new InputMethodCoreStub();
    userSession->OnClientDied(nullptr);
    userSession->OnImeDied(nullptr, ImeType::IME);
    bool isShowKeyboard = false;
    userSession->UpdateClientInfo(nullptr, { { UpdateFlag::ISSHOWKEYBOARD, isShowKeyboard } });
    int32_t ret = userSession->RemoveIme(nullptr, ImeType::IME);
    EXPECT_EQ(ret, ErrorCode::ERROR_NULL_POINTER);
}

/**
 * @tc.name: SA_ImeCfgManagerTest_002
 * @tc.desc: getImeCfg failed
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodPrivateMemberTest, SA_ImeCfgManagerTest_002, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest SA_ImeCfgManagerTest_002 TEST START");
    ImeCfgManager cfgManager;
    auto cfg = cfgManager.GetImeCfg(100);
    EXPECT_TRUE(cfg.currentSubName.empty());
    EXPECT_TRUE(cfg.currentIme.empty());
}

/**
 * @tc.name: SA_SwitchByCombinationKey_001
 * @tc.desc: keycode = MMI::KeyEvent::KEYCODE_0
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodPrivateMemberTest, SA_SwitchByCombinationKey_001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest SA_SwitchByCombinationKey_001 TEST START");
    ImeCfgManager cfgManager;
    auto ret = service_->SwitchByCombinationKey(0);
    EXPECT_EQ(ret, ErrorCode::ERROR_EX_UNSUPPORTED_OPERATION);
}

/**
 * @tc.name: SA_SwitchByCombinationKey_002
 * @tc.desc: SwitchLanguage()/SwitchMode():GetImeInfo failed
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodPrivateMemberTest, SA_SwitchByCombinationKey_002, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest SA_SwitchByCombinationKey_002 TEST START");
    ImeCfgManager cfgManager;
    auto ret = service_->SwitchByCombinationKey(KeyboardEvent::SHIFT_RIGHT_MASK);
    EXPECT_EQ(ret, ErrorCode::ERROR_BAD_PARAMETERS);
    ret = service_->SwitchByCombinationKey(KeyboardEvent::CAPS_MASK);
    EXPECT_EQ(ret, ErrorCode::ERROR_BAD_PARAMETERS);
}

/**
 * @tc.name: SA_SwitchByCombinationKey_003
 * @tc.desc: SwitchLanguage()/SwitchMode():is newIme
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodPrivateMemberTest, SA_SwitchByCombinationKey_003, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest SA_SwitchByCombinationKey_003 TEST START");
    ImeCfgManager cfgManager;
    service_->userId_ = 50;
    auto info = std::make_shared<ImeInfo>();
    info->isNewIme = true;
    info->prop = { .name = "testBundleName" };
    info->subProp = { .id = "testSubName" };
    ImeInfoInquirer::GetInstance().SetCurrentImeInfo(info);
    ImeCfgManager::GetInstance().imeConfigs_.push_back({ 50, "testBundleName/testExtName", "testSubName" });
    auto ret = service_->SwitchByCombinationKey(KeyboardEvent::SHIFT_RIGHT_MASK);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = service_->SwitchByCombinationKey(KeyboardEvent::CAPS_MASK);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: SA_SwitchByCombinationKey_004
 * @tc.desc: SwitchLanguage():info.subProp.language == "French"
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodPrivateMemberTest, SA_SwitchByCombinationKey_004, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest SA_SwitchByCombinationKey_004 TEST START");
    service_->userId_ = 50;
    auto info = std::make_shared<ImeInfo>();
    info->prop = { .name = "testBundleName", .id = "testExtName" };
    info->subProp = { .name = "testBundleName", .id = "testSubName", .language = "French" };
    ImeInfoInquirer::GetInstance().SetCurrentImeInfo(info);
    ImeCfgManager::GetInstance().imeConfigs_.push_back({ 50, "testBundleName/testExtName", "testSubName" });
    auto ret = service_->SwitchByCombinationKey(KeyboardEvent::SHIFT_RIGHT_MASK);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: SA_SwitchByCombinationKey_005
 * @tc.desc: SwitchLanguage()/SwitchMode():FindTargetSubtypeByCondition failed
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodPrivateMemberTest, SA_SwitchByCombinationKey_005, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest SA_SwitchByCombinationKey_005 TEST START");
    service_->userId_ = 50;
    auto info = std::make_shared<ImeInfo>();
    info->prop = { .name = "testBundleName", .id = "testExtName" };
    info->subProp = {
        .name = "testBundleName",
        .id = "testSubName",
        .mode = "upper",
        .language = "english",
    };
    info->subProps = { { .name = "testBundleName", .id = "testSubName", .mode = "upper", .language = "english" } };
    ImeInfoInquirer::GetInstance().SetCurrentImeInfo(info);
    ImeCfgManager::GetInstance().imeConfigs_.push_back({ 50, "testBundleName/testExtName", "testSubName" });
    auto ret = service_->SwitchByCombinationKey(KeyboardEvent::SHIFT_RIGHT_MASK);
    EXPECT_EQ(ret, ErrorCode::ERROR_BAD_PARAMETERS);
    ret = service_->SwitchByCombinationKey(KeyboardEvent::CAPS_MASK);
    EXPECT_EQ(ret, ErrorCode::ERROR_BAD_PARAMETERS);
}

/**
 * @tc.name: SA_SwitchByCombinationKey_006
 * @tc.desc: SwitchLanguage()/SwitchMode():StartInputService() failed
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodPrivateMemberTest, SA_SwitchByCombinationKey_006, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest SA_SwitchByCombinationKey_006 TEST START");
    service_->userId_ = 50;
    auto info = std::make_shared<ImeInfo>();
    info->prop = { .name = "testBundleName", .id = "testExtName" };
    info->subProp = { .name = "testBundleName", .id = "testSubName", .mode = "upper", .language = "english" };
    info->subProps = { { .name = "testBundleName", .id = "testSubName", .mode = "upper", .language = "english" },
        { .name = "testBundleName", .id = "testSubName1", .mode = "lower", .language = "chinese" } };
    ImeInfoInquirer::GetInstance().SetCurrentImeInfo(info);
    ImeCfgManager::GetInstance().imeConfigs_.push_back({ 50, "testBundleName/testExtName", "testSubName" });
    // english->chinese
    auto ret = service_->SwitchByCombinationKey(KeyboardEvent::SHIFT_RIGHT_MASK);
    EXPECT_EQ(ret, ErrorCode::ERROR_IME_START_FAILED);
    // upper->lower
    ret = service_->SwitchByCombinationKey(KeyboardEvent::CAPS_MASK);
    EXPECT_EQ(ret, ErrorCode::ERROR_IME_START_FAILED);

    info->subProp = { .name = "testBundleName", .id = "testSubName1", .mode = "upper", .language = "chinese" };
    info->subProps = { { .name = "testBundleName", .id = "testSubName", .mode = "lower", .language = "english" },
        { .name = "testBundleName", .id = "testSubName1", .mode = "lower", .language = "chinese" } };
    ImeInfoInquirer::GetInstance().SetCurrentImeInfo(info);
    ImeCfgManager::GetInstance().imeConfigs_.push_back({ 50, "testBundleName/testExtName", "testSubName1" });
    // chinese->english
    ret = service_->SwitchByCombinationKey(KeyboardEvent::SHIFT_RIGHT_MASK);
    EXPECT_EQ(ret, ErrorCode::ERROR_IME_START_FAILED);
    // lower->upper
    ret = service_->SwitchByCombinationKey(KeyboardEvent::CAPS_MASK);
    EXPECT_EQ(ret, ErrorCode::ERROR_IME_START_FAILED);
}

/**
 * @tc.name: SA_SwitchByCombinationKey_007
 * @tc.desc: SwitchType():StartInputService() failed
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodPrivateMemberTest, SA_SwitchByCombinationKey_007, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest SA_SwitchByCombinationKey_007 TEST START");
    auto userId = TddUtil::GetCurrentUserId();
    service_->userId_ = userId;
    std::vector<Property> props;
    InputMethodController::GetInstance()->ListInputMethod(props);
    if (props.size() == 1) {
        auto ret = service_->SwitchByCombinationKey(KeyboardEvent::SHIFT_RIGHT_MASK | KeyboardEvent::CTRL_RIGHT_MASK);
        EXPECT_EQ(ret, ErrorCode::ERROR_IME_START_FAILED);
    }
}

/**
 * @tc.name: SA_SwitchByCombinationKey_008
 * @tc.desc: SwitchType():find_if failed
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodPrivateMemberTest, SA_SwitchByCombinationKey_008, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest SA_SwitchByCombinationKey_008 TEST START");
    auto userId = TddUtil::GetCurrentUserId();
    service_->userId_ = userId;
    auto prop = InputMethodController::GetInstance()->GetCurrentInputMethod();
    auto subProp = InputMethodController::GetInstance()->GetCurrentInputMethodSubtype();
    ImeCfgManager::GetInstance().imeConfigs_.push_back({ userId, prop->name + "/" + prop->id, subProp->id });
    std::vector<Property> props;
    InputMethodController::GetInstance()->ListInputMethod(props);
    if (props.size() == 1) {
        auto ret = service_->SwitchByCombinationKey(KeyboardEvent::SHIFT_RIGHT_MASK | KeyboardEvent::CTRL_RIGHT_MASK);
        EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    }
}

/**
 * @tc.name: SA_testReleaseInput_001
 * @tc.desc: client is nullptr
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodPrivateMemberTest, SA_testReleaseInput_001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest SA_testReleaseInput_001 TEST START");
    auto ret = service_->ReleaseInput(nullptr);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);
}

/**
 * @tc.name: III_TestParseSubProp_001
 * @tc.desc: json not contain label/id/icon/mode/locale
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodPrivateMemberTest, III_TestParseSubProp_001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest III_TestParseSubProp_001 TEST START");
    nlohmann::json js;
    js["test"] = "test";
    SubProperty subProp{};
    ImeInfoInquirer::GetInstance().ParseSubProp(js, subProp);
    EXPECT_TRUE(subProp.label.empty());
    EXPECT_TRUE(subProp.icon.empty());
}

/**
 * @tc.name: III_TestParseSubProp_002
 * @tc.desc: json contain label/id/icon/mode/locale,but not correct type
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodPrivateMemberTest, III_TestParseSubProp_002, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest III_TestParseSubProp_002 TEST START");
    nlohmann::json js;
    js["label"] = 100;
    js["id"] = 100;
    js["icon"] = 100;
    js["mode"] = 100;
    js["locale"] = 100;
    SubProperty subProp{};
    ImeInfoInquirer::GetInstance().ParseSubProp(js, subProp);
    EXPECT_TRUE(subProp.label.empty());
    EXPECT_TRUE(subProp.icon.empty());
    EXPECT_TRUE(subProp.id.empty());
    EXPECT_TRUE(subProp.mode.empty());
    EXPECT_TRUE(subProp.locale.empty());
}

/**
 * @tc.name: III_TestParseSubProp_003
 * @tc.desc: json is array, not contain subtypes
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodPrivateMemberTest, III_TestParseSubProp_003, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest III_TestParseSubProp_003 TEST START");
    constexpr int32_t jsNum = 300;
    nlohmann::json js;
    for (uint32_t i = 0; i < jsNum; i++) {
        nlohmann::json temp;
        temp["label"] = 100;
        js["test"].push_back(temp);
    }
    std::vector<SubProperty> subProps;
    auto ret = ImeInfoInquirer::GetInstance().ParseSubProp(js, subProps);
    EXPECT_FALSE(ret);
    EXPECT_TRUE(subProps.empty());
}

/**
 * @tc.name: III_TestParseSubProp_004
 * @tc.desc: json contain subtypes, but not array
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodPrivateMemberTest, III_TestParseSubProp_004, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest III_TestParseSubProp_004 TEST START");
    nlohmann::json js;
    js["subtypes"] = 100;
    std::vector<SubProperty> subProps;
    auto ret = ImeInfoInquirer::GetInstance().ParseSubProp(js, subProps);
    EXPECT_FALSE(ret);
    EXPECT_TRUE(subProps.empty());
}

/**
 * @tc.name: III_TestParseSubProp_005
 * @tc.desc: json contain subtypes, is a empty array
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodPrivateMemberTest, III_TestParseSubProp_005, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest III_TestParseSubProp_005 TEST START");
    nlohmann::json js;
    js["subtypes"] = nlohmann::json::array();
    std::vector<SubProperty> subProps;
    auto ret = ImeInfoInquirer::GetInstance().ParseSubProp(js, subProps);
    EXPECT_FALSE(ret);
    EXPECT_TRUE(subProps.empty());
}

/**
 * @tc.name: III_TestParseSubProp_006
 * @tc.desc: json contain subtypes, is a array
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodPrivateMemberTest, III_TestParseSubProp_006, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest III_TestParseSubProp_006 TEST START");
    constexpr int32_t jsNum = 1;
    nlohmann::json js;
    for (uint32_t i = 0; i < jsNum; i++) {
        nlohmann::json temp;
        temp["label"] = "label";
        temp["id"] = "id";
        temp["icon"] = 100;
        temp["mode"] = "mode";
        temp["locale"] = "locale";
        js["subtypes"].push_back(temp);
    }
    std::vector<SubProperty> subProps;
    auto ret = ImeInfoInquirer::GetInstance().ParseSubProp(js, subProps);
    EXPECT_TRUE(ret);
    EXPECT_EQ(subProps[0].label, "label");
    EXPECT_EQ(subProps[0].id, "id");
    EXPECT_TRUE(subProps[0].icon.empty());
    EXPECT_EQ(subProps[0].mode, "mode");
    EXPECT_EQ(subProps[0].locale, "locale");
}

/**
 * @tc.name: III_TestParseSubProp_007
 * @tc.desc: json is array, num >MAX_SUBTYPE_NUM
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodPrivateMemberTest, III_TestParseSubProp_007, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest III_TestParseSubProp_007 TEST START");
    constexpr int32_t jsNum = 300;
    nlohmann::json js;
    for (uint32_t i = 0; i < jsNum; i++) {
        nlohmann::json temp;
        temp["label"] = 100;
        js["subtypes"].push_back(temp);
    }
    std::vector<SubProperty> subProps;
    ImeInfoInquirer::GetInstance().ParseSubProp(js, subProps);
    EXPECT_EQ(subProps.size(), MAX_SUBTYPE_NUM);
}

/**
 * @tc.name: III_TestParseSubProp_008
 * @tc.desc: profiles is abnormal
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodPrivateMemberTest, III_TestParseSubProp_008, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest III_TestParseSubProp_008 TEST START");
    std::vector<std::string> profiles;
    std::vector<SubProperty> subProps;
    auto ret = ImeInfoInquirer::GetInstance().ParseSubProp(profiles, subProps);
    EXPECT_FALSE(ret);
    profiles.emplace_back("AA");
    profiles.emplace_back("BB");
    ret = ImeInfoInquirer::GetInstance().ParseSubProp(profiles, subProps);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: III_TestGetCurrentInputMethodSubtype_001
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodPrivateMemberTest, III_TestGetCurrentSubtype_001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest III_TestGetCurrentInputMethodSubtype_001 TEST START");
    // currentIme is empty
    auto currentUserId = TddUtil::GetCurrentUserId();
    auto subProp = ImeInfoInquirer::GetInstance().GetCurrentSubtype(currentUserId);
    EXPECT_TRUE(subProp == nullptr);

    // subName is not find
    auto currentProp = InputMethodController::GetInstance()->GetCurrentInputMethod();
    ImeCfgManager::GetInstance().imeConfigs_.push_back(
        { currentUserId, currentProp->name + "/" + currentProp->id, "tt" });
    subProp = ImeInfoInquirer::GetInstance().GetCurrentSubtype(currentUserId);
    EXPECT_TRUE(subProp != nullptr);
    EXPECT_TRUE(subProp->name == currentProp->name);

    // get correct subProp
    auto currentSubProp = InputMethodController::GetInstance()->GetCurrentInputMethodSubtype();
    ImeCfgManager::GetInstance().imeConfigs_.push_back(
        { currentUserId, currentProp->name + "/" + currentProp->id, currentSubProp->id });
    subProp = ImeInfoInquirer::GetInstance().GetCurrentSubtype(currentUserId);
    ASSERT_TRUE(subProp != nullptr);
    EXPECT_TRUE(subProp->id == currentSubProp->id);
}

/**
 * @tc.name: III_TestGetCurrentInputMethod_001
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodPrivateMemberTest, III_TestGetCurrentIme_001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest III_TestGetCurrentInputMethod_001 TEST START");
    // currentIme is empty
    auto currentUserId = TddUtil::GetCurrentUserId();
    auto prop = ImeInfoInquirer::GetInstance().GetCurrentInputMethod(currentUserId);
    EXPECT_TRUE(prop == nullptr);

    // get correct prop
    auto currentProp = InputMethodController::GetInstance()->GetCurrentInputMethod();
    ImeCfgManager::GetInstance().imeConfigs_.push_back(
        { currentUserId, currentProp->name + "/" + currentProp->id, "test" });
    prop = ImeInfoInquirer::GetInstance().GetCurrentInputMethod(currentUserId);
    ASSERT_TRUE(prop != nullptr);
    EXPECT_TRUE(prop->id == currentProp->id);
}

/**
 * @tc.name: III_TestListEnabledInputMethod_001
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodPrivateMemberTest, III_TestListEnabledInputMethod_001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest III_TestListEnabledInputMethod_001 TEST START");
    // currentIme is empty
    std::vector<Property> props;
    auto currentUserId = TddUtil::GetCurrentUserId();
    auto ret = ImeInfoInquirer::GetInstance().ListEnabledInputMethod(currentUserId, props, false);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: III_TestListCurrentInputMethodSubtype_001
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodPrivateMemberTest, III_TestListCurrentInputMethodSubtype_001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest III_TestListCurrentInputMethodSubtype_001 TEST START");
    // currentIme is empty
    std::vector<SubProperty> subProps;
    auto currentUserId = TddUtil::GetCurrentUserId();
    auto ret = ImeInfoInquirer::GetInstance().ListCurrentInputMethodSubtype(currentUserId, subProps);
    EXPECT_EQ(ret, ErrorCode::ERROR_BAD_PARAMETERS);
}

/**
 * @tc.name: III_TestListInputMethod_001
 * @tc.desc: status is error
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodPrivateMemberTest, III_TestListInputMethod_001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest III_TestListInputMethod_001 TEST START");
    std::vector<Property> props;
    auto ret = ImeInfoInquirer::GetInstance().ListInputMethod(60, InputMethodStatus(10), props, false);
    EXPECT_EQ(ret, ErrorCode::ERROR_BAD_PARAMETERS);
}

/**
 * @tc.name: III_TestIsNewExtInfos_001
 * @tc.desc: has no metadata name = SUBTYPE_PROFILE_METADATA_NAME
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodPrivateMemberTest, III_TestIsNewExtInfos_001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest III_TestIsNewExtInfos_001 TEST START");
    ExtensionAbilityInfo extInfo;
    std::vector<SubProperty> subProps;
    auto ret = ImeInfoInquirer::GetInstance().ListInputMethodSubtype(50, extInfo, subProps);
    EXPECT_EQ(ret, ErrorCode::ERROR_BAD_PARAMETERS);
}

/**
 * @tc.name: ICM_TestDeleteImeCfg_001
 * @tc.desc: delete ime cfg correctly
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodPrivateMemberTest, ICM_TestDeleteImeCfg_001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest ICM_TestDeleteImeCfg_001 TEST START");
    ImeCfgManager::GetInstance().imeConfigs_.push_back({ 100, "testBundleName", "testSubName" });
    ImeCfgManager::GetInstance().DeleteImeCfg(100);
    EXPECT_TRUE(ImeCfgManager::GetInstance().imeConfigs_.empty());
}

/**
 * @tc.name: ICM_TestWriteCacheFile_001
 * @tc.desc: json is empty
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodPrivateMemberTest, ICM_TestWriteCacheFile_001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest ICM_TestDeleteImeCfg_001 TEST START");
    std::string path;
    nlohmann::json jsonCfg;
    auto ret = ImeCfgManager::GetInstance().Write(path, jsonCfg);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: ICM_TestFromJson_001
 * @tc.desc: json is array, not contain "imeCfg_list"
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodPrivateMemberTest, ICM_TestFromJson_001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest ICM_TestFromJson_001 TEST START");

    constexpr int32_t jsNum = 2;
    nlohmann::json js;
    for (uint32_t i = 0; i < jsNum; i++) {
        nlohmann::json temp;
        temp["userId"] = 100;
        js["test"].push_back(temp);
    }
    std::vector<ImePersistCfg> configs;
    ImeCfgManager::GetInstance().FromJson(js, configs);
    EXPECT_TRUE(configs.empty());
}

/**
 * @tc.name: ICM_TestFromJson_002
 * @tc.desc: json contain "imeCfg_list", but not array
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodPrivateMemberTest, ICM_TestFromJson_002, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest ICM_TestFromJson_002 TEST START");
    nlohmann::json js;
    js["imeCfg_list"] = 100;
    std::vector<ImePersistCfg> configs;
    ImeCfgManager::GetInstance().FromJson(js, configs);
    EXPECT_TRUE(configs.empty());
}

/**
 * @tc.name: ICM_TestFromJson_003
 * @tc.desc: json contain "imeCfg_list", is a empty array
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodPrivateMemberTest, ICM_TestFromJson_003, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest ICM_TestFromJson_003 TEST START");
    nlohmann::json js;
    js["imeCfg_list"] = nlohmann::json::array();
    std::vector<ImePersistCfg> configs;
    ImeCfgManager::GetInstance().FromJson(js, configs);
    EXPECT_TRUE(configs.empty());
}

/**
 * @tc.name: ICM_TestFromJson_004
 * @tc.desc: json contain "imeCfg_list", is a array
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodPrivateMemberTest, ICM_TestFromJson_004, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest ICM_TestFromJson_004 TEST START");
    constexpr int32_t jsNum = 1;
    nlohmann::json js;
    for (uint32_t i = 0; i < jsNum; i++) {
        nlohmann::json temp;
        temp["userId"] = "userId";
        temp["currentIme"] = 60;
        temp["currentSubName"] = "currentSubName";
        js["imeCfg_list"].push_back(temp);
    }
    std::vector<ImePersistCfg> configs;
    ImeCfgManager::GetInstance().FromJson(js, configs);
    EXPECT_EQ(configs.size(), jsNum);
    EXPECT_EQ(configs[0].userId, ImePersistCfg::INVALID_USERID);
    EXPECT_EQ(configs[0].currentSubName, "currentSubName");
    EXPECT_TRUE(configs[0].currentIme.empty());
}

/**
 * @tc.name: WMSConnectObserver_001
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodPrivateMemberTest, WMSConnectObserver_001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest WMSConnectObserver_001 TEST START");
    WmsConnectionObserver observer(nullptr);
    int32_t userId = 100;
    int32_t screenId = 0;

    observer.OnConnected(userId, screenId);
    EXPECT_EQ(WmsConnectionObserver::connectedUserId_.size(), 1);
    EXPECT_TRUE(WmsConnectionObserver::IsWmsConnected(userId));

    int32_t userId1 = 102;
    observer.OnConnected(userId1, screenId);
    EXPECT_EQ(WmsConnectionObserver::connectedUserId_.size(), 2);
    EXPECT_TRUE(WmsConnectionObserver::IsWmsConnected(userId1));

    observer.OnConnected(userId, screenId);
    EXPECT_EQ(WmsConnectionObserver::connectedUserId_.size(), 2);

    observer.OnDisconnected(userId, screenId);
    EXPECT_EQ(WmsConnectionObserver::connectedUserId_.size(), 1);
    EXPECT_FALSE(WmsConnectionObserver::IsWmsConnected(userId));
}
} // namespace MiscServices
} // namespace OHOS