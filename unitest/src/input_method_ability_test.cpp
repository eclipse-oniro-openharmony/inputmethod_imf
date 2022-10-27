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
#include <sys/time.h>

#include <cstdint>
#include <functional>
#include <string>
#include <thread>
#include <vector>

#include "global.h"
#include "i_input_data_channel.h"
#include "input_attribute.h"
#include "input_control_channel_stub.h"
#include "input_data_channel_proxy.h"
#include "input_data_channel_stub.h"
#include "input_method_agent_stub.h"
#include "input_method_core_proxy.h"
#include "input_method_core_stub.h"
#include "message_handler.h"

using namespace testing::ext;
namespace OHOS {
namespace MiscServices {
    class InputMethodAbilityTest : public testing::Test {
    public:
        static void SetUpTestCase(void);
        static void TearDownTestCase(void);
        void SetUp();
        void TearDown();
    };

    void InputMethodAbilityTest::SetUpTestCase(void)
    {
        IMSA_HILOGI("InputMethodAbilityTest::SetUpTestCase");
    }

    void InputMethodAbilityTest::TearDownTestCase(void)
    {
        IMSA_HILOGI("InputMethodAbilityTest::TearDownTestCase");
    }

    void InputMethodAbilityTest::SetUp(void)
    {
        IMSA_HILOGI("InputMethodAbilityTest::SetUp");
    }

    void InputMethodAbilityTest::TearDown(void)
    {
        IMSA_HILOGI("InputMethodAbilityTest::TearDown");
    }

    /**
    * @tc.name: testReadWriteIInputMethodAgent
    * @tc.desc: Checkout IInputMethodAgent.
    * @tc.type: FUNC
    * @tc.require: issueI5JBR6
    */
    HWTEST_F(InputMethodAbilityTest, testReadWriteIInputMethodAgent, TestSize.Level0)
    {
        sptr<InputMethodAgentStub> mInputMethodAgentStub = new InputMethodAgentStub();
        MessageParcel data;
        auto ret =  data.WriteRemoteObject(mInputMethodAgentStub->AsObject());
        EXPECT_TRUE(ret);
        auto remoteObject = data.ReadRemoteObject();
        sptr<IInputMethodAgent> iface = iface_cast<IInputMethodAgent>(remoteObject);
        EXPECT_TRUE(iface != nullptr);
    }

    /**
    * @tc.name: testReadWriteIInputMethodCore
    * @tc.desc: Checkout IInputMethodCore.
    * @tc.type: FUNC
    */
    HWTEST_F(InputMethodAbilityTest, testReadWriteIInputMethodCore, TestSize.Level0)
    {
        sptr<InputMethodCoreStub> mInputMethodCoreStub = new InputMethodCoreStub(0);
        MessageParcel data;
        auto ret =  data.WriteRemoteObject(mInputMethodCoreStub->AsObject());
        EXPECT_TRUE(ret);
        auto remoteObject = data.ReadRemoteObject();
        sptr<IInputMethodCore> iface = iface_cast<IInputMethodCore>(remoteObject);
        EXPECT_TRUE(iface != nullptr);
    }

    /**
     * @tc.name: testShowKeyboardInputMethodCoreProxy
     * @tc.desc: Test InputMethodCoreProxy ShowKeyboard
     * @tc.type: FUNC
     * @tc.require: issueI5NXHK
     */
    HWTEST_F(InputMethodAbilityTest, testShowKeyboardInputMethodCoreProxy, TestSize.Level0)
    {
        sptr<InputMethodCoreStub> coreStub = new InputMethodCoreStub(0);
        sptr<IInputMethodCore> core = coreStub;
        auto msgHandler = new (std::nothrow) MessageHandler();
        coreStub->SetMessageHandler(msgHandler);
        msgHandler = new MessageHandler();
        coreStub->SetMessageHandler(msgHandler);
        sptr<InputDataChannelStub> channelStub = new InputDataChannelStub();

        MessageParcel data;
        data.WriteRemoteObject(core->AsObject());
        data.WriteRemoteObject(channelStub->AsObject());
        sptr<IRemoteObject> coreObject = data.ReadRemoteObject();
        sptr<IRemoteObject> channelObject = data.ReadRemoteObject();

        sptr<InputMethodCoreProxy> coreProxy = new InputMethodCoreProxy(coreObject);
        sptr<InputDataChannelProxy> channelProxy = new InputDataChannelProxy(channelObject);
        SubProperty subProperty;
        auto ret = coreProxy->showKeyboard(channelProxy, true, subProperty);
        EXPECT_EQ(ret, 0);
    }

    /**
     * @tc.name: testShowKeyboardInputMethodCoreStub
     * @tc.desc: Test InputMethodCoreStub ShowKeyboard
     * @tc.type: FUNC
     * @tc.require: issueI5NXHK
     */
    HWTEST_F(InputMethodAbilityTest, testShowKeyboardInputMethodCoreStub, TestSize.Level0)
    {
        sptr<InputMethodCoreStub> coreStub = new InputMethodCoreStub(0);
        SubProperty subProperty;
        auto ret = coreStub->showKeyboard(nullptr, true, subProperty);
        EXPECT_EQ(ret, 0);
    }

    /**
    * @tc.name: testReadWriteIInputControlChannel
    * @tc.desc: Checkout IInputControlChannel.
    * @tc.type: FUNC
    */
    HWTEST_F(InputMethodAbilityTest, testReadWriteIInputControlChannel, TestSize.Level0)
    {
        sptr<InputControlChannelStub> mInputControlChannelStub = new InputControlChannelStub(0);
        MessageParcel data;
        auto ret =  data.WriteRemoteObject(mInputControlChannelStub->AsObject());
        EXPECT_TRUE(ret);
        auto remoteObject = data.ReadRemoteObject();
        sptr<IInputControlChannel> iface = iface_cast<IInputControlChannel>(remoteObject);
        EXPECT_TRUE(iface != nullptr);
    }

    /**
    * @tc.name: testSerializedInputAttribute
    * @tc.desc: Checkout the serialization of InputAttribute.
    * @tc.type: FUNC
    */
    HWTEST_F(InputMethodAbilityTest, testSerializedInputAttribute, TestSize.Level0)
    {
        InputAttribute inAttribute;
        inAttribute.inputPattern = InputAttribute::PATTERN_PASSWORD;
        MessageParcel data;
        EXPECT_TRUE(InputAttribute::Marshalling(inAttribute, data));
        InputAttribute outAttribute;
        EXPECT_TRUE(InputAttribute::Unmarshalling(outAttribute, data));
        EXPECT_TRUE(outAttribute.GetSecurityFlag());
    }

    /**
    * @tc.name: testSerializedKeyboardType
    * @tc.desc: Checkout the serialization of KeyboardType.
    * @tc.type: FUNC
    */
    HWTEST_F(InputMethodAbilityTest, testSerializedKeyboardType, TestSize.Level0)
    {
        int32_t def_value = 2021;
        sptr<KeyboardType> mKeyboardType = new KeyboardType();
        mKeyboardType->setId(def_value);
        MessageParcel data;
        auto ret =  data.WriteParcelable(mKeyboardType);
        EXPECT_TRUE(ret);
        sptr<KeyboardType> deserialization = data.ReadParcelable<KeyboardType>();
        EXPECT_TRUE(deserialization != nullptr);
        EXPECT_TRUE(deserialization->getId() == def_value);
    }
} // namespace MiscServices
} // namespace OHOS
