/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "window_adapter.h"

#include <gtest/gtest.h>

#include <functional>

#include "global.h"

namespace OHOS {
namespace MiscServices {
using namespace testing::ext;
class WindowAdapterTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
    }
    static void TearDownTestCase()
    {
    }
    void SetUp() override
    {
        WindowDisplayChangeHandler callback = [](OHOS::Rosen::CallingWindowInfo callingWindowInfo) {
            IMSA_HILOGD("callback result:%{public}s",
                WindowDisplayChangeListener::CallingWindowInfoToString(callingWindowInfo).c_str());
        };
        WindowAdapter::GetInstance().RegisterCallingWindowInfoChangedListener(callback);
    }
    void TearDown() override
    {
    }
};

/**
 * @tc.name: WindowAdapter_GetCallingWindowInfo
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WindowAdapterTest, WindowAdapter_GetCallingWindowInfo, TestSize.Level0)
{
    OHOS::Rosen::CallingWindowInfo callingWindowInfo;
    uint32_t windId = 0;
    int32_t userId = -1;
    auto ret = WindowAdapter::GetInstance().GetCallingWindowInfo(windId, userId, callingWindowInfo);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: WindowAdapter_GetFocusInfo
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WindowAdapterTest, WindowAdapter_GetFocusInfo, TestSize.Level0)
{
    OHOS::Rosen::FocusChangeInfo focusInfo;
    WindowAdapter::GetInstance().GetFocusInfo(focusInfo);
    EXPECT_TRUE(focusInfo.displayId_ >= 0);
}
} // namespace MiscServices
} // namespace OHOS
