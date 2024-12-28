/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
#include "ime_cfg_manager.h"

#include <gtest/gtest.h>

#include "file_operator_test.h"

using namespace OHOS::MiscServices;

class ImeCfgManagerTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        // Set up the test environment, e.g., clear configuration
        ImeCfgManager::GetInstance().DeleteImeCfg(1000); // Assume user ID is 1000
    }

    void TearDown() override
    {
        // Clean up the test environment
    }
};

/**
 * @tc.name: AddImeCfgTest
 * @tc.desc: Verify that the ImeCfgManager::AddImeCfg method works correctly when adding input method configuration
 * @tc.type: FUNC
 */
TEST_F(ImeCfgManagerTest, AddImeCfgTest)
{
    ImePersistInfo cfg;
    cfg.userId = 1000;
    cfg.currentIme = "com.example.ime/.ImeService";
    cfg.currentSubName = "Example IME";
    cfg.tempScreenLockIme = "";
    cfg.isDefaultImeSet = false;

    ImeCfgManager::GetInstance().AddImeCfg(cfg);

    ImePersistInfo addedCfg = ImeCfgManager::GetInstance().GetImeCfg(1000);
    EXPECT_EQ(addedCfg.userId, cfg.userId);
    EXPECT_EQ(addedCfg.currentIme, cfg.currentIme);
    EXPECT_EQ(addedCfg.currentSubName, cfg.currentSubName);
    EXPECT_EQ(addedCfg.tempScreenLockIme, cfg.tempScreenLockIme);
    EXPECT_EQ(addedCfg.isDefaultImeSet, cfg.isDefaultImeSet);
}

/**
 * @tc.name: ModifyImeCfgTest
 * @tc.desc: Verify that the ImeCfgManager::ModifyImeCfg method works correctly
 * when modifying input method configuration
 * @tc.type: FUNC
 */
TEST_F(ImeCfgManagerTest, ModifyImeCfgTest)
{
    ImePersistInfo cfg;
    cfg.userId = 1000;
    cfg.currentIme = "com.example.ime/.ImeService";
    cfg.currentSubName = "Example IME";
    cfg.tempScreenLockIme = "";
    cfg.isDefaultImeSet = false;

    ImeCfgManager::GetInstance().AddImeCfg(cfg);

    ImePersistInfo modifiedCfg;
    modifiedCfg.userId = 1000;
    modifiedCfg.currentIme = "com.example.ime/.ModifiedImeService";
    modifiedCfg.currentSubName = "Modified Example IME";
    modifiedCfg.tempScreenLockIme = "com.example.ime/.TempScreenLockIme";
    modifiedCfg.isDefaultImeSet = true;

    ImeCfgManager::GetInstance().ModifyImeCfg(modifiedCfg);

    ImePersistInfo updatedCfg = ImeCfgManager::GetInstance().GetImeCfg(1000);
    EXPECT_EQ(updatedCfg.userId, modifiedCfg.userId);
    EXPECT_EQ(updatedCfg.currentIme, modifiedCfg.currentIme);
    EXPECT_EQ(updatedCfg.currentSubName, modifiedCfg.currentSubName);
    EXPECT_EQ(updatedCfg.tempScreenLockIme, modifiedCfg.tempScreenLockIme);
    EXPECT_EQ(updatedCfg.isDefaultImeSet, modifiedCfg.isDefaultImeSet);
}

/**
 * @tc.name: GetImeCfgTest
 * @tc.desc: Verify that the ImeCfgManager::GetImeCfg method works correctly when retrieving input method configuration
 * @tc.type: FUNC
 */
TEST_F(ImeCfgManagerTest, GetImeCfgTest)
{
    ImePersistInfo cfg;
    cfg.userId = 1000;
    cfg.currentIme = "com.example.ime/.ImeService";
    cfg.currentSubName = "Example IME";
    cfg.tempScreenLockIme = "";
    cfg.isDefaultImeSet = false;

    ImeCfgManager::GetInstance().AddImeCfg(cfg);

    ImePersistInfo retrievedCfg = ImeCfgManager::GetInstance().GetImeCfg(1000);
    EXPECT_EQ(retrievedCfg.userId, cfg.userId);
    EXPECT_EQ(retrievedCfg.currentIme, cfg.currentIme);
    EXPECT_EQ(retrievedCfg.currentSubName, cfg.currentSubName);
    EXPECT_EQ(retrievedCfg.tempScreenLockIme, cfg.tempScreenLockIme);
    EXPECT_EQ(retrievedCfg.isDefaultImeSet, cfg.isDefaultImeSet);
}