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

#ifndef IMF_IMC_RESPONSE_CHANNEL_IMPL_H
#define IMF_IMC_RESPONSE_CHANNEL_IMPL_H

#include "iimc_response_channel.h"
#include "imc_response_channel_stub.h"
#include "iremote_object.h"
#include "service_response_data.h"

namespace OHOS {
namespace MiscServices {
class ImcResponseChannelImpl final
    : public ImcResponseChannelStub
        , public std::enable_shared_from_this<ImcResponseChannelImpl> {
    DISALLOW_COPY_AND_MOVE(ImcResponseChannelImpl);

public:
    ImcResponseChannelImpl();
    ~ImcResponseChannelImpl();
    ErrCode OnResponse(uint32_t requestId, int32_t resultErrCode, const ServiceResponseDataInner &response) override;
};
} // namespace MiscServices
} // namespace OHOS
#endif // IMF_IMC_RESPONSE_CHANNEL_IMPL_H
