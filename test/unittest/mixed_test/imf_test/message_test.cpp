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

// message_test.cpp
#include <gtest/gtest.h>
#include "message.h"
#include "message_parcel.h"

using namespace OHOS;
using namespace OHOS::MiscServices;

TEST(MessageTest, DefaultConstructor)
{
    Message msg;
    EXPECT_EQ(msg.GetMsgId(), 0);
    EXPECT_EQ(msg.GetMsgContent(), nullptr);
}

TEST(MessageTest, ParameterizedConstructor)
{
    MessageParcel *msgContent = new MessageParcel();
    msgContent->WriteInt32(123);
    Message msg(1, msgContent);
    EXPECT_EQ(msg.GetMsgId(), 1);
    EXPECT_NE(msg.GetMsgContent(), nullptr);
    int32_t value;
    msg.GetMsgContent()->ReadInt32(value);
    EXPECT_EQ(value, 123);
    delete msgContent;
}

TEST(MessageTest, CopyConstructor)
{
    MessageParcel *msgContent = new MessageParcel();
    msgContent->WriteInt32(123);
    Message original(1, msgContent);
    Message copy(original);
    EXPECT_EQ(copy.GetMsgId(), 1);
    EXPECT_NE(copy.GetMsgContent(), nullptr);
    int32_t value;
    copy.GetMsgContent()->ReadInt32(value);
    EXPECT_EQ(value, 123);
    delete msgContent;
}

TEST(MessageTest, AssignmentOperator)
{
    MessageParcel *msgContent = new MessageParcel();
    msgContent->WriteInt32(123);
    Message original(1, msgContent);
    Message copy;
    copy = original;
    EXPECT_EQ(copy.GetMsgId(), 1);
    EXPECT_NE(copy.GetMsgContent(), nullptr);
    int32_t value;
    copy.GetMsgContent()->ReadInt32(value);
    EXPECT_EQ(value, 123);
    delete msgContent;
}

TEST(MessageTest, Destructor)
{
    MessageParcel *msgContent = new MessageParcel();
    msgContent->WriteInt32(123);
    Message *msg = new Message(1, msgContent);
    EXPECT_EQ(msg.GetMsgContent(), 123);
    delete msg;
}

TEST(MessageTest, GetSetMsgId)
{
    Message msg;
    msg.SetMsgId(456);
    EXPECT_EQ(msg.GetMsgId(), 456);
}

TEST(MessageTest, GetSetMsgContent)
{
    MessageParcel *msgContent = new MessageParcel();
    msgContent->WriteInt32(123);
    Message msg;
    msg.SetMsgContent(msgContent);
    EXPECT_NE(msg.GetMsgContent(), nullptr);
    int32_t value;
    msg.GetMsgContent()->ReadInt32(value);
    EXPECT_EQ(value, 123);
    delete msgContent;
}

TEST(MessageTest, DeepCopy)
{
    MessageParcel *msgContent = new MessageParcel();
    msgContent->WriteInt32(123);
    Message original(1, msgContent);
    Message copy(original);
    EXPECT_EQ(copy.GetMsgId(), 1);
    EXPECT_NE(copy.GetMsgContent(), original.GetMsgContent());
    int32_t value;
    copy.GetMsgContent()->ReadInt32(value);
    EXPECT_EQ(value, 123);
    delete msgContent;
}

TEST(MessageTest, AssignmentDeepCopy)
{
    MessageParcel *msgContent = new MessageParcel();
    msgContent->WriteInt32(123);
    Message original(1, msgContent);
    Message copy;
    copy = original;
    EXPECT_EQ(copy.GetMsgId(), 1);
    EXPECT_NE(copy.GetMsgContent(), original.GetMsgContent());
    int32_t value;
    copy.GetMsgContent()->ReadInt32(value);
    EXPECT_EQ(value, 123);
    delete msgContent;
}
