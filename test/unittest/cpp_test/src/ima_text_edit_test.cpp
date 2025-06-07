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
    static constexpr const char *ABNORMAL_EDITOR_BOX_BUNDLE_NAME = "com.example.abnormalEditorBox";
    static constexpr const char *CLICK_CMD = "uinput -T -d 200 200 -u 200 200";
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
        TddUtil::ClickApp(CLICK_CMD);
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
            [&num]() { return getForwardRspNums_ == num && dealRet_ == ErrorCode::ERROR_IMA_CHANNEL_NULLPTR; });
        return getForwardRspNums_ == num && dealRet_ == ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }

    static bool WaitGetForwardRsp(const std::string &text)
    {
        std::unique_lock<std::mutex> lock(retCvLock_);
        retCv_.wait_for(lock, std::chrono::seconds(MAX_WAIT_TIME),
            [&]() { return dealRet_ == ErrorCode::NO_ERROR && text == getForwardText_; });
        return dealRet_ == ErrorCode::NO_ERROR;
    }

    static std::mutex retCvLock_;
    static std::condition_variable retCv_;
    static int32_t dealRet_;
    static int32_t getForwardRspNums_;
    static std::string getForwardText_;
    static std::string finalText_;
};

std::mutex ImaTextEditTest::retCvLock_;
std::condition_variable ImaTextEditTest::retCv_;
int32_t ImaTextEditTest::dealRet_{ ErrorCode::ERROR_CLIENT_NOT_BOUND };
const std::string ImaTextEditTest::INSERT_TEXT = "ABCDEFGHIJKMN";
std::string ImaTextEditTest::finalText_;
int32_t ImaTextEditTest::getForwardRspNums_{ 0 };
std::string ImaTextEditTest::getForwardText_;

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
    KeyboardListenerTestImpl::ResetParam();
    ret = InputMethodAbility::GetInstance().SendFunctionKey(funcKey, CommonRsp);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(WaitCommonRsp());
}

/**
 * @tc.name: ImaTextEditTest_GetForward
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ImaTextEditTest, ImaTextEditTest_GetForward, TestSize.Level0)
{
    IMSA_HILOGI("ImeProxyTest::ImaTextEditTest_GetForward");
    auto ret = InputMethodAbility::GetInstance().InsertText(INSERT_TEXT);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(KeyboardListenerTestImpl::WaitTextChange(INSERT_TEXT));

    auto expectText = INSERT_TEXT.substr(INSERT_TEXT.size() - DEL_LENGTH);
    std::u16string syncText;
    // sync
    ret = InputMethodAbility::GetInstance().GetTextBeforeCursor(GET_LENGTH, asyncText);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(Str16ToStr8(asyncText), expectText);

    // async
    std::u16string asyncText;
    KeyboardListenerTestImpl::ResetParam();
    ret = InputMethodAbility::GetInstance().GetTextBeforeCursor(GET_LENGTH, asyncText, GetForwardRsp);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(WaitGetForwardRsp(expectText));
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
    // async
    KeyboardListenerTestImpl::ResetParam();
    ret = InputMethodAbility::GetInstance().InsertText(INSERT_TEXT, CommonRsp);
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
    auto ret = InputMethodAbility::GetInstance().InsertText(INSERT_TEXT);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(KeyboardListenerTestImpl::WaitTextChange(INSERT_TEXT));

    // sync
    KeyboardListenerTestImpl::ResetParam();
    ret = InputMethodAbility::GetInstance().DeleteForward(DEL_LENGTH);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    finalText_ = INSERT_TEXT.substr(0, INSERT_TEXT.size() - DEL_LENGTH);
    EXPECT_TRUE(KeyboardListenerTestImpl::WaitTextChange(finalText_));

    // async
    KeyboardListenerTestImpl::ResetParam();
    ret = InputMethodAbility::GetInstance().DeleteForward(DEL_LENGTH, CommonRsp);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(WaitCommonRsp());
    finalText_ = finalText_.substr(0, finalText_.size() - DEL_LENGTH);
    EXPECT_TRUE(KeyboardListenerTestImpl::WaitTextChange(finalText_));
}

/**
 * @tc.name: ImaTextEditTest_ClearRspHandlers
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(ImaTextEditTest, ImaTextEditTest_ClearRspHandlers, TestSize.Level0)
{
    IMSA_HILOGI("ImeProxyTest::ImaTextEditTest_ClearRspHandlers");
    auto channelProxy = std::make_shared<InputDataChannelProxy>();
    auto channelWrap = std::make_shared<InputDataChannelProxyWrap>(channelProxy);
    auto delayTask = [&channelWrap]() {
        usleep(100000);
        channelWrap->ClearRspHandlers();
    };
    std::thread delayThread(delayTask);
    delayThread.detach();
    channelWrap->AddRspHandler();           // getforward异步
    channelWrap->AddRspHandler();           // getforward异步
    channelWrap->AddRspHandler();           // 同步
    auto ret = channelWrap->WaitResponse(); // 同步等待
    EXPECT_EQ(ret, ErrorCode::ERROR_IMA_CHANNEL_NULLPTR);
    EXPECT_TRUE(WaitGetForwardRspAbnormal(2));
}

} // namespace MiscServices
} // namespace OHOS
