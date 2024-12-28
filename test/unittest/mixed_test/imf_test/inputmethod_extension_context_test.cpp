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

#include <gtest/gtest.h>
#include "inputmethod_extension_module_loader.h"
#include "mock_runtime.h"
#include "mock_inputmethod_extension.h"

using namespace OHOS::AbilityRuntime;

class MockRuntime : public Runtime {
public:
    MOCK_METHOD(void, SomeRuntimeMethod, (), (const));
};

class MockInputMethodExtension : public InputMethodExtension {
public:
    static std::unique_ptr<InputMethodExtension> Create(const std::unique_ptr<Runtime>& runtime)
    {
        return std::make_unique<MockInputMethodExtension>();
    }
};

TEST(InputMethodExtensionModuleLoaderTest, Create)
{
    MockRuntime mockRuntime;
    std::unique_ptr<Runtime> runtime = std::make_unique<MockRuntime>(mockRuntime);

    InputMethodExtensionModuleLoader loader;
    Extension* extension = loader.Create(runtime);

    EXPECT_NE(extension, nullptr);
    EXPECT_TRUE(dynamic_cast<MockInputMethodExtension*>(extension) != nullptr);
}

TEST(InputMethodExtensionModuleLoaderTest, GetParams)
{
    InputMethodExtensionModuleLoader loader;
    std::map<std::string, std::string> params = loader.GetParams();

    EXPECT_EQ(params.size(), 2);
    EXPECT_EQ(params["type"], "2");
    EXPECT_EQ(params["name"], "InputMethodExtensionAbility");
}

TEST(InputMethodExtensionModuleLoaderTest, OHOS_EXTENSION_GetExtensionModule)
{
    void* module = OHOS_EXTENSION_GetExtensionModule();
    EXPECT_NE(module, nullptr);

    InputMethodExtensionModuleLoader* loader = static_cast<InputMethodExtensionModuleLoader*>(module);
    EXPECT_NE(loader, nullptr);
}
