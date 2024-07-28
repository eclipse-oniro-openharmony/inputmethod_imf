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
#include "full_ime_info_manager.h"
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
using namespace AppExecFwk;
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
    service_ = new (std::nothrow) InputMethodSystemAbility();
    if (service_ == nullptr) {
        return;
    }
    service_->OnStart();
}

void InputMethodPrivateMemberTest::TearDownTestCase(void)
{
    service_->OnStop();
    delete service_;
    service_ = nullptr;
    IMSA_HILOGI("InputMethodPrivateMemberTest::TearDownTestCase");
}

void InputMethodPrivateMemberTest::SetUp(void)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest::SetUp");
    ImeCfgManager::GetInstance().imeConfigs_.clear();
    FullImeInfoManager::GetInstance().fullImeInfos_.clear();
    service_->userId_ = MAIN_USER_ID;
}

void InputMethodPrivateMemberTest::TearDown(void)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest::TearDown");
    ImeCfgManager::GetInstance().imeConfigs_.clear();
    FullImeInfoManager::GetInstance().fullImeInfos_.clear();
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
    EXPECT_EQ(ability.GetUserSession(100), nullptr);
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
* @tc.name: SA_TestOnUserStarted
* @tc.desc: SA_TestOnUserStarted.
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(InputMethodPrivateMemberTest, SA_TestOnUserStarted, TestSize.Level0)
{
    // isScbEnable_ is true
    service_->isScbEnable_ = true;
    MessageParcel *parcel = nullptr;
    auto msg = std::make_shared<Message>(MessageID::MSG_ID_USER_START, parcel);
    auto ret = service_->OnUserStarted(msg.get());
    EXPECT_EQ(ret, ErrorCode::ERROR_NULL_POINTER);

    // msg is nullptr
    service_->isScbEnable_ = false;
    ret = service_->OnUserStarted(msg.get());
    EXPECT_EQ(ret, ErrorCode::ERROR_NULL_POINTER);

    // userId is same
    service_->userId_ = 50;
    MessageParcel *parcel1 = new MessageParcel();
    parcel1->WriteInt32(50);
    auto msg1 = std::make_shared<Message>(MessageID::MSG_ID_USER_START, parcel1);
    ret = service_->OnUserStarted(msg1.get());
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    // start ime
    WmsConnectionObserver observer(nullptr);
    observer.OnConnected(60, 0);
    // imeStarting_ is true
    IMSA_HILOGI("InputMethodPrivateMemberTest::imeStarting_ is true");
    service_->userId_ = 50;
    MessageParcel *parcel2 = new MessageParcel();
    parcel2->WriteInt32(60);
    auto msg2 = std::make_shared<Message>(MessageID::MSG_ID_USER_START, parcel2);
    ret = service_->OnUserStarted(msg2.get());
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    // imeStarting_ is false
    IMSA_HILOGI("InputMethodPrivateMemberTest::imeStarting_ is false");
    service_->userId_ = 50;
    MessageParcel *parcel3 = new MessageParcel();
    observer.OnConnected(333, 0);
    parcel3->WriteInt32(333);
    auto msg3 = std::make_shared<Message>(MessageID::MSG_ID_USER_START, parcel3);
    ret = service_->OnUserStarted(msg3.get());
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
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
    userSession->StopCurrentIme();
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
    FullImeInfo info;
    info.isNewIme = true;
    info.prop = { .name = "testBundleName" };
    info.subProps = { { .id = "testSubName" } };
    FullImeInfoManager::GetInstance().fullImeInfos_.insert({ MAIN_USER_ID, { info } });
    ImeCfgManager::GetInstance().imeConfigs_.push_back({ MAIN_USER_ID, "testBundleName/testExtName", "testSubName" });
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
    FullImeInfo info;
    info.prop = { .name = "testBundleName", .id = "testExtName" };
    info.subProps = { { .name = "testBundleName", .id = "testSubName", .language = "French" } };
    FullImeInfoManager::GetInstance().fullImeInfos_.insert({ MAIN_USER_ID, { info } });
    ImeCfgManager::GetInstance().imeConfigs_.push_back({ MAIN_USER_ID, "testBundleName/testExtName", "testSubName" });
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
    FullImeInfo info;
    info.prop = { .name = "testBundleName", .id = "testExtName" };
    info.subProps = { { .name = "testBundleName", .id = "testSubName", .mode = "upper", .language = "english" } };
    FullImeInfoManager::GetInstance().fullImeInfos_.insert({ MAIN_USER_ID, { info } });
    ImeCfgManager::GetInstance().imeConfigs_.push_back({ MAIN_USER_ID, "testBundleName/testExtName", "testSubName" });
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
    FullImeInfo info;
    info.prop = { .name = "testBundleName", .id = "testExtName" };
    info.subProps = { { .name = "testBundleName", .id = "testSubName", .mode = "upper", .language = "english" },
        { .name = "testBundleName", .id = "testSubName1", .mode = "lower", .language = "chinese" } };
    FullImeInfoManager::GetInstance().fullImeInfos_.insert({ MAIN_USER_ID, { info } });

    ImeCfgManager::GetInstance().imeConfigs_.push_back({ MAIN_USER_ID, "testBundleName/testExtName", "testSubName" });
    // english->chinese
    auto ret = service_->SwitchByCombinationKey(KeyboardEvent::SHIFT_RIGHT_MASK);
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
        EXPECT_EQ(ret, ErrorCode::NO_ERROR);
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
    ASSERT_TRUE(subProp != nullptr);
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
    WmsConnectionObserver::connectedUserId_.clear();
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

/**
 * @tc.name: IMC_testDeactivateClient
 * @tc.desc: DeactivateClient
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodPrivateMemberTest, IMC_testDeactivateClient, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest IMC_testDeactivateClient Test START");
    auto imc = InputMethodController::GetInstance();
    imc->agent_ = std::make_shared<InputMethodAgentStub>();
    MessageParcel data;
    data.WriteRemoteObject(imc->agent_->AsObject());
    imc->agentObject_ = data.ReadRemoteObject();
    imc->clientInfo_.state = ClientState::ACTIVE;
    imc->DeactivateClient();
    EXPECT_EQ(imc->clientInfo_.state, ClientState::INACTIVE);
    EXPECT_EQ(imc->agent_, nullptr);
    EXPECT_EQ(imc->agentObject_, nullptr);
}

/**
 * @tc.name: testIsPanelShown
 * @tc.desc: Test Panel Shown.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPrivateMemberTest, testIsPanelShown, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest PerUserSessionParameterNullptr003 TEST START");
    auto userSession = std::make_shared<PerUserSession>(MAIN_USER_ID);
    PanelInfo panelInfo = { .panelType = SOFT_KEYBOARD, .panelFlag = FLG_FIXED };
    bool flag = true;
    auto ret = userSession->IsPanelShown(panelInfo, flag);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: TestGetDefaultInputMethod_001
 * @tc.desc: TestGetDefaultInputMethod
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodPrivateMemberTest, TestGetDefaultInputMethod_001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest TestGetDefaultInputMethod_001 TEST START");
    // currentIme is empty
    std::shared_ptr<Property> prop;
    auto currentUserId = TddUtil::GetCurrentUserId();
    auto ret = ImeInfoInquirer::GetInstance().GetDefaultInputMethod(currentUserId, prop, false);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: TestGetDefaultInputMethod_002
 * @tc.desc: TestGetDefaultInputMethod
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodPrivateMemberTest, TestGetDefaultInputMethod_002, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest TestGetDefaultInputMethod_002 TEST START");
    // currentIme is empty
    std::shared_ptr<Property> prop;
    auto currentUserId = TddUtil::GetCurrentUserId();
    auto ret = ImeInfoInquirer::GetInstance().GetDefaultInputMethod(currentUserId, prop, true);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: TestGetResMgr
 * @tc.desc: GetResMgr
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodPrivateMemberTest, TestGetResMgr, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest TestGetResMgr TEST START");
    // currentIme is empty
    auto ret = ImeInfoInquirer::GetInstance().GetResMgr("/test");
    EXPECT_TRUE(ret != nullptr);
}

/**
 * @tc.name: TestQueryFullImeInfo
 * @tc.desc: QueryFullImeInfo
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodPrivateMemberTest, TestQueryFullImeInfo, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest TestQueryFullImeInfo TEST START");
    auto currentUserId = TddUtil::GetCurrentUserId();
    std::vector<FullImeInfo> infos;
    auto ret = ImeInfoInquirer::GetInstance().QueryFullImeInfo(currentUserId, infos);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: TestIsInputMethod
 * @tc.desc: IsInputMethod
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodPrivateMemberTest, TestIsInputMethod, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest TestIsInputMethod TEST START");
    auto currentUserId = TddUtil::GetCurrentUserId();
    auto bundleName = "testBundleName1";
    auto ret = ImeInfoInquirer::GetInstance().IsInputMethod(currentUserId, bundleName);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

 /**
 @tc.name: TestHandlePackageEvent
 @tc.desc: TestHandlePackageEvent
 @tc.type: FUNC
 @tc.require:
 */
HWTEST_F(InputMethodPrivateMemberTest, TestHandlePackageEvent, TestSize.Level0)
{
// msg is nullptr
    auto *msg = new Message(MessageID::MSG_ID_PACKAGE_REMOVED, nullptr);
    auto ret = service_->HandlePackageEvent(msg);
    EXPECT_EQ(ret, ErrorCode::ERROR_NULL_POINTER);
    MessageHandler::Instance()->SendMessage(msg);

    // PARCELABLE failed
    MessageParcel *parcel1 = new (std::nothrow) MessageParcel();
    auto bundleName = "testBundleName1";
    parcel1->WriteString(bundleName);
    auto msg1 = std::make_shared<Message>(MessageID::MSG_ID_PACKAGE_REMOVED, parcel1);
    auto ret1 = service_->HandlePackageEvent(msg1.get());
    EXPECT_EQ(ret1, ErrorCode::ERROR_EX_PARCELABLE);

    // userId is not same
    auto parcel2 = new (std::nothrow) MessageParcel();
    auto userId = 50;
    service_->userId_ = 60;
    parcel2->WriteInt32(userId);
    parcel2->WriteString(bundleName);
    auto msg2 = std::make_shared<Message>(MessageID::MSG_ID_PACKAGE_REMOVED, parcel2);
    auto ret2 = service_->HandlePackageEvent(msg2.get());
    EXPECT_EQ(ret2, ErrorCode::NO_ERROR);

    //remove bundle not current ime
    auto parcel3 = new (std::nothrow) MessageParcel();
    service_->userId_ = userId;
    ImeCfgManager::GetInstance().imeConfigs_.push_back({ 60, "testBundleName/testExtName", "testSubName" });
    parcel3->WriteInt32(userId);
    parcel3->WriteString(bundleName);
    auto msg3 = std::make_shared<Message>(MessageID::MSG_ID_PACKAGE_REMOVED, parcel3);
    auto ret3 = service_->HandlePackageEvent(msg3.get());
    EXPECT_EQ(ret3, ErrorCode::NO_ERROR);
}
} // namespace MiscServices
} // namespace OHOS