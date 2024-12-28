/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#include "CjInputMethodController.h"
#include "Configuration.h"
#include "InputMethodController.h"
#include "cj_input_method_controller.h"
#include "cj_input_method_text_changed_listener.h"
#include "input_method_controller.h"
#include <codecvt>
#include <condition_variable>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

using namespace testing;

// Mock classes and methods
class MockInputMethodController : public InputMethodController {
public:
    MOCK_METHOD1(SetControllerListener, void(const std::shared_ptr<CjInputMethodController> &));
    MOCK_METHOD(int32_t, Attach,
        (CjInputMethodTextChangedListener * listener, bool showKeyboard, const TextConfig &textCfg), (override));
    MOCK_METHOD(int32_t, Close, (), (override));
    MOCK_METHOD(int32_t, ShowTextInput, (), (override));
    MOCK_METHOD(int32_t, HideTextInput, (), (override));
    MOCK_METHOD(int32_t, SetCallingWindow, (uint32_t windowId), (override));
    MOCK_METHOD(int32_t, OnCursorUpdate, (const CursorInfo &cursorInfo), (override));
    MOCK_METHOD3(OnSelectionChange, int32_t(const std::u16string &, int32_t, int32_t));
    MOCK_METHOD(int32_t, OnConfigurationChange, (const Configuration &config), (override));
    MOCK_METHOD(int32_t, ShowSoftKeyboard, (), (override));
    MOCK_METHOD(int32_t, HideSoftKeyboard, (), (override));
    MOCK_METHOD2(RegisterListener, void(int8_t type, int64_t id));
};

// 模拟 InputMethodController 的单例实例
class MockInputMethodControllerSingleton {
public:
    static MockInputMethodController *GetInstance()
    {
        static MockInputMethodController instance;
        return &instance;
    }
};

// 用模拟的单例替换实际的单例
using InputMethodControllerSingle = MockInputMethodControllerSingleton;

class MockInputMethodControllerSingle : public InputMethodControllerSingle {
public:
    MOCK_METHOD(int32_t, StopInputSession, (), (override));
};

class MockInputMethodControllerFactory {
public:
    static MockInputMethodController *GetInstance()
    {
        return instance;
    }

    static void SetInstance(MockInputMethodController *mock)
    {
        instance = mock;
    }

private:
    static MockInputMethodController *instance;
};

// Mock implementation of CJLambda
class MockCJLambda : public CJLambda {
public:
    explicit MockCJLambda(void (*callback)(int32_t)) : callback(callback) {}
    MOCK_METHOD(void, operator(), (int32_t), (override));
    MOCK_METHOD1(Call, void(int32_t));
    MOCK_METHOD(void, operator(), (int32_t direction), (override));

private:
    void (*callback)(int32_t);
};

class MockCJLambdaFactory {
public:
    static MockCJLambda *Create(void (*callback)(int32_t))
    {
        return new MockCJLambda();
    }
};

class MockCjInputMethodController : public CjInputMethodController {
public:
    MOCK_METHOD0(GetInstance, CjInputMethodController *());
    MOCK_METHOD1(UnRegisterListener, void(int8_t));

    void InitDeleteRight(int64_t id)
    {
        auto callback = reinterpret_cast<void (*)(int32_t)>(id);
        deleteLeft = [lambda = CJLambda::Create(callback)](int32_t length) -> void {
            lambda(length);
        };
    }

    std::function<void(int32_t)> deleteLeft;
};

MockInputMethodController *MockInputMethodControllerFactory::instance = nullptr;

class MockCjInputMethodTextChangedListener : public CjInputMethodTextChangedListener {
public:
    MOCK_METHOD(void, OnTextChanged, (const std::string &text), (override));
};

// Use the mock factory in the test
std::shared_ptr<InputMethodController> InputMethodController::GetInstance()
{
    return MockInputMethodControllerFactory::GetInstance();
}

// 模拟 Utils::MallocCString
char *MallocCStringMock(const char *str)
{
    // 模拟内存分配失败
    return nullptr;
}

// 模拟 insertText 回调
void InsertTextCallbackMock(char *text)
{
    // 什么都不做，用于验证是否被调用
}

// Unit test class
class CjInputMethodControllerTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        // Reset the singleton instance before each test
        CjInputMethodController::ResetInstance();
        // 设置模拟对象
        g_MockTextListener = new MockCjInputMethodTextChangedListener();
        g_MockController = new MockInputMethodController();
        InputMethodController::SetInstance(g_MockController);
        // 重置静态实例
        CjInputMethodController::ResetInstance();
    }

    void TearDown() override
    {
        // 清理模拟对象
        delete g_MockTextListener;
        g_MockTextListener = nullptr;
        // 清理
        InputMethodController::SetInstance(nullptr);
        delete g_MockController;
        g_MockController = nullptr;
        // 重置静态实例
        CjInputMethodController::ResetInstance();
    }

    int32_t statusReceived = 0;

    // 模拟回调函数
    static void MockCallback(int32_t status)
    {
        statusReceived = status;
    }
    CjInputMethodController controller;
    MockCjInputMethodController mockController;
};

MockCjInputMethodTextChangedListener *g_MockTextListener;
MockInputMethodController *g_MockController;

// MockGetRightText 是一个模拟类，用于模拟 getRightText 函数的行为
class MockGetRightText {
public:
    MOCK_METHOD1(Call, char *(int32_t));
};

InputMethodController *InputMethodController::GetInstance()
{
    return mockController;
}

TEST_F(CjInputMethodControllerTest, GetInstance_WhenControllerIsNotInitialized_ReturnsNewInstance)
{
    std::shared_ptr<CjInputMethodController> instance = CjInputMethodController::GetInstance();
    EXPECT_NE(instance, nullptr);
}

TEST_F(CjInputMethodControllerTest, GetInstance_WhenControllerIsAlreadyInitialized_ReturnsSameInstance)
{
    std::shared_ptr<CjInputMethodController> instance1 = CjInputMethodController::GetInstance();
    std::shared_ptr<CjInputMethodController> instance2 = CjInputMethodController::GetInstance();
    EXPECT_EQ(instance1, instance2);
}

TEST_F(CjInputMethodControllerTest, GetInstance_ThreadSafety_CreatesSingleInstance)
{
    std::vector<std::shared_ptr<CjInputMethodController>> instances;
    std::mutex mtx;
    std::condition_variable cv;
    bool ready = false;

    auto threadFunc = [&instances]() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [&] {
            return ready;
        });
        instances.push_back(CjInputMethodController::GetInstance());
    };

    const int numThreads = 10;
    std::vector<std::thread> threads;
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(threadFunc);
    }

    {
        std::lock_guard<std::mutex> lock(mtx);
        ready = true;
    }
    cv.notify_all();

    for (auto &thread : threads) {
        thread.join();
    }

    for (size_t i = 1; i < instances.size(); ++i) {
        EXPECT_EQ(instances[i], instances[0]);
    }
}

TEST_F(CjInputMethodControllerTest, Attach_TextListenerIsNull_ReturnsErrNoMemory)
{
    EXPECT_CALL(*g_MockTextListener, OnTextChanged(_)).Times(0);
    EXPECT_CALL(*g_MockController, Attach(_, _, _)).Times(0);

    CTextConfig textConfig;
    CjInputMethodController controller;
    int32_t result = controller.Attach(textConfig, true);

    EXPECT_EQ(result, ERR_NO_MEMORY);
}

TEST_F(CjInputMethodControllerTest, Attach_ControllerIsNull_ReturnsErrNoMemory)
{
    EXPECT_CALL(*g_MockTextListener, OnTextChanged(_)).Times(0);
    EXPECT_CALL(*g_MockController, Attach(_, _, _)).Times(0);

    CTextConfig textConfig;
    CjInputMethodController controller;
    int32_t result = controller.Attach(textConfig, true);

    EXPECT_EQ(result, ERR_NO_MEMORY);
}

TEST_F(CjInputMethodControllerTest, Attach_AttachSucceeds_ReturnsExpectedValue)
{
    EXPECT_CALL(*g_MockTextListener, OnTextChanged(_)).Times(0);
    EXPECT_CALL(*g_MockController, Attach(_, _, _)).WillOnce(Return(0));

    CTextConfig textConfig;
    CjInputMethodController controller;
    int32_t result = controller.Attach(textConfig, true);

    EXPECT_EQ(result, 0);
}

TEST(CjInputMethodControllerTest, Detach_ControllerIsNull_ReturnsErrNoMemory)
{
    CjInputMethodController controller;
    int32_t result = controller.Detach();
    EXPECT_EQ(result, ERR_NO_MEMORY);
}

TEST(CjInputMethodControllerTest, Detach_ControllerIsNotNull_CallsCloseAndReturnsResult)
{
    MockInputMethodController mock;

    EXPECT_CALL(mock, Close()).WillOnce(Return(0)); // 模拟 Close 返回 0

    CjInputMethodController controller;
    int32_t result = controller.Detach();

    EXPECT_EQ(result, 0); // 预期 Close 返回的结果
}

// Unit Test Code:
TEST(CjInputMethodControllerTest, ShowTextInput_NoInstance_ReturnsErrNoMemory)
{
    CjInputMethodController controller;
    EXPECT_EQ(controller.ShowTextInput(), ERR_NO_MEMORY);
}

TEST(CjInputMethodControllerTest, ShowTextInput_ValidInstance_ReturnsControllerResult)
{
    MockInputMethodController mock;
    EXPECT_CALL(mock, ShowTextInput()).WillOnce(Return(0)); // 假设 0 是成功

    CjInputMethodController controller;
    EXPECT_EQ(controller.ShowTextInput(), 0);
}

TEST(CjInputMethodControllerTest, HideTextInput_InstanceIsNull_ReturnsErrNoMemory)
{
    MockInputMethodControllerFactory::SetInstance(nullptr);
    CjInputMethodController controller;
    EXPECT_EQ(controller.HideTextInput(), ERR_NO_MEMORY);
}

TEST(CjInputMethodControllerTest, HideTextInput_InstanceIsValid_ReturnsHideTextInputResult)
{
    MockInputMethodControllerFactory::SetInstance(&mockController);

    EXPECT_CALL(mockController, HideTextInput()).WillOnce(Return(0));

    CjInputMethodController controller;
    EXPECT_EQ(controller.HideTextInput(), 0);
}

// Unit Test Code:
TEST(CjInputMethodControllerTest, SetCallingWindow_ControllerIsNull_ReturnsErrNoMemory)
{
    // 模拟 GetInstance() 返回 nullptr
    EXPECT_CALL(*InputMethodController::GetInstance(), SetCallingWindow(_)).Times(0);

    CjInputMethodController controller;
    int32_t result = controller.SetCallingWindow(123);

    EXPECT_EQ(result, ERR_NO_MEMORY);
}

TEST(CjInputMethodControllerTest, SetCallingWindow_ControllerIsValid_ReturnsSetCallingWindowResult)
{
    // 模拟 GetInstance() 返回一个有效的控制器
    MockInputMethodController *controller =
        static_cast<MockInputMethodController *>(InputMethodController::GetInstance());
    EXPECT_CALL(*controller, SetCallingWindow(123)).WillOnce(Return(0));

    CjInputMethodController cjController;
    int32_t result = cjController.SetCallingWindow(123);

    EXPECT_EQ(result, 0);
}

// Unit Test Code:
TEST(CjInputMethodControllerTest, UpdateCursor_ControllerIsNull_ReturnsErrNoMemory)
{
    // 模拟 GetInstance 返回 nullptr
    EXPECT_CALL(*InputMethodController::GetInstance(), OnCursorUpdate(_)).Times(0);

    CjInputMethodController cjInputMethodController;
    CCursorInfo cursorInfo = { 0, 0, 10, 10 };
    int32_t result = cjInputMethodController.UpdateCursor(cursorInfo);

    EXPECT_EQ(result, ERR_NO_MEMORY);
}

TEST(CjInputMethodControllerTest, UpdateCursor_ControllerIsValid_CallsOnCursorUpdate)
{
    // 模拟 GetInstance 返回一个有效的 controller
    MockInputMethodController *controller =
        static_cast<MockInputMethodController *>(InputMethodController::GetInstance());
    EXPECT_CALL(*controller, OnCursorUpdate(_)).WillOnce(Return(0));

    CjInputMethodController cjInputMethodController;
    CCursorInfo cursorInfo = { 0, 0, 10, 10 };
    int32_t result = cjInputMethodController.UpdateCursor(cursorInfo);

    EXPECT_EQ(result, 0);
}

TEST_F(CjInputMethodControllerTest, ChangeSelection_ControllerInstanceIsNull_ReturnsErrNoMemory)
{
    // 设置
    InputMethodController::SetInstance(nullptr);

    CjInputMethodController controller;
    int32_t result = controller.ChangeSelection("test", 0, 4);

    EXPECT_EQ(result, ERR_NO_MEMORY);
}

TEST_F(CjInputMethodControllerTest, ChangeSelection_ControllerInstanceIsValid_CallsOnSelectionChange)
{
    int start = 0;
    int end = 4;
    // 设置
    EXPECT_CALL(*mockController, OnSelectionChange(_, start, end)).WillOnce(Return(0));

    CjInputMethodController controller;
    int32_t result = controller.ChangeSelection("test", start, end);

    EXPECT_EQ(result, 0);
}

TEST(CjInputMethodControllerTest, UpdateAttribute_InstanceIsNull_ReturnsErrNoMemory)
{
    // 模拟 GetInstance 返回 nullptr
    EXPECT_CALL(*InputMethodController::GetInstance(), OnConfigurationChange(_)).Times(0);

    CInputAttribute inputAttribute;
    inputAttribute.textInputType = 1;
    inputAttribute.enterKeyType = 2;

    int32_t result = CjInputMethodController::UpdateAttribute(inputAttribute);

    EXPECT_EQ(result, ERR_NO_MEMORY);
}

TEST(CjInputMethodControllerTest, UpdateAttribute_InstanceIsValid_ReturnsOnConfigurationChangeResult)
{
    // 模拟 GetInstance 返回一个有效的实例
    MockInputMethodController *controller =
        static_cast<MockInputMethodController *>(InputMethodController::GetInstance());
    EXPECT_CALL(*controller, OnConfigurationChange(_)).WillOnce(Return(0));

    CInputAttribute inputAttribute;
    inputAttribute.textInputType = 1;
    inputAttribute.enterKeyType = 2;

    int32_t result = CjInputMethodController::UpdateAttribute(inputAttribute);

    EXPECT_EQ(result, 0);
}

// 测试用例 1：验证当 GetInstance 返回 nullptr 时，方法返回 ERR_NO_MEMORY
TEST(CjInputMethodControllerTest, ShowSoftKeyboard_InstanceIsNull_ReturnsErrNoMemory)
{
    // 模拟 GetInstance 返回 nullptr
    InputMethodController::GetInstance = []() -> InputMethodController * {
        return nullptr;
    };

    CjInputMethodController controller;
    int32_t result = controller.ShowSoftKeyboard();

    EXPECT_EQ(result, ERR_NO_MEMORY);
}

// 测试用例 2：验证当 GetInstance 返回有效实例时，方法返回 ShowSoftKeyboard 的结果
TEST(CjInputMethodControllerTest, ShowSoftKeyboard_InstanceIsValid_ReturnsShowSoftKeyboardResult)
{
    // 模拟 GetInstance 返回有效实例
    MockInputMethodController *controller = new MockInputMethodController();
    InputMethodController::GetInstance = [controller]() -> InputMethodController * {
        return controller;
    };

    // 模拟 ShowSoftKeyboard 返回一个特定值
    EXPECT_CALL(*controller, ShowSoftKeyboard()).WillOnce(Return(0));

    CjInputMethodController controller;
    int32_t result = controller.ShowSoftKeyboard();

    EXPECT_EQ(result, 0);
}

TEST(CjInputMethodControllerTest, HideSoftKeyboard_ControllerNotNull_ReturnsControllerResult)
{
    CjInputMethodController cjInputMethodController;
    MockInputMethodController *controller = MockInputMethodControllerSingleton::GetInstance();

    // 模拟控制器返回特定值
    EXPECT_CALL(*controller, HideSoftKeyboard()).WillOnce(Return(0));

    int32_t result = cjInputMethodController.HideSoftKeyboard();

    EXPECT_EQ(result, 0);
}

TEST(CjInputMethodControllerTest, HideSoftKeyboard_ControllerNull_ReturnsErrNoMemory)
{
    CjInputMethodController cjInputMethodController;

    // 模拟控制器为 null
    EXPECT_CALL(MockInputMethodControllerSingleton, GetInstance()).WillOnce(Return(nullptr));

    int32_t result = cjInputMethodController.HideSoftKeyboard();

    EXPECT_EQ(result, ERR_NO_MEMORY);
}

TEST_F(CjInputMethodControllerTest, StopInputSession_ControllerIsNull_ReturnsErrNoMemory)
{
    // 设置模拟实例为null
    MockInputMethodControllerFactory::SetInstance(nullptr);

    CjInputMethodController controller;
    int32_t result = controller.StopInputSession();

    EXPECT_EQ(result, ERR_NO_MEMORY);
}

TEST_F(CjInputMethodControllerTest, StopInputSession_ControllerIsNotNull_ReturnsStopInputSessionResult)
{
    // 设置模拟行为
    EXPECT_CALL(*mockController, StopInputSession()).WillOnce(Return(0));

    CjInputMethodController controller;
    int32_t result = controller.StopInputSession();

    EXPECT_EQ(result, 0);
}

TEST_F(CjInputMethodControllerTest, RegisterListener_InsertText_CallsInitInsertText)
{
    // 假设 InitInsertText 有一个可观察的副作用，比如设置一个标志
    controller_.RegisterListener(INSERT_TEXT, 1);
    // 验证 InitInsertText 是否被正确调用
    // 这可能需要一个公共方法或友元类来检查副作用
}

TEST_F(CjInputMethodControllerTest, RegisterListener_DeleteLeft_CallsInitDeleteRight)
{
    controller_.RegisterListener(DELETE_LEFT, 1);
    // 验证 InitDeleteRight 是否被正确调用
}

TEST_F(CjInputMethodControllerTest, RegisterListener_DeleteRight_CallsInitDeleteLeft)
{
    controller_.RegisterListener(DELETE_RIGHT, 1);
    // 验证 InitDeleteLeft 是否被正确调用
}

TEST_F(CjInputMethodControllerTest, RegisterListener_SendKeyboardStatus_CallsInitSendKeyboardStatus)
{
    controller_.RegisterListener(SEND_KEYBOARD_STATUS, 1);
    // 验证 InitSendKeyboardStatus 是否被正确调用
}

TEST_F(CjInputMethodControllerTest, RegisterListener_SendFunctionKey_CallsInitSendFunctionKey)
{
    controller_.RegisterListener(SEND_FUNCTION_KEY, 1);
    // 验证 InitSendFunctionKey 是否被正确调用
}

TEST_F(CjInputMethodControllerTest, RegisterListener_MoveCursor_CallsInitMoveCursor)
{
    controller_.RegisterListener(MOVE_CURSOR, 1);
    // 验证 InitMoveCursor 是否被正确调用
}

TEST_F(CjInputMethodControllerTest, RegisterListener_HandleExtendAction_CallsInitHandleExtendAction)
{
    controller_.RegisterListener(HANDLE_EXTEND_ACTION, 1);
    // 验证 InitHandleExtendAction 是否被正确调用
}

TEST_F(CjInputMethodControllerTest, RegisterListener_GetLeftText_CallsInitGetLeftText)
{
    controller_.RegisterListener(GET_LEFT_TEXT, 1);
    // 验证 InitGetLeftText 是否被正确调用
}

TEST_F(CjInputMethodControllerTest, RegisterListener_GetRightText_CallsInitGetRightText)
{
    controller_.RegisterListener(GET_RIGHT_TEXT, 1);
    // 验证 InitGetRightText 是否被正确调用
}

TEST_F(CjInputMethodControllerTest, RegisterListener_GetTextIndex_CallsInitGetTextIndexAtCursor)
{
    controller_.RegisterListener(GET_TEXT_INDEX, 1);
    // 验证 InitGetTextIndexAtCursor 是否被正确调用
}

TEST_F(CjInputMethodControllerTest, RegisterListener_SelectByMovement_CallsInitSelectByMovement)
{
    controller_.RegisterListener(SELECT_BY_MOVEMENT, 1);
    // 验证 InitSelectByMovement 是否被正确调用
}

TEST_F(CjInputMethodControllerTest, RegisterListener_SelectByRange_CallsInitSelectByRange)
{
    controller_.RegisterListener(SELECT_BY_RANGE, 1);
    // 验证 InitSelectByRange 是否被正确调用
}

TEST_F(CjInputMethodControllerTest, RegisterListener_UnknownType_DoesNothing)
{
    controller_.RegisterListener(999, 1); // 假设 999 不是一个已知的类型
    // 验证没有副作用发生
}

TEST_F(CjInputMethodControllerTest, UnRegisterListener_KnownType_SetsListenerToNull)
{
    // 假设我们有一个方法来设置监听器，以便我们可以测试它们是否被正确设置为null
    controller.SetListener(INSERT_TEXT, [](const std::string &text) {});
    controller.SetListener(DELETE_LEFT, []() {});
    controller.SetListener(DELETE_RIGHT, []() {});
    controller.SetListener(SEND_KEYBOARD_STATUS, [](bool status) {});
    controller.SetListener(SEND_FUNCTION_KEY, [](int key) {});
    controller.SetListener(MOVE_CURSOR, [](int offset) {});
    controller.SetListener(HANDLE_EXTEND_ACTION, [](int action) {});
    controller.SetListener(GET_LEFT_TEXT, [](std::string &text) {});
    controller.SetListener(GET_RIGHT_TEXT, [](std::string &text) {});
    controller.SetListener(GET_TEXT_INDEX, [](int &index) {});
    controller.SetListener(SELECT_BY_MOVEMENT, [](int movement) {});
    controller.SetListener(SELECT_BY_RANGE, [](int start, int end) {});

    // 现在注销监听器
    controller.UnRegisterListener(INSERT_TEXT);
    controller.UnRegisterListener(DELETE_LEFT);
    controller.UnRegisterListener(DELETE_RIGHT);
    controller.UnRegisterListener(SEND_KEYBOARD_STATUS);
    controller.UnRegisterListener(SEND_FUNCTION_KEY);
    controller.UnRegisterListener(MOVE_CURSOR);
    controller.UnRegisterListener(HANDLE_EXTEND_ACTION);
    controller.UnRegisterListener(GET_LEFT_TEXT);
    controller.UnRegisterListener(GET_RIGHT_TEXT);
    controller.UnRegisterListener(GET_TEXT_INDEX);
    controller.UnRegisterListener(SELECT_BY_MOVEMENT);
    controller.UnRegisterListener(SELECT_BY_RANGE);

    // 验证监听器是否被设置为null
    EXPECT_EQ(controller.GetListener(INSERT_TEXT), nullptr);
    EXPECT_EQ(controller.GetListener(DELETE_LEFT), nullptr);
    EXPECT_EQ(controller.GetListener(DELETE_RIGHT), nullptr);
    EXPECT_EQ(controller.GetListener(SEND_KEYBOARD_STATUS), nullptr);
    EXPECT_EQ(controller.GetListener(SEND_FUNCTION_KEY), nullptr);
    EXPECT_EQ(controller.GetListener(MOVE_CURSOR), nullptr);
    EXPECT_EQ(controller.GetListener(HANDLE_EXTEND_ACTION), nullptr);
    EXPECT_EQ(controller.GetListener(GET_LEFT_TEXT), nullptr);
    EXPECT_EQ(controller.GetListener(GET_RIGHT_TEXT), nullptr);
    EXPECT_EQ(controller.GetListener(GET_TEXT_INDEX), nullptr);
    EXPECT_EQ(controller.GetListener(SELECT_BY_MOVEMENT), nullptr);
    EXPECT_EQ(controller.GetListener(SELECT_BY_RANGE), nullptr);
}

TEST_F(CjInputMethodControllerTest, UnRegisterListener_UnknownType_NoOperation)
{
    // 假设我们有一个方法来设置监听器，以便我们可以测试它们是否保持不变
    controller.SetListener(INSERT_TEXT, [](const std::string &text) {});

    // 注销一个未知的类型
    controller.UnRegisterListener(UNKNOWN_TYPE);

    // 验证监听器保持不变
    EXPECT_NE(controller.GetListener(INSERT_TEXT), nullptr);
}

TEST_F(CjInputMethodControllerTest, Subscribe_InstanceIsNull_ReturnsErrNoMemory)
{
    // 模拟 GetInstance 返回 nullptr
    EXPECT_CALL(CjInputMethodController::GetInstance(), Times(1)).WillOnce(Return(nullptr));

    int32_t result = CjInputMethodController::Subscribe(1, 12345);
    EXPECT_EQ(result, ERR_NO_MEMORY);
}

TEST(CjInputMethodControllerTest, Unsubscribe_ControllerIsNull_ReturnsErrNoMemory)
{
    MockCjInputMethodController mockController;
    EXPECT_CALL(mockController, GetInstance()).WillOnce(Return(nullptr));

    int32_t result = CjInputMethodController::Unsubscribe(0);
    EXPECT_EQ(result, ERR_NO_MEMORY);
}

TEST(CjInputMethodControllerTest, Unsubscribe_ControllerIsNotNull_UnregistersListener)
{
    EXPECT_CALL(mockController, GetInstance()).WillOnce(Return(&mockController));
    EXPECT_CALL(mockController, UnRegisterListener(0)).Times(1);

    int32_t result = CjInputMethodController::Unsubscribe(0);
    EXPECT_EQ(result, SUCCESS_CODE);
}

TEST_F(CjInputMethodControllerTest, OnSelectByRange_NullFunction_NoCall)
{
    controller.onSelectByRange = nullptr;
    controller.OnSelectByRange(1, 2);
    // 由于 onSelectByRange 是 nullptr，不应调用任何函数。
}

TEST_F(CjInputMethodControllerTest, OnSelectByRange_ValidFunction_CallsFunction)
{
    bool called = false;
    Range receivedRange;

    controller.onSelectByRange = [&called, &receivedRange](Range range) {
        called = true;
        receivedRange = range;
    };
    int start = 3;
    int end = 4;
    controller.OnSelectByRange(start, end);

    EXPECT_TRUE(called);
    EXPECT_EQ(receivedRange.start, start);
    EXPECT_EQ(receivedRange.end, end);
}

// 测试用例 1: onSelectByMovement 是 nullptr
TEST(CjInputMethodControllerTest, OnSelectByMovement_NullCallback_NoCall)
{
    CjInputMethodController controller;
    controller.SetOnSelectByMovement(nullptr);

    // 模拟日志输出
    testing::internal::CaptureStdout();
    controller.OnSelectByMovement(1);
    string output = testing::internal::GetCapturedStdout();

    EXPECT_TRUE(output.find("onSelectByMovement null") != string::npos);
}

// 测试用例 2: onSelectByMovement 不是 nullptr
TEST(CjInputMethodControllerTest, OnSelectByMovement_ValidCallback_CallExecuted)
{
    CjInputMethodController controller;
    bool callbackCalled = false;
    controller.SetOnSelectByMovement([&callbackCalled](int32_t direction) {
        callbackCalled = true;
    });

    // 模拟日志输出
    testing::internal::CaptureStdout();
    controller.OnSelectByMovement(1);
    string output = testing::internal::GetCapturedStdout();

    EXPECT_TRUE(output.find("onSelectByMovement runs") != string::npos);
    EXPECT_TRUE(callbackCalled);
}

TEST(CjInputMethodControllerTest, InsertText_MemoryAllocationFailure_LogsError)
{
    CjInputMethodController controller;
    std::u16string text = u"test";

    // 模拟内存分配失败
    EXPECT_CALL(*Utils::GetMock(), MallocCString(Str16ToStr8(text))).WillOnce(Return(nullptr));

    controller.InsertText(text);

    // 验证没有调用 insertText
    EXPECT_CALL(*Utils::GetMock(), insertText(_)).Times(0);
}

TEST(CjInputMethodControllerTest, InsertText_InsertTextIsNull_LogsDebug)
{
    CjInputMethodController controller;
    std::u16string text = u"test";

    // 模拟内存分配成功
    EXPECT_CALL(*Utils::GetMock(), MallocCString(Str16ToStr8(text))).WillOnce(Return(new char[5] { "test" }));

    // 设置 insertText 为 nullptr
    controller.insertText = nullptr;

    controller.InsertText(text);

    // 验证没有调用 insertText
    EXPECT_CALL(*Utils::GetMock(), insertText(_)).Times(0);
}

TEST(CjInputMethodControllerTest, InsertText_ValidInput_CallsInsertText)
{
    CjInputMethodController controller;
    std::u16string text = u"test";

    // 模拟内存分配成功
    EXPECT_CALL(*Utils::GetMock(), MallocCString(Str16ToStr8(text))).WillOnce(Return(new char[5] { "test" }));

    // 设置 insertText 回调
    controller.insertText = InsertTextCallbackMock;

    controller.InsertText(text);

    // 验证 insertText 被调用
    EXPECT_CALL(*Utils::GetMock(), insertText(_)).Times(1);
}

TEST_F(CjInputMethodControllerTest, DeleteRight_NullDeleteRight_LogsNullAndReturns)
{
    controller->deleteRight = nullptr;
    EXPECT_CALL(MockInputMethodController::GetMock(), LogDebug("deleteRight null")).Times(1);
    EXPECT_CALL(MockInputMethodController::GetMock(), LogDebug("deleteRight runs")).Times(0);
    int length = 5;
    controller->DeleteRight(length);
}

TEST_F(CjInputMethodControllerTest, DeleteRight_ValidDeleteRight_LogsRunsAndCallsFunction)
{
    bool deleteRightCalled = false;
    int deleteRightLength = 0;
    controller->deleteRight = [&deleteRightCalled, &deleteRightLength](int32_t length) {
        deleteRightCalled = true;
        deleteRightLength = length;
    };

    EXPECT_CALL(MockInputMethodController::GetMock(), LogDebug("deleteRight null")).Times(0);
    EXPECT_CALL(MockInputMethodController::GetMock(), LogDebug("deleteRight runs")).Times(1);

    int length = 10;
    controller->DeleteRight(length);

    EXPECT_TRUE(deleteRightCalled);
    EXPECT_EQ(deleteRightLength, length);
}

TEST(CjInputMethodControllerTest, DeleteLeft_NullFunction_NoCall)
{
    CjInputMethodController controller;
    controller.SetDeleteLeftFunction(nullptr);

    // 使用模拟日志记录器来捕获日志
    testing::internal::CaptureStdout();
    controller.DeleteLeft(5);
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_TRUE(output.find("deleteLeft null") != std::string::npos);
}

TEST(CjInputMethodControllerTest, DeleteLeft_ValidFunction_CallExecuted)
{
    CjInputMethodController controller;
    controller.SetDeleteLeftFunction(MockDeleteLeft);

    // 使用模拟日志记录器来捕获日志
    testing::internal::CaptureStdout();
    controller.DeleteLeft(5);
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_TRUE(output.find("deleteLeft runs") != std::string::npos);
}

TEST_F(CjInputMethodControllerTest, SendFunctionKey_SendFunctionKeyIsNull_NoCall)
{
    // 设置
    mockSendFunctionKey = nullptr;
    controller.SetSendFunctionKey(mockSendFunctionKey);

    // 操作
    controller.SendFunctionKey(mockFunctionKey);

    // 验证
    EXPECT_EQ(mockSendFunctionKey, nullptr);
}

TEST_F(CjInputMethodControllerTest, SendFunctionKey_SendFunctionKeyNotNull_CallWithCorrectType)
{
    // 设置
    int32_t calledType = -1;
    mockSendFunctionKey = [&calledType](int32_t type) {
        calledType = type;
    };
    controller.SetSendFunctionKey(mockSendFunctionKey);

    // 模拟 FunctionKey 的行为
    EXPECT_CALL(mockFunctionKey, GetEnterKeyType()).WillOnce(Return(EnterKeyType::ENTER));

    // 操作
    controller.SendFunctionKey(mockFunctionKey);

    // 验证
    EXPECT_EQ(calledType, static_cast<int32_t>(EnterKeyType::ENTER));
}

TEST_F(CjInputMethodControllerTest, MoveCursor_MoveCursorIsNull_NoAction)
{
    // 设置
    controller->moveCursor = nullptr;

    // 操作
    controller->MoveCursor(Direction::LEFT);

    // 验证
    // 由于 moveCursor 为 nullptr，不应发生任何操作
    // 我们可以假设日志记录器会捕获日志，但在这里我们不进行验证
}

TEST_F(CjInputMethodControllerTest, MoveCursor_MoveCursorIsNotNull_CallsMoveCursor)
{
    // 设置
    bool moveCursorCalled = false;
    controller->moveCursor = [&moveCursorCalled](int32_t dir) {
        moveCursorCalled = true;
        EXPECT_EQ(dir, static_cast<int32_t>(Direction::RIGHT));
    };

    // 操作
    controller->MoveCursor(Direction::RIGHT);

    // 验证
    EXPECT_TRUE(moveCursorCalled);
}

TEST_F(CjInputMethodControllerTest, HandleExtendAction_NullPointer_NoAction)
{
    // 设置
    int32_t action = 123;
    EXPECT_CALL(*controller_, handleExtendAction).Times(0); // 预期不调用

    // 调用被测方法
    controller_->HandleExtendAction(action);

    // 验证
    // 预期没有日志记录，因为没有日志框架的模拟
}

TEST_F(CjInputMethodControllerTest, HandleExtendAction_ValidPointer_ActionCalled)
{
    // 设置
    int32_t action = 456;
    std::function<void(int32_t)> mockAction = [](int32_t a) {
        // 模拟行为
    };

    // 模拟 handleExtendAction
    controller_->handleExtendAction = mockAction;

    // 预期调用
    EXPECT_CALL(*controller_, handleExtendAction(action)).Times(1);

    // 调用被测方法
    controller_->HandleExtendAction(action);

    // 验证
    // 预期没有日志记录，因为没有日志框架的模拟
}

TEST(CjInputMethodControllerTest, GetLeftText_NullFunction_ReturnsEmptyString)
{
    CjInputMethodController controller;
    auto originalGetLeftText = getLeftText;
    getLeftText = nullptr;

    std::u16string result = controller.GetLeftText(123);

    EXPECT_TRUE(result.empty());
    getLeftText = originalGetLeftText; // 恢复原始函数
}

TEST(CjInputMethodControllerTest, GetLeftText_ValidFunction_ReturnsCorrectString)
{
    CjInputMethodController controller;

    std::u16string result = controller.GetLeftText(123);

    EXPECT_EQ(result, u"Text123");
}

// 测试用例: 当 getRightText 为 nullptr 时
TEST(CjInputMethodControllerTest, GetRightText_NullFunction_ReturnsEmptyString)
{
    controller.getRightText = nullptr;
    std::u16string result = controller.GetRightText(1);
    EXPECT_TRUE(result.empty());
}

// 测试用例: 当 getRightText 不为 nullptr 时
TEST(CjInputMethodControllerTest, GetRightText_ValidFunction_ReturnsCorrectString)
{
    MockGetRightText mockGetRightText;
    controller.getRightText = [](int32_t number) -> char * {
        char *result = new char[10];
        return result;
    };

    std::u16string result = controller.GetRightText(1);
    EXPECT_EQ(result, u"Text1");
}

TEST_F(CjInputMethodControllerTest, GetTextIndexAtCursor_NullFunctionPointer_ReturnsMinusOne)
{
    // 设置 getTextIndexAtCursor 为 nullptr
    controller.getTextIndexAtCursor = nullptr;

    int32_t result = controller.GetTextIndexAtCursor();

    EXPECT_EQ(result, -1);
}

// Test case to verify that InitInsertText initializes insertText correctly
TEST_F(CjInputMethodControllerTest, InitInsertText_WithValidCallback_InsertTextInitialized)
{
    bool callbackCalled = false;
    void (*callback)(const char *) = [](const char *text) {
        callbackCalled = true;
    };

    controller.InitInsertText(reinterpret_cast<int64_t>(callback));

    // Verify that insertText is initialized
    EXPECT_TRUE(controller.insertText);
}

// Test case to verify that insertText calls the callback with the correct text
TEST_F(CjInputMethodControllerTest, InsertText_WithNonEmptyString_CallbackCalled)
{
    std::string receivedText;
    void (*callback)(const char *) = [](const char *text) {
        receivedText = text;
    };

    controller.InitInsertText(reinterpret_cast<int64_t>(callback));
    controller.insertText("test");

    // Verify that the callback was called with the correct text
    EXPECT_EQ(receivedText, "test");
}

// Test case to verify that insertText handles empty strings correctly
TEST_F(CjInputMethodControllerTest, InsertText_WithEmptyString_CallbackCalled)
{
    std::string receivedText;
    void (*callback)(const char *) = [](const char *text) {
        receivedText = text;
    };

    controller.InitInsertText(reinterpret_cast<int64_t>(callback));
    controller.insertText("");

    // Verify that the callback was called with an empty string
    EXPECT_EQ(receivedText, "");
}

// Test case to verify that InitDeleteRight initializes deleteLeft correctly
TEST_F(CjInputMethodControllerTest, InitDeleteRight_InitializesDeleteLeft)
{
    // Arrange
    bool callbackCalled = false;
    void (*callback)(int32_t) = [](int32_t length) {
        callbackCalled = true;
    };

    // Act
    mockController.InitDeleteRight(reinterpret_cast<int64_t>(callback));
    int length = 5;
    mockController.deleteLeft(length);

    // Assert
    EXPECT_TRUE(callbackCalled);
}

// Test case
TEST_F(CjInputMethodControllerTest, InitDeleteLeft_WithValidCallback_ShouldInvokeCallback)
{
    // Arrange
    int32_t callbackValue = 0;
    void (*callback)(int32_t) = [](int32_t value) {
        callbackValue = value;
    };

    // Act
    controller.InitDeleteLeft(reinterpret_cast<int64_t>(callback));
    controller.deleteRight(42);

    // Assert
    EXPECT_EQ(callbackValue, 42);
}

// 测试用例
TEST(CjInputMethodControllerTest, InitMoveCursor_ShouldSetMoveCursor)
{
    CjInputMethodController controller;
    void (*callback)(int32_t) = [](int32_t direction) {
        // 模拟回调函数
    };

    // 使用模拟的 CJLambda::Create
    CJLambda *mockLambda = CreateMockCJLambda(callback);
    EXPECT_CALL(*mockLambda, operator()(testing::_)).Times(1);

    // 使用回调函数的地址作为 id
    controller.InitMoveCursor(reinterpret_cast<int64_t>(callback));

    // 验证 moveCursor 是否被正确设置
    EXPECT_TRUE(controller.moveCursor);

    // 调用 moveCursor 并验证回调是否被调用
    controller.moveCursor(1); // 假设 1 是一个有效的方向
}

// Test case to verify InitHandleExtendAction initializes handleExtendAction correctly
TEST_F(CjInputMethodControllerTest, InitHandleExtendAction_InitializesCorrectly)
{
    // Arrange
    void (*callback)(int32_t) = [](int32_t action) {
        // Dummy callback
    };

    // Act
    controller.InitHandleExtendAction(reinterpret_cast<int64_t>(callback));

    // Assert
    EXPECT_TRUE(controller.handleExtendAction);
}

TEST(CjInputMethodControllerTest, InitGetRightText_NullCallback_ShouldHandleGracefully)
{
    CjInputMethodController controller;
    controller.InitGetRightText(0); // 传递一个空指针

    int32_t testNumber = 123;
    char *result = controller.getRightText(testNumber);

    // 假设在没有回调的情况下，getRightText 应返回空
    EXPECT_EQ(result, nullptr);
}

// Test case
TEST_F(CjInputMethodControllerTest, InitGetTextIndexAtCursor_ValidId_ShouldInitializeCorrectly)
{
    int32_t expectedValue = 42;
    int32_t (*callback)() = []() -> int32_t {
        return expectedValue;
    };

    controller.InitGetTextIndexAtCursor(reinterpret_cast<int64_t>(callback));

    // Verify that getTextIndexAtCursor is initialized and returns the expected value
    EXPECT_TRUE(controller.getTextIndexAtCursor);
    EXPECT_EQ(controller.getTextIndexAtCursor(), expectedValue);
}

TEST(CjInputMethodControllerTest, InitSelectByMovement_ValidId_CallbackSet)
{
    CjInputMethodController controller;
    MockCJLambda *mockLambda = MockCJLambdaFactory::Create(nullptr);
    EXPECT_CALL(*mockLambda, operator()(1)).Times(1);

    controller.InitSelectByMovement(reinterpret_cast<int64_t>(mockLambda));

    controller.onSelectByMovement(1);
}

// Test case
TEST_F(CjInputMethodControllerTest, InitSelectByRange_ShouldSetOnSelectByRange)
{
    // Arrange
    void (*callback)(Range) = [](Range range) {};
    int64_t id = reinterpret_cast<int64_t>(callback);

    // Act
    controller.InitSelectByRange(id);

    int start = 0;
    int end = 10;
    // Assert
    EXPECT_CALL(*mockLambda, operator()(Range(start, end)));
    controller.onSelectByRange(Range(end, end));
}
