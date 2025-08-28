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
#include "input_method_agent_service_impl.h"
#include "input_method_core_service_impl.h"
#include "input_method_controller.h"
#include "input_method_system_ability.h"
#include "peruser_session.h"
#include "wms_connection_observer.h"
#include "settings_data_utils.h"
#include "input_type_manager.h"
#include "user_session_manager.h"
#include "system_param_adapter.h"
#include "ime_state_manager_factory.h"
#undef private
#include <gtest/gtest.h>
#include <gtest/hwext/gtest-multithread.h>
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
#include "input_client_stub.h"
#include "input_method_agent_proxy.h"
#include "input_method_agent_stub.h"
#include "input_method_core_stub.h"
#include "im_common_event_manager.h"
#include "input_method_agent_service_impl.h"
#include "input_method_core_service_impl.h"
#include "input_client_service_impl.h"
#include "itypes_util.h"
#include "keyboard_event.h"
#include "os_account_manager.h"
#include "tdd_util.h"
#include "window_adapter.h"
#include "display_adapter.h"

using namespace testing::ext;
using namespace testing::mt;
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
    static void TestImfStartIme();
    static std::atomic<int32_t> tryLockFailCount_;
    static std::shared_ptr<PerUserSession> session_;
};
constexpr std::int32_t MAIN_USER_ID = 100;
constexpr std::int32_t INVALID_USER_ID = 10001;
constexpr std::int32_t INVALID_PROCESS_ID = -1;
constexpr int32_t MS_TO_US = 1000;
constexpr int32_t WAIT_FOR_THREAD_SCHEDULE = 10;
constexpr int32_t WAIT_ATTACH_FINISH_DELAY = 50;
constexpr uint32_t MAX_ATTACH_COUNT = 100000;
std::atomic<int32_t> InputMethodPrivateMemberTest::tryLockFailCount_ = 0;
std::shared_ptr<PerUserSession> InputMethodPrivateMemberTest::session_ = nullptr;
void InputMethodPrivateMemberTest::TestImfStartIme()
{
    auto imeToStart = std::make_shared<ImeNativeCfg>();
    int32_t startRet = session_->StartIme(imeToStart, false);
    IMSA_HILOGI("startRet is %{public}d.", startRet);
    if (startRet == ErrorCode::ERROR_IME_START_INPUT_FAILED) {
        tryLockFailCount_++;
        IMSA_HILOGI("tryLockFailCount_ is  %{public}d.", tryLockFailCount_.load());
    }
}
constexpr const char *EVENT_LARGE_MEMORY_STATUS_CHANGED = "usual.event.memmgr.large_memory_status_changed";
constexpr const char *EVENT_MEMORY_STATE = "memory_state";
constexpr const char *EVENT_PARAM_UID = "uid";

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
    std::vector<sptr<IRemoteObject>> agents;
    InputClientInfo clientInfo;
    clientInfo.client = nullptr;
    std::vector<BindImeInfo> imeInfo;
    int32_t ret = userSession->OnStartInput(clientInfo, agents, imeInfo);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);
    ret = userSession->OnReleaseInput(nullptr, 0);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);
    auto clientGroup = std::make_shared<ClientGroup>(DEFAULT_DISPLAY_ID, nullptr);
    auto client = clientGroup->GetClientInfo(nullptr);
    EXPECT_EQ(client, nullptr);
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
 * @tc.name: SA_SwitchByCombinationKey_Handler
 * @tc.desc: SwitchType():handler is nullptr
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodPrivateMemberTest, SA_SwitchByCombinationKey_Handler, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest SA_SwitchByCombinationKey_Handler TEST START");
    auto userId = TddUtil::GetCurrentUserId();
    service_->userId_ = userId;
    service_->DealSwitchRequest();
    EXPECT_NE(service_->serviceHandler_, nullptr);
    std::shared_ptr<AppExecFwk::EventHandler> tempHandler = service_->serviceHandler_;
    service_->serviceHandler_ = nullptr;
    service_->DealSwitchRequest();
    EXPECT_EQ(service_->serviceHandler_, nullptr);
    service_->serviceHandler_ = tempHandler;
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
 * @tc.name: III_TestRestoreInputMethod_001
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodPrivateMemberTest, III_TestRestoreInputMethod_001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest III_TestRestoreInputMethod_001 TEST START");
    std::string bundleName = "";
    auto ret = service_->RestoreInputmethod(bundleName);
    EXPECT_EQ(ret, ErrorCode::ERROR_ENABLE_IME);
 
    auto userId = service_->GetCallingUserId();
    auto currentProp = ImeInfoInquirer::GetInstance().GetCurrentInputMethod(userId);
    ret = service_->RestoreInputmethod(currentProp->name);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
 
    auto defaultIme = ImeInfoInquirer::GetInstance().GetDefaultIme();
    ret = service_->RestoreInputmethod(defaultIme.bundleName);
    EXPECT_TRUE(ret == ErrorCode::NO_ERROR || ret == ErrorCode::ERROR_IMSA_REBOOT_OLD_IME_NOT_STOP);
 
    bundleName = "com.example.newTestIme";
    EnabledStatus status = EnabledStatus::DISABLED;
    ImeInfoInquirer::GetInstance().systemConfig_.enableInputMethodFeature = true;
    service_->EnableIme(userId, bundleName, "", status);
    ImeEnabledInfoManager::GetInstance().GetEnabledStateInner(userId, bundleName, status);
    EXPECT_EQ(status, EnabledStatus::DISABLED);
    service_->RestoreInputmethod(bundleName);
    ImeEnabledInfoManager::GetInstance().GetEnabledStateInner(userId, bundleName, status);
    EXPECT_EQ(status, EnabledStatus::BASIC_MODE);
    ImeInfoInquirer::GetInstance().systemConfig_.enableInputMethodFeature = false;

    UserSessionManager::GetInstance().RemoveUserSession(userId);
    ret = service_->RestoreInputmethod(defaultIme.bundleName);
    EXPECT_EQ(ret, ErrorCode::ERROR_NULL_POINTER);
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
    ImeEnabledInfo imeInfo1;
    imeInfo1.bundleName = currentProp->name;
    imeInfo1.extensionName = currentProp->id;
    imeInfo1.extraInfo.isDefaultIme = true;
    imeInfo1.extraInfo.currentSubName = "tt";
    ImeEnabledCfg cfg1;
    cfg1.enabledInfos.emplace_back(imeInfo1);
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_.insert({ currentUserId, cfg1 });
    subProp = ImeInfoInquirer::GetInstance().GetCurrentSubtype(currentUserId);
    ASSERT_TRUE(subProp != nullptr);
    EXPECT_TRUE(subProp->name == currentProp->name);

    // get correct subProp
    auto currentSubProp = InputMethodController::GetInstance()->GetCurrentInputMethodSubtype();
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_.clear();
    ImeEnabledInfo imeInfo2;
    imeInfo2.bundleName = currentProp->name;
    imeInfo2.extensionName = currentProp->id;
    imeInfo2.extraInfo.isDefaultIme = true;
    imeInfo2.extraInfo.currentSubName = currentSubProp->id;
    ImeEnabledCfg cfg2;
    cfg2.enabledInfos.emplace_back(imeInfo2);
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_.insert({ currentUserId, cfg2 });
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
    imc->ClearAgentInfo();
    sptr<IInputMethodAgent> agent = new (std::nothrow) InputMethodAgentServiceImpl();
    imc->SetAgent(agent->AsObject(), "");
    imc->clientInfo_.state = ClientState::ACTIVE;
    imc->DeactivateClient();
    EXPECT_EQ(imc->clientInfo_.state, ClientState::INACTIVE);
    EXPECT_GE(imc->agentInfoList_.size(), 1);
    EXPECT_NE(imc->agentInfoList_[0].agent, nullptr);
    EXPECT_NE(imc->agentInfoList_[0].agentObject, nullptr);
    imc->ClearAgentInfo();
}

/**
 * @tc.name: testIsPanelShown
 * @tc.desc: Test Panel Shown.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPrivateMemberTest, testIsPanelShown, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest testIsPanelShown TEST START");
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

    //remove bundle not current ime
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
 * @tc.name: TestServiceStartInputType
 * @tc.desc: Test ServiceStartInputType
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodPrivateMemberTest, TestIsSupported, TestSize.Level0)
{
    auto ret = InputTypeManager::GetInstance().IsSupported(InputType::NONE);
    EXPECT_FALSE(ret);
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
    auto ret = userSession->OnUnRegisteredProxyIme(type, core, IPCSkeleton::GetCallingPid());
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    userSession->clientGroupMap_.clear();
    ret = userSession->RemoveAllCurrentClient();
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);
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
    ret = service_->StartInputType(static_cast<int32_t>(InputType::VOICEKB_INPUT));
    EXPECT_NE(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: TestStartInputTypeAsync
 * @tc.desc: Test StartInputTypeAsync
 * @tc.type: FUNC
 * @tc.require: issuesI794QF
 */
HWTEST_F(InputMethodPrivateMemberTest, TestStartInputTypeAsync, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest TestStartInputTypeAsync TEST START");
    InputType type = InputType::NONE;
    auto ret = service_->StartInputTypeAsync(static_cast<int32_t>(type));
    EXPECT_NE(ret, ErrorCode::NO_ERROR);
    ret = service_->StartInputTypeAsync(static_cast<int32_t>(InputType::VOICEKB_INPUT));
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
    auto ret3 = service_->IsCurrentIme(INVALID_USER_ID, 0);
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
    auto ret = SettingsDataUtils::GetInstance().RegisterObserver(observer);
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
    auto handler = UserSessionManager::GetInstance().eventHandler_;
    UserSessionManager::GetInstance().eventHandler_ = nullptr;
    InputMethodPrivateMemberTest::service_->userId_ = userId;
    MessageParcel *parcel1 = new (std::nothrow) MessageParcel();
    ASSERT_NE(parcel1, nullptr);
    EXPECT_TRUE(ITypesUtil::Marshal(*parcel1, userId));
    msg = std::make_shared<Message>(MessageID::MSG_ID_SCREEN_UNLOCK, parcel1);
    service_->OnScreenUnlock(msg.get());
    UserSessionManager::GetInstance().userSessions_.clear();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    UserSessionManager::GetInstance().eventHandler_ = handler;
}

/**
 * @tc.name: SA_OnScreenLock
 * @tc.desc: SA_OnScreenLock
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodPrivateMemberTest, SA_OnScreenLock, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest::SA_OnScreenLock start.");
    service_->OnScreenLock(nullptr);

    MessageParcel *parcel = nullptr;
    auto msg = std::make_shared<Message>(MessageID::MSG_ID_SCREEN_LOCK, parcel);
    service_->OnScreenLock(msg.get());

    int32_t userId = 1;
    InputMethodPrivateMemberTest::service_->userId_ = 2;
    parcel = new (std::nothrow) MessageParcel();
    ASSERT_NE(parcel, nullptr);
    EXPECT_TRUE(ITypesUtil::Marshal(*parcel, userId));
    msg = std::make_shared<Message>(MessageID::MSG_ID_SCREEN_LOCK, parcel);
    service_->OnScreenLock(msg.get());

    UserSessionManager::GetInstance().userSessions_.clear();
    auto handler = UserSessionManager::GetInstance().eventHandler_;
    UserSessionManager::GetInstance().eventHandler_ = nullptr;
    InputMethodPrivateMemberTest::service_->userId_ = userId;
    MessageParcel *parcel1 = new (std::nothrow) MessageParcel();
    ASSERT_NE(parcel1, nullptr);
    EXPECT_TRUE(ITypesUtil::Marshal(*parcel1, userId));
    msg = std::make_shared<Message>(MessageID::MSG_ID_SCREEN_LOCK, parcel1);
    service_->OnScreenLock(msg.get());
    UserSessionManager::GetInstance().userSessions_.clear();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    UserSessionManager::GetInstance().eventHandler_ = handler;
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
}

/**
 * @tc.name: SA_TestPerUserSessionOnScreenlocked
 * @tc.desc: SA_TestPerUserSessionOnScreenlocked.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodPrivateMemberTest, SA_TestPerUserSessionOnScreenlocked, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest::SA_TestPerUserSessionOnScreenlocked start.");
    auto userSession = std::make_shared<PerUserSession>(MAIN_USER_ID);
    userSession->imeData_.clear();
    userSession->OnScreenLock();
    ImeIdentification currentIme;
    InputTypeManager::GetInstance().Set(false, currentIme);
    EXPECT_FALSE(InputTypeManager::GetInstance().IsStarted());
}

/**
 * @tc.name: SA_TestPerUserSessionOnScreenUnlocked
 * @tc.desc: SA_TestPerUserSessionOnScreenUnlocked.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodPrivateMemberTest, SA_TestPerUserSessionOnScreenUnlocked001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest::SA_TestPerUserSessionOnScreenUnlocked001 start.");
    auto userSession = std::make_shared<PerUserSession>(MAIN_USER_ID);
    ImeIdentification currentIme;
    InputTypeManager::GetInstance().Set(false, currentIme);
    EXPECT_FALSE(InputTypeManager::GetInstance().IsStarted());
    userSession->OnScreenUnlock();
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
}

/**
 * @tc.name: Test_PerUserSession_UpdateClientInfo
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

/**
 * @tc.name: SA_StartPreconfiguredDefaultIme
 * @tc.desc: SA_StartPreconfiguredDefaultIme
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodPrivateMemberTest, SA_StartPreconfiguredDefaultIme, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest::SA_StartPreconfiguredDefaultIme start.");
    PerUserSession session(MAIN_USER_ID);
    // not default displayId
    uint64_t otherDisplayId = 10000000;
    session.virtualScreenDisplayId_.insert(otherDisplayId);
    auto [ret, status] = session.StartPreconfiguredDefaultIme(otherDisplayId);
    EXPECT_EQ(status, StartPreDefaultImeStatus::NO_NEED);

    // not has running ime
    session.imeData_.clear();
    auto [ret1, status1] = session.StartPreconfiguredDefaultIme(DEFAULT_DISPLAY_ID);
    EXPECT_EQ(status1, StartPreDefaultImeStatus::TO_START);

    std::string bundleName = "bundleName";
    std::string extName = "extName";
    std::string bundleName1 = "bundleName1";
    std::string extName1 = "extName1";
    // running ime same with pre default ime
    auto imeData1 = std::make_shared<ImeData>(nullptr, nullptr, nullptr, 100);
    imeData1->imeStatus = ImeStatus::READY;
    imeData1->ime = std::make_pair(bundleName, extName);
    std::vector<std::shared_ptr<ImeData>> imeDataList;
    imeDataList.push_back(imeData1);
    session.imeData_.insert_or_assign(ImeType::IME, imeDataList);
    ImeInfoInquirer::GetInstance().systemConfig_.defaultInputMethod = bundleName + "/" + extName;
    auto [ret2, status2] = session.StartPreconfiguredDefaultIme(DEFAULT_DISPLAY_ID);
    EXPECT_EQ(status2, StartPreDefaultImeStatus::HAS_STARTED);
    // running ime extName not same with pre default ime
    ImeInfoInquirer::GetInstance().systemConfig_.defaultInputMethod = bundleName + "/" + extName1;
    auto [ret3, status3] = session.StartPreconfiguredDefaultIme(DEFAULT_DISPLAY_ID);
    EXPECT_EQ(status3, StartPreDefaultImeStatus::TO_START);
    // running ime bundleName not same with pre default ime
    ImeInfoInquirer::GetInstance().systemConfig_.defaultInputMethod = bundleName1 + "/" + extName;
    auto [ret4, status4] = session.StartPreconfiguredDefaultIme(DEFAULT_DISPLAY_ID);
    EXPECT_EQ(status4, StartPreDefaultImeStatus::TO_START);
    // running ime not same with pre default ime
    ImeInfoInquirer::GetInstance().systemConfig_.defaultInputMethod = bundleName1 + "/" + extName1;
    auto [ret5, status5] = session.StartPreconfiguredDefaultIme(DEFAULT_DISPLAY_ID);
    EXPECT_EQ(status5, StartPreDefaultImeStatus::TO_START);
}

/**
 * @tc.name: SA_AllowSwitchImeByCombinationKey
 * @tc.desc: SA_AllowSwitchImeByCombinationKey
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodPrivateMemberTest, SA_AllowSwitchImeByCombinationKey, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest::SA_AllowSwitchImeByCombinationKey start.");
    PerUserSession session(MAIN_USER_ID);
    // not has current client info
    session.clientGroupMap_.clear();
    auto ret = session.IsImeSwitchForbidden();
    EXPECT_FALSE(ret);

    // has current client info
    ImeInfoInquirer::GetInstance().systemConfig_.defaultImeScreenList.clear();
    auto group = std::make_shared<ClientGroup>(DEFAULT_DISPLAY_ID, nullptr);
    sptr<IInputClient> client = new (std::nothrow) InputClientServiceImpl();
    group->currentClient_ = client;
    auto info = std::make_shared<InputClientInfo>();
    info->config.isSimpleKeyboardEnabled = true;
    group->mapClients_.insert_or_assign(client->AsObject(), info);
    session.clientGroupMap_.insert_or_assign(DEFAULT_DISPLAY_ID, group);
    ret = session.IsImeSwitchForbidden();
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: SA_SpecialScenarioCheck
 * @tc.desc: SA_SpecialScenarioCheck
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodPrivateMemberTest, SA_SpecialScenarioCheck, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest::SA_SpecialScenarioCheck start.");
    PerUserSession session(MAIN_USER_ID);
    // has current client info
    auto group = std::make_shared<ClientGroup>(DEFAULT_DISPLAY_ID, nullptr);
    sptr<IInputClient> client = new (std::nothrow) InputClientServiceImpl();
    group->currentClient_ = client;
    auto info = std::make_shared<InputClientInfo>();
    info->config.isSimpleKeyboardEnabled = true;
    group->mapClients_.insert_or_assign(client->AsObject(), info);
    session.clientGroupMap_.insert_or_assign(DEFAULT_DISPLAY_ID, group);
    auto allow = session.SpecialScenarioCheck();
    EXPECT_FALSE(allow);

    info->config.isSimpleKeyboardEnabled = false;
    group->mapClients_.insert_or_assign(client->AsObject(), info);
    session.clientGroupMap_.insert_or_assign(DEFAULT_DISPLAY_ID, group);
    allow = session.SpecialScenarioCheck();
    EXPECT_TRUE(allow);

    info->config.inputAttribute.inputPattern = InputAttribute::PATTERN_ONE_TIME_CODE;
    group->mapClients_.insert_or_assign(client->AsObject(), info);
    session.clientGroupMap_.insert_or_assign(DEFAULT_DISPLAY_ID, group);
    allow = session.SpecialScenarioCheck();
    EXPECT_FALSE(allow);

    info->config.inputAttribute.inputPattern = InputAttribute::PATTERN_PASSWORD;
    group->mapClients_.insert_or_assign(client->AsObject(), info);
    session.clientGroupMap_.insert_or_assign(DEFAULT_DISPLAY_ID, group);
    allow = session.SpecialScenarioCheck();
    EXPECT_FALSE(allow);
}

/**
 * @tc.name: SA_IsScreenLockOrSecurityFlag
 * @tc.desc: SA_IsScreenLockOrSecurityFlag
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodPrivateMemberTest, SA_IsScreenLockOrSecurityFlag, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest::SA_IsScreenLockOrSecurityFlag start.");
    PerUserSession session(MAIN_USER_ID);
    // has current client info
    auto group = std::make_shared<ClientGroup>(DEFAULT_DISPLAY_ID, nullptr);
    sptr<IInputClient> client = new (std::nothrow) InputClientServiceImpl();
    group->currentClient_ = client;
    auto info = std::make_shared<InputClientInfo>();
    info->config.inputAttribute.inputPattern = InputAttribute::PATTERN_TEXT;
    group->mapClients_.insert_or_assign(client->AsObject(), info);
    session.clientGroupMap_.insert_or_assign(DEFAULT_DISPLAY_ID, group);
    auto ret = session.IsImeSwitchForbidden();
    EXPECT_FALSE(ret);


    info->config.inputAttribute.inputPattern = InputAttribute::PATTERN_ONE_TIME_CODE;
    group->mapClients_.insert_or_assign(client->AsObject(), info);
    session.clientGroupMap_.insert_or_assign(DEFAULT_DISPLAY_ID, group);
    ret = session.IsImeSwitchForbidden();
    EXPECT_FALSE(ret);

    info->config.inputAttribute.inputPattern = InputAttribute::PATTERN_PASSWORD;
    group->mapClients_.insert_or_assign(client->AsObject(), info);
    session.clientGroupMap_.insert_or_assign(DEFAULT_DISPLAY_ID, group);
    ret = session.IsImeSwitchForbidden();
    EXPECT_TRUE(ret);

    info->config.inputAttribute.inputPattern = InputAttribute::PATTERN_TEXT;
    info->config.isSimpleKeyboardEnabled = true;
    group->mapClients_.insert_or_assign(client->AsObject(), info);
    session.clientGroupMap_.insert_or_assign(DEFAULT_DISPLAY_ID, group);
    ret = session.IsImeSwitchForbidden();
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: SA_SpecialSendPrivateData
 * @tc.desc: SA_SpecialSendPrivateData
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodPrivateMemberTest, SA_SpecialSendPrivateData, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest::SA_SpecialSendPrivateData start.");
    PerUserSession session(MAIN_USER_ID);
    std::string bundleName = "bundleName";
    std::string extName = "extName";
    std::string bundleName1 = "bundleName1";
    std::string extName1 = "extName1";
    auto imeData1 = std::make_shared<ImeData>(nullptr, nullptr, nullptr, 100);
    imeData1->imeStatus = ImeStatus::READY;
    imeData1->ime = std::make_pair(bundleName, extName);
    std::vector<std::shared_ptr<ImeData>> imeDataList;
    imeDataList.push_back(imeData1);
    session.imeData_.insert_or_assign(ImeType::IME, imeDataList);
    std::unordered_map<std::string, PrivateDataValue> privateCommand;
    // running ime same with pre default ime, send directly
    ImeInfoInquirer::GetInstance().systemConfig_.defaultInputMethod = bundleName + "/" + extName;
    auto ret = session.SpecialSendPrivateData(privateCommand);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    // running ime extName not same with pre default ime, start pre default ime failed
    ImeInfoInquirer::GetInstance().systemConfig_.defaultInputMethod = bundleName1 + "/" + extName1;
    ret = session.SpecialSendPrivateData(privateCommand);
    EXPECT_NE(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: SA_CheckInputTypeOption
 * @tc.desc: SA_CheckInputTypeOption
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodPrivateMemberTest, SA_CheckInputTypeOption, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest::SA_CheckInputTypeOption start.");
    InputMethodSystemAbility systemAbility;
    auto session = std::make_shared<PerUserSession>(MAIN_USER_ID);
    std::string bundleName = "bundleName";
    std::string extName = "extName";
    auto imeData1 = std::make_shared<ImeData>(nullptr, nullptr, nullptr, 100);
    imeData1->imeStatus = ImeStatus::READY;
    imeData1->ime = std::make_pair(bundleName, extName);
    std::vector<std::shared_ptr<ImeData>> imeDataList;
    imeDataList.push_back(imeData1);
    session->imeData_.insert_or_assign(ImeType::IME, imeDataList);
    UserSessionManager::GetInstance().userSessions_.insert_or_assign(MAIN_USER_ID, session);
    InputClientInfo info;
    // same textField, input type started
    info.isNotifyInputStart = false;
    InputTypeManager::GetInstance().isStarted_ = true;
    auto ret = systemAbility.CheckInputTypeOption(MAIN_USER_ID, info);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    std::string bundleName1 = "bundleName1";
    std::string extName1 = "extName1";
    std::string bundleName2 = "bundleName2";
    std::string extName2 = "extName2";
    ImeInfoInquirer::GetInstance().systemConfig_.defaultInputMethod = bundleName1 + "/" + extName1;
    ImeEnabledCfg cfg;
    ImeEnabledInfo enabledInfo{ bundleName2, extName2, EnabledStatus::BASIC_MODE };
    enabledInfo.extraInfo.isDefaultIme = true;
    cfg.enabledInfos.push_back(enabledInfo);
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_.insert_or_assign(MAIN_USER_ID, cfg);

    info.isNotifyInputStart = true;
    InputTypeManager::GetInstance().isStarted_ = true;
    ret = systemAbility.CheckInputTypeOption(MAIN_USER_ID, info);
    EXPECT_NE(ret, ErrorCode::NO_ERROR);

    info.isNotifyInputStart = false;
    InputTypeManager::GetInstance().isStarted_ = false;
    ret = systemAbility.CheckInputTypeOption(MAIN_USER_ID, info);
    EXPECT_NE(ret, ErrorCode::NO_ERROR);

    info.isNotifyInputStart = true;
    InputTypeManager::GetInstance().isStarted_ = false;
    ret = systemAbility.CheckInputTypeOption(MAIN_USER_ID, info);
    EXPECT_NE(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: SA_GetRealCurrentIme_001
 * @tc.desc: SA_GetRealCurrentIme_001
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodPrivateMemberTest, SA_GetRealCurrentIme_001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest::SA_GetRealCurrentIme_001 start.");
    std::shared_ptr<Property> realPreIme = nullptr;
    InputMethodController::GetInstance()->GetDefaultInputMethod(realPreIme);
    ASSERT_NE(realPreIme, nullptr);
    InputTypeManager::GetInstance().currentTypeIme_.bundleName = realPreIme->name;

    std::string bundleName1 = "bundleName1";
    std::string extName1 = "extName1";
    ImeEnabledCfg cfg;
    ImeEnabledInfo enabledInfo{ bundleName1, extName1, EnabledStatus::BASIC_MODE };
    enabledInfo.extraInfo.isDefaultIme = true;
    ImeEnabledInfo enabledInfo1{ realPreIme->name, realPreIme->id, EnabledStatus::BASIC_MODE };
    cfg.enabledInfos.push_back(enabledInfo);
    cfg.enabledInfos.push_back(enabledInfo1);
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_.insert_or_assign(MAIN_USER_ID, cfg);

    std::string bundleName2 = "bundleName2";
    std::string extName2 = "extName2";
    ImeInfoInquirer::GetInstance().systemConfig_.defaultInputMethod = bundleName2 + "/" + extName2;
    auto session = std::make_shared<PerUserSession>(MAIN_USER_ID);

    // input type start
    InputTypeManager::GetInstance().isStarted_ = true;
    auto ime = session->GetRealCurrentIme(true);
    ASSERT_NE(ime, nullptr);
    EXPECT_EQ(ime->bundleName, realPreIme->name);

    // input type not start, has no current client, needMinGuarantee is false
    InputTypeManager::GetInstance().isStarted_ = false;
    session->clientGroupMap_.clear();
    ime = session->GetRealCurrentIme(false);
    ASSERT_NE(ime, nullptr);
    EXPECT_EQ(ime->bundleName, bundleName1);

    // input type not start, has no current client, needMinGuarantee is true
    ime = session->GetRealCurrentIme(true);
    ASSERT_NE(ime, nullptr);
    EXPECT_EQ(ime->bundleName, bundleName2);
}

/**
 * @tc.name: SA_GetRealCurrentIme_002
 * @tc.desc: SA_GetRealCurrentIme_002
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodPrivateMemberTest, SA_GetRealCurrentIme_002, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest::SA_GetRealCurrentIme_002 start.");
    ImeInfoInquirer::GetInstance().systemConfig_.defaultImeScreenList.clear();
    std::shared_ptr<Property> realPreIme = nullptr;
    InputMethodController::GetInstance()->GetDefaultInputMethod(realPreIme);
    ASSERT_NE(realPreIme, nullptr);
    InputTypeManager::GetInstance().isTypeCfgReady_ = true;
    ImeIdentification inputTypeIme{ realPreIme->name, "" };
    InputTypeManager::GetInstance().inputTypes_.insert_or_assign(InputType::SECURITY_INPUT, inputTypeIme);

    std::string bundleName1 = "bundleName1";
    std::string extName1 = "extName1";
    ImeEnabledCfg cfg;
    ImeEnabledInfo enabledInfo{ bundleName1, extName1, EnabledStatus::BASIC_MODE };
    enabledInfo.extraInfo.isDefaultIme = true;
    cfg.enabledInfos.push_back(enabledInfo);
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_.insert_or_assign(MAIN_USER_ID, cfg);

    std::string bundleName2 = "bundleName2";
    std::string extName2 = "extName2";
    ImeInfoInquirer::GetInstance().systemConfig_.defaultInputMethod = bundleName2 + "/" + extName2;

    InputTypeManager::GetInstance().isStarted_ = false;
    auto session = std::make_shared<PerUserSession>(MAIN_USER_ID);
    auto group = std::make_shared<ClientGroup>(DEFAULT_DISPLAY_ID, nullptr);
    sptr<IInputClient> client = new (std::nothrow) InputClientServiceImpl();
    group->currentClient_ = client;
    auto info = std::make_shared<InputClientInfo>();
    // input type not start, has current client, input type is security, isSimpleKeyboardEnabled is true
    info->config.inputAttribute.inputPattern = InputAttribute::PATTERN_PASSWORD;
    info->config.isSimpleKeyboardEnabled = true;
    group->mapClients_.insert_or_assign(client->AsObject(), info);
    session->clientGroupMap_.insert_or_assign(DEFAULT_DISPLAY_ID, group);
    auto ime = session->GetRealCurrentIme(true);
    ASSERT_NE(ime, nullptr);
    EXPECT_EQ(ime->bundleName, realPreIme->name);
}

/**
 * @tc.name: SA_GetRealCurrentIme_003
 * @tc.desc: SA_GetRealCurrentIme_003
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodPrivateMemberTest, SA_GetRealCurrentIme_003, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest::SA_GetRealCurrentIme_003 start.");
    // input type not start, has current client, input type is NONE, isSimpleKeyboardEnabled is true
    InputTypeManager::GetInstance().isStarted_ = false;
    std::string bundleName1 = "bundleName1";
    std::string extName1 = "extName1";
    std::string subName1 = "subName1";
    ImeEnabledCfg cfg;
    ImeEnabledInfo enabledInfo{ bundleName1, extName1, EnabledStatus::BASIC_MODE };
    enabledInfo.extraInfo.isDefaultIme = true;
    enabledInfo.extraInfo.currentSubName = subName1;
    cfg.enabledInfos.push_back(enabledInfo);
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_.insert_or_assign(MAIN_USER_ID, cfg);

    auto session = std::make_shared<PerUserSession>(MAIN_USER_ID);
    auto group = std::make_shared<ClientGroup>(DEFAULT_DISPLAY_ID, nullptr);
    sptr<IInputClient> client = new (std::nothrow) InputClientServiceImpl();
    group->currentClient_ = client;
    auto info = std::make_shared<InputClientInfo>();
    info->config.inputAttribute.inputPattern = 0;
    info->config.isSimpleKeyboardEnabled = true;
    group->mapClients_.insert_or_assign(client->AsObject(), info);
    session->clientGroupMap_.insert_or_assign(DEFAULT_DISPLAY_ID, group);

    // preconfigured is nullptr
    ImeInfoInquirer::GetInstance().systemConfig_.defaultInputMethod = "abnormal";
    auto ime = session->GetRealCurrentIme(false);
    ASSERT_NE(ime, nullptr);
    EXPECT_EQ(ime->bundleName, bundleName1);
    EXPECT_EQ(ime->subName, subName1);

    // preconfigured is not nullptr,  preconfigured ime same with default ime
    ImeInfoInquirer::GetInstance().systemConfig_.defaultInputMethod = bundleName1 + "/" + extName1;
    ime = session->GetRealCurrentIme(true);
    ASSERT_NE(ime, nullptr);
    EXPECT_EQ(ime->bundleName, bundleName1);
    EXPECT_EQ(ime->subName, subName1);

    // preconfigured is not nullptr,  preconfigured ime not same with default ime
    std::string bundleName2 = "bundleName2";
    std::string extName2 = "extName2";
    ImeInfoInquirer::GetInstance().systemConfig_.defaultInputMethod = bundleName2 + "/" + extName2;
    ime = session->GetRealCurrentIme(true);
    ASSERT_NE(ime, nullptr);
    EXPECT_EQ(ime->bundleName, bundleName2);
    EXPECT_TRUE(ime->subName.empty());
}

/**
 * @tc.name: SA_NotifySubTypeChangedToIme
 * @tc.desc: SA_NotifySubTypeChangedToIme
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodPrivateMemberTest, SA_NotifySubTypeChangedToIme, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest::SA_NotifySubTypeChangedToIme start.");
    std::string bundleName1 = "bundleName1";
    std::string subName1 = "extName1";
    std::string bundleName2 = "bundleName2";
    std::string subName2 = "subName2";
    InputTypeManager::GetInstance().inputTypeImeList_.clear();
    InputTypeManager::GetInstance().inputTypeImeList_.insert({ bundleName1, subName1 });
    auto session = std::make_shared<PerUserSession>(MAIN_USER_ID);
    auto ret = session->NotifySubTypeChangedToIme(bundleName1, "");
    EXPECT_EQ(ret, ErrorCode::ERROR_IME_NOT_STARTED);
    ret = session->NotifySubTypeChangedToIme(bundleName1, subName1);
    EXPECT_EQ(ret, ErrorCode::ERROR_IME_NOT_STARTED);
    ret = session->NotifySubTypeChangedToIme(bundleName2, subName2);
    EXPECT_EQ(ret, ErrorCode::ERROR_IME_NOT_STARTED);
}

/**
 * @tc.name: StartImeTryLock
 * @tc.desc: StartImeTryLock
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodPrivateMemberTest, StartImeTryLock001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest::StartImeTryLock001 start.");
    session_ = std::make_shared<PerUserSession>(MAIN_USER_ID);
    SET_THREAD_NUM(5);
    GTEST_RUN_TASK(TestImfStartIme);
    IMSA_HILOGI("InputMethodPrivateMemberTest::StartImeTryLock %{public}d.", tryLockFailCount_.load());
    EXPECT_GT(tryLockFailCount_, 0); // at least one thread try lock failed
}

/**
 * @tc.name: TestCompareExchange_001
 * @tc.desc: TestCompareExchange.
 * @tc.type: FUNC
 * @tc.require: issuesIC7VH8
 * @tc.author:
 */
HWTEST_F(InputMethodPrivateMemberTest, TestCompareExchange_001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest TestCompareExchange_001 TEST START");
    auto userSession = std::make_shared<PerUserSession>(MAIN_USER_ID);
    userSession->largeMemoryState_ = 2;
    auto ret = userSession->CompareExchange(2);
    EXPECT_TRUE(ret);
    ret = userSession->CompareExchange(3);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: TestIsLargeMemoryStateNeed_001
 * @tc.desc: TestIsLargeMemoryStateNeed.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(InputMethodPrivateMemberTest, TestIsLargeMemoryStateNeed_001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest TestIsLargeMemoryStateNeed_001 TEST START");
    auto userSession = std::make_shared<PerUserSession>(MAIN_USER_ID);
    userSession->largeMemoryState_ = 2;
    auto ret = userSession->IsLargeMemoryStateNeed();
    EXPECT_TRUE(ret);
    userSession->largeMemoryState_ = 3;
    ret = userSession->IsLargeMemoryStateNeed();
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: TestIsLargeMemoryStateNeed_002
 * @tc.desc: Test IsLargeMemoryStateNeed.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(InputMethodPrivateMemberTest, TestIsLargeMemoryStateNeed_002, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest TestIsLargeMemoryStateNeed_002 TEST START");
    auto userSession = std::make_shared<PerUserSession>(MAIN_USER_ID);
    auto ret = userSession->UpdateLargeMemorySceneState(2);
    userSession->StartImeInImeDied();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    userSession->UpdateLargeMemorySceneState(3);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: TestHandleUpdateLargeMemoryState_001
 * @tc.desc: Test HandleUpdateLargeMemoryState.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(InputMethodPrivateMemberTest, TestHandleUpdateLargeMemoryState_001, TestSize.Level0)
{
    // msg is nullptr
    int32_t uid = 1014;
    auto *msg = new Message(MessageID::MSG_ID_UPDATE_LARGE_MEMORY_STATE, nullptr);
    ASSERT_NE(service_, nullptr);
    auto ret = service_->HandleUpdateLargeMemoryState(msg);
    EXPECT_EQ(ret, ErrorCode::ERROR_NULL_POINTER);
    ASSERT_NE(MessageHandler::Instance(), nullptr);
    MessageHandler::Instance()->SendMessage(msg);

    auto parcel1 = new (std::nothrow) MessageParcel();
    ASSERT_NE(parcel1, nullptr);
    parcel1->WriteInt32(uid);
    parcel1->WriteInt32(3);
    auto msg1 = std::make_shared<Message>(MessageID::MSG_ID_UPDATE_LARGE_MEMORY_STATE, parcel1);
    ASSERT_NE(msg1, nullptr);
    auto ret1 = service_->HandleUpdateLargeMemoryState(msg1.get());
    EXPECT_EQ(ret1, ErrorCode::NO_ERROR);

    auto parcel2 = new (std::nothrow) MessageParcel();
    ASSERT_NE(parcel2, nullptr);
    parcel2->WriteInt32(uid);
    parcel2->WriteInt32(4);
    auto msg2 = std::make_shared<Message>(MessageID::MSG_ID_UPDATE_LARGE_MEMORY_STATE, parcel2);
    ASSERT_NE(msg2, nullptr);
    auto ret2 = service_->HandleUpdateLargeMemoryState(msg2.get());
    EXPECT_EQ(ret2, ErrorCode::ERROR_BAD_PARAMETERS);

    auto parcel3 = new (std::nothrow) MessageParcel();
    ASSERT_NE(parcel3, nullptr);
    parcel3->WriteInt32(-1);
    parcel3->WriteInt32(3);
    auto msg3 = std::make_shared<Message>(MessageID::MSG_ID_UPDATE_LARGE_MEMORY_STATE, parcel3);
    ASSERT_NE(msg3, nullptr);
    auto ret3 = service_->HandleUpdateLargeMemoryState(msg3.get());
    EXPECT_EQ(ret3, ErrorCode::ERROR_NULL_POINTER);
}

/**
 * @tc.name: TestEventUpdateLargeMemoryState_001
 * @tc.desc: Test EventUpdateLargeMemoryState.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(InputMethodPrivateMemberTest, TestEventUpdateLargeMemoryState_001, TestSize.Level0)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENT_LARGE_MEMORY_STATUS_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    std::shared_ptr<ImCommonEventManager::EventSubscriber> subscriber =
        std::make_shared<ImCommonEventManager::EventSubscriber>(subscriberInfo);
    ASSERT_NE(subscriber, nullptr);
    EXPECT_NE(subscriber, nullptr);

    AAFwk::Want want;
    int32_t uid = 1014;
    int32_t memoryState = 3;
    want.SetAction(EVENT_LARGE_MEMORY_STATUS_CHANGED);
    want.SetParam(EVENT_PARAM_UID, uid);
    want.SetParam(EVENT_MEMORY_STATE, memoryState);
    EventFwk::CommonEventData data;
    data.SetWant(want);
    subscriber->HandleLargeMemoryStateUpdate(data);

    AAFwk::Want want1;
    uid = -1;
    memoryState = 3;
    want1.SetAction(EVENT_LARGE_MEMORY_STATUS_CHANGED);
    want1.SetParam(EVENT_PARAM_UID, uid);
    want1.SetParam(EVENT_MEMORY_STATE, memoryState);
    EventFwk::CommonEventData data1;
    data1.SetWant(want1);
    subscriber->HandleLargeMemoryStateUpdate(data1);
}

/**
 * @tc.name: TestGetDisableNumKeyAppDeviceTypes
 * @tc.desc: Test GetDisableNumKeyAppDeviceTypes.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodPrivateMemberTest, TestGetDisableNumKeyAppDeviceTypes, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest TestGetDisableNumKeyAppDeviceTypes START");
    std::string testDeviceType = "testDeviceType";
    ImeInfoInquirer::GetInstance().systemConfig_.disableNumKeyAppDeviceTypes.clear();
    ImeInfoInquirer::GetInstance().systemConfig_.disableNumKeyAppDeviceTypes.insert(testDeviceType);
    std::unordered_set<std::string> ret = ImeInfoInquirer::GetInstance().GetDisableNumKeyAppDeviceTypes();
    EXPECT_FALSE(ret.empty());
    EXPECT_EQ(ret.count(testDeviceType), 1);
}

/**
 * @tc.name: TestGetCompatibleDeviceType
 * @tc.desc: Test GetCompatibleDeviceType.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodPrivateMemberTest, TestGetCompatibleDeviceType, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest TestGetCompatibleDeviceType START");
    std::string testBundleName = "testBundleName";
    std::string testDeviceType = "testDeviceType";
    bool ret = ImeInfoInquirer::GetInstance().GetCompatibleDeviceType(testBundleName, testDeviceType);
    if (ret) {
        EXPECT_NE(testDeviceType, "testDeviceType");
    } else {
        EXPECT_EQ(testDeviceType, "testDeviceType");
    }
}

/**
 * @tc.name: TestAttachCount001
 * @tc.desc: Test TestAttachCount001.
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPrivateMemberTest, TestAttachCount001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest TestAttachCount001 TEST START");
    auto userSession = std::make_shared<PerUserSession>(MAIN_USER_ID);
    userSession->IncreaseAttachCount();
    EXPECT_EQ(userSession->GetAttachCount(), 1);
    userSession->DecreaseAttachCount();
    EXPECT_EQ(userSession->GetAttachCount(), 0);
}
 
/**
 * @tc.name: TestAttachCount002
 * @tc.desc: Test TestAttachCount002
 * @tc.type: FUNC
 */
HWTEST_F(InputMethodPrivateMemberTest, TestAttachCount002, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest TestAttachCount002 TEST START");
    auto userSession = std::make_shared<PerUserSession>(MAIN_USER_ID);
    for (uint32_t i = 0; i < MAX_ATTACH_COUNT + 1; i++) {
        userSession->IncreaseAttachCount();
    }
    EXPECT_EQ(userSession->GetAttachCount(), MAX_ATTACH_COUNT);
    for (uint32_t i = 0; i < MAX_ATTACH_COUNT + 1; i++) {
        userSession->DecreaseAttachCount();
    }
    EXPECT_EQ(userSession->GetAttachCount(), 0);
}
 
/**
 * @tc.name: SA_TestGetSecurityInputType
 * @tc.desc: SA_TestGetSecurityInputType
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodPrivateMemberTest, SA_TestGetSecurityInputType, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest::SA_TestGetSecurityInputType start.");
    InputClientInfo inputClientInfo;
    inputClientInfo.config.inputAttribute.inputPattern = InputAttribute::PATTERN_PASSWORD;
    auto ret = InputMethodPrivateMemberTest::service_->GetSecurityInputType(inputClientInfo);
    EXPECT_EQ(ret, InputType::SECURITY_INPUT);
 
    inputClientInfo.config.inputAttribute.inputPattern = InputAttribute::PATTERN_TEXT;
    ret = InputMethodPrivateMemberTest::service_->GetSecurityInputType(inputClientInfo);
    EXPECT_EQ(ret, InputType::NONE);
}
/**
 * @tc.name: SA_TestRestartIme001
 * @tc.desc: restart request will be discarded, and reartTasks will be reset
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodPrivateMemberTest, SA_TestRestartIme001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest::SA_TestRestartIme001 start.");
    auto runner = AppExecFwk::EventRunner::Create("test_RestartIme");
    auto eventHandler = std::make_shared<AppExecFwk::EventHandler>(runner);
    auto session = std::make_shared<PerUserSession>(MAIN_USER_ID, eventHandler);
 
    // attach conflict with the first scb startup event
    // restart request will be discarded, and reartTasks will be reset
    session->IncreaseAttachCount();
    EXPECT_EQ(session->GetAttachCount(), 1);
    session->AddRestartIme();
    session->AddRestartIme();
    usleep(WAIT_FOR_THREAD_SCHEDULE * MS_TO_US);
    EXPECT_EQ(session->restartTasks_, 0);
}
 
/**
 * @tc.name: SA_TestRestartIme002
 * @tc.desc: restart request will be delayed
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodPrivateMemberTest, SA_TestRestartIme002, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest::SA_TestRestartIme002 start.");
    auto runner = AppExecFwk::EventRunner::Create("test_RestartIme");
    auto eventHandler = std::make_shared<AppExecFwk::EventHandler>(runner);
    auto session = std::make_shared<PerUserSession>(MAIN_USER_ID, eventHandler);
 
    // attach conflict with no first scb startup event
    // restart request will be delayed
    session->IncreaseAttachCount();
    session->IncreaseScbStartCount();
    session->IncreaseScbStartCount();
    EXPECT_EQ(session->GetAttachCount(), 1);
    EXPECT_EQ(session->GetScbStartCount(), 2);
    session->AddRestartIme();
    usleep(WAIT_FOR_THREAD_SCHEDULE * MS_TO_US);
    // restart request be delayed, restartTasks_ no change
    EXPECT_EQ(session->restartTasks_, 1);
 
    // attach finished
    session->DecreaseAttachCount();
    usleep((WAIT_ATTACH_FINISH_DELAY + WAIT_FOR_THREAD_SCHEDULE) * MS_TO_US);
    EXPECT_EQ(session->restartTasks_, 0);
}
 
/**
 * @tc.name: SA_TestRestartIme003
 * @tc.desc: attach finished, and execute restart immediately
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodPrivateMemberTest, SA_TestRestartIme003, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest::SA_TestRestartIme003 start.");
    auto runner = AppExecFwk::EventRunner::Create("test_RestartIme");
    auto eventHandler = std::make_shared<AppExecFwk::EventHandler>(runner);
    auto session = std::make_shared<PerUserSession>(MAIN_USER_ID, eventHandler);
 
    // attach finished, and execute restart immediately
    EXPECT_EQ(session->GetAttachCount(), 0);
    session->AddRestartIme();
    usleep(WAIT_FOR_THREAD_SCHEDULE * MS_TO_US);
    EXPECT_EQ(session->restartTasks_, 0);
}

/**
 * @tc.name: SA_RestoreCurrentImeSubType
 * @tc.desc: SA_RestoreCurrentImeSubType
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodPrivateMemberTest, SA_RestoreCurrentImeSubType, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest::SA_RestoreCurrentImeSubType start.");
    InputTypeManager::GetInstance().isStarted_ = true;
    std::string bundleName = "bundleName";
    std::string extName = "extName";
    std::string subName = "subName";
    ImeEnabledCfg cfg;
    ImeEnabledInfo enabledInfo{ bundleName, extName, EnabledStatus::BASIC_MODE };
    enabledInfo.extraInfo.isDefaultIme = true;
    enabledInfo.extraInfo.currentSubName = subName;
    cfg.enabledInfos.push_back(enabledInfo);
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_.insert_or_assign(MAIN_USER_ID, cfg);

    auto session = std::make_shared<PerUserSession>(MAIN_USER_ID);
    // has no ready ime
    session->imeData_.clear();
    auto ret = session->RestoreCurrentImeSubType(0);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_FALSE(InputTypeManager::GetInstance().isStarted_);

    // has ready ime, ready ime not same with inputType ime
    InputTypeManager::GetInstance().isStarted_ = true;
    std::string bundleName1 = "bundleName1";
    std::string extName1 = "extName1";
    InputTypeManager::GetInstance().currentTypeIme_.bundleName = bundleName1;
    auto imeData = std::make_shared<ImeData>(nullptr, nullptr, nullptr, 10);
    imeData->imeStatus = ImeStatus::READY;
    imeData->ime = std::make_pair(bundleName, extName);
    std::vector<std::shared_ptr<ImeData>> imeDataList;
    imeDataList.push_back(imeData);
    session->imeData_.insert_or_assign(ImeType::IME, imeDataList);
    ret = session->RestoreCurrentImeSubType(0);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_FALSE(InputTypeManager::GetInstance().isStarted_);

    // has ready ime, ready ime same with inputType ime, ready ime same with default ime
    InputTypeManager::GetInstance().isStarted_ = true;
    InputTypeManager::GetInstance().currentTypeIme_.bundleName = bundleName;
    ret = session->RestoreCurrentImeSubType(0);
    EXPECT_EQ(ret, ErrorCode::ERROR_IME_NOT_STARTED);
    EXPECT_FALSE(InputTypeManager::GetInstance().isStarted_);

    // has ready ime, ready ime same with inputType ime, ready ime not same with default ime
    InputTypeManager::GetInstance().isStarted_ = true;
    InputTypeManager::GetInstance().currentTypeIme_.bundleName = bundleName1;
    imeData->ime = std::make_pair(bundleName1, extName1);
    imeDataList.push_back(imeData);
    session->imeData_.insert_or_assign(ImeType::IME, imeDataList);
    ret = session->RestoreCurrentImeSubType(0);
    EXPECT_EQ(ret, ErrorCode::ERROR_IME_NOT_STARTED);
    EXPECT_FALSE(InputTypeManager::GetInstance().isStarted_);
}

/**
 * @tc.name: SA_GetRealCurrentIme_004
 * @tc.desc: SA_GetRealCurrentIme_004
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodPrivateMemberTest, SA_GetRealCurrentIme_004, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest::SA_GetRealCurrentIme_004 start.");
    // input type not start, has no current client, screen locked
    std::string cmdResult;
    std::string cmd = "power-shell suspend";
    TddUtil::ExecuteCmd(cmd, cmdResult);
    sleep(1);
    InputTypeManager::GetInstance().isStarted_ = false;
    auto session = std::make_shared<PerUserSession>(MAIN_USER_ID);
    session->clientGroupMap_.clear();

    std::string bundleName1 = "bundleName1";
    std::string extName1 = "extName1";
    std::string subName1 = "subName1";
    std::string bundleName2 = "bundleName2";
    std::string extName2 = "extName2";
    ImeEnabledCfg cfg;
    ImeEnabledInfo enabledInfo{ bundleName1, extName1, EnabledStatus::BASIC_MODE };
    ImeEnabledInfo enabledInfo1{ bundleName2, extName2, EnabledStatus::BASIC_MODE };
    enabledInfo.extraInfo.isDefaultIme = true;
    enabledInfo.extraInfo.currentSubName = subName1;
    cfg.enabledInfos.push_back(enabledInfo);
    cfg.enabledInfos.push_back(enabledInfo1);
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_.insert_or_assign(MAIN_USER_ID, cfg);

    // preconfigured ime nullptr
    ImeInfoInquirer::GetInstance().systemConfig_.defaultInputMethod = "abnormal";
    auto ime = session->GetRealCurrentIme(false);
    ASSERT_NE(ime, nullptr);
    EXPECT_EQ(ime->bundleName, bundleName1);
    EXPECT_EQ(ime->subName, subName1);

    // preconfigured ime exist, default ime exist, preconfigured ime same with default ime
    ImeInfoInquirer::GetInstance().systemConfig_.defaultInputMethod = bundleName1 + "/" + extName1;
    ime = session->GetRealCurrentIme(false);
    ASSERT_NE(ime, nullptr);
    EXPECT_EQ(ime->bundleName, bundleName1);
    EXPECT_EQ(ime->subName, subName1);

    // preconfigured ime exist, default ime exist, preconfigured ime not same with default ime
    ImeInfoInquirer::GetInstance().systemConfig_.defaultInputMethod = bundleName2 + "/" + extName2;
    ime = session->GetRealCurrentIme(false);
    ASSERT_NE(ime, nullptr);
    EXPECT_EQ(ime->bundleName, bundleName2);
    EXPECT_TRUE(ime->subName.empty());
}

/**
 * @tc.name: SA_TestSysParamChanged_001
 * @tc.desc: SA_TestSysParamChanged_001
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodPrivateMemberTest, SA_TestSysParamChanged_001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest::SA_TestSysParamChanged_001 start.");
    auto ret = SystemParamAdapter::GetInstance().WatchParam("abnormal");
    EXPECT_EQ(ret, ErrorCode::ERROR_BAD_PARAMETERS);
    SystemParamAdapter::HandleSysParamChanged("key", "value", "key", 0);
    SystemParamAdapter::HandleSysParamChanged("key", "value", "abnormalKey", 0);
    InputMethodSystemAbility sysAbility;
    UserSessionManager::GetInstance().userSessions_.clear();
    sysAbility.OnSysMemChanged();
    auto userSession = std::make_shared<PerUserSession>(MAIN_USER_ID);
    UserSessionManager::GetInstance().userSessions_.insert_or_assign(MAIN_USER_ID, userSession);
    sysAbility.OnSysMemChanged();
    service_->userId_ = -1;
    ret = SystemParamAdapter::GetInstance().WatchParam(SystemParamAdapter::MEMORY_WATERMARK_KEY);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: SA_TryStartIme_001
 * @tc.desc: SA_TryStartIme_001
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodPrivateMemberTest, SA_TryStartIme_001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest::SA_TryStartIme_001 start.");
    auto userSession = std::make_shared<PerUserSession>(MAIN_USER_ID);
    userSession->isBlockStartedByLowMem_ = false;
    auto ret = userSession->TryStartIme();
    EXPECT_EQ(ret, ErrorCode::ERROR_OPERATION_NOT_ALLOWED);

    userSession->isBlockStartedByLowMem_ = true;
    auto imeData = std::make_shared<ImeData>(nullptr, nullptr, nullptr, 10);
    std::vector<std::shared_ptr<ImeData>> imeDataList;
    imeDataList.push_back(imeData);
    userSession->imeData_.insert_or_assign(ImeType::IME, imeDataList);
    ret = userSession->TryStartIme();
    EXPECT_EQ(ret, ErrorCode::ERROR_IME_HAS_STARTED);

    userSession->imeData_.clear();
    std::string bundleName1 = "bundleName1";
    std::string extName1 = "extName1";
    ImeEnabledCfg cfg;
    ImeEnabledInfo enabledInfo{ bundleName1, extName1, EnabledStatus::BASIC_MODE };
    enabledInfo.extraInfo.isDefaultIme = true;
    cfg.enabledInfos.push_back(enabledInfo);
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_.insert_or_assign(MAIN_USER_ID, cfg);

    std::string bundleName2 = "bundleName2";
    std::string extName2 = "extName2";
    ImeInfoInquirer::GetInstance().systemConfig_.defaultInputMethod = bundleName2 + "/" + extName2;
    userSession->isBlockStartedByLowMem_ = true;
    ret = userSession->TryStartIme();
    EXPECT_EQ(ret, ErrorCode::ERROR_OPERATION_NOT_ALLOWED);

    ImeInfoInquirer::GetInstance().systemConfig_.defaultInputMethod = bundleName1 + "/" + extName1;
    userSession->isBlockStartedByLowMem_ = true;
    ImeStateManagerFactory::GetInstance().ifDynamicStartIme_ = false;
    ret = userSession->TryStartIme();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    userSession->isBlockStartedByLowMem_ = true;
    ImeStateManagerFactory::GetInstance().ifDynamicStartIme_ = true;
    ret = userSession->TryStartIme();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: SA_TryDisconnectIme_001
 * @tc.desc: SA_TryDisconnectIme_001
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodPrivateMemberTest, SA_TryDisconnectIme_001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest::SA_TryDisconnectIme_001 start.");
    auto userSession = std::make_shared<PerUserSession>(MAIN_USER_ID);
    userSession->imeData_.clear();
    auto ret = userSession->TryDisconnectIme();
    EXPECT_EQ(ret, ErrorCode::ERROR_IME_NOT_STARTED);
    auto imeData = std::make_shared<ImeData>(nullptr, nullptr, nullptr, 10);
    std::vector<std::shared_ptr<ImeData>> imeDataList;
    imeDataList.push_back(imeData);
    userSession->imeData_.insert_or_assign(ImeType::IME, imeDataList);

    userSession->attachingCount_ = 1;
    ret = userSession->TryDisconnectIme();
    EXPECT_EQ(ret, ErrorCode::ERROR_OPERATION_NOT_ALLOWED);

    userSession->attachingCount_ = 0;
    auto group = std::make_shared<ClientGroup>(DEFAULT_DISPLAY_ID, nullptr);
    sptr<IInputClient> client = new (std::nothrow) InputClientServiceImpl();
    group->currentClient_ = client;
    auto info = std::make_shared<InputClientInfo>();
    group->mapClients_.insert_or_assign(client->AsObject(), info);
    userSession->clientGroupMap_.insert_or_assign(DEFAULT_DISPLAY_ID, group);
    ret = userSession->TryDisconnectIme();
    EXPECT_EQ(ret, ErrorCode::ERROR_OPERATION_NOT_ALLOWED);

    userSession->clientGroupMap_.clear();
    userSession->SetImeConnection(nullptr);
    ret = userSession->TryDisconnectIme();
    EXPECT_EQ(ret, ErrorCode::ERROR_IME_NOT_STARTED);
    sptr<AAFwk::IAbilityConnection> connection = new (std::nothrow) ImeConnection();
    userSession->SetImeConnection(connection);
    ret = userSession->TryDisconnectIme();
    EXPECT_EQ(ret, ErrorCode::ERROR_IMSA_IME_DISCONNECT_FAILED);
}

/**
 * @tc.name: SA_TestClearImeConnection_001
 * @tc.desc: SA_TestClearImeConnection_001
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodPrivateMemberTest, SA_TestClearImeConnection_001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest::SA_TestClearImeConnection_001 start.");
    sptr<AAFwk::IAbilityConnection> connection = new (std::nothrow) ImeConnection();
    sptr<AAFwk::IAbilityConnection> connection1 = new (std::nothrow) ImeConnection();
    auto userSession = std::make_shared<PerUserSession>(MAIN_USER_ID);

    userSession->SetImeConnection(connection);
    EXPECT_EQ(userSession->GetImeConnection(), connection);
    userSession->ClearImeConnection(connection);
    EXPECT_EQ(userSession->GetImeConnection(), nullptr);

    userSession->SetImeConnection(connection);
    EXPECT_EQ(userSession->GetImeConnection(), connection);
    userSession->ClearImeConnection(nullptr);
    EXPECT_EQ(userSession->GetImeConnection(), connection);

    userSession->SetImeConnection(nullptr);
    EXPECT_EQ(userSession->GetImeConnection(), nullptr);
    userSession->ClearImeConnection(connection);
    EXPECT_EQ(userSession->GetImeConnection(), nullptr);

    userSession->SetImeConnection(connection);
    EXPECT_EQ(userSession->GetImeConnection(), connection);
    userSession->ClearImeConnection(connection1);
    EXPECT_EQ(userSession->GetImeConnection(), connection);
}

/**
 * @tc.name: PerUserSession_AddImeData_001
 * @tc.desc: PerUserSession_AddImeData_001
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodPrivateMemberTest, PerUserSession_AddImeData_001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest::PerUserSession_AddImeData_001 start.");
    auto userSession = std::make_shared<PerUserSession>(MAIN_USER_ID);
    userSession->imeData_.clear();
    pid_t pid1 = 100;
    sptr<InputMethodCoreStub> coreStub1 = new (std::nothrow) InputMethodCoreServiceImpl();
    sptr<InputMethodAgentStub> agentStub1 = new (std::nothrow) InputMethodAgentServiceImpl();
    ASSERT_NE(agentStub1, nullptr);
    auto ret = userSession->AddImeData(ImeType::PROXY_IME, coreStub1, agentStub1->AsObject(), pid1);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    auto it = userSession->imeData_.find(ImeType::PROXY_IME);
    ASSERT_NE(it, userSession->imeData_.end());
    EXPECT_EQ(it->second.size(), 1);

    pid_t pid2 = 101;
    sptr<InputMethodCoreStub> coreStub2 = new (std::nothrow) InputMethodCoreServiceImpl();
    sptr<InputMethodAgentStub> agentStub2 = new (std::nothrow) InputMethodAgentServiceImpl();
    ASSERT_NE(agentStub2, nullptr);
    ret = userSession->AddImeData(ImeType::PROXY_IME, coreStub2, agentStub2->AsObject(), pid2);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    it = userSession->imeData_.find(ImeType::PROXY_IME);
    ASSERT_NE(it, userSession->imeData_.end());
    EXPECT_EQ(it->second.size(), 2);

    ret = userSession->AddImeData(ImeType::PROXY_IME, coreStub1, agentStub1->AsObject(), pid1);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    it = userSession->imeData_.find(ImeType::PROXY_IME);
    ASSERT_NE(it, userSession->imeData_.end());
    ASSERT_EQ(it->second.size(), 2);
    EXPECT_EQ(it->second[1]->pid, pid1);
}

/**
 * @tc.name: PerUserSession_AddImeData_002
 * @tc.desc: PerUserSession_AddImeData_002
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodPrivateMemberTest, PerUserSession_AddImeData_002, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest::PerUserSession_AddImeData_002 start.");
    auto userSession = std::make_shared<PerUserSession>(MAIN_USER_ID);
    userSession->imeData_.clear();
    pid_t pid1 = 100;
    sptr<InputMethodCoreStub> coreStub1 = new (std::nothrow) InputMethodCoreServiceImpl();
    sptr<InputMethodAgentStub> agentStub1 = new (std::nothrow) InputMethodAgentServiceImpl();
    ASSERT_NE(agentStub1, nullptr);
    userSession->isFirstPreemption_= true;
    auto ret = userSession->AddImeData(ImeType::PROXY_IME, coreStub1, agentStub1->AsObject(), pid1);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    auto it = userSession->imeData_.find(ImeType::PROXY_IME);
    ASSERT_NE(it, userSession->imeData_.end());
    EXPECT_EQ(it->second.size(), 1);

    userSession->isFirstPreemption_= false;
    pid_t pid2 = 101;
    sptr<InputMethodCoreStub> coreStub2 = new (std::nothrow) InputMethodCoreServiceImpl();
    sptr<InputMethodAgentStub> agentStub2 = new (std::nothrow) InputMethodAgentServiceImpl();
    auto imeData = std::make_shared<ImeData>(coreStub2, agentStub2, nullptr, pid2);
    ASSERT_NE(agentStub2, nullptr);
    ret = userSession->AddImeData(ImeType::PROXY_IME, coreStub2, agentStub2->AsObject(), pid2);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    it = userSession->imeData_.find(ImeType::PROXY_IME);
    ASSERT_NE(it, userSession->imeData_.end());
    EXPECT_EQ(it->second.size(), 2);

    userSession->isFirstPreemption_= false;
    ret = userSession->AddImeData(ImeType::PROXY_IME, coreStub1, agentStub1->AsObject(), pid1);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    it = userSession->imeData_.find(ImeType::PROXY_IME);
    ASSERT_NE(it, userSession->imeData_.end());
    ASSERT_EQ(it->second.size(), 2);
    EXPECT_EQ(it->second[1]->pid, pid1);

    sptr<InputMethodCoreStub> coreStub4 = new (std::nothrow) InputMethodCoreServiceImpl();
    sptr<InputMethodAgentStub> agentStub4 = new (std::nothrow) InputMethodAgentServiceImpl();
    ASSERT_NE(agentStub4, nullptr);
    userSession->isFirstPreemption_= true;
    pid_t pid4 = 104;
    ret = userSession->AddImeData(ImeType::PROXY_IME, coreStub4, agentStub4->AsObject(), pid4);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    it = userSession->imeData_.find(ImeType::PROXY_IME);
    ASSERT_NE(it, userSession->imeData_.end());
    ASSERT_EQ(it->second.size(), 3);
    EXPECT_EQ(it->second[2]->pid, pid4);
}

/**
 * @tc.name: PerUserSession_GetImeData_001
 * @tc.desc: imeData_ info abnormal
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodPrivateMemberTest, PerUserSession_GetImeData_001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest::PerUserSession_GetImeData_001 start.");
    auto userSession = std::make_shared<PerUserSession>(MAIN_USER_ID);
    userSession->imeData_.clear();
    auto imeData = userSession->GetImeData(ImeType::PROXY_IME);
    EXPECT_EQ(imeData, nullptr);
    pid_t pid1 = 101;
    imeData = userSession->GetImeData(pid1);
    EXPECT_EQ(imeData, nullptr);

    userSession->imeData_.insert_or_assign(ImeType::IME, std::vector<std::shared_ptr<ImeData>>{});
    imeData = userSession->GetImeData(ImeType::PROXY_IME);
    EXPECT_EQ(imeData, nullptr);
    imeData = userSession->GetImeData(pid1);
    EXPECT_EQ(imeData, nullptr);

    userSession->imeData_.insert_or_assign(ImeType::PROXY_IME, std::vector<std::shared_ptr<ImeData>>{});
    imeData = userSession->GetImeData(ImeType::PROXY_IME);
    EXPECT_EQ(imeData, nullptr);
    imeData = userSession->GetImeData(pid1);
    EXPECT_EQ(imeData, nullptr);
}

/**
 * @tc.name: PerUserSession_GetImeData_002
 * @tc.desc: imeData_ normal
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodPrivateMemberTest, PerUserSession_GetImeData_002, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest::PerUserSession_GetImeData_002 start.");
    auto userSession = std::make_shared<PerUserSession>(MAIN_USER_ID);
    userSession->imeData_.clear();
    pid_t pid1 = 101;
    sptr<InputMethodCoreStub> coreStub1 = new (std::nothrow) InputMethodCoreServiceImpl();
    sptr<InputMethodAgentStub> agentStub1 = new (std::nothrow) InputMethodAgentServiceImpl();
    ASSERT_NE(agentStub1, nullptr);
    userSession->AddImeData(ImeType::PROXY_IME, coreStub1, agentStub1->AsObject(), pid1);

    pid_t pid2 = 102;
    sptr<InputMethodCoreStub> coreStub2 = new (std::nothrow) InputMethodCoreServiceImpl();
    sptr<InputMethodAgentStub> agentStub2 = new (std::nothrow) InputMethodAgentServiceImpl();
    ASSERT_NE(agentStub2, nullptr);
    userSession->AddImeData(ImeType::PROXY_IME, coreStub2, agentStub2->AsObject(), pid2);
    auto it = userSession->imeData_.find(ImeType::PROXY_IME);
    ASSERT_NE(it, userSession->imeData_.end());
    EXPECT_EQ(it->second.size(), 2);

    auto imeData = userSession->GetImeData(ImeType::IME_MIRROR);
    EXPECT_EQ(imeData, nullptr);
    imeData = userSession->GetImeData(ImeType::PROXY_IME);
    ASSERT_NE(imeData, nullptr);
    EXPECT_EQ(imeData->pid, pid2);

    pid_t pid3 = 103;
    imeData = userSession->GetImeData(pid3);
    EXPECT_EQ(imeData, nullptr);
    imeData = userSession->GetImeData(pid1);
    ASSERT_NE(imeData, nullptr);
    EXPECT_EQ(imeData->pid, pid1);
}

/**
 * @tc.name: PerUserSession_RemoveImeData_001
 * @tc.desc: Remove imeData by pid
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodPrivateMemberTest, PerUserSession_RemoveImeData_001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest::PerUserSession_RemoveImeData_001 start.");
    auto userSession = std::make_shared<PerUserSession>(MAIN_USER_ID);
    userSession->imeData_.clear();
    pid_t pid1 = 101;
    sptr<InputMethodCoreStub> coreStub1 = new (std::nothrow) InputMethodCoreServiceImpl();
    sptr<InputMethodAgentStub> agentStub1 = new (std::nothrow) InputMethodAgentServiceImpl();
    ASSERT_NE(agentStub1, nullptr);
    userSession->AddImeData(ImeType::PROXY_IME, coreStub1, agentStub1->AsObject(), pid1);

    pid_t pid2 = 102;
    sptr<InputMethodCoreStub> coreStub2 = new (std::nothrow) InputMethodCoreServiceImpl();
    sptr<InputMethodAgentStub> agentStub2 = new (std::nothrow) InputMethodAgentServiceImpl();
    ASSERT_NE(agentStub2, nullptr);
    userSession->AddImeData(ImeType::PROXY_IME, coreStub2, agentStub2->AsObject(), pid2);
    auto it = userSession->imeData_.find(ImeType::PROXY_IME);
    ASSERT_NE(it, userSession->imeData_.end());
    EXPECT_EQ(it->second.size(), 2);

    pid_t pid3 = 103;
    userSession->RemoveImeData(pid3);
    it = userSession->imeData_.find(ImeType::PROXY_IME);
    ASSERT_NE(it, userSession->imeData_.end());
    EXPECT_EQ(it->second.size(), 2);

    userSession->RemoveImeData(pid2);
    it = userSession->imeData_.find(ImeType::PROXY_IME);
    ASSERT_NE(it, userSession->imeData_.end());
    ASSERT_EQ(it->second.size(), 1);
    EXPECT_EQ(it->second[0]->pid, pid1);

    userSession->RemoveImeData(pid1);
    it = userSession->imeData_.find(ImeType::PROXY_IME);
    EXPECT_EQ(it, userSession->imeData_.end());
}

/**
 * @tc.name: PerUserSession_RemoveImeData_002
 * @tc.desc: Remove imeData by imeType
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodPrivateMemberTest, PerUserSession_RemoveImeData_002, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest::PerUserSession_RemoveImeData_002 start.");
    auto userSession = std::make_shared<PerUserSession>(MAIN_USER_ID);
    userSession->imeData_.clear();
    pid_t pid1 = 101;
    sptr<InputMethodCoreStub> coreStub1 = new (std::nothrow) InputMethodCoreServiceImpl();
    sptr<InputMethodAgentStub> agentStub1 = new (std::nothrow) InputMethodAgentServiceImpl();
    ASSERT_NE(agentStub1, nullptr);
    userSession->AddImeData(ImeType::PROXY_IME, coreStub1, agentStub1->AsObject(), pid1);

    pid_t pid2 = 102;
    sptr<InputMethodCoreStub> coreStub2 = new (std::nothrow) InputMethodCoreServiceImpl();
    sptr<InputMethodAgentStub> agentStub2 = new (std::nothrow) InputMethodAgentServiceImpl();
    ASSERT_NE(agentStub2, nullptr);
    userSession->AddImeData(ImeType::PROXY_IME, coreStub2, agentStub2->AsObject(), pid2);
    auto it = userSession->imeData_.find(ImeType::PROXY_IME);
    ASSERT_NE(it, userSession->imeData_.end());
    EXPECT_EQ(it->second.size(), 2);

    userSession->RemoveImeData(ImeType::IME_MIRROR);
    it = userSession->imeData_.find(ImeType::PROXY_IME);
    ASSERT_NE(it, userSession->imeData_.end());
    EXPECT_EQ(it->second.size(), 2);

    userSession->RemoveImeData(ImeType::PROXY_IME);
    it = userSession->imeData_.find(ImeType::PROXY_IME);
    EXPECT_EQ(it, userSession->imeData_.end());
}

/**
 * @tc.name: IMSA_IsTmpIme
 * @tc.desc: IMSA_IsTmpIme
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodPrivateMemberTest, IMSA_IsTmpIme, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest::IMSA_IsTmpIme start.");
    InputMethodSystemAbility systemAbility;
    UserSessionManager::GetInstance().userSessions_.clear();
    uint32_t tokenId = 345;
    // not has MAIN_USER_ID userSession
    auto ret = systemAbility.IsTmpIme(MAIN_USER_ID, tokenId);
    EXPECT_FALSE(ret);
    auto userSession = std::make_shared<PerUserSession>(MAIN_USER_ID);
    UserSessionManager::GetInstance().userSessions_.insert_or_assign(MAIN_USER_ID, userSession);
    ImeEnabledCfg cfg;
    ImeEnabledInfo enabledInfo{ "", "extName1", EnabledStatus::BASIC_MODE };
    enabledInfo.extraInfo.isDefaultIme = true;
    cfg.enabledInfos.push_back(enabledInfo);
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_.insert_or_assign(MAIN_USER_ID, cfg);
    // has no running ime
    userSession->imeData_.clear();
    ret = systemAbility.IsTmpIme(MAIN_USER_ID, tokenId);
    EXPECT_FALSE(ret);
    auto imeData = std::make_shared<ImeData>(nullptr, nullptr, nullptr, 10);
    std::string bundleName2 = "bundleName2";
    imeData->ime.first = bundleName2;
    userSession->imeData_.insert_or_assign(ImeType::IME, std::vector<std::shared_ptr<ImeData>>{ imeData });
    FullImeInfo info;
    info.tokenId = tokenId;
    info.prop.name = bundleName2;
    FullImeInfoManager::GetInstance().fullImeInfos_.insert({ MAIN_USER_ID, { info } });
    // caller is same with running ime, but the bundleName of default ime is empty
    ret = systemAbility.IsTmpIme(MAIN_USER_ID, tokenId);
    EXPECT_FALSE(ret);
    enabledInfo.bundleName = bundleName2;
    cfg.enabledInfos.clear();
    cfg.enabledInfos.push_back(enabledInfo);
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_.insert_or_assign(MAIN_USER_ID, cfg);
    // caller is same with running ime, the bundleName of default ime is also same with
    ret = systemAbility.IsTmpIme(MAIN_USER_ID, tokenId);
    EXPECT_FALSE(ret);
    enabledInfo.bundleName = "diffBundleName";
    cfg.enabledInfos.clear();
    cfg.enabledInfos.push_back(enabledInfo);
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_.insert_or_assign(MAIN_USER_ID, cfg);
    // caller is same with running ime, but the bundleName of default ime is not same with
    ret = systemAbility.IsTmpIme(MAIN_USER_ID, tokenId);
    EXPECT_TRUE(ret);
    SwitchInfo switchInfo;
    switchInfo.bundleName = bundleName2;
    ret = systemAbility.IsTmpImeSwitchSubtype(MAIN_USER_ID, tokenId, switchInfo);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: PerUserSession_GetFinalCallingWindowInfo
 * @tc.desc: PerUserSession_GetFinalCallingWindowInfo
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InputMethodPrivateMemberTest, PerUserSession_GetFinalCallingWindowInfo, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest::PerUserSession_GetFinalCallingWindowInfo start.");
    auto userSession = std::make_shared<PerUserSession>(MAIN_USER_ID);
    Rosen::CallingWindowInfo callingWindowInfo;
    Rosen::FocusChangeInfo focusInfo;
    WindowAdapter::GetFocusInfo(focusInfo);
    WindowAdapter::GetCallingWindowInfo(focusInfo.windowId_, userSession->userId_, callingWindowInfo);
    auto screenName = DisplayAdapter::GetDisplayName(callingWindowInfo.displayId_);
    ImeInfoInquirer::GetInstance().systemConfig_.defaultMainDisplayScreenList.insert(screenName);
    InputClientInfo clientInfo;
    clientInfo.config.windowId = focusInfo.windowId_;
    ImfCallingWindowInfo windowInfo = userSession->GetFinalCallingWindowInfo(clientInfo);
    EXPECT_TRUE(windowInfo.displayId == DisplayAdapter::GetDefaultDisplayId());
}
} // namespace MiscServices
} // namespace OHOS