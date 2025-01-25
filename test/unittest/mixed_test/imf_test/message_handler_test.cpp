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

// message_handler_test.cpp
#include <gtest/gtest.h>
#include "inputmethod_message_handler.h"
#include "message.h"
#include "message_parcel.h"

using namespace OHOS;
using namespace OHOS::MiscServices;

TEST(MessageHandlerTest, Singleton)
{
    MessageHandler *handler1 = MessageHandler::Instance();
    MessageHandler *handler2 = MessageHandler::Instance();
    EXPECT_EQ(handler1, handler2);
}

TEST(MessageHandlerTest, SendMessageAndGetMessage)
{
    MessageHandler *handler = MessageHandler::Instance();
    MessageParcel *msgContent = new MessageParcel();
    msgContent->WriteInt32(123);
    Message *msg = new Message(1, msgContent);

    handler->SendMessage(msg);

    Message *receivedMsg = handler->GetMessage();
    EXPECT_EQ(receivedMsg->GetMsgId(), 1);
    int32_t value;
    receivedMsg->GetMsgContent()->ReadInt32(value);
    EXPECT_EQ(value, 123);

    delete receivedMsg;
    delete msgContent;
}

TEST(MessageHandlerTest, SendMultipleMessages)
{
    MessageHandler *handler = MessageHandler::Instance();

    MessageParcel *msgContent1 = new MessageParcel();
    msgContent1->WriteInt32(123);
    Message *msg1 = new Message(1, msgContent1);

    MessageParcel *msgContent2 = new MessageParcel();
    msgContent2->WriteInt32(456);
    Message *msg2 = new Message(2, msgContent2);

    handler->SendMessage(msg1);
    handler->SendMessage(msg2);

    Message *receivedMsg1 = handler->GetMessage();
    EXPECT_EQ(receivedMsg1->GetMsgId(), 1);
    int32_t value1;
    receivedMsg1->GetMsgContent()->ReadInt32(value1);
    EXPECT_EQ(value1, 123);

    Message *receivedMsg2 = handler->GetMessage();
    EXPECT_EQ(receivedMsg2->GetMsgId(), 2);
    int32_t value2;
    receivedMsg2->GetMsgContent()->ReadInt32(value2);
    EXPECT_EQ(value2, 456);

    delete receivedMsg1;
    delete receivedMsg2;
    delete msgContent1;
    delete msgContent2;
}

TEST(MessageHandlerTest, Destructor)
{
    MessageHandler *handler = MessageHandler::Instance();

    MessageParcel *msgContent1 = new MessageParcel();
    msgContent1->WriteInt32(123);
    Message *msg1 = new Message(1, msgContent1);

    MessageParcel *msgContent2 = new MessageParcel();
    msgContent2->WriteInt32(456);
    Message *msg2 = new Message(2, msgContent2);

    handler->SendMessage(msg1);
    handler->SendMessage(msg2);

    delete handler;
}

TEST(MessageHandlerTest, ThreadSafety)
{
    MessageHandler *handler = MessageHandler::Instance();

    auto sendMessages = [handler]() {
        for (int i = 0; i < 10; ++i) {
            MessageParcel *msgContent = new MessageParcel();
            msgContent->WriteInt32(i);
            Message *msg = new Message(i, msgContent);
            handler->SendMessage(msg);
        }
    };

    auto receiveMessages = [handler]() {
        for (int i = 0; i < 10; ++i) {
            Message *msg = handler->GetMessage();
            int32_t value;
            msg->GetMsgContent()->ReadInt32(value);
            EXPECT_EQ(value, i);
            delete msg;
        }
    };

    std::thread sender1(sendMessages);
    std::thread sender2(sendMessages);
    std::thread receiver1(receiveMessages);
    std::thread receiver2(receiveMessages);

    sender1.join();
    sender2.join();
    receiver1.join();
    receiver2.join();
}
