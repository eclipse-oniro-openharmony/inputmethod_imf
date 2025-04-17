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

#include "settings_data_utils.h"
#undef private

#include <gtest/gtest.h>

#include <algorithm>
#include <condition_variable>

#include "ime_info_inquirer.h"
#include "parameter.h"
#include "tdd_util.h"

using namespace testing::ext;
using namespace OHOS::DataShare;
namespace OHOS {
namespace MiscServices {
struct ImeEnabledNecessaryInfo {
    std::string bundleName;
    std::string extensionName;
    std::string installTime;
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
    static constexpr const char *DEFAULT_IME_KEY = "default_ime_key";
    static constexpr const char *DEFAULT_IME_INSTALL_TIME = "789456";
    static constexpr const char *IME_KEY1 = "key1";
    static constexpr const char *IME_KEY2 = "key2";
    static constexpr const char *BUNDLE_NAME1 = "bundleName1";
    static constexpr const char *BUNDLE_NAME2 = "bundleName2";
    static constexpr const char *EXT_NAME1 = "extName1";
    static constexpr const char *EXT_NAME2 = "extName2";
    static constexpr const char *INSTALL_TIME1 = "123456";
    static constexpr const char *INSTALL_TIME2 = "456789";
    static constexpr const char *RE_INSTALL_TIME = "33333";
    static constexpr int32_t USER_ID1 = 1;
    static constexpr int32_t WAIT_DATA_SHARE_CB_TIMEOUT = 300;
    static ImeNativeCfg defaultImeProp_;
    static int32_t currentUserId_;
    static void SetUpTestCase();
    static void TearDownTestCase();
    static void DataShareCallback();
    void SetUp();
    void TearDown();
    static bool WaitDataShareCallback(const std::map<int32_t, ImeEnabledCfg> &enabledCfg);
    static std::map<int32_t, std::vector<FullImeInfo>> GenerateFullImeInfos(
        std::map<int32_t, std::vector<std::string>> &easyInfos);
    static std::vector<FullImeInfo> GenerateFullImeInfos(std::vector<std::string> &imeKeys);
    static FullImeInfo GenerateFullImeInfo(const std::string &imeKey);
    static std::map<int32_t, ImeEnabledCfg> GenerateAllEnabledCfg(
        std::map<int32_t, std::vector<std::pair<std::string, EnabledStatus>>> &easyEnabledInfos);
    static ImeEnabledCfg GenerateAllEnabledCfg(std::vector<std::pair<std::string, EnabledStatus>> &easyEnabledInfos);

private:
    static std::string originalGlobalEnableImeStr_;
    static std::string originalGlobalFullExperienceStr_;
    static std::string currentUserEnableImeStr_;
    static std::map<int32_t, ImeEnabledCfg> enabledCfg_;
    static std::mutex dataShareCbCvMutex_;
    static std::condition_variable dataShareCbCv_;
    static std::unordered_map<std::string, ImeEnabledNecessaryInfo> imeEnabledNecessaryInfoMapping_;
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
    auto ret = SettingsDataUtils::GetInstance()->CreateAndRegisterObserver(
        uriProxy, ENABLE_IME, []() { DataShareCallback(); });
    IMSA_HILOGI("CreateAndRegisterObserver ret:%{public}d.", ret);

    ret = SettingsDataUtils::GetInstance()->GetStringValue(
        SETTING_URI_PROXY, SECURITY_MODE, originalGlobalFullExperienceStr_);
    IMSA_HILOGI("GetStringValue SECURITY_MODE ret:%{public}d.", ret);
    ret = SettingsDataUtils::GetInstance()->GetStringValue(SETTING_URI_PROXY, ENABLE_IME, originalGlobalEnableImeStr_);
    IMSA_HILOGI("GetStringValue ENABLE_IME ret:%{public}d.", ret);
    ret = SettingsDataUtils::GetInstance()->GetStringValue(
        SETTINGS_USER_DATA_URI + std::to_string(currentUserId_) + "?Proxy=true", ENABLE_IME, currentUserEnableImeStr_);
    IMSA_HILOGI("user GetStringValue ENABLE_IME ret:%{public}d.", ret);

    SettingsDataUtils::GetInstance()->isDataShareReady_ = true;
    defaultImeProp_ = ImeInfoInquirer::GetInstance().GetDefaultIme();
    imeEnabledNecessaryInfoMapping_ = {
        { IME_KEY1, { BUNDLE_NAME1, EXT_NAME1, INSTALL_TIME1 } },
        { IME_KEY2, { BUNDLE_NAME2, EXT_NAME2, INSTALL_TIME2 } },
        { DEFAULT_IME_KEY,
            {
                defaultImeProp_.bundleName,
                defaultImeProp_.extName,
                DEFAULT_IME_INSTALL_TIME,
            } },
    };
}

void ImeEnabledInfoManagerTest::TearDownTestCase()
{
    if (!originalGlobalFullExperienceStr_.empty()) {
        auto ret = SettingsDataUtils::GetInstance()->SetStringValue(
            SETTING_URI_PROXY, SECURITY_MODE, originalGlobalFullExperienceStr_);
        IMSA_HILOGI("SetStringValue SECURITY_MODE ret:%{public}d.", ret);
    }
    if (!originalGlobalEnableImeStr_.empty()) {
        auto ret = SettingsDataUtils::GetInstance()->SetStringValue(
            SETTING_URI_PROXY, ENABLE_IME, originalGlobalEnableImeStr_);
        IMSA_HILOGI("SetStringValue ENABLE_IME ret:%{public}d.", ret);
    }
    if (!currentUserEnableImeStr_.empty()) {
        SettingsDataUtils::GetInstance()->SetStringValue(
            SETTINGS_USER_DATA_URI + std::to_string(currentUserId_) + "?Proxy=true", ENABLE_IME,
            currentUserEnableImeStr_);
    }
}

void ImeEnabledInfoManagerTest::SetUp()
{
    TddUtil::DeleteGlobalTable(ENABLE_IME);
    TddUtil::DeleteGlobalTable(SECURITY_MODE);
    TddUtil::DeleteUserTable(currentUserId_, ENABLE_IME);
}

void ImeEnabledInfoManagerTest::TearDown()
{
    enabledCfg_.clear();
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_.clear();
}

void ImeEnabledInfoManagerTest::DataShareCallback()
{
    IMSA_HILOGI("start.");
    std::unique_lock<std::mutex> lock(dataShareCbCvMutex_);
    std::string content1;
    ImeEnabledCfg enabledCfg1;
    auto ret = SettingsDataUtils::GetInstance()->GetStringValue(
        SETTINGS_USER_DATA_URI + std::to_string(currentUserId_) + "?Proxy=true", ENABLE_IME, content1);
    if (!enabledCfg1.Unmarshall(content1)) {
        IMSA_HILOGE("%{public}d Unmarshall failed:%{public}d.", currentUserId_, ret);
    } else {
        enabledCfg_.insert_or_assign(currentUserId_, enabledCfg1);
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
            IMSA_HILOGI("enabledCfg info:[%{public}s,%{public}s,%{public}s,%{public}d].",
                info.bundleName.c_str(), info.extensionName.c_str(), info.installTime.c_str(), info.enabledStatus);
        }
    }
    for (const auto &cfg : enabledCfg_) {
        IMSA_HILOGI("enabledCfg_ base info:[%{public}d, %{public}s].", cfg.first, cfg.second.version.c_str());
        for (const auto &info : cfg.second.enabledInfos) {
            IMSA_HILOGI("enabledCfg_ info:[%{public}s,%{public}s,%{public}s,%{public}d].",
                info.bundleName.c_str(), info.extensionName.c_str(), info.installTime.c_str(), info.enabledStatus);
        }
    }
    for (const auto &cfg : ImeEnabledInfoManager::GetInstance().imeEnabledCfg_) {
        IMSA_HILOGI("cache base info:[%{public}d, %{public}s].", cfg.first, cfg.second.version.c_str());
        for (const auto &info : cfg.second.enabledInfos) {
            IMSA_HILOGI("cache info:[%{public}s,%{public}s,%{public}s,%{public}d].", info.bundleName.c_str(),
                info.extensionName.c_str(), info.installTime.c_str(), info.enabledStatus);
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
    std::map<int32_t, std::vector<std::string>> &easyInfos)
{
    std::map<int32_t, std::vector<FullImeInfo>> fullImeInfos;
    for (const auto &easyInfo : easyInfos) {
        std::vector<FullImeInfo> imeInfos;
        for (const auto &key : easyInfo.second) {
            FullImeInfo imeInfo;
            auto it = imeEnabledNecessaryInfoMapping_.find(key);
            if (it != imeEnabledNecessaryInfoMapping_.end()) {
                imeInfo.installTime = it->second.installTime;
                imeInfo.prop.name = it->second.bundleName;
                imeInfo.prop.id = it->second.extensionName;
            }
            imeInfos.push_back(imeInfo);
        }
        fullImeInfos.insert({ easyInfo.first, imeInfos });
    }
    return fullImeInfos;
}

std::vector<FullImeInfo> ImeEnabledInfoManagerTest::GenerateFullImeInfos(std::vector<std::string> &imeKeys)
{
    std::vector<FullImeInfo> imeInfos;
    for (const auto &key : imeKeys) {
        FullImeInfo imeInfo;
        auto it = imeEnabledNecessaryInfoMapping_.find(key);
        if (it != imeEnabledNecessaryInfoMapping_.end()) {
            imeInfo.installTime = it->second.installTime;
            imeInfo.prop.name = it->second.bundleName;
            imeInfo.prop.id = it->second.extensionName;
        }
        imeInfos.push_back(imeInfo);
    }
    return imeInfos;
}

FullImeInfo ImeEnabledInfoManagerTest::GenerateFullImeInfo(const std::string &imeKey)
{
    FullImeInfo imeInfo;
    auto it = imeEnabledNecessaryInfoMapping_.find(imeKey);
    if (it != imeEnabledNecessaryInfoMapping_.end()) {
        imeInfo.installTime = it->second.installTime;
        imeInfo.prop.name = it->second.bundleName;
        imeInfo.prop.id = it->second.extensionName;
    }
    return imeInfo;
}

std::map<int32_t, ImeEnabledCfg> ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(
    std::map<int32_t, std::vector<std::pair<std::string, EnabledStatus>>> &easyEnabledInfos)
{
    std::map<int32_t, ImeEnabledCfg> allEnabledCfg;
    for (const auto &easyInfo : easyEnabledInfos) {
        ImeEnabledCfg enabledCfg;
        enabledCfg.version = GetDisplayVersion();
        std::vector<ImeEnabledInfo> imeEnabledInfos;
        for (const auto &info : easyInfo.second) {
            ImeEnabledInfo imeEnabledInfo;
            auto it = imeEnabledNecessaryInfoMapping_.find(info.first);
            if (it != imeEnabledNecessaryInfoMapping_.end()) {
                imeEnabledInfo.installTime = it->second.installTime;
                imeEnabledInfo.bundleName = it->second.bundleName;
                imeEnabledInfo.extensionName = it->second.extensionName;
            }
            imeEnabledInfo.enabledStatus = info.second;
            imeEnabledInfos.push_back(imeEnabledInfo);
        }
        enabledCfg.enabledInfos = imeEnabledInfos;
        allEnabledCfg.insert({ easyInfo.first, enabledCfg });
    }
    return allEnabledCfg;
}

ImeEnabledCfg ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(
    std::vector<std::pair<std::string, EnabledStatus>> &easyEnabledInfos)
{
    ImeEnabledCfg enabledCfg;
    enabledCfg.version = GetDisplayVersion();
    std::vector<ImeEnabledInfo> imeEnabledInfos;
    for (const auto &info : easyEnabledInfos) {
        ImeEnabledInfo imeEnabledInfo;
        auto it = imeEnabledNecessaryInfoMapping_.find(info.first);
        if (it != imeEnabledNecessaryInfoMapping_.end()) {
            imeEnabledInfo.installTime = it->second.installTime;
            imeEnabledInfo.bundleName = it->second.bundleName;
            imeEnabledInfo.extensionName = it->second.extensionName;
        }
        imeEnabledInfo.enabledStatus = info.second;
        imeEnabledInfos.push_back(imeEnabledInfo);
    }
    enabledCfg.enabledInfos = imeEnabledInfos;
    return enabledCfg;
}

std::string ImeEnabledInfoManagerTest::originalGlobalEnableImeStr_;
std::string ImeEnabledInfoManagerTest::originalGlobalFullExperienceStr_;
std::string ImeEnabledInfoManagerTest::currentUserEnableImeStr_;
std::map<int32_t, ImeEnabledCfg> ImeEnabledInfoManagerTest::enabledCfg_;
std::mutex ImeEnabledInfoManagerTest::dataShareCbCvMutex_;
std::condition_variable ImeEnabledInfoManagerTest::dataShareCbCv_;
ImeNativeCfg ImeEnabledInfoManagerTest::defaultImeProp_;
int32_t ImeEnabledInfoManagerTest::currentUserId_;
std::unordered_map<std::string, ImeEnabledNecessaryInfo> ImeEnabledInfoManagerTest::imeEnabledNecessaryInfoMapping_;

/**
 * @tc.name: testInit_001
 * @tc.desc: Init:first startup after version burning,no table
 *                has sys ime
 *                test:sys ime default BASIC_MODE
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImeEnabledInfoManagerTest, testInit_001, TestSize.Level0)
{
    IMSA_HILOGI("ImeEnabledInfoManagerTest testInit_001 START");
    std::map<int32_t, std::vector<std::string>> easyInfos;
    easyInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_, { ImeEnabledInfoManagerTest::DEFAULT_IME_KEY } });
    auto ret = ImeEnabledInfoManager::GetInstance().Init(ImeEnabledInfoManagerTest::GenerateFullImeInfos(easyInfos));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    if (ImeInfoInquirer::GetInstance().GetSystemConfig().enableInputMethodFeature
        && ImeInfoInquirer::GetInstance().GetSystemConfig().enableFullExperienceFeature) {
        std::map<int32_t, std::vector<std::pair<std::string, EnabledStatus>>> easyEnabledInfos;
        easyEnabledInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_,
            { std::make_pair(ImeEnabledInfoManagerTest::DEFAULT_IME_KEY, EnabledStatus::BASIC_MODE) } });
        EXPECT_TRUE(ImeEnabledInfoManagerTest::WaitDataShareCallback(
            ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos)));
        EnabledStatus status = EnabledStatus::DISABLED;
        ret = ImeEnabledInfoManager::GetInstance().GetEnabledState(
            ImeEnabledInfoManagerTest::currentUserId_, ImeEnabledInfoManagerTest::defaultImeProp_.bundleName, status);
        EXPECT_EQ(ret, ErrorCode::NO_ERROR);
        EXPECT_EQ(status, EnabledStatus::BASIC_MODE);
    }
}

/**
 * @tc.name: testInit_002
 * @tc.desc: Init:upgrade only with global table(enable and full experience),
 *                has sys ime and BUNDLE_NAME1
 *                BUNDLE_NAME1 in full experience table
 *                BUNDLE_NAME2 in enable table
 *                test1:BUNDLE_NAME1 in full experience table， not in enable table, status is FULL_EXPERIENCE_MODE
 *                test2:BUNDLE_NAME2 in enable table, but not installed, will not add to new enable table
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImeEnabledInfoManagerTest, testInit_002, TestSize.Level0)
{
    IMSA_HILOGI("ImeEnabledInfoManagerTest testInit_002 START");
    EnabledImeCfgTest enableCfg;
    enableCfg.userImeCfg.cfgs.insert(
        { std::to_string(ImeEnabledInfoManagerTest::currentUserId_), { ImeEnabledInfoManagerTest::BUNDLE_NAME2 } });
    std::string enabledStr;
    enableCfg.Marshall(enabledStr);
    TddUtil::GenerateGlobalTable(ImeEnabledInfoManagerTest::ENABLE_IME, enabledStr);

    SecurityModeCfgTest SecurityModeCfg;
    SecurityModeCfg.userImeCfg.cfgs.insert(
        { std::to_string(ImeEnabledInfoManagerTest::currentUserId_), { ImeEnabledInfoManagerTest::BUNDLE_NAME1 } });
    std::string fullExperienceStr;
    SecurityModeCfg.Marshall(fullExperienceStr);
    TddUtil::GenerateGlobalTable(ImeEnabledInfoManagerTest::SECURITY_MODE, fullExperienceStr);

    std::map<int32_t, std::vector<std::string>> easyInfos;
    easyInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_, {
                                                                      ImeEnabledInfoManagerTest::DEFAULT_IME_KEY,
                                                                      ImeEnabledInfoManagerTest::IME_KEY1,
                                                                  } });
    auto ret = ImeEnabledInfoManager::GetInstance().Init(ImeEnabledInfoManagerTest::GenerateFullImeInfos(easyInfos));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    if (ImeInfoInquirer::GetInstance().GetSystemConfig().enableInputMethodFeature
        && ImeInfoInquirer::GetInstance().GetSystemConfig().enableFullExperienceFeature) {
        std::map<int32_t, std::vector<std::pair<std::string, EnabledStatus>>> easyEnabledInfos;
        easyEnabledInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_,
            {
                std::make_pair(ImeEnabledInfoManagerTest::IME_KEY1, EnabledStatus::FULL_EXPERIENCE_MODE),
                std::make_pair(ImeEnabledInfoManagerTest::DEFAULT_IME_KEY, EnabledStatus::BASIC_MODE),
            } });
        EXPECT_TRUE(ImeEnabledInfoManagerTest::WaitDataShareCallback(
            ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos)));
        EnabledStatus status = EnabledStatus::DISABLED;
        ret = ImeEnabledInfoManager::GetInstance().GetEnabledState(
            ImeEnabledInfoManagerTest::currentUserId_, ImeEnabledInfoManagerTest::BUNDLE_NAME1, status);
        EXPECT_EQ(ret, ErrorCode::NO_ERROR);
        EXPECT_EQ(status, EnabledStatus::FULL_EXPERIENCE_MODE);
    }
}

/**
 * @tc.name: testInit_003
 * @tc.desc: Init:upgrade only with global enable table
 *                has sys ime、BUNDLE_NAME1 and BUNDLE_NAME2
 *                BUNDLE_NAME1 in enable table
 *                test1:BUNDLE_NAME1 in enable table, status is BASIC_MODE
 *                test2:BUNDLE_NAME2 not in enable table and full experience table,
 *                      status is decided by initEnabledState
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImeEnabledInfoManagerTest, testInit_003, TestSize.Level0)
{
    IMSA_HILOGI("ImeEnabledInfoManagerTest testInit_003 START");
    EnabledImeCfgTest enableCfg;
    enableCfg.userImeCfg.cfgs.insert(
        { std::to_string(ImeEnabledInfoManagerTest::currentUserId_), { ImeEnabledInfoManagerTest::BUNDLE_NAME1 } });
    std::string enabledStr;
    enableCfg.Marshall(enabledStr);
    TddUtil::GenerateGlobalTable(ImeEnabledInfoManagerTest::ENABLE_IME, enabledStr);

    std::map<int32_t, std::vector<std::string>> easyInfos;
    easyInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_, {
                                                                      ImeEnabledInfoManagerTest::DEFAULT_IME_KEY,
                                                                      ImeEnabledInfoManagerTest::IME_KEY1,
                                                                      ImeEnabledInfoManagerTest::IME_KEY2,
                                                                  } });
    auto ret = ImeEnabledInfoManager::GetInstance().Init(ImeEnabledInfoManagerTest::GenerateFullImeInfos(easyInfos));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    if (ImeInfoInquirer::GetInstance().GetSystemConfig().enableInputMethodFeature
        && ImeInfoInquirer::GetInstance().GetSystemConfig().enableFullExperienceFeature) {
        EnabledStatus imeEnabledStatus2 = ImeInfoInquirer::GetInstance().GetSystemConfig().initEnabledState;
        std::map<int32_t, std::vector<std::pair<std::string, EnabledStatus>>> easyEnabledInfos;
        easyEnabledInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_,
            { std::make_pair(ImeEnabledInfoManagerTest::IME_KEY1, EnabledStatus::BASIC_MODE),
                std::make_pair(ImeEnabledInfoManagerTest::DEFAULT_IME_KEY, EnabledStatus::BASIC_MODE),
                std::make_pair(ImeEnabledInfoManagerTest::IME_KEY2, imeEnabledStatus2) } });
        EXPECT_TRUE(ImeEnabledInfoManagerTest::WaitDataShareCallback(
            ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos)));
        EnabledStatus status = EnabledStatus::DISABLED;
        ret = ImeEnabledInfoManager::GetInstance().GetEnabledState(
            ImeEnabledInfoManagerTest::currentUserId_, ImeEnabledInfoManagerTest::BUNDLE_NAME2, status);
        EXPECT_EQ(ret, ErrorCode::NO_ERROR);
        EXPECT_EQ(status, imeEnabledStatus2);
    }
}

/**
 * @tc.name: testInit_004
 * @tc.desc: Init:upgrade only with global full experience table
 *                has sys ime、 BUNDLE_NAME2
 *                BUNDLE_NAME2 in full experience table
 *                full experience table parse abnormal
 *                test:BUNDLE_NAME2 in full experience table， not in enable table,
 *                     full experience table parse abnormal, status is decided by initEnabledState
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImeEnabledInfoManagerTest, testInit_004, TestSize.Level0)
{
    IMSA_HILOGI("ImeEnabledInfoManagerTest testInit_004 START");
    SecurityModeCfgTest SecurityModeCfg;
    SecurityModeCfg.userImeCfg.cfgs.insert({ "", { ImeEnabledInfoManagerTest::BUNDLE_NAME2 } });
    std::string fullExperienceStr;
    SecurityModeCfg.Marshall(fullExperienceStr);
    TddUtil::GenerateGlobalTable(ImeEnabledInfoManagerTest::SECURITY_MODE, fullExperienceStr);

    std::map<int32_t, std::vector<std::string>> easyInfos;
    easyInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_, {
                                                                      ImeEnabledInfoManagerTest::DEFAULT_IME_KEY,
                                                                      ImeEnabledInfoManagerTest::IME_KEY2,
                                                                  } });
    auto ret = ImeEnabledInfoManager::GetInstance().Init(ImeEnabledInfoManagerTest::GenerateFullImeInfos(easyInfos));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    if (ImeInfoInquirer::GetInstance().GetSystemConfig().enableInputMethodFeature
        && ImeInfoInquirer::GetInstance().GetSystemConfig().enableFullExperienceFeature) {
        EnabledStatus imeEnabledStatus2 = ImeInfoInquirer::GetInstance().GetSystemConfig().initEnabledState;
        std::map<int32_t, std::vector<std::pair<std::string, EnabledStatus>>> easyEnabledInfos;
        easyEnabledInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_,
            { std::make_pair(ImeEnabledInfoManagerTest::DEFAULT_IME_KEY, EnabledStatus::BASIC_MODE),
                std::make_pair(ImeEnabledInfoManagerTest::IME_KEY2, imeEnabledStatus2) } });
        EXPECT_TRUE(ImeEnabledInfoManagerTest::WaitDataShareCallback(
            ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos)));
        EnabledStatus status = EnabledStatus::DISABLED;
        ret = ImeEnabledInfoManager::GetInstance().GetEnabledState(
            ImeEnabledInfoManagerTest::currentUserId_, ImeEnabledInfoManagerTest::BUNDLE_NAME2, status);
        EXPECT_EQ(ret, ErrorCode::NO_ERROR);
        EXPECT_EQ(status, imeEnabledStatus2);
    }
}

/**
 * @tc.name: testInit_005
 * @tc.desc: Init:upgrade only with global enable table
 *                has sys ime、 BUNDLE_NAME1
 *                BUNDLE_NAME1 in enable table
 *                enable table parse abnormal
 *                test:BUNDLE_NAME1 in in enable table, not in full experience table,
 *                     enable table parse abnormal, status is decided by initEnabledState
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImeEnabledInfoManagerTest, testInit_005, TestSize.Level0)
{
    IMSA_HILOGI("ImeEnabledInfoManagerTest testInit_004 START");
    EnabledImeCfgTest enableCfg;
    enableCfg.userImeCfg.cfgs.insert({ "", { ImeEnabledInfoManagerTest::BUNDLE_NAME1 } });
    std::string enabledStr;
    enableCfg.Marshall(enabledStr);
    TddUtil::GenerateGlobalTable(ImeEnabledInfoManagerTest::ENABLE_IME, enabledStr);

    std::map<int32_t, std::vector<std::string>> easyInfos;
    easyInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_, {
                                                                      ImeEnabledInfoManagerTest::DEFAULT_IME_KEY,
                                                                      ImeEnabledInfoManagerTest::IME_KEY1,
                                                                  } });
    auto ret = ImeEnabledInfoManager::GetInstance().Init(ImeEnabledInfoManagerTest::GenerateFullImeInfos(easyInfos));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    if (ImeInfoInquirer::GetInstance().GetSystemConfig().enableInputMethodFeature
        && ImeInfoInquirer::GetInstance().GetSystemConfig().enableFullExperienceFeature) {
        EnabledStatus imeEnabledStatus1 = ImeInfoInquirer::GetInstance().GetSystemConfig().initEnabledState;
        std::map<int32_t, std::vector<std::pair<std::string, EnabledStatus>>> easyEnabledInfos;
        easyEnabledInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_,
            {
                std::make_pair(ImeEnabledInfoManagerTest::DEFAULT_IME_KEY, EnabledStatus::BASIC_MODE),
                std::make_pair(ImeEnabledInfoManagerTest::IME_KEY1, imeEnabledStatus1),
            } });
        EXPECT_TRUE(ImeEnabledInfoManagerTest::WaitDataShareCallback(
            ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos)));
        EnabledStatus status = EnabledStatus::DISABLED;
        ret = ImeEnabledInfoManager::GetInstance().GetEnabledState(
            ImeEnabledInfoManagerTest::currentUserId_, ImeEnabledInfoManagerTest::BUNDLE_NAME1, status);
        EXPECT_EQ(ret, ErrorCode::NO_ERROR);
        EXPECT_EQ(status, imeEnabledStatus1);
    }
}

/**
 * @tc.name: testInit_006
 * @tc.desc: Init:upgrade with global table(enable and full experience) and user enable table
 *                has sys ime、BUNDLE_NAME1 and BUNDLE_NAME2
 *                sys ime in global enable table and in user enable table
 *                BUNDLE_NAME1 in global enable table、
 *                BUNDLE_NAME2 in global enable table and in user enable table
 *                test:has user enable table, but global enable table has normal enable info of this user,
 *                      It's up to global enable table
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImeEnabledInfoManagerTest, testInit_006, TestSize.Level0)
{
    IMSA_HILOGI("ImeEnabledInfoManagerTest testInit_006 START");
    EnabledImeCfgTest enableCfg;
    enableCfg.userImeCfg.cfgs.insert({ std::to_string(ImeEnabledInfoManagerTest::currentUserId_),
        { ImeEnabledInfoManagerTest::BUNDLE_NAME1, ImeEnabledInfoManagerTest::BUNDLE_NAME2,
            ImeEnabledInfoManagerTest::defaultImeProp_.bundleName } });
    std::string enabledStr;
    enableCfg.Marshall(enabledStr);
    TddUtil::GenerateGlobalTable(ImeEnabledInfoManagerTest::ENABLE_IME, enabledStr);

    SecurityModeCfgTest SecurityModeCfg;
    SecurityModeCfg.userImeCfg.cfgs.insert({ std::to_string(ImeEnabledInfoManagerTest::currentUserId_), {} });
    std::string fullExperienceStr;
    SecurityModeCfg.Marshall(fullExperienceStr);
    TddUtil::GenerateGlobalTable(ImeEnabledInfoManagerTest::SECURITY_MODE, fullExperienceStr);

    EnabledImeCfgTest enableCfg1;
    enableCfg1.userImeCfg.cfgs.insert({ std::to_string(ImeEnabledInfoManagerTest::currentUserId_),
        { ImeEnabledInfoManagerTest::BUNDLE_NAME2, ImeEnabledInfoManagerTest::defaultImeProp_.bundleName } });
    std::string userEnabledStr;
    enableCfg1.Marshall(userEnabledStr);
    TddUtil::GenerateUserTable(currentUserId_, ImeEnabledInfoManagerTest::ENABLE_IME, userEnabledStr);

    std::map<int32_t, std::vector<std::string>> easyInfos;
    easyInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_, {
                                                                      ImeEnabledInfoManagerTest::DEFAULT_IME_KEY,
                                                                      ImeEnabledInfoManagerTest::IME_KEY1,
                                                                      ImeEnabledInfoManagerTest::IME_KEY2,
                                                                  } });

    auto ret = ImeEnabledInfoManager::GetInstance().Init(ImeEnabledInfoManagerTest::GenerateFullImeInfos(easyInfos));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    if (ImeInfoInquirer::GetInstance().GetSystemConfig().enableInputMethodFeature
        && ImeInfoInquirer::GetInstance().GetSystemConfig().enableFullExperienceFeature) {
        std::map<int32_t, std::vector<std::pair<std::string, EnabledStatus>>> easyEnabledInfos;
        easyEnabledInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_,
            { std::make_pair(ImeEnabledInfoManagerTest::IME_KEY1, EnabledStatus::BASIC_MODE),
                std::make_pair(ImeEnabledInfoManagerTest::IME_KEY2, EnabledStatus::BASIC_MODE),
                std::make_pair(ImeEnabledInfoManagerTest::DEFAULT_IME_KEY, EnabledStatus::BASIC_MODE) } });
        EXPECT_TRUE(ImeEnabledInfoManagerTest::WaitDataShareCallback(
            ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos)));
    }
}

/**
 * @tc.name: testInit_007
 * @tc.desc: Init:upgrade with global table(enable and full experience) and user enable table
 *                has sys ime and BUNDLE_NAME1
 *                sys ime in user enable table
 *                BUNDLE_NAME1 in user enable table
 *                test:has user enable table, but global enable table not has enable info of this user,
 *                      It's up to user enable table
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImeEnabledInfoManagerTest, testInit_007, TestSize.Level0)
{
    IMSA_HILOGI("ImeEnabledInfoManagerTest testInit_007 START");
    EnabledImeCfgTest enableCfg;
    enableCfg.userImeCfg.cfgs.insert({ std::to_string(ImeEnabledInfoManagerTest::USER_ID1),
        { ImeEnabledInfoManagerTest::BUNDLE_NAME1, ImeEnabledInfoManagerTest::BUNDLE_NAME2,
            ImeEnabledInfoManagerTest::defaultImeProp_.bundleName } });
    std::string enabledStr;
    enableCfg.Marshall(enabledStr);
    TddUtil::GenerateGlobalTable(ImeEnabledInfoManagerTest::ENABLE_IME, enabledStr);

    SecurityModeCfgTest SecurityModeCfg;
    SecurityModeCfg.userImeCfg.cfgs.insert({ std::to_string(ImeEnabledInfoManagerTest::currentUserId_), {} });
    std::string fullExperienceStr;
    SecurityModeCfg.Marshall(fullExperienceStr);
    TddUtil::GenerateGlobalTable(ImeEnabledInfoManagerTest::SECURITY_MODE, fullExperienceStr);

    EnabledImeCfgTest enableCfg1;
    enableCfg1.userImeCfg.cfgs.insert({ std::to_string(ImeEnabledInfoManagerTest::currentUserId_),
        { ImeEnabledInfoManagerTest::BUNDLE_NAME1, ImeEnabledInfoManagerTest::defaultImeProp_.bundleName } });
    std::string userEnabledStr;
    enableCfg1.Marshall(userEnabledStr);
    TddUtil::GenerateUserTable(currentUserId_, ImeEnabledInfoManagerTest::ENABLE_IME, userEnabledStr);

    std::map<int32_t, std::vector<std::string>> easyInfos;
    easyInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_, {
                                                                      ImeEnabledInfoManagerTest::DEFAULT_IME_KEY,
                                                                      ImeEnabledInfoManagerTest::IME_KEY1,
                                                                  } });
    auto ret = ImeEnabledInfoManager::GetInstance().Init(ImeEnabledInfoManagerTest::GenerateFullImeInfos(easyInfos));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    if (ImeInfoInquirer::GetInstance().GetSystemConfig().enableInputMethodFeature
        && ImeInfoInquirer::GetInstance().GetSystemConfig().enableFullExperienceFeature) {
        std::map<int32_t, std::vector<std::pair<std::string, EnabledStatus>>> easyEnabledInfos;
        easyEnabledInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_,
            {
                std::make_pair(ImeEnabledInfoManagerTest::IME_KEY1, EnabledStatus::BASIC_MODE),
                std::make_pair(ImeEnabledInfoManagerTest::DEFAULT_IME_KEY, EnabledStatus::BASIC_MODE),
            } });
        EXPECT_TRUE(ImeEnabledInfoManagerTest::WaitDataShareCallback(
            ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos)));
    }
}

/**
 * @tc.name: testInit_008
 * @tc.desc: Init:upgrade with only with user enable table
 *                has sys ime、BUNDLE_NAME2
 *                sys ime、BUNDLE_NAME2 in user enable table
 *                test:has user enable table, not has global enable table, It's up to user enable table
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImeEnabledInfoManagerTest, testInit_008, TestSize.Level0)
{
    IMSA_HILOGI("ImeEnabledInfoManagerTest testInit_008 START");
    EnabledImeCfgTest enableCfg;
    enableCfg.userImeCfg.cfgs.insert({ std::to_string(ImeEnabledInfoManagerTest::currentUserId_),
        { ImeEnabledInfoManagerTest::BUNDLE_NAME2, ImeEnabledInfoManagerTest::defaultImeProp_.bundleName } });
    std::string userEnabledStr;
    enableCfg.Marshall(userEnabledStr);
    TddUtil::GenerateUserTable(currentUserId_, ImeEnabledInfoManagerTest::ENABLE_IME, userEnabledStr);

    std::map<int32_t, std::vector<std::string>> easyInfos;
    easyInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_, {
                                                                      ImeEnabledInfoManagerTest::DEFAULT_IME_KEY,
                                                                      ImeEnabledInfoManagerTest::IME_KEY2,
                                                                  } });
    auto ret = ImeEnabledInfoManager::GetInstance().Init(ImeEnabledInfoManagerTest::GenerateFullImeInfos(easyInfos));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    if (ImeInfoInquirer::GetInstance().GetSystemConfig().enableInputMethodFeature
        && ImeInfoInquirer::GetInstance().GetSystemConfig().enableFullExperienceFeature) {
        std::map<int32_t, std::vector<std::pair<std::string, EnabledStatus>>> easyEnabledInfos;
        easyEnabledInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_,
            { std::make_pair(ImeEnabledInfoManagerTest::IME_KEY2, EnabledStatus::BASIC_MODE),
                std::make_pair(ImeEnabledInfoManagerTest::DEFAULT_IME_KEY, EnabledStatus::BASIC_MODE) } });
        EXPECT_TRUE(ImeEnabledInfoManagerTest::WaitDataShareCallback(
            ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos)));
    }
}

/**
 * @tc.name: testInit_009
 * @tc.desc: Init:upgrade with global table(enable and full experience) and user enable table
 *                has sys ime、BUNDLE_NAME2
 *                sys ime、BUNDLE_NAME2 in global enable table
 *                sys ime、BUNDLE_NAME2 in user enable table
 *                BUNDLE_NAME2 in full experience
 *                test:in enable table, in full experience, the status is full experience
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImeEnabledInfoManagerTest, testInit_009, TestSize.Level0)
{
    IMSA_HILOGI("ImeEnabledInfoManagerTest testInit_009 START");
    EnabledImeCfgTest enableCfg;
    enableCfg.userImeCfg.cfgs.insert({ std::to_string(ImeEnabledInfoManagerTest::currentUserId_),
        { ImeEnabledInfoManagerTest::BUNDLE_NAME2, ImeEnabledInfoManagerTest::defaultImeProp_.bundleName } });
    std::string enabledStr;
    enableCfg.Marshall(enabledStr);
    TddUtil::GenerateGlobalTable(ImeEnabledInfoManagerTest::ENABLE_IME, enabledStr);

    SecurityModeCfgTest SecurityModeCfg;
    SecurityModeCfg.userImeCfg.cfgs.insert(
        { std::to_string(ImeEnabledInfoManagerTest::currentUserId_), { ImeEnabledInfoManagerTest::BUNDLE_NAME2 } });
    std::string fullExperienceStr;
    SecurityModeCfg.Marshall(fullExperienceStr);
    TddUtil::GenerateGlobalTable(ImeEnabledInfoManagerTest::SECURITY_MODE, fullExperienceStr);

    EnabledImeCfgTest enableCfg1;
    enableCfg1.userImeCfg.cfgs.insert({ std::to_string(ImeEnabledInfoManagerTest::currentUserId_),
        { ImeEnabledInfoManagerTest::BUNDLE_NAME2, ImeEnabledInfoManagerTest::defaultImeProp_.bundleName } });
    std::string userEnabledStr;
    enableCfg1.Marshall(userEnabledStr);
    TddUtil::GenerateUserTable(currentUserId_, ImeEnabledInfoManagerTest::ENABLE_IME, userEnabledStr);

    std::map<int32_t, std::vector<std::string>> easyInfos;
    easyInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_, {
                                                                      ImeEnabledInfoManagerTest::DEFAULT_IME_KEY,
                                                                      ImeEnabledInfoManagerTest::IME_KEY2,
                                                                  } });
    auto ret = ImeEnabledInfoManager::GetInstance().Init(ImeEnabledInfoManagerTest::GenerateFullImeInfos(easyInfos));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    if (ImeInfoInquirer::GetInstance().GetSystemConfig().enableInputMethodFeature
        && ImeInfoInquirer::GetInstance().GetSystemConfig().enableFullExperienceFeature) {
        std::map<int32_t, std::vector<std::pair<std::string, EnabledStatus>>> easyEnabledInfos;
        easyEnabledInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_,
            { std::make_pair(ImeEnabledInfoManagerTest::IME_KEY2, EnabledStatus::FULL_EXPERIENCE_MODE),
                std::make_pair(ImeEnabledInfoManagerTest::DEFAULT_IME_KEY, EnabledStatus::BASIC_MODE) } });
        EXPECT_TRUE(ImeEnabledInfoManagerTest::WaitDataShareCallback(
            ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos)));
    }
}

/**
 * @tc.name: testInit_010
 * @tc.desc: Init:device reboot with new user enable table
 *                has sys ime、BUNDLE_NAME2
 *                sys ime、BUNDLE_NAME2 in global enable table
 *                sys ime、BUNDLE_NAME2 in new user enable table, all BASIC_MODE
 *                BUNDLE_NAME2 in full experience table
 *                test:has new user table, It's up to new user enable table
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImeEnabledInfoManagerTest, testInit_010, TestSize.Level0)
{
    IMSA_HILOGI("ImeEnabledInfoManagerTest testInit_010 START");
    EnabledImeCfgTest enableCfg;
    enableCfg.userImeCfg.cfgs.insert({ std::to_string(ImeEnabledInfoManagerTest::currentUserId_),
        { ImeEnabledInfoManagerTest::BUNDLE_NAME2, ImeEnabledInfoManagerTest::defaultImeProp_.bundleName } });
    std::string enabledStr;
    enableCfg.Marshall(enabledStr);
    TddUtil::GenerateGlobalTable(ImeEnabledInfoManagerTest::ENABLE_IME, enabledStr);

    SecurityModeCfgTest securityModeCfg;
    securityModeCfg.userImeCfg.cfgs.insert(
        { std::to_string(ImeEnabledInfoManagerTest::currentUserId_), { ImeEnabledInfoManagerTest::BUNDLE_NAME2 } });
    std::string fullExperienceStr;
    securityModeCfg.Marshall(fullExperienceStr);
    TddUtil::GenerateGlobalTable(ImeEnabledInfoManagerTest::SECURITY_MODE, fullExperienceStr);

    std::map<int32_t, std::vector<std::pair<std::string, EnabledStatus>>> easyEnabledInfos;
    easyEnabledInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_,
        { std::make_pair(ImeEnabledInfoManagerTest::DEFAULT_IME_KEY, EnabledStatus::BASIC_MODE),
            std::make_pair(ImeEnabledInfoManagerTest::IME_KEY2, EnabledStatus::DISABLED) } });
    auto enabledCfg = ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos);
    std::string userEnabledStr;
    enabledCfg[ImeEnabledInfoManagerTest::currentUserId_].Marshall(userEnabledStr);
    TddUtil::GenerateUserTable(currentUserId_, ImeEnabledInfoManagerTest::ENABLE_IME, userEnabledStr);

    std::map<int32_t, std::vector<std::string>> easyInfos;
    easyInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_, {
                                                                      ImeEnabledInfoManagerTest::DEFAULT_IME_KEY,
                                                                      ImeEnabledInfoManagerTest::IME_KEY2,
                                                                  } });
    auto ret = ImeEnabledInfoManager::GetInstance().Init(ImeEnabledInfoManagerTest::GenerateFullImeInfos(easyInfos));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    if (ImeInfoInquirer::GetInstance().GetSystemConfig().enableInputMethodFeature
        && ImeInfoInquirer::GetInstance().GetSystemConfig().enableFullExperienceFeature) {
        EXPECT_TRUE(ImeEnabledInfoManagerTest::WaitDataShareCallback(enabledCfg));
    }
}

/**
 * @tc.name: testInit_011
 * @tc.desc: Init:imsa reboot with new user enable table
 *                has sys ime、BUNDLE_NAME2, BUNDLE_NAME2 install after uninstall when imsa died
 *                sys ime、BUNDLE_NAME2 in new user enable table, sys ime is BASIC_MODE,
 *                    BUNDLE_NAME2 is FULL_EXPERIENCE_MODE before imsa died
 *                test:bundle install after uninstall when imsa died,
 *                     the status is DISABLED or BASIC_MODE decided by initEnabledState
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImeEnabledInfoManagerTest, testInit_011, TestSize.Level0)
{
    IMSA_HILOGI("ImeEnabledInfoManagerTest testInit_011 START");
    std::vector<std::pair<std::string, EnabledStatus>> easyEnabledInfos = {
        std::make_pair(ImeEnabledInfoManagerTest::DEFAULT_IME_KEY, EnabledStatus::BASIC_MODE),
        std::make_pair(ImeEnabledInfoManagerTest::IME_KEY2, EnabledStatus::FULL_EXPERIENCE_MODE)
    };
    auto enabledCfg = ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos);
    std::string userEnabledStr;
    enabledCfg.Marshall(userEnabledStr);
    TddUtil::GenerateUserTable(currentUserId_, ImeEnabledInfoManagerTest::ENABLE_IME, userEnabledStr);

    std::vector<std::string> imeKeys = { ImeEnabledInfoManagerTest::DEFAULT_IME_KEY,
        ImeEnabledInfoManagerTest::IME_KEY2 };
    auto imeInfos = ImeEnabledInfoManagerTest::GenerateFullImeInfos(imeKeys);
    auto it = std::find_if(imeInfos.begin(), imeInfos.end(),
        [](const FullImeInfo &info) { return info.prop.name == ImeEnabledInfoManagerTest::BUNDLE_NAME2; });
    if (it != imeInfos.end()) {
        it->installTime = ImeEnabledInfoManagerTest::RE_INSTALL_TIME;
    }
    auto ret = ImeEnabledInfoManager::GetInstance().Init({ { ImeEnabledInfoManagerTest::currentUserId_, imeInfos } });
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    if (ImeInfoInquirer::GetInstance().GetSystemConfig().enableInputMethodFeature
        && ImeInfoInquirer::GetInstance().GetSystemConfig().enableFullExperienceFeature) {
        auto iter = std::find_if(enabledCfg.enabledInfos.begin(), enabledCfg.enabledInfos.end(),
            [](const ImeEnabledInfo &info) { return info.bundleName == ImeEnabledInfoManagerTest::BUNDLE_NAME2; });
        if (iter != enabledCfg.enabledInfos.end()) {
            iter->enabledStatus = ImeInfoInquirer::GetInstance().GetSystemConfig().initEnabledState;
            iter->installTime = ImeEnabledInfoManagerTest::RE_INSTALL_TIME;
        }
        EXPECT_TRUE(ImeEnabledInfoManagerTest::WaitDataShareCallback(
            { { ImeEnabledInfoManagerTest::currentUserId_, enabledCfg } }));
    }
}

/**
 * @tc.name: testInit_012
 * @tc.desc: Init:upgrade with no table
 *                has BUNDLE_NAME2
 *                test:enabledStatus is decided by sys enabled cfg and sys initEnabledState
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImeEnabledInfoManagerTest, testInit_012, TestSize.Level0)
{
    IMSA_HILOGI("ImeEnabledInfoManagerTest testInit_012 START");
    std::map<int32_t, std::vector<std::string>> easyInfos;
    easyInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_, { ImeEnabledInfoManagerTest::IME_KEY2 } });
    auto ret = ImeEnabledInfoManager::GetInstance().Init(ImeEnabledInfoManagerTest::GenerateFullImeInfos(easyInfos));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    if (!ImeInfoInquirer::GetInstance().GetSystemConfig().enableInputMethodFeature
        && !ImeInfoInquirer::GetInstance().GetSystemConfig().enableFullExperienceFeature) {
        EnabledStatus status = EnabledStatus::DISABLED;
        ret = ImeEnabledInfoManager::GetInstance().GetEnabledState(
            ImeEnabledInfoManagerTest::currentUserId_, ImeEnabledInfoManagerTest::BUNDLE_NAME2, status);
        EXPECT_EQ(ret, ErrorCode::NO_ERROR);
        EXPECT_EQ(status, EnabledStatus::FULL_EXPERIENCE_MODE);
    } else if (ImeInfoInquirer::GetInstance().GetSystemConfig().enableInputMethodFeature
               && !ImeInfoInquirer::GetInstance().GetSystemConfig().enableFullExperienceFeature) {
        auto enabledStatus1 = EnabledStatus::DISABLED;
        if (ImeInfoInquirer::GetInstance().GetSystemConfig().initEnabledState != EnabledStatus::DISABLED) {
            enabledStatus1 = EnabledStatus::FULL_EXPERIENCE_MODE;
        }
        std::map<int32_t, std::vector<std::pair<std::string, EnabledStatus>>> easyEnabledInfos;
        easyEnabledInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_,
            { std::make_pair(ImeEnabledInfoManagerTest::IME_KEY2, enabledStatus1) } });
        EXPECT_TRUE(ImeEnabledInfoManagerTest::WaitDataShareCallback(
            ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos)));

    } else if (!ImeInfoInquirer::GetInstance().GetSystemConfig().enableInputMethodFeature
               && ImeInfoInquirer::GetInstance().GetSystemConfig().enableFullExperienceFeature) {
        std::map<int32_t, std::vector<std::pair<std::string, EnabledStatus>>> easyEnabledInfos;
        easyEnabledInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_,
            { std::make_pair(ImeEnabledInfoManagerTest::IME_KEY2, EnabledStatus::BASIC_MODE) } });
        EXPECT_TRUE(ImeEnabledInfoManagerTest::WaitDataShareCallback(
            ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos)));
    } else {
        auto enabledStatus1 = ImeInfoInquirer::GetInstance().GetSystemConfig().initEnabledState;
        std::map<int32_t, std::vector<std::pair<std::string, EnabledStatus>>> easyEnabledInfos;
        easyEnabledInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_,
            { std::make_pair(ImeEnabledInfoManagerTest::IME_KEY2, enabledStatus1) } });
        EXPECT_TRUE(ImeEnabledInfoManagerTest::WaitDataShareCallback(
            ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos)));
    }
}

/**
 * @tc.name: testUserAdd_001
 * @tc.desc: has cache
 *           test:user enable table will not be mod if has cache
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImeEnabledInfoManagerTest, testUserAdd_001, TestSize.Level0)
{
    IMSA_HILOGI("ImeEnabledInfoManagerTest testUserAdd_001 START");
    std::map<int32_t, std::vector<std::pair<std::string, EnabledStatus>>> easyEnabledInfos;
    easyEnabledInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_,
        { std::make_pair(ImeEnabledInfoManagerTest::DEFAULT_IME_KEY, EnabledStatus::BASIC_MODE),
            std::make_pair(ImeEnabledInfoManagerTest::IME_KEY2, EnabledStatus::FULL_EXPERIENCE_MODE) } });
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_ =
        ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos);

    std::vector<std::string> imeKeys = {
        ImeEnabledInfoManagerTest::DEFAULT_IME_KEY,
        ImeEnabledInfoManagerTest::IME_KEY2,
    };
    auto imeInfos = ImeEnabledInfoManagerTest::GenerateFullImeInfos(imeKeys);
    auto ret = ImeEnabledInfoManager::GetInstance().Add(ImeEnabledInfoManagerTest::currentUserId_, imeInfos);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    if (ImeEnabledInfoManager::GetInstance().HasEnabledSwitch()) {
        EXPECT_FALSE(
            ImeEnabledInfoManagerTest::WaitDataShareCallback(ImeEnabledInfoManager::GetInstance().imeEnabledCfg_));
    }
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
    std::map<int32_t, std::vector<std::pair<std::string, EnabledStatus>>> easyEnabledInfos;
    easyEnabledInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_,
        { std::make_pair(ImeEnabledInfoManagerTest::DEFAULT_IME_KEY, EnabledStatus::BASIC_MODE),
            std::make_pair(ImeEnabledInfoManagerTest::IME_KEY2, EnabledStatus::FULL_EXPERIENCE_MODE) } });
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_ =
        ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos);
    auto ret = ImeEnabledInfoManager::GetInstance().Delete(ImeEnabledInfoManagerTest::currentUserId_);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    if (ImeEnabledInfoManager::GetInstance().HasEnabledSwitch()) {
        EXPECT_TRUE(ImeEnabledInfoManager::GetInstance().imeEnabledCfg_.empty());
    }
}

/**
 * @tc.name: testBundleAdd_001
 * @tc.desc: BUNDLE_NAME1 is added
 *           has cache, BUNDLE_NAME1 not in cache
 *           test1: user enabled table will be mod when bundle add
 *           test2:enabledStatus of BUNDLE_NAME1 is decided by sys enabled cfg and sys initEnabledState
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImeEnabledInfoManagerTest, testBundleAdd_001, TestSize.Level0)
{
    IMSA_HILOGI("ImeEnabledInfoManagerTest testBundleAdd_001 START");
    std::map<int32_t, std::vector<std::pair<std::string, EnabledStatus>>> easyEnabledInfos;
    easyEnabledInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_,
        { std::make_pair(ImeEnabledInfoManagerTest::DEFAULT_IME_KEY, EnabledStatus::BASIC_MODE) } });
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_ =
        ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos);
    auto imeInfo = ImeEnabledInfoManagerTest::GenerateFullImeInfo(ImeEnabledInfoManagerTest::IME_KEY1);
    auto ret = ImeEnabledInfoManager::GetInstance().Add(ImeEnabledInfoManagerTest::currentUserId_, imeInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    if (ImeInfoInquirer::GetInstance().GetSystemConfig().enableInputMethodFeature
        && ImeInfoInquirer::GetInstance().GetSystemConfig().enableFullExperienceFeature) {
        auto enabledStatus1 = ImeInfoInquirer::GetInstance().GetSystemConfig().initEnabledState;
        std::map<int32_t, std::vector<std::pair<std::string, EnabledStatus>>> easyEnabledInfos1;
        easyEnabledInfos1.insert({ ImeEnabledInfoManagerTest::currentUserId_,
            { std::make_pair(ImeEnabledInfoManagerTest::DEFAULT_IME_KEY, EnabledStatus::BASIC_MODE),
                std::make_pair(ImeEnabledInfoManagerTest::IME_KEY1, enabledStatus1) } });
        EXPECT_TRUE(ImeEnabledInfoManagerTest::WaitDataShareCallback(
            ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos1)));
    } else if (!ImeInfoInquirer::GetInstance().GetSystemConfig().enableInputMethodFeature
               && ImeInfoInquirer::GetInstance().GetSystemConfig().enableFullExperienceFeature) {
        std::map<int32_t, std::vector<std::pair<std::string, EnabledStatus>>> easyEnabledInfos1;
        easyEnabledInfos1.insert({ ImeEnabledInfoManagerTest::currentUserId_,
            { std::make_pair(ImeEnabledInfoManagerTest::DEFAULT_IME_KEY, EnabledStatus::BASIC_MODE),
                std::make_pair(ImeEnabledInfoManagerTest::IME_KEY1, EnabledStatus::BASIC_MODE) } });
        EXPECT_TRUE(ImeEnabledInfoManagerTest::WaitDataShareCallback(
            ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos1)));
    } else if (ImeInfoInquirer::GetInstance().GetSystemConfig().enableInputMethodFeature
               && !ImeInfoInquirer::GetInstance().GetSystemConfig().enableFullExperienceFeature) {
        auto enabledStatus1 = EnabledStatus::DISABLED;
        if (ImeInfoInquirer::GetInstance().GetSystemConfig().initEnabledState != EnabledStatus::DISABLED) {
            enabledStatus1 = EnabledStatus::FULL_EXPERIENCE_MODE;
        }
        std::map<int32_t, std::vector<std::pair<std::string, EnabledStatus>>> easyEnabledInfos1;
        easyEnabledInfos1.insert({ ImeEnabledInfoManagerTest::currentUserId_,
            { std::make_pair(ImeEnabledInfoManagerTest::DEFAULT_IME_KEY, EnabledStatus::BASIC_MODE),
                std::make_pair(ImeEnabledInfoManagerTest::IME_KEY1, enabledStatus1) } });
        EXPECT_TRUE(ImeEnabledInfoManagerTest::WaitDataShareCallback(
            ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos1)));
    } else {
        EnabledStatus status = EnabledStatus::DISABLED;
        ret = ImeEnabledInfoManager::GetInstance().GetEnabledState(
            ImeEnabledInfoManagerTest::currentUserId_, ImeEnabledInfoManagerTest::BUNDLE_NAME1, status);
        EXPECT_EQ(ret, ErrorCode::NO_ERROR);
        EXPECT_EQ(status, EnabledStatus::FULL_EXPERIENCE_MODE);
    }
}

/**
 * @tc.name: testBundleAdd_002
 * @tc.desc: BUNDLE_NAME2 is added
 *           has cache, BUNDLE_NAME2 in cache
 *           test1: user enabled table will be mod when bundle add
 *           test2:enabledStatus of BUNDLE_NAME1 is decided by sys enabled cfg and sys initEnabledState
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImeEnabledInfoManagerTest, testBundleAdd_002, TestSize.Level0)
{
    IMSA_HILOGI("ImeEnabledInfoManagerTest testBundleAdd_002 START");
    std::map<int32_t, std::vector<std::pair<std::string, EnabledStatus>>> easyEnabledInfos;
    easyEnabledInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_,
        { std::make_pair(ImeEnabledInfoManagerTest::DEFAULT_IME_KEY, EnabledStatus::BASIC_MODE),
            std::make_pair(ImeEnabledInfoManagerTest::IME_KEY2, EnabledStatus::FULL_EXPERIENCE_MODE) } });
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_ =
        ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos);

    auto imeInfo = ImeEnabledInfoManagerTest::GenerateFullImeInfo(ImeEnabledInfoManagerTest::IME_KEY2);
    auto ret = ImeEnabledInfoManager::GetInstance().Add(ImeEnabledInfoManagerTest::currentUserId_, imeInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    if (ImeInfoInquirer::GetInstance().GetSystemConfig().enableInputMethodFeature
        && ImeInfoInquirer::GetInstance().GetSystemConfig().enableFullExperienceFeature) {
        auto enabledStatus1 = ImeInfoInquirer::GetInstance().GetSystemConfig().initEnabledState;
        std::map<int32_t, std::vector<std::pair<std::string, EnabledStatus>>> easyEnabledInfos1;
        easyEnabledInfos1.insert({ ImeEnabledInfoManagerTest::currentUserId_,
            {
                std::make_pair(ImeEnabledInfoManagerTest::DEFAULT_IME_KEY, EnabledStatus::BASIC_MODE),
                std::make_pair(ImeEnabledInfoManagerTest::IME_KEY2, enabledStatus1),
            } });
        EXPECT_TRUE(ImeEnabledInfoManagerTest::WaitDataShareCallback(
            ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos1)));
    }
}

/**
 * @tc.name: testBundleDelete_001
 * @tc.desc: has cache, bundle deleted in cache
 *           test:user enabled table will be mod when bundle delete
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImeEnabledInfoManagerTest, testBundleDelete_001, TestSize.Level0)
{
    IMSA_HILOGI("ImeEnabledInfoManagerTest testBundleDelete_001 START");
    std::map<int32_t, std::vector<std::pair<std::string, EnabledStatus>>> easyEnabledInfos;
    easyEnabledInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_,
        { std::make_pair(ImeEnabledInfoManagerTest::DEFAULT_IME_KEY, EnabledStatus::BASIC_MODE),
            std::make_pair(ImeEnabledInfoManagerTest::IME_KEY2, EnabledStatus::FULL_EXPERIENCE_MODE) } });
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_ =
        ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos);

    auto ret = ImeEnabledInfoManager::GetInstance().Delete(
        ImeEnabledInfoManagerTest::currentUserId_, ImeEnabledInfoManagerTest::BUNDLE_NAME2);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    if (ImeEnabledInfoManager::GetInstance().HasEnabledSwitch()) {
        std::map<int32_t, std::vector<std::pair<std::string, EnabledStatus>>> easyEnabledInfos1;
        easyEnabledInfos1.insert({ ImeEnabledInfoManagerTest::currentUserId_,
            { std::make_pair(ImeEnabledInfoManagerTest::DEFAULT_IME_KEY, EnabledStatus::BASIC_MODE) } });
        EXPECT_TRUE(ImeEnabledInfoManagerTest::WaitDataShareCallback(
            ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos1)));
    }
}

/**
 * @tc.name: testBundleDelete_002
 * @tc.desc: has cache, bundle deleted not in cache
 *           test:user enabled table will not be mod if bundle deleted not in cache
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImeEnabledInfoManagerTest, testBundleDelete_002, TestSize.Level0)
{
    IMSA_HILOGI("ImeEnabledInfoManagerTest testBundleDelete_002 START");
    std::map<int32_t, std::vector<std::pair<std::string, EnabledStatus>>> easyEnabledInfos;
    easyEnabledInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_,
        { std::make_pair(ImeEnabledInfoManagerTest::DEFAULT_IME_KEY, EnabledStatus::BASIC_MODE),
            std::make_pair(ImeEnabledInfoManagerTest::IME_KEY2, EnabledStatus::FULL_EXPERIENCE_MODE) } });
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_ =
        ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos);
    auto ret = ImeEnabledInfoManager::GetInstance().Delete(
        ImeEnabledInfoManagerTest::currentUserId_, ImeEnabledInfoManagerTest::BUNDLE_NAME1);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    if (ImeEnabledInfoManager::GetInstance().HasEnabledSwitch()) {
        EXPECT_FALSE(
            ImeEnabledInfoManagerTest::WaitDataShareCallback(ImeEnabledInfoManager::GetInstance().imeEnabledCfg_));
    }
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
    std::map<int32_t, std::vector<std::pair<std::string, EnabledStatus>>> easyEnabledInfos;
    easyEnabledInfos.insert({ ImeEnabledInfoManagerTest::currentUserId_,
        { std::make_pair(ImeEnabledInfoManagerTest::DEFAULT_IME_KEY, EnabledStatus::BASIC_MODE),
            std::make_pair(ImeEnabledInfoManagerTest::IME_KEY2, EnabledStatus::FULL_EXPERIENCE_MODE) } });
    ImeEnabledInfoManager::GetInstance().imeEnabledCfg_ =
        ImeEnabledInfoManagerTest::GenerateAllEnabledCfg(easyEnabledInfos);

    auto ret = ImeEnabledInfoManager::GetInstance().Update(ImeEnabledInfoManagerTest::currentUserId_,
        ImeEnabledInfoManagerTest::BUNDLE_NAME2, "", EnabledStatus::DISABLED);
    if (ImeEnabledInfoManager::GetInstance().HasEnabledSwitch()) {
        EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    } else {
        EXPECT_EQ(ret, ErrorCode::ERROR_ENABLE_IME);
    }
    if (ImeInfoInquirer::GetInstance().GetSystemConfig().enableInputMethodFeature
        && ImeInfoInquirer::GetInstance().GetSystemConfig().enableFullExperienceFeature) {
        std::map<int32_t, std::vector<std::pair<std::string, EnabledStatus>>> easyEnabledInfos1;
        easyEnabledInfos1.insert({ ImeEnabledInfoManagerTest::currentUserId_,
            { std::make_pair(ImeEnabledInfoManagerTest::DEFAULT_IME_KEY, EnabledStatus::BASIC_MODE),
                std::make_pair(ImeEnabledInfoManagerTest::IME_KEY2, EnabledStatus::DISABLED) } });
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
        ImeEnabledInfoManagerTest::defaultImeProp_.bundleName, "", EnabledStatus::DISABLED);
    EXPECT_EQ(ret, ErrorCode::ERROR_DISABLE_SYSTEM_IME);
}
} // namespace MiscServices
} // namespace OHOS
