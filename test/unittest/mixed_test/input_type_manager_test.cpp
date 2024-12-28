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
#include "input_type_manager.h"

#include <gtest/gtest.h>

using namespace OHOS::MiscServices;

// Mock ImeIdentification for testing
class MockImeIdentification : public ImeIdentification {
public:
    MockImeIdentification(const std::string &bundleName, const std::string &subName)
        : ImeIdentification{ bundleName, subName }
    {
    }
};

/**
 * @tc.name: IsSupportedTest
 * @tc.desc: Verify that IsSupported method returns true when input type is supported
 * @tc.type: FUNC
 */
TEST(InputTypeManagerTest, IsSupportedTest)
{
    InputTypeManager &manager = InputTypeManager::GetInstance();
    manager.Init(); // Ensure configuration is initialized
    EXPECT_TRUE(manager.IsSupported(InputType::SECURITY_INPUT));
}

/**
 * @tc.name: IsInputTypeTest
 * @tc.desc: Verify that IsInputType method returns true when IME identification exists
 * @tc.type: FUNC
 */
TEST(InputTypeManagerTest, IsInputTypeTest)
{
    InputTypeManager &manager = InputTypeManager::GetInstance();
    manager.Init(); // Ensure configuration is initialized
    MockImeIdentification ime("com.example.security", "security_ime");
    EXPECT_TRUE(manager.IsInputType(ime));
}

/**
 * @tc.name: GetImeByInputTypeTest
 * @tc.desc: Verify that GetImeByInputType method correctly returns IME identification
 * @tc.type: FUNC
 */
TEST(InputTypeManagerTest, GetImeByInputTypeTest)
{
    InputTypeManager &manager = InputTypeManager::GetInstance();
    manager.Init(); // Ensure configuration is initialized
    ImeIdentification ime;
    EXPECT_EQ(manager.GetImeByInputType(InputType::SECURITY_INPUT, ime), ErrorCode::NO_ERROR);
    EXPECT_EQ(ime.bundleName, "com.example.security");
    EXPECT_EQ(ime.subName, "security_ime");
}

/**
 * @tc.name: SetTest
 * @tc.desc: Verify that Set method correctly sets IME status
 * @tc.type: FUNC
 */
TEST(InputTypeManagerTest, SetTest)
{
    InputTypeManager &manager = InputTypeManager::GetInstance();
    MockImeIdentification ime("com.example.security", "security_ime");
    manager.Set(true, ime);
    EXPECT_TRUE(manager.IsStarted());
    EXPECT_EQ(manager.GetCurrentIme().bundleName, ime.bundleName);
    EXPECT_EQ(manager.GetCurrentIme().subName, ime.subName);
}

/**
 * @tc.name: IsStartedTest
 * @tc.desc: Verify that IsStarted method correctly returns whether IME is started
 * @tc.type: FUNC
 */
TEST(InputTypeManagerTest, IsStartedTest)
{
    InputTypeManager &manager = InputTypeManager::GetInstance();
    manager.Set(false, ImeIdentification{}); // Set to not started
    EXPECT_FALSE(manager.IsStarted());
}

/**
 * @tc.name: IsSecurityImeStartedTest
 * @tc.desc: Verify that IsSecurityImeStarted method correctly returns whether security IME is started
 * @tc.type: FUNC
 */
TEST(InputTypeManagerTest, IsSecurityImeStartedTest)
{
    InputTypeManager &manager = InputTypeManager::GetInstance();
    manager.Init(); // Ensure configuration is initialized
    MockImeIdentification ime("com.example.security", "security_ime");
    manager.Set(true, ime);
    EXPECT_TRUE(manager.IsSecurityImeStarted());
}

/**
 * @tc.name: IsCameraImeStartedTest
 * @tc.desc: Verify that IsCameraImeStarted method correctly returns whether camera IME is started
 * @tc.type: FUNC
 */
TEST(InputTypeManagerTest, IsCameraImeStartedTest)
{
    InputTypeManager &manager = InputTypeManager::GetInstance();
    manager.Init(); // Ensure configuration is initialized
    MockImeIdentification ime("com.example.camera", "camera_ime");
    manager.Set(true, ime);
    EXPECT_TRUE(manager.IsCameraImeStarted());
}

/**
 * @tc.name: IsVoiceImeStartedTest
 * @tc.desc: Verify that IsVoiceImeStarted method correctly returns whether voice IME is started
 * @tc.type: FUNC
 */
TEST(InputTypeManagerTest, IsVoiceImeStartedTest)
{
    InputTypeManager &manager = InputTypeManager::GetInstance();
    manager.Init(); // Ensure configuration is initialized
    MockImeIdentification ime("com.example.voice", "voice_ime");
    manager.Set(true, ime);
    EXPECT_TRUE(manager.IsVoiceImeStarted());
}

/**
 * @tc.name: GetCurrentInputTypeTest
 * @tc.desc: Verify that GetCurrentInputType method correctly returns current input type
 * @tc.type: FUNC
 */
TEST(InputTypeManagerTest, GetCurrentInputTypeTest)
{
    InputTypeManager &manager = InputTypeManager::GetInstance();
    manager.Init(); // Ensure configuration is initialized
    MockImeIdentification ime("com.example.security", "security_ime");
    manager.Set(true, ime);
    EXPECT_EQ(manager.GetCurrentInputType(), InputType::SECURITY_INPUT);
}

/**
 * @tc.name: GetCurrentImeTest
 * @tc.desc: Verify that GetCurrentIme method correctly returns current IME identification
 * @tc.type: FUNC
 */
TEST(InputTypeManagerTest, GetCurrentImeTest)
{
    InputTypeManager &manager = InputTypeManager::GetInstance();
    MockImeIdentification ime("com.example.security", "security_ime");
    manager.Set(true, ime);
    ImeIdentification currentIme = manager.GetCurrentIme();
    EXPECT_EQ(currentIme.bundleName, ime.bundleName);
    EXPECT_EQ(currentIme.subName, ime.subName);
}

/**
 * @tc.name: InitTest
 * @tc.desc: Verify that Init method correctly initializes input type configuration
 * @tc.type: FUNC
 */
TEST(InputTypeManagerTest, InitTest)
{
    InputTypeManager &manager = InputTypeManager::GetInstance();
    EXPECT_TRUE(manager.Init());
    EXPECT_TRUE(manager.isTypeCfgReady_.load());
}