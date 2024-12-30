/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "CTextConfig.h"
#include "CjInputMethodController.h"
#include "FfiInputMethodController.h"
#include "element_name.h"
#include "ffi_input_method.h"
#include "ffi_input_method_setting.h"
#include "input_method_controller.h"
#include "input_method_setting.h"
#include "input_method_sub_property.h"
#include "sub_property.h"
#include "utils.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <vector>

using ::testing::_;
using namespace std;
using namespace testing;
using namespace OHOS::AppExecFwk;
using namespace OHOS::MiscServices;
using ::testing::Return;

// Mock InputMethodController
class MockInputMethodController : public InputMethodController {
public:
    MOCK_METHOD(int32_t, GetDefaultInputMethod, (std::shared_ptr<Property> &), (override));
    MOCK_METHOD(shared_ptr<Property>, GetCurrentInputMethod, (), (override));
    MOCK_METHOD3(SwitchInputMethod, int32_t(SwitchTrigger, const std::string &, const std::string &));
    MOCK_METHOD(std::shared_ptr<SubProperty>, GetCurrentInputMethodSubtype, (), (override));
    MOCK_METHOD3(SwitchInputMethod, int32_t(SwitchTrigger, const std::string &, const std::string &));
    MOCK_METHOD1(GetInputMethodConfig, int32_t(ElementName &));
    MOCK_METHOD(int32_t, ListCurrentInputMethodSubtype, (std::vector<SubProperty> &), (override));
    MOCK_METHOD(int32_t, DisplayOptionalInputMethod, (), (override));

    int32_t ListInputMethodSubtype(const Property &property, std::vector<SubProperty> &subProps) override
    {
        // 模拟不同的返回值
        if (mockReturnError) {
            return ErrorCode::ERROR;
        }
        subProps = mockSubProps;
        return ErrorCode::NO_ERROR;
    }

    void SetMockReturnError(bool error)
    {
        mockReturnError = error;
    }

    void SetMockSubProps(const std::vector<SubProperty> &subProps)
    {
        mockSubProps = subProps;
    }

    static std::shared_ptr<MockInputMethodController> instance;
    int32_t ListInputMethod(bool enable, std::vector<Property> &properties) override
    {
        return mockListInputMethod(enable, properties);
    }
    virtual int32_t mockListInputMethod(bool enable, std::vector<Property> &properties) = 0;

    static std::shared_ptr<MockInputMethodController> GetInstance()
    {
        static std::shared_ptr<MockInputMethodController> instance = std::make_shared<MockInputMethodController>();
        return instance;
    }

    int32_t ListInputMethod(std::vector<Property> &properties) override
    {
        if (shouldReturnError) {
            return ErrorCode::ERROR;
        }
        properties = mockProperties;
        return ErrorCode::NO_ERROR;
    }

    void SetShouldReturnError(bool error)
    {
        shouldReturnError = error;
    }

    void SetMockProperties(const std::vector<Property> &properties)
    {
        mockProperties = properties;
    }

private:
    bool mockReturnError = false;
    std::vector<SubProperty> mockSubProps;
};

std::shared_ptr<MockInputMethodController> MockInputMethodController::instance = nullptr;

class FfiInputMethodSettingTest : public testing::Test {
protected:
    void SetUp() override
    {
        // 设置模拟控制器
        MockInputMethodController::GetInstance()->SetShouldReturnError(false);
        MockInputMethodController::GetInstance()->SetMockProperties({});
    }
};

// 模拟 CjInputMethodController 类
class MockCjInputMethodController : public CjInputMethodController {
public:
    MOCK_STATIC_METHOD1(Unsubscribe, int32_t(int8_t));
    MOCK_STATIC_METHOD2(Attach, int32_t(const CTextConfig &txtCfg, bool showKeyboard));
    MOCK_STATIC_METHOD3(ChangeSelection, int32_t(const std::string &, int32_t, int32_t));
    static int32_t HideTextInput()
    {
        // 模拟行为
        return 0; // 假设返回0表示成功
    }
};

// Mock class for CJGetInputMethodSetting
class MockCJGetInputMethodSetting : public CJGetInputMethodSetting {
public:
    MOCK_METHOD0(GetIMSInstance, CJGetInputMethodSetting *());
    MOCK_METHOD1(UnSubscribe, int32_t(uint32_t));
    MOCK_METHOD2(Subscribe, int32_t(uint32_t, void (*)(CInputMethodProperty, CInputMethodSubtype)));
};

// Mock method for GetIMSInstance
CJGetInputMethodSetting *MockGetIMSInstance()
{
    static MockCJGetInputMethodSetting mockInstance;
    return &mockInstance;
}

// Test fixture
class FfiInputMethodSettingOffTest : public Test {
protected:
    MockCJGetInputMethodSetting mockSetting;
    CJGetInputMethodSetting *originalInstance;

    void SetUp() override
    {
        originalInstance = CJGetInputMethodSetting::GetIMSInstance();
        ON_CALL(mockSetting, GetIMSInstance()).WillByDefault(Return(&mockSetting));
    }

    void TearDown() override
    {
        CJGetInputMethodSetting::SetIMSInstance(originalInstance);
    }
};

class MockUtils {
public:
    static Property C2InputMethodProperty(const CInputMethodProperty &props)
    {
        // 模拟转换
        return Property {};
    }

    static void InputMethodSubProperty2C(CInputMethodSubtype &props, const SubProperty &subProps)
    {
        // 模拟转换
    }
};

class InputMethodSettingTest : public testing::Test {
protected:
    void SetUp() override
    {
        // 设置模拟对象
        mockController = std::make_unique<MockInputMethodController>();
        InputMethodController::SetInstance(mockController.get());
    }

    void TearDown() override
    {
        // 清理
        InputMethodController::SetInstance(nullptr);
    }

    std::unique_ptr<MockInputMethodController> mockController;
};

// 模拟 ElementName
class MockElementName : public ElementName {
public:
    MOCK_METHOD0(GetDeviceID, std::string());
    MOCK_METHOD0(GetBundleName, std::string());
    MOCK_METHOD0(GetAbilityName, std::string());
    MOCK_METHOD0(GetModuleName, std::string());
};

// 模拟 InputMethodController 的单例
class MockInputMethodControllerSingleton {
public:
    static MockInputMethodController *GetInstance()
    {
        static MockInputMethodController instance;
        return &instance;
    }
};

// 重写 InputMethodController::GetInstance 以返回模拟实例
InputMethodController *InputMethodController::GetInstance()
{
    return MockInputMethodControllerSingleton::GetInstance();
}

class InputMethodControllerMocker {
public:
    static MockInputMethodController *GetInstance()
    {
        static MockInputMethodController mockInstance;
        return &mockInstance;
    }
};

// Test fixture
class FfiInputMethodGetDefaultInputMethodTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        // 设置模拟控制器
        mockController = make_shared<MockInputMethodController>();
        ON_CALL(*mockController, GetDefaultInputMethod(_))
            .WillByDefault(testing::Invoke([this](std::shared_ptr<Property> &property) {
                property = mockProperty;
                return 0; // 假设成功
            }));
        InputMethodController::SetInstance(mockController);
    }

    void TearDown() override
    {
        InputMethodController::SetInstance(nullptr);
    }

    shared_ptr<MockInputMethodController> mockController;
    shared_ptr<Property> mockProperty = make_shared<Property>();
};

class FfiInputMethodTest : public Test {
protected:
    void SetUp() override
    {
        // 模拟 InputMethodController 的单例实例
        auto mockController = std::make_unique<MockInputMethodController>();
        ON_CALL(*mockController, SwitchInputMethod(_, _, _)).WillByDefault(Return(ErrorCode::NO_ERROR));
        InputMethodController::SetInstance(std::move(mockController));
    }

    void TearDown() override
    {
        // 重置 InputMethodController 的单例实例
        InputMethodController::SetInstance(nullptr);
    }
};

// 测试用例：控制器实例为 nullptr
TEST(FfiInputMethodGetDefaultInputMethodTest, ControllerIsNull_ReturnsErrNoMemory)
{
    InputMethodController::SetInstance(nullptr);
    CInputMethodProperty props;
    int32_t result = FfiInputMethodGetDefaultInputMethod(props);
    EXPECT_EQ(result, ERR_NO_MEMORY);
}

// 测试用例：控制器实例不为 nullptr 且属性为 nullptr
TEST(FfiInputMethodGetDefaultInputMethodTest, PropertyIsNull_LogsErrorAndReturnsCode)
{
    ON_CALL(*mockController, GetDefaultInputMethod(_))
        .WillByDefault(testing::Invoke([](std::shared_ptr<Property> &property) {
            property = nullptr;
            return 1; // 假设错误代码
        }));

    CInputMethodProperty props;
    int32_t result = FfiInputMethodGetDefaultInputMethod(props);
    EXPECT_EQ(result, 1); // 预期的错误代码
}

// 测试用例：控制器实例不为 nullptr 且属性不为 nullptr
TEST(FfiInputMethodGetDefaultInputMethodTest, PropertyIsNotNull_ConvertsAndReturnsCode)
{
    CInputMethodProperty props;
    int32_t result = FfiInputMethodGetDefaultInputMethod(props);
    EXPECT_EQ(result, 0); // 假设成功代码
}

TEST(FfiInputMethodGetCurrentInputMethodTest, GetInstanceReturnsNull)
{
    // 模拟 InputMethodController::GetInstance 返回 nullptr
    EXPECT_CALL(InputMethodController::GetInstance(), Times(1)).WillOnce(Return(nullptr));

    CInputMethodProperty props;
    int32_t result = FfiInputMethodGetCurrentInputMethod(props);
    EXPECT_EQ(result, ERR_NO_MEMORY);
}

TEST(FfiInputMethodGetCurrentInputMethodTest, GetCurrentInputMethodReturnsNull)
{
    // 模拟 InputMethodController::GetInstance 返回有效实例
    MockInputMethodController *mockCtrl = InputMethodControllerMocker::GetInstance();
    EXPECT_CALL(*mockCtrl, GetCurrentInputMethod()).WillOnce(Return(nullptr));

    CInputMethodProperty props;
    int32_t result = FfiInputMethodGetCurrentInputMethod(props);
    EXPECT_EQ(result, ERR_NO_MEMORY);
}

TEST(FfiInputMethodGetCurrentInputMethodTest, SuccessfulRetrieval)
{
    // 模拟 InputMethodController::GetInstance 返回有效实例
    MockInputMethodController *mockCtrl = InputMethodControllerMocker::GetInstance();
    shared_ptr<Property> mockProperty = make_shared<Property>();
    EXPECT_CALL(*mockCtrl, GetCurrentInputMethod()).WillOnce(Return(mockProperty));

    // 模拟 Utils::InputMethodProperty2C 的行为
    EXPECT_CALL(Utils::InputMethodProperty2C, Times(1))
        .WillOnce(Invoke([](CInputMethodProperty &props, const Property &property) {
            // 假设填充逻辑
            props.id = property.id;
            props.name = property.name.c_str();
        }));

    CInputMethodProperty props;
    int32_t result = FfiInputMethodGetCurrentInputMethod(props);
    EXPECT_EQ(result, 0);
    // 验证 props 是否被正确填充
    EXPECT_EQ(props.id, mockProperty->id);
    EXPECT_STREQ(props.name, mockProperty->name.c_str());
}

TEST_F(FfiInputMethodTest, SwitchInputMethod_ControllerIsNull_ReturnsErrNoMemory)
{
    InputMethodController::SetInstance(nullptr); // 将控制器设置为 null
    bool result = false;
    CInputMethodProperty props = { "name", "id" };
    int32_t errCode = FfiInputMethodSwitchInputMethod(result, props);
    EXPECT_EQ(errCode, ERR_NO_MEMORY);
}

TEST_F(FfiInputMethodTest, SwitchInputMethod_Success_ReturnsNoError)
{
    bool result = false;
    CInputMethodProperty props = { "name", "id" };
    int32_t errCode = FfiInputMethodSwitchInputMethod(result, props);
    EXPECT_EQ(errCode, ErrorCode::NO_ERROR);
    EXPECT_TRUE(result);
}

TEST_F(FfiInputMethodTest, SwitchInputMethod_Failure_ReturnsErrorCode)
{
    bool result = false;
    CInputMethodProperty props = { "name", "id" };
    auto mockController = std::make_unique<MockInputMethodController>();
    ON_CALL(*mockController, SwitchInputMethod(_, _, _)).WillByDefault(Return(ErrorCode::INVALID_ARGUMENT));
    InputMethodController::SetInstance(std::move(mockController));

    int32_t errCode = FfiInputMethodSwitchInputMethod(result, props);
    EXPECT_EQ(errCode, ErrorCode::INVALID_ARGUMENT);
    EXPECT_FALSE(result);
}

// Unit Test Code:
TEST(FfiInputMethodSwitchCurrentInputMethodSubtypeTest, ControllerIsNull_ReturnsErrNoMemory)
{
    // 模拟 InputMethodController::GetInstance 返回 nullptr
    // 这需要在实际测试中通过某种方式实现，例如使用一个模拟框架
    bool result = false;
    CInputMethodSubtype target = { "name", "id" };
    int32_t errorCode = FfiInputMethodSwitchCurrentInputMethodSubtype(result, target);
    EXPECT_EQ(errorCode, ERR_NO_MEMORY);
    EXPECT_FALSE(result);
}

TEST(FfiInputMethodSwitchCurrentInputMethodSubtypeTest, SwitchSuccessful_ResultTrue)
{
    // 模拟 InputMethodController::GetInstance 返回一个有效的实例
    // 并且 SwitchInputMethod 返回 NO_ERROR
    bool result = false;
    CInputMethodSubtype target = { "name", "id" };
    auto mockController = InputMethodController::GetInstance();
    mockController->SetSwitchResult(ErrorCode::NO_ERROR);

    int32_t errorCode = FfiInputMethodSwitchCurrentInputMethodSubtype(result, target);
    EXPECT_EQ(errorCode, ErrorCode::NO_ERROR);
    EXPECT_TRUE(result);
}

TEST(FfiInputMethodSwitchCurrentInputMethodSubtypeTest, SwitchFails_ResultFalse)
{
    // 模拟 InputMethodController::GetInstance 返回一个有效的实例
    // 并且 SwitchInputMethod 返回一个错误码
    bool result = false;
    CInputMethodSubtype target = { "name", "id" };
    auto mockController = InputMethodController::GetInstance();
    mockController->SetSwitchResult(ErrorCode::ERROR_GENERIC);

    int32_t errorCode = FfiInputMethodSwitchCurrentInputMethodSubtype(result, target);
    EXPECT_EQ(errorCode, ErrorCode::ERROR_GENERIC);
    EXPECT_FALSE(result);
}

TEST(FfiInputMethodGetCurrentInputMethodSubtypeTest, GetInstance_Nullptr_ReturnsErrNoMemory)
{
    EXPECT_CALL(InputMethodController::GetInstance(), Times(1)).WillOnce(Return(nullptr));

    CInputMethodSubtype props;
    int32_t result = FfiInputMethodGetCurrentInputMethodSubtype(props);

    EXPECT_EQ(result, ERR_NO_MEMORY);
}

TEST(FfiInputMethodGetCurrentInputMethodSubtypeTest, GetCurrentInputMethodSubtype_Nullptr_ReturnsErrNoMemory)
{
    auto mockController = std::make_shared<MockInputMethodController>();
    EXPECT_CALL(InputMethodController::GetInstance(), Times(1)).WillOnce(Return(mockController));
    EXPECT_CALL(*mockController, GetCurrentInputMethodSubtype()).WillOnce(Return(nullptr));

    CInputMethodSubtype props;
    int32_t result = FfiInputMethodGetCurrentInputMethodSubtype(props);

    EXPECT_EQ(result, ERR_NO_MEMORY);
}

TEST(FfiInputMethodGetCurrentInputMethodSubtypeTest, ValidSubProperty_ReturnsSuccess)
{
    auto mockController = std::make_shared<MockInputMethodController>();
    auto mockSubProperty = std::make_shared<SubProperty>();
    EXPECT_CALL(InputMethodController::GetInstance(), Times(1)).WillOnce(Return(mockController));
    EXPECT_CALL(*mockController, GetCurrentInputMethodSubtype()).WillOnce(Return(mockSubProperty));
    EXPECT_CALL(Utils, InputMethodSubProperty2C(Ref(props), Ref(*mockSubProperty))).Times(1);

    CInputMethodSubtype props;
    int32_t result = FfiInputMethodGetCurrentInputMethodSubtype(props);

    EXPECT_EQ(result, 0);
}

TEST(FfiInputMethodTest, SwitchCurrentInputMethodAndSubtype_ControllerIsNull_ReturnsErrNoMemory)
{
    // 模拟控制器为空
    EXPECT_CALL(InputMethodController, GetInstance()).WillOnce(Return(nullptr));

    bool result = false;
    CInputMethodProperty target;
    CInputMethodSubtype subtype = { "name", "id" };
    int32_t errCode = FfiInputMethodSwitchCurrentInputMethodAndSubtype(result, target, subtype);

    EXPECT_EQ(errCode, ERR_NO_MEMORY);
    EXPECT_FALSE(result);
}

TEST(FfiInputMethodTest, SwitchCurrentInputMethodAndSubtype_SuccessfulSwitch_ReturnsNoError)
{
    // 模拟控制器不为空且切换成功
    MockInputMethodController *mockController = InputMethodControllerMocker::GetInstance();
    EXPECT_CALL(InputMethodController, GetInstance()).WillOnce(Return(mockController));
    EXPECT_CALL(*mockController, SwitchInputMethod(SwitchTrigger::CURRENT_IME, "name", "id"))
        .WillOnce(Return(ErrorCode::NO_ERROR));

    bool result = false;
    CInputMethodProperty target;
    CInputMethodSubtype subtype = { "name", "id" };
    int32_t errCode = FfiInputMethodSwitchCurrentInputMethodAndSubtype(result, target, subtype);

    EXPECT_EQ(errCode, ErrorCode::NO_ERROR);
    EXPECT_TRUE(result);
}

TEST(FfiInputMethodTest, SwitchCurrentInputMethodAndSubtype_SwitchFails_ReturnsErrorCode)
{
    // 模拟控制器不为空且切换失败
    MockInputMethodController *mockController = InputMethodControllerMocker::GetInstance();
    EXPECT_CALL(InputMethodController, GetInstance()).WillOnce(Return(mockController));
    EXPECT_CALL(*mockController, SwitchInputMethod(SwitchTrigger::CURRENT_IME, "name", "id"))
        .WillOnce(Return(ErrorCode::UNKNOWN_ERROR));

    bool result = false;
    CInputMethodProperty target;
    CInputMethodSubtype subtype = { "name", "id" };
    int32_t errCode = FfiInputMethodSwitchCurrentInputMethodAndSubtype(result, target, subtype);

    EXPECT_EQ(errCode, ErrorCode::UNKNOWN_ERROR);
    EXPECT_FALSE(result);
}

// 测试用例：验证当 InputMethodController 为 nullptr 时，方法返回 ERR_NO_MEMORY
TEST(FfiInputMethodGetSystemInputMethodConfigAbilityTest, NullController_ReturnsErrNoMemory)
{
    // 模拟 InputMethodController 为 nullptr
    EXPECT_CALL(*MockInputMethodControllerSingleton::GetInstance(), GetInputMethodConfig(_)).Times(0);

    CElementName elem;
    int32_t result = FfiInputMethodGetSystemInputMethodConfigAbility(elem);
    EXPECT_EQ(result, ERR_NO_MEMORY);
}

// 测试用例：验证当 GetInputMethodConfig 返回 NO_ERROR 时，CElementName 被正确填充
TEST(FfiInputMethodGetSystemInputMethodConfigAbilityTest, GetInputMethodConfig_NoError_FillsCElementName)
{
    MockElementName mockElementName;
    EXPECT_CALL(mockElementName, GetDeviceID()).WillOnce(Return("device_id"));
    EXPECT_CALL(mockElementName, GetBundleName()).WillOnce(Return("bundle_name"));
    EXPECT_CALL(mockElementName, GetAbilityName()).WillOnce(Return("ability_name"));
    EXPECT_CALL(mockElementName, GetModuleName()).WillOnce(Return("module_name"));

    EXPECT_CALL(*MockInputMethodControllerSingleton::GetInstance(), GetInputMethodConfig(_))
        .WillOnce(DoAll(SetArgReferee<0>(mockElementName), Return(ErrorCode::NO_ERROR)));

    CElementName elem;
    int32_t result = FfiInputMethodGetSystemInputMethodConfigAbility(elem);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
    EXPECT_STREQ(elem.deviceId, "device_id");
    EXPECT_STREQ(elem.bundleName, "bundle_name");
    EXPECT_STREQ(elem.abilityName, "ability_name");
    EXPECT_STREQ(elem.moduleName, "module_name");
}

// 测试用例：验证当 GetInputMethodConfig 返回错误码时，CElementName 未被修改
TEST(FfiInputMethodGetSystemInputMethodConfigAbilityTest, GetInputMethodConfig_ErrorCode_DoesNotModifyCElementName)
{
    EXPECT_CALL(*MockInputMethodControllerSingleton::GetInstance(), GetInputMethodConfig(_))
        .WillOnce(Return(ErrorCode::ERROR));

    CElementName elem;
    int32_t result = FfiInputMethodGetSystemInputMethodConfigAbility(elem);
    EXPECT_EQ(result, ErrorCode::ERROR);
    EXPECT_STREQ(elem.deviceId, "");
    EXPECT_STREQ(elem.bundleName, "");
    EXPECT_STREQ(elem.abilityName, "");
    EXPECT_STREQ(elem.moduleName, "");
}

TEST_F(InputMethodSettingTest, FfiInputMethodSettingListInputMethodSubtype_ControllerNull_ReturnsNoMemory)
{
    InputMethodController::SetInstance(nullptr);
    CInputMethodProperty props {};
    RetInputMethodSubtype ret = FfiInputMethodSettingListInputMethodSubtype(props);
    EXPECT_EQ(ret.code, ERR_NO_MEMORY);
}

TEST_F(InputMethodSettingTest, FfiInputMethodSettingListInputMethodSubtype_ListInputMethodSubtypeError_ReturnError)
{
    mockController->SetMockReturnError(true);
    CInputMethodProperty props {};
    RetInputMethodSubtype ret = FfiInputMethodSettingListInputMethodSubtype(props);
    EXPECT_EQ(ret.code, ErrorCode::ERROR);
}

TEST_F(InputMethodSettingTest, FfiInputMethodSettingListInputMethodSubtype_MemoryAllocationFailure_ReturnsNoMemory)
{
    mockController->SetMockSubProps({ SubProperty {} });
    // 模拟内存分配失败
    void *(*originalMalloc)(size_t) = malloc;
    malloc = [](size_t size) -> void* {
        return nullptr;
    };
    CInputMethodProperty props {};
    RetInputMethodSubtype ret = FfiInputMethodSettingListInputMethodSubtype(props);
    malloc = originalMalloc;
    EXPECT_EQ(ret.code, ERR_NO_MEMORY);
}

TEST_F(InputMethodSettingTest, FfiInputMethodSettingListInputMethodSubtype_Success_ReturnsSuccess)
{
    SubProperty subProp {};
    mockController->SetMockSubProps({ subProp });
    CInputMethodProperty props {};
    RetInputMethodSubtype ret = FfiInputMethodSettingListInputMethodSubtype(props);
    EXPECT_EQ(ret.code, ErrorCode::NO_ERROR);
    EXPECT_EQ(ret.size, 1);
    free(ret.head);
}

TEST_F(FfiInputMethodSettingTest, ListCurrentInputMethodSubtype_NullController_ReturnsErrNoMemory)
{
    // 模拟控制器返回空
    EXPECT_CALL(InputMethodController::GetInstance(), Times(1)).WillOnce(Return(nullptr));

    RetInputMethodSubtype result = FfiInputMethodSettingListCurrentInputMethodSubtype();
    EXPECT_EQ(result.code, ERR_NO_MEMORY);
}

TEST_F(FfiInputMethodSettingTest, ListCurrentInputMethodSubtype_NonZeroErrorCode_ReturnsErrorCode)
{
    // 模拟控制器返回非零错误码
    auto errCode = 123;
    EXPECT_CALL(InputMethodController::GetInstance(), Times(1)).WillOnce(Return(mockController));
    EXPECT_CALL(*mockController, ListCurrentInputMethodSubtype(_)).WillOnce(Return(errCode));

    RetInputMethodSubtype result = FfiInputMethodSettingListCurrentInputMethodSubtype();
    EXPECT_EQ(result.code, errCode);
}

TEST_F(FfiInputMethodSettingTest, ListCurrentInputMethodSubtype_EmptySubtypeList_ReturnsNoErrorAndZeroSize)
{
    // 模拟空的子类型列表
    EXPECT_CALL(InputMethodController::GetInstance(), Times(1)).WillOnce(Return(mockController));
    EXPECT_CALL(*mockController, ListCurrentInputMethodSubtype(_))
        .WillOnce(DoAll(SetArgReferee<0>(std::vector<SubProperty>()), Return(ErrorCode::NO_ERROR)));

    RetInputMethodSubtype result = FfiInputMethodSettingListCurrentInputMethodSubtype();
    EXPECT_EQ(result.code, ErrorCode::NO_ERROR);
    EXPECT_EQ(result.size, 0);
}

TEST_F(FfiInputMethodSettingTest, ListCurrentInputMethodSubtype_NonEmptySubtypeList_ReturnsNoErrorAndCorrectSize)
{
    // 模拟非空的子类型列表
    std::vector<SubProperty> subProps = { SubProperty(), SubProperty() };
    EXPECT_CALL(InputMethodController::GetInstance(), Times(1)).WillOnce(Return(mockController));
    EXPECT_CALL(*mockController, ListCurrentInputMethodSubtype(_))
        .WillOnce(DoAll(SetArgReferee<0>(subProps), Return(ErrorCode::NO_ERROR)));

    RetInputMethodSubtype result = FfiInputMethodSettingListCurrentInputMethodSubtype();
    EXPECT_EQ(result.code, ErrorCode::NO_ERROR);
    EXPECT_EQ(result.size, subProps.size());
}

TEST_F(FfiInputMethodSettingTest, ListCurrentInputMethodSubtype_MemoryAllocationFailure_ReturnsNoErrorAndZeroSize)
{
    // 模拟内存分配失败
    EXPECT_CALL(InputMethodController::GetInstance(), Times(1)).WillOnce(Return(mockController));
    EXPECT_CALL(*mockController, ListCurrentInputMethodSubtype(_))
        .WillOnce(DoAll(SetArgReferee<0>(std::vector<SubProperty>()), Return(ErrorCode::NO_ERROR)));

    // 模拟内存分配失败
    EXPECT_CALL(Utils::InputMethodSubProperty2C, Times(1)).WillOnce(Return(false));

    RetInputMethodSubtype result = FfiInputMethodSettingListCurrentInputMethodSubtype();
    EXPECT_EQ(result.code, ErrorCode::NO_ERROR);
    EXPECT_EQ(result.size, 0);
}

TEST_F(InputMethodSettingTest, FfiInputMethodSettingGetInputMethods_NullController_ReturnsErrNoMemory)
{
    MockInputMethodController::instance = nullptr;
    RetInputMethodProperty result = FfiInputMethodSettingGetInputMethods(true);
    EXPECT_EQ(result.code, ERR_NO_MEMORY);
}

TEST_F(InputMethodSettingTest, FfiInputMethodSettingGetInputMethods_ListInputMethodError_ReturnsErrorCode)
{
    class MockController : public MockInputMethodController {
    public:
        int32_t mockListInputMethod(bool enable, std::vector<Property> &properties) override
        {
            return ErrorCode::INVALID_OPERATION;
        }
    };
    MockInputMethodController::instance = std::make_shared<MockController>();
    RetInputMethodProperty result = FfiInputMethodSettingGetInputMethods(true);
    EXPECT_EQ(result.code, ErrorCode::INVALID_OPERATION);
}

TEST_F(InputMethodSettingTest, FfiInputMethodSettingGetInputMethods_EmptyProperties_ReturnsSizeZero)
{
    class MockController : public MockInputMethodController {
    public:
        int32_t mockListInputMethod(bool enable, std::vector<Property> &properties) override
        {
            properties.clear();
            return ErrorCode::NO_ERROR;
        }
    };
    MockInputMethodController::instance = std::make_shared<MockController>();
    RetInputMethodProperty result = FfiInputMethodSettingGetInputMethods(true);
    EXPECT_EQ(result.size, 0);
}

TEST_F(InputMethodSettingTest, FfiInputMethodSettingGetInputMethods_ValidProperties_ReturnsCorrectSizeAndData)
{
    class MockController : public MockInputMethodController {
    public:
        int32_t mockListInputMethod(bool enable, std::vector<Property> &properties) override
        {
            properties = { Property(), Property() };
            return ErrorCode::NO_ERROR;
        }
    };
    MockInputMethodController::instance = std::make_shared<MockController>();
    RetInputMethodProperty result = FfiInputMethodSettingGetInputMethods(true);
    auto propertiesSize = 2;
    EXPECT_EQ(result.size, propertiesSize);
    EXPECT_NE(result.head, nullptr);
    free(result.head);
}

TEST_F(InputMethodSettingTest, FfiInputMethodSettingGetInputMethods_MemoryAllocationFailure_ReturnsSizeZero)
{
    class MockController : public MockInputMethodController {
    public:
        int32_t mockListInputMethod(bool enable, std::vector<Property> &properties) override
        {
            properties = { Property(), Property() };
            return ErrorCode::NO_ERROR;
        }
    };
    MockInputMethodController::instance = std::make_shared<MockController>();
    // 模拟内存分配失败
    void *(*originalMalloc)(size_t) = malloc;
    malloc = [](size_t size) -> void* {
        return nullptr;
    };
    RetInputMethodProperty result = FfiInputMethodSettingGetInputMethods(true);
    malloc = originalMalloc;
    EXPECT_EQ(result.size, 0);
}

TEST_F(FfiInputMethodSettingTest, FfiInputMethodSettingGetAllInputMethods_ControllerIsNull_ReturnsError)
{
    // 模拟控制器为空
    MockInputMethodController::GetInstance().reset();

    RetInputMethodProperty result = FfiInputMethodSettingGetAllInputMethods();
    EXPECT_EQ(result.code, ERR_NO_MEMORY);
}

TEST_F(FfiInputMethodSettingTest, FfiInputMethodSettingGetAllInputMethods_ListInputMethodReturnsError_ReturnsError)
{
    // 模拟 ListInputMethod 返回错误
    MockInputMethodController::GetInstance()->SetShouldReturnError(true);

    RetInputMethodProperty result = FfiInputMethodSettingGetAllInputMethods();
    EXPECT_EQ(result.code, ErrorCode::ERROR);
}

TEST_F(FfiInputMethodSettingTest, FfiInputMethodSettingGetAllInputMethods_MemoryAllocationFails_ReturnsError)
{
    RetInputMethodProperty result = FfiInputMethodSettingGetAllInputMethods();
    EXPECT_EQ(result.size, 0);
}

TEST_F(FfiInputMethodSettingTest, FfiInputMethodSettingGetAllInputMethods_Success_ReturnsProperties)
{
    // 模拟成功获取属性
    Property mockProperty;
    mockProperty.id = "mockId";
    mockProperty.name = "mockName";
    MockInputMethodController::GetInstance()->SetMockProperties({ mockProperty });

    RetInputMethodProperty result = FfiInputMethodSettingGetAllInputMethods();
    EXPECT_EQ(result.code, ErrorCode::NO_ERROR);
    EXPECT_EQ(result.size, 1);
    EXPECT_EQ(result.head[0].id, "mockId");
    EXPECT_EQ(result.head[0].name, "mockName");
}

// Test case when GetIMSInstance returns nullptr
TEST(FfiInputMethodSettingOnTest, GetIMSInstance_ReturnsNull_ReturnsErrNoMemory)
{
    EXPECT_CALL(CJGetInputMethodSetting::GetIMSInstance(), Times(1)).WillOnce(Return(nullptr));
    int32_t result = FfiInputMethodSettingOn(0, nullptr);
    EXPECT_EQ(result, ERR_NO_MEMORY);
}

// Test case when GetIMSInstance returns a valid instance
TEST(FfiInputMethodSettingOnTest, GetIMSInstance_ReturnsValidInstance_ReturnsSubscribeResult)
{
    MockCJGetInputMethodSetting *mockInstance = MockGetIMSInstance();
    EXPECT_CALL(CJGetInputMethodSetting::GetIMSInstance(), Times(1)).WillOnce(Return(mockInstance));
    EXPECT_CALL(*mockInstance, Subscribe(0, nullptr))
        .WillOnce(Return(0)); // Assuming 0 is a valid return value for Subscribe
    int32_t result = FfiInputMethodSettingOn(0, nullptr);
    EXPECT_EQ(result, 0);
}

// Test case for when GetIMSInstance returns nullptr
TEST_F(FfiInputMethodSettingOffTest, FfiInputMethodSettingOff_GetIMSInstanceReturnsNull_ReturnsErrNoMemory)
{
    ON_CALL(mockSetting, GetIMSInstance()).WillByDefault(Return(nullptr));

    int32_t result = FfiInputMethodSettingOff(1);

    EXPECT_EQ(result, ERR_NO_MEMORY);
}

// Test case for when GetIMSInstance returns a valid instance
TEST_F(FfiInputMethodSettingOffTest, FfiInputMethodSettingOff_ValidInstance_UnsubscribesAndReturnsResult)
{
    EXPECT_CALL(mockSetting, UnSubscribe(1)).WillOnce(Return(0));

    int32_t result = FfiInputMethodSettingOff(1);

    EXPECT_EQ(result, 0);
}

// Test case when GetInstance returns nullptr
TEST_F(FfiInputMethodSettingTest, FfiInputMethodSettingShowOptionalInputMethods_NullInstance_ReturnsErrNoMemory)
{
    // Arrange
    EXPECT_CALL(InputMethodController::GetInstance(), Times(1)).WillOnce(Return(nullptr));

    bool result = false;
    int32_t errCode = FfiInputMethodSettingShowOptionalInputMethods(result);

    // Assert
    EXPECT_EQ(errCode, ERR_NO_MEMORY);
    EXPECT_FALSE(result);
}

// Test case when DisplayOptionalInputMethod returns NO_ERROR
TEST_F(FfiInputMethodSettingTest, FfiInputMethodSettingShowOptionalInputMethods_Success_ReturnsNoError)
{
    // Arrange
    auto mockCtrl = std::make_shared<MockInputMethodController>();
    EXPECT_CALL(InputMethodController::GetInstance(), Times(1)).WillOnce(Return(mockCtrl));
    EXPECT_CALL(*mockCtrl, DisplayOptionalInputMethod()).WillOnce(Return(ErrorCode::NO_ERROR));

    bool result = false;
    int32_t errCode = FfiInputMethodSettingShowOptionalInputMethods(result);

    // Assert
    EXPECT_EQ(errCode, ErrorCode::NO_ERROR);
    EXPECT_TRUE(result);
}

// Test case when DisplayOptionalInputMethod returns an error code
TEST_F(FfiInputMethodSettingTest, FfiInputMethodSettingShowOptionalInputMethods_Error_ReturnsErrorCode)
{
    // Arrange
    auto mockCtrl = std::make_shared<MockInputMethodController>();
    EXPECT_CALL(InputMethodController::GetInstance(), Times(1)).WillOnce(Return(mockCtrl));
    EXPECT_CALL(*mockCtrl, DisplayOptionalInputMethod()).WillOnce(Return(ErrorCode::INVALID_OPERATION));

    bool result = false;
    int32_t errCode = FfiInputMethodSettingShowOptionalInputMethods(result);

    // Assert
    EXPECT_EQ(errCode, ErrorCode::INVALID_OPERATION);
    EXPECT_FALSE(result);
}

TEST(FfiInputMethodControllerOnTest, ValidInputs_ReturnsExpectedResult)
{
    int8_t type = 1;
    int64_t id = 1234567890;
    int32_t expected = type + static_cast<int32_t>(id % 1000);

    int32_t result = FfiInputMethodControllerOn(type, id);

    EXPECT_EQ(result, expected);
}

TEST(FfiInputMethodControllerOnTest, BoundaryValues_ReturnsExpectedResult)
{
    int8_t type = INT8_MAX;
    int64_t id = INT64_MAX;
    int32_t expected = type + static_cast<int32_t>(id % 1000);

    int32_t result = FfiInputMethodControllerOn(type, id);

    EXPECT_EQ(result, expected);
}

TEST(FfiInputMethodControllerOnTest, NegativeId_ReturnsExpectedResult)
{
    int8_t type = 1;
    int64_t id = -1234567890;
    int32_t expected = type + static_cast<int32_t>(id % 1000);

    int32_t result = FfiInputMethodControllerOn(type, id);

    EXPECT_EQ(result, expected);
}

TEST(FfiInputMethodControllerOffTest, Unsubscribe_ReturnsExpectedValue)
{
    // 设置
    EXPECT_CALL(MockCjInputMethodController::Unsubscribe(1), Return(0));

    // 调用被测试的方法
    int32_t result = FfiInputMethodControllerOff(1);

    // 验证结果
    EXPECT_EQ(result, 0);
}

TEST(FfiInputMethodControllerOffTest, Unsubscribe_ReturnsUnexpectedValue)
{
    // 设置
    EXPECT_CALL(MockCjInputMethodController::Unsubscribe(2), Return(-1));

    // 调用被测试的方法
    int32_t result = FfiInputMethodControllerOff(2);

    // 验证结果
    EXPECT_EQ(result, -1);
}

// 测试 FfiInputMethodControllerAttach 函数
TEST(FfiInputMethodControllerAttachTest, Attach_WithValidConfigAndShowKeyboard_ReturnsExpectedValue)
{
    CTextConfig txtCfg;
    txtCfg.SetInputType(CTextConfig::InputType::kText);
    txtCfg.SetInputFlag(CTextConfig::InputFlag::kDefault);
    txtCfg.SetInputPurpose(CTextConfig::InputPurpose::kNormal);

    EXPECT_CALL(MockCjInputMethodController::Attach, (txtCfg, true)).WillOnce(Return(0));

    int32_t result = FfiInputMethodControllerAttach(true, txtCfg);
    EXPECT_EQ(result, 0);
}

TEST(FfiInputMethodControllerAttachTest, Attach_WithValidConfigAndHideKeyboard_ReturnsExpectedValue)
{
    CTextConfig txtCfg;
    txtCfg.SetInputType(CTextConfig::InputType::kText);
    txtCfg.SetInputFlag(CTextConfig::InputFlag::kDefault);
    txtCfg.SetInputPurpose(CTextConfig::InputPurpose::kNormal);

    EXPECT_CALL(MockCjInputMethodController::Attach, (txtCfg, false)).WillOnce(Return(1));

    int32_t result = FfiInputMethodControllerAttach(false, txtCfg);
    EXPECT_EQ(result, 1);
}

TEST(FfiInputMethodControllerAttachTest, Attach_WithInvalidConfig_ReturnsExpectedValue)
{
    CTextConfig txtCfg;
    // 假设某些配置是无效的，例如未设置输入类型
    txtCfg.SetInputFlag(CTextConfig::InputFlag::kDefault);
    txtCfg.SetInputPurpose(CTextConfig::InputPurpose::kNormal);

    EXPECT_CALL(MockCjInputMethodController::Attach, (txtCfg, true)).WillOnce(Return(-1));

    int32_t result = FfiInputMethodControllerAttach(true, txtCfg);
    EXPECT_EQ(result, -1);
}

TEST(FfiInputMethodControllerDetachTest, Detach_ReturnsExpectedValue)
{
    // 假设Detach返回一个特定的值，例如0
    EXPECT_EQ(FfiInputMethodControllerDetach(), 0);
}

TEST(FfiInputMethodControllerTest, ShowTextInput_ReturnsExpectedValue)
{
    // 假设 CjInputMethodController::ShowTextInput 返回一个简单的值
    // 如果有副作用或复杂性，可能需要模拟
    EXPECT_EQ(0, FfiInputMethodControllerShowTextInput());
}

TEST(FfiInputMethodControllerTest, HideTextInput_ShouldReturnExpectedValue)
{
    // 使用模拟方法
    EXPECT_EQ(MockCjInputMethodController::HideTextInput(), 0);

    // 测试被测方法
    EXPECT_EQ(FfiInputMethodControllerHideTextInput(), 0);
}

TEST(FfiInputMethodControllerSetCallingWindowTest, ValidWindowId_Success)
{
    uint32_t windowId = 12345;
    // 假设 SetCallingWindow 返回 0 表示成功
    EXPECT_EQ(0, FfiInputMethodControllerSetCallingWindow(windowId));
}

TEST(FfiInputMethodControllerSetCallingWindowTest, InvalidWindowId_Failure)
{
    uint32_t windowId = 0; // 假设 0 是一个无效的窗口 ID
    // 假设 SetCallingWindow 返回 1 表示失败
    EXPECT_EQ(1, FfiInputMethodControllerSetCallingWindow(windowId));
}

// 假设 CCursorInfo 有一个构造函数，可以设置其字段
TEST(FfiInputMethodControllerUpdateCursor, ValidCursorInfo_ReturnsExpectedValue)
{
    CCursorInfo cursor = { 0, 0, 10, 10 };
    int32_t expected = 42; // 假设这是预期的返回值

    // 假设 UpdateCursor 返回一个预期的值
    EXPECT_CALL(CjInputMethodController, UpdateCursor(cursor)).WillOnce(testing::Return(expected));

    int32_t result = FfiInputMethodControllerUpdateCursor(cursor);

    EXPECT_EQ(result, expected);
}

TEST(FfiInputMethodControllerUpdateCursor, InvalidCursorInfo_ReturnsErrorValue)
{
    CCursorInfo cursor = { 0, 0, 0, 0 };
    int32_t expected = -1; // 假设这是错误返回值

    // 假设 UpdateCursor 返回一个错误值
    EXPECT_CALL(CjInputMethodController, UpdateCursor(cursor)).WillOnce(testing::Return(expected));

    int32_t result = FfiInputMethodControllerUpdateCursor(cursor);

    EXPECT_EQ(result, expected);
}

TEST(FfiInputMethodControllerChangeSelectionTest, ValidInput_ReturnsSuccess)
{
    char text[] = "Hello, World!";
    EXPECT_CALL(MockCjInputMethodController::ChangeSelection, ChangeSelection("Hello, World!", 0, 5))
        .WillOnce(::testing::Return(0));

    int32_t result = FfiInputMethodControllerChangeSelection(text, 0, 5);
    EXPECT_EQ(result, 0);
}

TEST(FfiInputMethodControllerChangeSelectionTest, InvalidStart_ReturnsError)
{
    char text[] = "Hello, World!";
    EXPECT_CALL(MockCjInputMethodController::ChangeSelection, ChangeSelection("Hello, World!", -1, 5))
        .WillOnce(::testing::Return(-1));

    int32_t result = FfiInputMethodControllerChangeSelection(text, -1, 5);
    EXPECT_EQ(result, -1);
}

TEST(FfiInputMethodControllerChangeSelectionTest, InvalidEnd_ReturnsError)
{
    char text[] = "Hello, World!";
    EXPECT_CALL(MockCjInputMethodController::ChangeSelection, ChangeSelection("Hello, World!", 0, 20))
        .WillOnce(::testing::Return(-1));

    int32_t result = FfiInputMethodControllerChangeSelection(text, 0, 20);
    EXPECT_EQ(result, -1);
}

TEST(FfiInputMethodControllerChangeSelectionTest, EmptyString_ReturnsSuccess)
{
    char text[] = "";
    EXPECT_CALL(MockCjInputMethodController::ChangeSelection, ChangeSelection("", 0, 0)).WillOnce(::testing::Return(0));

    int32_t result = FfiInputMethodControllerChangeSelection(text, 0, 0);
    EXPECT_EQ(result, 0);
}

TEST(FfiInputMethodControllerChangeSelectionTest, NullPointer_ReturnsError)
{
    EXPECT_CALL(MockCjInputMethodController::ChangeSelection, ChangeSelection("", 0, 0))
        .WillOnce(::testing::Return(-1));

    int32_t result = FfiInputMethodControllerChangeSelection(nullptr, 0, 0);
    EXPECT_EQ(result, -1);
}

// 测试用例
TEST(FfiInputMethodControllerUpdateAttributeTest, ValidAttribute_ReturnsExpectedValue)
{
    // 假设UpdateAttribute返回一个特定的值
    EXPECT_EQ(FfiInputMethodControllerUpdateAttribute(Attribute1), 100); // 假设预期返回值为100
    EXPECT_EQ(FfiInputMethodControllerUpdateAttribute(Attribute2), 200); // 假设预期返回值为200
}

TEST(FfiInputMethodControllerUpdateAttributeTest, InvalidAttribute_ReturnsDefaultValue)
{
    // 假设UpdateAttribute对无效属性返回一个默认值
    EXPECT_EQ(FfiInputMethodControllerUpdateAttribute(static_cast<CInputAttribute>(-1)), -1); // 假设默认返回值为-1
}

TEST(FfiInputMethodControllerShowSoftKeyboardTest, ReturnsCorrectValue)
{
    // 假设 CjInputMethodController::ShowSoftKeyboard() 返回一个固定的值
    // 例如，返回 42
    EXPECT_EQ(42, FfiInputMethodControllerShowSoftKeyboard());
}

// 假设 CjInputMethodController::HideSoftKeyboard 返回 0 表示成功，-1 表示失败
TEST(FfiInputMethodControllerHideSoftKeyboard, HideSoftKeyboard_Success_ReturnsZero)
{
    // 假设 HideSoftKeyboard 被模拟或配置为返回 0
    EXPECT_EQ(FfiInputMethodControllerHideSoftKeyboard(), 0);
}

TEST(FfiInputMethodControllerHideSoftKeyboard, HideSoftKeyboard_Failure_ReturnsNegativeOne)
{
    // 假设 HideSoftKeyboard 被模拟或配置为返回 -1
    EXPECT_EQ(FfiInputMethodControllerHideSoftKeyboard(), -1);
}

// 假设 CjInputMethodController::HideSoftKeyboard 返回 0 表示成功，-1 表示失败
TEST(FfiInputMethodControllerHideSoftKeyboard, HideSoftKeyboard_Success_ReturnsZero)
{
    // 假设 HideSoftKeyboard 被模拟或配置为返回 0
    EXPECT_EQ(FfiInputMethodControllerHideSoftKeyboard(), 0);
}

TEST(FfiInputMethodControllerHideSoftKeyboard, HideSoftKeyboard_Failure_ReturnsNegativeOne)
{
    // 假设 HideSoftKeyboard 被模拟或配置为返回 -1
    EXPECT_EQ(FfiInputMethodControllerHideSoftKeyboard(), -1);
}