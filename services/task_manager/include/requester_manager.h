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

#ifndef IMF_SERVICES_REQUESTER_MANAGER_H
#define IMF_SERVICES_REQUESTER_MANAGER_H

#include <stdint.h>

#include <memory>
#include <mutex>
#include <unordered_map>

#include "input_death_recipient.h"
#include "iremote_object.h"
#include "iima_response_channel.h"
#include "iimc_response_channel.h"

namespace OHOS {
namespace MiscServices {
struct RequesterInfo {
    uint32_t requestCount{ 0 };
    sptr<IImaResponseChannel> imaResponseChannel{ nullptr };
    sptr<IImcResponseChannel> imcResponseChannel{ nullptr };
    sptr<InputDeathRecipient> deathRecipient{ nullptr };
    void AddChannel(sptr<IImaResponseChannel> imaChannel, sptr<IImcResponseChannel> imcChannel)
    {
        if (imaChannel != nullptr) {
            imaResponseChannel = imaChannel;
        }
        if (imcChannel != nullptr) {
            imcResponseChannel = imcChannel;
        }
    }
};

class RequesterManager : public std::enable_shared_from_this<RequesterManager> {
private:
    RequesterManager() = default;

public:
    ~RequesterManager() = default;

    RequesterManager(const RequesterManager &) = delete;
    RequesterManager(RequesterManager &&) = delete;
    RequesterManager &operator=(const RequesterManager &) = delete;
    RequesterManager &operator=(RequesterManager &&) = delete;

    static RequesterManager &GetInstance();

    std::shared_ptr<RequesterInfo> GetRequester(int32_t pid);
    int32_t AddImaChannel(int32_t pid, sptr<IImaResponseChannel> channel);
    int32_t AddImcChannel(int32_t pid, sptr<IImcResponseChannel> channel);

    void TaskIn(int32_t pid);
    void TaskOut(int32_t pid);

private:
    int32_t AddChannel(int32_t pid, sptr<IImaResponseChannel> imaChannel, sptr<IImcResponseChannel> imcChannel);
    void OnClientDied(int32_t pid);
    std::mutex mutex_{};
    std::unordered_map<int32_t, std::shared_ptr<RequesterInfo>> requesterMap_;
};
} // namespace MiscServices
} // namespace OHOS

#endif // IMF_SERVICES_REQUESTER_MANAGER_H
