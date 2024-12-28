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

#include "native_inputmethod_utils.h"
#include <gtest/gtest.h>

using namespace OHOS::MiscServices;

HWTEST_F(ErrorCodeConvertTest, ErrorCodeExistsInMap_ReturnsCorrectInputMethodErrorCode)
{
    int32_t errorCode = ErrorCode::ERROR_CLIENT_NOT_FOUND;
    InputMethod_ErrorCode expected = IME_ERR_IMCLIENT;
    InputMethod_ErrorCode result = ErrorCodeConvert(errorCode);
    EXPECT_EQ(result, expected);
}

HWTEST_F(ErrorCodeConvertTest, ErrorCodeDoesNotExistInMap_ReturnsIME_ERR_UNDEFINED)
{
    int32_t errorCode = 9999; // An error code not in the map
    InputMethod_ErrorCode expected = IME_ERR_UNDEFINED;
    InputMethod_ErrorCode result = ErrorCodeConvert(errorCode);
    EXPECT_EQ(result, expected);
}