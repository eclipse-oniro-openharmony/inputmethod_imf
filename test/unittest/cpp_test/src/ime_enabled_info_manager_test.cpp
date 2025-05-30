/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#include "ime_enabled_info_manager.h"

#include "enable_upgrade_manager.h"
#include "settings_data_utils.h"
#undef private

#include <gtest/gtest.h>

#include <algorithm>
#include <condition_variable>
#include <map>

#include "file_operator.h"
#include "ime_info_inquirer.h"
#include "parameter.h"
#include "tdd_util.h"

using namespace testing::ext;
using namespace OHOS::DataShare;
namespace OHOS {
namespace MiscServices {
struct ImeFixedInfo {
    std::string bundleName;
    std::string extensionName;
};

struct ImeEasyInfo {
    std::string imeKey;
    EnabledStatus status{ EnabledStatus::DISABLED };
    bool isDefaultIme{ false };
    bool isDefaultImeSet{ false };
    bool isTmpIme{ false };
    std::string currentSubName;
};

struct UserImeConfigTest : public Serializable {
    std::map<std::string, std::vector<std::string>> cfgs;
    bool Marshal(cJSON *node) const override
    {
        bool ret = true;
        for (const auto &cfg : cfgs) {
            ret = SetValue(node, cfg.first, cfg.second) && ret;
        }
        return ret;
    }
};

struct EnabledImeCfgTest : public Serializable {
    UserImeConfigTest userImeCfg;
    bool Marshal(cJSON *node) const override
    {
        return SetValue(node, GET_NAME(enableImeList), userImeCfg);
    }
};

struct SecurityModeCfgTest : public Serializable {
    UserImeConfigTest userImeCfg;
    bool Marshal(cJSON *node) const override
    {
        return SetValue(node, GET_NAME(fullExperienceList), userImeCfg);
    }
};
class ImeEnabledInfoManagerTest : public testing::Test {
public:
    static constexpr const char *ENABLE_IME = "settings.inputmethod.enable_ime";
    static constexpr const char *SECURITY_MODE = "settings.inputmethod.full_experience";
    static constexpr const char *SYS_IME_KEY = "default_ime_key";
    static constexpr const char *SYS_IME_CUR_SUBNAME = "sysSubName";
    static constexpr const char *IME_KEY1 = "key1";
    static constexpr const char *IME_KEY2 = "key2";
    static constexpr const char *IME_KEY3 = "key3";
    static constexpr const char *BUNDLE_NAME1 = "bundleName1";
    static constexpr const char *BUNDLE_NAME2 = "bundleName2";
    static constexpr const char *BUNDLE_NAME3 = "com.example.newTestIme";
    static constexpr const char *EXT_NAME1 = "extName1";
    static constexpr const char *EXT_NAME2 = "extName2";
    static constexpr const char *EXT_NAME3 = "InputMethodExtAbility";
    static constexpr const char *CUR_SUBNAME1 = "subName1";
    static constexpr const char *CUR_SUBNAME2 = "subName2";
    static constexpr int32_t USER_ID1 = 1;
    static constexpr int32_t WAIT_DATA_SHARE_CB_TIMEOUT = 300;
    static constexpr const char *IME_CFG_FILE_PATH = "/data/service/el1/public/imf/ime_cfg.json";
    static ImeNativeCfg sysImeProp_;
    static int32_t currentUserId_;
    static std::map<int32_t, ImeEnabledCfg> enabledCfg_;
    static void SetUpTestCase();
    static void TearDownTestCase();
    static void DataShareCallback();
    void SetUp();
    void TearDown();
    static bool WaitDataShareCallback(const std::map<int32_t, ImeEnabledCfg> &enabledCfg);
    static std::map<int32_t, std::vector<FullImeInfo>> GenerateFullImeInfos(
        const std::map<int32_t, std::vector<std::string>> &easyInfos);
    static std::vector<FullImeInfo> GenerateFullImeInfos(const std::vector<std::string> &imeKeys);
    static FullImeInfo GenerateFullImeInfo(const std::string &imeKey);
    static std::map<int32_t, ImeEnabledCfg> GenerateAllEnabledCfg(
        const std::map<int32_t, std::vector<ImeEasyInfo>> &easyEnabledInfos);
    static ImeEnabledCfg GenerateAllEnabledCfg(const std::vector<ImeEasyInfo> &easyEnabledInfos);
    static std::string GetImePersistCfg();
    static void ModImePersistCfg(const std::string &content);
    static bool IsAllSwitchOn();
    static bool IsNoSwitchOn();
    static void ModImePersistCfg(const ImePersistCfg &cfg);
    static void SetGlobalEnableTable(int32_t userId, const std::vector<std::string> &bundleNames);
    static void SetUserEnableTable(int32_t userId, const std::vector<std::string> &bundleNames);
    static void SetNewUserEnableTable(int32_t userId, const std::vector<ImeEasyInfo> &easyEnabledInfos);
    static void SetGlobalFullExperienceTable(int32_t userId, const std::vector<std::string> &bundleNames);

private:
    static std::string originalGlobalEnableImeStr_;
    static std::string originalGlobalFullExperienceStr_;
    static std::string currentUserEnableImeStr_;
    static std::mutex dataShareCbCvMutex_;
    static std::condition_variable dataShareCbCv_;
    static std::string originalImePersistCfgStr_;
    static std::map<std::string, ImeFixedInfo> imeBasicInfoMapping_;
};

void ImeEnabledInfoManagerTest::SetUpTestCase()
{
    TddUtil::GrantNativePermission();
    ImeInfoInquirer::GetInstance().InitSystemConfig();
    IMSA_HILOGI("sys cfg:[%{public}d,%{public}d,%{public}d].",
        ImeInfoInquirer::GetInstance().GetSystemConfig().enableInputMethodFeature,
        ImeInfoInquirer::GetInstance().GetSystemConfig().enableFullExperienceFeature,
        ImeInfoInquirer::GetInstance().GetSystemConfig().initEnabledState);
    currentUserId_ = TddUtil::GetCurrentUserId();
    std::string uriProxy = SETTINGS_USER_DATA_URI + std::to_string(currentUserId_) + "?Proxy=true";
    auto ret = SettingsDataUtils::GetInstance().CreateAndRegisterObserver(
        uriProxy, ENABLE_IME, []() { DataShareCallback(); });
    IMSA_HILOGI("CreateAndRegisterObserver ret:%{public}d.", ret);

    originalGlobalFullExperienceStr_ = TddUtil::GetGlobalTable(SECURITY_MODE);
    IMSA_HILOGI("originalGlobalFullExperienceStr_:%{public}s.", originalGlobalFullExperienceStr_.c_str());
    originalGlobalEnableImeStr_ = TddUtil::GetGlobalTable(ENABLE_IME);
    IMSA_HILOGI("originalGlobalEnableImeStr_:%{public}s.", originalGlobalEnableImeStr_.c_str());
    currentUserEnableImeStr_ = TddUtil::GetUserTable(currentUserId_, ENABLE_IME);
    IMSA_HILOGI("currentUserEnableImeStr_:%{public}s.", currentUserEnableImeStr_.c_str());
    originalImePersistCfgStr_ = GetImePersistCfg();

    SettingsDataUtils::GetInstance().isDataShareReady_ = true;
    sysImeProp_ = ImeInfoInquirer::GetInstance().GetDefaultIme();
    imeBasicInfoMapping_ = {
        { IME_KEY1, { BUNDLE_NAME1, EXT_NAME1 } },
        { IME_KEY2, { BUNDLE_NAME2, EXT_NAME2 } },
        { IME_KEY3, { BUNDLE_NAME3, EXT_NAME3 } },
        { SYS_IME_KEY,
            {
                sysImeProp_.bundleName,
                sysImeProp_.extName,
            } },
    };
}

void ImeEnabledInfoManagerTest::TearDownTestCase()
{
    if (!originalGlobalFullExperienceStr_.empty()) {
        TddUtil::SetGlobalTable(SECURITY_MODE, originalGlobalFullExperienceStr_);
    }
    if (!originalGlobalEnableImeStr_.empty()) {
        TddUtil::SetGlobalTable(ENABLE_IME, originalGlobalEnableImeStr_);
    }
    if (!currentUserEnableImeStr_.empty()) {
        TddUtil::SetUserTable(currentUserId_, ENABLE_IME, currentUserEnableImeStr_);
    }
    if (!originalImePersistCfgStr_.empty()) {
        ModImePersistCfg(originalImePersistCfgStr_);
    }
}

void ImeEnabledInfoManagerTest::SetUp()
{
    TddUtil::DeleteGlobalTable(ENABLE_IME);
    TddUtil::DeleteGlobalTable(SECURITY_MODE);
    TddUtil::DeleteUserTable(currentUserId_, ENABLE_IME);
    ModImePersistCfg("");
    enabledCfg_.clear();
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_.clear();
    EnableUpgradeManager::GetInstance().upgradedUserId_.clear();
}

void ImeEnabledInfoManagerTest::TearDown()
{
    TddUtil::DeleteGlobalTable(ENABLE_IME);
    TddUtil::DeleteGlobalTable(SECURITY_MODE);
    TddUtil::DeleteUserTable(currentUserId_, ENABLE_IME);
    ModImePersistCfg("");
    enabledCfg_.clear();
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_.clear();
    EnableUpgradeManager::GetInstance().upgradedUserId_.clear();
}

void ImeEnabledInfoManagerTest::DataShareCallback()
{
    IMSA_HILOGI("start.");
    std::unique_lock<std::mutex> lock(dataShareCbCvMutex_);
    std::string content;
    ImeEnabledCfg enabledCfg;
    content = TddUtil::GetUserTable(currentUserId_, ENABLE_IME);
    if (!enabledCfg.Unmarshall(content)) {
        IMSA_HILOGE("%{public}d Unmarshall failed.", currentUserId_);
    } else {
        enabledCfg_.insert_or_assign(currentUserId_, enabledCfg);
    }
    IMSA_HILOGI("notify.");
    dataShareCbCv_.notify_one();
}

bool ImeEnabledInfoManagerTest::WaitDataShareCallback(const std::map<int32_t, ImeEnabledCfg> &enabledCfg)
{
    std::unique_lock<std::mutex> lock(dataShareCbCvMutex_);
    dataShareCbCv_.wait_for(lock, std::chrono::milliseconds(WAIT_DATA_SHARE_CB_TIMEOUT), [&enabledCfg]() {
        return enabledCfg == enabledCfg_ && enabledCfg_ == ImeEnabledInfoManager::GetInstance().imeEnabledCfg_;
    });
    for (const auto &cfg : enabledCfg) {
        IMSA_HILOGI("enabledCfg base info:[%{public}d, %{public}s].", cfg.first, cfg.second.version.c_str());
        for (const auto &info : cfg.second.enabledInfos) {
            IMSA_HILOGI("enabledCfg info:[%{public}s,%{public}s,%{public}d].", info.bundleName.c_str(),
                info.extensionName.c_str(), info.enabledStatus);
        }
    }
    for (const auto &cfg : enabledCfg_) {
        IMSA_HILOGI("enabledCfg_ base info:[%{public}d, %{public}s].", cfg.first, cfg.second.version.c_str());
        for (const auto &info : cfg.second.enabledInfos) {
            IMSA_HILOGI("enabledCfg_ info:[%{public}s,%{public}s,%{public}d].", info.bundleName.c_str(),
                info.extensionName.c_str(), info.enabledStatus);
        }
    }
    for (const auto &cfg : ImeEnabledInfoManager::GetInstance().imeEnabledCfg_) {
        IMSA_HILOGI("cache base info:[%{public}d, %{public}s].", cfg.first, cfg.second.version.c_str());
        for (const auto &info : cfg.second.enabledInfos) {
            IMSA_HILOGI("cache info:[%{public}s,%{public}s,%{public}d].", info.bundleName.c_str(),
                info.extensionName.c_str(), info.enabledStatus);
        }
    }
    if (enabledCfg != enabledCfg_) {
        IMSA_HILOGI("enabledCfg not same enabledCfg_.");
    }
    if (ImeEnabledInfoManager::GetInstance().imeEnabledCfg_ != enabledCfg) {
        IMSA_HILOGI("enabledCfg not same ImeEnabledInfoManager::GetInstance().imeEnabledCfg_.");
    }
    return enabledCfg == enabledCfg_ && enabledCfg_ == ImeEnabledInfoManager::GetInstance().imeEnabledCfg_;
}

std::map<int32_t, std::vector<FullImeInfo>> ImeEnabledInfoManagerTest::GenerateFullImeInfos(
    const std::map<int32_t, std::vector<std::string>> &easyInfos)
{
    std::map<int32_t, std::vector<FullImeInfo>> fullImeInfos;
    for (const auto &easyInfo : easyInfos) {
        auto imeInfos = GenerateFullImeInfos(easyInfo.second);
        fullImeInfos.insert({ easyInfo.first, imeInfos });
    }
    return fullImeInfos;
}

std::vector<FullImeInfo> ImeEnabledInfoManagerTest::GenerateFullImeInfos(const std::vector<std::string> &imeKeys)
{
    std::vector<FullImeInfo> imeInfos;
    for (const auto &key : imeKeys) {
        auto imeInfo = GenerateFullImeInfo(key);
        imeInfos.push_back(imeInfo);
    }
    return imeInfos;
}

FullImeInfo ImeEnabledInfoManagerTest::GenerateFullImeInfo(const std::string &imeKey)
{
    FullImeInfo imeInfo;
    auto it = imeBasicInfoMapping_.find(imeKey);
    if (it != imeBasicInfoMapping_.end()) {
        imeInfo.prop.name = it->second.bundleName;
        imeInfo.prop.id = it->second.extensionName;
    }
    return imeInfo;
}

std::map<int32_t, ImeEnabledCfg> ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(
    const std::map<int32_t, std::vector<ImeEasyInfo>> &easyEnabledInfos)
{
    std::map<int32_t, ImeEnabledCfg> allEnabledCfg;
    for (const auto &easyInfo : easyEnabledInfos) {
        auto enabledCfg = GenerateAllEnabledCfg(easyInfo.second);
        allEnabledCfg.insert({ easyInfo.first, enabledCfg });
    }
    return allEnabledCfg;
}

ImeEnabledCfg ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(const std::vector<ImeEasyInfo> &easyEnabledInfos)
{
    ImeEnabledCfg enabledCfg;
    enabledCfg.version = GetDisplayVersion();
    std::vector<ImeEnabledInfo> imeEnabledInfos;
    for (const auto &info : easyEnabledInfos) {
        ImeEnabledInfo imeEnabledInfo;
        std::string key = info.imeKey;
        bool hasExtName = true;
        auto pos = info.imeKey.find('/');
        if (pos != std::string::npos && pos + 1 < info.imeKey.size()) {
            key = info.imeKey.substr(0, pos);
            hasExtName = false;
        }
        auto it = imeBasicInfoMapping_.find(key);
        if (it != imeBasicInfoMapping_.end()) {
            imeEnabledInfo.bundleName = it->second.bundleName;
            if (hasExtName) {
                imeEnabledInfo.extensionName = it->second.extensionName;
            }
        }
        imeEnabledInfo.enabledStatus = info.status;
        imeEnabledInfo.extraInfo.isDefaultIme = info.isDefaultIme;
        imeEnabledInfo.extraInfo.isDefaultImeSet = info.isDefaultImeSet;
        imeEnabledInfo.extraInfo.isTmpIme = info.isTmpIme;
        imeEnabledInfo.extraInfo.currentSubName = info.currentSubName;
        imeEnabledInfos.push_back(imeEnabledInfo);
    }
    enabledCfg.enabledInfos = imeEnabledInfos;
    return enabledCfg;
}

void ImeEnabledInfoManagerTest::ModImePersistCfg(const std::string &content)
{
    if (!FileOperator::Write(IME_CFG_FILE_PATH, content, O_CREAT | O_WRONLY | O_SYNC | O_TRUNC)) {
        IMSA_HILOGE("failed to WriteJsonFile!");
    }
}

std::string ImeEnabledInfoManagerTest::GetImePersistCfg()
{
    std::string cfg;
    bool ret = FileOperator::Read(IME_CFG_FILE_PATH, cfg);
    if (!ret) {
        IMSA_HILOGE("failed to ReadJsonFile!");
    }
    return cfg;
}

bool ImeEnabledInfoManagerTest::IsAllSwitchOn()
{
    return ImeInfoInquirer::GetInstance().GetSystemConfig().enableInputMethodFeature
           && ImeInfoInquirer::GetInstance().GetSystemConfig().enableFullExperienceFeature;
}
bool ImeEnabledInfoManagerTest::IsNoSwitchOn()
{
    return !ImeInfoInquirer::GetInstance().GetSystemConfig().enableInputMethodFeature
           && !ImeInfoInquirer::GetInstance().GetSystemConfig().enableFullExperienceFeature;
}

void ImeEnabledInfoManagerTest::SetGlobalEnableTable(int32_t userId, const std::vector<std::string> &bundleNames)
{
    EnabledImeCfgTest enableCfg;
    enableCfg.userImeCfg.cfgs.insert({ std::to_string(userId), bundleNames });
    std::string enabledStr;
    enableCfg.Marshall(enabledStr);
    TddUtil::SetGlobalTable(ImeEnabledInfoManagerTest::ENABLE_IME, enabledStr);
}

void ImeEnabledInfoManagerTest::SetUserEnableTable(int32_t userId, const std::vector<std::string> &bundleNames)
{
    EnabledImeCfgTest enableCfg;
    enableCfg.userImeCfg.cfgs.insert({ std::to_string(userId), bundleNames });
    std::string userEnabledStr;
    enableCfg.Marshall(userEnabledStr);
    TddUtil::SetUserTable(userId, ImeEnabledInfoManagerTest::ENABLE_IME, userEnabledStr);
}

void ImeEnabledInfoManagerTest::SetNewUserEnableTable(int32_t userId, const std::vector<ImeEasyInfo> &easyEnabledInfos)
{
    auto enabledCfg = ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos);
    std::string userEnabledStr;
    enabledCfg.Marshall(userEnabledStr);
    TddUtil::SetUserTable(userId, ImeEnabledInfoManagerTest::ENABLE_IME, userEnabledStr);
}

void ImeEnabledInfoManagerTest::SetGlobalFullExperienceTable(
    int32_t userId, const std::vector<std::string> &bundleNames)
{
    SecurityModeCfgTest secModeCfg;
    secModeCfg.userImeCfg.cfgs.insert({ std::to_string(userId), bundleNames });
    std::string fullExperienceStr;
    secModeCfg.Marshall(fullExperienceStr);
    TddUtil::SetGlobalTable(ImeEnabledInfoManagerTest::SECURITY_MODE, fullExperienceStr);
}

void ImeEnabledInfoManagerTest::ModImePersistCfg(const ImePersistCfg &cfg)
{
    std::string content;
    cfg.Marshall(content);
    ModImePersistCfg(content);
}

std::string ImeEnabledInfoManagerTest::originalGlobalEnableImeStr_;
std::string ImeEnabledInfoManagerTest::originalGlobalFullExperienceStr_;
std::string ImeEnabledInfoManagerTest::currentUserEnableImeStr_;
std::map<int32_t, ImeEnabledCfg> ImeEnabledInfoManagerTest::enabledCfg_;
std::mutex ImeEnabledInfoManagerTest::dataShareCbCvMutex_;
std::condition_variable ImeEnabledInfoManagerTest::dataShareCbCv_;
ImeNativeCfg ImeEnabledInfoManagerTest::sysImeProp_;
int32_t ImeEnabledInfoManagerTest::currentUserId_;
std::string ImeEnabledInfoManagerTest::originalImePersistCfgStr_;
std::map<std::string, ImeFixedInfo> ImeEnabledInfoManagerTest::imeBasicInfoMapping_;

/**
 * @tc.name: testInit_001
 * @tc.desc: Init:upgrade with no table
 *                has sys ime
 *                test1:AllSwitchOn--sys ime is BASIC_MODE, sys ime is default ime
 *                test2:NoSwitchOn--sys ime is FULL_EXPERIENCE_MODE, sys ime is default ime
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImeEnabledInfoManagerTest, testInit_001, TestSize.Level0)
{
    IMSA_HILOGI("ImeEnabledInfoManagerTest testInit_001 START");
    auto ret = ImeEnabledInfoManager::GetInstance().Init(ImeEnabledInfoManagerTest::GenerateFullImeInfos(
        { { ImeEnabledInfoManagerTest::currentUserId_, { ImeEnabledInfoManagerTest::SYS_IME_KEY } } }));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    if (ImeEnabledInfoManagerTest::IsAllSwitchOn()) {
        std::map<int32_t, std::vector<ImeEasyInfo>> easyEnabledInfos;
        easyEnabledInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_,
            { { ImeEnabledInfoManagerTest::SYS_IME_KEY, EnabledStatus::BASIC_MODE, true } } });
        EXPECT_TRUE(ImeEnabledInfoManagerTest::WaitDataShareCallback(
            ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos)));
    }
    if (ImeEnabledInfoManagerTest::IsNoSwitchOn()) {
        std::map<int32_t, std::vector<ImeEasyInfo>> easyEnabledInfos;
        easyEnabledInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_,
            { { ImeEnabledInfoManagerTest::SYS_IME_KEY, EnabledStatus::FULL_EXPERIENCE_MODE, true } } });
        EXPECT_TRUE(ImeEnabledInfoManagerTest::WaitDataShareCallback(
            ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos)));
    }
}

/**
 * @tc.name: testInit_002
 * @tc.desc: Init:upgrade with no table
 *                has sys ime、BUNDLE_NAME1
 *                BUNDLE_NAME1 is default ime that user set
 *                test1:NoSwitchOn--sys ime is FULL_EXPERIENCE_MODE
 *                test2:NoSwitchOn--BUNDLE_NAME1 is FULL_EXPERIENCE_MODE, default ime can retain
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImeEnabledInfoManagerTest, testInit_002, TestSize.Level0)
{
    IMSA_HILOGI("ImeEnabledInfoManagerTest testInit_002 START");
    ImePersistCfg cfg;
    cfg.imePersistInfo.emplace_back(ImeEnabledInfoManagerTest::currentUserId_,
        std::string(ImeEnabledInfoManagerTest::BUNDLE_NAME1) + "/" + ImeEnabledInfoManagerTest::EXT_NAME1, "", true);
    ImeEnabledInfoManagerTest::ModImePersistCfg(cfg);

    auto ret = ImeEnabledInfoManager::GetInstance().Init(
        ImeEnabledInfoManagerTest::GenerateFullImeInfos({ { ImeEnabledInfoManagerTest::currentUserId_,
            { ImeEnabledInfoManagerTest::SYS_IME_KEY, ImeEnabledInfoManagerTest::IME_KEY1 } } }));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    if (ImeEnabledInfoManagerTest::IsNoSwitchOn()) {
        std::map<int32_t, std::vector<ImeEasyInfo>> easyEnabledInfos;
        easyEnabledInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_,
            { { ImeEnabledInfoManagerTest::SYS_IME_KEY, EnabledStatus::FULL_EXPERIENCE_MODE },
                { ImeEnabledInfoManagerTest::IME_KEY1, EnabledStatus::FULL_EXPERIENCE_MODE, true, true } } });
        EXPECT_TRUE(ImeEnabledInfoManagerTest::WaitDataShareCallback(
            ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos)));
    }
}

/**
 * @tc.name: testInit_003
 * @tc.desc: Init:upgrade with global enable table and full experience table
 *                has sys ime、BUNDLE_NAME1
 *                BUNDLE_NAME1 in full experience table, is default ime that user set
 *                BUNDLE_NAME2 in enable table
 *                test1:BUNDLE_NAME1 in full experience table， not in enable table, status is FULL_EXPERIENCE_MODE
 *                test2:BUNDLE_NAME2 in enable table, but not installed, status is BASIC_MODE
 *                test3:BUNDLE_NAME1 is default ime that user set, can retain
 *                test4:sys ime in no table, default BASIC_MODE
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImeEnabledInfoManagerTest, testInit_003, TestSize.Level0)
{
    IMSA_HILOGI("ImeEnabledInfoManagerTest testInit_003 START");
    ImeEnabledInfoManagerTest::SetGlobalEnableTable(
        ImeEnabledInfoManagerTest::currentUserId_, { ImeEnabledInfoManagerTest::BUNDLE_NAME2 });
    ImeEnabledInfoManagerTest::SetGlobalFullExperienceTable(
        ImeEnabledInfoManagerTest::currentUserId_, { ImeEnabledInfoManagerTest::BUNDLE_NAME1 });
    ImePersistCfg cfg;
    cfg.imePersistInfo.emplace_back(ImeEnabledInfoManagerTest::currentUserId_,
        std::string(ImeEnabledInfoManagerTest::BUNDLE_NAME1) + "/" + ImeEnabledInfoManagerTest::EXT_NAME1, "", true);
    ImeEnabledInfoManagerTest::ModImePersistCfg(cfg);

    std::map<int32_t, std::vector<std::string>> easyInfos;
    easyInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_, {
                                                                      ImeEnabledInfoManagerTest::SYS_IME_KEY,
                                                                      ImeEnabledInfoManagerTest::IME_KEY1,
                                                                  } });
    auto ret = ImeEnabledInfoManager::GetInstance().Init(ImeEnabledInfoManagerTest::GenerateFullImeInfos(easyInfos));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    if (ImeEnabledInfoManagerTest::IsAllSwitchOn()) {
        std::map<int32_t, std::vector<ImeEasyInfo>> easyEnabledInfos;
        easyEnabledInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_,
            { { std::string(ImeEnabledInfoManagerTest::IME_KEY2) + "/" + "noExtName", EnabledStatus::BASIC_MODE },
                { ImeEnabledInfoManagerTest::IME_KEY1, EnabledStatus::FULL_EXPERIENCE_MODE, true, true },
                { ImeEnabledInfoManagerTest::SYS_IME_KEY, EnabledStatus::BASIC_MODE } } });

        EXPECT_TRUE(ImeEnabledInfoManagerTest::WaitDataShareCallback(
            ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos)));
    }
}

/**
 * @tc.name: testInit_004
 * @tc.desc: Init:upgrade only with global enable table
 *                has sys ime、 BUNDLE_NAME2
 *                BUNDLE_NAME1 in enable table, is default ime
 *                test1:BUNDLE_NAME1 in enable table, is default ime, but not installed, default ime mod to sys ime
 *                test2:BUNDLE_NAME2 in no table, status is DISABLED
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImeEnabledInfoManagerTest, testInit_004, TestSize.Level0)
{
    IMSA_HILOGI("ImeEnabledInfoManagerTest testInit_004 START");
    ImeEnabledInfoManagerTest::SetGlobalEnableTable(
        ImeEnabledInfoManagerTest::currentUserId_, { ImeEnabledInfoManagerTest::BUNDLE_NAME1 });
    ImePersistCfg cfg;
    cfg.imePersistInfo.emplace_back(ImeEnabledInfoManagerTest::currentUserId_,
        std::string(ImeEnabledInfoManagerTest::BUNDLE_NAME1) + "/" + ImeEnabledInfoManagerTest::EXT_NAME1, "", true);
    ImeEnabledInfoManagerTest::ModImePersistCfg(cfg);

    std::map<int32_t, std::vector<std::string>> easyInfos;
    easyInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_, {
                                                                      ImeEnabledInfoManagerTest::SYS_IME_KEY,
                                                                      ImeEnabledInfoManagerTest::IME_KEY2,
                                                                  } });
    auto ret = ImeEnabledInfoManager::GetInstance().Init(ImeEnabledInfoManagerTest::GenerateFullImeInfos(easyInfos));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    if (ImeEnabledInfoManagerTest::IsAllSwitchOn()) {
        std::map<int32_t, std::vector<ImeEasyInfo>> easyEnabledInfos;
        easyEnabledInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_,
            { { std::string(ImeEnabledInfoManagerTest::IME_KEY1) + "/" + "noExtName", EnabledStatus::BASIC_MODE },
                { ImeEnabledInfoManagerTest::SYS_IME_KEY, EnabledStatus::BASIC_MODE, true },
                { ImeEnabledInfoManagerTest::IME_KEY2, EnabledStatus::DISABLED } } });
        EXPECT_TRUE(ImeEnabledInfoManagerTest::WaitDataShareCallback(
            ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos)));
    }
}

/**
 * @tc.name: testInit_005
 * @tc.desc: Init:upgrade with global enable table and full experience table
 *                has sys ime、 BUNDLE_NAME1、BUNDLE_NAME2
 *                BUNDLE_NAME1 in global enable table and full experience table
 *                BUNDLE_NAME2 in no table, is default ime
 *                test1:BUNDLE_NAME1 in global enable table and full experience table, status is FULL_EXPERIENCE_MODE
 *                test2:BUNDLE_NAME2 in no table, status is DISABLED, default ime mod to sys ime
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImeEnabledInfoManagerTest, testInit_005, TestSize.Level0)
{
    IMSA_HILOGI("ImeEnabledInfoManagerTest testInit_005 START");
    ImeEnabledInfoManagerTest::SetGlobalEnableTable(
        ImeEnabledInfoManagerTest::currentUserId_, { ImeEnabledInfoManagerTest::BUNDLE_NAME1 });
    ImeEnabledInfoManagerTest::SetGlobalFullExperienceTable(
        ImeEnabledInfoManagerTest::currentUserId_, { ImeEnabledInfoManagerTest::BUNDLE_NAME1 });
    ImePersistCfg cfg;
    cfg.imePersistInfo.emplace_back(ImeEnabledInfoManagerTest::currentUserId_,
        std::string(ImeEnabledInfoManagerTest::BUNDLE_NAME2) + "/" + ImeEnabledInfoManagerTest::EXT_NAME2, "", false);
    ImeEnabledInfoManagerTest::ModImePersistCfg(cfg);

    std::map<int32_t, std::vector<std::string>> easyInfos;
    easyInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_, {
                                                                      ImeEnabledInfoManagerTest::SYS_IME_KEY,
                                                                      ImeEnabledInfoManagerTest::IME_KEY1,
                                                                      ImeEnabledInfoManagerTest::IME_KEY2,
                                                                  } });
    auto ret = ImeEnabledInfoManager::GetInstance().Init(ImeEnabledInfoManagerTest::GenerateFullImeInfos(easyInfos));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    if (ImeEnabledInfoManagerTest::IsAllSwitchOn()) {
        std::map<int32_t, std::vector<ImeEasyInfo>> easyEnabledInfos;
        easyEnabledInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_,
            { { ImeEnabledInfoManagerTest::IME_KEY1, EnabledStatus::FULL_EXPERIENCE_MODE },
                { ImeEnabledInfoManagerTest::SYS_IME_KEY, EnabledStatus::BASIC_MODE, true },
                { ImeEnabledInfoManagerTest::IME_KEY2, EnabledStatus::DISABLED } } });
        EXPECT_TRUE(ImeEnabledInfoManagerTest::WaitDataShareCallback(
            ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos)));
    }
}

/**
 * @tc.name: testInit_006
 * @tc.desc: Init:upgrade only with global enable table and full experience table
 *                has sys ime、 BUNDLE_NAME1
 *                BUNDLE_NAME1 in enable table and full experience table, is default ime
 *                enable table parse abnormal
 *                test:BUNDLE_NAME1 status is FULL_EXPERIENCE_MODE, default ime retain
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImeEnabledInfoManagerTest, testInit_006, TestSize.Level0)
{
    IMSA_HILOGI("ImeEnabledInfoManagerTest testInit_006 START");
    ImeEnabledInfoManagerTest::SetGlobalEnableTable(0, { ImeEnabledInfoManagerTest::BUNDLE_NAME1 });
    ImeEnabledInfoManagerTest::SetGlobalFullExperienceTable(
        ImeEnabledInfoManagerTest::currentUserId_, { ImeEnabledInfoManagerTest::BUNDLE_NAME1 });
    ImePersistCfg cfg;
    cfg.imePersistInfo.emplace_back(ImeEnabledInfoManagerTest::currentUserId_,
        std::string(ImeEnabledInfoManagerTest::BUNDLE_NAME1) + "/" + ImeEnabledInfoManagerTest::EXT_NAME1,
        ImeEnabledInfoManagerTest::CUR_SUBNAME1, true);
    ImeEnabledInfoManagerTest::ModImePersistCfg(cfg);

    std::map<int32_t, std::vector<std::string>> easyInfos;
    easyInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_, {
                                                                      ImeEnabledInfoManagerTest::SYS_IME_KEY,
                                                                      ImeEnabledInfoManagerTest::IME_KEY1,
                                                                  } });
    auto ret = ImeEnabledInfoManager::GetInstance().Init(ImeEnabledInfoManagerTest::GenerateFullImeInfos(easyInfos));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    if (ImeEnabledInfoManagerTest::IsAllSwitchOn()) {
        std::map<int32_t, std::vector<ImeEasyInfo>> easyEnabledInfos;
        easyEnabledInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_,
            { { ImeEnabledInfoManagerTest::IME_KEY1, EnabledStatus::FULL_EXPERIENCE_MODE, true, true, false,
                  ImeEnabledInfoManagerTest::CUR_SUBNAME1 },
                { ImeEnabledInfoManagerTest::SYS_IME_KEY, EnabledStatus::BASIC_MODE } } });
        EXPECT_TRUE(ImeEnabledInfoManagerTest::WaitDataShareCallback(
            ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos)));
    }
}

/**
 * @tc.name: testInit_007
 * @tc.desc: Init:upgrade only with global enable table and full experience table
 *                has sys ime、 BUNDLE_NAME1
 *                BUNDLE_NAME1 in enable table and full experience table, is default ime
 *                full experience table parse abnormal
 *                test:BUNDLE_NAME1 status is BASIC_MODE, default ime retain
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImeEnabledInfoManagerTest, testInit_007, TestSize.Level0)
{
    IMSA_HILOGI("ImeEnabledInfoManagerTest testInit_007 START");
    ImeEnabledInfoManagerTest::SetGlobalEnableTable(
        ImeEnabledInfoManagerTest::currentUserId_, { ImeEnabledInfoManagerTest::BUNDLE_NAME1 });
    ImeEnabledInfoManagerTest::SetGlobalFullExperienceTable(0, { ImeEnabledInfoManagerTest::BUNDLE_NAME1 });
    ImePersistCfg cfg;
    cfg.imePersistInfo.emplace_back(ImeEnabledInfoManagerTest::currentUserId_,
        std::string(ImeEnabledInfoManagerTest::BUNDLE_NAME1) + "/" + ImeEnabledInfoManagerTest::EXT_NAME1,
        ImeEnabledInfoManagerTest::CUR_SUBNAME1, true);
    ImeEnabledInfoManagerTest::ModImePersistCfg(cfg);

    std::map<int32_t, std::vector<std::string>> easyInfos;
    easyInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_, {
                                                                      ImeEnabledInfoManagerTest::SYS_IME_KEY,
                                                                      ImeEnabledInfoManagerTest::IME_KEY1,
                                                                  } });
    auto ret = ImeEnabledInfoManager::GetInstance().Init(ImeEnabledInfoManagerTest::GenerateFullImeInfos(easyInfos));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    if (ImeEnabledInfoManagerTest::IsAllSwitchOn()) {
        std::map<int32_t, std::vector<ImeEasyInfo>> easyEnabledInfos;
        easyEnabledInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_,
            { { ImeEnabledInfoManagerTest::IME_KEY1, EnabledStatus::BASIC_MODE, true, true, false,
                  ImeEnabledInfoManagerTest::CUR_SUBNAME1 },
                { ImeEnabledInfoManagerTest::SYS_IME_KEY, EnabledStatus::BASIC_MODE } } });
        EXPECT_TRUE(ImeEnabledInfoManagerTest::WaitDataShareCallback(
            ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos)));
    }
}

/**
 * @tc.name: testInit_008
 * @tc.desc: Init:upgrade with global enable table and user enable table
 *                has sys ime、BUNDLE_NAME1 and BUNDLE_NAME2
 *                sys ime in global enable table and in user enable table,is default ime
 *                BUNDLE_NAME1 in global enable table、
 *                BUNDLE_NAME2 in global enable table and in user enable table
 *                test:has user enable table, but global enable table has normal enable info of this user,
 *                      It's up to global enable table
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImeEnabledInfoManagerTest, testInit_008, TestSize.Level0)
{
    IMSA_HILOGI("ImeEnabledInfoManagerTest testInit_008 START");
    ImeEnabledInfoManagerTest::SetGlobalEnableTable(ImeEnabledInfoManagerTest::currentUserId_,
        { ImeEnabledInfoManagerTest::sysImeProp_.bundleName, ImeEnabledInfoManagerTest::BUNDLE_NAME1,
            ImeEnabledInfoManagerTest::BUNDLE_NAME2 });
    ImeEnabledInfoManagerTest::SetUserEnableTable(ImeEnabledInfoManagerTest::currentUserId_,
        { ImeEnabledInfoManagerTest::sysImeProp_.bundleName, ImeEnabledInfoManagerTest::BUNDLE_NAME2 });
    ImePersistCfg cfg;
    cfg.imePersistInfo.emplace_back(ImeEnabledInfoManagerTest::currentUserId_,
        ImeEnabledInfoManagerTest::sysImeProp_.bundleName + "/" + ImeEnabledInfoManagerTest::sysImeProp_.extName,
        ImeEnabledInfoManagerTest::SYS_IME_CUR_SUBNAME, true);
    ImeEnabledInfoManagerTest::ModImePersistCfg(cfg);

    std::map<int32_t, std::vector<std::string>> easyInfos;
    easyInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_, {
                                                                      ImeEnabledInfoManagerTest::SYS_IME_KEY,
                                                                      ImeEnabledInfoManagerTest::IME_KEY1,
                                                                      ImeEnabledInfoManagerTest::IME_KEY2,
                                                                  } });

    auto ret = ImeEnabledInfoManager::GetInstance().Init(ImeEnabledInfoManagerTest::GenerateFullImeInfos(easyInfos));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    if (ImeEnabledInfoManagerTest::IsAllSwitchOn()) {
        std::map<int32_t, std::vector<ImeEasyInfo>> easyEnabledInfos;
        easyEnabledInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_,
            { { ImeEnabledInfoManagerTest::IME_KEY1, EnabledStatus::BASIC_MODE },
                { ImeEnabledInfoManagerTest::IME_KEY2, EnabledStatus::BASIC_MODE },
                { ImeEnabledInfoManagerTest::SYS_IME_KEY, EnabledStatus::BASIC_MODE, true, true, false,
                    ImeEnabledInfoManagerTest::SYS_IME_CUR_SUBNAME } } });
        EXPECT_TRUE(ImeEnabledInfoManagerTest::WaitDataShareCallback(
            ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos)));
    }
}

/**
 * @tc.name: testInit_009
 * @tc.desc: Init:upgrade with global enable table and user enable table
 *                has sys ime and BUNDLE_NAME1
 *                sys ime in user enable table
 *                BUNDLE_NAME1 in user enable table, is default ime
 *                test:has user enable table, global enable table not has enable info of this user,
 *                      It's up to user enable table
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImeEnabledInfoManagerTest, testInit_009, TestSize.Level0)
{
    IMSA_HILOGI("ImeEnabledInfoManagerTest testInit_009 START");
    ImeEnabledInfoManagerTest::SetGlobalEnableTable(
        0, { ImeEnabledInfoManagerTest::sysImeProp_.bundleName, ImeEnabledInfoManagerTest::BUNDLE_NAME1,
               ImeEnabledInfoManagerTest::BUNDLE_NAME2 });
    ImeEnabledInfoManagerTest::SetUserEnableTable(ImeEnabledInfoManagerTest::currentUserId_,
        { ImeEnabledInfoManagerTest::sysImeProp_.bundleName, ImeEnabledInfoManagerTest::BUNDLE_NAME1 });
    ImePersistCfg cfg;
    cfg.imePersistInfo.emplace_back(ImeEnabledInfoManagerTest::currentUserId_,
        std::string(ImeEnabledInfoManagerTest::BUNDLE_NAME1) + "/" + ImeEnabledInfoManagerTest::EXT_NAME1, "", false);
    ImeEnabledInfoManagerTest::ModImePersistCfg(cfg);

    std::map<int32_t, std::vector<std::string>> easyInfos;
    easyInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_, {
                                                                      ImeEnabledInfoManagerTest::SYS_IME_KEY,
                                                                      ImeEnabledInfoManagerTest::IME_KEY1,
                                                                  } });
    auto ret = ImeEnabledInfoManager::GetInstance().Init(ImeEnabledInfoManagerTest::GenerateFullImeInfos(easyInfos));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    if (ImeEnabledInfoManagerTest::IsAllSwitchOn()) {
        std::map<int32_t, std::vector<ImeEasyInfo>> easyEnabledInfos;
        easyEnabledInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_,
            { { ImeEnabledInfoManagerTest::IME_KEY1, EnabledStatus::BASIC_MODE, true },
                { ImeEnabledInfoManagerTest::SYS_IME_KEY, EnabledStatus::BASIC_MODE } } });
        EXPECT_TRUE(ImeEnabledInfoManagerTest::WaitDataShareCallback(
            ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos)));
    }
}

/**
 * @tc.name: testInit_010
 * @tc.desc: Init:device reboot with new user enable table
 *                has sys ime、BUNDLE_NAME2
 *                sys ime、BUNDLE_NAME2 in global enable table
 *                sys ime、BUNDLE_NAME2 in new user enable table, all BASIC_MODE, BUNDLE_NAME2 is default ime
 *                BUNDLE_NAME2 in full experience table
 *                test:has new user table, It's up to new user enable table
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImeEnabledInfoManagerTest, testInit_010, TestSize.Level0)
{
    IMSA_HILOGI("ImeEnabledInfoManagerTest testInit_010 START");
    ImeEnabledInfoManagerTest::SetGlobalEnableTable(ImeEnabledInfoManagerTest::currentUserId_,
        { ImeEnabledInfoManagerTest::sysImeProp_.bundleName, ImeEnabledInfoManagerTest::BUNDLE_NAME2 });
    ImeEnabledInfoManagerTest::SetGlobalFullExperienceTable(
        ImeEnabledInfoManagerTest::currentUserId_, { ImeEnabledInfoManagerTest::BUNDLE_NAME2 });
    ImePersistCfg cfg;
    cfg.imePersistInfo.emplace_back(ImeEnabledInfoManagerTest::currentUserId_,
        ImeEnabledInfoManagerTest::sysImeProp_.bundleName + "/" + ImeEnabledInfoManagerTest::sysImeProp_.extName,
        ImeEnabledInfoManagerTest::SYS_IME_CUR_SUBNAME, true);
    ImeEnabledInfoManagerTest::ModImePersistCfg(cfg);

    std::vector<ImeEasyInfo> easyTableInfos = { { ImeEnabledInfoManagerTest::SYS_IME_KEY, EnabledStatus::BASIC_MODE },
        { ImeEnabledInfoManagerTest::IME_KEY2, EnabledStatus::BASIC_MODE, true } };
    ImeEnabledInfoManagerTest::SetNewUserEnableTable(ImeEnabledInfoManagerTest::currentUserId_, easyTableInfos);

    std::map<int32_t, std::vector<std::string>> easyInfos;
    easyInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_, {
                                                                      ImeEnabledInfoManagerTest::SYS_IME_KEY,
                                                                      ImeEnabledInfoManagerTest::IME_KEY2,
                                                                  } });
    auto ret = ImeEnabledInfoManager::GetInstance().Init(ImeEnabledInfoManagerTest::GenerateFullImeInfos(easyInfos));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    if (ImeEnabledInfoManagerTest::IsAllSwitchOn()) {
        std::map<int32_t, std::vector<ImeEasyInfo>> easyEnabledInfos;
        easyEnabledInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_, easyTableInfos });
        EXPECT_TRUE(ImeEnabledInfoManagerTest::WaitDataShareCallback(
            ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos)));
    }
}

/**
 * @tc.name: testInit_011
 * @tc.desc: Init:imsa restart with new user enable table
 *                has sys ime、BUNDLE_NAME2
 *                BUNDLE_NAME2 install when imsa died
 *                test:bundle install when imsa died can save in imsa restart
 *
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImeEnabledInfoManagerTest, testInit_011, TestSize.Level0)
{
    IMSA_HILOGI("ImeEnabledInfoManagerTest testInit_011 START");
    std::vector<ImeEasyInfo> easyTableInfos = { { ImeEnabledInfoManagerTest::SYS_IME_KEY, EnabledStatus::BASIC_MODE,
        true } };
    ImeEnabledInfoManagerTest::SetNewUserEnableTable(ImeEnabledInfoManagerTest::currentUserId_, easyTableInfos);
    std::map<int32_t, std::vector<std::string>> easyInfos;
    easyInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_, {
                                                                      ImeEnabledInfoManagerTest::SYS_IME_KEY,
                                                                      ImeEnabledInfoManagerTest::IME_KEY2,
                                                                  } });
    auto ret = ImeEnabledInfoManager::GetInstance().Init(ImeEnabledInfoManagerTest::GenerateFullImeInfos(easyInfos));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    if (ImeEnabledInfoManagerTest::IsAllSwitchOn()) {
        std::map<int32_t, std::vector<ImeEasyInfo>> easyEnabledInfos;
        easyEnabledInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_,
            { { ImeEnabledInfoManagerTest::SYS_IME_KEY, EnabledStatus::BASIC_MODE, true },
                { ImeEnabledInfoManagerTest::IME_KEY2,
                    ImeInfoInquirer::GetInstance().GetSystemConfig().initEnabledState } } });
        EXPECT_TRUE(ImeEnabledInfoManagerTest::WaitDataShareCallback(
            ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos)));
    }
    if (ImeEnabledInfoManagerTest::IsNoSwitchOn()) {
        std::map<int32_t, std::vector<ImeEasyInfo>> easyEnabledInfos;
        easyEnabledInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_,
            { { ImeEnabledInfoManagerTest::SYS_IME_KEY, EnabledStatus::BASIC_MODE, true },
                { ImeEnabledInfoManagerTest::IME_KEY2, EnabledStatus::FULL_EXPERIENCE_MODE } } });
        EXPECT_TRUE(ImeEnabledInfoManagerTest::WaitDataShareCallback(
            ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos)));
    }
}

/**
 * @tc.name: testInit_012
 * @tc.desc: Init:imsa restart with new user enable table
 *                BUNDLE_NAME2 is default ime , and uninstall when imsa died
 *                test:default ime uninstall when imsa died,  default ime will mode to sys ime in imsa restart
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImeEnabledInfoManagerTest, testInit_012, TestSize.Level0)
{
    IMSA_HILOGI("ImeEnabledInfoManagerTest testInit_012 START");
    std::vector<ImeEasyInfo> easyTableInfos = { { ImeEnabledInfoManagerTest::SYS_IME_KEY, EnabledStatus::BASIC_MODE },
        { ImeEnabledInfoManagerTest::IME_KEY2, EnabledStatus::BASIC_MODE, true } };
    ImeEnabledInfoManagerTest::SetNewUserEnableTable(ImeEnabledInfoManagerTest::currentUserId_, easyTableInfos);
    std::map<int32_t, std::vector<std::string>> easyInfos;
    easyInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_, { ImeEnabledInfoManagerTest::SYS_IME_KEY } });
    auto ret = ImeEnabledInfoManager::GetInstance().Init(ImeEnabledInfoManagerTest::GenerateFullImeInfos(easyInfos));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    if (ImeEnabledInfoManagerTest::IsAllSwitchOn()) {
        std::map<int32_t, std::vector<ImeEasyInfo>> easyEnabledInfos;
        easyEnabledInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_,
            { { ImeEnabledInfoManagerTest::SYS_IME_KEY, EnabledStatus::BASIC_MODE, true },
                { ImeEnabledInfoManagerTest::IME_KEY2, EnabledStatus::BASIC_MODE } } });
        EXPECT_TRUE(ImeEnabledInfoManagerTest::WaitDataShareCallback(
            ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos)));
    }
    if (ImeEnabledInfoManagerTest::IsNoSwitchOn()) {
        std::map<int32_t, std::vector<ImeEasyInfo>> easyEnabledInfos;
        easyEnabledInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_,
            { { ImeEnabledInfoManagerTest::SYS_IME_KEY, EnabledStatus::BASIC_MODE, true },
                { ImeEnabledInfoManagerTest::IME_KEY2, EnabledStatus::BASIC_MODE } } });
        EXPECT_TRUE(ImeEnabledInfoManagerTest::WaitDataShareCallback(
            ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos)));
    }
}

/**
 * @tc.name: testInit_013
 * @tc.desc: Init:imsa restart with new user enable table
 *                not has default ime before upgrade
 *                BUNDLE_NAME2 uninstall before upgrade, and install again when imsa died after upgrade
 *                test1:not has default ime, sys ime set to default ime when upgrade
 *                test2:BUNDLE_NAME2 can not obtain extName when upgrade, install again when imsa died,
 *                     can obtain extName after imsa restart
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImeEnabledInfoManagerTest, testInit_013, TestSize.Level0)
{
    IMSA_HILOGI("ImeEnabledInfoManagerTest testInit_013 START");
    ImeEnabledInfoManagerTest::SetGlobalEnableTable(
        ImeEnabledInfoManagerTest::currentUserId_, { ImeEnabledInfoManagerTest::BUNDLE_NAME2 });
    std::map<int32_t, std::vector<std::string>> easyInfos;
    easyInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_, { ImeEnabledInfoManagerTest::SYS_IME_KEY } });
    auto ret = ImeEnabledInfoManager::GetInstance().Init(ImeEnabledInfoManagerTest::GenerateFullImeInfos(easyInfos));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    if (ImeEnabledInfoManagerTest::IsAllSwitchOn()) {
        std::map<int32_t, std::vector<ImeEasyInfo>> easyEnabledInfos;
        easyEnabledInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_,
            { { std::string(ImeEnabledInfoManagerTest::IME_KEY2) + "/" + "noExtName", EnabledStatus::BASIC_MODE },
                { ImeEnabledInfoManagerTest::SYS_IME_KEY, EnabledStatus::BASIC_MODE, true } } });
        EXPECT_TRUE(ImeEnabledInfoManagerTest::WaitDataShareCallback(
            ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos)));
    }
    auto imeInfo = ImeEnabledInfoManagerTest::GenerateFullImeInfo(ImeEnabledInfoManagerTest::IME_KEY2);
    ret = ImeEnabledInfoManager::GetInstance().Add(ImeEnabledInfoManagerTest::currentUserId_, imeInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    if (ImeEnabledInfoManagerTest::IsAllSwitchOn()) {
        std::map<int32_t, std::vector<ImeEasyInfo>> easyEnabledInfos1;
        easyEnabledInfos1.insert({ ImeEnabledInfoManagerTest::currentUserId_,
            { { ImeEnabledInfoManagerTest::IME_KEY2, EnabledStatus::BASIC_MODE },
                { ImeEnabledInfoManagerTest::SYS_IME_KEY, EnabledStatus::BASIC_MODE, true } } });
        EXPECT_TRUE(ImeEnabledInfoManagerTest::WaitDataShareCallback(
            ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos1)));
    }
}

/**
 * @tc.name: testUserSwitch_001
 * @tc.desc: has cache
 *           test:user enable table will not be mod if has cache
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImeEnabledInfoManagerTest, testUserAdd_001, TestSize.Level0)
{
    IMSA_HILOGI("ImeEnabledInfoManagerTest testUserAdd_001 START");
    std::map<int32_t, std::vector<ImeEasyInfo>> easyEnabledInfos;
    easyEnabledInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_,
        { { ImeEnabledInfoManagerTest::SYS_IME_KEY, EnabledStatus::BASIC_MODE, true },
            { ImeEnabledInfoManagerTest::IME_KEY2, EnabledStatus::BASIC_MODE } } });
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_ =
        ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos);

    std::vector<std::string> imeKeys = {
        ImeEnabledInfoManagerTest::SYS_IME_KEY,
        ImeEnabledInfoManagerTest::IME_KEY2,
    };
    auto imeInfos = ImeEnabledInfoManagerTest::GenerateFullImeInfos(imeKeys);
    auto ret = ImeEnabledInfoManager::GetInstance().Switch(ImeEnabledInfoManagerTest::currentUserId_, imeInfos);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_FALSE(ImeEnabledInfoManagerTest::WaitDataShareCallback(ImeEnabledInfoManager::GetInstance().imeEnabledCfg_));
    EXPECT_TRUE(ImeEnabledInfoManagerTest::enabledCfg_.empty());
}

/**
 * @tc.name: testUserDelete_001
 * @tc.desc: has cache
 *           test:cache will be deleted when user deleted
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImeEnabledInfoManagerTest, testUserDelete_001, TestSize.Level0)
{
    IMSA_HILOGI("ImeEnabledInfoManagerTest testUserDelete_001 START");
    std::map<int32_t, std::vector<ImeEasyInfo>> easyEnabledInfos;
    easyEnabledInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_,
        { { ImeEnabledInfoManagerTest::SYS_IME_KEY, EnabledStatus::BASIC_MODE, true },
            { ImeEnabledInfoManagerTest::IME_KEY2, EnabledStatus::BASIC_MODE } } });
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_ =
        ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos);
    EXPECT_FALSE(ImeEnabledInfoManager::GetInstance().imeEnabledCfg_.empty());
    auto ret = ImeEnabledInfoManager::GetInstance().Delete(ImeEnabledInfoManagerTest::currentUserId_);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(ImeEnabledInfoManager::GetInstance().imeEnabledCfg_.empty());
}

/**
 * @tc.name: testBundleAdd_001
 * @tc.desc: BUNDLE_NAME1 is added
 *           has cache, BUNDLE_NAME1 not in cache
 *           test1: bundle add first the status is decided by initEnabledState
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImeEnabledInfoManagerTest, testBundleAdd_001, TestSize.Level0)
{
    IMSA_HILOGI("ImeEnabledInfoManagerTest testBundleAdd_001 START");
    std::map<int32_t, std::vector<ImeEasyInfo>> easyEnabledInfos;
    easyEnabledInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_,
        { { ImeEnabledInfoManagerTest::SYS_IME_KEY, EnabledStatus::BASIC_MODE, true } } });
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_ =
        ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos);
    auto imeInfo = ImeEnabledInfoManagerTest::GenerateFullImeInfo(ImeEnabledInfoManagerTest::IME_KEY2);
    auto ret = ImeEnabledInfoManager::GetInstance().Add(ImeEnabledInfoManagerTest::currentUserId_, imeInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    if (ImeEnabledInfoManagerTest::IsAllSwitchOn()) {
        std::map<int32_t, std::vector<ImeEasyInfo>> easyEnabledInfos1;
        easyEnabledInfos1.insert({ ImeEnabledInfoManagerTest::currentUserId_,
            { { ImeEnabledInfoManagerTest::SYS_IME_KEY, EnabledStatus::BASIC_MODE, true },
                { ImeEnabledInfoManagerTest::IME_KEY2,
                    ImeInfoInquirer::GetInstance().GetSystemConfig().initEnabledState } } });
        EXPECT_TRUE(ImeEnabledInfoManagerTest::WaitDataShareCallback(
            ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos1)));
    }
    if (ImeEnabledInfoManagerTest::IsNoSwitchOn()) {
        std::map<int32_t, std::vector<ImeEasyInfo>> easyEnabledInfos2;
        easyEnabledInfos2.insert({ ImeEnabledInfoManagerTest::currentUserId_,
            { { ImeEnabledInfoManagerTest::SYS_IME_KEY, EnabledStatus::BASIC_MODE, true },
                { ImeEnabledInfoManagerTest::IME_KEY2, EnabledStatus::FULL_EXPERIENCE_MODE } } });
        EXPECT_TRUE(ImeEnabledInfoManagerTest::WaitDataShareCallback(
            ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos2)));
    }
}

/**
 * @tc.name: testBundleAdd_002
 * @tc.desc: BUNDLE_NAME2 is added
 *           has cache, BUNDLE_NAME2 in cache, extensionName not empty
 *           test1: bundle not first add the status is decided by last
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImeEnabledInfoManagerTest, testBundleAdd_002, TestSize.Level0)
{
    IMSA_HILOGI("ImeEnabledInfoManagerTest testBundleAdd_002 START");
    std::map<int32_t, std::vector<ImeEasyInfo>> easyEnabledInfos;
    easyEnabledInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_,
        { { ImeEnabledInfoManagerTest::SYS_IME_KEY, EnabledStatus::BASIC_MODE, true },
            { ImeEnabledInfoManagerTest::IME_KEY2, EnabledStatus::FULL_EXPERIENCE_MODE } } });
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_ =
        ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos);
    auto imeInfo = ImeEnabledInfoManagerTest::GenerateFullImeInfo(ImeEnabledInfoManagerTest::IME_KEY2);
    auto ret = ImeEnabledInfoManager::GetInstance().Add(ImeEnabledInfoManagerTest::currentUserId_, imeInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_FALSE(ImeEnabledInfoManagerTest::WaitDataShareCallback(
        ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos)));
    EXPECT_TRUE(ImeEnabledInfoManagerTest::enabledCfg_.empty());
}

/**
 * @tc.name: testBundleAdd_003
 * @tc.desc: BUNDLE_NAME2 is added
 *           has cache, BUNDLE_NAME2 in cache, extensionName empty
 *           test1: bundle uninstall before upgrade, install again that the extName can be set
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImeEnabledInfoManagerTest, testBundleAdd_003, TestSize.Level0)
{
    IMSA_HILOGI("ImeEnabledInfoManagerTest testBundleAdd_003 START");
    std::map<int32_t, std::vector<ImeEasyInfo>> easyEnabledInfos;
    easyEnabledInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_,
        { { ImeEnabledInfoManagerTest::SYS_IME_KEY, EnabledStatus::BASIC_MODE, true },
            { std::string(ImeEnabledInfoManagerTest::IME_KEY2) + "/" + "noExtName",
                EnabledStatus::FULL_EXPERIENCE_MODE } } });
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_ =
        ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos);
    auto imeInfo = ImeEnabledInfoManagerTest::GenerateFullImeInfo(ImeEnabledInfoManagerTest::IME_KEY2);
    auto ret = ImeEnabledInfoManager::GetInstance().Add(ImeEnabledInfoManagerTest::currentUserId_, imeInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    std::map<int32_t, std::vector<ImeEasyInfo>> easyEnabledInfos1;
    easyEnabledInfos1.insert({ ImeEnabledInfoManagerTest::currentUserId_,
        { { ImeEnabledInfoManagerTest::SYS_IME_KEY, EnabledStatus::BASIC_MODE, true },
            { ImeEnabledInfoManagerTest::IME_KEY2, EnabledStatus::FULL_EXPERIENCE_MODE } } });
    EXPECT_TRUE(ImeEnabledInfoManagerTest::WaitDataShareCallback(
        ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos1)));
}

/**
 * @tc.name: testBundleDelete_001
 * @tc.desc: current ime is deleted
 *           test:current ime delete, current ime(default ime) mod to sys ime
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImeEnabledInfoManagerTest, testBundleDelete_001, TestSize.Level0)
{
    IMSA_HILOGI("ImeEnabledInfoManagerTest testBundleDelete_001 START");
    std::map<int32_t, std::vector<ImeEasyInfo>> easyEnabledInfos;
    easyEnabledInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_,
        { { ImeEnabledInfoManagerTest::SYS_IME_KEY, EnabledStatus::BASIC_MODE },
            { ImeEnabledInfoManagerTest::IME_KEY2, EnabledStatus::BASIC_MODE, true } } });
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_ =
        ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos);

    auto ret = ImeEnabledInfoManager::GetInstance().Delete(
        ImeEnabledInfoManagerTest::currentUserId_, ImeEnabledInfoManagerTest::BUNDLE_NAME2);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    std::map<int32_t, std::vector<ImeEasyInfo>> easyEnabledInfos1;
    easyEnabledInfos1.insert({ ImeEnabledInfoManagerTest::currentUserId_,
        { { ImeEnabledInfoManagerTest::SYS_IME_KEY, EnabledStatus::BASIC_MODE, true },
            { ImeEnabledInfoManagerTest::IME_KEY2, EnabledStatus::BASIC_MODE } } });
    EXPECT_TRUE(ImeEnabledInfoManagerTest::WaitDataShareCallback(
        ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos1)));
}

/**
 * @tc.name: testBundleDelete_002
 * @tc.desc: not current ime delete
 *           test:not current ime delete, no need deal
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImeEnabledInfoManagerTest, testBundleDelete_002, TestSize.Level0)
{
    IMSA_HILOGI("ImeEnabledInfoManagerTest testBundleDelete_002 START");
    std::map<int32_t, std::vector<ImeEasyInfo>> easyEnabledInfos;
    easyEnabledInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_,
        { { ImeEnabledInfoManagerTest::SYS_IME_KEY, EnabledStatus::BASIC_MODE, true },
            { ImeEnabledInfoManagerTest::IME_KEY2, EnabledStatus::BASIC_MODE } } });
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_ =
        ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos);

    auto ret = ImeEnabledInfoManager::GetInstance().Delete(
        ImeEnabledInfoManagerTest::currentUserId_, ImeEnabledInfoManagerTest::BUNDLE_NAME2);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_FALSE(ImeEnabledInfoManagerTest::WaitDataShareCallback(
        ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos)));
}

/**
 * @tc.name: testBundleEnabledStatusUpdate_001
 * @tc.desc: has cache
 *           test:user enabled table will be mod when bundle enabledStatus changed
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImeEnabledInfoManagerTest, testBundleEnabledStatusUpdate_001, TestSize.Level0)
{
    IMSA_HILOGI("ImeEnabledInfoManagerTest testBundleEnabledStatusUpdate_001 START");
    std::map<int32_t, std::vector<ImeEasyInfo>> easyEnabledInfos;
    easyEnabledInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_,
        { { ImeEnabledInfoManagerTest::IME_KEY3, EnabledStatus::BASIC_MODE } } });
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_ =
        ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos);

    auto ret = ImeEnabledInfoManager::GetInstance().Update(ImeEnabledInfoManagerTest::currentUserId_,
        BUNDLE_NAME3, EXT_NAME3, EnabledStatus::FULL_EXPERIENCE_MODE);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    if (ImeEnabledInfoManagerTest::IsAllSwitchOn()) {
        std::map<int32_t, std::vector<ImeEasyInfo>> easyEnabledInfos1;
        easyEnabledInfos1.insert({ ImeEnabledInfoManagerTest::currentUserId_,
            { { ImeEnabledInfoManagerTest::IME_KEY3, EnabledStatus::FULL_EXPERIENCE_MODE } } });
        EXPECT_TRUE(ImeEnabledInfoManagerTest::WaitDataShareCallback(
            ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos1)));
    }
}

/**
 * @tc.name: testBundleEnabledStatusUpdate_002
 * @tc.desc: test:default ime not allow DISABLED
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImeEnabledInfoManagerTest, testBundleEnabledStatusUpdate_002, TestSize.Level0)
{
    IMSA_HILOGI("ImeEnabledInfoManagerTest testBundleEnabledStatusUpdate_002 START");
    auto ret = ImeEnabledInfoManager::GetInstance().Update(ImeEnabledInfoManagerTest::currentUserId_,
        ImeEnabledInfoManagerTest::sysImeProp_.bundleName, ImeEnabledInfoManagerTest::sysImeProp_.extName,
        EnabledStatus::DISABLED);
    EXPECT_EQ(ret, ErrorCode::ERROR_OPERATE_SYSTEM_IME);
}

/**
 * @tc.name: testBundleEnabledStatusUpdate_003
 * @tc.desc: test:new status same with old status. no need deal
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImeEnabledInfoManagerTest, testBundleEnabledStatusUpdate_003, TestSize.Level0)
{
    IMSA_HILOGI("ImeEnabledInfoManagerTest testBundleEnabledStatusUpdate_003 START");
    std::map<int32_t, std::vector<ImeEasyInfo>> easyEnabledInfos;
    easyEnabledInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_,
        { { ImeEnabledInfoManagerTest::IME_KEY3, EnabledStatus::BASIC_MODE } } });
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_ =
        ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos);

    auto ret = ImeEnabledInfoManager::GetInstance().Update(ImeEnabledInfoManagerTest::currentUserId_,
        BUNDLE_NAME3, EXT_NAME3, EnabledStatus::BASIC_MODE);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    if (ImeEnabledInfoManagerTest::IsAllSwitchOn()) {
        std::map<int32_t, std::vector<ImeEasyInfo>> easyEnabledInfos1;
        easyEnabledInfos1.insert({ ImeEnabledInfoManagerTest::currentUserId_,
            { { ImeEnabledInfoManagerTest::IME_KEY3, EnabledStatus::BASIC_MODE } } });
        EXPECT_FALSE(ImeEnabledInfoManagerTest::WaitDataShareCallback(
            ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos1)));
        EXPECT_TRUE(ImeEnabledInfoManagerTest::enabledCfg_.empty());
    }
}

/**
 * @tc.name: testBundleEnabledStatusUpdate_004
 * @tc.desc: test:extensionName not find
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImeEnabledInfoManagerTest, testBundleEnabledStatusUpdate_004, TestSize.Level0)
{
    IMSA_HILOGI("ImeEnabledInfoManagerTest testBundleEnabledStatusUpdate_003 START");
    std::map<int32_t, std::vector<ImeEasyInfo>> easyEnabledInfos;
    easyEnabledInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_,
        { { ImeEnabledInfoManagerTest::SYS_IME_KEY, EnabledStatus::BASIC_MODE } } });
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_ =
        ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos);

    auto ret = ImeEnabledInfoManager::GetInstance().Update(ImeEnabledInfoManagerTest::currentUserId_,
        ImeEnabledInfoManagerTest::sysImeProp_.bundleName, "error", EnabledStatus::BASIC_MODE);
    EXPECT_EQ(ret, ErrorCode::ERROR_IME_NOT_FOUND);
}

/**
 * @tc.name: testSetCurrentIme_001
 * @tc.desc: test:current ime changed
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImeEnabledInfoManagerTest, testSetCurrentIme_001, TestSize.Level0)
{
    IMSA_HILOGI("ImeEnabledInfoManagerTest testSetCurrentIme_001 START");
    std::map<int32_t, std::vector<ImeEasyInfo>> easyEnabledInfos;
    easyEnabledInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_,
        { { ImeEnabledInfoManagerTest::SYS_IME_KEY, EnabledStatus::BASIC_MODE, true },
            { ImeEnabledInfoManagerTest::IME_KEY2, EnabledStatus::BASIC_MODE } } });
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_ =
        ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos);

    auto ret = ImeEnabledInfoManager::GetInstance().SetCurrentIme(ImeEnabledInfoManagerTest::currentUserId_,
        std::string(ImeEnabledInfoManagerTest::BUNDLE_NAME2) + "/" + ImeEnabledInfoManagerTest::EXT_NAME2,
        ImeEnabledInfoManagerTest::CUR_SUBNAME2, true);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    std::map<int32_t, std::vector<ImeEasyInfo>> easyEnabledInfos1;
    easyEnabledInfos1.insert({ ImeEnabledInfoManagerTest::currentUserId_,
        { { ImeEnabledInfoManagerTest::SYS_IME_KEY, EnabledStatus::BASIC_MODE },
            { ImeEnabledInfoManagerTest::IME_KEY2, EnabledStatus::BASIC_MODE, true, true, false,
                ImeEnabledInfoManagerTest::CUR_SUBNAME2 } } });
    EXPECT_TRUE(ImeEnabledInfoManagerTest::WaitDataShareCallback(
        ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos1)));
}

/**
 * @tc.name: testSetCurrentIme_002
 * @tc.desc: test:current ime changed, but has tmp ime, no need to deal
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImeEnabledInfoManagerTest, testSetCurrentIme_002, TestSize.Level0)
{
    IMSA_HILOGI("ImeEnabledInfoManagerTest testSetCurrentIme_002 START");
    std::map<int32_t, std::vector<ImeEasyInfo>> easyEnabledInfos;
    easyEnabledInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_,
        { { ImeEnabledInfoManagerTest::SYS_IME_KEY, EnabledStatus::BASIC_MODE, false, false, true },
            { ImeEnabledInfoManagerTest::IME_KEY2, EnabledStatus::BASIC_MODE, true, true, false,
                ImeEnabledInfoManagerTest::CUR_SUBNAME2 } } });
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_ =
        ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos);

    auto ret = ImeEnabledInfoManager::GetInstance().SetCurrentIme(ImeEnabledInfoManagerTest::currentUserId_,
        std::string(ImeEnabledInfoManagerTest::BUNDLE_NAME2) + "/" + ImeEnabledInfoManagerTest::EXT_NAME2,
        ImeEnabledInfoManagerTest::CUR_SUBNAME1, false);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    std::map<int32_t, std::vector<ImeEasyInfo>> easyEnabledInfos1;
    easyEnabledInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_,
        { { ImeEnabledInfoManagerTest::SYS_IME_KEY, EnabledStatus::BASIC_MODE, false, false, true },
            { ImeEnabledInfoManagerTest::IME_KEY2, EnabledStatus::BASIC_MODE, true, false, false,
                ImeEnabledInfoManagerTest::CUR_SUBNAME1 } } });
    EXPECT_FALSE(ImeEnabledInfoManagerTest::WaitDataShareCallback(
        ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos1)));
    EXPECT_TRUE(ImeEnabledInfoManagerTest::enabledCfg_.empty());
}

/**
 * @tc.name: testSetTmpIme_001
 * @tc.desc: test:tmp ime changed, but same with original tmp ime, no need deal
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImeEnabledInfoManagerTest, testSetTmpIme_001, TestSize.Level0)
{
    IMSA_HILOGI("ImeEnabledInfoManagerTest testSetTmpIme_001 START");
    std::map<int32_t, std::vector<ImeEasyInfo>> easyEnabledInfos;
    easyEnabledInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_,
        { { ImeEnabledInfoManagerTest::SYS_IME_KEY, EnabledStatus::BASIC_MODE, false, false, true },
            { ImeEnabledInfoManagerTest::IME_KEY2, EnabledStatus::BASIC_MODE, true, true, false,
                ImeEnabledInfoManagerTest::CUR_SUBNAME2 } } });
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_ =
        ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos);

    auto ret = ImeEnabledInfoManager::GetInstance().SetTmpIme(
        ImeEnabledInfoManagerTest::currentUserId_, std::string(ImeEnabledInfoManagerTest::sysImeProp_.bundleName) + "/"
                                                       + ImeEnabledInfoManagerTest::sysImeProp_.extName);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_FALSE(ImeEnabledInfoManagerTest::WaitDataShareCallback(
        ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos)));
    EXPECT_TRUE(ImeEnabledInfoManagerTest::enabledCfg_.empty());
}

/**
 * @tc.name: testSetTmpIme_002
 * @tc.desc: test:tmp ime clear, but not has tmp ime, no need to deal
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImeEnabledInfoManagerTest, testSetTmpIme_002, TestSize.Level0)
{
    IMSA_HILOGI("ImeEnabledInfoManagerTest testSetTmpIme_002 START");
    std::map<int32_t, std::vector<ImeEasyInfo>> easyEnabledInfos;
    easyEnabledInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_,
        { { ImeEnabledInfoManagerTest::SYS_IME_KEY, EnabledStatus::BASIC_MODE },
            { ImeEnabledInfoManagerTest::IME_KEY2, EnabledStatus::BASIC_MODE, true, true, false,
                ImeEnabledInfoManagerTest::CUR_SUBNAME2 } } });
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_ =
        ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos);

    auto ret = ImeEnabledInfoManager::GetInstance().SetTmpIme(ImeEnabledInfoManagerTest::currentUserId_, "");
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_FALSE(ImeEnabledInfoManagerTest::WaitDataShareCallback(
        ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos)));
    EXPECT_TRUE(ImeEnabledInfoManagerTest::enabledCfg_.empty());
}

/**
 * @tc.name: testSetTmpIme_003
 * @tc.desc: test:tmp ime clear
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImeEnabledInfoManagerTest, testSetTmpIme_003, TestSize.Level0)
{
    IMSA_HILOGI("ImeEnabledInfoManagerTest testSetTmpIme_003 START");
    std::map<int32_t, std::vector<ImeEasyInfo>> easyEnabledInfos;
    easyEnabledInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_,
        { { ImeEnabledInfoManagerTest::SYS_IME_KEY, EnabledStatus::BASIC_MODE, false, false, true },
            { ImeEnabledInfoManagerTest::IME_KEY2, EnabledStatus::BASIC_MODE, true, true, false,
                ImeEnabledInfoManagerTest::CUR_SUBNAME2 } } });
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_ =
        ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos);

    auto ret = ImeEnabledInfoManager::GetInstance().SetTmpIme(ImeEnabledInfoManagerTest::currentUserId_, "");
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    std::map<int32_t, std::vector<ImeEasyInfo>> easyEnabledInfos1;
    easyEnabledInfos1.insert({ ImeEnabledInfoManagerTest::currentUserId_,
        { { ImeEnabledInfoManagerTest::SYS_IME_KEY, EnabledStatus::BASIC_MODE },
            { ImeEnabledInfoManagerTest::IME_KEY2, EnabledStatus::BASIC_MODE, true, true, false,
                ImeEnabledInfoManagerTest::CUR_SUBNAME2 } } });
    EXPECT_TRUE(ImeEnabledInfoManagerTest::WaitDataShareCallback(
        ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos1)));
}

/**
 * @tc.name: testSetTmpIme_004
 * @tc.desc: test:tmp ime set
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImeEnabledInfoManagerTest, testSetTmpIme_004, TestSize.Level0)
{
    IMSA_HILOGI("ImeEnabledInfoManagerTest testSetTmpIme_004 START");
    std::map<int32_t, std::vector<ImeEasyInfo>> easyEnabledInfos;
    easyEnabledInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_,
        { { ImeEnabledInfoManagerTest::SYS_IME_KEY, EnabledStatus::BASIC_MODE },
            { ImeEnabledInfoManagerTest::IME_KEY2, EnabledStatus::BASIC_MODE, true, true, false,
                ImeEnabledInfoManagerTest::CUR_SUBNAME2 } } });
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_ =
        ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos);

    auto ret = ImeEnabledInfoManager::GetInstance().SetTmpIme(
        ImeEnabledInfoManagerTest::currentUserId_, std::string(ImeEnabledInfoManagerTest::sysImeProp_.bundleName) + "/"
                                                       + ImeEnabledInfoManagerTest::sysImeProp_.extName);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    std::map<int32_t, std::vector<ImeEasyInfo>> easyEnabledInfos1;
    easyEnabledInfos1.insert({ ImeEnabledInfoManagerTest::currentUserId_,
        { { ImeEnabledInfoManagerTest::SYS_IME_KEY, EnabledStatus::BASIC_MODE, false, false, true },
            { ImeEnabledInfoManagerTest::IME_KEY2, EnabledStatus::BASIC_MODE, true, true, false,
                ImeEnabledInfoManagerTest::CUR_SUBNAME2 } } });
    EXPECT_TRUE(ImeEnabledInfoManagerTest::WaitDataShareCallback(
        ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos1)));
}

/**
 * @tc.name: testGetCurrentImeCfg_001
 * @tc.desc: test:has tmp ime, current ime is tmp ime
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImeEnabledInfoManagerTest, testGetCurrentImeCfg_001, TestSize.Level0)
{
    IMSA_HILOGI("ImeEnabledInfoManagerTest testGetCurrentImeCfg_001 START");
    std::map<int32_t, std::vector<ImeEasyInfo>> easyEnabledInfos;
    easyEnabledInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_,
        { { ImeEnabledInfoManagerTest::SYS_IME_KEY, EnabledStatus::BASIC_MODE, false, false, true },
            { ImeEnabledInfoManagerTest::IME_KEY2, EnabledStatus::BASIC_MODE, true, true, false,
                ImeEnabledInfoManagerTest::CUR_SUBNAME2 } } });
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_ =
        ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos);
    auto currentIme = ImeEnabledInfoManager::GetInstance().GetCurrentImeCfg(ImeEnabledInfoManagerTest::currentUserId_);
    ASSERT_NE(currentIme, nullptr);
    EXPECT_EQ(currentIme->bundleName, ImeEnabledInfoManagerTest::sysImeProp_.bundleName);
    EXPECT_EQ(currentIme->extName, ImeEnabledInfoManagerTest::sysImeProp_.extName);
    EXPECT_EQ(currentIme->imeId, ImeEnabledInfoManagerTest::sysImeProp_.imeId);
}

/**
 * @tc.name: testGetCurrentImeCfg_002
 * @tc.desc: test:has no tmp ime, current ime is default ime
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImeEnabledInfoManagerTest, testGetCurrentImeCfg_002, TestSize.Level0)
{
    IMSA_HILOGI("ImeEnabledInfoManagerTest testGetCurrentImeCfg_002 START");
    std::map<int32_t, std::vector<ImeEasyInfo>> easyEnabledInfos;
    easyEnabledInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_,
        { { ImeEnabledInfoManagerTest::SYS_IME_KEY, EnabledStatus::BASIC_MODE },
            { ImeEnabledInfoManagerTest::IME_KEY2, EnabledStatus::BASIC_MODE, true, true, false,
                ImeEnabledInfoManagerTest::CUR_SUBNAME2 } } });
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_ =
        ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos);
    auto currentIme = ImeEnabledInfoManager::GetInstance().GetCurrentImeCfg(ImeEnabledInfoManagerTest::currentUserId_);
    ASSERT_NE(currentIme, nullptr);
    EXPECT_EQ(currentIme->bundleName, ImeEnabledInfoManagerTest::BUNDLE_NAME2);
    EXPECT_EQ(currentIme->extName, ImeEnabledInfoManagerTest::EXT_NAME2);
    EXPECT_EQ(currentIme->subName, ImeEnabledInfoManagerTest::CUR_SUBNAME2);
    EXPECT_EQ(currentIme->imeId,
        std::string(ImeEnabledInfoManagerTest::BUNDLE_NAME2) + "/" + ImeEnabledInfoManagerTest::EXT_NAME2);
}
} // namespace MiscServices
} // namespace OHOS
