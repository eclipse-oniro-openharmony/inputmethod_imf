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
#include "input_data_channel_proxy_wrap.h"
#include "input_method_ability.h"
#include "task_manager.h"
#undef private

#include <gtest/gtest.h>

#include "ability_manager_client.h"
#include "global.h"
#include "ime_event_monitor_manager_impl.h"
#include "ime_setting_listener_test_impl.h"
#include "input_method_ability_interface.h"
#include "input_method_controller.h"
#include "input_method_engine_listener_impl.h"
#include "input_method_types.h"
#include "keyboard_listener_test_impl.h"
#include "scope_utils.h"
#include "tdd_util.h"
#include "text_listener.h"
#include "variant_util.h"
using namespace testing::ext;
namespace OHOS {
namespace MiscServices {
class ImaTextEditTest : public testing::Test {
public:
    static constexpr const char *NORMAL_EDITOR_BOX_BUNDLE_NAME = "com.example.editorbox";
    static const std::string INSERT_TEXT;
    static constexpr int32_t GET_LENGTH = 2;
    static constexpr int32_t DEL_LENGTH = 1;
    static constexpr int32_t DIRECTION = static_cast<int32_t>(Direction::LEFT); // 左移
    static constexpr int32_t LEFT_INDEX = 1;
    static constexpr int32_t RIGHT_INDEX = 3;
    static constexpr int32_t MAX_WAIT_TIME = 1;
    static void SetUpTestCase(void)
    {
        std::shared_ptr<Property> property = InputMethodController::GetInstance()->GetCurrentInputMethod();
        std::string bundleName = property != nullptr ? property->name : "default.inputmethod.unittest";
        auto currentImeTokenId = TddUtil::GetTestTokenID(bundleName);
        {
            TokenScope scope(currentImeTokenId);
            InputMethodAbility::GetInstance().SetCoreAndAgent();
        }
        InputMethodAbility::GetInstance().SetImeListener(std::make_shared<InputMethodEngineListenerImpl>());
        InputMethodAbility::GetInstance().SetKdListener(std::make_shared<KeyboardListenerTestImpl>());
        TddUtil::StartApp(NORMAL_EDITOR_BOX_BUNDLE_NAME);
        TddUtil::ClickApp();
        EXPECT_TRUE(InputMethodEngineListenerImpl::WaitInputStart());
        EXPECT_TRUE(TddUtil::WaitTaskEmpty());
    }
    static void TearDownTestCase(void)
    {
    }
    void SetUp()
    {
        IMSA_HILOGI("ImaTextEditTest::SetUp");
    }
    void TearDown()
    {
        IMSA_HILOGI("ImaTextEditTest::TearDown");
        KeyboardListenerTestImpl::ResetParam();
        auto ret = InputMethodAbility::GetInstance().DeleteForward(finalText_.size());
        if (!finalText_.empty() && ret == ErrorCode::NO_ERROR) {
            EXPECT_TRUE(KeyboardListenerTestImpl::WaitTextChange(""));
        }
        finalText_.clear();
        ResetParams();
        InputMethodEngineListenerImpl::ResetParam();
        KeyboardListenerTestImpl::ResetParam();
    }

    static void ResetParams()
    {
        dealRet_ = ErrorCode::ERROR_CLIENT_NOT_BOUND;
        getForwardRspNums_ = 0;
        getForwardText_ = "";
        getIndex_ = 0;
    }

    static void CommonRsp(int32_t ret, const ResponseData &data)
    {
        std::lock_guard<std::mutex> lock(retCvLock_);
        dealRet_ = ret;
        retCv_.notify_one();
    }

    static bool WaitCommonRsp()
    {
        std::unique_lock<std::mutex> lock(retCvLock_);
        retCv_.wait_for(lock, std::chrono::seconds(MAX_WAIT_TIME), []() { return dealRet_ == ErrorCode::NO_ERROR; });
        return dealRet_ == ErrorCode::NO_ERROR;
    }

    static void GetForwardRsp(int32_t ret, const ResponseData &data)
    {
        std::lock_guard<std::mutex> lock(retCvLock_);
        getForwardRspNums_++;
        dealRet_ = ret;
        VariantUtil::GetValue(data, getForwardText_);
        retCv_.notify_one();
    }

    static bool WaitGetForwardRspAbnormal(int32_t num)
    {
        std::unique_lock<std::mutex> lock(retCvLock_);
        retCv_.wait_for(lock, std::chrono::seconds(MAX_WAIT_TIME),
            [&num]() { return getForwardRspNums_ == num && dealRet_ == ErrorCode::ERROR_IMA_DATA_CHANNEL_ABNORMAL; });
        return getForwardRspNums_ == num && dealRet_ == ErrorCode::ERROR_IMA_DATA_CHANNEL_ABNORMAL;
    }

    static bool WaitGetForwardRsp(const std::string &text)
    {
        std::unique_lock<std::mutex> lock(retCvLock_);
        retCv_.wait_for(lock, std::chrono::seconds(MAX_WAIT_TIME),
            [&]() { return dealRet_ == ErrorCode::NO_ERROR && text == getForwardText_; });
        return dealRet_ == ErrorCode::NO_ERROR;
    }

    static void GetTextIndexAtCursorRsp(int32_t ret, const ResponseData &data)
    {
        std::lock_guard<std::mutex> lock(retCvLock_);
        getForwardRspNums_++;
        dealRet_ = ret;
        VariantUtil::GetValue(data, getIndex_);
        retCv_.notify_one();
    }

    static bool WaitGetTextIndexAtCursorRsp(int32_t index)
    {
        std::unique_lock<std::mutex> lock(retCvLock_);
        retCv_.wait_for(lock, std::chrono::seconds(MAX_WAIT_TIME),
            [&]() { return dealRet_ == ErrorCode::NO_ERROR && index == getIndex_; });
        return dealRet_ == ErrorCode::NO_ERROR;
    }

    static std::mutex retCvLock_;
    static std::condition_variable retCv_;
    static int32_t dealRet_;
    static int32_t getForwardRspNums_;
    static std::string getForwardText_;
    static int32_t getIndex_;
    static std::string finalText_;
};

std::mutex ImaTextEditTest::retCvLock_;
std::condition_variable ImaTextEditTest::retCv_;
int32_t ImaTextEditTest::dealRet_{ ErrorCode::ERROR_CLIENT_NOT_BOUND };
const std::string ImaTextEditTest::INSERT_TEXT = "ABCDEFGHIJKMN";
std::string ImaTextEditTest::finalText_;
int32_t ImaTextEditTest::getForwardRspNums_{ 0 };
std::string ImaTextEditTest::getForwardText_;
int32_t ImaTextEditTest::getIndex_{ 0 };

/**
 * @tc.name: ImaTextEditTest_GetForward
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ImaTextEditTest, ImaTextEditTest_GetForward, TestSize.Level0)
{
    IMSA_HILOGI("ImeProxyTest::ImaTextEditTest_GetForward");
    finalText_ = INSERT_TEXT;
    auto ret = InputMethodAbility::GetInstance().InsertText(INSERT_TEXT);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(KeyboardListenerTestImpl::WaitTextChange(INSERT_TEXT));

    auto expectText = INSERT_TEXT.substr(INSERT_TEXT.size() - GET_LENGTH);
    std::u16string syncText;
    // sync
    ret = InputMethodAbility::GetInstance().GetTextBeforeCursor(GET_LENGTH, syncText);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(Str16ToStr8(syncText), expectText);

    // async
    std::u16string asyncText;
    ret = InputMethodAbility::GetInstance().GetTextBeforeCursor(GET_LENGTH, asyncText, GetForwardRsp);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(WaitGetForwardRsp(expectText));
}

/**
 * @tc.name: ImaTextEditTest_GetBackward
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ImaTextEditTest, ImaTextEditTest_GetBackward, TestSize.Level0)
{
    IMSA_HILOGI("ImeProxyTest::ImaTextEditTest_GetBackward");
    finalText_ = INSERT_TEXT;
    auto ret = InputMethodAbility::GetInstance().InsertText(INSERT_TEXT);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(KeyboardListenerTestImpl::WaitTextChange(INSERT_TEXT));

    std::string expectText;
    std::u16string syncText;
    // sync
    ret = InputMethodAbility::GetInstance().GetTextAfterCursor(GET_LENGTH, syncText);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(Str16ToStr8(syncText), expectText);

    // async
    std::u16string asyncText;
    ret = InputMethodAbility::GetInstance().GetTextAfterCursor(GET_LENGTH, asyncText, GetForwardRsp);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(WaitGetForwardRsp(expectText));
}

/**
 * @tc.name: ImaTextEditTest_GetTextIndexAtCursor
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ImaTextEditTest, ImaTextEditTest_GetTextIndexAtCursor, TestSize.Level0)
{
    IMSA_HILOGI("ImeProxyTest::ImaTextEditTest_GetTextIndexAtCursor");
    finalText_ = INSERT_TEXT;
    auto ret = InputMethodAbility::GetInstance().InsertText(INSERT_TEXT);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(KeyboardListenerTestImpl::WaitTextChange(INSERT_TEXT));

    int32_t syncIndex = 0;
    int32_t expectIndex = INSERT_TEXT.size();
    // sync
    ret = InputMethodAbility::GetInstance().GetTextIndexAtCursor(syncIndex);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(syncIndex, expectIndex);

    // async
    int32_t asyncIndex = 0;
    ret = InputMethodAbility::GetInstance().GetTextIndexAtCursor(asyncIndex, GetTextIndexAtCursorRsp);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(WaitGetTextIndexAtCursorRsp(expectIndex));
}

/**
 * @tc.name: ImaTextEditTest_InsertText
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ImaTextEditTest, ImaTextEditTest_InsertText, TestSize.Level0)
{
    IMSA_HILOGI("ImeProxyTest::ImaTextEditTest_InsertText");
    // sync
    auto ret = InputMethodAbility::GetInstance().InsertText(INSERT_TEXT);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(KeyboardListenerTestImpl::WaitTextChange(INSERT_TEXT));
    KeyboardListenerTestImpl::ResetParam();
    // async
    ret = InputMethodAbility::GetInstance().InsertText(INSERT_TEXT, CommonRsp);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(WaitCommonRsp());
    finalText_ = INSERT_TEXT + INSERT_TEXT;
    EXPECT_TRUE(KeyboardListenerTestImpl::WaitTextChange(finalText_));
}

/**
 * @tc.name: ImaTextEditTest_SetPreviewText_FinishTextPreview
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ImaTextEditTest, ImaTextEditTest_SetPreviewText_FinishTextPreview, TestSize.Level0)
{
    IMSA_HILOGI("ImeProxyTest::ImaTextEditTest_SetPreviewText_FinishTextPreview");
    Range range = { -1, -1 };
    // sync
    auto ret = InputMethodAbility::GetInstance().SetPreviewText(INSERT_TEXT, range);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = InputMethodAbility::GetInstance().FinishTextPreview();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(KeyboardListenerTestImpl::WaitTextChange(INSERT_TEXT));
    KeyboardListenerTestImpl::ResetParam();

    // async
    ret = InputMethodAbility::GetInstance().SetPreviewText(INSERT_TEXT, range, CommonRsp);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(WaitCommonRsp());
    ResetParams();
    ret = InputMethodAbility::GetInstance().FinishTextPreview(CommonRsp);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(WaitCommonRsp());
    finalText_ = INSERT_TEXT + INSERT_TEXT;
    EXPECT_TRUE(KeyboardListenerTestImpl::WaitTextChange(finalText_));
}
/**
 * @tc.name: ImaTextEditTest_DeleteForward
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ImaTextEditTest, ImaTextEditTest_DeleteForward, TestSize.Level0)
{
    IMSA_HILOGI("ImeProxyTest::ImaTextEditTest_DeleteForward");
    // sync
    auto ret = InputMethodAbility::GetInstance().DeleteForward(DEL_LENGTH);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    // async
    ret = InputMethodAbility::GetInstance().DeleteForward(DEL_LENGTH, CommonRsp);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(WaitCommonRsp());
}

/**
 * @tc.name: ImaTextEditTest_DeleteBackward
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ImaTextEditTest, ImaTextEditTest_DeleteBackward, TestSize.Level0)
{
    IMSA_HILOGI("ImeProxyTest::ImaTextEditTest_DeleteBackward");
    // sync
    auto ret = InputMethodAbility::GetInstance().DeleteBackward(DEL_LENGTH);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    // async
    ret = InputMethodAbility::GetInstance().DeleteBackward(DEL_LENGTH, CommonRsp);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(WaitCommonRsp());
}

/**
 * @tc.name: ImaTextEditTest_SendExtendAction
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ImaTextEditTest, ImaTextEditTest_SendExtendAction, TestSize.Level0)
{
    IMSA_HILOGI("ImeProxyTest::ImaTextEditTest_SendExtendAction");
    // sync
    int32_t action = 1;
    auto ret = InputMethodAbility::GetInstance().SendExtendAction(action);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    // async
    ret = InputMethodAbility::GetInstance().SendExtendAction(action, CommonRsp);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(WaitCommonRsp());
}

/**
 * @tc.name: ImaTextEditTest_MoveCursor
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ImaTextEditTest, ImaTextEditTest_MoveCursor, TestSize.Level0)
{
    IMSA_HILOGI("ImeProxyTest::ImaTextEditTest_MoveCursor");
    // sync
    int32_t direction = 3;
    auto ret = InputMethodAbility::GetInstance().MoveCursor(direction);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    // async
    ret = InputMethodAbility::GetInstance().MoveCursor(direction, CommonRsp);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(WaitCommonRsp());
}

/**
 * @tc.name: ImaTextEditTest_SelectByRange
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ImaTextEditTest, ImaTextEditTest_SelectByRange, TestSize.Level0)
{
    IMSA_HILOGI("ImeProxyTest::ImaTextEditTest_SelectByRange");
    // sync
    int32_t start = 1;
    int32_t end = 2;
    auto ret = InputMethodAbility::GetInstance().SelectByRange(start, end);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    // async
    ret = InputMethodAbility::GetInstance().SelectByRange(start, end, CommonRsp);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(WaitCommonRsp());
}

/**
 * @tc.name: ImaTextEditTest_SelectByMovement
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ImaTextEditTest, ImaTextEditTest_SelectByMovement, TestSize.Level0)
{
    IMSA_HILOGI("ImeProxyTest::ImaTextEditTest_SelectByMovement");
    // sync
    int32_t direction = 1;
    auto ret = InputMethodAbility::GetInstance().SelectByMovement(direction);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    // async
    ret = InputMethodAbility::GetInstance().SelectByMovement(direction, CommonRsp);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(WaitCommonRsp());
}

/**
 * @tc.name: ImaTextEditTest_SendFunctionKey
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ImaTextEditTest, ImaTextEditTest_SendFunctionKey, TestSize.Level0)
{
    IMSA_HILOGI("ImeProxyTest::ImaTextEditTest_SendFunctionKey");
    int32_t funcKey = 1;
    // sync
    auto ret = InputMethodAbility::GetInstance().SendFunctionKey(funcKey);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    // async
    ret = InputMethodAbility::GetInstance().SendFunctionKey(funcKey, CommonRsp);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(WaitCommonRsp());
}

/**
 * @tc.name: ImaTextEditTest_ClearRspHandlers
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ImaTextEditTest, ImaTextEditTest_ClearRspHandlers, TestSize.Level0)
{
    IMSA_HILOGI("ImeProxyTest::ImaTextEditTest_ClearRspHandlers");
    auto channelProxy = std::make_shared<InputDataChannelProxy>(nullptr);
    auto channelWrap = std::make_shared<InputDataChannelProxyWrap>(channelProxy, nullptr);
    auto delayTask = [&channelWrap]() {
        usleep(100000);
        channelWrap->ClearRspHandlers();
    };
    std::thread delayThread(delayTask);

    channelWrap->AddRspHandler(GetForwardRsp, false, 0);
    channelWrap->AddRspHandler(GetForwardRsp, false, 0);
    auto handler = channelWrap->AddRspHandler(GetForwardRsp, true, 0);
    auto ret = channelWrap->WaitResponse(handler, nullptr);
    EXPECT_EQ(ret, ErrorCode::ERROR_IMA_DATA_CHANNEL_ABNORMAL);
    EXPECT_TRUE(WaitGetForwardRspAbnormal(2));
    delayThread.detach();
}

/**
 * @tc.name: ImaTextEditTest_DeleteRspHandler
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ImaTextEditTest, ImaTextEditTest_DeleteRspHandler, TestSize.Level0)
{
    constexpr std::size_t UNANSWERED_MAX_NUMBER = 1000;
    IMSA_HILOGI("ImeProxyTest::ImaTextEditTest_DeleteRspHandler");
    auto channelProxy = std::make_shared<InputDataChannelProxy>(nullptr);
    auto channelWrap = std::make_shared<InputDataChannelProxyWrap>(channelProxy, nullptr);

    std::shared_ptr<ResponseHandler> firstHandler = nullptr;
    std::shared_ptr<ResponseHandler> lastHandler = nullptr;
    firstHandler = channelWrap->AddRspHandler(GetForwardRsp, false, 0);
    for (int i = 0; i < UNANSWERED_MAX_NUMBER; ++i) {
        lastHandler = channelWrap->AddRspHandler(GetForwardRsp, false, 0);
    }
    ASSERT_NE(firstHandler, nullptr);
    ASSERT_NE(lastHandler, nullptr);
    EXPECT_EQ(channelWrap->DeleteRspHandler(firstHandler->msgId), ErrorCode::NO_ERROR);
    EXPECT_EQ(channelWrap->DeleteRspHandler(0), ErrorCode::NO_ERROR);
}

/**
 * @tc.name: ImaTextEditTest_HandleResponse
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ImaTextEditTest, ImaTextEditTest_HandleResponse, TestSize.Level0)
{
    IMSA_HILOGI("ImeProxyTest::ImaTextEditTest_HandleResponse");
    auto channelProxy = std::make_shared<InputDataChannelProxy>(nullptr);
    auto channelWrap = std::make_shared<InputDataChannelProxyWrap>(channelProxy, nullptr);

    std::shared_ptr<ResponseHandler> handler = nullptr;
    handler = channelWrap->AddRspHandler(CommonRsp, false, 0);
    ASSERT_NE(handler, nullptr);
    ResponseInfo rspInfo = { ErrorCode::NO_ERROR, std::monostate{} };
    channelWrap->HandleResponse(handler->msgId, rspInfo);
    EXPECT_TRUE(WaitCommonRsp());

    std::shared_ptr<ResponseHandler> handler1 = nullptr;
    handler1 = channelWrap->AddRspHandler(nullptr, false, 0);
    ASSERT_NE(handler1, nullptr);
    EXPECT_EQ(channelWrap->HandleResponse(handler1->msgId - 1, rspInfo), ErrorCode::NO_ERROR);
    EXPECT_EQ(channelWrap->HandleResponse(handler1->msgId, rspInfo), ErrorCode::NO_ERROR);
}

/**
 * @tc.name: ImaTextEditTest_HandleMsg
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ImaTextEditTest, ImaTextEditTest_HandleMsg, TestSize.Level0)
{
    IMSA_HILOGI("ImeProxyTest::ImaTextEditTest_HandleResponse");
    auto channelProxy = std::make_shared<InputDataChannelProxy>(nullptr);
    auto channelWrap = std::make_shared<InputDataChannelProxyWrap>(channelProxy, nullptr);

    ResponseInfo rspInfo = { ErrorCode::NO_ERROR, std::monostate{} };
    std::shared_ptr<ResponseHandler> handler = nullptr;
    handler = channelWrap->AddRspHandler(nullptr, false, 0);
    ASSERT_NE(handler, nullptr);
    std::lock_guard<std::mutex> lock(channelWrap->rspMutex_);
    EXPECT_EQ(channelWrap->HandleMsg(handler->msgId, rspInfo), ErrorCode::NO_ERROR);
}

/**
 * @tc.name: ImaTextEditTest_Report
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ImaTextEditTest, ImaTextEditTest_Report, TestSize.Level0)
{
    const int64_t REPORT_TIMEOUT = 200 + 1; // 200 is BASE_TEXT_OPERATION_TIMEOUT, unit ms
    IMSA_HILOGI("ImeProxyTest::ImaTextEditTest_Report");
    auto channelProxy = std::make_shared<InputDataChannelProxy>(nullptr);
    auto channelWrap = std::make_shared<InputDataChannelProxyWrap>(channelProxy, nullptr);

    channelWrap->ReportBaseTextOperation(1, ErrorCode::NO_ERROR, 1);
    channelWrap->ReportBaseTextOperation(1, ErrorCode::ERROR_NULL_POINTER, 1);
    channelWrap->ReportBaseTextOperation(1, ErrorCode::NO_ERROR, REPORT_TIMEOUT);
}
} // namespace MiscServices
} // namespace OHOS
