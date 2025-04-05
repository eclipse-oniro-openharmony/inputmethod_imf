/*
 * Copyright (C) 2021-2023 Huawei Device Co., Ltd.
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
#include "input_method_ability.h"
#include "itypes_util.h"
#undef private

#include <cstdint>
#include <gtest/gtest.h>
#include <string>

#include "global.h"
#include "inputmethod_sysevent.h"
#include "itypes_util.h"
#include "tdd_util.h"
#include "input_method_engine_listener.h"

using namespace testing::ext;
namespace OHOS {
namespace MiscServices {
class ITypesUtilTest : public testing::Test {
public:
    class InputMethodEngineListenerImpl : public InputMethodEngineListener {
    public:
        InputMethodEngineListenerImpl() = default;
        ~InputMethodEngineListenerImpl() = default;

        void OnKeyboardStatus(bool isShow)
        {
            IMSA_HILOGI("InputMethodEngineListenerImpl OnKeyboardStatus");
        }

        void OnInputStart()
        {
            IMSA_HILOGI("InputMethodEngineListenerImpl OnInputStart");
        }

        int32_t OnInputStop()
        {
            IMSA_HILOGI("InputMethodEngineListenerImpl OnInputStop");
            return ErrorCode::NO_ERROR;
        }

        void OnSetCallingWindow(uint32_t windowId)
        {
            IMSA_HILOGI("InputMethodEngineListenerImpl OnSetCallingWindow");
        }

        void OnSetSubtype(const SubProperty &property)
        {
            IMSA_HILOGI("InputMethodEngineListenerImpl OnSetSubtype");
        }

        void OnSecurityChange(int32_t security)
        {
            IMSA_HILOGI("InputMethodEngineListenerImpl OnSecurityChange");
        }

        void ReceivePrivateCommand(const std::unordered_map<std::string, PrivateDataValue> &privateCommand)
        {
            IMSA_HILOGI("InputMethodEngineListenerImpl ReceivePrivateCommand");
        }
    };

    void SetUp()
    {
        IMSA_HILOGI("ITypesUtilTest::SetUp");
    }
    void TearDown()
    {
        IMSA_HILOGI("ITypesUtilTest::TearDown");
    }
};

/**
 * @tc.name: testMarshallAndUnMarshallUint64
 * @tc.desc: IMA
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ITypesUtilTest, testMarshallAndUnMarshallUint64, TestSize.Level1)
{
    IMSA_HILOGI("ITypesUtilTest testMarshallingUint64 Test START");
    MessageParcel data;
    uint64_t input = 10001;
    data.WriteUint64(input);
    auto ret = ITypesUtil::Marshalling(input, data);
    EXPECT_TRUE(ret);
    ret = ITypesUtil::Unmarshalling(input, data);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: testMarshallAndUnMarshallProperty
 * @tc.desc: IMA
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ITypesUtilTest, testMarshallAndUnMarshallProperty, TestSize.Level1)
{
    IMSA_HILOGI("ITypesUtilTest testMarshallAndUnMarshallProperty Test START");
    MessageParcel data;
    Property input;
    string name = "test";
    data.WriteString(name);
    auto ret = ITypesUtil::Unmarshalling(input, data);
    EXPECT_FALSE(ret);
    ret = ITypesUtil::Marshalling(input, data);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: testMarshallAndUnMarshallSubProperty
 * @tc.desc: IMA
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ITypesUtilTest, testMarshallAndUnMarshallSubProperty, TestSize.Level1)
{
    IMSA_HILOGI("ITypesUtilTest testMarshallAndUnMarshallSubProperty Test START");
    MessageParcel data;
    SubProperty input;
    string name = "test";
    data.WriteString(name);
    auto ret = ITypesUtil::Unmarshalling(input, data);
    EXPECT_FALSE(ret);
    ret = ITypesUtil::Marshalling(input, data);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: testMarshallAndUnMarshallInputAttribute
 * @tc.desc: IMA
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ITypesUtilTest, testMarshallAndUnMarshallInputAttribute, TestSize.Level1)
{
    IMSA_HILOGI("ITypesUtilTest testMarshallAndUnMarshallInputAttribute Test START");
    MessageParcel data;
    InputAttribute input;
    string name = "test";
    data.WriteString(name);
    auto ret = ITypesUtil::Unmarshalling(input, data);
    EXPECT_FALSE(ret);
    ret = ITypesUtil::Marshalling(input, data);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: testMarshallAndUnMarshallInputAttributeSuccess
 * @tc.desc: IMA
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ITypesUtilTest, testMarshallAndUnMarshallInputAttributeSuccess_WithBundleName, TestSize.Level1)
{
    IMSA_HILOGI("ITypesUtilTest testMarshallAndUnMarshallInputAttributeSuccess Test START");
    MessageParcel data;
    InputAttribute attrIn { .bundleName = "MyBundleName" };
    auto ret = ITypesUtil::Marshalling(attrIn, data);
    EXPECT_TRUE(ret);

    InputAttribute attrOut;
    ret = ITypesUtil::Unmarshalling(attrOut, data);
    EXPECT_TRUE(ret);
    EXPECT_EQ(attrIn, attrOut);
    EXPECT_EQ(attrIn.bundleName, attrOut.bundleName);
}

/**
 * @tc.name: testMarshallAndUnMarshallInputAttributeSuccess
 * @tc.desc: IMA
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ITypesUtilTest, testMarshallAndUnMarshallInputAttribute_WithEmptyBundleName, TestSize.Level1)
{
    IMSA_HILOGI("ITypesUtilTest testMarshallAndUnMarshallInputAttributeSuccess Test START");
    MessageParcel data;
    InputAttribute attrIn { .bundleName = "" };
    auto ret = ITypesUtil::Marshalling(attrIn, data);
    EXPECT_TRUE(ret);

    InputAttribute attrOut;
    ret = ITypesUtil::Unmarshalling(attrOut, data);
    EXPECT_TRUE(ret);
    EXPECT_EQ(attrIn, attrOut);
    EXPECT_EQ(attrIn.bundleName, attrOut.bundleName);
}

/**
 * @tc.name: testMarshallAndUnMarshallTextTotalConfig
 * @tc.desc: IMA
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ITypesUtilTest, testMarshallAndUnMarshallTextTotalConfig, TestSize.Level1)
{
    IMSA_HILOGI("ITypesUtilTest testMarshallAndUnMarshallTextTotalConfig Test START");
    MessageParcel data;
    TextTotalConfig input;
    auto ret = ITypesUtil::Unmarshalling(input, data);
    EXPECT_FALSE(ret);
    ret = ITypesUtil::Marshalling(input, data);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: testMarshallAndUnMarshallInputClientInfo
 * @tc.desc: IMA
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ITypesUtilTest, testMarshallAndUnMarshallInputClientInfo, TestSize.Level1)
{
    IMSA_HILOGI("ITypesUtilTest testMarshallAndUnMarshallInputClientInfo Test START");
    MessageParcel data;
    InputClientInfo input;
    auto ret = ITypesUtil::Unmarshalling(input, data);
    EXPECT_FALSE(ret);
    ret = ITypesUtil::Marshalling(input, data);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: testMarshallAndUnMarshallImeWindowInfo
 * @tc.desc: IMA
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ITypesUtilTest, testMarshallAndUnMarshallImeWindowInfo, TestSize.Level1)
{
    IMSA_HILOGI("ITypesUtilTest testMarshallAndUnMarshallImeWindowInfo Test START");
    MessageParcel data;
    ImeWindowInfo input;
    auto ret = ITypesUtil::Unmarshalling(input, data);
    EXPECT_FALSE(ret);
    ret = ITypesUtil::Marshalling(input, data);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: testMarshallAndUnMarshallSysPanelStatus
 * @tc.desc: IMA
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ITypesUtilTest, testMarshallAndUnMarshallSysPanelStatus, TestSize.Level1)
{
    IMSA_HILOGI("ITypesUtilTest testMarshallAndUnMarshallSysPanelStatus Test START");
    MessageParcel data;
    SysPanelStatus input;
    auto ret = ITypesUtil::Unmarshalling(input, data);
    EXPECT_FALSE(ret);
    ret = ITypesUtil::Marshalling(input, data);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: testMarshallAndUnMarshallElementName
 * @tc.desc: IMA
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ITypesUtilTest, testMarshallAndUnMarshallElementName, TestSize.Level1)
{
    IMSA_HILOGI("ITypesUtilTest testMarshallAndUnMarshallElementName Test START");
    MessageParcel data;
    OHOS::AppExecFwk::ElementName input;
    auto ret = ITypesUtil::Unmarshalling(input, data);
    EXPECT_FALSE(ret);
    ret = ITypesUtil::Marshalling(input, data);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: testMarshallAndUnMarshallInputType
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ITypesUtilTest, testMarshallAndUnMarshallInputType, TestSize.Level1)
{
    IMSA_HILOGI("ITypesUtilTest testMarshallAndUnMarshallInputType Test START");
    MessageParcel data;
    InputType input { InputType::NONE };
    auto ret = ITypesUtil::Unmarshalling(input, data);
    EXPECT_FALSE(ret);
    data.WriteInt32(1);
    ret = ITypesUtil::Unmarshalling(input, data);
    EXPECT_TRUE(ret);
    ret = ITypesUtil::Marshalling(input, data);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: testMarshallAndUnMarshallRange
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ITypesUtilTest, testMarshallAndUnMarshallRange, TestSize.Level1)
{
    IMSA_HILOGI("ITypesUtilTest testMarshallAndUnMarshallRange Test START");
    MessageParcel data;
    Range input;
    auto ret = ITypesUtil::Unmarshalling(input, data);
    EXPECT_FALSE(ret);
    ret = ITypesUtil::Marshalling(input, data);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: testInputMethodSysEvent
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ITypesUtilTest, testInputMethodSysEvent, TestSize.Level1)
{
    IMSA_HILOGI("ITypesUtilTest testInputMethodSysEvent Test START");
    auto ret = InputMethodSysEvent::GetInstance().StartTimerForReport();
    EXPECT_TRUE(ret);
}
} // namespace MiscServices
} // namespace OHOS
