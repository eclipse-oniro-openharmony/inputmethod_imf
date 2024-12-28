/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "ime_connection.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "element_name.h"
#include "iremote_object.h"
#include "mock_log.h"

using namespace OHOS;
using namespace MiscServices;
using namespace testing;

class MockElementName : public AppExecFwk::ElementName {
public:
    MockElementName(const std::string &bundleName, const std::string &abilityName)
        : AppExecFwk::ElementName(bundleName, abilityName)
    {
    }
};

class MockIRemoteObject : public IRemoteObject {
public:
    MockIRemoteObject() : IRemoteObject(nullptr)
    {
    }
};

class ImeConnectionTest : public testing::Test {
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

/**
 * @tc.name: OnAbilityConnectDone_ValidInputs_LogsCorrectly
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ImeConnectionTest, OnAbilityConnectDone_ValidInputs_LogsCorrectly, TestSize.Level0)
{
    MockElementName element("com.example.bundle", "com.example.ability");
    MockIRemoteObject remoteObject;
    int32_t resultCode = 0;

    ImeConnection imeConnection;
    imeConnection.OnAbilityConnectDone(element, &remoteObject, resultCode);

    EXPECT_TRUE(MockLog::Contain("ime: com.example.bundle/com.example.ability, ret: 0"));
}

/**
 * @tc.name: OnAbilityDisconnectDone_ValidInputs_LogsCorrectly
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ImeConnectionTest, OnAbilityDisconnectDone_ValidInputs_LogsCorrectly, TestSize.Level0)
{
    MockElementName element("com.example.bundle", "com.example.ability");
    int32_t resultCode = 1;

    ImeConnection imeConnection;
    imeConnection.OnAbilityDisconnectDone(element, resultCode);

    EXPECT_TRUE(MockLog::Contain("ime: com.example.bundle/com.example.ability, ret: 1"));
}