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

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>
#include "inputtypemanager_fuzzer.h"
#include "input_method_ability.h"
#include "input_method_agent_stub.h"
#include "system_cmd_channel_stub.h"
#include "input_method_engine_listener_impl.h"
#include "input_data_channel_stub.h"
#include "input_method_panel.h"
#include "message_parcel.h"
#include "message_option.h"
#include "inputtypemanager.h"

using namespace OHOS;
using namespace OHOS::MiscServices;

class InputTypeManagerFuzzerTest : public testing::Test {
protected:
    void SetUp() override {
        // 设置模拟对象
        inputMethodAbility_ = std::make_shared<InputMethodAbility>();
        inputMethodAgentStub_ = std::make_shared<InputMethodAgentStub>();
        systemCmdChannelStub_ = std::make_shared<SystemCmdChannelStub>();
        inputMethodEngineListenerImpl_ = std::make_shared<InputMethodEngineListenerImpl>();
        inputDataChannelStub_ = std::make_shared<InputDataChannelStub>();
        inputMethodPanel_ = std::make_shared<InputMethodPanel>();
    }

    void TearDown() override {
        // 清理模拟对象
        inputMethodAbility_.reset();
        inputMethodAgentStub_.reset();
        systemCmdChannelStub_.reset();
        inputMethodEngineListenerImpl_.reset();
        inputDataChannelStub_.reset();
        inputMethodPanel_.reset();
    }

    std::shared_ptr<InputMethodAbility> inputMethodAbility_;
    std::shared_ptr<InputMethodAgentStub> inputMethodAgentStub_;
    std::shared_ptr<SystemCmdChannelStub> systemCmdChannelStub_;
    std::shared_ptr<InputMethodEngineListenerImpl> inputMethodEngineListenerImpl_;
    std::shared_ptr<InputDataChannelStub> inputDataChannelStub_;
    std::shared_ptr<InputMethodPanel> inputMethodPanel_;
};

TEST_F(InputTypeManagerFuzzerTest, FuzzIsSupported_ValidInput_ShouldReturnTrue) {
    uint8_t data[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    size_t size = sizeof(data) / sizeof(data[0]);
    EXPECT_TRUE(FuzzIsSupported(data, size));
}

TEST_F(InputTypeManagerFuzzerTest, FuzzIsInputType_ValidInput_ShouldReturnTrue) {
    int32_t userId = 10;
    std::string packageName = "testPackage";
    EXPECT_TRUE(FuzzIsInputType(userId, packageName));
}

TEST_F(InputTypeManagerFuzzerTest, FuzzGetImeByInputType_ValidInput_ShouldReturnTrue) {
    int32_t userId = 20;
    std::string packageName = "testPackage";
    EXPECT_TRUE(FuzzGetImeByInputType(userId, packageName));
}

TEST_F(InputTypeManagerFuzzerTest, FuzzSet_ValidInput_ShouldReturnTrue) {
    int32_t userId = 30;
    std::string packageName = "testPackage";
    EXPECT_TRUE(FuzzSet(userId, packageName));
}

TEST_F(InputTypeManagerFuzzerTest, FuzzIsStarted_ValidInput_ShouldReturnTrue) {
    int32_t userId = 40;
    std::string packageName = "testPackage";
    EXPECT_TRUE(FuzzIsStarted(userId, packageName));
}

TEST_F(InputTypeManagerFuzzerTest, FuzzIsSecurityImeStarted_ValidInput_ShouldReturnTrue) {
    int32_t userId = 50;
    std::string packageName = "testPackage";
    EXPECT_TRUE(FuzzIsSecurityImeStarted(userId, packageName));
}