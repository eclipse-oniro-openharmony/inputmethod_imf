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
#include "client_group.h"
#include "full_ime_info_manager.h"
#include "ime_cfg_manager.h"
#include "ime_info_inquirer.h"
#include "input_method_agent_service_impl.h"
#include "input_method_core_service_impl.h"
#include "input_method_controller.h"
#include "input_method_system_ability.h"
#include "peruser_session.h"
#include "wms_connection_observer.h"
#include "settings_data_utils.h"
#undef private
#include <gtest/gtest.h>
#include <sys/time.h>
#include <unistd.h>

#include <string>
#include <vector>

#include "application_info.h"
#include "combination_key.h"
#include "focus_change_listener.h"
#include "global.h"
#include "iinput_method_agent.h"
#include "iinput_method_core.h"
#include "ime_cfg_manager.h"
#include "input_method_agent_service_impl.h"
#include "input_method_core_service_impl.h"
#include "input_client_service_impl.h"
#include "itypes_util.h"
#include "keyboard_event.h"
#include "os_account_manager.h"
#include "tdd_util.h"
#include "user_session_manager.h"

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
constexpr std::int32_t INVALID_USER_ID = 10001;
constexpr std::int32_t INVALID_PROCESS_ID = -1;
void InputMethodPrivateMemberTest::SetUpTestCase(void)
{
    std::this_thread::sleep_for(std::chrono::seconds(1));
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
    IMSA_HILOGI("InputMethodPrivateMemberTest::TearDownTestCase");
}

void InputMethodPrivateMemberTest::SetUp(void)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest::SetUp");
    ImeCfgManager::GetInstance().imeConfigs_.clear();
    FullImeInfoManager::GetInstance().fullImeInfos_.clear();
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_.clear();
    service_->userId_ = MAIN_USER_ID;
}

void InputMethodPrivateMemberTest::TearDown(void)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest::TearDown");
    ImeCfgManager::GetInstance().imeConfigs_.clear();
    FullImeInfoManager::GetInstance().fullImeInfos_.clear();
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_.clear();
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
    EXPECT_EQ(ability.identityChecker_, nullptr);
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
    Metadata metadata[metaDataNums] = {
        { "language", "english", "" },
        { "mode",     "mode",    "" },
        { "locale",   "local",   "" },
        { "icon",     "icon",    "" },
        { "",         "",        "" }
    };
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
    auto clientGroup = std::make_shared<ClientGroup>(DEFAULT_DISPLAY_ID, nullptr);
    int32_t ret = userSession->ShowKeyboard(imc->clientInfo_.client, clientGroup);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_FOUND);
    ret = userSession->HideKeyboard(imc->clientInfo_.client, clientGroup);
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
    auto clientGroup = std::make_shared<ClientGroup>(DEFAULT_DISPLAY_ID, nullptr);
    auto imc = InputMethodController::GetInstance();

    auto clientInfo = clientGroup->GetClientInfo(imc->clientInfo_.client->AsObject());
    EXPECT_EQ(clientInfo, nullptr);

    clientInfo = clientGroup->GetCurrentClientInfo();
    EXPECT_EQ(clientInfo, nullptr);

    bool clientInfoIsNull = clientGroup->IsCurClientFocused(INVALID_PROCESS_ID, INVALID_USER_ID);
    EXPECT_FALSE(clientInfoIsNull);

    clientInfo = clientGroup->GetClientInfo(INVALID_USER_ID);
    EXPECT_EQ(clientInfo, nullptr);

    clientGroup->SetCurrentClient(nullptr);
    userSession->clientGroupMap_.clear();
    userSession->OnUnfocused(DEFAULT_DISPLAY_ID, 0, 0);
    int32_t ret = userSession->OnHideCurrentInput(DEFAULT_DISPLAY_ID);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_FOUND);
    ret = userSession->OnShowCurrentInput(DEFAULT_DISPLAY_ID);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_FOUND);

    clientGroup->SetCurrentClient(imc->clientInfo_.client);
    ret = userSession->OnShowCurrentInput(DEFAULT_DISPLAY_ID);
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
    clientInfo.client = nullptr;
    std::pair<int64_t, std::string> imeInfo;
    int32_t ret = userSession->OnStartInput(clientInfo, agent, imeInfo);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);
    ret = userSession->OnReleaseInput(nullptr, 0);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);
    auto clientGroup = std::make_shared<ClientGroup>(DEFAULT_DISPLAY_ID, nullptr);
    auto client = clientGroup->GetClientInfo(nullptr);
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
    auto clientGroup = std::make_shared<ClientGroup>(DEFAULT_DISPLAY_ID, nullptr);
    userSession->OnClientDied(nullptr);
    userSession->OnImeDied(nullptr, ImeType::IME);
    bool isShowKeyboard = false;
    clientGroup->UpdateClientInfo(nullptr, { { UpdateFlag::ISSHOWKEYBOARD, isShowKeyboard } });
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
    ImeEnabledCfg cfg;
    ImeEnabledInfo imeInfo;
    imeInfo.bundleName = "bundleName";
    imeInfo.extensionName = "extName";
    imeInfo.extraInfo.isDefaultIme = true;
    imeInfo.extraInfo.currentSubName = "subName";
    cfg.enabledInfos.emplace_back(imeInfo);
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_.insert({ MAIN_USER_ID, cfg });
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
    info.prop.name = "testBundleName";
    SubProperty sub;
    sub.id = "testSubName";
    info.subProps.push_back(sub);
    FullImeInfoManager::GetInstance().fullImeInfos_.insert({ MAIN_USER_ID, { info } });
    ImeEnabledCfg cfg;
    ImeEnabledInfo imeInfo;
    imeInfo.bundleName = "testBundleName";
    imeInfo.extensionName = "testExtName";
    imeInfo.extraInfo.isDefaultIme = true;
    imeInfo.extraInfo.currentSubName = "testSubName";
    cfg.enabledInfos.emplace_back(imeInfo);
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_.insert({ MAIN_USER_ID, cfg });
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
    info.prop.name = "testBundleName";
    info.prop.id = "testExtName";
    SubProperty sub;
    sub.name = "testBundleName";
    sub.id = "testSubName";
    sub.language = "French";
    info.subProps.push_back(sub);
    FullImeInfoManager::GetInstance().fullImeInfos_.insert({ MAIN_USER_ID, { info } });
    ImeEnabledCfg cfg;
    ImeEnabledInfo imeInfo;
    imeInfo.bundleName = "testBundleName";
    imeInfo.extensionName = "testExtName";
    imeInfo.extraInfo.isDefaultIme = true;
    imeInfo.extraInfo.currentSubName = "testSubName";
    cfg.enabledInfos.emplace_back(imeInfo);
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_.insert({ MAIN_USER_ID, cfg });
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
    info.prop.name = "testBundleName";
    info.prop.id = "testExtName";
    SubProperty sub;
    sub.name = "testBundleName";
    sub.id = "testSubName";
    sub.mode = "upper";
    sub.language = "english";
    info.subProps.push_back(sub);
    FullImeInfoManager::GetInstance().fullImeInfos_.insert({ MAIN_USER_ID, { info } });
    ImeEnabledCfg cfg;
    ImeEnabledInfo imeInfo;
    imeInfo.bundleName = "testBundleName";
    imeInfo.extensionName = "testExtName";
    imeInfo.extraInfo.isDefaultIme = true;
    imeInfo.extraInfo.currentSubName = "testSubName";
    cfg.enabledInfos.emplace_back(imeInfo);
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_.insert({ MAIN_USER_ID, cfg });
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
    info.prop.name = "testBundleName";
    info.prop.id = "testExtName";
    SubProperty sub;
    sub.name = "testBundleName";
    sub.id = "testSubName";
    sub.mode = "upper";
    sub.language = "english";
    info.subProps.push_back(sub);
    SubProperty sub1;
    sub1.name = "testBundleName";
    sub1.id = "testSubName1";
    sub1.mode = "lower";
    sub1.language = "chinese";
    info.subProps.push_back(sub1);
    FullImeInfoManager::GetInstance().fullImeInfos_.insert({ MAIN_USER_ID, { info } });

    ImeEnabledCfg cfg;
    ImeEnabledInfo imeInfo;
    imeInfo.bundleName = "testBundleName";
    imeInfo.extensionName = "testExtName";
    imeInfo.extraInfo.isDefaultIme = true;
    imeInfo.extraInfo.currentSubName = "testSubName";
    cfg.enabledInfos.emplace_back(imeInfo);
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_.insert({ MAIN_USER_ID, cfg });
    // english->chinese
    auto ret = service_->SwitchByCombinationKey(KeyboardEvent::SHIFT_RIGHT_MASK);
    EXPECT_EQ(ret, ErrorCode::ERROR_IMSA_REBOOT_OLD_IME_NOT_STOP);
    // lower->upper
    ret = service_->SwitchByCombinationKey(KeyboardEvent::CAPS_MASK);
    EXPECT_EQ(ret, ErrorCode::ERROR_IMSA_REBOOT_OLD_IME_NOT_STOP);
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
    ImeEnabledCfg cfg;
    ImeEnabledInfo imeInfo;
    imeInfo.bundleName = prop->name;
    imeInfo.extensionName = prop->id;
    imeInfo.extraInfo.isDefaultIme = true;
    imeInfo.extraInfo.currentSubName = subProp->id;
    cfg.enabledInfos.emplace_back(imeInfo);
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_.insert({ userId, cfg });
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
    auto ret = service_->ReleaseInput(nullptr, 0);
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
    ImeEnabledCfg cfg;
    ImeEnabledInfo imeInfo;
    cfg.enabledInfos.push_back(imeInfo);
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_.insert({ currentUserId, cfg });
    auto subProp = ImeInfoInquirer::GetInstance().GetCurrentSubtype(currentUserId);
    EXPECT_TRUE(subProp == nullptr);

    // subName is not find
    auto currentProp = InputMethodController::GetInstance()->GetCurrentInputMethod();
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_.clear();
    imeInfo.bundleName = currentProp->name;
    imeInfo.extensionName = currentProp->id;
    imeInfo.extraInfo.isDefaultIme = true;
    imeInfo.extraInfo.currentSubName = "tt";
    cfg.enabledInfos.emplace_back(imeInfo);
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_.insert({ currentUserId, cfg });
    subProp = ImeInfoInquirer::GetInstance().GetCurrentSubtype(currentUserId);
    ASSERT_TRUE(subProp != nullptr);
    EXPECT_TRUE(subProp->name == currentProp->name);

    // get correct subProp
    auto currentSubProp = InputMethodController::GetInstance()->GetCurrentInputMethodSubtype();
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_.clear();
    imeInfo.bundleName = currentProp->name;
    imeInfo.extensionName = currentProp->id;
    imeInfo.extraInfo.isDefaultIme = true;
    imeInfo.extraInfo.currentSubName = currentSubProp->id;
    cfg.enabledInfos.emplace_back(imeInfo);
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_.insert({ currentUserId, cfg });
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
    ImeEnabledCfg cfg;
    ImeEnabledInfo imeInfo;
    cfg.enabledInfos.push_back(imeInfo);
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_.insert({ currentUserId, cfg });
    auto prop = ImeInfoInquirer::GetInstance().GetCurrentInputMethod(currentUserId);
    EXPECT_TRUE(prop == nullptr);

    // get correct prop
    auto currentProp = InputMethodController::GetInstance()->GetCurrentInputMethod();
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_.clear();
    imeInfo.bundleName = currentProp->name;
    imeInfo.extensionName = currentProp->id;
    imeInfo.extraInfo.isDefaultIme = true;
    imeInfo.extraInfo.currentSubName = currentProp->id;
    cfg.enabledInfos.emplace_back(imeInfo);
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_.insert({ currentUserId, cfg });
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
    auto ret = ImeInfoInquirer::GetInstance().ListEnabledInputMethod(currentUserId, props);
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
    ImeEnabledCfg cfg;
    ImeEnabledInfo imeInfo;
    cfg.enabledInfos.push_back(imeInfo);
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_.insert({ currentUserId, cfg });
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
    auto ret = ImeInfoInquirer::GetInstance().ListInputMethod(60, InputMethodStatus(10), props);
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
    ImeCfgManager::GetInstance().imeConfigs_.push_back({ 100, "testBundleName", "testSubName", false });
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
    imc->agent_ = std::make_shared<InputMethodAgentServiceImpl>();
    MessageParcel data;
    data.WriteRemoteObject(imc->agent_->AsObject());
    imc->agentObject_ = data.ReadRemoteObject();
    imc->clientInfo_.state = ClientState::ACTIVE;
    imc->DeactivateClient();
    EXPECT_EQ(imc->clientInfo_.state, ClientState::INACTIVE);
    EXPECT_NE(imc->agent_, nullptr);
    EXPECT_NE(imc->agentObject_, nullptr);
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
    PanelInfo panelInfo;
    panelInfo.panelType = SOFT_KEYBOARD;
    panelInfo.panelFlag = FLG_FIXED;
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

    // remove bundle not current ime
    auto parcel3 = new (std::nothrow) MessageParcel();
    service_->userId_ = userId;
    ImeCfgManager::GetInstance().imeConfigs_.push_back({ 60, "testBundleName/testExtName", "testSubName", false });
    parcel3->WriteInt32(userId);
    parcel3->WriteString(bundleName);
    auto msg3 = std::make_shared<Message>(MessageID::MSG_ID_PACKAGE_REMOVED, parcel3);
    auto ret3 = service_->HandlePackageEvent(msg3.get());
    EXPECT_EQ(ret3, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: testGetSubProperty001
 * @tc.desc: Test testGetSubProperty
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodPrivateMemberTest, testGetSubProperty001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest testGetSubProperty001 START");
    int32_t userId = 100;
    const std::string subName = "defaultImeId";
    SubProperty subProp;
    const std::vector<OHOS::AppExecFwk::ExtensionAbilityInfo> extInfos;
    auto ret = ImeInfoInquirer::GetInstance().GetSubProperty(userId, subName, extInfos, subProp);
    EXPECT_EQ(ret, ErrorCode::ERROR_PACKAGE_MANAGER);
}

/**
 * @tc.name: testGetSubProperty002
 * @tc.desc: Test testGetSubProperty
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodPrivateMemberTest, testGetSubProperty002, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest testGetSubProperty002 START");
    int32_t userId = 100;
    const std::string subName = "defaultImeId";
    SubProperty subProp;
    ExtensionAbilityInfo extInfo;
    extInfo.name = "test";
    const std::vector<OHOS::AppExecFwk::ExtensionAbilityInfo> extInfos = { extInfo };
    auto ret = ImeInfoInquirer::GetInstance().GetSubProperty(userId, subName, extInfos, subProp);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: testListInputMethodSubtype
 * @tc.desc: Test ListInputMethodSubtype
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodPrivateMemberTest, testListInputMethodSubtype, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest testListInputMethodSubtype START");
    int32_t userId = 100;
    const std::string subName = "defaultImeId";
    std::vector<SubProperty> subProps;
    const std::vector<OHOS::AppExecFwk::ExtensionAbilityInfo> extInfos;
    auto ret = ImeInfoInquirer::GetInstance().ListInputMethodSubtype(userId, extInfos, subProps);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: testGetInputMethodConfig
 * @tc.desc: Test GetInputMethodConfig
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodPrivateMemberTest, testGetInputMethodConfig, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest testGetInputMethodConfig START");
    int32_t userId = 100;
    AppExecFwk::ElementName inputMethodConfig;
    auto ret = ImeInfoInquirer::GetInstance().GetInputMethodConfig(userId, inputMethodConfig);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: TestOnSecurityChange
 * @tc.desc: Test OnSecurityChange
 * @tc.type: FUNC
 * @tc.require: issuesI794QF
 */
HWTEST_F(InputMethodPrivateMemberTest, TestOnSecurityChange, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest TestOnSecurityChange TEST START");
    auto userSession = std::make_shared<PerUserSession>(MAIN_USER_ID);
    auto clientGroup = std::make_shared<ClientGroup>(DEFAULT_DISPLAY_ID, nullptr);
    auto imc = InputMethodController::GetInstance();
    int32_t ret = userSession->ShowKeyboard(imc->clientInfo_.client, clientGroup);
    userSession->OnSecurityChange(10);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_FOUND);
    ret = userSession->ShowKeyboard(nullptr, nullptr);
    EXPECT_EQ(ret, ErrorCode::ERROR_IMSA_NULLPTR);
}

/**
 * @tc.name: TestServiceStartInputType
 * @tc.desc: Test ServiceStartInputType
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodPrivateMemberTest, TestServiceStartInputType, TestSize.Level0)
{
    auto ret = service_->ExitCurrentInputType();
    EXPECT_NE(ret, ErrorCode::NO_ERROR);
    ret = service_->StartInputType(static_cast<int32_t>(InputType::NONE));
    EXPECT_NE(ret, ErrorCode::NO_ERROR);
    const PanelInfo panelInfo;
    bool isShown = false;
    ret = service_->IsPanelShown(panelInfo, isShown);
    EXPECT_EQ(ret, ErrorCode::ERROR_STATUS_SYSTEM_PERMISSION);
}

/**
 * @tc.name: TestIsSupported
 * @tc.desc: Test IsSupported
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodPrivateMemberTest, TestIsSupported, TestSize.Level0)
{
    auto ret = InputTypeManager::GetInstance().IsSupported(InputType::NONE);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: TestGetImeByInputType
 * @tc.desc: Test GetImeByInputType
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodPrivateMemberTest, TestGetImeByInputType, TestSize.Level0)
{
    ImeIdentification ime;
    auto ret = InputTypeManager::GetInstance().GetImeByInputType(InputType::NONE, ime);
    EXPECT_EQ(ret, ErrorCode::ERROR_PARSE_CONFIG_FILE);
}

/**
 * @tc.name: TestOnUnRegisteredProxyIme
 * @tc.desc: Test OnUnRegisteredProxyIme
 * @tc.type: FUNC
 * @tc.require: issuesI794QF
 */
HWTEST_F(InputMethodPrivateMemberTest, TestOnUnRegisteredProxyIme, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest TestOnUnRegisteredProxyIme TEST START");
    auto userSession = std::make_shared<PerUserSession>(MAIN_USER_ID);
    UnRegisteredType type = UnRegisteredType::REMOVE_PROXY_IME;
    const sptr<IInputMethodCore> core;
    auto ret = userSession->OnUnRegisteredProxyIme(type, core);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    type = UnRegisteredType::SWITCH_PROXY_IME_TO_IME;
    ret = userSession->OnUnRegisteredProxyIme(type, core);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_BOUND);
    userSession->clientGroupMap_.clear();
    ret = userSession->RemoveAllCurrentClient();
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);
}

/**
 * @tc.name: TestIsInputTypeSupported
 * @tc.desc: Test IsInputTypeSupported
 * @tc.type: FUNC
 * @tc.require: issuesI794QF
 */
HWTEST_F(InputMethodPrivateMemberTest, TestIsInputTypeSupported, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest TestIsInputTypeSupported TEST START");
    InputType type = InputType::SECURITY_INPUT;
    bool resultValue = false;
    auto ret = service_->IsInputTypeSupported(static_cast<int32_t>(type), resultValue);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: TestStartInputType
 * @tc.desc: Test StartInputType
 * @tc.type: FUNC
 * @tc.require: issuesI794QF
 */
HWTEST_F(InputMethodPrivateMemberTest, TestStartInputType, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest TestStartInputType TEST START");
    InputType type = InputType::NONE;
    auto ret = service_->StartInputType(static_cast<int32_t>(type));
    EXPECT_NE(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: TestFullImeInfoManager_Update001
 * @tc.desc: Test FullImeInfoManager_Update
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodPrivateMemberTest, TestFullImeInfoManager_Update001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest TestFullImeInfoManager_Update001 TEST START");
    std::string bundleName = "ttttttttt";
    auto ret = FullImeInfoManager::GetInstance().Update(MAIN_USER_ID, bundleName);
    EXPECT_EQ(ret, ErrorCode::ERROR_PACKAGE_MANAGER);
}

/**
 * @tc.name: TestFullImeInfoManager_Update002
 * @tc.desc: Test FullImeInfoManager_Update
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodPrivateMemberTest, TestFullImeInfoManager_Update002, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest TestFullImeInfoManager_Update002 TEST START");
    std::string bundleName = "testBundleName";
    auto ret = FullImeInfoManager::GetInstance().Update(MAIN_USER_ID, bundleName);
    EXPECT_EQ(ret, ErrorCode::ERROR_PACKAGE_MANAGER);
}

/**
 * @tc.name: TestFullImeInfoManager_Has
 * @tc.desc: Test FullImeInfoManager_Has
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodPrivateMemberTest, TestFullImeInfoManager_Has, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest TestFullImeInfoManager_Has TEST START");
    int32_t userId = 1234567890;
    std::string bundleName = "ttttttttttt";
    auto ret = FullImeInfoManager::GetInstance().Has(userId, bundleName);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: TestFullImeInfoManager_Get
 * @tc.desc: Test FullImeInfoManager_Get
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodPrivateMemberTest, TestFullImeInfoManager_Get, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest TestFullImeInfoManager_Get TEST START");
    FullImeInfo info;
    info.isNewIme = true;
    info.prop.name = "testBundleName";
    SubProperty sub;
    sub.id = "testSubName";
    info.subProps.push_back(sub);
    FullImeInfoManager::GetInstance().fullImeInfos_.insert({ MAIN_USER_ID, { info } });
    uint32_t invalidTokenId = 4294967295;
    auto ret = FullImeInfoManager::GetInstance().Get(MAIN_USER_ID, invalidTokenId);
    EXPECT_EQ(ret, "");
}

/**
 * @tc.name: TestIsMatch
 * @tc.desc: CombinationKey IsMatch
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodPrivateMemberTest, TestIsMatch, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest CombinationKey::IsMatch TEST START");
    uint32_t state = 50; // Assuming 50 is a valid state for this combination key.
    int32_t value = 100;
    CombinationKeyFunction invliadCombinationKey = static_cast<CombinationKeyFunction>(value);
    auto ret = CombinationKey::IsMatch(invliadCombinationKey, state);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: test_OnFocusedAndOnUnfocused001
 * @tc.desc: test OnFocusedAndOnUnfocused
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPrivateMemberTest, test_OnFocusedAndOnUnfocused001, TestSize.Level0)
{
    IMSA_HILOGI("test_OnFocusedAndOnUnfocused001 TEST START");
    const sptr<Rosen::FocusChangeInfo> focusChangeInfo = nullptr;
    FocusHandle handle;
    FocusChangedListener focusChangedListener(handle);
    focusChangedListener.OnFocused(focusChangeInfo);
    focusChangedListener.OnUnfocused(focusChangeInfo);
    EXPECT_EQ(focusChangeInfo, nullptr);
}

/**
 * @tc.name: test_OnFocusedAndOnUnfocused002
 * @tc.desc: test KeyEvent Callback.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPrivateMemberTest, test_OnFocusedAndOnUnfocused002, TestSize.Level0)
{
    IMSA_HILOGI("test_OnFocusedAndOnUnfocused002 TEST START");
    const sptr<Rosen::FocusChangeInfo> focusChangeInfo = new Rosen::FocusChangeInfo();
    FocusHandle handle = nullptr;
    FocusChangedListener focusChangedListener(handle);
    focusChangedListener.OnFocused(focusChangeInfo);
    focusChangedListener.OnUnfocused(focusChangeInfo);
    EXPECT_EQ(handle, nullptr);
}

/**
 * @tc.name: test_WmsConnectionObserver
 * @tc.desc: test KeyEvent Callback.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPrivateMemberTest, test_WmsConnectionObserver, TestSize.Level0)
{
    IMSA_HILOGI("test_WmsConnectionObserver TEST START");
    WmsConnectionObserver observer(nullptr);
    int32_t invalidUserId = 1234567890;
    observer.Remove(invalidUserId);
    ASSERT_EQ(observer.connectedUserId_.find(invalidUserId), observer.connectedUserId_.end());
}

/**
 * @tc.name: BranchCoverage001
 * @tc.desc: BranchCoverage
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPrivateMemberTest, BranchCoverage001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest BranchCoverage001 TEST START");
    auto userSession = std::make_shared<PerUserSession>(MAIN_USER_ID);
    sptr<IInputMethodCore> core = nullptr;
    sptr<IRemoteObject> agent = nullptr;
    pid_t pid { -1 };
    auto ret = userSession->UpdateImeData(core, agent, pid);
    EXPECT_EQ(ret, ErrorCode::ERROR_NULL_POINTER);

    InputClientInfo clientInfo;
    clientInfo.channel = nullptr;
    auto ret2 = service_->PrepareInput(INVALID_USER_ID, clientInfo);
    EXPECT_NE(ret2, ErrorCode::NO_ERROR);

    clientInfo.config.inputAttribute.inputPattern = 7;
    clientInfo.isNotifyInputStart = false;
    ret2 = service_->CheckInputTypeOption(INVALID_USER_ID, clientInfo);
    EXPECT_NE(ret2, ErrorCode::NO_ERROR);

    clientInfo.isNotifyInputStart = true;
    ret2 = service_->CheckInputTypeOption(INVALID_USER_ID, clientInfo);
    EXPECT_EQ(ret2, ErrorCode::ERROR_IMSA_USER_SESSION_NOT_FOUND);

    const std::string bundleName;
    const std::string subName;
    SwitchTrigger trigger = SwitchTrigger::IMSA;
    ret2 = service_->SwitchInputMethod(bundleName, subName, static_cast<uint32_t>(trigger));
    EXPECT_EQ(ret2, ErrorCode::ERROR_BAD_PARAMETERS);

    const std::shared_ptr<ImeInfo> info = nullptr;
    ret2 = service_->SwitchSubType(INVALID_USER_ID, info);
    EXPECT_NE(ret2, ErrorCode::NO_ERROR);

    const SwitchInfo switchInfo;
    ret2 = service_->SwitchInputType(INVALID_USER_ID, switchInfo);
    EXPECT_NE(ret2, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: BranchCoverage002
 * @tc.desc: BranchCoverage
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPrivateMemberTest, BranchCoverage002, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest BranchCoverage002 TEST START");
    auto msgPtr = std::make_shared<Message>(0, nullptr);
    const OHOS::MiscServices::Message* msg = msgPtr.get();
    auto ret = service_->OnUserRemoved(msg);
    EXPECT_EQ(ret, ErrorCode::ERROR_NULL_POINTER);
    ret = service_->OnUserStop(msg);
    EXPECT_EQ(ret, ErrorCode::ERROR_NULL_POINTER);
    ret = service_->OnHideKeyboardSelf(msg);
    EXPECT_EQ(ret, ErrorCode::ERROR_NULL_POINTER);
    ret = service_->HandlePackageEvent(msg);
    EXPECT_EQ(ret, ErrorCode::ERROR_NULL_POINTER);

    service_->HandleUserSwitched(INVALID_USER_ID);

    const SwitchInfo switchInfo;
    UserSessionManager::GetInstance().RemoveUserSession(INVALID_USER_ID);
    auto ret2 = service_->OnStartInputType(INVALID_USER_ID, switchInfo, false);
    EXPECT_EQ(ret2, ErrorCode::ERROR_IMSA_USER_SESSION_NOT_FOUND);

    bool needHide = false;
    InputType type = InputType::NONE;
    auto ret3 = service_->IsCurrentIme(INVALID_USER_ID);
    service_->NeedHideWhenSwitchInputType(INVALID_USER_ID, type, needHide);
    EXPECT_FALSE(ret3);
}

/**
 * @tc.name: BranchCoverage003
 * @tc.desc: BranchCoverage
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPrivateMemberTest, BranchCoverage003, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest BranchCoverage003 TEST START");
    UserSessionManager::GetInstance().RemoveUserSession(INVALID_USER_ID);
    const std::string bundleName = "";
    std::vector<AppExecFwk::ExtensionAbilityInfo> extInfos;
    ImeInfoInquirer inquirer;
    auto ret = inquirer.GetExtInfosByBundleName(INVALID_USER_ID, bundleName, extInfos);
    EXPECT_EQ(ret, ErrorCode::ERROR_BAD_PARAMETERS);

    auto ret2 = inquirer.ListInputMethodInfo(INVALID_USER_ID);
    EXPECT_TRUE(ret2.empty());

    auto ret3 = inquirer.GetDumpInfo(INVALID_USER_ID);
    EXPECT_TRUE(ret3 == "");

    std::vector<Property> props = {};
    ret = inquirer.ListEnabledInputMethod(INVALID_USER_ID, props);
    EXPECT_NE(ret, ErrorCode::NO_ERROR);

    ret = inquirer.ListDisabledInputMethod(INVALID_USER_ID, props);
    EXPECT_NE(ret, ErrorCode::NO_ERROR);

    SwitchInfo switchInfo;
    uint32_t cacheCount = -1;
    ret = inquirer.GetSwitchInfoBySwitchCount(switchInfo, INVALID_USER_ID, cacheCount);
    EXPECT_NE(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: BranchCoverage004
 * @tc.desc: BranchCoverage
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPrivateMemberTest, BranchCoverage004, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest BranchCoverage003 TEST START");
    auto userSession = std::make_shared<PerUserSession>(MAIN_USER_ID);
    sptr<SettingsDataObserver> observer;
    std::shared_ptr<DataShare::DataShareHelper> helper;
    std::string invaildString;
    pid_t pid { -1 };
    auto ret = SettingsDataUtils::GetInstance().RegisterObserver(invaildString, observer);
    EXPECT_EQ(ret, ErrorCode::ERROR_NULL_POINTER);
    ret = SettingsDataUtils::GetInstance().GetStringValue(invaildString, invaildString, invaildString);
    EXPECT_EQ(ret, ErrorCode::ERROR_NULL_POINTER);
    ret = userSession->OnHideInput(nullptr);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_FOCUSED);
    ret = userSession->OnShowInput(nullptr);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_FOCUSED);

    sptr<IInputClient> client = new (std::nothrow) InputClientServiceImpl();
    ASSERT_NE(client, nullptr);
    auto clientGroup = std::make_shared<ClientGroup>(DEFAULT_DISPLAY_ID, nullptr);
    DetachOptions options = { .isUnbindFromClient = false, .isInactiveClient = false, .isNotifyClientAsync = false };
    ret = userSession->RemoveClient(nullptr, clientGroup, options);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);
    ret = userSession->RemoveClient(client, nullptr, options);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);
    ret = userSession->BindClientWithIme(nullptr, ImeType::IME, false);
    userSession->UnBindClientWithIme(nullptr, options);
    EXPECT_EQ(ret, ErrorCode::ERROR_IMSA_NULLPTR);
    ret = userSession->OnSetCallingWindow(0, 0, nullptr);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_FOCUSED);

    auto ret2 = SettingsDataUtils::GetInstance().ReleaseDataShareHelper(helper);
    EXPECT_TRUE(ret2);
    ret2 = SettingsDataUtils::GetInstance().SetStringValue(invaildString, invaildString, invaildString);
    EXPECT_FALSE(ret2);
    ret2 = clientGroup->IsCurClientFocused(-1, -1);
    EXPECT_FALSE(ret2);
    ret2 = clientGroup->IsCurClientUnFocused(-1, -1);
    EXPECT_FALSE(ret2);
    auto startRet = userSession->StartInputService(nullptr);
    EXPECT_EQ(startRet, ErrorCode::ERROR_IMSA_IME_TO_START_NULLPTR);
    startRet = userSession->StartIme(nullptr, false);
    EXPECT_EQ(startRet, ErrorCode::ERROR_IMSA_IME_TO_START_NULLPTR);

    auto ret3 = clientGroup->GetClientInfo(nullptr);
    EXPECT_EQ(ret3, nullptr);
    ret3 = clientGroup->GetClientInfo(pid);
    EXPECT_EQ(ret3, nullptr);
}

/**
 * @tc.name: SA_TestIMSAOnScreenUnlocked
 * @tc.desc: SA_TestIMSAOnScreenUnlocked.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodPrivateMemberTest, SA_TestIMSAOnScreenUnlocked, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest::SA_TestIMSAOnScreenUnlocked start.");
    service_->OnScreenUnlock(nullptr);

    MessageParcel *parcel = nullptr;
    auto msg = std::make_shared<Message>(MessageID::MSG_ID_SCREEN_UNLOCK, parcel);
    service_->OnScreenUnlock(msg.get());

    int32_t userId = 1;
    InputMethodPrivateMemberTest::service_->userId_ = 2;
    parcel = new (std::nothrow) MessageParcel();
    ASSERT_NE(parcel, nullptr);
    EXPECT_TRUE(ITypesUtil::Marshal(*parcel, userId));
    msg = std::make_shared<Message>(MessageID::MSG_ID_SCREEN_UNLOCK, parcel);
    service_->OnScreenUnlock(msg.get());

    UserSessionManager::GetInstance().userSessions_.clear();
    InputMethodPrivateMemberTest::service_->userId_ = userId;
    MessageParcel *parcel1 = new (std::nothrow) MessageParcel();
    ASSERT_NE(parcel1, nullptr);
    EXPECT_TRUE(ITypesUtil::Marshal(*parcel1, userId));
    msg = std::make_shared<Message>(MessageID::MSG_ID_SCREEN_UNLOCK, parcel1);
    service_->OnScreenUnlock(msg.get());
    UserSessionManager::GetInstance().userSessions_.clear();
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

/**
 * @tc.name: SA_TestPerUserSessionOnScreenUnlocked
 * @tc.desc: SA_TestPerUserSessionOnScreenUnlocked.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodPrivateMemberTest, SA_TestPerUserSessionOnScreenUnlocked, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest::SA_TestPerUserSessionOnScreenUnlocked start.");
    auto userSession = std::make_shared<PerUserSession>(MAIN_USER_ID);
    userSession->imeData_.clear();
    userSession->OnScreenUnlock();

    userSession->InitImeData({ "", "" });
    userSession->OnScreenUnlock();

    auto imeCfg = ImeCfgManager::GetInstance().GetCurrentImeCfg(MAIN_USER_ID);
    EXPECT_NE(imeCfg, nullptr);
    userSession->imeData_.clear();
    userSession->InitImeData({ imeCfg->bundleName, imeCfg->extName });
    userSession->OnScreenUnlock();
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

/**
 * @tc.name: SA_TestGetScreenLockIme
 * @tc.desc: SA_TestGetScreenLockIme
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodPrivateMemberTest, SA_TestGetScreenLockIme, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest::SA_TestGetScreenLockIme start.");
    std::string ime;
    int32_t userId = MAIN_USER_ID;
    auto ret = InputMethodPrivateMemberTest::service_->GetScreenLockIme(userId, ime);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ret = InputMethodPrivateMemberTest::service_->GetAlternativeIme(userId, ime);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    SystemConfig systemConfig_0 = ImeInfoInquirer::GetInstance().systemConfig_;
    ImeInfoInquirer::GetInstance().systemConfig_.defaultInputMethod = "abc";
    ret = InputMethodPrivateMemberTest::service_->GetScreenLockIme(userId, ime);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ImeInfoInquirer::GetInstance().systemConfig_ = systemConfig_0;
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

/**
 * @tc.name: Test_ClientGroup_UpdateClientInfo
 * @tc.desc: Test UpdateClientInfo
 * @tc.type: FUNC
 * @tc.require:IBZ0Y6
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodPrivateMemberTest, Test_ClientGroup_UpdateClientInfo, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest Test_ClientGroup_UpdateClientInfo TEST START");
    auto clientGroup = std::make_shared<ClientGroup>(DEFAULT_DISPLAY_ID, nullptr);
    clientGroup->mapClients_.clear();
    bool isShowKeyboard = true;
    sptr<IInputClient> client = new (std::nothrow) InputClientServiceImpl();
    ASSERT_NE(client, nullptr);
    // not find client
    clientGroup->UpdateClientInfo(client->AsObject(), { { UpdateFlag::ISSHOWKEYBOARD, isShowKeyboard } });
    clientGroup->mapClients_.insert({ client->AsObject(), nullptr });
    // client info is nullptr
    clientGroup->UpdateClientInfo(client->AsObject(), { { UpdateFlag::ISSHOWKEYBOARD, isShowKeyboard } });

    auto info = std::make_shared<InputClientInfo>();
    clientGroup->mapClients_.insert_or_assign(client->AsObject(), info);
    // update abnormal
    clientGroup->UpdateClientInfo(client->AsObject(), { { UpdateFlag::CLIENT_TYPE, isShowKeyboard } });
    auto it = clientGroup->mapClients_.find(client->AsObject());
    ASSERT_NE(it, clientGroup->mapClients_.end());
    ASSERT_NE(it->second, nullptr);
    EXPECT_EQ(it->second->type, ClientType::INNER_KIT);
    // update correctly
    uint32_t eventFlag = 10;
    TextTotalConfig config;
    config.windowId = 1000;
    ImeType bindImeType = ImeType::PROXY_IME;
    ClientState state = ClientState::ACTIVE;
    uint32_t uiExtensionTokenId = 9999;
    ClientType type = ClientType::JS;
    clientGroup->UpdateClientInfo(client->AsObject(),
        { { UpdateFlag::BINDIMETYPE, bindImeType }, { UpdateFlag::ISSHOWKEYBOARD, isShowKeyboard },
            { UpdateFlag::EVENTFLAG, eventFlag }, { UpdateFlag::TEXT_CONFIG, config }, { UpdateFlag::STATE, state },
            { UpdateFlag::UIEXTENSION_TOKENID, uiExtensionTokenId }, { UpdateFlag::CLIENT_TYPE, type } });
    it = clientGroup->mapClients_.find(client->AsObject());
    ASSERT_NE(it, clientGroup->mapClients_.end());
    ASSERT_NE(it->second, nullptr);
    EXPECT_EQ(it->second->isShowKeyboard, isShowKeyboard);
    EXPECT_EQ(it->second->eventFlag, eventFlag);
    EXPECT_EQ(it->second->config.windowId, config.windowId);
    EXPECT_EQ(it->second->bindImeType, bindImeType);
    EXPECT_EQ(it->second->uiExtensionTokenId, uiExtensionTokenId);
    EXPECT_EQ(it->second->state, state);
    EXPECT_EQ(it->second->type, type);
}
} // namespace MiscServices
} // namespace OHOS