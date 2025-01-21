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

#include "imc_hisysevent_reporter.h"

namespace OHOS {
namespace MiscServices {
ImcHiSysEventReporter &ImcHiSysEventReporter::GetInstance()
{
    static ImcHiSysEventReporter instance;
    return instance;
}

ImcHiSysEventReporter::ImcHiSysEventReporter()
{
}

ImcHiSysEventReporter::~ImcHiSysEventReporter()
{
}

bool ImcHiSysEventReporter::IsValidErrCode(int32_t errCode)
{
    return !((ErrorCode::ERROR_IMA_BEGIN < errCode && errCode < ErrorCode::ERROR_IMA_END)
             || (ErrorCode::ERROR_IMSA_BEGIN < errCode && errCode < ErrorCode::ERROR_IMSA_END));
}

bool ImcHiSysEventReporter::IsFault(int32_t errCode)
{
    return errCode != ErrorCode::ERROR_TEXT_PREVIEW_NOT_SUPPORTED;
}
} // namespace MiscServices
} // namespace OHOS