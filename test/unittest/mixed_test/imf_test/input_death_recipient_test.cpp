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

#include "gtest/gtest.h"
#include "input_death_recipient.h"

using namespace OHOS::MiscServices;

class MockRemoteDiedHandler : public RemoteDiedHandler {
public:
    MOCK_METHOD1(OnRemoteDied, void(const wptr<IRemoteObject>&));
};

TEST(InputDeathRecipientTest, SetDeathRecipient)
{
    InputDeathRecipient recipient;
    MockRemoteDiedHandler handler;
    EXPECT_CALL(handler, OnRemoteDied(testing::_)).Times(1);
    recipient.SetDeathRecipient(std::move(handler));
    sptr<IRemoteObject> remote = nullptr;
    recipient.OnRemoteDied(remote);
}