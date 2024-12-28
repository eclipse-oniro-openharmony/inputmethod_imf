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

#include "focus_change_listener.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "global.h"

namespace OHOS {
namespace MiscServices {
using namespace testing;

class MockFocusChangeInfo : public Rosen::FocusChangeInfo {
public:
    int32_t pid_ = 123;
    int32_t uid_ = 456;
};

class MockFocusHandle {
public:
    MOCK_METHOD3(operator(), void(bool, int32_t, int32_t));
};

class FocusChangedListenerTest : public Test {
protected:
    void SetUp() override
    {
        focusHandle_ = std::make_shared<MockFocusHandle>();
        focusChangedListener_ = std::make_shared<FocusChangedListener>();
        focusChangedListener_->SetFocusHandle(focusHandle_);
    }

    std::shared_ptr<MockFocusHandle> focusHandle_;
    std::shared_ptr<FocusChangedListener> focusChangedListener_;
};

/**
 * @tc.name: OnFocused_FocusChangeInfoIsNull_NoCallToFocusHandle
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FocusChangedListenerTest, OnFocused_FocusChangeInfoIsNull_NoCallToFocusHandle, TestSize.Level0)
{
    focusChangedListener_->OnFocused(nullptr);
    EXPECT_CALL(*focusHandle_, operator()(true, _, _)).Times(0);
}

/**
 * @tc.name: OnFocused_FocusHandleIsNull_NoCallToFocusHandle
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FocusChangedListenerTest, OnFocused_FocusHandleIsNull_NoCallToFocusHandle, TestSize.Level0)
{
    focusChangedListener_->SetFocusHandle(nullptr);
    auto focusChangeInfo = std::make_shared<MockFocusChangeInfo>();
    focusChangedListener_->OnFocused(focusChangeInfo);
    EXPECT_CALL(*focusHandle_, operator()(true, _, _)).Times(0);
}

/**
 * @tc.name: OnFocused_ValidInputs_CallsFocusHandle
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FocusChangedListenerTest, OnFocused_ValidInputs_CallsFocusHandle, TestSize.Level0)
{
    auto focusChangeInfo = std::make_shared<MockFocusChangeInfo>();
    EXPECT_CALL(*focusHandle_, operator()(true, focusChangeInfo->pid_, focusChangeInfo->uid_)).Times(1);
    focusChangedListener_->OnFocused(focusChangeInfo);
}

/**
 * @tc.name: OnUnfocused_FocusChangeInfoIsNull_NoCallToFocusHandle
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FocusChangedListenerTest, OnUnfocused_FocusChangeInfoIsNull_NoCallToFocusHandle, TestSize.Level0)
{
    focusChangedListener_->OnUnfocused(nullptr);
    EXPECT_CALL(*focusHandle_, operator()(false, _, _)).Times(0);
}

/**
 * @tc.name: OnUnfocused_FocusHandleIsNull_NoCallToFocusHandle
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FocusChangedListenerTest, OnUnfocused_FocusHandleIsNull_NoCallToFocusHandle, TestSize.Level0)
{
    focusChangedListener_->SetFocusHandle(nullptr);
    auto focusChangeInfo = std::make_shared<MockFocusChangeInfo>();
    focusChangedListener_->OnUnfocused(focusChangeInfo);
    EXPECT_CALL(*focusHandle_, operator()(false, _, _)).Times(0);
}

/**
 * @tc.name: OnUnfocused_ValidInputs_CallsFocusHandle
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FocusChangedListenerTest, OnUnfocused_ValidInputs_CallsFocusHandle, TestSize.Level0)
{
    auto focusChangeInfo = std::make_shared<MockFocusChangeInfo>();
    EXPECT_CALL(*focusHandle_, operator()(false, focusChangeInfo->pid_, focusChangeInfo->uid_)).Times(1);
    focusChangedListener_->OnUnfocused(focusChangeInfo);
}
} // namespace MiscServices
} // namespace OHOS