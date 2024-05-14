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

#define private public
#define protected public
#include "ime_system_channel.h"
#undef private
#include <gtest/gtest.h>

#include "scope_utils.h"
#include "tdd_util.h"
using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace MiscServices {
class OnSystemCmdListenerImpl : public OnSystemCmdListener {
    void ReceivePrivateCommand(const std::unordered_map<std::string, PrivateDataValue> &privateCommand) override
    {
    }
    void NotifyIsShowSysPanel(bool shouldSysPanelShow) override
    {
    }
};
class ImeSystemChannelTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static sptr<ImeSystemCmdChannel> imeSystemChannel_;
    static sptr<OnSystemCmdListener> sysCmdListener_;
    static uint64_t permissionTokenId_;
};
sptr<ImeSystemCmdChannel> ImeSystemChannelTest::imeSystemChannel_;
sptr<OnSystemCmdListener> ImeSystemChannelTest::sysCmdListener_;
uint64_t ImeSystemChannelTest::permissionTokenId_ = 0;

void ImeSystemChannelTest::SetUpTestCase(void)
{
    TddUtil::StorageSelfTokenID();
    imeSystemChannel_ = ImeSystemCmdChannel::GetInstance();
    sysCmdListener_ = new (std::nothrow) OnSystemCmdListenerImpl();
    permissionTokenId_ =
        TddUtil::AllocTestTokenID(true, "ohos.inputMethod.test", { "ohos.permission.CONNECT_IME_ABILITY" });
}

void ImeSystemChannelTest::TearDownTestCase(void)
{
    IMSA_HILOGI("ImeSystemChannelTest::TearDownTestCase");
    imeSystemChannel_->ConnectSystemCmd(nullptr);
    imeSystemChannel_->ClearSystemCmdAgent();
    TddUtil::RestoreSelfTokenID();
}

void ImeSystemChannelTest::SetUp(void)
{
    IMSA_HILOGI("ImeSystemChannelTest::SetUp");
}

void ImeSystemChannelTest::TearDown(void)
{
    IMSA_HILOGI("ImeSystemChannelTest::TearDown");
}

/**
 * @tc.name: testConnectSystemCmd001
 * @tc.desc: SystemCmdChannel ConnectSystemCmd.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ImeSystemChannelTest, testConnectSystemCmd001, TestSize.Level0)
{
    IMSA_HILOGI("ImeSystemChannelTest testConnectSystemCmd001 Test START");
    auto ret = imeSystemChannel_->ConnectSystemCmd(sysCmdListener_);
    EXPECT_EQ(ret, ErrorCode::ERROR_STATUS_SYSTEM_PERMISSION);
}

/**
 * @tc.name: testConnectSystemCmd002
 * @tc.desc: SystemCmdChannel ConnectSystemCmd.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ImeSystemChannelTest, testConnectSystemCmd002, TestSize.Level0)
{
    IMSA_HILOGI("ImeSystemChannelTest testConnectSystemCmd002 Test START");
    TokenScope scope(ImeSystemChannelTest::permissionTokenId_);
    auto ret = imeSystemChannel_->ConnectSystemCmd(sysCmdListener_);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: testSendPrivateCommand001
 * @tc.desc: SystemCmdChannel SendPrivateCommand.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ImeSystemChannelTest, testSendPrivateCommand001, TestSize.Level0)
{
    IMSA_HILOGI("ImeSystemChannelTest testSendPrivateCommand001 Test START");
    TokenScope scope(ImeSystemChannelTest::permissionTokenId_);
    std::unordered_map<std::string, PrivateDataValue> privateCommand;
    auto ret = imeSystemChannel_->SendPrivateCommand(privateCommand);
    EXPECT_EQ(ret, ErrorCode::ERROR_INVALID_PRIVATE_COMMAND);
}

/**
 * @tc.name: testImeSystemChannel_nullptr
 * @tc.desc: SystemCmdChannel ReceivePrivateCommand.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ImeSystemChannelTest, testImeSystemChannel_nullptr, TestSize.Level0)
{
    IMSA_HILOGI("ImeSystemChannelTest testImeSystemChannel_nullptr Test START");
    std::unordered_map<std::string, PrivateDataValue> privateCommand;
    PrivateDataValue privateDataValue1 = std::string("stringValue");
    privateCommand.emplace("value1", privateDataValue1);
    imeSystemChannel_->systemCmdListener_ = nullptr;
    imeSystemChannel_->OnConnectCmdReady(nullptr);
    imeSystemChannel_->GetSmartMenuCfg();
    int32_t ret = imeSystemChannel_->ReceivePrivateCommand(privateCommand);
    EXPECT_EQ(ret, ErrorCode::ERROR_EX_NULL_POINTER);
    ret = imeSystemChannel_->ShowSysPanel(true);
    EXPECT_EQ(ret, ErrorCode::ERROR_NULL_POINTER);
}
} // namespace MiscServices
} // namespace OHOS