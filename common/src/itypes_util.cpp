/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "itypes_util.h"

#include "global.h"
#include "iremote_object.h"

namespace OHOS {
namespace MiscServices {
bool ITypesUtil::Marshal(MessageParcel &data)
{
    return true;
}

bool ITypesUtil::Unmarshal(MessageParcel &data)
{
    return true;
}

bool ITypesUtil::Marshalling(int32_t input, MessageParcel &data)
{
    return data.WriteInt32(input);
}

bool ITypesUtil::Unmarshalling(int32_t &output, MessageParcel &data)
{
    return data.ReadInt32(output);
}

bool ITypesUtil::Marshalling(const std::string &input, MessageParcel &data)
{
    return data.WriteString(input);
}

bool ITypesUtil::Unmarshalling(std::string &output, MessageParcel &data)
{
    return data.ReadString(output);
}
} // namespace MiscServices
} // namespace OHOS