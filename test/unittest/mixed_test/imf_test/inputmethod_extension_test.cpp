/*
* Copyright (C) 2021 Huawei Device Co., Ltd.
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
#include "inputmethod_extension.h"
#include "mock_runtime.h"
#include "mock_ability_local_record.h"
#include "mock_ohos_application.h"
#include "mock_ability_handler.h"
#include "mock_irremoteobject.h"
#include "mock_configuration.h"
#include "mock_resource_manager.h"

using namespace OHOS::AbilityRuntime;
using namespace testing;

class MockRuntime : public Runtime {
public:
    MOCK_METHOD(Language, GetLanguage, (), (const));
};

class MockAbilityLocalRecord : public AbilityLocalRecord {
public:
    MOCK_METHOD(void, SetAbility, (const std::shared_ptr<Ability>& ability), (override));
    MOCK_METHOD(void, SetToken, (const sptr<IRemoteObject>& token), (override));
};

class MockOHOSApplication : public OHOSApplication {
public:
    MOCK_METHOD(void, AttachBaseContext, (const std::shared_ptr<Context>& context), (override));
    MOCK_METHOD(void, OnConfigurationUpdated, (const AppExecFwk::Configuration& config), (override));
};

class MockAbilityHandler : public AbilityHandler {
public:
    MockAbilityHandler(const std::shared_ptr<Ability>& owner, const std::shared_ptr<EventRunner>& runner)
        : AbilityHandler(owner, runner) {}
    MOCK_METHOD(void, PostTask, (std::function<void()> task, const std::string& name), (override));
};

class MockIRemoteObject : public IRemoteObject {
public:
    MockIRemoteObject() : IRemoteObject(nullptr) {}
    MOCK_METHOD(int32_t, SendRequest, (uint32_t code, MessageParcel& data,
                                          MessageParcel& reply, MessageOption& option), (override));
};

class MockConfiguration : public AppExecFwk::Configuration {
public:
    MOCK_METHOD(void, UpdateLocale, (const std::string& language, const std::string& country), (override));
    MOCK_METHOD(void, UpdateColorMode, (int32_t colorMode), (override));
};

class MockResourceManager : public Global::Resource::ResourceManager {
public:
    MOCK_METHOD(void, UpdateConfiguration, (const AppExecFwk::Configuration& config), (override));
};

TEST(InputMethodExtensionTest, Create)
{
    std::unique_ptr<Runtime> runtime = nullptr;
    InputMethodExtension* extension = InputMethodExtension::Create(runtime);
    EXPECT_NE(extension, nullptr);
    delete extension;

    std::unique_ptr<MockRuntime> jsRuntime = std::make_unique<MockRuntime>();
    EXPECT_CALL(*jsRuntime, GetLanguage()).WillOnce(Return(Runtime::Language::JS));
    extension = InputMethodExtension::Create(std::move(jsRuntime));
    EXPECT_NE(extension, nullptr);
    delete extension;

    std::unique_ptr<MockRuntime> otherRuntime = std::make_unique<MockRuntime>();
    EXPECT_CALL(*otherRuntime, GetLanguage()).WillOnce(Return(Runtime::Language::JAVA));
    extension = InputMethodExtension::Create(std::move(otherRuntime));
    EXPECT_NE(extension, nullptr);
    delete extension;
}

TEST(InputMethodExtensionTest, Init)
{
    MockAbilityLocalRecord mockRecord;
    MockOHOSApplication mockApplication;
    MockAbilityHandler mockHandler(nullptr, nullptr);
    MockIRemoteObject mockToken;

    InputMethodExtension extension;
    extension.Init(std::make_shared<MockAbilityLocalRecord>(mockRecord),
        std::make_shared<MockOHOSApplication>(mockApplication),
        std::make_shared<MockAbilityHandler>(mockHandler),
        sptr<IRemoteObject>(new MockIRemoteObject()));
    EXPECT_NE(context, nullptr);
}

TEST(InputMethodExtensionTest, CreateAndInitContext)
{
    MockAbilityLocalRecord mockRecord;
    MockOHOSApplication mockApplication;
    MockAbilityHandler mockHandler(nullptr, nullptr);
    MockIRemoteObject mockToken;

    InputMethodExtension extension;
    std::shared_ptr<InputMethodExtensionContext> context =
        extension.CreateAndInitContext(std::make_shared<MockAbilityLocalRecord>(mockRecord),
            std::make_shared<MockOHOSApplication>(mockApplication),
            std::make_shared<MockAbilityHandler>(mockHandler),
            sptr<IRemoteObject>(new MockIRemoteObject()));

    EXPECT_NE(context, nullptr);
}

TEST(InputMethodExtensionTest, OnConfigurationUpdated)
{
    MockConfiguration mockConfig;
    MockResourceManager mockResourceManager;

    InputMethodExtension extension;
    std::shared_ptr<InputMethodExtensionContext> context =
        extension.CreateAndInitContext(std::make_shared<MockAbilityLocalRecord>(),
            std::make_shared<MockOHOSApplication>(),
            std::make_shared<MockAbilityHandler>(nullptr, nullptr),
            sptr<IRemoteObject>(new MockIRemoteObject()));

    EXPECT_CALL(mockConfig, UpdateLocale(_, _)).Times(1);
    EXPECT_CALL(mockConfig, UpdateColorMode(_)).Times(1);
    EXPECT_CALL(mockResourceManager, UpdateConfiguration(_)).Times(1);

    context->SetResourceManager(std::make_shared<MockResourceManager>(mockResourceManager));
    extension.SetContext(context);
    extension.OnConfigurationUpdated(mockConfig);
}
